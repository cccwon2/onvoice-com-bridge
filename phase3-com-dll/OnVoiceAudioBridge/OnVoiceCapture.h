// OnVoiceCapture.h : COnVoiceCapture 선언
#pragma once

#include "resource.h"
#include "OnVoiceAudioBridge_i.h"
#include "AudioCaptureEngine.h"  // ⭐ 오디오 캡처 엔진 클래스 정의 필요
#include "ProcessHelper.h"       // ⭐ 프로세스 찾기 유틸리티
#include <vector>
#include <atlbase.h>             // CComGITPtr

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "단일 스레드 COM 개체는 전체 DCOM 지원을 포함하지 않는 Windows Mobile 플랫폼과 같은 Windows CE 플랫폼에서 제대로 지원되지 않습니다."
#endif

using namespace ATL;

// ========================================
// CaptureState 열거형
//  - COM 외부에 숫자값(0~3)으로 전달
// ========================================
enum class CaptureState : LONG
{
    Stopped = 0,   // 중지
    Starting = 1,  // 시작 중
    Capturing = 2, // 캡처 중
    Stopping = 3   // 중지 중
};

// ========================================
// COnVoiceCapture COM 클래스
//  - AudioCaptureEngine 을 감싸고
//  - VBScript / Node.js(winax) 등과 연동
// ========================================
class ATL_NO_VTABLE COnVoiceCapture :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<COnVoiceCapture, &CLSID_OnVoiceCapture>,
    public IDispatchImpl<IOnVoiceCapture, &IID_IOnVoiceCapture, &LIBID_OnVoiceAudioBridgeLib, 1, 0>,
    public IConnectionPointContainerImpl<COnVoiceCapture>,
    public IConnectionPointImpl<COnVoiceCapture, &__uuidof(_IOnVoiceCaptureEvents)>,
    public IAudioDataCallback   // AudioCaptureEngine → 콜백
{
public:
    COnVoiceCapture()
        : m_pEngine(nullptr)
        , m_engineOwner(true)
        , m_state(CaptureState::Stopped)
        , m_targetPid(0)
        , m_ownerThreadId(GetCurrentThreadId())
    {
    }

    // 소멸자는 cpp 파일에 구현됨
    ~COnVoiceCapture();

    DECLARE_REGISTRY_RESOURCEID(106)

    BEGIN_COM_MAP(COnVoiceCapture)
        COM_INTERFACE_ENTRY(IOnVoiceCapture)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IConnectionPointContainer)
    END_COM_MAP()

    BEGIN_CONNECTION_POINT_MAP(COnVoiceCapture)
        CONNECTION_POINT_ENTRY(__uuidof(_IOnVoiceCaptureEvents))
    END_CONNECTION_POINT_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
        return S_OK;
    }

    void FinalRelease()
    {
        // 정리는 소멸자에서 처리
    }

public:
    // ========================================
    // IOnVoiceCapture 인터페이스 구현
    // ========================================

    // PID 기반 캡처 시작
    STDMETHOD(StartCapture)(LONG processId);

    // 캡처 중지
    STDMETHOD(StopCapture)();

    // 현재 상태 조회
    STDMETHOD(GetCaptureState)(LONG* pState);

    // Chrome 브라우저 프로세스 찾기
    STDMETHOD(FindChromeProcess)(LONG* pPid);

    // [수정] Edge 브라우저 프로세스 찾기 (누락되었던 부분 추가)
    STDMETHOD(FindEdgeProcess)(LONG* pPid);

    // Discord 프로세스 찾기
    STDMETHOD(FindDiscordProcess)(LONG* pPid);

    // ========================================
    // IAudioDataCallback 구현
    //  - AudioCaptureEngine → COnVoiceCapture 로 PCM 전달
    // ========================================
    void OnAudioData(BYTE* pData, UINT32 dataSize) override;

    // ========================================
    // 이벤트 브로드캐스트 헬퍼
    //  - RAW PCM → SAFEARRAY(VT_UI1) 로 감싸서
    //    _IOnVoiceCaptureEvents.OnAudioData(Byte[]) 호출
    // ========================================
    HRESULT Fire_OnAudioData(BYTE* pData, UINT32 dataSize);

    // ========================================
    // 테스트/DI 지원: 외부에서 엔진 주입
    // ========================================
    // 주의: AudioCaptureEngine* 구체 클래스로 변경하거나, 
    // IAudioCaptureEngine 인터페이스에 가상 소멸자가 있어야 함.
    void SetAudioCaptureEngine(AudioCaptureEngine* pEngine, bool takeOwnership = false)
    {
        if (m_pEngine && m_engineOwner) {
            m_pEngine->Stop();
            delete m_pEngine;
        }
        m_pEngine = pEngine;
        m_engineOwner = takeOwnership;
    }

private:
    // ========================================
    // 멤버 변수
    // ========================================

    // [수정] CPP 파일의 new AudioCaptureEngine()과 일치하도록 구체 클래스 포인터 사용 권장
    // 만약 인터페이스(IAudioCaptureEngine)를 쓰려면 반드시 가상 소멸자가 있어야 합니다.
    AudioCaptureEngine* m_pEngine;       // PID 기반 엔진 객체

    bool m_engineOwner;                  // 엔진 소유 여부 (삭제 책임)
    CaptureState m_state;                // 현재 상태
    LONG m_targetPid;                    // 타깃 PID
    DWORD m_ownerThreadId;               // 객체 생성 스레드 ID

    // 🔥 VBScript/JS 이벤트 싱크를 스레드 간 안전하게 호출하기 위한 GIT 프록시들
    std::vector<CComGITPtr<IDispatch>> m_gitSinks;
};

OBJECT_ENTRY_AUTO(__uuidof(OnVoiceCapture), COnVoiceCapture)