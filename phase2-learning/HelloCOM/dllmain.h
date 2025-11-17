// dllmain.h : 모듈 클래스의 선언입니다.

class CHelloCOMModule : public ATL::CAtlDllModuleT< CHelloCOMModule >
{
public :
	DECLARE_LIBID(LIBID_HelloCOMLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_HELLOCOM, "{54b80cd5-fcd6-46b0-a39b-792cfbf5f4fa}")
};

extern class CHelloCOMModule _AtlModule;
