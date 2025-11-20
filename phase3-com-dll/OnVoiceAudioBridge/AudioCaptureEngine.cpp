// AudioCaptureEngine.cpp
#include "pch.h"
#include "AudioCaptureEngine.h"
#include <stdio.h>
#include <objbase.h>

AudioCaptureEngine::AudioCaptureEngine()
    : m_pCallback(nullptr)
{
    printf("[Engine] AudioCaptureEngine (PID 기반) 생성\n");
}

AudioCaptureEngine::~AudioCaptureEngine()
{
    printf("[Engine] AudioCaptureEngine 소멸자 시작\n");

    // 가능하면 여기서도 동일 스레드에서 정리되겠지만,
    // 규칙 상 "StartCapture와 같은 스레드"에서만 Stop을 호출해야 하므로
    // 너무 공격적으로 Stop() 을 세게 부르지는 않고,
    // 상태만 보고 필요할 때만 시도하는 정도로 둔다.
    eCaptureState state = m_capture.GetState();
    if (state != eCaptureState::READY)
    {
        printf("[Engine] 소멸자: 캡처가 아직 진행 중 → Stop() 시도\n");
        Stop();
    }

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

    // ⭐ 기본값: IntermediateThread / Queue 비활성 (라이브러리 기본 설정)
    printf("[Engine] ✅ IntermediateThread 기본값 사용 (QUEUE 미사용)\n");

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

HRESULT AudioCaptureEngine::Stop()
{
    eCaptureState state = m_capture.GetState();
    printf("[Engine] Stop() 호출 (state=%d)\n", (int)state);

    if (state == eCaptureState::READY) {
        m_pCallback = nullptr;
        return S_OK;
    }

    // ⭐ [핵심 수정 1] 콜백 연결을 먼저 끊습니다.
    // 이제 오디오 스레드는 데이터를 캡처해도 HandleLoopbackData에서 즉시 리턴하게 됩니다.
    m_pCallback = nullptr;

    // ⭐ [핵심 수정 2] 진행 중인 콜백이 빠져나갈 시간을 줍니다.
    // 만약 오디오 스레드가 이미 HandleLoopbackData 내부에 진입해 있다면,
    // 이 Sleep 동안 빠져나올 것입니다. (Join 전에 Main Thread가 잠시 숨을 쉅니다)
    printf("[Engine] 콜백 차단 및 잔여 작업 대기 (50ms)...\n");
    Sleep(50);

    // ⭐ [기존 로직] 이제 안전하게 스레드를 종료(Join)합니다.
    // 더 이상 오디오 스레드가 Main Thread로 Invoke를 날리지 않으므로 데드락이 걸리지 않습니다.
    eCaptureError err = m_capture.StopCapture();

    if (err != eCaptureError::NONE) {
        HRESULT hrLast = m_capture.GetLastErrorResult();
        printf("[Engine] ⚠️ StopCapture 실패: %s (hr=0x%08X)\n",
            LoopbackCaptureConst::GetErrorText(err), hrLast);
        return (hrLast != S_OK) ? hrLast : E_FAIL;
    }

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
    // ⭐ [핵심 수정 3] 콜백 포인터가 없으면 즉시 리턴 (Lock-free 안전장치)
    if (m_pCallback == nullptr)
    {
        // Stop()이 호출된 상태이므로 데이터를 무시합니다.
        return;
    }

    const size_t size = static_cast<size_t>(std::distance(begin, end));

    // (로그 노이즈를 줄이기 위해 주석 처리하거나 빈도 조절 가능)
    // printf("[Engine] HandleLoopbackData: size=%zu bytes\n", size);

    if (size == 0) return;

    // COM 초기화 (스레드별 1회)
    static thread_local bool s_comInitialized = false;
    if (!s_comInitialized)
    {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        s_comInitialized = true;
    }

    BYTE* pData = reinterpret_cast<BYTE*>(&(*begin));

    // 한 번 더 체크 (안전 제일)
    if (m_pCallback) {
        m_pCallback->OnAudioData(pData, static_cast<UINT32>(size));
    }
}
