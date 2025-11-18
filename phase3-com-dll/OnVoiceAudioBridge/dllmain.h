// dllmain.h : 모듈 클래스의 선언입니다.

class COnVoiceAudioBridgeModule : public ATL::CAtlDllModuleT< COnVoiceAudioBridgeModule >
{
public :
	DECLARE_LIBID(LIBID_OnVoiceAudioBridgeLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_ONVOICEAUDIOBRIDGE, "{03ece82b-7d84-476f-99e0-1f74529d22e2}")
};

extern class COnVoiceAudioBridgeModule _AtlModule;
