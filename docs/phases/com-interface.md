# COM 인터페이스 구현 📡

OnVoiceAudioBridge COM 인터페이스 구현 상세

**완료 날짜**: 2025-11-18  
**소요 시간**: 2시간  
**상태**: ✅ 완료

---

## 🎯 개요

VBScript, JavaScript(winax) 등에서 호출 가능한 COM 인터페이스를 구현하여 PID 기반 오디오 캡처를 제어할 수 있도록 함.

---

## 📋 구현된 인터페이스

### IOnVoiceCapture

**인터페이스 타입**: `IDispatch` (dual)  
**GUID**: `43a468da-7889-46c9-99de-38cb93e4e649`

**메서드**:

#### 1. StartCapture

```idl
[id(1), helpstring("특정 프로세스의 오디오 캡처 시작")]
HRESULT StartCapture([in] LONG processId);
```

**기능**: 지정된 PID의 프로세스 오디오 캡처 시작

**파라미터**:
- `processId` (in): 캡처할 프로세스 ID

**반환값**:
- `S_OK`: 성공
- `E_INVALIDARG`: 잘못된 PID (<= 0)
- `HRESULT_FROM_WIN32(ERROR_BUSY)`: 이미 캡처 중
- `E_OUTOFMEMORY`: 메모리 부족
- 기타 WASAPI 에러

**상태 전환**:
- `Stopped` → `Starting` → `Capturing` (성공 시)
- `Stopped` → `Starting` → `Stopped` (실패 시)

---

#### 2. StopCapture

```idl
[id(2), helpstring("오디오 캡처 중지")]
HRESULT StopCapture();
```

**기능**: 현재 진행 중인 오디오 캡처 중지

**반환값**:
- `S_OK`: 성공 (이미 중지된 경우도 성공)

**상태 전환**:
- `Capturing` → `Stopping` → `Stopped`
- `Starting` → `Stopping` → `Stopped`

---

#### 3. GetCaptureState

```idl
[id(3), helpstring("현재 캡처 상태 반환")]
HRESULT GetCaptureState([out, retval] LONG* pState);
```

**기능**: 현재 캡처 상태 조회

**파라미터**:
- `pState` (out, retval): 상태 값 (0=Stopped, 1=Starting, 2=Capturing, 3=Stopping)

**반환값**:
- `S_OK`: 성공
- `E_POINTER`: NULL 포인터

---

## 🏗️ 구현 구조

### 클래스 계층

```cpp
class COnVoiceCapture :
    public CComObjectRootEx<CComMultiThreadModel>,      // 멀티스레드 지원
    public CComCoClass<COnVoiceCapture, &CLSID_OnVoiceCapture>,
    public IDispatchImpl<IOnVoiceCapture, ...>,          // IDispatch 구현
    public IConnectionPointContainerImpl<COnVoiceCapture>, // 이벤트 컨테이너
    public IConnectionPointImpl<COnVoiceCapture, ...>,  // 이벤트 포인트
    public IAudioDataCallback                            // 캡처 엔진 콜백
```

### 멤버 변수

```cpp
private:
    AudioCaptureEngine* m_pEngine;       // PID 기반 캡처 엔진
    CaptureState m_state;                // 현재 상태
    LONG m_targetPid;                    // 타깃 프로세스 ID
    DWORD m_ownerThreadId;               // 객체 생성 스레드 ID
    std::vector<CComGITPtr<IDispatch>> m_gitSinks;  // GIT 프록시 리스트
```

---

## 🔧 핵심 구현 로직

### StartCapture 구현

```cpp
STDMETHODIMP COnVoiceCapture::StartCapture(LONG processId)
{
    // 1. 유효성 검사
    if (processId <= 0) return E_INVALIDARG;
    if (m_state == Starting || m_state == Capturing) 
        return HRESULT_FROM_WIN32(ERROR_BUSY);

    // 2. AudioCaptureEngine 지연 생성
    if (!m_pEngine) {
        m_pEngine = new (std::nothrow) AudioCaptureEngine();
        if (!m_pEngine) return E_OUTOFMEMORY;
    }

    // 3. GIT 프록시 등록 (이벤트 싱크용)
    m_gitSinks.clear();
    // ... 이벤트 싱크를 GIT에 등록 ...

    // 4. 캡처 시작
    m_state = CaptureState::Starting;
    HRESULT hr = m_pEngine->Start(static_cast<DWORD>(processId), this);
    
    if (SUCCEEDED(hr)) {
        m_state = CaptureState::Capturing;
    } else {
        m_state = CaptureState::Stopped;
        m_gitSinks.clear();
    }

    return hr;
}
```

### StopCapture 구현

```cpp
STDMETHODIMP COnVoiceCapture::StopCapture()
{
    if (m_state == Stopped || m_state == Stopping) {
        return S_OK;
    }

    m_state = CaptureState::Stopping;

    if (m_pEngine) {
        HRESULT hr = m_pEngine->Stop();
        // 에러 처리...
    }

    m_state = CaptureState::Stopped;
    m_targetPid = 0;
    m_gitSinks.clear();

    return S_OK;
}
```

---

## 📝 IDL 파일 구조

```idl
[
    object,
    uuid(43a468da-7889-46c9-99de-38cb93e4e649),
    dual,                    // IDispatch + vtable
    nonextensible,
    pointer_default(unique)
]
interface IOnVoiceCapture : IDispatch
{
    [id(1)] HRESULT StartCapture([in] LONG processId);
    [id(2)] HRESULT StopCapture();
    [id(3)] HRESULT GetCaptureState([out, retval] LONG* pState);
};

[
    uuid(fe3c62ec-02f2-4c63-8266-d538a86fd7f9)
]
coclass OnVoiceCapture
{
    [default] interface IOnVoiceCapture;
    [default, source] dispinterface _IOnVoiceCaptureEvents;
};
```

---

## ✅ 검증 결과

### VBScript 테스트

```vbscript
Set capture = CreateObject("OnVoiceAudioBridge.OnVoiceCapture")

' 초기 상태 확인
state = capture.GetCaptureState()  ' → 0 (Stopped)

' 캡처 시작
capture.StartCapture(12345)
state = capture.GetCaptureState()  ' → 2 (Capturing)

' 캡처 중지
capture.StopCapture()
state = capture.GetCaptureState()  ' → 0 (Stopped)
```

**결과**: ✅ 모든 테스트 통과

---

## 🔗 관련 문서

- **[이벤트 시스템 구현](event-system.md)** - COM 이벤트 전송
- **[캡처 엔진 구현](capture-engine.md)** - 실제 WASAPI 캡처
- **[테스트 및 검증](testing.md)** - VBScript 테스트

---

**다음**: [이벤트 시스템 구현 →](event-system.md)

