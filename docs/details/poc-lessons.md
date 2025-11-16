# PoC Lessons Learned 🎓

ProcessLoopbackCapture 분석 및 실전 검증 결과

---

## 목차

- [핵심 패턴 5가지](#핵심-패턴-5가지)
- [메모리 관리](#메모리-관리)
- [Chrome 프로세스 구조](#chrome-프로세스-구조)
- [검증 결과](#검증-결과)
- [OnVoice 적용](#onvoice-적용)

---

## 핵심 패턴 5가지

### 1. 비동기 활성화 패턴

**문제**: ActivateAudioInterfaceAsync는 비동기

**해결**: Completion Handler + Event

```cpp
class ActivateCompletionHandler :
    public IActivateAudioInterfaceCompletionHandler
{
    HANDLE m_hEvent;

    HRESULT ActivateCompleted(IActivateAudioInterfaceAsyncOperation* op) {
        // 결과 획득
        op->GetActivateResult(&hr, &pUnknown);

        // 완료 신호
        SetEvent(m_hEvent);
        return S_OK;
    }
};

// 사용
ActivateAudioInterfaceAsync(..., pHandler, ...);
WaitForSingleObject(pHandler->m_hEvent, INFINITE);
```

### 2. 캡처 스레드 패턴

**문제**: WASAPI 캡처는 블로킹

**해결**: 별도 스레드 + this 포인터 전달

```cpp
static DWORD WINAPI CaptureThreadProc(LPVOID param) {
    ProcessLoopbackCapture* pThis = (ProcessLoopbackCapture*)param;

    // 우선순위 상승
    AvSetMmThreadCharacteristics(TEXT("Audio"), ...);

    pThis->m_pAudioClient->Start();
    SetEvent(pThis->m_hStartEvent);

    while (pThis->m_state == CAPTURING) {
        // 캡처 루프
    }

    return 0;
}
```

### 3. 3-Layer 에러 처리

```cpp
// Layer 1: enum
enum class eCaptureError {
    NONE,
    ALREADY_CAPTURING,
    INVALID_FORMAT,
    ...
};

// Layer 2: HRESULT 저장
HRESULT m_lastHResult;

// Layer 3: 텍스트 변환
const char* GetErrorText(eCaptureError error);
```

### 4. 스레드 안전성

```cpp
std::atomic<eCaptureState> m_state;

// Thread-safe
eCaptureState GetState() const {
    return m_state.load();
}
```

### 5. Lock-Free Queue

```cpp
moodycamel::ReaderWriterQueue<AudioChunk> m_queue;

// 오디오 스레드
m_queue.enqueue(chunk);

// Worker 스레드
if (m_queue.try_dequeue(chunk)) {
    // 느린 작업
}
```

---

## 메모리 관리

### DO: NULL 체크 후 Release

```cpp
if (m_pAudioClient != NULL) {
    m_pAudioClient->Release();
    m_pAudioClient = NULL;
}
```

### DO: 역순 정리

```cpp
// 생성 순서: Enumerator → Device → AudioClient → CaptureClient
// 정리 순서: CaptureClient → AudioClient → Device → Enumerator
```

### DO: 소멸자 자동 정리

```cpp
~ProcessLoopbackCapture() {
    if (m_state == CAPTURING) {
        StopCapture();
    }
}
```

### DON'T: Release 없이 재할당

```cpp
// ❌ 메모리 누수!
m_pAudioClient = NULL;

// ✅
if (m_pAudioClient) {
    m_pAudioClient->Release();
}
m_pAudioClient = NULL;
```

---

## Chrome 프로세스 구조

### 멀티프로세스 아키텍처

```
chrome.exe                      ← 브라우저 (오디오 재생)
chrome.exe --type=gpu-process   ← GPU
chrome.exe --type=renderer      ← 각 탭
chrome.exe --type=utility       ← 유틸리티
```

### 자동 감지

**방법 1: 명령줄 인수** (최선)

```cpp
std::wstring cmdLine = GetProcessCommandLine(pid);
if (cmdLine.find(L"--type=") == std::wstring::npos) {
    // 브라우저 프로세스!
}
```

**방법 2: 메모리** (Fallback)

```cpp
// 메모리 가장 큰 chrome.exe
```

---

## 검증 결과

### PID 기반 캡처

```
Discord (15678) → 16kHz PCM → WAV ✅
Chrome (12812) → 16kHz PCM → WAV ✅

격리:
- Discord만 캡처 ✅
- Windows 알림 무시 ✅
```

### 자동 감지

```
Discord: FindDiscordProcess() ✅
Chrome: FindChromeBrowserProcess() ✅
```

### 16kHz 변환

```
앱 출력: 48kHz
Windows: 자동 변환
캡처: 16kHz ✅

결론: SpeexDSP 불필요!
```

---

## OnVoice 적용

### 재구현 체크리스트

**참고할 패턴**:

- [x] 비동기 활성화
- [x] 캡처 스레드
- [x] 3-layer 에러
- [x] atomic 상태
- [ ] Lock-Free Queue (Phase 2)

**단순화**:

- [ ] Pause/Resume 제거
- [ ] 16kHz mono만
- [ ] Queue는 나중에

**추가**:

- [ ] COM 이벤트 (IConnectionPoint)
- [ ] SAFEARRAY 변환
- [ ] PID 유효성 검증

---

**[← 돌아가기](../learning-notes.md)**
