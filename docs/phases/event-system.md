# COM 이벤트 시스템 구현 🔔

OnVoiceAudioBridge COM 이벤트 시스템 구현 상세

**완료 날짜**: 2025-11-18  
**데드락 수정**: 2025-11-20  
**소요 시간**: 2시간  
**상태**: ✅ 완료

---

## 🎯 개요

오디오 캡처 스레드에서 발생하는 실시간 PCM 데이터를 VBScript/JavaScript로 안전하게 전송하기 위한 COM 이벤트 시스템 구현.

**핵심 도전과제**: 오디오 캡처 스레드(MTA)에서 스크립트 아파트먼트(STA)로 안전하게 이벤트 전송

---

## 🏗️ 아키텍처

### 데이터 흐름

```
ProcessLoopbackCapture (오디오 스레드, MTA)
  ↓
AudioCaptureEngine::HandleLoopbackData
  ↓
IAudioDataCallback::OnAudioData
  ↓
COnVoiceCapture::Fire_OnAudioData
  ↓
GIT 프록시를 통한 마샬링
  ↓
IDispatch::Invoke (스크립트 아파트먼트, STA)
  ↓
VBScript/JavaScript 이벤트 핸들러
```

### 스레드 모델

- **오디오 캡처 스레드**: MTA (Multi-Threaded Apartment)
- **스크립트 아파트먼트**: STA (Single-Threaded Apartment)
- **해결책**: GIT (Global Interface Table) 프록시 사용

---

## 📋 구현된 인터페이스

### _IOnVoiceCaptureEvents

**인터페이스 타입**: `dispinterface`  
**GUID**: `52b4a16b-9f83-4a3e-9240-4dd6676540ea`

**이벤트**:

#### OnAudioData

```idl
[id(1), helpstring("16kHz mono PCM audio chunk")]
void OnAudioData([in] SAFEARRAY(BYTE) pcmData);
```

**기능**: 실시간 오디오 데이터 청크 전송

**파라미터**:
- `pcmData` (in): 16kHz mono 16-bit PCM 바이트 배열

**호출 빈도**: 오디오 스트림에 따라 실시간 (약 10-20ms 간격)

---

## 🔧 핵심 구현

### 1. IConnectionPoint 구현

```cpp
class COnVoiceCapture :
    public IConnectionPointContainerImpl<COnVoiceCapture>,
    public IConnectionPointImpl<COnVoiceCapture, &__uuidof(_IOnVoiceCaptureEvents)>
{
    BEGIN_CONNECTION_POINT_MAP(COnVoiceCapture)
        CONNECTION_POINT_ENTRY(__uuidof(_IOnVoiceCaptureEvents))
    END_CONNECTION_POINT_MAP()
};
```

**역할**: 이벤트 싱크(스크립트)의 등록/해제 관리

---

### 2. GIT 프록시 등록 (StartCapture 시)

```cpp
// StartCapture에서 이벤트 싱크를 GIT에 등록
m_gitSinks.clear();
const int nConnections = m_vec.GetSize();  // ATL 내부 싱크 리스트
m_gitSinks.resize(nConnections);

for (int i = 0; i < nConnections; ++i) {
    IUnknown* pUnk = m_vec.GetAt(i);
    CComQIPtr<IDispatch> spDisp(pUnk);
    
    if (spDisp) {
        // GIT에 등록 (스레드 간 안전한 프록시 생성)
        m_gitSinks[i].Attach(spDisp);
    }
}
```

**목적**: 오디오 스레드에서 스크립트 아파트먼트로 안전하게 호출하기 위한 프록시 준비

---

### 3. 이벤트 전송 (Fire_OnAudioData)

```cpp
HRESULT COnVoiceCapture::Fire_OnAudioData(BYTE* pData, UINT32 dataSize)
{
    // ⭐ [데드락 방지] 캡처 중이 아니면 즉시 리턴
    if (m_state != CaptureState::Capturing)
        return S_OK;
    
    if (!pData || dataSize == 0)
        return S_OK;
    
    // 1. SAFEARRAY 생성
    SAFEARRAYBOUND sab = { dataSize, 0 };
    SAFEARRAY* psa = SafeArrayCreate(VT_UI1, 1, &sab);
    
    // 2. 데이터 복사
    BYTE* pArrayData = nullptr;
    SafeArrayAccessData(psa, (void**)&pArrayData);
    memcpy(pArrayData, pData, dataSize);
    SafeArrayUnaccessData(psa);
    
    // 3. VARIANT로 감싸기
    VARIANT varAudio;
    VariantInit(&varAudio);
    varAudio.vt = VT_ARRAY | VT_UI1;
    varAudio.parray = psa;
    
    // 4. 각 이벤트 싱크에 전송
    for (int i = 0; i < m_gitSinks.size(); ++i) {
        CComPtr<IDispatch> spDispatch;
        
        // GIT에서 프록시 가져오기 (이 스레드에서 사용 가능)
        HRESULT hrGit = m_gitSinks[i].CopyTo(&spDispatch);
        if (FAILED(hrGit) || !spDispatch) continue;
        
        // IDispatch::Invoke 호출
        DISPPARAMS dp = {};
        dp.rgvarg = &varAudio;
        dp.cArgs = 1;
        
        spDispatch->Invoke(
            dispidOnAudioData,  // [id(1)] OnAudioData
            IID_NULL,
            LOCALE_USER_DEFAULT,
            DISPATCH_METHOD,
            &dp,
            nullptr,
            nullptr,
            nullptr
        );
    }
    
    // 5. 정리
    VariantClear(&varAudio);  // SAFEARRAY도 함께 해제
    return S_OK;
}
```

**핵심 포인트**:
- `CComGITPtr::CopyTo()`: GIT에서 현재 스레드용 프록시 획득
- `IDispatch::Invoke()`: 스크립트 아파트먼트로 마샬링되어 호출
- `SAFEARRAY`: 바이너리 데이터를 스크립트로 전달하는 표준 방법

---

## 🔐 스레드 안전성

### 문제 상황

```
오디오 캡처 스레드 (MTA)
  → 직접 IDispatch 호출 시도
  → ❌ 크래시! (STA 아파트먼트 규칙 위반)
```

### 해결책: GIT 프록시

```
오디오 캡처 스레드 (MTA)
  → GIT 프록시 획득 (CopyTo)
  → IDispatch::Invoke 호출
  → ✅ COM 런타임이 자동으로 마샬링
  → 스크립트 아파트먼트 (STA)에서 안전하게 실행
```

**GIT (Global Interface Table)**:
- COM 런타임의 전역 인터페이스 테이블
- 스레드 간 인터페이스 포인터 공유를 안전하게 처리
- 프록시를 통한 자동 마샬링

---

## 🐛 데드락 방지 (2025-11-20 수정)

### 문제 상황

**데드락 발생 시나리오**:
```
1. Main Thread: StopCapture() 호출
2. Main Thread: m_pEngine->Stop() 호출 → 스레드 Join 대기
3. 오디오 스레드: Fire_OnAudioData() 내부에서 IDispatch::Invoke 호출
4. 오디오 스레드: Main Thread(STA)로 마샬링 대기
5. ❌ 데드락! (Main Thread는 Join 대기, 오디오 스레드는 Invoke 대기)
```

### 해결책: 3단계 방어

#### 1. Fire_OnAudioData에서 상태 체크 (1차 방어)

```cpp
HRESULT COnVoiceCapture::Fire_OnAudioData(BYTE* pData, UINT32 dataSize)
{
    // ⭐ 캡처 중이 아니면 즉시 리턴 (데드락 방지 핵심)
    if (m_state != CaptureState::Capturing)
        return S_OK;
    // ... 나머지 로직
}
```

**효과**: `StopCapture()`에서 상태를 `Stopping`으로 변경하면 오디오 스레드가 더 이상 이벤트를 보내지 않음

#### 2. StopCapture에서 상태 먼저 변경 (2차 방어)

```cpp
STDMETHODIMP COnVoiceCapture::StopCapture()
{
    // 1. 상태를 먼저 변경하여 오디오 스레드가 이벤트를 더 이상 보내지 않게 함
    m_state = CaptureState::Stopping;
    
    // 2. VBScript가 마지막 이벤트를 처리하고 루프를 빠져나올 시간을 벌어줌
    Sleep(200);  // 50ms -> 200ms로 증가
    
    // 3. 엔진 정지 (스레드 Join)
    // 이제 오디오 스레드는 Fire_OnAudioData 진입 시 m_state 체크에서 튕겨 나가므로
    // 메인 스레드를 기다리지 않고 바로 종료됩니다.
    hr = m_pEngine->Stop();
}
```

**효과**: 상태 변경으로 오디오 스레드가 이벤트 전송을 중단하고, 대기 시간으로 진행 중인 이벤트 처리 완료

#### 3. AudioCaptureEngine::Stop에서 콜백 먼저 끊기 (3차 방어)

```cpp
HRESULT AudioCaptureEngine::Stop()
{
    // ⭐ [핵심 수정 1] 콜백 연결을 먼저 끊습니다.
    // 이제 오디오 스레드는 데이터를 캡처해도 HandleLoopbackData에서 즉시 리턴하게 됩니다.
    m_pCallback = nullptr;
    
    // ⭐ [핵심 수정 2] 진행 중인 콜백이 빠져나갈 시간을 줍니다.
    Sleep(50);
    
    // ⭐ [기존 로직] 이제 안전하게 스레드를 종료(Join)합니다.
    // 더 이상 오디오 스레드가 Main Thread로 Invoke를 날리지 않으므로 데드락이 걸리지 않습니다.
    eCaptureError err = m_capture.StopCapture();
}
```

**효과**: 콜백을 먼저 끊어서 오디오 스레드가 더 이상 콜백을 호출하지 않도록 차단

### 수정 결과

```
✅ StopCapture() 호출 시 데드락 없이 정상 종료
✅ 오디오 스레드가 안전하게 종료
✅ VBScript 이벤트 처리 완료 후 정상 종료
```

---

## 📝 IDL 파일 구조

```idl
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
    [default, source] dispinterface _IOnVoiceCaptureEvents;  // 이벤트 소스
};
```

**`[source]` 속성**: 이 인터페이스가 이벤트를 발생시키는 소스임을 명시

---

## ✅ 검증 결과

### VBScript 테스트

```vbscript
' 이벤트 prefix 지정하여 COM 객체 생성
Set capture = WScript.CreateObject("OnVoiceAudioBridge.OnVoiceCapture", "OnVoice_")

' 캡처 시작
capture.StartCapture(12345)

' 이벤트 핸들러 (자동 호출됨!)
Sub OnVoice_OnAudioData(ByVal audioData)
    Dim size
    size = UBound(audioData) - LBound(audioData) + 1
    WScript.Echo "[Event] OnAudioData 수신! size=" & size & " bytes"
End Sub
```

**결과**:
```
✅ CreateObject(..., "OnVoice_") 성공
✅ StartCapture(PID) → 실제 오디오 캡처 시작
✅ OnVoice_OnAudioData 이벤트 수신 성공
✅ 16kHz mono PCM 데이터 실시간 전송 확인
✅ 스레드 간 안전성 검증 완료
```

---

## 🎓 학습 포인트

### 1. IConnectionPoint 패턴

- **역할**: 이벤트 소스와 싱크 간 연결 관리
- **ATL 지원**: `IConnectionPointImpl` 템플릿으로 간단히 구현

### 2. GIT 프록시 패턴

- **문제**: MTA 스레드에서 STA 객체 직접 호출 불가
- **해결**: GIT에 등록 → 프록시 획득 → 자동 마샬링

### 3. SAFEARRAY 사용

- **목적**: 바이너리 데이터를 스크립트로 전달
- **주의**: `VariantClear()`로 반드시 해제

---

## 🔗 관련 문서

- **[COM 인터페이스 구현](com-interface.md)** - 기본 인터페이스
- **[캡처 엔진 구현](capture-engine.md)** - 오디오 데이터 생성
- **[테스트 및 검증](testing.md)** - 이벤트 테스트

---

**다음**: [캡처 엔진 구현 →](capture-engine.md)

