// AudioCaptureEngine.h
// PID 기반 WASAPI Process Loopback 캡처를 담당하는 래퍼 클래스
#pragma once

#include <windows.h>
#include <vector>

#include "ProcessLoopbackCapture.h"  // PID 기반 캡처 엔진

// COM 쪽(COnVoiceCapture)으로 오디오 데이터를 전달하기 위한 콜백 인터페이스
class IAudioDataCallback {
public:
    virtual ~IAudioDataCallback() {}

    // pData: PCM 바이트 포인터
    // dataSize: 바이트 길이
    virtual void OnAudioData(BYTE* pData, UINT32 dataSize) = 0;
};

// PID 기반 WASAPI 캡처 엔진 (ProcessLoopbackCapture thin wrapper)
class AudioCaptureEngine {
public:
    AudioCaptureEngine();
    ~AudioCaptureEngine();

    // 특정 PID 프로세스의 오디오 캡처 시작
    HRESULT Start(DWORD pid, IAudioDataCallback* pCallback);

    // 캡처 중지 (COM에서 호출하는 외부 API)
    //  - 내부에서는 별도 스레드에서 StopInternal() 을 실행
    HRESULT Stop();

    // 현재 캡처 중인지 여부
    bool IsCapturing();

    // (선택) 디버깅용: 내부 상태 그대로 노출
    eCaptureState GetState();

private:
    // ProcessLoopbackCapture 에 넘기는 static 콜백
    static void LoopbackCallback(
        const std::vector<unsigned char>::iterator& begin,
        const std::vector<unsigned char>::iterator& end,
        void* userData);

    // 실제 콜백 처리 (IAudioDataCallback 으로 전달)
    void HandleLoopbackData(
        const std::vector<unsigned char>::iterator& begin,
        const std::vector<unsigned char>::iterator& end);

    // Stop 전용 워커 스레드 엔트리
    static DWORD WINAPI StopThreadProc(LPVOID param);

    // 실제 동기 Stop 로직 (StopCapture 호출)
    HRESULT StopInternal();

private:
    ProcessLoopbackCapture m_capture;   // PID 기반 캡처 엔진
    IAudioDataCallback* m_pCallback; // COnVoiceCapture 의 콜백 (OnAudioData)

    HANDLE                 m_hStopThread;    // Stop 전용 스레드 핸들
    volatile LONG          m_stopThreadFlag; // 0=없음, 1=실행 중
};
