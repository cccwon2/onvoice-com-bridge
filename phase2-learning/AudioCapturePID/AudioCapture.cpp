#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <mfapi.h>
#include <shobjidl.h>   // IAgileObject
#include <iostream>
#include <new>          // std::nothrow

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "mmdevapi.lib")
#pragma comment(lib, "avrt.lib")
#pragma comment(lib, "mfplat.lib")

// ========================================
// ✅ 수동 타입 정의 (SDK 호환성 보장)
// ========================================

#ifndef AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS

typedef enum ProcessLoopbackMode {
    PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE = 0,
    PROCESS_LOOPBACK_MODE_EXCLUDE_TARGET_PROCESS_TREE = 1
} ProcessLoopbackMode;

typedef struct AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS {
    DWORD TargetProcessId;                      // 캡처할 프로세스 PID
    ProcessLoopbackMode ProcessLoopbackMode;    // 캡처 모드
} AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS;

typedef enum AUDIOCLIENT_ACTIVATION_TYPE {
    AUDIOCLIENT_ACTIVATION_TYPE_DEFAULT = 0,
    AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK = 1
} AUDIOCLIENT_ACTIVATION_TYPE;

typedef struct AUDIOCLIENT_ACTIVATION_PARAMS {
    AUDIOCLIENT_ACTIVATION_TYPE ActivationType;
    union {
        AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS ProcessLoopbackParams;
    };
} AUDIOCLIENT_ACTIVATION_PARAMS;

#endif  // AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS

// ========================================
// Phase 4.2: 비동기 완료 핸들러 클래스
// ========================================

class ActivateAudioInterfaceCompletionHandler :
    public IActivateAudioInterfaceCompletionHandler,
    public IAgileObject   // 스레드 경계 안전
{
private:
    LONG      m_refCount;
    HANDLE    m_hEvent;
    HRESULT   m_hrActivateResult;
    IUnknown* m_pUnknown;

public:
    ActivateAudioInterfaceCompletionHandler() :
        m_refCount(1),
        m_hEvent(CreateEvent(NULL, FALSE, FALSE, NULL)),
        m_hrActivateResult(E_FAIL),
        m_pUnknown(nullptr)
    {
        printf("[핸들러] 생성됨 (참조 카운트: %ld, 이벤트: %p)\n",
            m_refCount, m_hEvent);
    }

    virtual ~ActivateAudioInterfaceCompletionHandler() {
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

    // ===== IUnknown =====
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override {
        printf("[핸들러] QueryInterface 호출됨\n");

        if (ppvObject == nullptr)
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

    STDMETHODIMP_(ULONG) AddRef() override {
        LONG count = InterlockedIncrement(&m_refCount);
        printf("[핸들러] AddRef: %ld\n", count);
        return count;
    }

    STDMETHODIMP_(ULONG) Release() override {
        LONG count = InterlockedDecrement(&m_refCount);
        printf("[핸들러] Release: %ld\n", count);

        if (count == 0) {
            printf("[핸들러] 참조 카운트 0 → 삭제!\n");
            delete this;
            return 0;
        }
        return count;
    }

    // ===== IActivateAudioInterfaceCompletionHandler =====
    STDMETHODIMP ActivateCompleted(
        IActivateAudioInterfaceAsyncOperation* operation) override
    {
        printf("[핸들러] ActivateCompleted 호출됨! 🎉\n");

        HRESULT hr = S_OK;
        HRESULT hrActivateResult = E_FAIL;
        IUnknown* pUnknown = nullptr;

        hr = operation->GetActivateResult(&hrActivateResult, &pUnknown);
        if (FAILED(hr)) {
            printf("[핸들러] ❌ GetActivateResult 실패: 0x%X\n", hr);
            m_hrActivateResult = hr;
            SetEvent(m_hEvent);
            return hr;
        }

        if (FAILED(hrActivateResult)) {
            printf("[핸들러] ❌ 오디오 활성화 실패: 0x%X\n", hrActivateResult);
            m_hrActivateResult = hrActivateResult;
            if (pUnknown) {
                pUnknown->Release();
            }
            SetEvent(m_hEvent);
            return hrActivateResult;
        }

        m_hrActivateResult = hrActivateResult;
        m_pUnknown = pUnknown; // 나중에 Release

        printf("[핸들러] ✅ 활성화 성공! IAudioClient 획득\n");

        SetEvent(m_hEvent);
        return S_OK;
    }

    // ===== 헬퍼 =====
    HANDLE GetEvent() const {
        return m_hEvent;
    }

    HRESULT GetActivateResult(HRESULT* phr, IUnknown** ppUnknown) {
        if (!phr || !ppUnknown)
            return E_POINTER;

        *phr = m_hrActivateResult;
        *ppUnknown = m_pUnknown;   // ✅ 여기 오타 수정됨 (ppvUnknown → ppUnknown)

        if (m_pUnknown)
            m_pUnknown->AddRef();

        return S_OK;
    }
};

// ========================================
// main
// ========================================

int main() {
    HRESULT hr = S_OK;

    // Process Loopback용 파라미터 BLOB를 위한 포인터
    AUDIOCLIENT_ACTIVATION_PARAMS* pActivationParamsHeap = nullptr;

    // 1. COM 초기화 (MTA)
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        printf("COM 초기화 실패: 0x%X\n", hr);
        return 1;
    }

    IMMDeviceEnumerator* enumerator = nullptr;
    IMMDevice* device = nullptr;
    LPWSTR deviceIdString = nullptr;
    IActivateAudioInterfaceAsyncOperation* pAsyncOp = nullptr;
    ActivateAudioInterfaceCompletionHandler* pHandler = nullptr;
    IUnknown* pUnknown = nullptr;
    IAudioClient* audioClient = nullptr;

    // 2. 디바이스 열거자
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), NULL,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&enumerator
    );
    if (FAILED(hr)) {
        printf("디바이스 열거자 생성 실패: 0x%X\n", hr);
        CoUninitialize();
        return 1;
    }

    // 3. 기본 렌더 디바이스
    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    if (FAILED(hr)) {
        printf("기본 오디오 디바이스 가져오기 실패: 0x%X\n", hr);
        enumerator->Release();
        CoUninitialize();
        return 1;
    }

    printf("✅ 오디오 디바이스 획득 성공!\n");

    // ========================================
    // Phase 4.1: 디바이스 ID
    // ========================================
    printf("\n=== Phase 4.1: 디바이스 ID 획득 ===\n");

    hr = device->GetId(&deviceIdString);
    if (FAILED(hr)) {
        printf("❌ 디바이스 ID 가져오기 실패: 0x%X\n", hr);
        device->Release();
        enumerator->Release();
        CoUninitialize();
        return 1;
    }

    wprintf(L"디바이스 ID: %s\n", deviceIdString);
    size_t idLength = wcslen(deviceIdString);
    printf("ID 길이: %zu 문자\n", idLength);
    printf("✅ Phase 4.1 완료!\n\n");

    // ========================================
    // Phase 4.3: PID 파라미터 설정
    // ========================================
    printf("\n=== Phase 4.3: PID 기반 캡처 시작 ===\n");

    DWORD targetPid = 21616;  // 테스트용 Chrome PID
    printf("대상 PID: %lu\n", targetPid);
    printf("프로세스: Chrome 브라우저 (확인됨)\n");

    AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS loopbackParams = {};
    loopbackParams.TargetProcessId = targetPid;
    loopbackParams.ProcessLoopbackMode =
        PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE;

    printf("모드: INCLUDE_TARGET_PROCESS_TREE (자식 프로세스 포함)\n");

    AUDIOCLIENT_ACTIVATION_PARAMS activationParams = {};
    activationParams.ActivationType =
        AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK;
    activationParams.ProcessLoopbackParams = loopbackParams;

    printf("활성화 타입: PROCESS_LOOPBACK\n");

    // 🔥 핵심: CoTaskMemAlloc으로 힙에 복사
    pActivationParamsHeap = (AUDIOCLIENT_ACTIVATION_PARAMS*)
        CoTaskMemAlloc(sizeof(AUDIOCLIENT_ACTIVATION_PARAMS));
    if (!pActivationParamsHeap) {
        printf("❌ CoTaskMemAlloc 실패 (AUDIOCLIENT_ACTIVATION_PARAMS)\n");
        CoTaskMemFree(deviceIdString);
        device->Release();
        enumerator->Release();
        CoUninitialize();
        return 1;
    }
    memcpy(pActivationParamsHeap, &activationParams, sizeof(activationParams));

    // PROPVARIANT BLOB 설정
    PROPVARIANT activateParams;
    PropVariantInit(&activateParams);
    activateParams.vt = VT_BLOB;
    activateParams.blob.cbSize = sizeof(AUDIOCLIENT_ACTIVATION_PARAMS);
    activateParams.blob.pBlobData =
        reinterpret_cast<BYTE*>(pActivationParamsHeap);

    printf("PROPVARIANT 래핑 완료 (크기: %lu 바이트)\n",
        activateParams.blob.cbSize);
    printf("✅ Phase 4.3 완료!\n\n");

    // ========================================
    // Phase 4.4: ActivateAudioInterfaceAsync 호출
    // ========================================
    printf("\n=== Phase 4.4: 비동기 오디오 활성화 ===\n");

    // 핸들러 생성
    pHandler = new (std::nothrow) ActivateAudioInterfaceCompletionHandler();
    if (!pHandler) {
        printf("❌ 핸들러 생성 실패!\n");
        if (pActivationParamsHeap) {
            CoTaskMemFree(pActivationParamsHeap);
        }
        CoTaskMemFree(deviceIdString);
        device->Release();
        enumerator->Release();
        CoUninitialize();
        return 1;
    }
    printf("✅ 완료 핸들러 생성됨\n");

    // 비동기 활성화
    hr = ActivateAudioInterfaceAsync(
        deviceIdString,
        __uuidof(IAudioClient),
        &activateParams,
        pHandler,
        &pAsyncOp
    );

    if (FAILED(hr)) {
        printf("❌ ActivateAudioInterfaceAsync 실패: 0x%X\n", hr);
        if (pActivationParamsHeap) {
            CoTaskMemFree(pActivationParamsHeap);
            pActivationParamsHeap = nullptr;
        }
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
        printf("   - Chrome이 실행 중인지, PID %lu가 맞는지 확인\n", targetPid);

        if (pActivationParamsHeap) {
            CoTaskMemFree(pActivationParamsHeap);
            pActivationParamsHeap = nullptr;
        }

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

        if (pActivationParamsHeap) {
            CoTaskMemFree(pActivationParamsHeap);
            pActivationParamsHeap = nullptr;
        }

        pAsyncOp->Release();
        pHandler->Release();
        CoTaskMemFree(deviceIdString);
        device->Release();
        enumerator->Release();
        CoUninitialize();
        return 1;
    }

    printf("완료! ✅\n");

    // 결과 획득
    HRESULT hrActivateResult = E_FAIL;
    hr = pHandler->GetActivateResult(&hrActivateResult, &pUnknown);
    if (FAILED(hr)) {
        printf("❌ GetActivateResult 실패: 0x%X\n", hr);

        if (pActivationParamsHeap) {
            CoTaskMemFree(pActivationParamsHeap);
            pActivationParamsHeap = nullptr;
        }

        pAsyncOp->Release();
        pHandler->Release();
        CoTaskMemFree(deviceIdString);
        device->Release();
        enumerator->Release();
        CoUninitialize();
        return 1;
    }

    if (FAILED(hrActivateResult)) {
        printf("❌ 오디오 활성화 실패: 0x%X\n", hrActivateResult);
        if (hrActivateResult == E_INVALIDARG) {
            printf("   → PID/파라미터 문제 가능성\n");
        }

        if (pUnknown) {
            pUnknown->Release();
            pUnknown = nullptr;
        }

        if (pActivationParamsHeap) {
            CoTaskMemFree(pActivationParamsHeap);
            pActivationParamsHeap = nullptr;
        }

        pAsyncOp->Release();
        pHandler->Release();
        CoTaskMemFree(deviceIdString);
        device->Release();
        enumerator->Release();
        CoUninitialize();
        return 1;
    }

    printf("✅ 활성화 성공!\n");

    // 이제 BLOB 메모리는 더 이상 필요 없음
    if (pActivationParamsHeap) {
        CoTaskMemFree(pActivationParamsHeap);
        pActivationParamsHeap = nullptr;
    }

    // IUnknown → IAudioClient
    hr = pUnknown->QueryInterface(__uuidof(IAudioClient),
        (void**)&audioClient);
    pUnknown->Release();
    pUnknown = nullptr;

    if (FAILED(hr)) {
        printf("❌ IAudioClient 변환 실패: 0x%X\n", hr);
        pAsyncOp->Release();
        pHandler->Release();
        CoTaskMemFree(deviceIdString);
        device->Release();
        enumerator->Release();
        CoUninitialize();
        return 1;
    }

    printf("✅ IAudioClient 획득 성공!\n");
    printf("✅ 이제 PID %lu의 오디오만 캡처 가능합니다! 🎉\n\n", targetPid);
    printf("Phase 4.4 완료!\n");

    // (여기서부터 audioClient->Initialize / Capture 루프 붙이면 진짜 캡처)

    // 정리
    printf("\n정리 중...\n");
    audioClient->Release();
    pAsyncOp->Release();
    pHandler->Release();
    CoTaskMemFree(deviceIdString);
    device->Release();
    enumerator->Release();
    CoUninitialize();
    printf("정리 완료!\n");

    printf("\nEnter 키를 눌러 종료...\n");
    getchar();
    return 0;
}
