// AudioCaptureEngine.cpp
#include "pch.h"
#include "AudioCaptureEngine.h"
#include <stdio.h>
#include <objbase.h>

DWORD WINAPI AudioCaptureEngine::StopThreadProc(LPVOID param)
{
    AudioCaptureEngine* self = static_cast<AudioCaptureEngine*>(param);
    if (!self)
        return 0;

    self->StopInternal();
    ::InterlockedExchange(&self->m_stopThreadFlag, 0);
    return 0;
}

AudioCaptureEngine::AudioCaptureEngine()
    : m_pCallback(nullptr)
    , m_hStopThread(nullptr)
    , m_stopThreadFlag(0)
{
    printf("[Engine] AudioCaptureEngine (PID 기반) 생성\n");
}

AudioCaptureEngine::~AudioCaptureEngine()
{
    printf("[Engine] AudioCaptureEngine 소멸자 시작\n");

    // 이미 Stop 스레드가 돌고 있으면 먼저 기다린다.
    if (m_hStopThread)
    {
        printf("[Engine] 소멸자: Stop 스레드 대기 중...\n");
        DWORD dw = WaitForSingleObject(m_hStopThread, 5000);
        printf("[Engine] 소멸자: Stop 스레드 wait 결과=0x%08X\n", dw);
        CloseHandle(m_hStopThread);
        m_hStopThread = nullptr;
    }

    // 아직 READY 상태가 아니라면 내부적으로 한 번 더 정리 시도
    StopInternal();

    printf("[Engine] AudioCaptureEngine 소멸자 끝\n");
}

HRESULT AudioCaptureEngine::Start(DWORD pid, IAudioDataCallback* pCallback)
{
    printf("[Engine] Start(pid=%lu) 호출\n", pid);

    if (pid == 0) {
        printf("[Engine] ❌ 잘못된 PID (0)\n");
        return E_INVALIDARG;
    }

    if (pCallback == nullptr) {
        printf("[Engine] ❌ pCallback == nullptr\n");
        return E_POINTER;
    }

    eCaptureState state = m_capture.GetState();
    if (state != eCaptureState::READY) {
        printf("[Engine] ❌ 현재 상태에서 StartCapture 불가 (state=%d)\n", (int)state);
        return HRESULT_FROM_WIN32(ERROR_BUSY);
    }

    m_pCallback = pCallback;

    // 1) 캡처 형식 설정
    eCaptureError err = m_capture.SetCaptureFormat(
        16000,
        16,
        1,
        WAVE_FORMAT_PCM
    );

    if (err != eCaptureError::NONE) {
        printf("[Engine] ❌ SetCaptureFormat 실패: %s\n",
            LoopbackCaptureConst::GetErrorText(err));
        m_pCallback = nullptr;
        return E_FAIL;
    }

    printf("[Engine] ✅ SetCaptureFormat 성공 (16kHz, 16-bit, mono, PCM)\n");

    // 2) 타깃 프로세스 설정
    err = m_capture.SetTargetProcess(pid, true);
    if (err != eCaptureError::NONE) {
        printf("[Engine] ❌ SetTargetProcess 실패: %s\n",
            LoopbackCaptureConst::GetErrorText(err));
        m_pCallback = nullptr;
        return E_FAIL;
    }

    printf("[Engine] ✅ SetTargetProcess 성공 (PID=%lu, inclusive=true)\n", pid);

    // 3) 콜백 등록
    err = m_capture.SetCallback(&AudioCaptureEngine::LoopbackCallback, this);
    if (err != eCaptureError::NONE) {
        printf("[Engine] ❌ SetCallback 실패: %s\n",
            LoopbackCaptureConst::GetErrorText(err));
        m_pCallback = nullptr;
        return E_FAIL;
    }

    printf("[Engine] ✅ SetCallback 성공\n");

    // ⭐ 중간 스레드 기본값 사용 (PoC와 동일)
    printf("[Engine] ✅ IntermediateThread 기본값 사용\n");

    // 4) 캡처 시작
    printf("[Engine] StartCapture 호출 중...\n");
    err = m_capture.StartCapture();
    if (err != eCaptureError::NONE) {
        HRESULT hrLast = m_capture.GetLastErrorResult();
        printf("[Engine] ❌ StartCapture 실패: %s (hr=0x%08X)\n",
            LoopbackCaptureConst::GetErrorText(err), hrLast);
        m_pCallback = nullptr;
        return (hrLast != S_OK) ? hrLast : E_FAIL;
    }

    // 5) 상태 확인
    state = m_capture.GetState();
    printf("[Engine] ✅ StartCapture 성공! (PID=%lu, state=%d)\n", pid, (int)state);

    if (state != eCaptureState::CAPTURING) {
        printf("[Engine] ⚠️  경고: 상태가 CAPTURING이 아닙니다 (state=%d)\n", (int)state);
    }

    printf("[Engine] 오디오 캡처가 시작되었습니다.\n");
    return S_OK;
}

// COM(COnVoiceCapture)에서 부르는 Stop()
// → 가능한 한 빨리 리턴하고, 내부 스레드에서 StopInternal() 실행
HRESULT AudioCaptureEngine::Stop()
{
    eCaptureState state = m_capture.GetState();
    printf("[Engine] Stop() 호출 (state=%d)\n", (int)state);

    if (state == eCaptureState::READY) {
        printf("[Engine] 이미 READY 상태 (중지됨)\n");
        m_pCallback = nullptr;
        return S_OK;
    }

    // 이미 Stop 스레드가 돌고 있다면 추가 요청은 무시
    LONG oldFlag = ::InterlockedCompareExchange(&m_stopThreadFlag, 1, 0);
    if (oldFlag != 0) {
        printf("[Engine] Stop() 이미 실행 중 (중복 호출 무시)\n");
        return S_OK;
    }

    // 이전 Stop 스레드 핸들이 남아 있으면 정리
    if (m_hStopThread) {
        CloseHandle(m_hStopThread);
        m_hStopThread = nullptr;
    }

    HANDLE hThread = ::CreateThread(
        nullptr,
        0,
        &AudioCaptureEngine::StopThreadProc,
        this,
        0,
        nullptr
    );

    if (!hThread)
    {
        DWORD err = GetLastError();
        printf("[Engine] ❌ Stop() 스레드 생성 실패 (err=%lu), 동기 StopInternal() 수행\n", err);
        ::InterlockedExchange(&m_stopThreadFlag, 0);
        return StopInternal();
    }

    m_hStopThread = hThread;

    printf("[Engine] Stop() 비동기 요청 완료\n");
    return S_OK;
}

HRESULT AudioCaptureEngine::StopInternal()
{
    eCaptureState state = m_capture.GetState();
    printf("[Engine] StopInternal() 실행 (state=%d)\n", (int)state);

    if (state == eCaptureState::READY) {
        printf("[Engine] StopInternal: 이미 READY 상태 (중지됨)\n");
        m_pCallback = nullptr;
        return S_OK;
    }

    eCaptureError err = m_capture.StopCapture();
    if (err != eCaptureError::NONE) {
        HRESULT hrLast = m_capture.GetLastErrorResult();
        printf("[Engine] ⚠️  StopCapture 실패: %s (hr=0x%08X)\n",
            LoopbackCaptureConst::GetErrorText(err), hrLast);
        m_pCallback = nullptr;
        return (hrLast != S_OK) ? hrLast : E_FAIL;
    }

    // 라이브러리가 내부 스레드를 정리할 시간을 약간 준다
    Sleep(200);

    m_pCallback = nullptr;
    printf("[Engine] ✅ StopCapture 성공, 상태 READY\n");
    return S_OK;
}

bool AudioCaptureEngine::IsCapturing()
{
    return m_capture.GetState() == eCaptureState::CAPTURING;
}

eCaptureState AudioCaptureEngine::GetState()
{
    return m_capture.GetState();
}

// LoopbackCapture 가 호출하는 정적 콜백
void AudioCaptureEngine::LoopbackCallback(
    const std::vector<unsigned char>::iterator& begin,
    const std::vector<unsigned char>::iterator& end,
    void* userData)
{
    AudioCaptureEngine* pThis = static_cast<AudioCaptureEngine*>(userData);
    if (!pThis) return;

    pThis->HandleLoopbackData(begin, end);
}

void AudioCaptureEngine::HandleLoopbackData(
    const std::vector<unsigned char>::iterator& begin,
    const std::vector<unsigned char>::iterator& end)
{
    if (!m_pCallback)
    {
        printf("[Engine] ⚠️  HandleLoopbackData: m_pCallback is NULL!\n");
        return;
    }

    const size_t size = static_cast<size_t>(std::distance(begin, end));

    static int callCount = 0;
    printf("[Engine] HandleLoopbackData: size=%zu bytes [#%d] (state=%d)\n",
        size, callCount, (int)m_capture.GetState());
    callCount++;

    if (size == 0)
    {
        printf("[Engine] ⚠️  빈 데이터 (size=0)\n");
        return;
    }

    // COM 초기화 (스레드별 1회)
    static thread_local bool s_comInitialized = false;
    if (!s_comInitialized)
    {
        HRESULT hrCo = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (SUCCEEDED(hrCo) || hrCo == RPC_E_CHANGED_MODE)
        {
            s_comInitialized = true;
            printf("[Engine] 오디오 스레드 COM 초기화 완료 (TID=%lu)\n", GetCurrentThreadId());
        }
        else
        {
            printf("[Engine] ❌ COM 초기화 실패 (HR=0x%08X)\n", hrCo);
            return;
        }
    }

    BYTE* pData = reinterpret_cast<BYTE*>(&(*begin));
    m_pCallback->OnAudioData(pData, static_cast<UINT32>(size));
}
