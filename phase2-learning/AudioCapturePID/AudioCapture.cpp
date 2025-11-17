#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <iostream>
#include <mfapi.h>

#pragma comment(lib, "mmdevapi.lib")
#pragma comment(lib, "avrt.lib")
#pragma comment(lib, "mfplat.lib")

// ========================================
// ✅ 수동 타입 정의 (SDK 호환성 보장)
// ========================================

// 이미 정의되어 있는지 확인
#ifndef AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS

// ProcessLoopbackMode enum 정의
typedef enum ProcessLoopbackMode {
    PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE = 0,
    PROCESS_LOOPBACK_MODE_EXCLUDE_TARGET_PROCESS_TREE = 1
} ProcessLoopbackMode;

// AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS 구조체 정의
typedef struct AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS {
    DWORD TargetProcessId;                      // 캡처할 프로세스 PID
    ProcessLoopbackMode ProcessLoopbackMode;    // 캡처 모드
} AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS;

// AUDIOCLIENT_ACTIVATION_TYPE enum 정의
typedef enum AUDIOCLIENT_ACTIVATION_TYPE {
    AUDIOCLIENT_ACTIVATION_TYPE_DEFAULT = 0,
    AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK = 1
} AUDIOCLIENT_ACTIVATION_TYPE;

// AUDIOCLIENT_ACTIVATION_PARAMS 구조체 정의
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
    public IActivateAudioInterfaceCompletionHandler  // COM 인터페이스 상속
{
private:
    // COM 참조 카운팅 (중요!)
    LONG m_refCount;

    // 완료 이벤트 (대기용)
    HANDLE m_hEvent;

    // 활성화 결과 저장
    HRESULT m_hrActivateResult;

    // IAudioClient 객체 (IUnknown으로 받음)
    IUnknown* m_pUnknown;

public:
    // 생성자: 멤버 변수 초기화
    ActivateAudioInterfaceCompletionHandler() :
        m_refCount(1),                      // 참조 카운트 1로 시작
        m_hEvent(CreateEvent(NULL, FALSE, FALSE, NULL)),  // 이벤트 생성
        m_hrActivateResult(E_FAIL),         // 기본값: 실패
        m_pUnknown(nullptr)                 // NULL로 초기화
    {
        printf("[핸들러] 생성됨 (참조 카운트: %d, 이벤트: %p)\n",
            m_refCount, m_hEvent);
    }

    // 소멸자: 리소스 정리
    virtual ~ActivateAudioInterfaceCompletionHandler() {
        printf("[핸들러] 소멸 시작 (참조 카운트: %d)\n", m_refCount);

        // 이벤트 핸들 정리
        if (m_hEvent) {
            CloseHandle(m_hEvent);
            m_hEvent = NULL;
        }

        // IAudioClient 정리
        if (m_pUnknown) {
            m_pUnknown->Release();
            m_pUnknown = NULL;
        }

        printf("[핸들러] 소멸 완료\n");
    }

    // ===== IUnknown 메서드 (필수!) =====

    // 1️. QueryInterface: "이 인터페이스 지원하나요?"
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) {
        printf("[핸들러] QueryInterface 호출됨\n");

        // NULL 체크
        if (ppvObject == nullptr) {
            return E_POINTER;
        }

        // 지원하는 인터페이스 확인
        if (riid == __uuidof(IUnknown) ||
            riid == __uuidof(IActivateAudioInterfaceCompletionHandler)) {

            // 지원함! 자기 자신 반환
            *ppvObject = static_cast<IActivateAudioInterfaceCompletionHandler*>(this);
            AddRef();  // 참조 카운트 증가
            return S_OK;
        }

        // 지원 안 함
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    // 2️. AddRef: 참조 카운트 증가
    STDMETHODIMP_(ULONG) AddRef() {
        LONG count = InterlockedIncrement(&m_refCount);
        printf("[핸들러] AddRef: %d\n", count);
        return count;
    }

    // 3️. Release: 참조 카운트 감소 (0이면 삭제!)
    STDMETHODIMP_(ULONG) Release() {
        LONG count = InterlockedDecrement(&m_refCount);
        printf("[핸들러] Release: %d\n", count);

        if (count == 0) {
            printf("[핸들러] 참조 카운트 0 → 삭제!\n");
            delete this;  // 자기 자신 삭제
            return 0;
        }

        return count;
    }

    // ===== IActivateAudioInterfaceCompletionHandler 메서드 =====

    // 4️. ActivateCompleted: 비동기 작업 완료 콜백 (핵심!)
    STDMETHODIMP ActivateCompleted(IActivateAudioInterfaceAsyncOperation* operation) {
        printf("[핸들러] ActivateCompleted 호출됨! 🎉\n");

        // 1️. 활성화 결과 획득
        HRESULT hrActivateResult;
        IUnknown* pUnknown = nullptr;

        HRESULT hr = operation->GetActivateResult(&hrActivateResult, &pUnknown);
        if (FAILED(hr)) {
            printf("[핸들러] ❌ GetActivateResult 실패: 0x%X\n", hr);
            m_hrActivateResult = hr;
            SetEvent(m_hEvent);  // 실패해도 이벤트 신호
            return hr;
        }

        // 2️. 활성화 자체는 성공했는가?
        if (FAILED(hrActivateResult)) {
            printf("[핸들러] ❌ 오디오 활성화 실패: 0x%X\n", hrActivateResult);
            m_hrActivateResult = hrActivateResult;
            SetEvent(m_hEvent);
            return hrActivateResult;
        }

        // 3️. 성공! IAudioClient 저장
        m_hrActivateResult = hrActivateResult;
        m_pUnknown = pUnknown;  // 소유권 이전 (나중에 Release)

        printf("[핸들러] ✅ 활성화 성공! IAudioClient 획득\n");

        // 4️. 완료 신호 (대기 중인 메인 스레드 깨우기)
        SetEvent(m_hEvent);

        return S_OK;
    }

    // ===== 헬퍼 메서드 =====

    // 이벤트 핸들 반환 (WaitForSingleObject용)
    HANDLE GetEvent() const {
        return m_hEvent;
    }

    // 활성화 결과 반환 (main에서 호출)
    HRESULT GetActivateResult(HRESULT* phr, IUnknown** ppUnknown) {
        if (phr == nullptr || ppUnknown == nullptr) {
            return E_POINTER;
        }

        *phr = m_hrActivateResult;
        *ppUnknown = m_pUnknown;

        // IUnknown 복사 시 AddRef 필요!
        if (m_pUnknown) {
            m_pUnknown->AddRef();
        }

        return S_OK;
    }
};

// ========================================
// Phase 4.2 종료
// ========================================

int main() {
    // 1. COM 초기화
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        printf("COM 초기화 실패: 0x%X\n", hr);
        return 1;
    }

    // 2. 디바이스 열거자 생성
    IMMDeviceEnumerator* enumerator = NULL;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
        CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&enumerator);

    if (FAILED(hr)) {
        printf("디바이스 열거자 생성 실패: 0x%X\n", hr);
        CoUninitialize();
        return 1;
    }

    // 3. 기본 오디오 디바이스 가져오기
    IMMDevice* device = NULL;
    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    if (FAILED(hr)) {
        printf("기본 오디오 디바이스 가져오기 실패: 0x%X\n", hr);
        enumerator->Release();
        CoUninitialize();
        return 1;
    }

    printf("✅ 오디오 디바이스 획득 성공!\n");  // ✅ 한 번만!

    // ========================================
    // Phase 4.1: 디바이스 ID 문자열 가져오기
    // ========================================
    printf("\n=== Phase 4.1: 디바이스 ID 획득 ===\n");

    LPWSTR deviceIdString = NULL;
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

    // 1️⃣ 대상 PID 설정 (하드코딩)
    // ⚠️ TODO: 위에서 확인한 실제 PID로 변경!
    DWORD targetPid = 4488;  // ← 여기에 확인한 Chrome PID 입력!

    printf("대상 PID: %d\n", targetPid);
    printf("프로세스: Chrome 브라우저 (추정)\n");

    // 2️⃣ Process Loopback 파라미터 설정
    AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS loopbackParams = {};
    loopbackParams.TargetProcessId = targetPid;
    loopbackParams.ProcessLoopbackMode =
        PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE;

    printf("모드: INCLUDE_TARGET_PROCESS_TREE (자식 프로세스 포함)\n");

    // 3️⃣ Activation 파라미터 설정
    AUDIOCLIENT_ACTIVATION_PARAMS activationParams = {};
    activationParams.ActivationType =
        AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK;
    activationParams.ProcessLoopbackParams = loopbackParams;

    printf("활성화 타입: PROCESS_LOOPBACK\n");

    // 4️⃣ PROPVARIANT로 래핑
    PROPVARIANT activateParams;
    PropVariantInit(&activateParams);
    activateParams.vt = VT_BLOB;
    activateParams.blob.cbSize = sizeof(activationParams);
    activateParams.blob.pBlobData = (BYTE*)&activationParams;

    printf("PROPVARIANT 래핑 완료 (크기: %d 바이트)\n",
        activateParams.blob.cbSize);

    printf("✅ Phase 4.3 완료!\n\n");

    // ========================================
    // 임시 정리 (Phase 4.3 테스트용)
    // ========================================
    CoTaskMemFree(deviceIdString);
    device->Release();
    enumerator->Release();
    CoUninitialize();

    // ✅ 여기서 대기!
    printf("\nEnter 키를 눌러 종료...\n");
    getchar();  // ← 사용자가 Enter 누를 때까지 대기

    return 0;
}