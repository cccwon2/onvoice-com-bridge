// dllmain.h : 모듈 클래스의 선언입니다.

class COnVoiceAudioBridgeModule : public ATL::CAtlDllModuleT< COnVoiceAudioBridgeModule >
{
public :
	DECLARE_LIBID(LIBID_OnVoiceAudioBridgeLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_ONVOICEAUDIOBRIDGE, "{419618dd-d242-4428-8f62-5651cb46a9be}")
};

extern class COnVoiceAudioBridgeModule _AtlModule;
