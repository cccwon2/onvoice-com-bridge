# C++ 학습 노트 📚

OnVoice COM 브리지 개발 핵심 개념 빠른 참조

**마지막 업데이트**: 2025-11-17 (Day 1 완료)  
**상태**: Week 1 진행 중 (40% 완료) ✅

---

## 🔖 빠른 참조

### COM 필수 체크리스트

```cpp
// 1. COM 초기화 (main 시작)
CoInitializeEx(NULL, COINIT_MULTITHREADED);

// 2. COM 객체 생성
IMMDeviceEnumerator* p = NULL;
CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
                 __uuidof(IMMDeviceEnumerator), (void**)&p);

// 3. 사용
p->SomeMethod();

// 4. 정리 (역순!)
p->Release();
p = NULL;

// 5. COM 해제 (main 끝)
CoUninitialize();
```

### WASAPI 초기화 순서

```
1. IMMDeviceEnumerator (디바이스 목록)
   ↓
2. IMMDevice (특정 디바이스)
   ↓
3. IAudioClient (오디오 세션)
   ↓
4. IAudioCaptureClient (데이터 읽기)
```

### 주요 HRESULT 값

| 코드      | 16진수     | 의미        |
| --------- | ---------- | ----------- |
| S_OK      | 0x00000000 | 성공        |
| E_FAIL    | 0x80004005 | 실패        |
| E_POINTER | 0x80004003 | NULL 포인터 |

### 필수 링크 라이브러리

```
ole32.lib     (COM 기본)
oleaut32.lib  (COM 자동화)
mmdevapi.lib  (WASAPI)
avrt.lib      (스레드 우선순위)
psapi.lib     (프로세스 정보)
```

---

## 💡 COM 핵심 3가지

### 1. IUnknown - 모든 COM 객체의 기본

**3가지 필수 메서드**:

```cpp
QueryInterface()  // "이 기능 있나요?"
AddRef()          // "나 이거 쓸게요" (참조 +1)
Release()         // "다 썼어요" (참조 -1, 0이면 삭제)
```

**황금 규칙**:

```
✅ AddRef 횟수 = Release 횟수
❌ Release 빼먹으면 → 메모리 누수
❌ 과도한 Release → 크래시
```

### 2. 참조 카운팅 - 자동 메모리 관리

**비유**: 도서관 공유 책

```
김원 대출 → 카운트 = 1
철수 대출 → 카운트 = 2
김원 반납 → 카운트 = 1
철수 반납 → 카운트 = 0 → 책 서가로 회수
```

**코드**:

```cpp
// 객체 받을 때: 이미 AddRef됨
enumerator->GetDevice(..., &device);  // 카운트 = 1

// 다 쓰면 Release
device->Release();  // 카운트 = 0 → 자동 삭제
```

### 3. HRESULT - 에러 처리

**체크 방법**:

```cpp
HRESULT hr = someFunction();

// 방법 1: SUCCEEDED / FAILED 매크로 (권장)
if (FAILED(hr)) {
    printf("실패: 0x%X\n", hr);
    return hr;
}

// 방법 2: 직접 비교
if (hr == S_OK) {
    // 성공
}
```

**[더 보기: COM 상세](details/com-deep-dive.md)**

---

## 🎵 WASAPI 핵심 3가지

### 1. Loopback Capture (스피커 출력 캡처)

**핵심**: 시스템 오디오 출력을 캡처

```cpp
// 루프백 모드 초기화
hr = audioClient->Initialize(
    AUDCLNT_SHAREMODE_SHARED,        // 공유 모드
    AUDCLNT_STREAMFLAGS_LOOPBACK,    // 루프백 플래그 ⭐
    10000000,                         // 1초 버퍼
    0,
    waveFormat,
    nullptr
);
```

**결과**:
- ✅ 스피커로 출력되는 모든 소리 캡처
- ✅ 실시간 캡처 (10-20ms 지연)

### 2. Process-Specific Loopback (Day 2 예정)

**핵심**: 특정 앱의 오디오만 캡처

```cpp
AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS params = {};
params.TargetProcessId = discordPid;  // Discord만!
params.ProcessLoopbackMode = PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE;
```

**결과**:
- ✅ Discord 소리만 캡처
- ❌ Windows 알림, 다른 앱 무시

### 3. 캡처 루프 패턴

```cpp
pAudioClient->Start();

while (!stopped) {
    Sleep(10);  // 10ms 대기

    UINT32 packetLength = 0;
    pCaptureClient->GetNextPacketSize(&packetLength);

    while (packetLength > 0) {
        pCaptureClient->GetBuffer(&data, &numFrames, &flags, ...);

        // 데이터 처리
        if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {
            ProcessAudio(data, numFrames);
        }

        pCaptureClient->ReleaseBuffer(numFrames);
        pCaptureClient->GetNextPacketSize(&packetLength);
    }
}

pAudioClient->Stop();
```

**[더 보기: WASAPI 상세](details/wasapi-deep-dive.md)**

---

## 🆕 Day 1 학습 내용 (2025-11-17)

### Phase 1: Visual Studio 2026 + ATL 설정

**완료한 것**:
- ✅ Visual Studio Community 2026 설치
- ✅ ATL 구성 요소 설치 (v145 빌드 도구)
- ✅ HelloCOM ATL 프로젝트 생성 및 빌드 성공
- ✅ COM DLL 생성 확인 (HelloCOM.dll)

**배운 것**:
- ATL 프로젝트 템플릿 사용법
- "사용자 단위 리디렉션" 설정 (regsvr32 권한 문제 해결)
- In-Process DLL vs Out-of-Process EXE

**주요 설정**:
```
워크로드: C++를 사용한 데스크톱 개발
구성 요소: C++ ATL for latest v145 build tools
링커 설정: ole32.lib;oleaut32.lib;uuid.lib
```

---

### Phase 2: C++ 기초 학습

#### 2.1 포인터와 참조 (CppBasics 프로젝트)

**핵심 개념**:

```cpp
// 포인터: 메모리 주소를 담는 변수
int age = 42;
int* ptr = &age;      // ptr은 age의 주소를 가짐
cout << *ptr;         // 42 출력 (* 연산자로 값 접근)

// 참조: 변수의 별명
int& ref = age;       // ref는 age의 별명
ref = 100;            // age도 100으로 변경됨

// 이중 포인터 (COM에서 자주 사용)
int** ppValue = &ptr; // 포인터의 포인터
**ppValue = 200;      // age가 200으로 변경
```

**연산자 정리**:
| 표기 | 의미 | 예시 |
|------|------|------|
| `int* ptr` | 포인터 선언 | `IMMDevice* device` |
| `&변수` | 변수의 주소 | `&age` |
| `*ptr` | 포인터가 가리키는 값 | `*ptr = 100` |
| `ptr->Method()` | 포인터로 메서드 호출 | `device->Release()` |
| `nullptr` | NULL 포인터 (C++11) | `int* ptr = nullptr` |

**중요한 규칙**:
- ✅ NULL 포인터 사용 전 반드시 체크: `if (ptr != nullptr)`
- ❌ NULL 포인터에 `*` 연산자 사용 → 크래시!

---

#### 2.2 COM 기본 개념 (COMBasics 프로젝트)

**실습 내용**:
```cpp
// 1. COM 초기화
CoInitialize(nullptr);

// 2. 디바이스 열거자 생성
IMMDeviceEnumerator* enumerator = nullptr;
CoCreateInstance(__uuidof(MMDeviceEnumerator), ...);

// 3. 기본 오디오 디바이스 가져오기
IMMDevice* device = nullptr;
enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);

// 4. 디바이스 속성 읽기
IPropertyStore* props = nullptr;
device->OpenPropertyStore(STGM_READ, &props);
props->GetValue(PKEY_Device_FriendlyName, &varName);

// 5. 정리 (역순!)
props->Release();
device->Release();
enumerator->Release();
CoUninitialize();
```

**검증 결과**:
```
✅ COM 초기화 성공
✅ 디바이스 열거자 생성 성공
✅ 기본 오디오 디바이스 획득!
✅ 정리 완료! (메모리 누수 없음)
```

---

### Phase 3.1: WASAPI 기본 오디오 캡처 (AudioCapture 프로젝트)

**완료한 기능**:
- ✅ 시스템 오디오 루프백 캡처
- ✅ 실시간 패킷 스트리밍
- ✅ 오디오 형식 정보 추출
- ✅ 5초간 연속 캡처 성공

**캡처 결과**:
```
오디오 형식:
- 샘플링 레이트: 48000 Hz
- 채널 수: 2 (스테레오)
- 비트 깊이: 32 bits (Float)

캡처 성과:
- 총 패킷 수: 500개
- 총 프레임 수: 240,000 (48kHz × 5초)
- 실제 데이터: DB 6A 81 BC... (0이 아님 ✅)
```

**핵심 코드 패턴**:

```cpp
// 1. 오디오 클라이언트 활성화
IAudioClient* audioClient = nullptr;
device->Activate(__uuidof(IAudioClient), ...);

// 2. 믹스 형식 가져오기
WAVEFORMATEX* waveFormat = nullptr;
audioClient->GetMixFormat(&waveFormat);

// 3. 루프백 모드로 초기화
audioClient->Initialize(
    AUDCLNT_SHAREMODE_SHARED,
    AUDCLNT_STREAMFLAGS_LOOPBACK,  // ⭐ 중요!
    10000000,  // 1초 버퍼
    0,
    waveFormat,
    nullptr
);

// 4. 캡처 클라이언트 획득
IAudioCaptureClient* captureClient = nullptr;
audioClient->GetService(__uuidof(IAudioCaptureClient), ...);

// 5. 캡처 시작
audioClient->Start();

// 6. 데이터 읽기 루프
while (!stopped) {
    Sleep(10);
    
    UINT32 packetLength = 0;
    captureClient->GetNextPacketSize(&packetLength);
    
    while (packetLength > 0) {
        BYTE* pData = nullptr;
        UINT32 numFrames = 0;
        DWORD flags = 0;
        
        captureClient->GetBuffer(&pData, &numFrames, &flags, ...);
        
        // 데이터 처리
        if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {
            ProcessAudio(pData, numFrames);
        }
        
        captureClient->ReleaseBuffer(numFrames);
        captureClient->GetNextPacketSize(&packetLength);
    }
}

// 7. 정리
audioClient->Stop();
captureClient->Release();
CoTaskMemFree(waveFormat);
audioClient->Release();
```

**중요한 발견**:
1. ✅ **루프백 모드 작동**: 스피커 출력 실시간 캡처 성공
2. ✅ **패킷 기반 스트리밍**: 10ms마다 폴링, 낮은 지연
3. ✅ **메모리 관리**: CoTaskMemFree로 waveFormat 해제 필수
4. ✅ **에러 없음**: 모든 COM 객체 정상 Release

---

## 📊 Day 1 진행 상황

### 시간 추적

| Phase | 계획 | 실제 | 상태 |
|-------|------|------|------|
| Phase 1 (VS 설정) | 2h | 1h | ✅ 완료 |
| Phase 2 (C++ 기초) | 2h | 1.5h | ✅ 완료 |
| Phase 3.1 (기본 캡처) | 2h | 1.5h | ✅ 완료 |
| **합계** | **6h** | **4h** | **-2h** 절감! |

### 완료한 프로젝트

1. **HelloCOM** - ATL DLL 템플릿 (빌드 성공)
2. **CppBasics** - 포인터/참조 실습 (실행 성공)
3. **COMBasics** - COM 디바이스 정보 (실행 성공)
4. **AudioCapture** - WASAPI 루프백 캡처 (실행 성공) ⭐

### 학습 성과

**C++ 개념**:
- ✅ 포인터와 참조의 차이
- ✅ 이중 포인터 (`void**`)
- ✅ `nullptr` vs `NULL`
- ✅ `->` vs `.` 연산자

**COM 개념**:
- ✅ `IUnknown` 인터페이스
- ✅ `AddRef()` / `Release()` 참조 카운팅
- ✅ `CoInitialize()` / `CoUninitialize()`
- ✅ `CoCreateInstance()` 객체 생성
- ✅ `HRESULT` 에러 처리

**WASAPI 개념**:
- ✅ `IMMDeviceEnumerator` (디바이스 목록)
- ✅ `IMMDevice` (디바이스 객체)
- ✅ `IAudioClient` (오디오 세션)
- ✅ `IAudioCaptureClient` (캡처 인터페이스)
- ✅ `AUDCLNT_STREAMFLAGS_LOOPBACK` (루프백 모드)
- ✅ 패킷 기반 스트리밍 패턴

---

## 🎯 다음 단계 (Day 2)

### Phase 4: PID 기반 캡처 (가장 중요!)

**목표**: Discord/Chrome만 선택적으로 캡처

**핵심 API**:
```cpp
AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS params = {};
params.TargetProcessId = chromePid;  // Chrome만!
params.ProcessLoopbackMode = PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE;

ActivateAudioInterfaceAsync(
    deviceId,
    __uuidof(IAudioClient),
    &params,
    ...
);
```

**예상 소요**: 2-3시간  
**난이도**: ⭐⭐⭐ 어려움

---

## 🎉 Week 0-1 완료 요약

### Week 0 (이미 완료)
- ✅ COM 기초 개념 학습
- ✅ WASAPI 아키텍처 이해
- ✅ ProcessLoopbackCapture 코드 분석
- ✅ **콘솔 PoC 완성** (PID 기반 캡처)

### Week 1 (40% 완료)
- ✅ Visual Studio 2026 + ATL 설정
- ✅ C++ 포인터/참조 학습
- ✅ COM 실습 (디바이스 정보)
- ✅ WASAPI 기본 캡처 성공
- ⏳ PID 기반 캡처 (다음)
- ⏳ 리소스 누수 수정 (다음)
- ⏳ ATL COM DLL 프로젝트 (다음)

### 시간 효율

| 항목 | 계획 | 실제 | 차이 |
|------|------|------|------|
| Week 0 | 14h | 6h | **-8h** ✨ |
| Week 1 (Day 1) | 6h | 4h | **-2h** ✨ |
| **누적 절감** | 20h | 10h | **-10h** 🎉 |
| **남은 시간** | 50h | 40h | - |

---

## 📚 상세 문서

### COM 심화
- **[COM Deep Dive](details/com-deep-dive.md)**: IUnknown, CoCreateInstance, 스마트 포인터 등

### WASAPI 심화
- **[WASAPI Deep Dive](details/wasapi-deep-dive.md)**: 6단계 초기화, 에러 처리, 성능 최적화

### PoC 학습
- **[PoC Lessons Learned](details/poc-lessons.md)**: ProcessLoopbackCapture 패턴 5가지, 메모리 관리

---

## 🔧 트러블슈팅

### 자주 발생하는 에러

**1. `regsvr32` 권한 에러 (코드: 5)**
```
해결: 프로젝트 속성 → 링커 → 일반 → 사용자 단위 리디렉션 → "예(/user)"
```

**2. `unresolved external symbol __imp_CoCreateInstance`**
```
해결: 링커 → 입력 → 추가 종속성에 ole32.lib 추가
```

**3. `fatal error C1083: 포함 파일을 열 수 없습니다: 'atlbase.h'`**
```
해결: Visual Studio Installer → 수정 → ATL 구성 요소 설치
```

**4. NULL 포인터 크래시**
```cpp
// ❌ 잘못된 코드
IMMDevice* device = nullptr;
device->Activate(...);  // 크래시!

// ✅ 올바른 코드
if (device != nullptr) {
    device->Activate(...);
}
```

---

## 💡 핵심 학습 포인트

### 포인터 사용 규칙
```cpp
// 1. 항상 nullptr로 초기화
IMMDevice* device = nullptr;

// 2. 사용 전 NULL 체크
if (device != nullptr) {
    device->Activate(...);
}

// 3. 사용 후 Release
device->Release();
device = nullptr;  // 이중 해제 방지
```

### COM 객체 수명 관리
```cpp
// 생성 (AddRef 자동 호출, count = 1)
enumerator->GetDevice(&device);

// 복사 (AddRef 수동 호출, count = 2)
IMMDevice* copy = device;
copy->AddRef();

// 정리 (Release 호출)
copy->Release();     // count = 1
device->Release();   // count = 0 → 삭제
```

### WASAPI 버퍼 관리
```cpp
// 1. 버퍼 획득
BYTE* pData = nullptr;
captureClient->GetBuffer(&pData, ...);

// 2. 데이터 처리
memcpy(buffer, pData, size);

// 3. 버퍼 반드시 해제!
captureClient->ReleaseBuffer(numFrames);
```

---

**문서 체계**:

```
learning-notes.md          ← 지금 여기 (빠른 참조 + Day 1 학습)
├── details/
│   ├── com-deep-dive.md   (COM 상세)
│   ├── wasapi-deep-dive.md (WASAPI 상세)
│   └── poc-lessons.md     (PoC 학습)
├── build-errors.md        (에러 해결)
└── phase-progress.md      (진행 상황)
```

---

**다음 학습 목표**: Phase 4 (PID 기반 캡처) - Discord/Chrome만 선택적 캡처 🎯