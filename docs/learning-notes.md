# C++ 학습 노트 📚

OnVoice COM 브리지 개발 핵심 개념 빠른 참조

**마지막 업데이트**: 2025-11-20 (E2E 테스트 성공)  
**상태**: Week 1 완료 ✅ + E2E 테스트 성공 🎉 → Week 2 준비 중

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

### ATL COM 프로젝트 체크리스트 (신규!) ⭐

```cpp
// 1. ATL 프로젝트 생성
// Visual Studio → 새 프로젝트 → "ATL 프로젝트"
// 옵션: DLL (동적 연결 라이브러리)

// 2. IDL 파일에 인터페이스 정의
interface IOnVoiceCapture : IDispatch
{
    [id(1)] HRESULT StartCapture([in] LONG pid);
    [id(2)] HRESULT StopCapture();
    [id(3)] HRESULT GetCaptureState([out, retval] LONG* pState);
};

// 3. 클래스에 멤버 변수 추가
BOOL m_bIsCapturing;  // 상태
LONG m_targetPid;     // PID

// 4. 메서드 구현
STDMETHODIMP COnVoiceCapture::StartCapture(LONG processId)
{
    if (m_bIsCapturing) return E_FAIL;
    m_targetPid = processId;
    m_bIsCapturing = TRUE;
    return S_OK;
}

// 5. VBScript로 테스트
Set capture = CreateObject("OnVoiceAudioBridge.OnVoiceCapture")
capture.StartCapture(12345)
WScript.Echo capture.GetCaptureState()  ' → 1
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
mfplat.lib    (Media Foundation - ActivateAudioInterfaceAsync) ⭐
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

### 2. Process-Specific Loopback ✅ (Day 2 완료!)

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

## 🆕 Week 1 학습 내용 (2025-11-18) ⭐ 신규!

### Phase 7-9: COM 브리지 완성

**완료한 것**:

- ✅ Phase 7: OnVoiceAudioBridge ATL 프로젝트 생성
- ✅ IDL 인터페이스 정의 (IOnVoiceCapture)
- ✅ 3개 메서드 구현 (StartCapture, StopCapture, GetCaptureState)
- ✅ 상태 관리 개선 (CaptureState enum)
- ✅ Phase 8: COM 이벤트 콜백 구현
- ✅ Phase 9: AudioCaptureEngine 통합
- ✅ 실제 WASAPI 캡처 및 실시간 스트리밍
- ✅ VBScript 이벤트 수신 테스트 성공

**배운 것**:

- ✅ ATL 프로젝트 마법사 사용법
- ✅ IDL (Interface Definition Language) 문법
- ✅ `IDispatch` 인터페이스 상속
- ✅ `[in]`, `[out, retval]` 파라미터 속성
- ✅ **COM 이벤트 시스템** (IConnectionPoint, IConnectionPointContainer)
- ✅ **GIT (Global Interface Table) 프록시**를 통한 스레드 간 안전한 이벤트 전송
- ✅ **SAFEARRAY**를 사용한 오디오 데이터 전송
- ✅ VBScript 이벤트 수신 테스트
- ✅ **AudioCaptureEngine** 래퍼 클래스 패턴

---

#### 핵심 개념 5가지:

#### 1. ATL 프로젝트 구조

**ATL (Active Template Library)**: COM 객체를 쉽게 만들 수 있는 C++ 템플릿 라이브러리

```
OnVoiceAudioBridge/
├── OnVoiceAudioBridge.idl      # 인터페이스 정의 (IDL)
├── OnVoiceCapture.h            # 클래스 선언
├── OnVoiceCapture.cpp          # 클래스 구현
├── OnVoiceCapture.rgs          # 레지스트리 스크립트
├── OnVoiceAudioBridge_i.h      # IDL에서 자동 생성 (타입 정의)
└── OnVoiceAudioBridge_i.c      # IDL에서 자동 생성 (GUID)
```

**프로젝트 생성 단계**:

```
1. Visual Studio → 새 프로젝트 → "ATL 프로젝트"
2. 애플리케이션 유형: "동적 연결 라이브러리 (DLL)"
3. ATL 사용 허용: 체크
4. MFC 지원: 체크 해제
5. 완료 → 프로젝트 생성됨
```

#### 2. IDL (Interface Definition Language)

**IDL**: COM 인터페이스를 정의하는 언어 (사람과 컴파일러 모두가 읽을 수 있음)

```idl
// ========================================
// 인터페이스 정의
// ========================================
[
    object,                                    // COM 인터페이스
    uuid(43a468da-7889-46c9-99de-38cb93e4e649), // 고유 ID (GUID)
    dual,                                      // IDispatch + vtable
    nonextensible,                             // 확장 불가
    pointer_default(unique)                    // 포인터 기본 속성
]
interface IOnVoiceCapture : IDispatch
{
    // 메서드 1: 캡처 시작
    [id(1), helpstring("특정 프로세스의 오디오 캡처 시작")]
    HRESULT StartCapture([in] LONG processId);

    // 메서드 2: 캡처 중지
    [id(2), helpstring("오디오 캡처 중지")]
    HRESULT StopCapture();

    // 메서드 3: 상태 확인
    [id(3), helpstring("현재 캡처 상태 반환")]
    HRESULT GetCaptureState([out, retval] LONG* pState);
};
```

**IDL 속성 설명**:
| 속성 | 의미 |
|------|------|
| `[id(1)]` | 메서드 ID (IDispatch에서 사용) |
| `[in]` | 입력 파라미터 (호출자 → COM) |
| `[out]` | 출력 파라미터 (COM → 호출자) |
| `[retval]` | 반환 값 (VBScript에서 직접 받을 수 있음) |
| `helpstring` | 설명 문자열 (문서화) |

#### 3. IDispatch vs 일반 COM 인터페이스

**IDispatch**: VBScript, JavaScript 같은 스크립트 언어에서 호출 가능

```cpp
// 일반 COM 인터페이스 (C++만)
interface IOnVoiceCapture : IUnknown
{
    HRESULT StartCapture(LONG pid);
};

// IDispatch 인터페이스 (스크립트 언어도 가능!)
interface IOnVoiceCapture : IDispatch  // ⭐
{
    [id(1)] HRESULT StartCapture([in] LONG pid);
};
```

**dual 속성**: 두 가지 방식 모두 지원

```idl
[dual]  // C++ vtable + IDispatch 모두 지원
interface IOnVoiceCapture : IDispatch { ... };
```

#### 4. 클래스 구현 패턴

**OnVoiceCapture.h**:

```cpp
class ATL_NO_VTABLE COnVoiceCapture :
    public CComObjectRootEx<CComSingleThreadModel>,  // ATL 기본 기능
    public CComCoClass<COnVoiceCapture, &CLSID_OnVoiceCapture>, // COM 클래스
    public IDispatchImpl<IOnVoiceCapture, &IID_IOnVoiceCapture, ...> // IDispatch
{
public:
    COnVoiceCapture()
    {
        m_bIsCapturing = FALSE;  // 초기 상태: 중지
        m_targetPid = 0;
    }

    // COM 맵 (QueryInterface에서 사용)
    BEGIN_COM_MAP(COnVoiceCapture)
        COM_INTERFACE_ENTRY(IOnVoiceCapture)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    // 메서드 선언
    STDMETHOD(StartCapture)(LONG processId);
    STDMETHOD(StopCapture)();
    STDMETHOD(GetCaptureState)(LONG* pState);

private:
    BOOL m_bIsCapturing;  // 캡처 중인지 여부
    LONG m_targetPid;     // 대상 프로세스 ID
};
```

**OnVoiceCapture.cpp**:

```cpp
STDMETHODIMP COnVoiceCapture::StartCapture(LONG processId)
{
    // 1. 이미 실행 중이면 에러
    if (m_bIsCapturing) {
        return E_FAIL;
    }

    // 2. PID 저장
    m_targetPid = processId;

    // 3. 캡처 시작 (나중에 실제 로직 추가)
    m_bIsCapturing = TRUE;

    return S_OK;  // 성공
}

STDMETHODIMP COnVoiceCapture::GetCaptureState(LONG* pState)
{
    // 1. NULL 포인터 체크 (중요!)
    if (pState == NULL) {
        return E_POINTER;
    }

    // 2. 상태 반환
    *pState = m_bIsCapturing ? 1 : 0;

    return S_OK;
}
```

#### 5. VBScript 테스트 패턴

**TestOnVoiceCapture.vbs**:

```vbscript
' ==========================================
' OnVoice COM 브리지 테스트 스크립트
' ==========================================

' 1. COM 객체 생성
WScript.Echo "[1단계] COM 객체 생성 중..."
Set capture = CreateObject("OnVoiceAudioBridge.OnVoiceCapture")
WScript.Echo "[OK] COM 객체 생성 성공!"

' 2. 초기 상태 확인
WScript.Echo "[2단계] 초기 상태 확인 중..."
initialState = capture.GetCaptureState()
WScript.Echo "초기 상태: " & initialState & " (0=중지, 1=실행 중)"

If initialState = 0 Then
    WScript.Echo "[OK] 예상대로 중지 상태입니다!"
Else
    WScript.Echo "[FAIL] 예상과 다른 상태입니다!"
End If

' 3. 캡처 시작
WScript.Echo "[3단계] 캡처 시작 (테스트 PID: 12345)..."
capture.StartCapture(12345)
WScript.Echo "[OK] StartCapture 호출 완료!"

' 4. 실행 중 상태 확인
WScript.Echo "[4단계] 캡처 시작 후 상태 확인..."
runningState = capture.GetCaptureState()
WScript.Echo "현재 상태: " & runningState

If runningState = 1 Then
    WScript.Echo "[OK] 예상대로 실행 중입니다!"
Else
    WScript.Echo "[FAIL] 예상과 다른 상태입니다!"
End If

' 5. 캡처 중지
WScript.Echo "[5단계] 캡처 중지..."
capture.StopCapture()
WScript.Echo "[OK] StopCapture 호출 완료!"

' 6. 최종 상태 확인
WScript.Echo "[6단계] 최종 상태 확인..."
finalState = capture.GetCaptureState()
WScript.Echo "최종 상태: " & finalState

If finalState = 0 Then
    WScript.Echo "[OK] 예상대로 중지 상태입니다!"
Else
    WScript.Echo "[FAIL] 예상과 다른 상태입니다!"
End If

' 7. 요약
WScript.Echo "=========================================="
WScript.Echo "모든 테스트 완료!"
WScript.Echo "=========================================="
WScript.Echo "결과 요약:"
WScript.Echo "- COM 객체 생성: OK"
WScript.Echo "- StartCapture: OK"
WScript.Echo "- StopCapture: OK"
WScript.Echo "- GetCaptureState: OK"
```

**실행 방법**:

```powershell
# PowerShell에서 실행
cd x64\Debug
C:\Windows\System32\cscript.exe //nologo TestOnVoiceCapture.vbs
```

---

### Phase 7-9 검증 결과

**Phase 7 (기본 COM 인터페이스)**:

```
✅ CreateObject("OnVoiceAudioBridge.OnVoiceCapture") 성공
✅ 초기 상태: 0 (Stopped)
✅ StartCapture(12345) → 상태: 2 (Capturing)
✅ StopCapture() → 상태: 0 (Stopped)
```

**Phase 8-9 (이벤트 및 실제 캡처)**:

```
✅ CreateObject(..., "OnVoice_") 이벤트 연결 성공
✅ StartCapture(PID) → 실제 WASAPI 캡처 시작
✅ OnVoice_OnAudioData 이벤트 수신 성공
✅ 16kHz mono PCM 데이터 실시간 전송 확인
✅ 상태 전환: Stopped → Starting → Capturing → Stopping → Stopped
```

**E2E 테스트 결과 (2025-11-20)** 🎉:

```
✅ 945개의 오디오 청크 수집 성공
✅ 총 데이터 크기: 323,190 bytes
✅ StopCapture 정상 작동
✅ WAV 파일 저장 완료: capture_Chrome_2025-11-20_오후_25001.wav
✅ 전체 E2E 테스트 통과!
```

**테스트 스크립트**:

- `TestOnVoiceEvents.vbs` - 이벤트 수신 테스트
- `TestPidCapture.vbs` - PID 기반 캡처 테스트
- `TestAudioCaptureEngine.vbs` - 엔진 테스트

**중요한 발견**:

1. ✅ **ATL 마법사로 빠른 프로젝트 생성**
2. ✅ **IDL 수정 → 빌드 → 자동 코드 생성**
3. ✅ **IConnectionPoint를 통한 이벤트 시스템 구현**
4. ✅ **GIT 프록시로 스레드 간 안전한 이벤트 전송**
5. ✅ **SAFEARRAY를 사용한 바이너리 데이터 전송**
6. ✅ **AudioCaptureEngine 래퍼 패턴으로 코드 재사용**
7. ✅ **VBScript 이벤트 수신 테스트로 빠른 검증**
8. ✅ **CaptureState enum으로 정교한 상태 관리**

---

## 📊 진행 상황 (Day 1-3)

### 시간 추적

| Phase                 | 계획    | 실제   | 상태          |
| --------------------- | ------- | ------ | ------------- |
| **Day 1**             |         |        |               |
| Phase 1 (VS 설정)     | 2h      | 1h     | ✅ 완료       |
| Phase 2 (C++ 기초)    | 2h      | 1.5h   | ✅ 완료       |
| Phase 3.1 (기본 캡처) | 2h      | 1.5h   | ✅ 완료       |
| **Day 2**             |         |        |               |
| Phase 4 (PID 캡처)    | 3h      | 2h     | ✅ 완료       |
| **Day 3**             |         |        |               |
| Phase 7 (COM DLL)     | 4h      | 2h     | ✅ 완료       |
| **합계**              | **13h** | **8h** | **-5h** 절감! |

### 완료한 프로젝트 (6개)

1. **HelloCOM** - ATL DLL 템플릿 (빌드 성공)
2. **CppBasics** - 포인터/참조 실습 (실행 성공)
3. **COMBasics** - COM 디바이스 정보 (실행 성공)
4. **AudioCapture** - WASAPI 루프백 캡처 (실행 성공)
5. **AudioCapturePID** - PID 기반 캡처 (실행 성공) ⭐
6. **OnVoiceAudioBridge** - ATL COM DLL 프로젝트 (VBScript 테스트 성공) ⭐ 신규!

### 학습 성과

**C++ 개념**:

- ✅ 포인터와 참조의 차이
- ✅ 이중 포인터 (`void**`)
- ✅ `nullptr` vs `NULL`
- ✅ `->` vs `.` 연산자
- ✅ 클래스 상속 및 다중 상속 (`IAgileObject`)

**COM 개념**:

- ✅ `IUnknown` 인터페이스
- ✅ `AddRef()` / `Release()` 참조 카운팅
- ✅ `CoInitialize()` / `CoUninitialize()`
- ✅ `CoCreateInstance()` 객체 생성
- ✅ `HRESULT` 에러 처리
- ✅ **`IAgileObject` (MTA 안정성)** (Day 2)
- ✅ **비동기 COM 작업 동기화** (Day 2)
- ✅ **ATL 프로젝트 구조** ⭐ (Day 3)
- ✅ **IDL (Interface Definition Language)** ⭐ (Day 3)
- ✅ **IDispatch 인터페이스** ⭐ (Day 3)
- ✅ **VBScript COM 테스트** ⭐ (Day 3)
- ✅ **IConnectionPoint / IConnectionPointContainer** ⭐ (Day 3-4)
- ✅ **GIT 프록시를 통한 스레드 간 이벤트 전송** ⭐ (Day 3-4)
- ✅ **SAFEARRAY를 사용한 바이너리 데이터 전송** ⭐ (Day 3-4)

**WASAPI 개념**:

- ✅ `IMMDeviceEnumerator` (디바이스 목록)
- ✅ `IMMDevice` (디바이스 객체)
- ✅ `IAudioClient` (오디오 세션)
- ✅ `IAudioCaptureClient` (캡처 인터페이스)
- ✅ `AUDCLNT_STREAMFLAGS_LOOPBACK` (루프백 모드)
- ✅ 패킷 기반 스트리밍 패턴
- ✅ **`ActivateAudioInterfaceAsync` (비동기 활성화)** (Day 2)
- ✅ **`AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS` (PID 지정)** (Day 2)
- ✅ **`VIRTUAL_AUDIO_DEVICE_PROCESS_LOOPBACK` (가상 디바이스)** (Day 2)
- ✅ **AudioCaptureEngine 래퍼 패턴** ⭐ (Day 3-4)
- ✅ **ProcessLoopbackCapture 통합** ⭐ (Day 3-4)

---

## 🎯 다음 단계 (Week 2)

### Phase 10-12: Electron 연동 (최우선) ⭐

**목표**: COM DLL → Electron Main → Renderer 실시간 전송

**난이도**: ⭐⭐⭐⭐ **매우 어려움**

**핵심 작업**:

1. winax 설치 및 재빌드
2. Electron Main 프로세스에서 COM 객체 생성
3. 이벤트 수신 및 Renderer로 전송
4. IPC 통신 구현
5. E2E 테스트

**테스트 코드** (목표):

```javascript
// Electron Main 프로세스
const winax = require("winax");
const capture = new winax.Object("OnVoiceAudioBridge.OnVoiceCapture");

capture.OnAudioData = (audioData) => {
  // Renderer로 전송
  mainWindow.webContents.send("audio-data", audioData);
};

capture.StartCapture(discordPid);
```

**예상 소요**: 18-22시간

---

## 🎉 Week 0-1 완료 요약

### Week 0 (완료)

- ✅ COM 기초 개념 학습
- ✅ WASAPI 아키텍처 이해
- ✅ ProcessLoopbackCapture 코드 분석
- ✅ **콘솔 PoC 완성** (PID 기반 캡처)

### Week 1 (100% 완료) ✅

- ✅ Visual Studio 2026 + ATL 설정
- ✅ C++ 포인터/참조 학습
- ✅ COM 실습 (디바이스 정보)
- ✅ WASAPI 기본 캡처 성공
- ✅ **PID 기반 캡처 성공** (Day 2)
- ✅ **ATL COM DLL 프로젝트 완성** ⭐ (Day 3)
- ✅ **COM 이벤트 콜백 완성** ⭐ (Day 3-4)
- ✅ **캡처 엔진 통합 완성** ⭐ (Day 3-4)

### 시간 효율

| 항목             | 계획 | 실제 | 차이        |
| ---------------- | ---- | ---- | ----------- |
| Week 0           | 14h  | 6h   | **-8h** ✨  |
| Week 1 (Day 1)   | 6h   | 4h   | **-2h** ✨  |
| Week 1 (Day 2)   | 8h   | 2h   | **-6h** ✨  |
| Week 1 (Day 3-4) | 11h  | 6h   | **-5h** ✨  |
| **누적 절감**    | 39h  | 18h  | **-21h** 🎉 |
| **남은 시간**    | 50h  | 32h  | -           |

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

**5. `unresolved external symbol ActivateAudioInterfaceAsync`**

```
해결: 링커 → 입력 → 추가 종속성에 mfplat.lib 추가
```

**6. VBScript 실행 시 "개체를 만들 수 없습니다"** ⭐ 신규

```
문제: CreateObject("OnVoiceAudioBridge.OnVoiceCapture") 실패

해결 방법:
1. 관리자 권한으로 명령 프롬프트 열기
2. cd x64\Debug
3. regsvr32 OnVoiceAudioBridge.dll
4. "DllRegisterServer... 성공" 확인
5. VBScript 다시 실행

또는 프로젝트 속성에서:
링커 → 일반 → 사용자 단위 리디렉션 → "예(/user)"
```

**7. StopCapture 시 데드락 발생** 🐛 신규 (2025-11-20 수정)

```
문제: StopCapture() 호출 시 데드락 발생
- Main Thread: 스레드 Join 대기
- 오디오 스레드: IDispatch::Invoke 마샬링 대기

해결 방법 (3단계 방어):
1. Fire_OnAudioData에서 상태 체크 먼저 (Capturing이 아니면 즉시 리턴)
2. StopCapture에서 상태 먼저 변경 + 200ms 대기
3. AudioCaptureEngine::Stop에서 콜백 먼저 끊기 + 50ms 대기

결과: ✅ 데드락 없이 안전하게 종료
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

### ATL COM 프로젝트 패턴 (신규!) ⭐

```cpp
// 1. IDL 수정
interface IOnVoiceCapture : IDispatch
{
    [id(1)] HRESULT StartCapture([in] LONG pid);
};

// 2. 빌드 (Ctrl+Shift+B)
// → OnVoiceAudioBridge_i.h 자동 생성

// 3. 클래스에 멤버 변수 추가
private:
    BOOL m_bIsCapturing;
    LONG m_targetPid;

// 4. 메서드 구현
STDMETHODIMP COnVoiceCapture::StartCapture(LONG processId)
{
    if (m_bIsCapturing) return E_FAIL;
    m_targetPid = processId;
    m_bIsCapturing = TRUE;
    return S_OK;
}

// 5. VBScript 테스트
Set obj = CreateObject("OnVoiceAudioBridge.OnVoiceCapture")
obj.StartCapture(12345)
```

---

**문서 체계**:

```
learning-notes.md          ← 지금 여기 (빠른 참조 + Day 1-3 학습)
├── details/
│   ├── com-deep-dive.md   (COM 상세)
│   ├── wasapi-deep-dive.md (WASAPI 상세)
│   └── poc-lessons.md     (PoC 학습)
├── build-errors.md        (에러 해결)
└── phase-progress.md      (진행 상황)
```

---

**다음 학습 목표**: Phase 8 (COM 이벤트 콜백) - COM → Electron 이벤트 전송 구현 🎯
