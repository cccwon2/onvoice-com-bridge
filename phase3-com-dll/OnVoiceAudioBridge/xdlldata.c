// dlldata.c의 래퍼입니다.

#ifdef _MERGE_PROXYSTUB // 프록시/스텁 DLL을 병합합니다.

#define REGISTER_PROXY_DLL //DllRegisterServer 등입니다.

#define USE_STUBLESS_PROXY	//MIDL 스위치 /Oicf로만 정의됩니다.

#pragma comment(lib, "rpcns4.lib")
#pragma comment(lib, "rpcrt4.lib")

#define ENTRY_PREFIX	Prx

#include "dlldata.c"
#include "OnVoiceAudioBridge_p.c"

#endif //_MERGE_PROXYSTUB
