# COM Deep Dive 🏗️

COM (Component Object Model) 상세 가이드

---

## 목차

- [IUnknown 3대 메서드](#iunknown-3대-메서드)
- [CoInitialize / CoUninitialize](#coinitialize--couninitialize)
- [CoCreateInstance](#cocreateinstance)
- [참조 카운팅 상세](#참조-카운팅-상세)
- [실전 예제](#실전-예제)
- [스마트 포인터](#스마트-포인터)

---

## IUnknown 3대 메서드

### QueryInterface() - "이 기능 있나요?"

```cpp
HRESULT QueryInterface(REFIID riid, void** ppvObject);
```

**실생활 비유**:

```
김원: "택배 기사님, 요리도 할 수 있나요?"
택배: "아니요, 저는 배달만 합니다" (E_NOINTERFACE)

김원: "택배 기사님, 짐 나르는 것도 할 수 있나요?"
택배: "네, 제 본업이죠!" (S_OK, 인터페이스 제공)
```

**예제**:

```cpp
IMMDevice* device = NULL;
IAudioClient* audioClient = NULL;

// "device야, 너 IAudioClient 기능도 있어?"
HRESULT hr = device->QueryInterface(
    __uuidof(IAudioClient),
    (void**)&audioClient
);

if (SUCCEEDED(hr)) {
    // 있습니다! audioClient 사용 가능
    audioClient->Initialize(...);
    audioClient->Release();
}
```

### AddRef() - "나 이거 쓸게요!"

```cpp
ULONG AddRef();
```

**언제 호출하나?**:

```cpp
IMMDevice* device1 = NULL;
enumerator->GetDevice(..., &device1);  // 내부에서 AddRef → 카운트 = 1

// 다른 곳에서도 쓰려면
IMMDevice* device2 = device1;
device2->AddRef();  // 명시적 호출 → 카운트 = 2
```

### Release() - "다 썼어요!"

```cpp
ULONG Release();
```

**반드시 호출**:

```cpp
device1->Release();  // 카운트 = 1
device2->Release();  // 카운트 = 0 → 객체 삭제
```

---

## CoInitialize / CoUninitialize

### 기본 사용

```cpp
int main() {
    // COM 초기화 (제일 먼저!)
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        return 1;
    }

    // ... COM 작업 ...

    // COM 해제 (제일 마지막!)
    CoUninitialize();
    return 0;
}
```

### 멀티스레드

```cpp
// 단일 스레드 (STA)
CoInitialize(NULL);

// 멀티스레드 (MTA) - WASAPI 캡처에 사용
CoInitializeEx(NULL, COINIT_MULTITHREADED);
```

**차이점**:

- STA: 메시지 펌프 필요, UI 스레드
- MTA: 자유로운 스레드, 백그라운드 작업

---

## CoCreateInstance

### 기본 사용

```cpp
IMMDeviceEnumerator* enumerator = NULL;

HRESULT hr = CoCreateInstance(
    __uuidof(MMDeviceEnumerator),      // CLSID: 만들 객체
    NULL,                               // Aggregation (보통 NULL)
    CLSCTX_ALL,                         // 실행 위치
    __uuidof(IMMDeviceEnumerator),      // IID: 원하는 인터페이스
    (void**)&enumerator                 // 결과 포인터
);
```

### 매개변수 설명

| 매개변수     | 설명          | 예시                            |
| ------------ | ------------- | ------------------------------- |
| rclsid       | 클래스 ID     | `__uuidof(MMDeviceEnumerator)`  |
| pUnkOuter    | 집합 객체     | 보통 `NULL`                     |
| dwClsContext | 실행 컨텍스트 | `CLSCTX_ALL` (어디든)           |
| riid         | 인터페이스 ID | `__uuidof(IMMDeviceEnumerator)` |
| ppv          | 결과 포인터   | `(void**)&enumerator`           |

---

## 참조 카운팅 상세

### 규칙

| 상황        | 행동             | 예시                  |
| ----------- | ---------------- | --------------------- |
| 객체를 받음 | 이미 AddRef됨    | `GetDevice(&p)`       |
| 객체를 복사 | `AddRef()` 호출  | `p2=p1; p2->AddRef()` |
| 다 쓴 후    | `Release()` 호출 | `p->Release()`        |
| 함수에 전달 | 받는 쪽이 결정   | 보통 AddRef 안 함     |

### 예제: 안전한 복사

```cpp
void SafeCopy(IMMDevice** ppDest, IMMDevice* pSrc) {
    if (*ppDest != NULL) {
        (*ppDest)->Release();  // 기존 객체 해제
    }

    *ppDest = pSrc;

    if (*ppDest != NULL) {
        (*ppDest)->AddRef();   // 새 객체 참조 증가
    }
}
```

---

## 실전 예제

### 예제 1: 기본 패턴

```cpp
#include <windows.h>
#include <mmdeviceapi.h>

int main() {
    // 1. COM 초기화
    CoInitialize(NULL);

    // 2. 객체 생성
    IMMDeviceEnumerator* enumerator = NULL;
    CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
                     __uuidof(IMMDeviceEnumerator), (void**)&enumerator);

    // 3. 사용
    IMMDevice* device = NULL;
    enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);

    // 4. 정리 (역순!)
    device->Release();
    enumerator->Release();

    // 5. COM 해제
    CoUninitialize();

    return 0;
}
```

### 예제 2: 에러 처리

```cpp
HRESULT InitializeAudio() {
    HRESULT hr;
    IMMDeviceEnumerator* enumerator = NULL;
    IMMDevice* device = NULL;

    hr = CoInitialize(NULL);
    if (FAILED(hr)) return hr;

    hr = CoCreateInstance(..., &enumerator);
    if (FAILED(hr)) goto cleanup;

    hr = enumerator->GetDefaultAudioEndpoint(..., &device);
    if (FAILED(hr)) goto cleanup;

cleanup:
    if (device) device->Release();
    if (enumerator) enumerator->Release();
    CoUninitialize();

    return hr;
}
```

---

## 스마트 포인터

### ComPtr (고급)

```cpp
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

void SmartExample() {
    CoInitialize(NULL);

    // 자동 AddRef/Release
    ComPtr<IMMDeviceEnumerator> enumerator;
    ComPtr<IMMDevice> device;

    CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                     CLSCTX_ALL, IID_PPV_ARGS(&enumerator));

    enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);

    // 함수 끝나면 자동 Release!
    CoUninitialize();
}
```

**장점**:

- 자동 메모리 관리
- 예외 안전
- 메모리 누수 방지

**단점**:

- 초보자에겐 복잡
- Phase 1-8에서는 수동 관리 권장

---

**[← 돌아가기](../learning-notes.md)**
