# Phase별 진행 상황 📊

60시간 스프린트의 각 Phase별 상세 진행 기록

---

## ⏱️ 전체 타임라인 (수정됨)

| 주차   | 시간 범위 | 주요 마일스톤    | 상태       |
| ------ | --------- | ---------------- | ---------- |
| Week 0 | T+0-6h    | **PoC 완성** ⭐  | ✅ 완료    |
| Week 1 | T+6-20h   | COM 브리지 기초  | 🔄 진행 중 (40%) |
| Week 2 | T+20-40h  | Electron 연동    | 📋 계획    |
| Week 3 | T+40-50h  | 테스트 및 안정화 | 📋 계획    |

**중요 변경사항**:

- ✨ ProcessLoopbackCapture 레포 발견으로 **PoC 단계 추가**
- ⏰ Phase 3-6 (WASAPI 직접 구현) → **PoC로 대체**
- 📉 예상 개발 시간: 60시간 → **50시간** (10시간 단축!)

---

## Week 0: PoC 완성 및 검증 ✅

### Phase 0: 프로젝트 초기화 (완료)

- **시간**: T+0h
- **난이도**: ⭐ 쉬움
- **목표**: GitHub 레포 생성, 문서 구조 세팅
- **완료 날짜**: 2025-11-16
- **실제 소요 시간**: 0.5시간

**체크포인트**:

- [x] GitHub 레포 생성 (onvoice-com-bridge)
- [x] .gitignore 설정 (VisualStudio + 커스텀)
- [x] README.md 작성
- [x] docs 폴더 구조 생성
- [x] learning-notes.md 템플릿
- [x] build-errors.md 템플릿
- [x] phase-progress.md 템플릿
- [x] 첫 커밋 완료

**학습 내용**:

- Git 워크플로우 설계
- 프로젝트 문서화 전략
- 학습 노트 시스템 구축

---

### Phase 0.5: COM 기초 개념 학습 (완료)

- **시간**: T+0.5-2h
- **난이도**: ⭐⭐ 보통
- **목표**: COM 핵심 개념 이해
- **완료 날짜**: 2025-11-16
- **실제 소요 시간**: 1.5시간

**체크포인트**:

- [x] COM이란 무엇인가 학습
- [x] IUnknown 3가지 메서드 이해
- [x] HRESULT 에러 코드 체계
- [x] CoInitialize/CoUninitialize
- [x] 참조 카운팅 (AddRef/Release)
- [x] learning-notes.md에 상세 정리

**학습 내용**:

```markdown
핵심 개념 5가지:

1. IUnknown (QueryInterface, AddRef, Release)
2. HRESULT (에러 코드 및 체크 매크로)
3. CoInitialize/CoUninitialize (COM 초기화)
4. CoCreateInstance (COM 객체 생성)
5. 참조 카운팅 (메모리 관리)

실생활 비유:

- COM = 아파트 관리 규칙
- IUnknown = 모든 COM 객체의 DNA
- 참조 카운팅 = 도서관 공유 책 시스템
```

**배운 점**:

- ✅ COM은 언어 간 통신 표준
- ✅ 참조 카운팅으로 자동 메모리 관리
- ✅ OnVoice에서 C++ DLL ↔ Electron 연결에 필수

---

### Phase 0.6: WASAPI 개념 학습 (완료)

- **시간**: T+2-3h
- **난이도**: ⭐⭐ 보통
- **목표**: WASAPI 아키텍처 및 Loopback Capture 이해
- **완료 날짜**: 2025-11-16
- **실제 소요 시간**: 1시간

**체크포인트**:

- [x] WASAPI 아키텍처 이해
- [x] Loopback Capture vs 일반 Capture
- [x] Process-Specific Loopback 원리
- [x] WASAPI 초기화 6단계 흐름
- [x] 캡처 루프 패턴 (GetBuffer/ReleaseBuffer)
- [x] learning-notes.md에 상세 정리

**학습 내용**:

```markdown
핵심 컴포넌트 4가지:

1. IMMDeviceEnumerator (디바이스 목록)
2. IMMDevice (특정 디바이스)
3. IAudioClient (오디오 세션)
4. IAudioCaptureClient (실제 데이터)

Process-Specific Loopback:

- 일반 Loopback: 시스템 전체 오디오
- Process Loopback: 특정 앱만 (Discord/Chrome)
- 구조체: AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS
```

**배운 점**:

- ✅ WASAPI = 저수준 오디오 API (낮은 지연)
- ✅ Loopback = 스피커 출력 캡처
- ✅ PID 기반으로 앱별 오디오 격리 가능
- ✅ 16kHz 변환이 Windows 엔진에서 자동 처리됨 (SpeexDSP 불필요!)

---

### Phase 0.7: ProcessLoopbackCapture 코드 리뷰 (완료)

- **시간**: T+3-4h
- **난이도**: ⭐⭐⭐ 어려움
- **목표**: 레퍼런스 코드 분석 및 패턴 학습
- **완료 날짜**: 2025-11-16
- **실제 소요 시간**: 1시간

**체크포인트**:

- [x] ProcessLoopbackCapture.h 분석
- [x] 에러 코드 enum 이해
- [x] 콜백 함수 시그니처 파악
- [x] 클래스 멤버 변수 분석
- [x] Public/Private API 메서드 파악
- [x] ProcessLoopbackCapture.cpp 핵심 로직 분석
- [x] StartCapture() 구현 이해
- [x] ActivateAudioInterface() 비동기 패턴 학습
- [x] CaptureThreadProc() 캡처 루프 분석
- [x] learning-notes.md에 패턴 정리

**발견한 핵심 패턴 5가지**:

**1. 비동기 활성화 패턴**

```cpp
// Completion Handler 클래스
class ActivateCompletionHandler :
    public IActivateAudioInterfaceCompletionHandler
{
    HRESULT ActivateCompleted(...) {
        // 작업 완료 후 이벤트 신호
        SetEvent(m_hEvent);
    }
};

// 사용
ActivateAudioInterfaceAsync(..., pHandler, ...);
WaitForSingleObject(pHandler->m_hEvent, INFINITE);
```

**2. 캡처 스레드 패턴**

```cpp
// Static 함수 + this 포인터 전달
static DWORD WINAPI CaptureThreadProc(LPVOID param) {
    ProcessLoopbackCapture* pThis = (ProcessLoopbackCapture*)param;

    // 우선순위 상승
    AvSetMmThreadCharacteristics(TEXT("Audio"), ...);

    // 캡처 루프
    while (pThis->m_state == CAPTURING) {
        // GetBuffer / Process / ReleaseBuffer
    }
}
```

**3. 3-Layer 에러 처리**

```
Layer 1: 사용자 친화적 enum (eCaptureError)
Layer 2: HRESULT 저장 (m_lastHResult)
Layer 3: 에러 텍스트 변환 (GetErrorText)
```

**4. 스레드 안전성**

```cpp
std::atomic<eCaptureState> m_state;  // atomic!

// Thread-safe getter
eCaptureState GetState() const {
    return m_state.load();
}
```

**5. Lock-Free Queue (선택적)**

```cpp
moodycamel::ReaderWriterQueue<AudioChunk> m_queue;

// 오디오 스레드
m_queue.enqueue(chunk);  // Lock-free!

// Worker 스레드
if (m_queue.try_dequeue(chunk)) {
    // 느린 작업 (네트워크 전송 등)
}
```

**배운 점**:

- ✅ 비동기 COM 작업 동기화 패턴
- ✅ 오디오 스레드 우선순위 관리
- ✅ 메모리 누수 방지 (NULL 체크 + Release)
- ✅ OnVoice 재구현 청사진 확보

---

### Phase 0.8: 콘솔 PoC 개발 (완료) ⭐

- **시간**: T+4-6h
- **난이도**: ⭐⭐ 보통
- **목표**: PID 기반 오디오 캡처 실제 검증
- **완료 날짜**: 2025-11-16
- **실제 소요 시간**: 2시간

**체크포인트**:

- [x] Visual Studio 콘솔 프로젝트 생성
- [x] ProcessLoopbackCapture 파일 추가
- [x] 링커 라이브러리 설정 (mmdevapi.lib, avrt.lib)
- [x] PID 입력 및 캡처 테스트
- [x] 16kHz mono PCM 캡처 검증
- [x] 실시간 볼륨 표시 (RMS 계산)
- [x] WAV 파일 저장 기능
- [x] Chrome 프로세스 자동 감지 구현
- [x] Discord 프로세스 자동 감지 구현

**핵심 코드**:

```cpp
// 16kHz, 16bit, Mono, PCM 설정
capture.SetCaptureFormat(16000, 16, 1, WAVE_FORMAT_PCM);

// Chrome 브라우저 프로세스 자동 찾기
DWORD chromePid = FindChromeBrowserProcess();

// 캡처 시작
capture.StartCapture();

// 콜백에서 실시간 볼륨 계산
double rms = sqrt(sum / numSamples);
int volumeBars = (int)(rms * 50);
printf("Volume: ");
for (int i = 0; i < volumeBars; i++) printf("█");
```

**Chrome 프로세스 자동 감지**:

```cpp
// --type= 플래그가 없는 chrome.exe = 브라우저 프로세스
std::wstring cmdLine = GetProcessCommandLine(pid);
if (cmdLine.find(L"--type=") == std::wstring::npos) {
    return pid;  // 브라우저 프로세스!
}

// Fallback: 메모리 가장 큰 chrome.exe
```

**테스트 결과**:

```
✅ Chrome 브라우저 프로세스 자동 감지 성공
✅ Discord 프로세스 자동 감지 성공
✅ 16kHz mono 캡처 검증 완료
✅ 타 앱 소리 격리 확인 (Discord만 캡처, Windows 알림 무시)
✅ WAV 파일 재생 확인 (Windows Media Player)
✅ 총 캡처 데이터: ~0.32 MB (10초)

볼륨 표시 예시:
[  0.15 MB] Volume: ███████████████████
[  0.31 MB] Volume: ████████████████████████
```

**발견한 문제와 해결**:

**문제 1**: Chrome 멀티프로세스 구조

```
chrome.exe (브라우저)              → 오디오 재생 ✅
chrome.exe --type=gpu-process      → 오디오 없음
chrome.exe --type=renderer         → 오디오 없음
chrome.exe --type=utility          → 오디오 없음
```

**해결**: 명령줄 인수 분석으로 브라우저 프로세스 자동 감지

**문제 2**: 수동 PID 입력 불편

```
사용자: "PID가 뭐예요?"
개발자: "Shift+ESC 눌러서..."
```

**해결**: 자동 프로세스 감지 구현

**배운 점**:

- ✅ **PID 기반 캡처가 실제로 작동함** (가장 중요!)
- ✅ 16kHz 변환이 자동으로 됨 (SpeexDSP 불필요)
- ✅ ProcessLoopbackCapture API 사용법 체득
- ✅ Chrome 멀티프로세스 아키텍처 이해
- ✅ 프로세스 자동 감지 전략 확립

---

## Week 1: COM 브리지 기초 (진행 중)

### Phase 1: Visual Studio 2026 + ATL 설정 (완료) ✅

- **시간**: T+6-8h
- **난이도**: ⭐ 쉬움
- **목표**: ATL 프로젝트 템플릿으로 "Hello COM" 빌드
- **완료 날짜**: 2025-11-17 (Day 1)
- **실제 소요 시간**: 1시간

**체크포인트**:

- [x] Visual Studio 2026 Community 설치
- [x] Windows 11 SDK 확인
- [x] ATL 지원 설치 (v145 빌드 도구)
- [x] ATL 프로젝트 "HelloCOM" 생성
- [x] 빈 COM DLL 빌드 성공 (HelloCOM.dll)
- [x] regsvr32 권한 문제 해결 (사용자 단위 리디렉션)

**워크로드 설정**:
```
✅ C++를 사용한 데스크톱 개발
✅ MSVC v145 - VS 2026 C++ x64/x86 빌드 도구
✅ Windows 11 SDK (10.0.26100.0)
✅ C++ ATL for latest v145 build tools
```

**프로젝트 속성 설정**:
```
링커 → 일반 → 사용자 단위 리디렉션: 예(/user)
링커 → 입력 → 추가 종속성: ole32.lib;oleaut32.lib;uuid.lib
```

**빌드 결과**:
```
========== 빌드: 성공 1, 실패 0 ==========
출력: x64\Debug\HelloCOM.dll (52KB)
```

**학습 내용**:

1. **ATL 프로젝트 구조**:
   - DLL (In-Process) vs EXE (Out-of-Process)
   - "병합된 프록시/스텁" 옵션의 의미
   - "특성 사용됨" (Attributed) 코드 생성

2. **Visual Studio 2026 차이점**:
   - 빌드 도구 버전: v145 (2022는 v143)
   - 기본 플랫폼: x64 우선
   - Windows 11 SDK 기본 사용

3. **regsvr32 권한 문제**:
   - 에러 코드 5: 관리자 권한 필요
   - 해결: "사용자 단위 리디렉션" 활성화
   - MVP 개발 중에는 시스템 등록 불필요

**배운 점**:

- ✅ ATL 마법사 사용법
- ✅ COM DLL 프로젝트 생성 및 빌드
- ✅ Visual Studio 2026 새 기능
- ✅ 링커 설정의 중요성

---

### Phase 2: C++ 기초 학습 (완료) ✅

- **시간**: T+8-10h
- **난이도**: ⭐⭐ 보통
- **목표**: 포인터, 참조, COM 기본 개념 실습
- **완료 날짜**: 2025-11-17 (Day 1)
- **실제 소요 시간**: 1.5시간

**체크포인트**:

- [x] **CppBasics 프로젝트**: 포인터와 참조 실습
- [x] **COMBasics 프로젝트**: COM 기본 실습
- [x] 포인터 (`int*`) vs 참조 (`int&`) 차이 이해
- [x] 이중 포인터 (`int**`) 개념
- [x] `nullptr` 사용법
- [x] COM 초기화 및 디바이스 정보 가져오기
- [x] learning-notes.md 업데이트

#### 2.1 포인터와 참조 실습 (CppBasics)

**핵심 개념 정리**:

```cpp
// 1. 포인터: 메모리 주소를 담는 변수
int age = 42;
int* ptr = &age;      // ptr은 age의 주소
cout << *ptr;         // 42 출력 (* 연산자로 값 접근)

// 2. 참조: 변수의 별명
int& ref = age;       // ref는 age의 별명
ref = 100;            // age도 100으로 변경

// 3. 이중 포인터 (COM에서 자주 사용)
int** ppValue = &ptr; // 포인터의 포인터
**ppValue = 200;      // age가 200으로 변경

// 4. nullptr (C++11)
int* nullPtr = nullptr;
if (nullPtr == nullptr) {
    // nullPtr이 NULL인지 체크
}
```

**연산자 정리**:

| 표기 | 의미 | 예시 |
|------|------|------|
| `int* ptr` | 포인터 선언 | `IMMDevice* device` |
| `&변수` | 변수의 주소 | `&age` |
| `*ptr` | 포인터가 가리키는 값 | `*ptr = 100` |
| `ptr->Method()` | 포인터로 메서드 호출 | `device->Release()` |
| `nullptr` | NULL 포인터 | `int* ptr = nullptr` |

**중요한 규칙**:
- ✅ NULL 포인터 사용 전 반드시 체크: `if (ptr != nullptr)`
- ❌ NULL 포인터에 `*` 연산자 사용 → 크래시!

#### 2.2 COM 기본 개념 (COMBasics)

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

// 4. 디바이스 속성 읽기 (시도)
IPropertyStore* props = nullptr;
device->OpenPropertyStore(STGM_READ, &props);

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

**배운 점**:

- ✅ 포인터와 참조의 실제 사용법
- ✅ `->` 연산자 (포인터 멤버 접근)
- ✅ COM 객체 생명주기 관리
- ✅ `CoInitialize` → 작업 → `CoUninitialize` 패턴
- ✅ `Release()` 호출의 중요성

---

### Phase 3.1: WASAPI 기본 오디오 캡처 (완료) ✅

- **시간**: T+10-14h (일부)
- **난이도**: ⭐⭐ 보통
- **목표**: 시스템 오디오 루프백 캡처 성공
- **완료 날짜**: 2025-11-17 (Day 1)
- **실제 소요 시간**: 1.5시간

**체크포인트**:

- [x] **AudioCapture 프로젝트** 생성
- [x] 링커 라이브러리 설정 (ole32.lib, oleaut32.lib, avrt.lib)
- [x] 오디오 클라이언트 활성화
- [x] 믹스 형식 가져오기 (48kHz, 2ch, 32bit)
- [x] 루프백 모드로 초기화
- [x] 캡처 클라이언트 획득
- [x] 5초간 오디오 데이터 캡처
- [x] 실시간 패킷 수신 확인

**핵심 코드 구조**:

```cpp
// 1. 오디오 클라이언트 활성화
IAudioClient* audioClient = nullptr;
device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&audioClient);

// 2. 믹스 형식 가져오기
WAVEFORMATEX* waveFormat = nullptr;
audioClient->GetMixFormat(&waveFormat);

// 3. 루프백 모드로 초기화
audioClient->Initialize(
    AUDCLNT_SHAREMODE_SHARED,
    AUDCLNT_STREAMFLAGS_LOOPBACK,  // ⭐ 루프백 플래그!
    10000000,  // 1초 버퍼
    0,
    waveFormat,
    nullptr
);

// 4. 캡처 클라이언트 획득
IAudioCaptureClient* captureClient = nullptr;
audioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&captureClient);

// 5. 캡처 시작
audioClient->Start();

// 6. 데이터 읽기 루프
while (capturing) {
    Sleep(10);  // 10ms 대기
    
    UINT32 packetLength = 0;
    captureClient->GetNextPacketSize(&packetLength);
    
    while (packetLength > 0) {
        BYTE* pData = nullptr;
        UINT32 numFrames = 0;
        DWORD flags = 0;
        
        captureClient->GetBuffer(&pData, &numFrames, &flags, nullptr, nullptr);
        
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
CoTaskMemFree(waveFormat);  // ⭐ CoTaskMemFree 필수!
audioClient->Release();
```

**캡처 결과**:

```
오디오 형식:
- 샘플링 레이트: 48000 Hz
- 채널 수: 2 (스테레오)
- 비트 깊이: 32 bits (Float)

캡처 성과:
- 총 패킷 수: 500개
- 총 프레임 수: 240,000 (48kHz × 5초)
- 첫 10바이트: DB 6A 81 BC DB 6A 81 BC 4C 9D (실제 오디오 데이터!)

검증:
✅ 루프백 모드 작동 (스피커 출력 캡처)
✅ 실시간 패킷 수신 (10ms 폴링)
✅ 메모리 정리 완료 (CoTaskMemFree)
✅ 크래시 없음
```

**중요한 발견**:

1. **루프백 모드 성공**:
   - YouTube, Spotify 등 스피커로 출력되는 모든 소리 캡처
   - 마이크 입력 없이도 오디오 캡처 가능

2. **패킷 기반 스트리밍**:
   - 10ms마다 폴링으로 낮은 지연
   - `GetNextPacketSize()` → `GetBuffer()` → `ReleaseBuffer()` 패턴

3. **메모리 관리**:
   - `waveFormat`은 `CoTaskMemFree()` 필수!
   - `malloc`/`free`가 아님에 주의

4. **에러 없이 완료**:
   - 모든 COM 객체 정상 Release
   - 5초간 안정적 캡처

**배운 점**:

- ✅ WASAPI 루프백 캡처 전체 플로우
- ✅ `AUDCLNT_STREAMFLAGS_LOOPBACK` 플래그 중요성
- ✅ 패킷 기반 스트리밍 패턴
- ✅ `CoTaskMemFree()` 사용법
- ✅ 실시간 오디오 처리 기초

---

### Phase 3.2: PID 기반 캡처 (예정) ⏳

- **시간**: T+14-18h
- **난이도**: ⭐⭐⭐ 어려움
- **목표**: Chrome/Discord만 선택적 캡처
- **예정 날짜**: 2025-11-18 (Day 2)
- **예상 소요 시간**: 2-3시간

**체크포인트**:

- [ ] `AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS` 구조체 설정
- [ ] `ActivateAudioInterfaceAsync` API 사용
- [ ] 비동기 완료 핸들러 구현
- [ ] Chrome PID로 YouTube만 캡처
- [ ] Discord PID로 Discord만 캡처
- [ ] 타 앱 소리 격리 검증 (Spotify, Windows 알림)

**핵심 코드 (미리보기)**:

```cpp
// 1. PID 기반 캡처 파라미터 설정
AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS params = {};
params.TargetProcessId = chromePid;  // Chrome만!
params.ProcessLoopbackMode = PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE;

// 2. 비동기 활성화
ActivateAudioInterfaceAsync(
    deviceId,
    __uuidof(IAudioClient),
    (PROPVARIANT*)&params,
    pCompletionHandler,
    &pAsyncOp
);

// 3. 완료 대기
WaitForSingleObject(hEvent, INFINITE);

// 4. IAudioClient 획득
pCompletionHandler->GetActivateResult(&hr, &pAudioClientUnk);
pAudioClientUnk->QueryInterface(__uuidof(IAudioClient), (void**)&pAudioClient);
```

**예상 과제**:

1. **비동기 패턴 이해**:
   - `IActivateAudioInterfaceCompletionHandler` 구현
   - 이벤트 동기화 (`CreateEvent`, `WaitForSingleObject`)

2. **디바이스 ID 가져오기**:
   - `IMMDevice::GetId()` 사용
   - Wide string 처리

3. **에러 처리**:
   - 비동기 작업 실패 시 처리
   - PID 유효성 검증

---

### Phase 7: ATL COM 프로젝트 생성 (예정)

- **시간**: T+18-22h
- **난이도**: ⭐⭐⭐ 어려움
- **목표**: OnVoice COM DLL 뼈대 작성
- **예정 날짜**: 2025-11-19-20 (Day 3-4)
- **예상 소요 시간**: 4시간

**체크포인트**:

- [ ] ATL 프로젝트 "OnVoiceAudioBridge" 생성
- [ ] IDL 파일에 인터페이스 정의
- [ ] `IOnVoiceCapture` 인터페이스
- [ ] `StartCapture(LONG pid)` 메서드
- [ ] `StopCapture()` 메서드
- [ ] `GetState()` 메서드
- [ ] VBScript로 COM 호출 테스트

**예상 코드 구조**:

```cpp
// IDL 파일
[
    uuid(12345678-1234-1234-1234-123456789012),
    version(1.0),
]
library OnVoiceAudioBridgeLib
{
    [
        uuid(87654321-4321-4321-4321-210987654321),
        dual,
        oleautomation
    ]
    interface IOnVoiceCapture : IDispatch
    {
        [id(1)] HRESULT StartCapture([in] LONG pid);
        [id(2)] HRESULT StopCapture();
        [id(3)] HRESULT GetState([out, retval] LONG* pState);
    };
    
    [
        uuid(ABCDEF12-3456-7890-ABCD-EF1234567890),
    ]
    coclass OnVoiceCapture
    {
        [default] interface IOnVoiceCapture;
    };
};
```

---

### Phase 8: OnVoice 캡처 엔진 통합 (예정)

- **시간**: T+22-28h
- **난이도**: ⭐⭐⭐⭐ 매우 어려움
- **목표**: ProcessLoopbackCapture 패턴 적용
- **예정 날짜**: 2025-11-20-21 (Day 4-5)
- **예상 소요 시간**: 6시간

**체크포인트**:

- [ ] Phase 3.2 코드를 COM DLL로 이식
- [ ] 캡처 스레드 생성 및 관리
- [ ] 에러 처리 체계화
- [ ] `FindChromeBrowserProcess()` 통합
- [ ] VBScript로 Discord 오디오 캡처 테스트
- [ ] 메모리 누수 검사

---

## 📈 시간 트래킹

### Week 0 (완료)

| Phase     | 계획 시간 | 실제 시간 | 차이      | 비고               |
| --------- | --------- | --------- | --------- | ------------------ |
| Phase 0   | 0.5h      | 0.5h      | -         | 프로젝트 초기화    |
| Phase 0.5 | -         | 1.5h      | +1.5h     | COM 기초 (추가)    |
| Phase 0.6 | -         | 1h        | +1h       | WASAPI 학습 (추가) |
| Phase 0.7 | -         | 1h        | +1h       | 코드 리뷰 (추가)   |
| Phase 0.8 | -         | 2h        | +2h       | PoC 개발 (추가)    |
| **합계**  | **0.5h**  | **6h**    | **+5.5h** | **PoC 완성** ✅    |

**Week 0 총 투입 시간**: 6시간  
**남은 예상 시간**: 44시간

---

### Week 1 - Day 1 (완료) ✅

| Phase | 계획 시간 | 실제 시간 | 차이 | 비고 |
|-------|----------|----------|------|------|
| Phase 1 (VS 설정) | 2h | 1h | **-1h** ✨ | ATL 환경 구축 |
| Phase 2 (C++ 기초) | 2h | 1.5h | **-0.5h** ✨ | 포인터/COM 학습 |
| Phase 3.1 (기본 캡처) | 2h | 1.5h | **-0.5h** ✨ | 루프백 캡처 성공 |
| **합계** | **6h** | **4h** | **-2h** 🎉 | **초과 달성!** |

**Day 1 총 투입 시간**: 4시간  
**남은 예상 시간**: 40시간

---

### 누적 시간 (Week 0 + Day 1)

| 항목 | 계획 | 실제 | 차이 |
|------|------|------|------|
| Week 0 | 14h (원래 Phase 3-6) | 6h | **-8h** ✨ |
| Week 1 Day 1 | 6h | 4h | **-2h** ✨ |
| **누적 절감** | 20h | 10h | **-10h** 🎉 |
| **남은 예산** | 50h | 40h | - |

---

## 🎓 Day 1 핵심 학습 성과

### 완성한 프로젝트 4개

1. **HelloCOM** - ATL DLL 템플릿 이해 ✅
2. **CppBasics** - 포인터/참조 실습 ✅
3. **COMBasics** - COM 디바이스 정보 ✅
4. **AudioCapture** - WASAPI 루프백 캡처 ⭐ ✅

### 기술적 이해

**C++ 개념**:
- ✅ 포인터 (`int*`) vs 참조 (`int&`)
- ✅ 이중 포인터 (`int**`)
- ✅ `nullptr` 사용법
- ✅ `->` vs `.` 연산자

**COM 개념**:
- ✅ `CoInitialize()` / `CoUninitialize()`
- ✅ `CoCreateInstance()` 객체 생성
- ✅ `AddRef()` / `Release()` 참조 카운팅
- ✅ `HRESULT` 에러 처리
- ✅ `SUCCEEDED()` / `FAILED()` 매크로

**WASAPI 개념**:
- ✅ `IMMDeviceEnumerator` (디바이스 목록)
- ✅ `IMMDevice` (디바이스 객체)
- ✅ `IAudioClient` (오디오 세션)
- ✅ `IAudioCaptureClient` (캡처 인터페이스)
- ✅ `AUDCLNT_STREAMFLAGS_LOOPBACK` (루프백 모드)
- ✅ 패킷 기반 스트리밍 (10ms 폴링)
- ✅ `CoTaskMemFree()` 메모리 해제

### 실전 경험

**검증 완료**:
- ✅ ATL COM DLL 빌드 성공
- ✅ COM 객체 생명주기 관리
- ✅ WASAPI 루프백 캡처 (48kHz, 240K 프레임)
- ✅ 실시간 패킷 스트리밍 (500개 패킷)
- ✅ 메모리 정리 완료 (누수 없음)

---

## 🎯 다음 단계 (Day 2)

### 우선순위 1: Phase 3.2 (PID 기반 캡처) ⭐

**목표**: Chrome/Discord만 선택적 캡처

**핵심 작업**:
1. `AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS` 구조체 설정
2. `ActivateAudioInterfaceAsync` API 사용
3. 비동기 완료 핸들러 구현
4. Chrome YouTube만 캡처 검증
5. Discord만 캡처 검증

**예상 소요**: 2-3시간  
**난이도**: ⭐⭐⭐ 어려움

**참고 자료**:
- Week 0 콘솔 PoC (ProcessLoopbackCapture 사용)
- Microsoft WASAPI 문서

---

## 📚 참고 자료

**완성된 문서**:

- ✅ `docs/learning-notes.md`: COM + WASAPI + Day 1 학습 상세
- ✅ `docs/README.md`: 프로젝트 전체 현황
- ✅ `docs/phase-progress.md`: 이 문서
- ✅ `docs/build-errors.md`: 에러 해결 (업데이트 예정)

**레퍼런스 코드**:

- ✅ `phase1-console/poc/`: 콘솔 PoC (Week 0)
- ✅ `phase1-console/reference/ProcessLoopbackCapture/`: 원본 레포
- ✅ `phase2-learning/HelloCOM/`: ATL DLL 템플릿
- ✅ `phase2-learning/CppBasics/`: 포인터 실습
- ✅ `phase2-learning/COMBasics/`: COM 기본
- ✅ `phase2-learning/AudioCapture/`: WASAPI 캡처 ⭐

**외부 자료**:

- [ProcessLoopbackCapture GitHub](https://github.com/Naseband/ProcessLoopbackCapture)
- [Microsoft WASAPI 문서](https://learn.microsoft.com/en-us/windows/win32/coreaudio/wasapi)
- [ATL COM 튜토리얼](https://learn.microsoft.com/en-us/cpp/atl/)

---

## 🎉 현재까지 완료 요약

### Week 0 (2025-11-16)

- ✅ 프로젝트 초기화 및 문서 구조
- ✅ COM 기초 개념 학습
- ✅ WASAPI 아키텍처 이해
- ✅ ProcessLoopbackCapture 코드 분석
- ✅ **콘솔 PoC 완성 및 검증** ⭐
- ✅ Chrome/Discord 프로세스 자동 감지
- ✅ PID 기반 오디오 캡처 실전 검증

### Day 1 (2025-11-17)

- ✅ **Visual Studio 2026 + ATL 환경 구축**
- ✅ **C++ 포인터/참조 학습 완료**
- ✅ **COM 기본 개념 습득** (AddRef/Release, HRESULT)
- ✅ **WASAPI 루프백 캡처 성공** ⭐
  - 48kHz 스테레오, 240,000 프레임
  - 500개 패킷 실시간 수신
  - 실제 오디오 데이터 검증 완료

### 학습한 프로젝트

1. **HelloCOM** - ATL DLL 템플릿 이해
2. **CppBasics** - 포인터와 참조 실습
3. **COMBasics** - COM 디바이스 정보 가져오기
4. **AudioCapture** - WASAPI 루프백 캡처 ⭐

### 시간 효율

```
Week 0: 14h → 6h (-8h)
Day 1:  6h → 4h (-2h)
───────────────────────
누적 절감: -10h ✨
남은 예산: 40h
진행률: 30% (10h / 50h)
```

### 자신감 획득

```
Week 0:
"PID 기반 캡처가 정말 되는구나!" ✅
"16kHz 변환이 자동으로 되네!" ✅
"프로세스 자동 감지도 가능하구나!" ✅

Day 1:
"ATL 프로젝트 빌드 성공!" ✅
"포인터/COM 개념 이해했다!" ✅
"WASAPI 루프백 캡처 작동한다!" ✅
"이제 PID 캡처만 추가하면 되겠다!" ✅
```

---

**다음 목표**: Phase 3.2 (PID 기반 캡처) - Day 2 🚀

**마지막 업데이트**: 2025-11-17 (Day 1 완료)  
**Week 1 상태**: 진행 중 (40% 완료) 🔄  
**다음 마일스톤**: Day 2 - PID 기반 캡처 (T+10-18h)