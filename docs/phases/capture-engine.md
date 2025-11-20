# 오디오 캡처 엔진 구현 🎙️

OnVoiceAudioBridge 오디오 캡처 엔진 구현 상세

**완료 날짜**: 2025-11-18  
**소요 시간**: 2시간  
**상태**: ✅ 완료

---

## 🎯 개요

PID 기반 프로세스별 오디오 캡처를 담당하는 엔진 구현. `ProcessLoopbackCapture`를 래핑하여 COM 인터페이스와 통합.

---

## 🏗️ 아키텍처

### 계층 구조

```
COnVoiceCapture (COM 클래스)
  ↓
AudioCaptureEngine (래퍼 클래스)
  ↓
ProcessLoopbackCapture (실제 WASAPI 캡처)
  ↓
Windows WASAPI
```

### 데이터 흐름

```
WASAPI 오디오 스트림
  ↓
ProcessLoopbackCapture (오디오 스레드)
  ↓
AudioCaptureEngine::LoopbackCallback (static)
  ↓
AudioCaptureEngine::HandleLoopbackData
  ↓
IAudioDataCallback::OnAudioData
  ↓
COnVoiceCapture::OnAudioData
  ↓
Fire_OnAudioData (COM 이벤트)
```

---

## 📋 구현된 클래스

### AudioCaptureEngine

**역할**: `ProcessLoopbackCapture`를 래핑하여 COM과 통합

**주요 메서드**:

#### Start

```cpp
HRESULT Start(DWORD pid, IAudioDataCallback* pCallback);
```

**기능**: 특정 PID 프로세스의 오디오 캡처 시작

**파라미터**:
- `pid`: 캡처할 프로세스 ID
- `pCallback`: 오디오 데이터 수신 콜백

**처리 과정**:
1. 캡처 형식 설정 (16kHz, 16-bit, mono, PCM)
2. 타깃 프로세스 설정 (inclusive mode)
3. 콜백 등록
4. 캡처 시작

**반환값**:
- `S_OK`: 성공
- `E_INVALIDARG`: 잘못된 PID
- `E_POINTER`: NULL 콜백
- `HRESULT_FROM_WIN32(ERROR_BUSY)`: 이미 캡처 중
- 기타 WASAPI 에러

---

#### Stop

```cpp
HRESULT Stop();
```

**기능**: 현재 진행 중인 캡처 중지

**처리 과정** (데드락 방지 수정):
1. **콜백 연결 먼저 끊기** (`m_pCallback = nullptr`)
   - 오디오 스레드가 더 이상 콜백을 호출하지 않도록 차단
2. **진행 중인 콜백 완료 대기** (50ms Sleep)
   - 이미 콜백 내부에 진입한 스레드가 빠져나올 시간 확보
3. **스레드 안전하게 종료** (`ProcessLoopbackCapture::StopCapture()`)
   - 더 이상 오디오 스레드가 Main Thread로 Invoke를 날리지 않으므로 데드락 방지
4. 콜백 포인터 정리

---

### IAudioDataCallback

**역할**: 오디오 데이터를 COM 클래스로 전달하는 콜백 인터페이스

```cpp
class IAudioDataCallback {
public:
    virtual ~IAudioDataCallback() {}
    virtual void OnAudioData(BYTE* pData, UINT32 dataSize) = 0;
};
```

**구현**: `COnVoiceCapture`가 이 인터페이스를 구현하여 오디오 데이터를 받음

---

## 🔧 핵심 구현

### 1. 캡처 형식 설정

```cpp
// STT 용도로 16kHz / 16bit / mono 설정
eCaptureError err = m_capture.SetCaptureFormat(
    16000,             // SampleRate
    16,                // BitDepth
    1,                 // Channels
    WAVE_FORMAT_PCM    // PCM
);
```

**설정값**:
- Sample Rate: 16kHz (STT 최적화)
- Bit Depth: 16-bit
- Channels: Mono
- Format: PCM

**이유**: STT(음성 인식)에 최적화된 형식으로, Windows WASAPI가 자동으로 변환 처리

---

### 2. 타깃 프로세스 설정

```cpp
// inclusive: 이 PID 트리만 캡처
err = m_capture.SetTargetProcess(pid, /*bInclusive=*/true);
```

**Inclusive 모드**: 지정된 PID와 그 자식 프로세스만 캡처

**예시**:
- Discord PID 지정 → Discord 앱의 오디오만 캡처
- Chrome PID 지정 → Chrome 브라우저의 오디오만 캡처
- 다른 앱의 오디오는 무시

---

### 3. 콜백 등록

```cpp
// static 콜백 함수 등록
err = m_capture.SetCallback(
    &AudioCaptureEngine::LoopbackCallback,  // static 함수
    this                                      // userData (this 포인터)
);
```

**Static 콜백 패턴**:
```cpp
// ProcessLoopbackCapture가 호출하는 static 함수
static void LoopbackCallback(
    const std::vector<unsigned char>::iterator& begin,
    const std::vector<unsigned char>::iterator& end,
    void* userData)
{
    AudioCaptureEngine* pThis = static_cast<AudioCaptureEngine*>(userData);
    pThis->HandleLoopbackData(begin, end);
}

// 실제 처리 (인스턴스 메서드)
void HandleLoopbackData(
    const std::vector<unsigned char>::iterator& begin,
    const std::vector<unsigned char>::iterator& end)
{
    // 데이터 크기 계산
    const size_t size = std::distance(begin, end);
    
    // COM 초기화 (오디오 스레드용)
    static thread_local bool s_comInitialized = false;
    if (!s_comInitialized) {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        s_comInitialized = true;
    }
    
    // 콜백 호출
    BYTE* pData = reinterpret_cast<BYTE*>(&(*begin));
    m_pCallback->OnAudioData(pData, static_cast<UINT32>(size));
}
```

**중요**: 오디오 스레드에서 COM을 사용하기 위해 `CoInitializeEx` 호출 필요

---

## 🔐 스레드 안전성

### 오디오 스레드 COM 초기화

```cpp
// HandleLoopbackData에서
static thread_local bool s_comInitialized = false;
if (!s_comInitialized) {
    HRESULT hrCo = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (SUCCEEDED(hrCo) || hrCo == RPC_E_CHANGED_MODE) {
        s_comInitialized = true;
    }
}
```

**이유**: 
- 오디오 캡처 스레드는 별도 스레드에서 실행
- COM 이벤트 전송을 위해 COM 초기화 필요
- `thread_local`로 스레드당 한 번만 초기화

**주의**: `CoUninitialize()`는 호출하지 않음 (프로세스 종료 시 자동 정리)

---

## 📊 상태 관리

### ProcessLoopbackCapture 상태

```cpp
enum class eCaptureState {
    READY = 0,      // 준비됨 (중지됨)
    CAPTURING,      // 캡처 중
    PAUSED          // 일시 정지
};
```

### 상태 전환

```
READY → CAPTURING (StartCapture)
CAPTURING → READY (StopCapture)
```

**검증**: `GetState()`로 현재 상태 확인 후 작업 수행

---

## ✅ 검증 결과

### 캡처 테스트

```cpp
// AudioCaptureEngine 생성
AudioCaptureEngine* engine = new AudioCaptureEngine();

// 캡처 시작
HRESULT hr = engine->Start(discordPid, callback);

// 결과 확인
// ✅ 16kHz mono PCM 데이터 수신
// ✅ 실시간 스트리밍 정상 작동
// ✅ 프로세스 격리 확인 (Discord만 캡처)
```

**검증 항목**:
- ✅ Discord/Chrome PID로 선택적 캡처 성공
- ✅ 16kHz mono PCM 데이터 확인
- ✅ 실시간 스트리밍 정상 작동
- ✅ 프로세스 격리 확인 (다른 앱 오디오 무시)

---

## 🎓 학습 포인트

### 1. 래퍼 패턴

- **목적**: 기존 라이브러리를 COM과 통합
- **장점**: 기존 코드 재사용, COM 인터페이스 분리

### 2. Static 콜백 패턴

- **문제**: C 스타일 콜백은 인스턴스 메서드 직접 호출 불가
- **해결**: Static 함수 + userData(this 포인터)

### 3. 스레드 로컬 COM 초기화

- **필요성**: 오디오 스레드에서 COM 사용 시 초기화 필요
- **주의**: `thread_local`로 스레드당 한 번만 초기화

---

## 🔗 관련 문서

- **[COM 인터페이스 구현](com-interface.md)** - StartCapture/StopCapture
- **[이벤트 시스템 구현](event-system.md)** - OnAudioData 이벤트
- **[테스트 및 검증](testing.md)** - 캡처 테스트

---

**다음**: [테스트 및 검증 →](testing.md)

