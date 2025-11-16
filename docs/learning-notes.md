# C++ 학습 노트 📚

OnVoice COM 브리지 개발 과정에서 배운 C++ 및 Windows 프로그래밍 내용을 기록합니다.

---

## 목차

- [COM 기초](#com-기초)
  - [COM이란 무엇인가?](#com이란-무엇인가)
  - [핵심 개념 5가지](#핵심-개념-5가지)
  - [OnVoice에서 왜 필요한가?](#onvoice에서-왜-필요한가)
  - [실전 예제](#실전-예제)
- [포인터와 참조](#포인터와-참조)
- [WASAPI 오디오 캡처](#wasapi-오디오-캡처)
- [ATL 프로젝트](#atl-프로젝트)
- [빌드 시스템](#빌드-시스템)

---

## COM 기초

### 2025-11-16: COM이란 무엇인가?

**Component Object Model (COM)**: Windows에서 서로 다른 프로그램이나 언어로 작성된 코드를 연결하는 표준 방식

#### 🏠 실생활 비유

COM은 **아파트 관리 규칙**과 같습니다:

```
아파트(Windows) 안에 여러 가구(프로그램)가 살고 있습니다.

❌ 규칙이 없다면:
- 각 가구가 마음대로 복도 사용 → 충돌
- 쓰레기를 아무데나 버림 → 메모리 누수
- 옆집과 소통 방법이 제각각 → 호환성 문제

✅ COM 규칙이 있으면:
- 정해진 방식으로만 소통 (IUnknown 인터페이스)
- 쓰레기는 본인이 치움 (참조 카운팅)
- 모든 가구가 같은 규칙 사용 (표준 인터페이스)
```

#### 🎯 OnVoice에서의 역할

```
C++ DLL (오디오 캡처 엔진)
    ↓ COM 인터페이스
JavaScript (Electron)
```

**문제**: C++과 JavaScript는 완전히 다른 언어

- C++의 포인터를 JavaScript가 이해 못함
- C++의 함수를 JavaScript가 직접 호출 못함

**해결**: COM이 "번역기" 역할

- C++ 함수를 COM 인터페이스로 포장
- JavaScript(winax)가 COM 표준 방식으로 호출
- 데이터도 COM 규격에 맞춰 변환

---

### 핵심 개념 5가지

#### 1️⃣ IUnknown: 모든 COM 객체의 DNA

**비유**: 사람의 혈액형처럼, 모든 COM 객체가 가진 기본 특성

```cpp
// 모든 COM 인터페이스는 IUnknown을 상속받음
interface IUnknown {
    // Q: 당신은 다른 능력도 있나요?
    HRESULT QueryInterface(REFIID riid, void** ppvObject);

    // Q: 당신을 사용하는 사람이 늘어났어요
    ULONG AddRef();

    // Q: 당신을 다 썼어요
    ULONG Release();
};
```

**3가지 메서드 상세**:

##### `QueryInterface()`: "당신은 이것도 할 수 있나요?"

```cpp
// 예시: 오디오 디바이스에게 묻기
IMMDevice* device = NULL;
// "당신은 IAudioClient 기능도 있나요?"
HRESULT hr = device->QueryInterface(
    __uuidof(IAudioClient),  // "IAudioClient를 할 수 있나요?"
    (void**)&audioClient     // "할 수 있으면 여기에 넣어주세요"
);

if (SUCCEEDED(hr)) {
    // 가능합니다! audioClient를 사용하세요
}
```

**실생활 비유**:

```
김원: "택배 기사님, 요리도 할 수 있나요?"
택배 기사: "아니요, 저는 배달만 합니다" (E_NOINTERFACE 반환)

김원: "택배 기사님, 짐 나르는 것도 할 수 있나요?"
택배 기사: "네, 그건 제 본업이죠!" (S_OK 반환, 인터페이스 포인터 제공)
```

##### `AddRef()`: "나 이거 쓸게요!" (참조 카운트 +1)

```cpp
IMMDevice* device1 = NULL;
enumerator->GetDefaultAudioEndpoint(..., &device1);
// 내부적으로 AddRef() 자동 호출 → 참조 카운트 = 1

IMMDevice* device2 = device1;  // 같은 객체를 가리킴
device2->AddRef();  // 명시적 호출 → 참조 카운트 = 2

// 이제 이 디바이스를 2곳에서 사용 중
```

##### `Release()`: "나 다 썼어요!" (참조 카운트 -1)

```cpp
device1->Release();  // 참조 카운트 2 → 1
device2->Release();  // 참조 카운트 1 → 0
// 0이 되면 → 객체 자동 삭제 (메모리 해제)
```

**실생활 비유**:

```
공유 도서 (COM 객체)

김원: 책 빌림 → AddRef() → 참조 카운트 = 1
철수: 같은 책 빌림 → AddRef() → 참조 카운트 = 2

김원: 책 반납 → Release() → 참조 카운트 = 1
철수: 책 반납 → Release() → 참조 카운트 = 0
→ 도서관이 책을 서가로 회수 (메모리 해제)
```

**중요 규칙**:

```
✅ DO: AddRef() 횟수 = Release() 횟수 (반드시!)
❌ DON'T: Release()를 빼먹으면 → 메모리 누수
❌ DON'T: 과도한 Release() → 크래시 (이미 삭제된 객체 접근)
```

---

#### 2️⃣ HRESULT: COM 함수의 "성공/실패 보고서"

**타입**: 32비트 정수 (16진수로 표현)

```cpp
typedef long HRESULT;  // 사실은 정수

// 성공 코드 (최상위 비트 = 0)
#define S_OK           0x00000000  // 완벽한 성공
#define S_FALSE        0x00000001  // 성공했지만 특이 사항 있음

// 실패 코드 (최상위 비트 = 1)
#define E_FAIL         0x80004005  // 일반적인 실패
#define E_POINTER      0x80004003  // NULL 포인터 에러
#define E_NOTIMPL      0x80004001  // 구현되지 않은 기능
#define E_OUTOFMEMORY  0x8007000E  // 메모리 부족
```

**사용 방법**:

```cpp
HRESULT hr;  // 결과를 담을 변수

// 방법 1: SUCCEEDED() 매크로
hr = device->Activate(...);
if (SUCCEEDED(hr)) {
    printf("성공!\n");
} else {
    printf("실패: 0x%X\n", hr);
}

// 방법 2: FAILED() 매크로 (더 일반적)
hr = audioClient->Initialize(...);
if (FAILED(hr)) {
    printf("초기화 실패: 0x%X\n", hr);
    return hr;  // 에러를 호출자에게 전파
}

// 방법 3: 직접 비교 (특정 성공 케이스 체크)
hr = enumerator->GetDevice(id, &device);
if (hr == S_OK) {
    printf("디바이스 찾음!\n");
} else if (hr == E_NOTFOUND) {
    printf("디바이스 없음\n");
}
```

**실생활 비유**:

```
택배 기사가 배달 결과를 보고합니다:

S_OK (0x00000000): "정상 배달 완료! 집 앞에 놓음"
S_FALSE (0x00000001): "배달은 했는데 경비실에 맡김"
E_FAIL (0x80004005): "배달 실패 (이유 불명)"
E_POINTER (0x80004003): "주소가 잘못됨 (NULL)"
E_NOTFOUND (0x80070490): "그 주소에 사람이 안 삼"
```

---

#### 3️⃣ CoInitialize / CoUninitialize: COM 세상의 입장권

**비유**: 놀이공원 입장/퇴장

```cpp
int main() {
    // 🎫 입장권 구매 (COM 초기화)
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        printf("놀이공원 문 닫혔음!\n");
        return 1;
    }

    // 🎢 놀이기구 타기 (COM 객체 사용)
    IMMDeviceEnumerator* enumerator = NULL;
    hr = CoCreateInstance(..., &enumerator);

    // ... 작업 ...

    enumerator->Release();

    // 🚪 퇴장 (COM 해제)
    CoUninitialize();
    return 0;
}
```

**중요 규칙**:

```
✅ CoInitialize()는 프로그램 당 1번 (main 함수 시작 부분)
✅ CoUninitialize()는 프로그램 당 1번 (main 함수 끝 부분)
✅ 중간에 COM 객체를 아무리 많이 만들어도 괜찮음
❌ CoInitialize() 없이 COM 사용 → E_NOTINITIALIZED 에러
```

**멀티스레드 주의**:

```cpp
// 단일 스레드 (STA): 대부분의 경우
CoInitialize(NULL);

// 멀티스레드 (MTA): 서버 프로그램
CoInitializeEx(NULL, COINIT_MULTITHREADED);
```

---

#### 4️⃣ CoCreateInstance(): COM 객체 공장

**비유**: 전자제품 주문

```cpp
IMMDeviceEnumerator* enumerator = NULL;

HRESULT hr = CoCreateInstance(
    __uuidof(MMDeviceEnumerator),      // 주문서: "오디오 디바이스 열거자 주세요"
    NULL,                               // 중간 상인 없음 (직접 구매)
    CLSCTX_ALL,                         // 어디서든 만들어줘 (같은 프로세스 or 다른 프로세스)
    __uuidof(IMMDeviceEnumerator),      // 포장: "IMMDeviceEnumerator 인터페이스로 포장"
    (void**)&enumerator                 // 배송지: enumerator 변수
);

if (SUCCEEDED(hr)) {
    // 제품 도착! 사용 가능
    enumerator->GetDefaultAudioEndpoint(...);

    // 다 쓰면 반납
    enumerator->Release();
}
```

**매개변수 설명**:

| 매개변수       | 의미                     | OnVoice 사용 예                 |
| -------------- | ------------------------ | ------------------------------- |
| `rclsid`       | 만들 객체 ID (CLSID)     | `__uuidof(MMDeviceEnumerator)`  |
| `pUnkOuter`    | 집합 객체 (고급)         | 보통 `NULL`                     |
| `dwClsContext` | 실행 위치                | `CLSCTX_ALL` (어디든)           |
| `riid`         | 받을 인터페이스 ID (IID) | `__uuidof(IMMDeviceEnumerator)` |
| `ppv`          | 결과 받을 포인터         | `(void**)&enumerator`           |

---

#### 5️⃣ 참조 카운팅 (Reference Counting): 자동 메모리 관리

**문제 상황**:

```cpp
// ❌ C++ 일반 포인터의 문제
MyClass* obj = new MyClass();

void FunctionA(MyClass* obj) {
    // 나는 이 객체를 계속 쓸 건데...
}

void FunctionB(MyClass* obj) {
    delete obj;  // 내가 지웠어!
}

FunctionA(obj);
FunctionB(obj);
// FunctionA로 돌아가면? → 크래시! (이미 삭제된 객체 사용)
```

**COM의 해결책**:

```cpp
// ✅ COM 객체의 참조 카운팅
IMMDevice* device = NULL;
enumerator->GetDefaultAudioEndpoint(..., &device);
// 내부적으로: device의 참조 카운트 = 1

void FunctionA(IMMDevice* device) {
    device->AddRef();  // 참조 카운트 = 2 (나도 쓸게!)
    // ... 작업 ...
    device->Release();  // 참조 카운트 = 1 (나는 다 썼어)
}

void FunctionB(IMMDevice* device) {
    device->Release();  // 참조 카운트 = 0 → 자동 삭제
}

FunctionA(device);  // 안전: device는 여전히 유효
FunctionB(device);  // 안전: 마지막 사용자가 삭제
```

**참조 카운트 규칙**:

| 상황             | 해야 할 일                  | 예시                                    |
| ---------------- | --------------------------- | --------------------------------------- |
| 객체를 받았을 때 | 이미 AddRef됨 (추가 호출 X) | `GetDefaultAudioEndpoint(&device)`      |
| 객체를 복사할 때 | `AddRef()` 호출             | `device2 = device1; device2->AddRef();` |
| 다 쓴 후         | `Release()` 호출            | `device->Release();`                    |
| 함수에 전달할 때 | 받는 쪽에서 결정            | 보통 AddRef 안 함                       |

**실생활 비유 (최종판)**:

```
도서관 공유 책 시스템:

1. 김원이 책 대출 → 카운터 = 1
2. 철수도 같은 책 필요 → AddRef() → 카운터 = 2
3. 김원 반납 → Release() → 카운터 = 1
4. 철수 반납 → Release() → 카운터 = 0
   → 사서가 자동으로 책을 서가에 꽂음 (메모리 해제)

규칙:
- 빌릴 때마다 카운터 +1
- 반납할 때마다 카운터 -1
- 카운터가 0이 되면 책 회수
```

---

### OnVoice에서 왜 필요한가?

#### 우리의 아키텍처

```
┌─────────────────────────────────────────┐
│  Electron (JavaScript)                  │
│  - React UI                             │
│  - 사용자 인터랙션                        │
└──────────────┬──────────────────────────┘
               │ winax (COM 클라이언트)
               ↓
┌─────────────────────────────────────────┐
│  C++ COM DLL                            │
│  - WASAPI 오디오 캡처                    │
│  - PID 기반 프로세스 격리                 │
│  - SpeexDSP 리샘플링                     │
└─────────────────────────────────────────┘
```

#### COM이 해결하는 3가지 문제

**1️⃣ 언어 장벽 해소**

```
문제: JavaScript는 C++ 포인터를 이해 못함

해결: COM 인터페이스 = 공통 언어
- C++: "나는 IDispatch 인터페이스를 제공해"
- JavaScript (winax): "나는 IDispatch를 호출할 수 있어"
→ 서로 소통 가능!
```

**2️⃣ 프로세스 간 통신**

```
Electron Main 프로세스 ←→ C++ DLL

COM이 없다면:
- 공유 메모리 수동 관리 (복잡)
- 파이프/소켓 프로토콜 직접 구현 (번거로움)

COM이 있다면:
- new winax.Object('OnVoiceAudioBridge.Capture')
- bridge.Start(processId);
→ 간단!
```

**3️⃣ 메모리 안전성**

```
WASAPI 객체들 (IMMDevice, IAudioClient 등)은 모두 COM 객체
→ 참조 카운팅으로 자동 메모리 관리
→ 메모리 누수 방지
```

---

### 실전 예제

#### 예제 1: 기본 COM 사용 패턴

```cpp
#include <windows.h>
#include <mmdeviceapi.h>
#include <iostream>

int main() {
    // ========== 1단계: COM 초기화 ==========
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        printf("❌ COM 초기화 실패: 0x%X\n", hr);
        return 1;
    }
    printf("✅ COM 초기화 성공\n");

    // ========== 2단계: COM 객체 생성 ==========
    IMMDeviceEnumerator* enumerator = NULL;
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),        // CLSID: 만들 객체
        NULL,                                 // pUnkOuter
        CLSCTX_ALL,                          // 실행 위치
        __uuidof(IMMDeviceEnumerator),       // IID: 받을 인터페이스
        (void**)&enumerator                  // 결과 포인터
    );

    if (FAILED(hr)) {
        printf("❌ 디바이스 열거자 생성 실패: 0x%X\n", hr);
        CoUninitialize();
        return 1;
    }
    printf("✅ 디바이스 열거자 생성 성공\n");

    // ========== 3단계: COM 객체 사용 ==========
    IMMDevice* device = NULL;
    hr = enumerator->GetDefaultAudioEndpoint(
        eRender,   // 스피커/헤드폰
        eConsole,  // 일반 사용
        &device    // 결과 받을 포인터 (내부적으로 AddRef 됨)
    );

    if (FAILED(hr)) {
        printf("❌ 기본 오디오 디바이스 가져오기 실패: 0x%X\n", hr);
        enumerator->Release();  // 정리!
        CoUninitialize();
        return 1;
    }
    printf("✅ 오디오 디바이스 획득 성공\n");

    // ========== 4단계: 정리 (역순!) ==========
    // 규칙: 나중에 만든 것부터 Release
    device->Release();        // device 먼저
    enumerator->Release();    // enumerator 나중에

    // COM 해제
    CoUninitialize();
    printf("✅ 모든 리소스 정리 완료\n");

    return 0;
}
```

**출력 예시**:

```
✅ COM 초기화 성공
✅ 디바이스 열거자 생성 성공
✅ 오디오 디바이스 획득 성공
✅ 모든 리소스 정리 완료
```

---

#### 예제 2: 에러 처리 패턴

```cpp
HRESULT CaptureAudio() {
    HRESULT hr;
    IMMDeviceEnumerator* enumerator = NULL;
    IMMDevice* device = NULL;
    IAudioClient* audioClient = NULL;

    // COM 초기화
    hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        return hr;  // 즉시 반환
    }

    // 디바이스 열거자 생성
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        NULL,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&enumerator
    );
    if (FAILED(hr)) {
        goto cleanup;  // 정리 코드로 점프
    }

    // 기본 디바이스 가져오기
    hr = enumerator->GetDefaultAudioEndpoint(
        eRender,
        eConsole,
        &device
    );
    if (FAILED(hr)) {
        goto cleanup;
    }

    // IAudioClient 인터페이스 가져오기
    hr = device->Activate(
        __uuidof(IAudioClient),
        CLSCTX_ALL,
        NULL,
        (void**)&audioClient
    );
    if (FAILED(hr)) {
        goto cleanup;
    }

    // ... 오디오 캡처 작업 ...

cleanup:
    // 안전한 정리: NULL 체크 후 Release
    if (audioClient != NULL) {
        audioClient->Release();
    }
    if (device != NULL) {
        device->Release();
    }
    if (enumerator != NULL) {
        enumerator->Release();
    }

    CoUninitialize();
    return hr;
}
```

**포인트**:

- `goto cleanup` 패턴: 에러 발생 시 일관된 정리
- NULL 체크: Release 전에 항상 확인
- 역순 정리: 나중에 만든 것부터 해제

---

#### 예제 3: 스마트 포인터 (고급, 나중에 사용)

COM 작업을 더 안전하게:

```cpp
#include <wrl/client.h>  // Windows Runtime Library
using Microsoft::WRL::ComPtr;

HRESULT CaptureAudioSmart() {
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) return hr;

    // ComPtr: 자동으로 AddRef/Release 관리
    ComPtr<IMMDeviceEnumerator> enumerator;
    ComPtr<IMMDevice> device;
    ComPtr<IAudioClient> audioClient;

    // CoCreateInstance
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        IID_PPV_ARGS(&enumerator)  // 타입 안전 매크로
    );
    if (FAILED(hr)) {
        CoUninitialize();
        return hr;
    }

    // GetDefaultAudioEndpoint
    hr = enumerator->GetDefaultAudioEndpoint(
        eRender,
        eConsole,
        &device
    );
    // ... 작업 ...

    // 함수 끝나면 자동으로 Release! (스마트 포인터 소멸자)
    CoUninitialize();
    return hr;
}
```

**장점**:

- `Release()` 호출 불필요 (자동)
- 예외 발생해도 안전 (RAII 패턴)
- 메모리 누수 거의 불가능

**단점**:

- 초보자에겐 복잡 (템플릿 문법)
- Phase 1-8에서는 수동 관리로 학습 추천

---

## 🎓 학습 체크리스트

### COM 기본 개념 이해도 테스트

**자신에게 물어보세요**:

- [ ] COM이 무엇인지 한 문장으로 설명할 수 있나요?
- [ ] IUnknown의 3가지 메서드를 말할 수 있나요?
- [ ] AddRef()와 Release()의 차이를 설명할 수 있나요?
- [ ] HRESULT를 SUCCEEDED() 매크로로 체크할 수 있나요?
- [ ] CoInitialize()를 어디에 써야 하는지 아나요?
- [ ] QueryInterface가 왜 필요한지 이해하나요?
- [ ] 참조 카운팅의 원리를 설명할 수 있나요?

**7개 중 5개 이상 체크** → Phase 1 진행 준비 완료!  
**4개 이하** → 이 문서를 다시 한 번 읽어보세요

---

## 🔖 빠른 참조 카드

### HRESULT 값

| 코드             | 16진수     | 의미            | 대응              |
| ---------------- | ---------- | --------------- | ----------------- |
| S_OK             | 0x00000000 | 성공            | 계속 진행         |
| S_FALSE          | 0x00000001 | 성공 (주의사항) | 조건 확인         |
| E_FAIL           | 0x80004005 | 일반 실패       | 로그 후 종료      |
| E_POINTER        | 0x80004003 | NULL 포인터     | NULL 체크         |
| E_NOTIMPL        | 0x80004001 | 미구현          | 다른 방법 시도    |
| E_OUTOFMEMORY    | 0x8007000E | 메모리 부족     | 리소스 해제       |
| E_NOTINITIALIZED | 0x800401F0 | COM 미초기화    | CoInitialize 호출 |

### 매크로

```cpp
SUCCEEDED(hr)  // hr >= 0 (성공)
FAILED(hr)     // hr < 0 (실패)
```

### COM 객체 생명주기

```cpp
// 1. 생성
CoCreateInstance(..., &ptr);       // 참조 카운트 = 1

// 2. 사용
ptr->SomeMethod();

// 3. 복사 (필요시)
ptr2 = ptr;
ptr2->AddRef();                    // 참조 카운트 = 2

// 4. 정리
ptr->Release();                    // 참조 카운트 = 1
ptr2->Release();                   // 참조 카운트 = 0 → 삭제
```

---

## 포인터와 참조

### 2025-11-16: 포인터 vs 참조

(Phase 2에서 학습 예정)

---

## WASAPI 오디오 캡처

### 2025-11-16: WASAPI란 무엇인가?

**Windows Audio Session API (WASAPI)**: Windows Vista 이후 도입된 저수준 오디오 API

#### 🎵 실생활 비유

WASAPI는 **녹음 스튜디오의 믹서 보드**와 같습니다:

```
일반 앱의 오디오 재생:
사용자 앱 → DirectSound/MME → Windows 오디오 엔진 → 스피커

WASAPI 사용:
사용자 앱 → WASAPI → 오디오 엔진 직접 접근 → 최소 지연, 높은 품질
```

**장점**:

- ✅ **낮은 지연 (Low Latency)**: DirectSound보다 10배 빠름
- ✅ **높은 품질**: 비트-퍼펙트 오디오 (변환 없음)
- ✅ **프로세스별 제어**: 특정 앱만 캡처/제어 가능
- ✅ **Exclusive 모드**: 오디오 디바이스 독점 사용

**OnVoice에서의 역할**:

```
Discord/Chrome/Edge 오디오
    ↓ (WASAPI Loopback Capture)
C++ 캡처 엔진
    ↓ (16kHz PCM)
FastAPI 서버 (Deepgram STT)
```

---

### WASAPI 아키텍처 이해

#### 핵심 컴포넌트 4가지

```
┌─────────────────────────────────────┐
│  1. IMMDeviceEnumerator             │  ← 오디오 디바이스 목록 관리
│     - GetDefaultAudioEndpoint()     │
│     - EnumAudioEndpoints()          │
└──────────────┬──────────────────────┘
               ↓
┌─────────────────────────────────────┐
│  2. IMMDevice                       │  ← 특정 오디오 디바이스
│     - Activate(IAudioClient)        │
│     - GetId(), GetState()           │
└──────────────┬──────────────────────┘
               ↓
┌─────────────────────────────────────┐
│  3. IAudioClient                    │  ← 오디오 세션 관리
│     - Initialize()                  │
│     - Start(), Stop()               │
│     - GetService(IAudioCaptureClient)│
└──────────────┬──────────────────────┘
               ↓
┌─────────────────────────────────────┐
│  4. IAudioCaptureClient             │  ← 실제 오디오 데이터
│     - GetBuffer()                   │
│     - ReleaseBuffer()               │
└─────────────────────────────────────┘
```

---

### 2025-11-16: WASAPI Loopback Capture

**Loopback Capture**: "스피커로 나가는 소리"를 중간에 가로채기

#### 🔊 작동 원리

```
일반 재생 흐름:
앱 (Discord) → 오디오 엔진 → 스피커

Loopback Capture:
앱 (Discord) → 오디오 엔진 → ┬ → 스피커
                             └ → IAudioCaptureClient → 우리 앱
```

**실생활 비유**:

```
전화기 도청 장치처럼, 전화선 중간에 분기를 만들어서
상대방과 내 목소리를 모두 녹음하는 것과 같음.

차이점:
- 일반 마이크: 실제 마이크 입력 캡처
- Loopback: 스피커 출력을 캡처 (앱이 재생하는 소리)
```

---

### Process-Specific Loopback Capture (핵심!)

**일반 Loopback의 문제**:

```
시스템 전체 오디오를 캡처
→ Discord + YouTube + 카카오톡 알림 소리 + 시스템 효과음
→ 모든 소리가 섞여서 들어옴 (OnVoice에선 쓸모없음)
```

**Process-Specific Loopback의 해결**:

```cpp
// Windows 10 1803 (Build 17134) 이상에서 지원
AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS loopbackParams = {};
loopbackParams.TargetProcessId = discordPid;  // Discord만!
loopbackParams.ProcessLoopbackMode = PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE;

AUDIOCLIENT_ACTIVATION_PARAMS activationParams = {};
activationParams.ActivationType = AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK;
activationParams.ProcessLoopbackParams = &loopbackParams;
```

**결과**:

- ✅ Discord 프로세스의 오디오만 캡처
- ✅ Discord의 자식 프로세스도 포함 (웹뷰 등)
- ✅ Chrome, YouTube 소리는 완전히 무시

---

### WASAPI 초기화 흐름 (ProcessLoopbackCapture 기준)

#### 전체 흐름 다이어그램

```
시작
 ↓
┌──────────────────────────────────┐
│ 1. COM 초기화                    │
│    CoInitializeEx(COINIT_MTA)    │
└──────────┬───────────────────────┘
           ↓
┌──────────────────────────────────┐
│ 2. 기본 오디오 디바이스 획득      │
│    IMMDeviceEnumerator           │
│    →GetDefaultAudioEndpoint      │
│    →IMMDevice                    │
└──────────┬───────────────────────┘
           ↓
┌──────────────────────────────────┐
│ 3. IAudioClient 활성화           │
│    ActivateAudioInterfaceAsync   │
│    + PROCESS_LOOPBACK_PARAMS     │
└──────────┬───────────────────────┘
           ↓
┌──────────────────────────────────┐
│ 4. 오디오 포맷 설정              │
│    GetMixFormat() or 사용자 지정  │
│    IAudioClient->Initialize()    │
└──────────┬───────────────────────┘
           ↓
┌──────────────────────────────────┐
│ 5. IAudioCaptureClient 획득      │
│    IAudioClient->GetService()    │
└──────────┬───────────────────────┘
           ↓
┌──────────────────────────────────┐
│ 6. 캡처 루프 시작                │
│    IAudioClient->Start()         │
│    GetBuffer() 반복 호출         │
└──────────────────────────────────┘
```

---

### 단계별 상세 코드 패턴

#### Step 1: COM 초기화

```cpp
// 멀티스레드 환경 (Apartment-Threaded가 아닌 MTA)
HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
if (FAILED(hr)) {
    printf("❌ COM 초기화 실패: 0x%X\n", hr);
    return hr;
}
```

**왜 COINIT_MULTITHREADED?**

- WASAPI 캡처는 **별도 스레드**에서 실행됨
- STA(Single-Threaded Apartment)는 메시지 펌프 필요
- MTA(Multi-Threaded Apartment)는 자유로운 스레드 사용

---

#### Step 2: 기본 오디오 디바이스 획득

```cpp
IMMDeviceEnumerator* pEnumerator = NULL;
IMMDevice* pDevice = NULL;

// 디바이스 열거자 생성
hr = CoCreateInstance(
    __uuidof(MMDeviceEnumerator),
    NULL,
    CLSCTX_ALL,
    __uuidof(IMMDeviceEnumerator),
    (void**)&pEnumerator
);

if (FAILED(hr)) {
    printf("❌ 디바이스 열거자 생성 실패\n");
    return hr;
}

// 기본 렌더링 디바이스 (스피커) 획득
hr = pEnumerator->GetDefaultAudioEndpoint(
    eRender,    // 렌더링 디바이스 (스피커/헤드폰)
    eConsole,   // 일반 용도 (게임/멀티미디어)
    &pDevice    // 결과 받을 포인터
);

if (FAILED(hr)) {
    printf("❌ 기본 오디오 디바이스 획득 실패\n");
    pEnumerator->Release();
    return hr;
}

// 정리
pEnumerator->Release();  // 더 이상 필요 없음
```

**eRender vs eCapture**:

```
eRender: 스피커/헤드폰 (출력 디바이스)
  → Loopback Capture에서는 eRender 사용!
  → "스피커로 나가는 소리"를 캡처하기 때문

eCapture: 마이크 (입력 디바이스)
  → 일반 녹음에서 사용
```

**eConsole vs eMultimedia vs eCommunications**:

```
eConsole: 일반 용도 (게임, 영화)
eMultimedia: 멀티미디어 (거의 eConsole과 동일)
eCommunications: 통신 (스카이프, 줌 등)
  → Discord는 통신 앱이지만 eConsole로도 잡힘
```

---

#### Step 3: IAudioClient 활성화 (Process Loopback)

**핵심 구조체**:

```cpp
// 1. Process Loopback 파라미터
AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS loopbackParams = {};
loopbackParams.TargetProcessId = targetPid;  // Discord/Chrome PID
loopbackParams.ProcessLoopbackMode =
    PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE;
    // ↑ 타깃 프로세스 + 자식 프로세스 포함

// 2. Activation 파라미터
AUDIOCLIENT_ACTIVATION_PARAMS activationParams = {};
activationParams.ActivationType =
    AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK;
activationParams.ProcessLoopbackParams = &loopbackParams;

// 3. PropVariant로 래핑
PROPVARIANT activateParams = {};
activateParams.vt = VT_BLOB;
activateParams.blob.cbSize = sizeof(activationParams);
activateParams.blob.pBlobData = (BYTE*)&activationParams;
```

**비동기 활성화**:

```cpp
// ActivateAudioInterfaceAsync: 비동기 호출!
IActivateAudioInterfaceAsyncOperation* pAsyncOp = NULL;

hr = ActivateAudioInterfaceAsync(
    deviceIdString,          // 디바이스 ID (IMMDevice에서 획득)
    __uuidof(IAudioClient),  // 요청할 인터페이스
    &activateParams,         // Process Loopback 파라미터
    pCompletionHandler,      // 완료 콜백 (IActivateAudioInterfaceCompletionHandler)
    &pAsyncOp                // 비동기 작업 객체
);

if (FAILED(hr)) {
    printf("❌ 오디오 인터페이스 활성화 실패\n");
    return hr;
}

// 완료 대기 (이벤트 사용)
WaitForSingleObject(hEvent, INFINITE);
```

**왜 비동기인가?**

- 오디오 디바이스 초기화는 시간이 걸림 (수백 ms)
- UI 스레드 블로킹 방지
- OnVoice에서는: 캡처 스레드에서 호출하므로 블로킹 허용

---

#### Step 4: 오디오 포맷 설정

```cpp
IAudioClient* pAudioClient = NULL;  // Step 3에서 획득

// 방법 1: 디바이스 기본 포맷 사용
WAVEFORMATEX* pWaveFormat = NULL;
hr = pAudioClient->GetMixFormat(&pWaveFormat);

// 방법 2: 사용자 정의 포맷 (OnVoice 방식)
WAVEFORMATEX waveFormat = {};
waveFormat.wFormatTag = WAVE_FORMAT_PCM;        // 또는 WAVE_FORMAT_IEEE_FLOAT
waveFormat.nChannels = 1;                       // Mono
waveFormat.nSamplesPerSec = 16000;              // 16kHz
waveFormat.wBitsPerSample = 16;                 // 16bit
waveFormat.nBlockAlign = waveFormat.nChannels *
                         waveFormat.wBitsPerSample / 8;  // 2 bytes
waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec *
                             waveFormat.nBlockAlign;     // 32000 bytes/sec
waveFormat.cbSize = 0;                          // 확장 정보 없음

// IAudioClient 초기화
hr = pAudioClient->Initialize(
    AUDCLNT_SHAREMODE_SHARED,        // 공유 모드 (독점 모드 아님)
    AUDCLNT_STREAMFLAGS_LOOPBACK |   // Loopback 플래그 (중요!)
    AUDCLNT_STREAMFLAGS_EVENTCALLBACK, // 이벤트 기반 콜백
    0,                               // 버퍼 지속 시간 (0 = 기본값)
    0,                               // 주기 (공유 모드에서는 0)
    &waveFormat,                     // 요청 포맷
    NULL                             // 오디오 세션 GUID (NULL = 새 세션)
);

if (FAILED(hr)) {
    printf("❌ IAudioClient 초기화 실패: 0x%X\n", hr);
    return hr;
}
```

**AUDCLNT_STREAMFLAGS_LOOPBACK**:

- 가장 중요한 플래그!
- 이게 없으면 일반 캡처 (마이크)
- 이게 있으면 Loopback (스피커 출력 캡처)

**Sample Rate Conversion (SRC)**:

```
앱이 48kHz로 재생 중
    ↓
우리가 16kHz 요청
    ↓
Windows 오디오 엔진이 자동으로 다운샘플링
    ↓
우리에게 16kHz 데이터 전달
```

→ **SpeexDSP 없이도 16kHz 획득 가능!** (MVP에서 활용)

---

#### Step 5: IAudioCaptureClient 획득

```cpp
IAudioCaptureClient* pCaptureClient = NULL;

hr = pAudioClient->GetService(
    __uuidof(IAudioCaptureClient),
    (void**)&pCaptureClient
);

if (FAILED(hr)) {
    printf("❌ IAudioCaptureClient 획득 실패\n");
    return hr;
}

// 이벤트 핸들 생성 및 설정
HANDLE hCaptureEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
pAudioClient->SetEventHandle(hCaptureEvent);
```

---

#### Step 6: 캡처 루프

```cpp
// 캡처 시작
hr = pAudioClient->Start();
if (FAILED(hr)) {
    printf("❌ 캡처 시작 실패\n");
    return hr;
}

printf("✅ 캡처 시작!\n");

// 캡처 루프
while (!stopFlag) {
    // 이벤트 대기 (새 데이터 도착 시 신호)
    DWORD waitResult = WaitForSingleObject(hCaptureEvent, 2000);

    if (waitResult != WAIT_OBJECT_0) {
        printf("⚠️ 타임아웃 또는 에러\n");
        continue;
    }

    // 다음 패킷 크기 확인
    UINT32 packetLength = 0;
    hr = pCaptureClient->GetNextPacketSize(&packetLength);

    while (packetLength > 0) {
        BYTE* pData = NULL;
        UINT32 numFrames = 0;
        DWORD flags = 0;

        // 버퍼 가져오기
        hr = pCaptureClient->GetBuffer(
            &pData,       // 오디오 데이터 포인터
            &numFrames,   // 프레임 개수
            &flags,       // 플래그 (무음 여부 등)
            NULL,         // 디바이스 위치 (선택)
            NULL          // QPCPosition (선택)
        );

        if (FAILED(hr)) {
            printf("❌ GetBuffer 실패\n");
            break;
        }

        // 데이터 처리
        if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {
            // 실제 오디오 데이터
            size_t dataSize = numFrames * waveFormat.nBlockAlign;

            // 콜백 호출 (사용자 정의)
            if (audioCallback) {
                audioCallback(pData, dataSize);
            }
        }

        // 버퍼 해제 (중요!)
        hr = pCaptureClient->ReleaseBuffer(numFrames);

        // 다음 패킷
        hr = pCaptureClient->GetNextPacketSize(&packetLength);
    }
}

// 캡처 정지
pAudioClient->Stop();
printf("✅ 캡처 정지\n");
```

**플래그 이해**:

```cpp
AUDCLNT_BUFFERFLAGS_SILENT (0x2):
  → 무음 구간 (실제 데이터 없음, 0으로 채워진 버퍼)
  → 처리 스킵 가능

AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY (0x1):
  → 데이터 불연속성 (글리치 발생)
  → 로그 남기기
```

---

### ProcessLoopbackCapture 래퍼의 장점

**우리가 직접 구현해야 할 것들**:

```
❌ IActivateAudioInterfaceCompletionHandler 구현 (COM 인터페이스)
❌ 비동기 완료 이벤트 관리
❌ 에러 코드 → 문자열 변환
❌ 참조 카운팅 메모리 관리
❌ 스레드 안전성 보장
```

**ProcessLoopbackCapture가 해주는 것**:

```
✅ 위의 모든 복잡도를 단순한 API로 감춤
✅ SetCaptureFormat(16000, 16, 1, WAVE_FORMAT_PCM)
✅ SetTargetProcess(pid)
✅ SetCallback(myCallback)
✅ StartCapture() / StopCapture()
```

**우리 전략**:

```
1. ProcessLoopbackCapture 코드를 읽으며 WASAPI 패턴 학습
2. OnVoice에 필요한 부분만 추출하여 재구현
3. 불필요한 기능 제거 (Pause/Resume은 나중에)
4. COM DLL 인터페이스에 최적화
```

---

### 주요 에러 및 해결

#### AUDCLNT_E_DEVICE_IN_USE (0x8889000A)

**원인**: 디바이스가 이미 독점 모드로 사용 중

**해결**:

```cpp
// ❌ 독점 모드 (Exclusive)
hr = pAudioClient->Initialize(
    AUDCLNT_SHAREMODE_EXCLUSIVE,  // 독점!
    ...
);

// ✅ 공유 모드 (Shared) - OnVoice는 이걸 써야 함
hr = pAudioClient->Initialize(
    AUDCLNT_SHAREMODE_SHARED,  // 공유
    ...
);
```

---

#### AUDCLNT_E_UNSUPPORTED_FORMAT (0x88890008)

**원인**: 요청한 포맷을 디바이스가 지원 안 함

**해결**:

```cpp
// 방법 1: IsFormatSupported로 먼저 체크
WAVEFORMATEX* pClosestMatch = NULL;
hr = pAudioClient->IsFormatSupported(
    AUDCLNT_SHAREMODE_SHARED,
    &waveFormat,
    &pClosestMatch  // 가장 가까운 포맷 제안
);

if (hr == S_FALSE) {
    printf("요청 포맷 불가, 대안: %d Hz\n",
        pClosestMatch->nSamplesPerSec);
    // pClosestMatch 사용
}

// 방법 2: GetMixFormat 사용
WAVEFORMATEX* pMixFormat = NULL;
hr = pAudioClient->GetMixFormat(&pMixFormat);
// 디바이스 기본 포맷으로 캡처 후 수동 리샘플링
```

---

#### E_INVALIDARG (0x80070057)

**원인**: NULL 포인터 또는 잘못된 파라미터

**체크리스트**:

```cpp
// ✅ 항상 NULL 체크
if (pAudioClient == NULL) {
    printf("❌ pAudioClient가 NULL!\n");
    return E_POINTER;
}

// ✅ 구조체 초기화
AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS params = {};  // = {} 필수!
params.TargetProcessId = pid;

// ✅ PID 유효성 체크
if (pid == 0) {
    printf("❌ 잘못된 PID\n");
    return E_INVALIDARG;
}
```

---

### 성능 최적화 팁

#### 1️⃣ 버퍼 크기 최적화

```cpp
// 작은 버퍼 = 낮은 지연, 높은 CPU
// 큰 버퍼 = 높은 지연, 낮은 CPU

// OnVoice 권장: 10-20ms
REFERENCE_TIME bufferDuration = 100000;  // 10ms (100ns 단위)

hr = pAudioClient->Initialize(
    AUDCLNT_SHAREMODE_SHARED,
    AUDCLNT_STREAMFLAGS_LOOPBACK,
    bufferDuration,  // 버퍼 지속 시간
    0,
    &waveFormat,
    NULL
);
```

#### 2️⃣ 스레드 우선순위

```cpp
// 캡처 스레드 우선순위 상승
DWORD taskIndex = 0;
HANDLE hTask = AvSetMmThreadCharacteristics(TEXT("Audio"), &taskIndex);

if (hTask == NULL) {
    printf("⚠️ 스레드 우선순위 설정 실패 (계속 진행 가능)\n");
}

// ... 캡처 루프 ...

// 정리
if (hTask != NULL) {
    AvRevertMmThreadCharacteristics(hTask);
}
```

#### 3️⃣ Lock-Free Queue (ProcessLoopbackCapture 옵션)

```cpp
// 문제: 콜백에서 파일 쓰기/네트워크 전송 → 글리치
void AudioCallback(BYTE* data, size_t size) {
    // ❌ 시간 걸리는 작업
    SendToServer(data, size);  // 네트워크 IO
    WriteToFile(data, size);   // 디스크 IO
}

// 해결: Lock-Free Queue로 다른 스레드에 넘기기
ReaderWriterQueue<AudioChunk> g_queue;

void AudioCallback(BYTE* data, size_t size) {
    // ✅ 빠른 작업만
    AudioChunk chunk(data, size);
    g_queue.enqueue(chunk);  // Lock-free!
}

void WorkerThread() {
    AudioChunk chunk;
    while (running) {
        if (g_queue.try_dequeue(chunk)) {
            // ✅ 여기서 느린 작업
            SendToServer(chunk.data, chunk.size);
        }
    }
}
```

---

### 2025-11-16: ProcessLoopbackCapture 코드 리뷰

#### 발견한 핵심 패턴 5가지

##### 1️⃣ 비동기 활성화 패턴

**문제**: `ActivateAudioInterfaceAsync`는 비동기 API
**해결**: Completion Handler + Event 대기

```cpp
// Completion Handler 클래스
class ActivateCompletionHandler :
    public IActivateAudioInterfaceCompletionHandler
{
    HANDLE m_hEvent;  // 완료 신호용 이벤트

public:
    HRESULT ActivateCompleted(IActivateAudioInterfaceAsyncOperation* pAsyncOp) {
        // 작업 완료 후 결과 획득
        HRESULT hrActivate = S_OK;
        IUnknown* pUnknown = NULL;

        pAsyncOp->GetActivateResult(&hrActivate, &pUnknown);

        if (SUCCEEDED(hrActivate)) {
            // IAudioClient 인터페이스 얻기
            pUnknown->QueryInterface(__uuidof(IAudioClient), ...);
        }

        // 대기 중인 스레드에 신호
        SetEvent(m_hEvent);

        return S_OK;
    }
};

// 호출 코드
ActivateAudioInterfaceAsync(..., pHandler, ...);
WaitForSingleObject(pHandler->m_hEvent, INFINITE);  // 완료 대기
```

**배운 점**:

- COM 콜백 인터페이스 구현 방법
- 비동기 작업 동기화 패턴 (Event)
- `GetActivateResult()`로 결과 획득

**OnVoice 적용**:

```
COM DLL에서도 같은 패턴 사용
→ Completion Handler는 private 클래스로 감추기
→ StartCapture는 블로킹이므로 별도 스레드 필수
```

---

##### 2️⃣ 캡처 스레드 패턴

**문제**: WASAPI 캡처는 블로킹 작업
**해결**: 별도 스레드 + 상태 관리

```cpp
// Static 스레드 함수
static DWORD WINAPI CaptureThreadProc(LPVOID param) {
    ProcessLoopbackCapture* pThis = (ProcessLoopbackCapture*)param;

    // 1. 우선순위 상승
    HANDLE hTask = AvSetMmThreadCharacteristics(TEXT("Audio"), &taskIndex);

    // 2. 캡처 시작
    pThis->m_pAudioClient->Start();
    SetEvent(pThis->m_hStartEvent);  // 시작 완료 신호

    // 3. 루프
    while (pThis->m_state == CAPTURING) {
        WaitForSingleObject(pThis->m_hCaptureEvent, 2000);
        // GetBuffer / Process / ReleaseBuffer
    }

    // 4. 정리
    pThis->m_pAudioClient->Stop();
    AvRevertMmThreadCharacteristics(hTask);

    return 0;
}

// 스레드 생성
m_hCaptureThread = CreateThread(
    NULL,                    // 기본 보안 속성
    0,                       // 기본 스택 크기
    CaptureThreadProc,       // 스레드 함수
    this,                    // 파라미터 (this 포인터!)
    0,                       // 즉시 실행
    NULL                     // 스레드 ID 불필요
);

// 시작 대기 (블로킹!)
WaitForSingleObject(m_hStartEvent, INFINITE);
```

**배운 점**:

- static 함수에 this 포인터 전달 패턴
- 스레드 우선순위 상승 (Audio 특성)
- 시작 완료 신호 (Event)
- 타임아웃 2000ms로 교착 상태 방지

**OnVoice 적용**:

```
COM StartCapture(pid):
  1. 캡처 스레드 생성
  2. 즉시 S_OK 반환 (비블로킹)
  3. 스레드 내부에서 실제 StartCapture 호출

주의:
  - 스레드 생성/종료는 반드시 같은 COM 스레드에서!
```

---

##### 3️⃣ 에러 처리 패턴

**3가지 에러 처리 레이어**:

```cpp
// Layer 1: 사용자 친화적 enum
enum class eCaptureError {
    NONE,
    ALREADY_CAPTURING,
    NOT_CAPTURING,
    INVALID_FORMAT,
    INVALID_PROCESS_ID,
    ACTIVATION_FAILED,
    INITIALIZATION_FAILED
};

// Layer 2: HRESULT 저장
class ProcessLoopbackCapture {
private:
    HRESULT m_lastHResult;  // 마지막 Windows 에러

public:
    HRESULT GetLastErrorResult() const {
        return m_lastHResult;
    }
};

// Layer 3: 에러 텍스트 변환
const char* GetErrorText(eCaptureError error) {
    switch (error) {
        case NONE: return "Success";
        case ALREADY_CAPTURING: return "Already capturing";
        // ...
    }
}

// 사용 예시
eCaptureError error = capture.StartCapture();
if (error != eCaptureError::NONE) {
    printf("Error: %s\n", GetErrorText(error));
    printf("HRESULT: 0x%X\n", capture.GetLastErrorResult());
}
```

**배운 점**:

- 사용자는 enum만 보면 됨
- 디버깅 시 HRESULT 확인 가능
- 텍스트 변환으로 로깅 편리

**OnVoice 적용**:

```
COM DLL도 3-layer 에러 처리:
  1. COM HRESULT (S_OK, E_FAIL 등)
  2. 내부 에러 코드 (enum)
  3. GetLastError() 메서드로 상세 정보
```

---

##### 4️⃣ 스레드 안전성 패턴

**atomic을 활용한 상태 관리**:

```cpp
#include <atomic>

class ProcessLoopbackCapture {
private:
    std::atomic<eCaptureState> m_state;  // atomic!

public:
    // Thread-safe getter
    eCaptureState GetState() const {
        return m_state.load();  // 원자적 읽기
    }

    // 상태 변경
    void ChangeState(eCaptureState newState) {
        m_state.store(newState);  // 원자적 쓰기
    }
};

// 캡처 스레드에서
while (pThis->m_state == CAPTURING) {  // atomic 읽기
    // ...
}

// 메인 스레드에서
m_state = STOPPING;  // atomic 쓰기 → 스레드가 즉시 감지
```

**배운 점**:

- `std::atomic<T>`로 lock 없이 스레드 안전
- `load()`, `store()` 명시적 호출
- 또는 암묵적 변환 (컴파일러가 처리)

**OnVoice 적용**:

```
COM 객체의 상태 관리:
  - m_state를 atomic으로
  - GetState만 thread-safe 보장
  - Start/Stop은 같은 스레드 제약 명시
```

---

##### 5️⃣ Lock-Free Queue 패턴 (선택적)

**문제**: 오디오 콜백에서 느린 작업 금지
**해결**: Lock-Free Queue + Worker Thread

```cpp
#ifdef PROCESS_LOOPBACK_CAPTURE_USE_QUEUE

#include "readerwriterqueue.h"  // cameron314's queue

class ProcessLoopbackCapture {
private:
    moodycamel::ReaderWriterQueue<AudioChunk> m_queue;
    std::thread m_workerThread;
    std::atomic<bool> m_workerRunning;

    void WorkerThreadProc() {
        AudioChunk chunk;
        while (m_workerRunning) {
            if (m_queue.try_dequeue(chunk)) {
                // 여기서 느린 작업 OK
                if (m_callback) {
                    m_callback(chunk.data.begin(), chunk.data.end(), m_pUserData);
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }
};

// 오디오 스레드
void ProcessAudioBuffer(BYTE* pData, UINT32 numFrames) {
    if (m_useQueue) {
        AudioChunk chunk(pData, numFrames);
        m_queue.enqueue(chunk);  // Lock-free!
        return;
    }

    // 직접 콜백
    m_callback(...);
}

#endif
```

**배운 점**:

- Single-Producer-Single-Consumer Queue
- Lock-free = 뮤텍스 없이 스레드 안전
- `try_dequeue()` = 블로킹 없음

**OnVoice 적용**:

```
Phase 1 MVP: Queue 없이 (단순화)
  - 콜백에서 빠른 작업만 (버퍼 복사)

Phase 2 확장: Queue 추가
  - 콜백 → Queue
  - Worker → Deepgram 전송
  - 지연 증가하지만 안정성 ↑
```

---

#### 메모리 관리 Best Practices

##### ✅ DO: NULL 체크 후 Release

```cpp
if (m_pAudioClient != NULL) {
    m_pAudioClient->Release();
    m_pAudioClient = NULL;  // 중요!
}
```

##### ✅ DO: 역순 정리

```cpp
// 생성 순서: Enumerator → Device → AudioClient → CaptureClient
// 정리 순서: CaptureClient → AudioClient → Device → Enumerator
```

##### ✅ DO: 소멸자에서 자동 정리

```cpp
~ProcessLoopbackCapture() {
    if (m_state == CAPTURING) {
        StopCapture();  // 자동 정리
    }
}
```

##### ❌ DON'T: Release 없이 재할당

```cpp
// ❌ 메모리 누수!
m_pAudioClient = NULL;  // 이전 객체는?

// ✅ 올바른 방법
if (m_pAudioClient) {
    m_pAudioClient->Release();
}
m_pAudioClient = NULL;
```

---

#### OnVoice 재구현 시 체크리스트

**참고할 패턴**:

- [x] 비동기 활성화 + Event 대기
- [x] 캡처 스레드 분리 (static 함수 + this 포인터)
- [x] 3-layer 에러 처리
- [x] atomic 상태 관리
- [ ] Lock-Free Queue (나중에)

**단순화할 부분**:

- [ ] Pause/Resume 제거 (MVP에서 불필요)
- [ ] Queue 제거 (Phase 1에서는)
- [ ] 다중 포맷 지원 제거 (16kHz mono만)

**추가할 부분**:

- [ ] COM 이벤트 콜백 (IConnectionPoint)
- [ ] SAFEARRAY 변환 (BYTE\* → JS Array)
- [ ] PID 유효성 검증

---

### Chrome 프로세스 구조 이해

#### Chrome의 멀티프로세스 아키텍처

**문제**: Chrome은 하나의 앱이지만 여러 프로세스로 실행됨

```
chrome.exe                      ← 브라우저 (메인 프로세스)
chrome.exe --type=gpu-process   ← GPU 프로세스
chrome.exe --type=renderer      ← 렌더러 (각 탭)
chrome.exe --type=utility       ← 유틸리티 (Audio Service 등)
```

**오디오 재생**: 브라우저 프로세스가 관리!

#### 자동 감지 전략

**방법 1: 명령줄 인수 확인** (최선)

```cpp
// --type= 플래그가 없는 chrome.exe = 브라우저 프로세스
if (cmdLine.find(L"--type=") == std::wstring::npos) {
    // 브라우저 프로세스 발견!
}
```

**방법 2: 메모리 기준** (Fallback)

```cpp
// 메모리가 가장 큰 chrome.exe = 브라우저 프로세스 (대부분)
```

**방법 3: Chrome 내부 작업 관리자** (수동)

```
Shift + ESC → 브라우저 프로세스 PID 확인
```

#### 배운 점

- Chrome의 프로세스 격리 (Process Isolation)
- 명령줄 인수로 프로세스 타입 구분
- NtQueryInformationProcess API 사용
- PEB (Process Environment Block) 접근

#### OnVoice 적용

```
사용자가 "Chrome 음성 감지" 선택
  → 자동으로 브라우저 프로세스 PID 찾기
  → WASAPI Process Loopback 시작
  → 사용자는 PID를 몰라도 됨!
```

### OnVoice 적용 계획

#### Phase 3 (T+4-6h): 레퍼런스 학습

```
✅ ProcessLoopbackCapture.h/.cpp 읽기
✅ 위의 WASAPI 패턴 이해
✅ 콘솔 PoC로 PID 기반 캡처 테스트
```

#### Phase 4 (T+6-10h): 재구현

```
OnVoiceAudioCapture 클래스 작성:
  - ActivateAudioClient() 구현
  - InitializeAudioClient() 구현
  - CaptureLoop() 구현
  - 에러 처리 체계화
```

#### Phase 7+ (T+10-40h): COM 통합

```
ATL DLL에서 OnVoiceAudioCapture 래핑:
  - StartCapture(pid) → 스레드 생성
  - OnAudioData 이벤트 → SAFEARRAY 전달
```

---

## 🎓 학습 체크리스트

**WASAPI 이해도 테스트**:

- [ ] WASAPI가 무엇인지 한 문장으로 설명할 수 있나요?
- [ ] Loopback Capture와 일반 Capture의 차이를 아나요?
- [ ] Process-Specific Loopback이 왜 필요한지 설명할 수 있나요?
- [ ] IAudioClient 초기화 흐름을 그릴 수 있나요?
- [ ] GetBuffer/ReleaseBuffer의 역할을 아나요?
- [ ] AUDCLNT_STREAMFLAGS_LOOPBACK 플래그의 의미를 아나요?
- [ ] 16kHz 샘플레이트 변환이 자동으로 되는 이유를 아나요?

**7개 중 5개 이상 체크** → Phase 3 실습 준비 완료!

---

## 🔖 빠른 참조

### 핵심 인터페이스 체인

```
IMMDeviceEnumerator (디바이스 찾기)
  → IMMDevice (특정 디바이스)
    → IAudioClient (오디오 세션)
      → IAudioCaptureClient (데이터 읽기)
```

### 필수 링크 라이브러리

```
프로젝트 속성 → 링커 → 입력 → 추가 종속성:
mmdevapi.lib   (ActivateAudioInterfaceAsync)
avrt.lib       (AvSetMmThreadCharacteristics)
```

### 중요 플래그

| 플래그                            | 값         | 의미          |
| --------------------------------- | ---------- | ------------- |
| AUDCLNT_STREAMFLAGS_LOOPBACK      | 0x00020000 | Loopback 캡처 |
| AUDCLNT_STREAMFLAGS_EVENTCALLBACK | 0x00040000 | 이벤트 기반   |
| AUDCLNT_BUFFERFLAGS_SILENT        | 0x2        | 무음 구간     |

---

**마지막 업데이트**: 2025-11-16  
**다음 학습**: Phase 3 실습 (콘솔 PoC)  
**참고 자료**: ProcessLoopbackCapture 레포, Microsoft WASAPI 문서

---

## ATL 프로젝트

(Phase 7에서 학습 예정)

---

## 빌드 시스템

(필요 시 추가 예정)

---

**마지막 업데이트**: 2025-11-16  
**다음 학습 주제**: 포인터와 참조 (Phase 2)
