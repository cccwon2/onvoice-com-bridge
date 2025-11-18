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
    Stop();
    printf("[Engine] AudioCaptureEngine 소멸자 끝\n");
}

// =======================================================
// 캡처 시작 (PID 기반 Process Loopback)
// =======================================================
HRESULT AudioCaptureEngine::Start(DWORD pid, IAudioDataCallback* pCallback)
{
    printf("[Engine] Start(pid=%lu) 호출\n", pid);

    if (pid == 0) {
        printf("[Engine] 잘못된 PID (0)\n");
        return E_INVALIDARG;
    }

    if (pCallback == nullptr) {
        printf("[Engine] pCallback == nullptr\n");
        return E_POINTER;
    }

    // READY 상태가 아니면 Start 불가
    eCaptureState state = m_capture.GetState();
    if (state != eCaptureState::READY) {
        printf("[Engine] 현재 상태에서 StartCapture 불가 (state=%d)\n", (int)state);
        return HRESULT_FROM_WIN32(ERROR_BUSY);
    }

    m_pCallback = pCallback;

    // 1) STT 용도로 16kHz / 16bit / mono 설정 (PoC와 동일)
    eCaptureError err = m_capture.SetCaptureFormat(
        16000,             // SampleRate
        16,                // BitDepth
        1,                 // Channels
        WAVE_FORMAT_PCM    // PCM
    );

    if (err != eCaptureError::NONE) {
        printf("[Engine] ❌ SetCaptureFormat 실패: %s\n",
            LoopbackCaptureConst::GetErrorText(err));
        printf("[Engine] 가능한 원인: 캡처 형식 파라미터가 잘못됨\n");
        m_pCallback = nullptr;
        return E_FAIL;
    }

    // 2) 타깃 프로세스 설정 (inclusive: 이 PID 트리만 캡처)
    err = m_capture.SetTargetProcess(pid, /*bInclusive=*/true);
    if (err != eCaptureError::NONE) {
        printf("[Engine] ❌ SetTargetProcess 실패: %s\n",
            LoopbackCaptureConst::GetErrorText(err));
        printf("[Engine] 가능한 원인: PID가 잘못되었거나 캡처가 이미 시작됨\n");
        m_pCallback = nullptr;
        return E_FAIL;
    }

    // 3) 콜백 등록 (PoC의 AudioCallback 역할)
    err = m_capture.SetCallback(&AudioCaptureEngine::LoopbackCallback, this);
    if (err != eCaptureError::NONE) {
        printf("[Engine] ❌ SetCallback 실패: %s\n",
            LoopbackCaptureConst::GetErrorText(err));
        printf("[Engine] 가능한 원인: 콜백 함수가 NULL이거나 캡처가 이미 시작됨\n");
        m_pCallback = nullptr;
        return E_FAIL;
    }

    // (선택) 중간 스레드는 지금은 사용하지 않으므로 설정 생략
    // 필요하면 SetIntermediateThreadEnabled(false) 호출 가능

    // 4) 캡처 시작
    err = m_capture.StartCapture();
    if (err != eCaptureError::NONE) {
        HRESULT hrLast = m_capture.GetLastErrorResult();
        printf("[Engine] ❌ StartCapture 실패: %s (hr=0x%08X)\n",
            LoopbackCaptureConst::GetErrorText(err), hrLast);
        printf("[Engine] 가능한 원인:\n");
        printf("[Engine]   1. PID가 잘못됨 (프로세스가 종료됨)\n");
        printf("[Engine]   2. 해당 프로세스가 오디오를 재생하지 않음\n");
        printf("[Engine]   3. 오디오 디바이스 문제\n");
        if (hrLast == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
            printf("[Engine]   4. VIRTUAL_AUDIO_DEVICE_PROCESS_LOOPBACK, PID 조합 문제일 수 있음\n");
        }
        m_pCallback = nullptr;
        return (hrLast != S_OK) ? hrLast : E_FAIL;
    }

    printf("[Engine] ✅ StartCapture 성공 (PID=%lu)\n", pid);
    return S_OK;
}

// =======================================================
// 캡처 중지
// =======================================================
HRESULT AudioCaptureEngine::Stop()
{
    eCaptureState state = m_capture.GetState();
    printf("[Engine] Stop() 호출 (state=%d)\n", (int)state);

    if (state == eCaptureState::READY) {
        printf("[Engine] 이미 READY 상태 (중지됨)\n");
        m_pCallback = nullptr;
        return S_OK;
    }

    eCaptureError err = m_capture.StopCapture();
    if (err != eCaptureError::NONE) {
        HRESULT hrLast = m_capture.GetLastErrorResult();
        printf("[Engine] ⚠️  StopCapture 실패: %s (hr=0x%08X)\n",
            LoopbackCaptureConst::GetErrorText(err), hrLast);
        printf("[Engine] 가능한 원인: 내부 스레드 종료 중 문제 발생\n");
        m_pCallback = nullptr;
        return (hrLast != S_OK) ? hrLast : E_FAIL;
    }

    // 내부 캡처 스레드가 완전히 종료될 때까지 대기 (AudioCaptureTest 참고)
    // ProcessLoopbackCapture의 내부 스레드가 완전히 종료되도록 충분한 시간 확보
    Sleep(200);

    m_pCallback = nullptr;
    printf("[Engine] ✅ StopCapture 성공, 상태 READY\n");
    return S_OK;
}

// =======================================================
// 현재 캡처 중인지 여부
// =======================================================
bool AudioCaptureEngine::IsCapturing()
{
    return m_capture.GetState() == eCaptureState::CAPTURING;
}

eCaptureState AudioCaptureEngine::GetState()
{
    return m_capture.GetState();
}

// =======================================================
// static 콜백 → 인스턴스 메서드로 전달
// =======================================================
void AudioCaptureEngine::LoopbackCallback(
    const std::vector<unsigned char>::iterator& begin,
    const std::vector<unsigned char>::iterator& end,
    void* userData)
{
    AudioCaptureEngine* pThis = static_cast<AudioCaptureEngine*>(userData);
    if (!pThis) return;

    pThis->HandleLoopbackData(begin, end);
}

// =======================================================
// 실제 콜백 처리: IAudioDataCallback::OnAudioData 호출
// =======================================================
void AudioCaptureEngine::HandleLoopbackData(
    const std::vector<unsigned char>::iterator& begin,
    const std::vector<unsigned char>::iterator& end)
{
    if (!m_pCallback)
        return;

    const size_t size = static_cast<size_t>(std::distance(begin, end));
    if (size == 0)
        return;

    // 🔹 이 함수는 "ProcessLoopbackCapture의 오디오 스레드"에서 호출됩니다.
    //    이 스레드에서도 COM을 한 번 초기화해 줘야
    //    COnVoiceCapture::Fire_OnAudioData → IDispatch::Invoke 가 정상 동작합니다.
    static thread_local bool s_comInitialized = false;
    if (!s_comInitialized)
    {
        HRESULT hrCo = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (SUCCEEDED(hrCo) || hrCo == RPC_E_CHANGED_MODE)
        {
            s_comInitialized = true;
        }
    }

    BYTE* pData = reinterpret_cast<BYTE*>(&(*begin));
    m_pCallback->OnAudioData(pData, static_cast<UINT32>(size));
}
