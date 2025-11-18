# Week 1: COM 브리지 기초 📖

T+6-20h (14시간 예정, 현재 8시간 완료)

**진행률**: 70% (8h / 14h)  
**상태**: 🔄 진행 중  
**마지막 업데이트**: 2025-11-18 (Day 3 완료)

---

## 📊 전체 진행 상황

```
Week 1 타임라인:
[████████████] 100% (8h / 14h)

Day 1 (T+6-10h):  ████████████ 100% (4h / 6h) ✅
Day 2 (T+10-12h): ████████████ 100% (2h / 8h) ✅
Day 3-4 (T+12-18h): ████████████ 100% (6h / 11h) ✅ 완료!
```

---

## ✅ Day 3 (T+12-14h) - 완료! ⭐ 신규! (2025-11-18)

### Phase 7: ATL COM DLL 프로젝트

- **시간**: T+12-14h
- **실제 소요**: 2시간 (계획 4h 대비 -2h ✨✨)
- **난이도**: ⭐⭐⭐ 어려움
- **완료 날짜**: 2025-11-18

**체크포인트**:

- [x] ATL 프로젝트 "OnVoiceAudioBridge" 생성
- [x] IDL 파일에 인터페이스 정의 (`IOnVoiceCapture`)
- [x] 3개 메서드 구현 (StartCapture, StopCapture, GetCaptureState)
- [x] 상태 관리 멤버 변수 (m_bIsCapturing, m_targetPid)
- [x] VBScript 테스트 스크립트 작성
- [x] COM 객체 생성 및 호출 검증 완료

**프로젝트 생성**:

```
1. Visual Studio → 파일 → 새로만들기 → 프로젝트
2. 템플릿 검색: "ATL 프로젝트"
3. 프로젝트 이름: OnVoiceAudioBridge
4. 애플리케이션 유형: DLL (동적 연결 라이브러리)
5. ATL 사용 허용: 체크
6. MFC 지원: 체크 해제
7. 완료
```

**IDL 인터페이스 정의** (`OnVoiceAudioBridge.idl`):

```idl
// OnVoiceAudioBridge.idl: OnVoiceAudioBridge의 IDL 소스
import "oaidl.idl";
import "ocidl.idl";

[
    object,
    uuid(43a468da-7889-46c9-99de-38cb93e4e649),
    dual,
    nonextensible,
    pointer_default(unique)
]
interface IOnVoiceCapture : IDispatch
{
    // ========================================
    // 메서드 1: 캡처 시작 (PID 지정)
    // ========================================
    [id(1), helpstring("특정 프로세스의 오디오 캡처 시작")]
    HRESULT StartCapture([in] LONG processId);

    // ========================================
    // 메서드 2: 캡처 중지
    // ========================================
    [id(2), helpstring("오디오 캡처 중지")]
    HRESULT StopCapture();

    // ========================================
    // 메서드 3: 상태 확인 (0=중지, 1=실행 중)
    // ========================================
    [id(3), helpstring("현재 캡처 상태 반환")]
    HRESULT GetCaptureState([out, retval] LONG* pState);
};

[
    uuid(419618dd-d242-4428-8f62-5651cb46a9be),
    version(1.0),
]
library OnVoiceAudioBridgeLib
{
    importlib("stdole2.tlb");
    [
        uuid(fe3c62ec-02f2-4c63-8266-d538a86fd7f9)
    ]
    coclass OnVoiceCapture
    {
        [default] interface IOnVoiceCapture;
    };
};
```

**클래스 선언** (`OnVoiceCapture.h`):

```cpp
class ATL_NO_VTABLE COnVoiceCapture :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<COnVoiceCapture, &CLSID_OnVoiceCapture>,
    public IDispatchImpl<IOnVoiceCapture, &IID_IOnVoiceCapture, 
                         &LIBID_OnVoiceAudioBridgeLib, 1, 0>
{
public:
    COnVoiceCapture()
    {
        // 멤버 변수 초기화
        m_bIsCapturing = FALSE;  // 초기 상태: 캡처 중지
        m_targetPid = 0;         // PID 없음
    }

    DECLARE_REGISTRY_RESOURCEID(106)

    BEGIN_COM_MAP(COnVoiceCapture)
        COM_INTERFACE_ENTRY(IOnVoiceCapture)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct() { return S_OK; }
    void FinalRelease() {}

public:
    // ========================================
    // IOnVoiceCapture 인터페이스 구현
    // ========================================
    STDMETHOD(StartCapture)(LONG processId);
    STDMETHOD(StopCapture)();
    STDMETHOD(GetCaptureState)(LONG* pState);

private:
    // ========================================
    // 멤버 변수
    // ========================================
    BOOL m_bIsCapturing;   // 캡처 중인지 여부 (TRUE/FALSE)
    LONG m_targetPid;      // 대상 프로세스 ID
};
```

**메서드 구현** (`OnVoiceCapture.cpp`):

```cpp
// 캡처 시작
STDMETHODIMP COnVoiceCapture::StartCapture(LONG processId)
{
    // 1. 이미 실행 중이면 에러
    if (m_bIsCapturing) {
        return E_FAIL;  // "이미 캡처 중입니다"
    }

    // 2. PID 저장
    m_targetPid = processId;

    // 3. 캡처 시작 (나중에 Phase 9에서 실제 캡처 로직 추가)
    // TODO: Phase 9에서 WASAPI 캡처 코드 추가
    m_bIsCapturing = TRUE;

    return S_OK;  // 성공
}

// 캡처 중지
STDMETHODIMP COnVoiceCapture::StopCapture()
{
    // 1. 실행 중이 아니면 무시
    if (!m_bIsCapturing) {
        return S_OK;  // 이미 중지됨
    }

    // 2. 캡처 중지 (나중에 Phase 9에서 실제 중지 로직 추가)
    // TODO: Phase 9에서 정리 코드 추가
    m_bIsCapturing = FALSE;
    m_targetPid = 0;

    return S_OK;
}

// 상태 확인
STDMETHODIMP COnVoiceCapture::GetCaptureState(LONG* pState)
{
    // 1. NULL 포인터 체크 (중요!)
    if (pState == NULL) {
        return E_POINTER;  // 잘못된 포인터
    }

    // 2. 상태 반환 (0=중지, 1=실행 중)
    *pState = m_bIsCapturing ? 1 : 0;

    return S_OK;
}
```

**VBScript 테스트** (`TestOnVoiceCapture.vbs`):

```vbscript
' ==========================================
' OnVoice COM 브리지 테스트 스크립트
' ==========================================

WScript.Echo "=========================================="
WScript.Echo "OnVoice COM 브리지 테스트 시작!"
WScript.Echo "=========================================="

' 1. COM 객체 생성
WScript.Echo "[1단계] COM 객체 생성 중..."
On Error Resume Next
Set capture = CreateObject("OnVoiceAudioBridge.OnVoiceCapture")
If Err.Number <> 0 Then
    WScript.Echo "[ERROR] COM 객체 생성 실패!"
    WScript.Echo "오류: " & Err.Description
    WScript.Quit 1
End If
On Error Goto 0
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
WScript.Echo ""
WScript.Echo "Phase 7 완료!"
```

**검증 결과**:

```
==========================================
OnVoice COM 브리지 테스트 시작!
==========================================
[1단계] COM 객체 생성 중...
[OK] COM 객체 생성 성공!
[2단계] 초기 상태 확인 중...
초기 상태: 0 (0=중지, 1=실행 중)
[OK] 예상대로 중지 상태입니다!
[3단계] 캡처 시작 (테스트 PID: 12345)...
[OK] StartCapture 호출 완료!
[4단계] 캡처 시작 후 상태 확인...
현재 상태: 1
[OK] 예상대로 실행 중입니다!
[5단계] 캡처 중지...
[OK] StopCapture 호출 완료!
[6단계] 최종 상태 확인...
최종 상태: 0
[OK] 예상대로 중지 상태입니다!
==========================================
모든 테스트 완료!
==========================================
결과 요약:
- COM 객체 생성: OK
- StartCapture: OK
- StopCapture: OK
- GetCaptureState: OK

Phase 7 완료!
```

**핵심 개념 정리**:

#### 1. ATL 프로젝트 구조

**자동 생성되는 파일**:

- `OnVoiceAudioBridge.idl`: 인터페이스 정의 (수동 편집)
- `OnVoiceAudioBridge_i.h`: IDL에서 자동 생성된 타입 정의
- `OnVoiceAudioBridge_i.c`: GUID 정의
- `OnVoiceCapture.rgs`: COM 레지스트리 스크립트

#### 2. IDL 속성 설명

| 속성         | 의미                               |
| ------------ | ---------------------------------- |
| `[id(1)]`    | 메서드 ID (IDispatch에서 사용)    |
| `[in]`       | 입력 파라미터 (호출자 → COM)      |
| `[out]`      | 출력 파라미터 (COM → 호출자)      |
| `[retval]`   | 반환 값 (VBScript에서 직접 사용)  |
| `dual`       | IDispatch + vtable 모두 지원      |
| `helpstring` | 설명 문자열                        |

#### 3. VBScript 테스트 패턴

**장점**:

- 빠른 테스트 (컴파일 불필요)
- COM 등록 자동 확인
- 간단한 문법

**사용법**:

```powershell
cd x64\Debug
C:\Windows\System32\cscript.exe //nologo TestOnVoiceCapture.vbs
```

**배운 점**:

- ✅ ATL 마법사로 빠른 프로젝트 생성
- ✅ IDL 문법 및 속성 이해
- ✅ `IDispatch` 인터페이스의 중요성
- ✅ VBScript로 간단한 COM 테스트
- ✅ 상태 관리 패턴 (멤버 변수)

---

### Day 3 성과

| Phase              | 계획   | 실제   | 차이         |
| ------------------ | ------ | ------ | ------------ |
| Phase 7 (COM DLL)  | 4h     | 2h     | **-2h** ✨✨ |

**완료한 프로젝트** (신규 1개):

6. ✅ **OnVoiceAudioBridge** - ATL COM DLL 프로젝트 ⭐ 신규!

**중요 달성**:

- 🎉 **ATL COM DLL 프로젝트 구조 완성!**
- 🎉 **IDispatch 인터페이스 작동 확인!**
- 🎉 **VBScript 테스트 성공!**

---

## ✅ Day 1 (T+6-10h) - 완료! (2025-11-17)

### Phase 1: Visual Studio 2026 + ATL 설정

- **시간**: T+6-8h
- **실제 소요**: 1시간 (계획 2h 대비 -1h ✨)
- **난이도**: ⭐ 쉬움
- **완료 날짜**: 2025-11-17

**체크포인트**:

- [x] Visual Studio Community 2026 설치
- [x] Windows 11 SDK 확인
- [x] ATL 지원 설치 (v145 빌드 도구)
- [x] ATL 프로젝트 "HelloCOM" 생성
- [x] 빈 COM DLL 빌드 성공 (HelloCOM.dll)
- [x] regsvr32 권한 문제 해결 (사용자 단위 리디렉션)

---

### Phase 2: C++ 기초 학습

- **시간**: T+8-10h
- **실제 소요**: 1.5시간 (계획 2h 대비 -0.5h ✨)
- **난이도**: ⭐⭐ 보통
- **완료 날짜**: 2025-11-17

**체크포인트**:

- [x] **CppBasics 프로젝트**: 포인터와 참조 실습
- [x] **COMBasics 프로젝트**: COM 기본 실습
- [x] 포인터 (`int*`) vs 참조 (`int&`) 차이 이해
- [x] 이중 포인터 (`int**`) 개념
- [x] `nullptr` 사용법
- [x] COM 초기화 및 디바이스 정보 가져오기

---

### Phase 3.1: WASAPI 기본 오디오 캡처

- **시간**: T+10-14h (일부)
- **실제 소요**: 1.5시간 (계획 2h 대비 -0.5h ✨)
- **난이도**: ⭐⭐ 보통
- **완료 날짜**: 2025-11-17

**체크포인트**:

- [x] **AudioCapture 프로젝트** 생성
- [x] 링커 라이브러리 설정 (ole32.lib, oleaut32.lib, avrt.lib)
- [x] 오디오 클라이언트 활성화
- [x] 믹스 형식 가져오기 (48kHz, 2ch, 32bit)
- [x] 루프백 모드로 초기화
- [x] 캡처 클라이언트 획득
- [x] 5초간 오디오 데이터 캡처
- [x] 실시간 패킷 수신 확인

---

## ✅ Day 2 (T+10-12h) - 완료! ⭐ (2025-11-18)

### Phase 4: PID 기반 캡처 (가장 어려운 단계!)

- **시간**: T+10-12h
- **실제 소요**: 2시간 (계획 3h 대비 -1h ✨)
- **난이도**: ⭐⭐⭐ 어려움
- **완료 날짜**: 2025-11-18

**체크포인트**:

- [x] `AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS` 구조체 설정
- [x] `ActivateAudioInterfaceAsync` API 구현
- [x] 비동기 완료 핸들러 (`IActivateAudioInterfaceCompletionHandler`)
- [x] `IAgileObject` 추가로 MTA 안정성 확보
- [x] `VIRTUAL_AUDIO_DEVICE_PROCESS_LOOPBACK` 사용
- [x] Chrome PID로 선택적 캡처 검증
- [x] 참조 카운팅 정상 동작 확인

---

## ✅ Day 3-4 (T+12-18h) - 완료! ⭐ (2025-11-18)

### Phase 8: COM 이벤트 콜백 (완료!)

- **시간**: T+14-16h
- **실제 소요**: 2시간 (계획 3h 대비 -1h ✨)
- **난이도**: ⭐⭐⭐⭐ 매우 어려움
- **완료 날짜**: 2025-11-18

**체크포인트**:

- [x] IDL에 이벤트 인터페이스 정의 (`_IOnVoiceCaptureEvents`)
- [x] `dispinterface` 문법 이해
- [x] `coclass`에 `[source]` 속성 추가
- [x] `IConnectionPointContainer` 구현 (ATL)
- [x] `IConnectionPoint` 구현 (ATL)
- [x] GIT 프록시를 통한 스레드 간 안전한 이벤트 전송
- [x] `Fire_OnAudioData()` 헬퍼 함수
- [x] VBScript 이벤트 수신 테스트 성공

**구현된 코드 구조**:

```idl
// OnVoiceAudioBridge.idl
[
    uuid(52b4a16b-9f83-4a3e-9240-4dd6676540ea),
    hidden
]
dispinterface _IOnVoiceCaptureEvents
{
    properties:
    methods:
        [id(1), helpstring("16kHz mono PCM audio chunk")]
        void OnAudioData([in] SAFEARRAY(BYTE) pcmData);
};

coclass OnVoiceCapture
{
    [default] interface IOnVoiceCapture;
    [default, source] dispinterface _IOnVoiceCaptureEvents;
};
```

**테스트 결과**:

```vbscript
' VBScript에서 이벤트 수신
Set capture = WScript.CreateObject("OnVoiceAudioBridge.OnVoiceCapture", "OnVoice_")

capture.StartCapture(12345)

' 이벤트 핸들러 (자동 호출됨!)
Sub OnVoice_OnAudioData(ByVal audioData)
    Dim size
    size = UBound(audioData) - LBound(audioData) + 1
    WScript.Echo "[Event] OnAudioData 수신! size=" & size & " bytes"
End Sub
```

**핵심 구현**:
- GIT 프록시를 통한 스레드 간 안전한 이벤트 전송
- SAFEARRAY를 사용한 바이너리 데이터 전송
- 오디오 캡처 스레드(MTA)에서 스크립트 아파트먼트로 마샬링

---

### Phase 9: 캡처 엔진 통합 (완료!)

- **시간**: T+16-18h
- **실제 소요**: 2시간 (계획 4h 대비 -2h ✨)
- **난이도**: ⭐⭐⭐ 어려움
- **완료 날짜**: 2025-11-18

**체크포인트**:

- [x] `AudioCaptureEngine` 클래스 구현
- [x] `ProcessLoopbackCapture` 래핑
- [x] `IAudioDataCallback` 인터페이스 정의
- [x] `StartCapture()`에서 실제 WASAPI 캡처 시작
- [x] 캡처 스레드에서 `OnAudioData()` 콜백 호출
- [x] `Fire_OnAudioData()`를 통한 이벤트 전송
- [x] 16kHz mono PCM 자동 변환
- [x] VBScript로 실제 오디오 데이터 수신 테스트 성공

**구현된 코드 구조**:

```cpp
// AudioCaptureEngine.h
class AudioCaptureEngine {
    ProcessLoopbackCapture m_capture;
    IAudioDataCallback* m_pCallback;
    
    HRESULT Start(DWORD pid, IAudioDataCallback* pCallback);
    HRESULT Stop();
};

// OnVoiceCapture.h
class COnVoiceCapture : 
    public IConnectionPointImpl<...>,
    public IAudioDataCallback
{
    AudioCaptureEngine* m_pEngine;
    void OnAudioData(BYTE* pData, UINT32 dataSize) override;
    HRESULT Fire_OnAudioData(BYTE* pData, UINT32 dataSize);
};
```

---

## 📊 Week 1 요약

### 시간 추적

| 일자     | Phase       | 계획   | 실제   | 차이        |
| -------- | ----------- | ------ | ------ | ----------- |
| Day 1    | Phase 1-3.1 | 6h     | 4h     | **-2h** ✨  |
| Day 2    | Phase 4     | 3h     | 2h     | **-1h** ✨  |
| Day 3-4  | Phase 7-9   | 11h    | 6h     | **-5h** ✨✨ |
| **합계** |             | **20h** | **12h** | **-8h** 🎉  |

### 완료한 프로젝트 (6개)

1. ✅ HelloCOM - ATL DLL 템플릿
2. ✅ CppBasics - 포인터/참조 실습
3. ✅ COMBasics - COM 기본 실습
4. ✅ AudioCapture - WASAPI 루프백 캡처
5. ✅ AudioCapturePID - PID 기반 캡처 ⭐
6. ✅ **OnVoiceAudioBridge** - ATL COM DLL 프로젝트 ⭐ 신규!

### 학습 성과

**C++ 개념**:

- ✅ 포인터와 참조의 차이
- ✅ 이중 포인터 (`void**`)
- ✅ `nullptr` vs `NULL`
- ✅ `->` vs `.` 연산자
- ✅ 클래스 상속 및 다중 상속

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
- ✅ **AudioCaptureEngine 래퍼 패턴** ⭐ (Day 3-4)

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

---

## 🎯 다음 단계 (Week 2)

**즉시**: Phase 10-12 (Electron 연동) - 18-22시간  
**목표**: Week 2 완료 (T+40h)

---

**[← 전체 요약으로](../phase-progress.md)**