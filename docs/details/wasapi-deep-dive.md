# WASAPI Deep Dive 🎵

Windows Audio Session API 상세 가이드

---

## 목차

- [WASAPI란?](#wasapi란)
- [핵심 컴포넌트 4가지](#핵심-컴포넌트-4가지)
- [초기화 6단계](#초기화-6단계)
- [Process Loopback 상세](#process-loopback-상세)
- [캡처 루프 패턴](#캡처-루프-패턴)
- [에러 처리](#에러-처리)
- [성능 최적화](#성능-최적화)

---

## WASAPI란?

**Windows Audio Session API**: Vista 이후 저수준 오디오 API

### 장점

- ✅ 낮은 지연 (DirectSound 대비 10배 빠름)
- ✅ 높은 품질 (비트 퍼펙트)
- ✅ 프로세스별 제어
- ✅ Exclusive 모드 지원

### OnVoice 사용

```
Discord/Chrome → WASAPI Loopback → 16kHz PCM → STT
```

---

## 핵심 컴포넌트 4가지

```
IMMDeviceEnumerator (디바이스 목록)
    ↓
IMMDevice (특정 디바이스)
    ↓
IAudioClient (오디오 세션)
    ↓
IAudioCaptureClient (데이터 읽기)
```

### 1. IMMDeviceEnumerator

**역할**: 오디오 디바이스 목록 관리

```cpp
IMMDeviceEnumerator* enumerator = NULL;
CoCreateInstance(__uuidof(MMDeviceEnumerator), ...);

// 기본 스피커 가져오기
IMMDevice* device = NULL;
enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
```

### 2. IMMDevice

**역할**: 특정 디바이스 표현

```cpp
// IAudioClient 활성화
IAudioClient* audioClient = NULL;
device->Activate(__uuidof(IAudioClient), CLSCTX_ALL,
                 NULL, (void**)&audioClient);
```

### 3. IAudioClient

**역할**: 오디오 세션 관리

```cpp
// 초기화
WAVEFORMATEX format = {...};
audioClient->Initialize(
    AUDCLNT_SHAREMODE_SHARED,
    AUDCLNT_STREAMFLAGS_LOOPBACK,
    0, 0, &format, NULL
);

// 시작/정지
audioClient->Start();
audioClient->Stop();
```

### 4. IAudioCaptureClient

**역할**: 실제 오디오 데이터 읽기

```cpp
IAudioCaptureClient* captureClient = NULL;
audioClient->GetService(__uuidof(IAudioCaptureClient),
                        (void**)&captureClient);

// 데이터 읽기
BYTE* data;
UINT32 numFrames;
captureClient->GetBuffer(&data, &numFrames, &flags, NULL, NULL);
// ... 처리 ...
captureClient->ReleaseBuffer(numFrames);
```

---

## 초기화 6단계

### Step 1: COM 초기화

```cpp
CoInitializeEx(NULL, COINIT_MULTITHREADED);
```

### Step 2: 디바이스 획득

```cpp
IMMDeviceEnumerator* enumerator;
CoCreateInstance(__uuidof(MMDeviceEnumerator), ...);

IMMDevice* device;
enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
```

### Step 3: IAudioClient 활성화

```cpp
// Process Loopback용 (비동기)
AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS params = {...};
ActivateAudioInterfaceAsync(...);
WaitForSingleObject(hEvent, INFINITE);
```

### Step 4: 포맷 설정

```cpp
WAVEFORMATEX format = {};
format.wFormatTag = WAVE_FORMAT_PCM;
format.nChannels = 1;          // Mono
format.nSamplesPerSec = 16000; // 16kHz
format.wBitsPerSample = 16;
format.nBlockAlign = 2;
format.nAvgBytesPerSec = 32000;

audioClient->Initialize(
    AUDCLNT_SHAREMODE_SHARED,
    AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
    0, 0, &format, NULL
);
```

### Step 5: IAudioCaptureClient 획득

```cpp
IAudioCaptureClient* captureClient;
audioClient->GetService(__uuidof(IAudioCaptureClient),
                        (void**)&captureClient);

HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
audioClient->SetEventHandle(hEvent);
```

### Step 6: 캡처 시작

```cpp
audioClient->Start();
```

---

## Process Loopback 상세

### 일반 Loopback vs Process Loopback

```
일반 Loopback:
시스템 전체 오디오 (Discord + YouTube + 알림 + ...)

Process Loopback:
특정 앱만 (Discord만!)
```

### 구조체 설정

```cpp
// 1. Process Loopback 파라미터
AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS loopbackParams = {};
loopbackParams.TargetProcessId = discordPid;
loopbackParams.ProcessLoopbackMode =
    PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE;

// 2. Activation 파라미터
AUDIOCLIENT_ACTIVATION_PARAMS activationParams = {};
activationParams.ActivationType =
    AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK;
activationParams.ProcessLoopbackParams = &loopbackParams;

// 3. PROPVARIANT 래핑
PROPVARIANT activateParams = {};
activateParams.vt = VT_BLOB;
activateParams.blob.cbSize = sizeof(activationParams);
activateParams.blob.pBlobData = (BYTE*)&activationParams;
```

### 비동기 활성화

```cpp
ActivateAudioInterfaceAsync(
    deviceIdString,
    __uuidof(IAudioClient),
    &activateParams,
    pCompletionHandler,
    &pAsyncOp
);

WaitForSingleObject(hCompletionEvent, INFINITE);
```

---

## 캡처 루프 패턴

```cpp
audioClient->Start();

while (!stopped) {
    // 이벤트 대기
    WaitForSingleObject(hCaptureEvent, 2000);

    UINT32 packetLength;
    captureClient->GetNextPacketSize(&packetLength);

    while (packetLength > 0) {
        BYTE* data;
        UINT32 numFrames;
        DWORD flags;

        captureClient->GetBuffer(&data, &numFrames, &flags, NULL, NULL);

        // 무음 아니면 처리
        if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {
            ProcessAudio(data, numFrames * blockAlign);
        }

        captureClient->ReleaseBuffer(numFrames);
        captureClient->GetNextPacketSize(&packetLength);
    }
}

audioClient->Stop();
```

### 플래그

| 플래그                                 | 값  | 의미          |
| -------------------------------------- | --- | ------------- |
| AUDCLNT_BUFFERFLAGS_SILENT             | 0x2 | 무음 구간     |
| AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY | 0x1 | 데이터 불연속 |

---

## 에러 처리

### AUDCLNT_E_DEVICE_IN_USE

```cpp
// ❌ 독점 모드
audioClient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, ...);

// ✅ 공유 모드
audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, ...);
```

### AUDCLNT_E_UNSUPPORTED_FORMAT

```cpp
// 포맷 지원 확인
WAVEFORMATEX* closestMatch;
HRESULT hr = audioClient->IsFormatSupported(
    AUDCLNT_SHAREMODE_SHARED,
    &requestedFormat,
    &closestMatch
);

if (hr == S_FALSE) {
    // closestMatch 사용
}
```

---

## 성능 최적화

### 1. 버퍼 크기

```cpp
// 10ms 버퍼 (100ns 단위)
REFERENCE_TIME bufferDuration = 100000;

audioClient->Initialize(
    AUDCLNT_SHAREMODE_SHARED,
    AUDCLNT_STREAMFLAGS_LOOPBACK,
    bufferDuration,
    0, &format, NULL
);
```

### 2. 스레드 우선순위

```cpp
DWORD taskIndex;
HANDLE hTask = AvSetMmThreadCharacteristics(TEXT("Audio"), &taskIndex);

// ... 캡처 루프 ...

AvRevertMmThreadCharacteristics(hTask);
```

### 3. Lock-Free Queue

```cpp
// 오디오 스레드
queue.enqueue(audioData);  // 빠름!

// Worker 스레드
if (queue.try_dequeue(data)) {
    SendToServer(data);  // 느린 작업
}
```

---

**[← 돌아가기](../learning-notes.md)**
