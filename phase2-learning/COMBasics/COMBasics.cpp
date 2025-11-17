#include <iostream>
#include <windows.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

// COM 라이브러리 링크
#pragma comment(lib, "ole32.lib")

using namespace std;

int main()
{
    cout << "=== COM 기초 실습: 오디오 디바이스 열거 ===" << endl << endl;

    // ===== 1단계: COM 초기화 (필수!) =====
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        cout << "❌ COM 초기화 실패: 0x" << hex << hr << endl;
        return 1;
    }
    cout << "✅ COM 초기화 성공" << endl << endl;

    // ===== 2단계: COM 객체 생성 =====
    IMMDeviceEnumerator* enumerator = NULL;
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),        // CLSID
        NULL,                                 // 집계 없음
        CLSCTX_ALL,                          // 컨텍스트
        __uuidof(IMMDeviceEnumerator),       // IID
        (void**)&enumerator                  // 결과 포인터
    );

    if (FAILED(hr)) {
        cout << "❌ 디바이스 열거자 생성 실패: 0x" << hex << hr << endl;
        CoUninitialize();
        return 1;
    }
    cout << "✅ 디바이스 열거자 생성 성공" << endl << endl;

    // ===== 3단계: 기본 오디오 디바이스 가져오기 =====
    IMMDevice* device = NULL;
    hr = enumerator->GetDefaultAudioEndpoint(
        eRender,    // 재생 디바이스
        eConsole,   // 콘솔 역할
        &device     // 결과 포인터
    );

    if (FAILED(hr)) {
        cout << "❌ 기본 오디오 디바이스 가져오기 실패: 0x" << hex << hr << endl;
        enumerator->Release();
        CoUninitialize();
        return 1;
    }
    cout << "✅ 기본 오디오 디바이스 획득 성공!" << endl << endl;

    // ===== 4단계: 디바이스 이름 가져오기 (보너스) =====
    IPropertyStore* props = NULL;
    hr = device->OpenPropertyStore(STGM_READ, &props);
    if (SUCCEEDED(hr)) {
        PROPVARIANT varName;
        PropVariantInit(&varName);

        hr = props->GetValue(PKEY_Device_FriendlyName, &varName);
        if (SUCCEEDED(hr)) {
            wcout << L"디바이스 이름: " << varName.pwszVal << endl << endl;
            PropVariantClear(&varName);
        }

        props->Release();  // ✅ Release 호출!
    }

    // ===== 5단계: 정리 (역순으로!) =====
    cout << "정리 중..." << endl;
    device->Release();       // device 해제
    enumerator->Release();   // enumerator 해제
    CoUninitialize();        // COM 해제
    cout << "✅ 모든 리소스 정리 완료" << endl;

    cout << endl << "계속하려면 아무 키나 누르세요..." << endl;
    getchar();
    return 0;
}