# 구현 비교: AudioCapture vs AudioCaptureEngine 📊

AudioCapture.cpp (학습용)와 AudioCaptureEngine.cpp (프로덕션용)의 차이점 분석

**작성일**: 2025-11-18  
**목적**: 학습 단계에서 프로덕션 단계로의 진화 과정 이해

---

## 🎯 개요

| 항목 | AudioCapture.cpp | AudioCaptureEngine.cpp |
|------|------------------|------------------------|
| **위치** | `phase2-learning/AudioCapturePID/` | `phase3-com-dll/OnVoiceAudioBridge/` |
| **목적** | WASAPI PID 캡처 학습 | COM DLL 통합용 래퍼 |
| **복잡도** | 낮음 (직접 구현) | 중간 (추상화 레이어) |
| **재사용성** | 낮음 (main 함수) | 높음 (클래스 기반) |

---

## 📋 구조 비교

### AudioCapture.cpp (학습용)

**특징**:
- `main()` 함수로 직접 실행
- 모든 WASAPI 초기화를 직접 구현
- `ActivateAudioInterfaceCompletionHandler` 클래스를 직접 작성
- `ActivateAudioInterfaceAsync` 직접 호출
- 실제 캡처 루프는 미구현 (주석 처리)

**코드 구조**:
```cpp
int main()
{
    // 1. COM 초기화
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    
    // 2. 디바이스 열거자 생성
    CoCreateInstance(__uuidof(MMDeviceEnumerator), ...);
    
    // 3. PID 기반 루프백 파라미터 설정
    AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS loopbackParams = {};
    AUDIOCLIENT_ACTIVATION_PARAMS blob = {};
    PROPVARIANT activation_params = {};
    
    // 4. ActivateAudioInterfaceAsync 호출
    ActivateAudioInterfaceAsync(
        VIRTUAL_AUDIO_DEVICE_PROCESS_LOOPBACK,
        __uuidof(IAudioClient),
        &activation_params,
        pHandler,
        &pAsyncOp);
    
    // 5. IAudioClient 획득
    // ...(실제 캡처 루프는 미구현)...
}
```

---

### AudioCaptureEngine.cpp (프로덕션용)

**특징**:
- `ProcessLoopbackCapture` 래퍼 클래스
- COM 인터페이스와 통합 (`IAudioDataCallback`)
- 캡처 엔진의 생명주기 관리
- 실제 오디오 데이터 전송 구현

**코드 구조**:
```cpp
class AudioCaptureEngine {
    ProcessLoopbackCapture m_capture;
    IAudioDataCallback* m_pCallback;
    
    HRESULT Start(DWORD pid, IAudioDataCallback* pCallback);
    HRESULT Stop();
};

HRESULT AudioCaptureEngine::Start(DWORD pid, IAudioDataCallback* pCallback)
{
    // 1. 캡처 형식 설정
    m_capture.SetCaptureFormat(16000, 16, 1, WAVE_FORMAT_PCM);
    
    // 2. 타깃 프로세스 설정
    m_capture.SetTargetProcess(pid, true);
    
    // 3. 콜백 등록
    m_capture.SetCallback(&LoopbackCallback, this);
    
    // 4. 캡처 시작 (내부적으로 ActivateAudioInterfaceAsync 호출)
    m_capture.StartCapture();
}
```

---

## 🔍 주요 차이점

### 1. **추상화 레벨**

#### AudioCapture.cpp
- **낮은 추상화**: WASAPI API를 직접 호출
- **명시적 제어**: 모든 단계를 직접 구현
- **학습 목적**: WASAPI 동작 원리 이해

```cpp
// 직접 구조체 설정
AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS loopbackParams = {};
loopbackParams.TargetProcessId = targetPid;
loopbackParams.ProcessLoopbackMode = 
    PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE;

AUDIOCLIENT_ACTIVATION_PARAMS blob = {};
blob.ActivationType = AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK;
blob.ProcessLoopbackParams = loopbackParams;

// 직접 PROPVARIANT 래핑
PROPVARIANT activation_params = {};
activation_params.vt = VT_BLOB;
activation_params.blob.cbSize = sizeof(AUDIOCLIENT_ACTIVATION_PARAMS);
activation_params.blob.pBlobData = reinterpret_cast<BYTE*>(&blob);

// 직접 ActivateAudioInterfaceAsync 호출
ActivateAudioInterfaceAsync(
    VIRTUAL_AUDIO_DEVICE_PROCESS_LOOPBACK,
    __uuidof(IAudioClient),
    &activation_params,
    pHandler,
    &pAsyncOp);
```

#### AudioCaptureEngine.cpp
- **높은 추상화**: `ProcessLoopbackCapture` 래퍼 사용
- **간단한 API**: 복잡한 설정을 메서드 호출로 대체
- **프로덕션 목적**: 재사용 가능한 컴포넌트

```cpp
// 간단한 메서드 호출
m_capture.SetTargetProcess(pid, true);
m_capture.StartCapture();

// 내부적으로 ProcessLoopbackCapture::StartCapture()에서:
// - AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS 설정
// - AUDIOCLIENT_ACTIVATION_PARAMS 설정
// - PROPVARIANT 래핑
// - ActivateAudioInterfaceAsync 호출
// 모두 자동 처리됨
```

---

### 2. **비동기 처리**

#### AudioCapture.cpp
- **수동 이벤트 대기**: `WaitForSingleObject` 직접 사용
- **완료 핸들러 직접 구현**: `ActivateAudioInterfaceCompletionHandler` 클래스 작성
- **IAgileObject 구현**: MTA 스레드 안전성 직접 처리

```cpp
class ActivateAudioInterfaceCompletionHandler :
    public IActivateAudioInterfaceCompletionHandler,
    public IAgileObject
{
    HANDLE m_hEvent;
    HRESULT m_hrActivateResult;
    IUnknown* m_pUnknown;
    
    STDMETHODIMP ActivateCompleted(...) {
        // 수동으로 결과 처리
        operation->GetActivateResult(&hrActivateResult, &pUnknown);
        SetEvent(m_hEvent);
    }
};

// main에서 대기
WaitForSingleObject(pHandler->GetEvent(), 5000);
```

#### AudioCaptureEngine.cpp
- **자동 처리**: `ProcessLoopbackCapture`가 내부적으로 처리
- **블로킹 API**: `StartCapture()`가 완료될 때까지 대기
- **에러 처리**: `eCaptureError` enum으로 통일

```cpp
// 단순한 블로킹 호출
eCaptureError err = m_capture.StartCapture();
if (err != eCaptureError::NONE) {
    HRESULT hrLast = m_capture.GetLastErrorResult();
    return hrLast;
}
```

---

### 3. **콜백 시스템**

#### AudioCapture.cpp
- **콜백 미구현**: 실제 오디오 데이터 수신 로직 없음
- **IAudioClient만 획득**: 이후 캡처 루프는 주석 처리

```cpp
printf("✅ IAudioClient 획득 성공!\n");
printf("✅ 이제 PID %lu 의 오디오만 캡처 가능합니다! 🎉\n", targetPid);

// ...(여기부터는 실제 WASAPI 캡처 초기화/루프 들어갈 자리)...
```

#### AudioCaptureEngine.cpp
- **완전한 콜백 구현**: `IAudioDataCallback` 인터페이스 사용
- **실제 데이터 전송**: 오디오 스레드에서 COM 객체로 전달
- **COM 초기화**: 오디오 스레드에서 COM 사용 가능하도록 처리

```cpp
// 콜백 인터페이스
class IAudioDataCallback {
    virtual void OnAudioData(BYTE* pData, UINT32 dataSize) = 0;
};

// 오디오 스레드에서 호출
void AudioCaptureEngine::HandleLoopbackData(...)
{
    // COM 초기화 (오디오 스레드용)
    static thread_local bool s_comInitialized = false;
    if (!s_comInitialized) {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        s_comInitialized = true;
    }
    
    // 콜백 호출
    m_pCallback->OnAudioData(pData, static_cast<UINT32>(size));
}
```

---

### 4. **에러 처리**

#### AudioCapture.cpp
- **HRESULT 직접 처리**: 각 단계마다 HRESULT 확인
- **상세한 로깅**: 각 단계의 성공/실패를 printf로 출력
- **수동 정리**: 실패 시 모든 리소스 수동 해제

```cpp
if (FAILED(hr)) {
    printf("❌ ActivateAudioInterfaceAsync 실패: 0x%08X\n", hr);
    pHandler->Release();
    CoTaskMemFree(deviceIdString);
    device->Release();
    enumerator->Release();
    CoUninitialize();
    return 1;
}
```

#### AudioCaptureEngine.cpp
- **에러 enum 사용**: `eCaptureError`로 통일된 에러 처리
- **상세한 에러 메시지**: 가능한 원인 제시
- **자동 정리**: 소멸자에서 자동으로 리소스 해제

```cpp
if (err != eCaptureError::NONE) {
    HRESULT hrLast = m_capture.GetLastErrorResult();
    printf("[Engine] ❌ StartCapture 실패: %s (hr=0x%08X)\n",
        LoopbackCaptureConst::GetErrorText(err), hrLast);
    printf("[Engine] 가능한 원인:\n");
    printf("[Engine]   1. PID가 잘못됨 (프로세스가 종료됨)\n");
    printf("[Engine]   2. 해당 프로세스가 오디오를 재생하지 않음\n");
    // ...
    return (hrLast != S_OK) ? hrLast : E_FAIL;
}
```

---

### 5. **상태 관리**

#### AudioCapture.cpp
- **상태 관리 없음**: 단순히 성공/실패만 확인
- **일회성 실행**: main 함수 종료 시 모든 리소스 해제

#### AudioCaptureEngine.cpp
- **상태 기반 제어**: `eCaptureState` enum 사용
- **생명주기 관리**: Start/Stop으로 상태 전환
- **재사용 가능**: 여러 번 Start/Stop 가능

```cpp
// READY 상태가 아니면 Start 불가
eCaptureState state = m_capture.GetState();
if (state != eCaptureState::READY) {
    return HRESULT_FROM_WIN32(ERROR_BUSY);
}

// 상태 전환: READY → CAPTURING
err = m_capture.StartCapture();
```

---

### 6. **캡처 형식 설정**

#### AudioCapture.cpp
- **형식 설정 없음**: IAudioClient만 획득하고 종료
- **실제 캡처 미구현**: Initialize, Start, 캡처 루프 모두 없음

#### AudioCaptureEngine.cpp
- **명시적 형식 설정**: 16kHz, 16-bit, mono, PCM
- **완전한 캡처 파이프라인**: 형식 설정 → 프로세스 설정 → 콜백 등록 → 시작

```cpp
// STT 용도로 16kHz / 16bit / mono 설정
eCaptureError err = m_capture.SetCaptureFormat(
    16000,             // SampleRate
    16,                // BitDepth
    1,                 // Channels
    WAVE_FORMAT_PCM    // PCM
);
```

---

## 🔄 진화 과정

### 단계 1: AudioCapture.cpp (학습)
```
목표: WASAPI PID 캡처 원리 이해
- ActivateAudioInterfaceAsync 직접 호출
- 비동기 완료 핸들러 직접 구현
- IAudioClient 획득까지 성공
```

### 단계 2: ProcessLoopbackCapture (참조 구현)
```
목표: 재사용 가능한 캡처 엔진 확보
- 모든 WASAPI 초기화 로직 캡슐화
- 상태 관리 및 에러 처리 통합
- 실제 캡처 루프 구현
```

### 단계 3: AudioCaptureEngine (프로덕션)
```
목표: COM DLL과 통합
- ProcessLoopbackCapture 래핑
- IAudioDataCallback 인터페이스 제공
- COM 스레드 안전성 확보
```

---

## 📊 비교 요약

| 기능 | AudioCapture.cpp | AudioCaptureEngine.cpp |
|------|------------------|------------------------|
| **WASAPI 직접 호출** | ✅ 있음 | ❌ 없음 (ProcessLoopbackCapture 사용) |
| **비동기 핸들러 구현** | ✅ 직접 구현 | ❌ ProcessLoopbackCapture 내부 처리 |
| **실제 캡처 루프** | ❌ 없음 | ✅ ProcessLoopbackCapture 내부 |
| **오디오 데이터 전송** | ❌ 없음 | ✅ IAudioDataCallback 구현 |
| **상태 관리** | ❌ 없음 | ✅ eCaptureState 사용 |
| **에러 처리** | ✅ HRESULT 직접 | ✅ eCaptureError + 상세 메시지 |
| **재사용성** | ❌ main 함수만 | ✅ 클래스 기반 |
| **COM 통합** | ❌ 없음 | ✅ IAudioDataCallback 인터페이스 |
| **스레드 안전성** | ✅ IAgileObject 구현 | ✅ ProcessLoopbackCapture + COM 초기화 |

---

## 🎓 학습 포인트

### AudioCapture.cpp에서 배운 것
1. **WASAPI PID 캡처 원리**: `ActivateAudioInterfaceAsync` 사용법
2. **비동기 처리**: `IActivateAudioInterfaceCompletionHandler` 구현
3. **MTA 스레드 안전성**: `IAgileObject` 필요성
4. **PROPVARIANT 래핑**: 구조체를 COM으로 전달하는 방법

### AudioCaptureEngine.cpp로 개선된 것
1. **추상화**: 복잡한 WASAPI 로직을 간단한 API로 감춤
2. **재사용성**: 클래스 기반으로 여러 인스턴스 생성 가능
3. **에러 처리**: 통일된 에러 enum과 상세 메시지
4. **COM 통합**: `IAudioDataCallback`으로 데이터 전송

---

## 🔗 관련 문서

- **[캡처 엔진 구현](capture-engine.md)** - AudioCaptureEngine 상세
- **[COM 인터페이스 구현](com-interface.md)** - COnVoiceCapture와의 통합
- **[이벤트 시스템 구현](event-system.md)** - 오디오 데이터 전송

---

**다음**: [캡처 엔진 구현 →](capture-engine.md)

