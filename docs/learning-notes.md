# C++ 학습 노트 📚

OnVoice COM 브리지 개발 핵심 개념 빠른 참조

**마지막 업데이트**: 2025-11-16  
**상태**: Week 0 완료 ✅

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

// 방법 1: SUCCEEDED / FAILED 매크로
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

### 1. Process-Specific Loopback

**핵심**: 특정 앱의 오디오만 캡처

```cpp
AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS params = {};
params.TargetProcessId = discordPid;  // Discord만!
params.ProcessLoopbackMode = PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE;
```

**결과**:

- ✅ Discord 소리만 캡처
- ❌ Windows 알림, 다른 앱 무시

### 2. 16kHz 자동 변환

**핵심**: Windows 오디오 엔진이 자동 변환

```cpp
WAVEFORMATEX format = {};
format.nSamplesPerSec = 16000;  // 16kHz 요청
// → Windows가 자동으로 48kHz → 16kHz 변환!
```

**결과**: SpeexDSP 불필요 ✅

### 3. 캡처 루프 패턴

```cpp
pAudioClient->Start();

while (!stopped) {
    WaitForSingleObject(hEvent, 2000);

    pCaptureClient->GetBuffer(&data, &numFrames, &flags, ...);

    // 데이터 처리
    if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {
        ProcessAudio(data, numFrames);
    }

    pCaptureClient->ReleaseBuffer(numFrames);
}

pAudioClient->Stop();
```

**[더 보기: WASAPI 상세](details/wasapi-deep-dive.md)**

---

## 🎉 Week 0 완료 요약

### 달성한 것

- ✅ COM 기초 개념 학습
- ✅ WASAPI 아키텍처 이해
- ✅ ProcessLoopbackCapture 코드 분석
- ✅ **콘솔 PoC 완성** (PID 기반 캡처)
- ✅ Chrome/Discord 프로세스 자동 감지

### 검증 완료

```
Discord (PID: 15678) → 16kHz PCM → WAV ✅
Chrome (PID: 12812) → 16kHz PCM → WAV ✅

타 앱 소리 격리: ✅
자동 감지: ✅
16kHz 변환: ✅
```

### 핵심 발견

1. **PID 기반 캡처 작동**: Discord만 캡처, 다른 앱 무시 ✅
2. **16kHz 자동 변환**: SpeexDSP 불필요 ✅
3. **Chrome 프로세스 구조**: `--type=` 플래그로 브라우저 찾기 ✅

### 시간 효율

- 투입: 6시간
- 절감: 8시간 (원래 14시간 계획)
- 남은: 44시간

**[더 보기: PoC 상세 학습](details/poc-lessons.md)**

---

## 📚 상세 문서

### COM 심화

- **[COM Deep Dive](details/com-deep-dive.md)**: IUnknown, CoCreateInstance, 스마트 포인터 등

### WASAPI 심화

- **[WASAPI Deep Dive](details/wasapi-deep-dive.md)**: 6단계 초기화, 에러 처리, 성능 최적화

### PoC 학습

- **[PoC Lessons Learned](details/poc-lessons.md)**: ProcessLoopbackCapture 패턴 5가지, 메모리 관리

---

## 🎯 다음 단계

**Week 1 시작**: Phase 1 (Visual Studio 2026 + ATL)

**필요한 지식**:

- ✅ COM 기초
- ✅ WASAPI 개념
- ✅ 레퍼런스 코드 이해
- [ ] 포인터/참조 (Phase 2에서)

**준비 완료!** 🚀

---

**문서 체계**:

```
learning-notes.md          ← 지금 여기 (빠른 참조)
├── details/
│   ├── com-deep-dive.md   (COM 상세)
│   ├── wasapi-deep-dive.md (WASAPI 상세)
│   └── poc-lessons.md     (PoC 학습)
├── build-errors.md        (에러 해결)
└── phase-progress.md      (진행 상황)
```
