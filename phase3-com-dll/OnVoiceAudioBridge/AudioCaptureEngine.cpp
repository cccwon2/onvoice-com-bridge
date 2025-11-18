// AudioCaptureEngine.cpp
#include "pch.h"
#include "AudioCaptureEngine.h"
#include <stdio.h>

// 생성자
AudioCaptureEngine::AudioCaptureEngine()
    : m_pAudioClient(nullptr)
    , m_pCaptureClient(nullptr)
    , m_hCaptureThread(nullptr)
    , m_hStopEvent(nullptr)
    , m_state(STATE_STOPPED)
    , m_targetPid(0)
    , m_pCallback(nullptr)
{
    m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (m_hStopEvent == NULL) {
        printf("[Engine] ❌ 중지 이벤트 생성 실패!\n");
    }
    else {
        printf("[Engine] ✅ AudioCaptureEngine 생성됨\n");
    }
}

// 소멸자
AudioCaptureEngine::~AudioCaptureEngine()
{
    printf("[Engine] 소멸자 시작...\n");

    // 1. 캡처가 실행 중이면 중지
    if (IsCapturing()) {
        Stop();
    }

    // 2. COM 객체 해제
    if (m_pCaptureClient) {
        m_pCaptureClient->Release();
        m_pCaptureClient = nullptr;
        printf("[Engine] CaptureClient 해제\n");
    }

    if (m_pAudioClient) {
        m_pAudioClient->Release();
        m_pAudioClient = nullptr;
        printf("[Engine] AudioClient 해제\n");
    }

    // 3. 이벤트 핸들 닫기
    if (m_hStopEvent) {
        CloseHandle(m_hStopEvent);
        m_hStopEvent = nullptr;
        printf("[Engine] StopEvent 해제\n");
    }

    printf("[Engine] ✅ AudioCaptureEngine 소멸 완료\n");
}

// ========================================
// 아래 함수들: 빈 스텁 (다음 단계에서 구현)
// ========================================

// 캡처 시작 (스텁)
HRESULT AudioCaptureEngine::Start(DWORD pid, IAudioDataCallback* pCallback)
{
    printf("[Engine] Start() 호출됨 (PID: %lu)\n", pid);

    // TODO: 다음 단계에서 구현
    m_targetPid = pid;
    m_pCallback = pCallback;
    m_state = STATE_STOPPED; // 아직 시작 안 함

    return S_OK;
}

// 캡처 중지 (스텁)
HRESULT AudioCaptureEngine::Stop()
{
    printf("[Engine] Stop() 호출됨\n");

    if (m_state == STATE_STOPPED) {
        printf("[Engine] 이미 중지 상태입니다\n");
        return S_OK;
    }

    // TODO: 다음 단계에서 스레드 중지 로직 구현

    // 중지 이벤트 신호
    if (m_hStopEvent) {
        SetEvent(m_hStopEvent);
    }

    // 스레드가 있으면 대기
    if (m_hCaptureThread) {
        printf("[Engine] 캡처 스레드 종료 대기...\n");
        WaitForSingleObject(m_hCaptureThread, 5000); // 5초 대기
        CloseHandle(m_hCaptureThread);
        m_hCaptureThread = nullptr;
    }

    // 상태 변경
    m_state = STATE_STOPPED;
    printf("[Engine] ✅ 중지 완료\n");

    return S_OK;
}

// 캡처 스레드 함수 (스텁)
DWORD WINAPI AudioCaptureEngine::CaptureThreadProc(LPVOID lpParam)
{
    printf("[Engine] 캡처 스레드 시작 (스텁)\n");

    // lpParam은 AudioCaptureEngine* this 포인터
    AudioCaptureEngine* pEngine = (AudioCaptureEngine*)lpParam;

    if (pEngine) {
        pEngine->DoCaptureLoop();
    }

    printf("[Engine] 캡처 스레드 종료\n");
    return 0;
}

// 실제 캡처 루프 (스텁)
void AudioCaptureEngine::DoCaptureLoop()
{
    printf("[Engine] DoCaptureLoop() 시작 (스텁)\n");

    // TODO: 다음 단계에서 Day 2 캡처 코드 이식

    // 지금은 1초 대기 후 종료
    Sleep(1000);

    printf("[Engine] DoCaptureLoop() 종료\n");
}