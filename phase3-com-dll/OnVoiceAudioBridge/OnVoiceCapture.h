// OnVoiceCapture.h : COnVoiceCapture 선언

#pragma once
#include "resource.h"       // 주 기호입니다.

#include "OnVoiceAudioBridge_i.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "단일 스레드 COM 개체는 전체 DCOM 지원을 포함하지 않는 Windows Mobile 플랫폼과 같은 Windows CE 플랫폼에서 제대로 지원되지 않습니다. ATL이 단일 스레드 COM 개체의 생성을 지원하고 단일 스레드 COM 개체 구현을 사용할 수 있도록 _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA를 정의하십시오. rgs 파일의 스레딩 모델은 DCOM Windows CE가 아닌 플랫폼에서 지원되는 유일한 스레딩 모델이므로 'Free'로 설정되어 있습니다."
#endif

using namespace ATL;

// COnVoiceCapture

class ATL_NO_VTABLE COnVoiceCapture :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<COnVoiceCapture, &CLSID_OnVoiceCapture>,
	public IDispatchImpl<IOnVoiceCapture, &IID_IOnVoiceCapture, &LIBID_OnVoiceAudioBridgeLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	// ================================
	// ★ 이벤트용 ATL 베이스 클래스 추가
	// ================================
	public IConnectionPointContainerImpl<COnVoiceCapture>,                             // 이벤트 컨테이너
	public IConnectionPointImpl<COnVoiceCapture, &__uuidof(_IOnVoiceCaptureEvents)>    // _IOnVoiceCaptureEvents 소스
{
public:
	COnVoiceCapture()
	{
		// 멤버 변수 초기화
		m_bIsCapturing = FALSE;  // 초기 상태: 캡처 중지
		m_targetPid = 0;         // PID 없음
	}

	DECLARE_REGISTRY_RESOURCEID(106)

	// ================================
	// COM 맵
	// ================================
	BEGIN_COM_MAP(COnVoiceCapture)
		COM_INTERFACE_ENTRY(IOnVoiceCapture)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IConnectionPointContainer) // ★ 이벤트 컨테이너 인터페이스 노출
	END_COM_MAP()

	// ================================
	// ★ Connection Point 맵
	// ================================
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
	}

public:
	// ========================================
	// IOnVoiceCapture 인터페이스 구현
	// ========================================

	// 캡처 시작
	STDMETHOD(StartCapture)(LONG processId);

	// 캡처 중지
	STDMETHOD(StopCapture)();

	// 상태 확인
	STDMETHOD(GetCaptureState)(LONG* pState);

private:
	// ========================================
	// 멤버 변수
	// ========================================
	BOOL m_bIsCapturing;   // 캡처 중인지 여부 (TRUE/FALSE)
	LONG m_targetPid;      // 대상 프로세스 ID
};

OBJECT_ENTRY_AUTO(__uuidof(OnVoiceCapture), COnVoiceCapture)
