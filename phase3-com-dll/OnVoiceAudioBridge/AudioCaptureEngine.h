// AudioCaptureEngine.h
// 실제 WASAPI 오디오 캡처를 담당하는 클래스
#pragma once

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>

// 캡처 상태를 나타내는 열거형
enum CaptureState {
    STATE_STOPPED = 0,      // 중지됨
    STATE_STARTING = 1,     // 시작 중
    STATE_CAPTURING = 2,    // 캡처 중
    STATE_STOPPING = 3      // 중지 중
};

// 오디오 데이터를 전달할 콜백 인터페이스
class IAudioDataCallback {
public:
    virtual ~IAudioDataCallback() {}

    // 캡처된 오디오 데이터를 받는 함수
    // pData: 오디오 데이터 포인터
    // dataSize: 데이터 크기 (바이트)
    virtual void OnAudioData(BYTE* pData, UINT32 dataSize) = 0;
};

// WASAPI를 사용한 프로세스별 오디오 캡처 엔진
class AudioCaptureEngine {
private:
    // COM 객체들
    IAudioClient* m_pAudioClient;           // 오디오 세션
    IAudioCaptureClient* m_pCaptureClient;  // 캡처 인터페이스

    // 스레드 관련
    HANDLE m_hCaptureThread;     // 캡처 스레드 핸들
    HANDLE m_hStopEvent;         // 중지 신호 이벤트

    // 상태
    CaptureState m_state;        // 현재 상태
    DWORD m_targetPid;           // 대상 프로세스 ID

    // 콜백
    IAudioDataCallback* m_pCallback;  // 데이터 수신자

    // 스레드 함수 (static이어야 CreateThread에 전달 가능)
    static DWORD WINAPI CaptureThreadProc(LPVOID lpParam);

    // 실제 캡처 로직 (멤버 함수)
    void DoCaptureLoop();

public:
    AudioCaptureEngine();
    ~AudioCaptureEngine();

    // 캡처 시작 (PID 지정)
    HRESULT Start(DWORD pid, IAudioDataCallback* pCallback);

    // 캡처 중지
    HRESULT Stop();

    // 현재 상태 확인
    CaptureState GetState() const { return m_state; }
    bool IsCapturing() const { return m_state == STATE_CAPTURING; }
};
