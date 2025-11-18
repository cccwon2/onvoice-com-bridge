#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <mfapi.h>
#include <shobjidl.h>                 // IAgileObject
#include <audioclientactivationparams.h>  // ✅ Process loopback용 구조체 + 상수

#include <iostream>

#pragma comment(lib, "mmdevapi.lib")
#pragma comment(lib, "avrt.lib")
#pragma comment(lib, "mfplat.lib")

// ========================================
// Phase 4.2: 비동기 완료 핸들러 클래스
// ========================================

class ActivateAudioInterfaceCompletionHandler :
    public IActivateAudioInterfaceCompletionHandler,
    public IAgileObject              // ✅ MTA 스레드에서도 안전하게 호출되도록
{
private:
    LONG    m_refCount;
    HANDLE  m_hEvent;
    HRESULT m_hrActivateResult;
    IUnknown* m_pUnknown;

public:
    ActivateAudioInterfaceCompletionHandler()
        : m_refCount(1),
        m_hEvent(CreateEvent(NULL, FALSE, FALSE, NULL)),
        m_hrActivateResult(E_FAIL),
        m_pUnknown(nullptr)
    {
        printf("[핸들러] 생성됨 (참조 카운트: %ld, 이벤트: %p)\n",
            m_refCount, m_hEvent);
    }

    virtual ~ActivateAudioInterfaceCompletionHandler()
    {
        printf("[핸들러] 소멸 시작 (참조 카운트: %ld)\n", m_refCount);

        if (m_hEvent) {
            CloseHandle(m_hEvent);
            m_hEvent = NULL;
        }

        if (m_pUnknown) {
            m_pUnknown->Release();
            m_pUnknown = NULL;
        }

        printf("[핸들러] 소멸 완료\n");
    }

    // IUnknown ===========================

    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override
    {
        printf("[핸들러] QueryInterface 호출됨\n");

        if (!ppvObject)
            return E_POINTER;

        if (riid == __uuidof(IUnknown) ||
            riid == __uuidof(IActivateAudioInterfaceCompletionHandler)) {

            *ppvObject =
                static_cast<IActivateAudioInterfaceCompletionHandler*>(this);
            AddRef();
            return S_OK;
        }
        else if (riid == __uuidof(IAgileObject)) {
            *ppvObject = static_cast<IAgileObject*>(this);
            AddRef();
            return S_OK;
        }

        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef() override
    {
        LONG count = InterlockedIncrement(&m_refCount);
        printf("[핸들러] AddRef: %ld\n", count);
        return count;
    }

    STDMETHODIMP_(ULONG) Release() override
    {
        LONG count = InterlockedDecrement(&m_refCount);
        printf("[핸들러] Release: %ld\n", count);

        if (count == 0) {
            printf("[핸들러] 참조 카운트 0 → 삭제!\n");
            delete this;
            return 0;
        }

        return count;
    }

    // IActivateAudioInterfaceCompletionHandler ==========

    STDMETHODIMP ActivateCompleted(
        IActivateAudioInterfaceAsyncOperation* operation) override
    {
        printf("[핸들러] ActivateCompleted 호출됨! 🎉\n");

        HRESULT hrActivateResult = E_FAIL;
        IUnknown* pUnknown = nullptr;

        HRESULT hr = operation->GetActivateResult(&hrActivateResult, &pUnknown);
        if (FAILED(hr)) {
            printf("[핸들러] ❌ GetActivateResult 실패: 0x%08X\n", hr);
            m_hrActivateResult = hr;
            SetEvent(m_hEvent);
            return hr;
        }

        if (FAILED(hrActivateResult)) {
            printf("[핸들러] ❌ 오디오 활성화 실패: 0x%08X\n", hrActivateResult);
            m_hrActivateResult = hrActivateResult;
            if (pUnknown) {
                pUnknown->Release();
            }
            SetEvent(m_hEvent);
            return hrActivateResult;
        }

        m_hrActivateResult = hrActivateResult;
        m_pUnknown = pUnknown;    // 이후 소멸자에서 Release

        printf("[핸들러] ✅ 활성화 성공! IAudioClient 획득\n");

        SetEvent(m_hEvent);
        return S_OK;
    }

    // 헬퍼 ==============================

    HANDLE GetEvent() const { return m_hEvent; }

    HRESULT GetActivateResult(HRESULT* phr, IUnknown** ppUnknown)
    {
        if (!phr || !ppUnknown)
            return E_POINTER;

        *phr = m_hrActivateResult;
        *ppUnknown = m_pUnknown;

        if (m_pUnknown)
            m_pUnknown->AddRef();

        return S_OK;
    }
};

// ========================================
// main
// ========================================

int main()
{
    // 1. COM 초기화 (MTA)
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        printf("COM 초기화 실패: 0x%08X\n", hr);
        return 1;
    }

    // 2. 기본 렌더 디바이스 확인 (디버깅용)
    IMMDeviceEnumerator* enumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&enumerator);
    if (FAILED(hr)) {
        printf("디바이스 열거자 생성 실패: 0x%08X\n", hr);
        CoUninitialize();
        return 1;
    }

    IMMDevice* device = nullptr;
    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    if (FAILED(hr)) {
        printf("기본 오디오 디바이스 가져오기 실패: 0x%08X\n", hr);
        enumerator->Release();
        CoUninitialize();
        return 1;
    }

    printf("✅ 오디오 디바이스 획득 성공!\n");

    LPWSTR deviceIdString = nullptr;
    hr = device->GetId(&deviceIdString);
    if (FAILED(hr)) {
        printf("❌ 디바이스 ID 가져오기 실패: 0x%08X\n", hr);
        device->Release();
        enumerator->Release();
        CoUninitialize();
        return 1;
    }

    wprintf(L"디바이스 ID: %s\n", deviceIdString);

    // ========================================
    // PID 기반 루프백 파라미터 설정
    // ========================================

    printf("\n=== Phase 4.3: PID 기반 캡처 시작 ===\n");

    DWORD targetPid = 21616;   // 🔧 여기만 실제 Chrome/Discord PID로 수정

    printf("대상 PID: %lu\n", targetPid);
    printf("프로세스: Chrome 브라우저 (추정)\n");

    // ProcessLoopbackCapture.cpp와 동일한 패턴으로 설정
    AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS loopbackParams = {};
    loopbackParams.TargetProcessId = targetPid;
    loopbackParams.ProcessLoopbackMode =
        PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE;

    AUDIOCLIENT_ACTIVATION_PARAMS blob = {};  // ✅ ProcessLoopbackCapture.cpp와 동일한 변수명 사용
    blob.ActivationType = AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK;
    blob.ProcessLoopbackParams = loopbackParams;

    PROPVARIANT activation_params = {};  // ✅ ProcessLoopbackCapture.cpp와 동일한 변수명 사용
    activation_params.vt = VT_BLOB;
    activation_params.blob.cbSize = sizeof(AUDIOCLIENT_ACTIVATION_PARAMS);
    activation_params.blob.pBlobData = reinterpret_cast<BYTE*>(&blob);  // ✅ blob (AUDIOCLIENT_ACTIVATION_PARAMS)의 주소를 가리킴

    printf("PROPVARIANT 래핑 완료 (크기: %lu 바이트)\n",
        activation_params.blob.cbSize);

    // ========================================
    // ActivateAudioInterfaceAsync 호출
    // ========================================

    printf("\n=== Phase 4.4: 비동기 오디오 활성화 ===\n");

    ActivateAudioInterfaceCompletionHandler* pHandler =
        new ActivateAudioInterfaceCompletionHandler();

    if (!pHandler) {
        printf("❌ 핸들러 생성 실패!\n");
        CoTaskMemFree(deviceIdString);
        device->Release();
        enumerator->Release();
        CoUninitialize();
        return 1;
    }

    printf("✅ 완료 핸들러 생성됨\n");

    IActivateAudioInterfaceAsyncOperation* pAsyncOp = nullptr;

    hr = ActivateAudioInterfaceAsync(
        VIRTUAL_AUDIO_DEVICE_PROCESS_LOOPBACK,   // ✅ 핵심 수정
        __uuidof(IAudioClient),
        &activation_params,  // ✅ 변수명 수정
        pHandler,
        &pAsyncOp);

    if (FAILED(hr)) {
        printf("❌ ActivateAudioInterfaceAsync 실패: 0x%08X\n", hr);
        pHandler->Release();
        CoTaskMemFree(deviceIdString);
        device->Release();
        enumerator->Release();
        CoUninitialize();
        return 1;
    }

    printf("✅ 비동기 활성화 호출 성공! (작업 진행 중...)\n");

    // 완료 대기 (최대 5초)
    printf("완료 대기 중");
    DWORD waitResult = WaitForSingleObject(pHandler->GetEvent(), 5000);
    printf(" → ");

    if (waitResult == WAIT_TIMEOUT) {
        printf("❌ 타임아웃! (5초 초과)\n");
        pAsyncOp->Release();
        pHandler->Release();
        CoTaskMemFree(deviceIdString);
        device->Release();
        enumerator->Release();
        CoUninitialize();
        return 1;
    }

    if (waitResult != WAIT_OBJECT_0) {
        printf("❌ 대기 실패: %lu\n", waitResult);
        pAsyncOp->Release();
        pHandler->Release();
        CoTaskMemFree(deviceIdString);
        device->Release();
        enumerator->Release();
        CoUninitialize();
        return 1;
    }

    printf("완료! ✅\n");

    HRESULT hrActivateResult = E_FAIL;
    IUnknown* pUnknown = nullptr;

    hr = pHandler->GetActivateResult(&hrActivateResult, &pUnknown);
    if (FAILED(hr)) {
        printf("❌ GetActivateResult 실패: 0x%08X\n", hr);
        pAsyncOp->Release();
        pHandler->Release();
        CoTaskMemFree(deviceIdString);
        device->Release();
        enumerator->Release();
        CoUninitialize();
        return 1;
    }

    if (FAILED(hrActivateResult)) {
        printf("❌ 오디오 활성화 실패: 0x%08X\n", hrActivateResult);
        if (hrActivateResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
            printf("   → VIRTUAL_AUDIO_DEVICE_PROCESS_LOOPBACK, PID 조합 문제일 수 있음\n");
        }

        if (pUnknown) pUnknown->Release();
        pAsyncOp->Release();
        pHandler->Release();
        CoTaskMemFree(deviceIdString);
        device->Release();
        enumerator->Release();
        CoUninitialize();
        return 1;
    }

    printf("✅ 활성화 성공!\n");

    IAudioClient* audioClient = nullptr;
    hr = pUnknown->QueryInterface(__uuidof(IAudioClient),
        (void**)&audioClient);
    pUnknown->Release();
    pUnknown = nullptr;

    if (FAILED(hr)) {
        printf("❌ IAudioClient 변환 실패: 0x%08X\n", hr);
        pAsyncOp->Release();
        pHandler->Release();
        CoTaskMemFree(deviceIdString);
        device->Release();
        enumerator->Release();
        CoUninitialize();
        return 1;
    }

    printf("✅ IAudioClient 획득 성공!\n");
    printf("✅ 이제 PID %lu 의 오디오만 캡처 가능합니다! 🎉\n", targetPid);

    // ...(여기부터는 실제 WASAPI 캡처 초기화/루프 들어갈 자리)...

    // 정리
    audioClient->Release();
    pAsyncOp->Release();
    pHandler->Release();
    CoTaskMemFree(deviceIdString);
    device->Release();
    enumerator->Release();
    CoUninitialize();

    printf("\n정리 완료!\n");
    printf("\nEnter 키를 눌러 종료...\n");
    getchar();

    return 0;
}
