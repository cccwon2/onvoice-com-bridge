# OnVoice COM Audio Bridge 🎙️

Windows WASAPI 기반 프로세스별 오디오 캡처 COM 브리지

[![Phase](https://img.shields.io/badge/Phase-Week%201%20Complete-green)]()
[![Progress](https://img.shields.io/badge/Progress-70%25-blue)]()
[![Time](https://img.shields.io/badge/Spent-18h%20%2F%2050h-orange)]()

---

## 🎯 프로젝트 개요

**목표**: Discord, Chrome, Edge의 오디오를 선택적으로 캡처하여 Electron 앱으로 실시간 전송

**핵심 기능**:

- ✅ 기본 WASAPI 루프백 캡처 (Day 1 완료)
- ✅ 자동 프로세스 감지 (Discord/Chrome) (Week 0 완료)
- ✅ 16kHz mono PCM 자동 변환 (Week 0 검증)
- ✅ PID 기반 프로세스별 오디오 격리 (Day 2 완료) ⭐
- ✅ ATL COM DLL 프로젝트 구조 완성 (Day 3 완료) ⭐
- ✅ COM 이벤트 기반 실시간 스트리밍 (Phase 8 완료) ⭐
- ✅ 캡처 엔진 통합 및 실제 오디오 캡처 (Phase 9 완료) ⭐

**팀**: H.E.A.R. (OnVoice 프로젝트)  
**개발 기간**: 50시간 스프린트 (Week 0-3)  
**성능 목표**: 평균 latency < 3초

---

## 🏆 최신 업데이트 (2025-11-20, Week 1 완료 + E2E 테스트 성공)

### ✅ E2E 테스트 완료 (2025-11-20) 🎉

**WAV 파일 저장 테스트 성공**:

```
✅ 945개의 오디오 청크 수집
✅ 총 데이터 크기: 323,190 bytes
✅ StopCapture 정상 작동
✅ WAV 파일 저장 완료: capture_Chrome_2025-11-20_오후_25001.wav
✅ 전체 E2E 테스트 통과!
```

### ✅ Week 1 완료 내용 (2025-11-18)

**Phase 7-9: COM 브리지 완성** ⭐ 전체 통합 완료!

```
핵심 달성:
✅ OnVoiceAudioBridge ATL COM DLL 프로젝트 생성
✅ IOnVoiceCapture 인터페이스 3개 메서드 구현
✅ StartCapture(PID) / StopCapture() / GetCaptureState() 작동
✅ Phase 8: COM 이벤트 콜백 완성 (IConnectionPoint, GIT 프록시)
✅ Phase 9: AudioCaptureEngine 통합 (실제 WASAPI 캡처)
✅ 16kHz mono PCM 자동 변환 및 실시간 스트리밍
✅ VBScript 이벤트 수신 테스트 성공
✅ 상태 관리 개선 (CaptureState enum)
✅ WAV 파일 저장 E2E 테스트 성공 (2025-11-20) 🎉
```

**VBScript 이벤트 테스트 결과**:

```vbscript
' 1. COM 객체 생성 (이벤트 prefix 지정)
Set capture = WScript.CreateObject("OnVoiceAudioBridge.OnVoiceCapture", "OnVoice_")

' 2. 캡처 시작 (PID: 실제 Discord/Chrome PID)
capture.StartCapture(12345)

' 3. 이벤트 핸들러 (자동 호출됨!)
Sub OnVoice_OnAudioData(ByVal audioData)
    Dim size
    size = UBound(audioData) - LBound(audioData) + 1
    WScript.Echo "[Event] OnAudioData 수신! size=" & size & " bytes"
End Sub

' 4. 5초 대기 (이벤트 수신)
WScript.Sleep 5000

' 5. 캡처 중지
capture.StopCapture()
```

### 📊 Week 1 성과

| 항목                     | 계획    | 실제    | 상태             |
| ------------------------ | ------- | ------- | ---------------- |
| Phase 7 (COM DLL 구현)   | 4h      | 2h      | ✅ 초과 달성!    |
| Phase 8 (COM 이벤트)     | 3h      | 2h      | ✅ 초과 달성!    |
| Phase 9 (캡처 엔진 통합) | 4h      | 2h      | ✅ 초과 달성!    |
| **Week 1 누적**          | **14h** | **8h**  | **-6h 절감** ✨  |
| **전체 누적**            | **38h** | **18h** | **-20h 절감** 🎉 |

---

## 🗓️ 전체 개발 로드맵

### ✅ Week 0: PoC 완성 (T+0-6h) - 완료!

- [x] 프로젝트 초기화
- [x] COM 기초 학습
- [x] WASAPI 개념 학습
- [x] ProcessLoopbackCapture 분석
- [x] 콘솔 PoC 개발
- [x] 프로세스 자동 감지 (Discord/Chrome)
- [x] 16kHz 자동 변환 검증

**성과**:

- ✅ PID 기반 캡처 검증 완료
- ✅ 시간 절감: 8시간 (14h → 6h)

---

### 📋 Week 1: COM 브리지 기초 (T+6-20h) - 진행 중 (70% 완료)

#### ✅ Day 1 (T+6-10h) - 완료!

- [x] **Phase 1**: Visual Studio 2026 + ATL 설치 및 설정
  - [x] VS 2026 Community 설치
  - [x] ATL 구성 요소 설치
  - [x] HelloCOM 프로젝트 빌드 성공
- [x] **Phase 2**: C++ 기초 (포인터/참조)
  - [x] CppBasics 프로젝트 - 포인터 실습
  - [x] COMBasics 프로젝트 - COM 기본 실습
  - [x] COM 개념: AddRef/Release, HRESULT
- [x] **Phase 3.1**: WASAPI 기본 캡처
  - [x] AudioCapture 프로젝트 생성
  - [x] 루프백 모드 캡처 성공
  - [x] 48kHz 스테레오 240,000 프레임 캡처

#### ✅ Day 2 (T+10-12h) - 완료! ⭐

- [x] **Phase 4**: PID 기반 캡처 (가장 어려운 단계!)
  - [x] `AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS` 구조체 설정
  - [x] `ActivateAudioInterfaceAsync` API 구현
  - [x] 비동기 완료 핸들러 (`IActivateAudioInterfaceCompletionHandler`)
  - [x] `IAgileObject` 추가로 MTA 안정성 확보
  - [x] Chrome PID로 선택적 캡처 검증
  - [x] 참조 카운팅 정상 동작 확인

**실제 소요**: 2시간 (계획 3h 대비 -1h) 🎉

#### ✅ Day 3-4 (T+12-18h) - 완료! ⭐ 신규!

- [x] **Phase 7**: ATL COM DLL 프로젝트

  - [x] OnVoiceAudioBridge 프로젝트 생성
  - [x] IDL 인터페이스 정의 (IOnVoiceCapture)
  - [x] 3개 메서드 구현 (StartCapture, StopCapture, GetCaptureState)
  - [x] 상태 관리 개선 (CaptureState enum)
  - [x] VBScript 테스트 스크립트 작성

- [x] **Phase 8**: COM 이벤트 콜백

  - [x] IDL에 `_IOnVoiceCaptureEvents` 인터페이스 정의
  - [x] `IConnectionPoint` / `IConnectionPointContainer` 구현
  - [x] GIT 프록시를 통한 스레드 간 안전한 이벤트 전송
  - [x] `Fire_OnAudioData()` 헬퍼 함수 구현
  - [x] VBScript 이벤트 수신 테스트 성공

- [x] **Phase 9**: 캡처 엔진 통합
  - [x] `AudioCaptureEngine` 클래스 구현
  - [x] `ProcessLoopbackCapture` 래핑
  - [x] 실제 WASAPI 캡처 통합
  - [x] 16kHz mono PCM 자동 변환
  - [x] 실시간 오디오 데이터 스트리밍

**실제 소요**: 6시간 (계획 11h 대비 -5h) 🎉

#### ✅ Week 1 완료! (T+6-18h)

**완료된 Phase**:

- ✅ Phase 1-3.1: VS 설정, C++ 기초, WASAPI 기본 캡처
- ✅ Phase 4: PID 기반 캡처
- ✅ Phase 7: ATL COM DLL 프로젝트
- ✅ Phase 8: COM 이벤트 콜백
- ✅ Phase 9: 캡처 엔진 통합

**Week 1 실제 소요**: 8시간 (계획 14h 대비 -6h) ✨

---

### 📋 Week 2: COM DLL 및 Electron 연동 (T+18-40h) - 계획

#### ✅ Day 3-4: COM 이벤트 및 캡처 통합 (완료!)

- [x] **Phase 8**: COM 이벤트 콜백

  - [x] IDL에 이벤트 인터페이스 정의 (`_IOnVoiceCaptureEvents`)
  - [x] `IConnectionPoint` / `IConnectionPointContainer` 구현
  - [x] OnAudioData(SAFEARRAY\*) 이벤트
  - [x] GIT 프록시를 통한 스레드 간 안전한 전송
  - [x] VBScript 이벤트 수신 테스트 성공

- [x] **Phase 9**: 캡처 엔진 통합
  - [x] `AudioCaptureEngine` 클래스 구현
  - [x] `ProcessLoopbackCapture` 래핑
  - [x] StartCapture에서 실제 WASAPI 캡처 시작
  - [x] OnAudioData 이벤트로 데이터 전송
  - [x] 16kHz mono PCM 자동 변환

#### Day 7: Electron 연동

- [ ] **Phase 10-12**: winax + Electron
  - [ ] winax 설치 및 재빌드
  - [ ] Electron Main 프로세스 연동
  - [ ] Renderer 프로세스로 데이터 전송

**예상 소요**: 18-22시간

---

### 📋 Week 3: 테스트 및 완성 (T+40-50h) - 계획

- [ ] **Phase 13**: 다중 프로세스 테스트

  - [ ] Discord/Chrome/Edge 동시 캡처
  - [ ] 프로세스 격리 검증

- [ ] **Phase 14**: 안정화
  - [ ] 크래시 시나리오 테스트
  - [ ] 메모리 누수 검사 (1시간 연속 실행)
  - [ ] 최종 E2E 테스트

**예상 소요**: 10시간

---

## 📂 프로젝트 구조

```
onvoice-com-bridge/
├── docs/                           # 📚 문서
│   ├── learning-notes.md           # 빠른 참조 + 핵심 개념 학습 ✅
│   ├── phase-progress.md           # 기능별 구현 현황 ✅
│   ├── phases/                     # 기능별 구현 상세
│   │   ├── com-interface.md        # COM 인터페이스 구현 ⭐
│   │   ├── event-system.md         # 이벤트 시스템 구현 ⭐
│   │   ├── capture-engine.md       # 캡처 엔진 구현 ⭐
│   │   ├── testing.md              # 테스트 및 검증 ⭐
│   │   ├── week0-poc.md            # PoC 완성 (참고용)
│   │   └── week1-com-bridge.md     # Week 1 상세 (참고용)
│   ├── details/                    # 심화 학습
│   │   ├── com-deep-dive.md        # COM 상세
│   │   ├── wasapi-deep-dive.md     # WASAPI 상세
│   │   └── poc-lessons.md          # PoC 학습
│   └── build-errors.md             # 에러 해결
│
├── phase1-console/                 # 🧪 PoC 및 학습 (Week 0 완료) ✅
│   ├── reference/                  # ProcessLoopbackCapture 원본 참조
│   │   ├── ProcessLoopbackCapture.cpp/h  # 원본 구현
│   │   ├── examples/simple_recorder/     # 예제 코드
│   │   └── README.md                     # 참조 문서
│   └── AudioCaptureTest/          # 콘솔 PoC 프로젝트
│       ├── AudioCaptureTest.cpp   # 메인 프로그램
│       ├── ProcessLoopbackCapture.cpp/h  # PID 기반 캡처 구현
│       ├── AudioCaptureTest.vcxproj      # 프로젝트 파일
│       ├── captured_audio.wav     # 캡처된 오디오 (출력)
│       └── x64/Debug/
│           └── AudioCaptureTest.exe
│
├── phase2-learning/                # 📖 Day 1-2 학습 프로젝트 ✅
│   ├── HelloCOM/                   # ATL DLL 템플릿
│   ├── CppBasics/                  # 포인터/참조 실습
│   ├── COMBasics/                  # COM 기본 실습
│   ├── AudioCapture/               # WASAPI 루프백 캡처 (Day 1)
│   └── AudioCapturePID/            # PID 기반 캡처 (Day 2) ⭐
│
├── phase3-com-dll/                 # 🔧 COM DLL (Week 1 완료) ⭐
│   └── OnVoiceAudioBridge/         # ATL COM DLL 프로젝트 (Day 3-4) ✅
│       ├── OnVoiceAudioBridge.idl  # 인터페이스 정의
│       ├── OnVoiceCapture.h/.cpp    # 구현
│       ├── AudioCaptureEngine.h/.cpp  # 캡처 엔진
│       ├── TestOnVoiceEvents.vbs    # 이벤트 수신 테스트
│       ├── TestPidCapture.vbs        # PID 캡처 테스트
│       ├── TestAudioCaptureEngine.vbs  # 엔진 연동 테스트
│       └── x64/Debug/
│           └── OnVoiceAudioBridge.dll
│
├── phase4-electron/                # ⚡ Electron 연동 (Week 2 예정)
│   └── test-winax/
│
└── scripts/                        # 🤖 유틸리티
    ├── build.bat
    └── get-pid.ps1
```

---

## 🚀 빌드 및 실행

### Day 3 프로젝트 - 현재 가능 ✅ 신규!

#### OnVoiceAudioBridge (ATL COM DLL) ⭐

```bash
1. Visual Studio 2026 열기
2. phase3-com-dll/OnVoiceAudioBridge/OnVoiceAudioBridge.sln 열기
3. Ctrl+Shift+B (빌드)
4. 출력 확인:
   ========== 빌드: 성공 1 ==========
   x64\Debug\OnVoiceAudioBridge.dll 생성됨
```

**VBScript 테스트 실행**:

```powershell
# PowerShell에서 실행
cd phase3-com-dll\OnVoiceAudioBridge\x64\Debug

# 1. 이벤트 수신 테스트 (권장)
C:\Windows\System32\cscript.exe //nologo ..\..\TestOnVoiceEvents.vbs [PID]

# 2. PID 캡처 테스트
C:\Windows\System32\cscript.exe //nologo ..\..\TestPidCapture.vbs

# 3. 엔진 연동 테스트
C:\Windows\System32\cscript.exe //nologo ..\..\TestAudioCaptureEngine.vbs

# 예상 출력 (이벤트 테스트):
# =========================================
# OnVoiceCapture 이벤트 테스트 (GIT 버전)
# =========================================
# [1] 이벤트 Prefix 포함해서 COM 객체 생성...
# OK: COM 객체 생성 완료
# [2] 초기 상태: 0 (0=Stopped,1=Starting,2=Capturing,3=Stopping)
# [3] StartCapture(12345) 호출...
# StartCapture 성공 (HR=0)
# [Event] OnAudioData 수신! size=3200 bytes
# [Event] OnAudioData 수신! size=3200 bytes
# ...
```

### Day 2 프로젝트 - 가능 ✅

#### AudioCapturePID (PID 기반 캡처) ⭐

```bash
1. Visual Studio 2026 열기
2. phase2-learning/AudioCapturePID/AudioCapturePID.sln 열기
3. AudioCapture.cpp에서 PID 수정:
   DWORD targetPid = 21616;  // 실제 Chrome/Discord PID
4. Ctrl+F5 (실행)
5. 결과 확인:
   - 비동기 활성화 성공
   - IAudioClient 획득 성공
   - 참조 카운팅 정상 (AddRef/Release)
   - PID의 오디오만 캡처 가능
```

### Day 1 학습 프로젝트 - 가능 ✅

#### HelloCOM (ATL DLL 템플릿)

```bash
1. Visual Studio 2026 열기
2. phase2-learning/HelloCOM/HelloCOM.sln 열기
3. Ctrl+Shift+B (빌드)
4. x64/Debug/HelloCOM.dll 생성 확인
```

---

### Week 0 PoC (콘솔 앱) - 가능 ✅

```bash
# Visual Studio에서 열기
cd phase1-console/AudioCaptureTest
start AudioCaptureTest.slnx

# 빌드 후 실행
1. Ctrl + F5 (실행)
2. 옵션 선택:
   1. Discord (자동 감지)
   2. Chrome (자동 감지)
   3. 수동 PID 입력
3. 10초간 오디오 재생
4. captured_audio.wav 확인
```

---

## 🛠️ 기술 스택

### C++ 레이어

- **개발 환경**: Visual Studio Community 2026
- **프로젝트 타입**: ATL COM In-Process DLL ⭐
- **API**: Windows WASAPI (Loopback + Process-Specific)
  - `ActivateAudioInterfaceAsync` (비동기 활성화) ✅
  - `AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS` (PID 지정) ✅
  - `IActivateAudioInterfaceCompletionHandler` (완료 콜백) ✅
- **COM**: ATL, `IDispatch`, `IConnectionPoint` (예정)
- **참조 구현**: [ProcessLoopbackCapture](https://github.com/Naseband/ProcessLoopbackCapture)
- **링커 라이브러리**: ole32.lib, oleaut32.lib, mmdevapi.lib, avrt.lib, mfplat.lib

### Electron 레이어 (예정)

- **Runtime**: Node.js + React.js
- **COM 클라이언트**: winax
- **IPC**: Electron Main ↔ Renderer

### FastAPI 서버 (기존 구현)

- **STT**: Deepgram + Whisper (fallback)
- **OCR**: PaddleOCR (한국어 특화)
- **AI**: KoELECTRA (유해 표현 탐지)

---

## 📖 학습 자료

### 빠른 시작

- **[learning-notes.md](docs/learning-notes.md)**: 빠른 참조 + 핵심 개념 학습 내용
- **[phase-progress.md](docs/phase-progress.md)**: 기능별 구현 현황 및 진행 상황

### 기능별 구현 상세

- **[COM 인터페이스 구현](docs/phases/com-interface.md)**: IOnVoiceCapture 인터페이스 상세
- **[이벤트 시스템 구현](docs/phases/event-system.md)**: IConnectionPoint, GIT 프록시 구현
- **[캡처 엔진 구현](docs/phases/capture-engine.md)**: AudioCaptureEngine, ProcessLoopbackCapture
- **[테스트 및 검증](docs/phases/testing.md)**: VBScript 테스트 스크립트 및 검증 결과

### 심화 학습

- **[COM Deep Dive](docs/details/com-deep-dive.md)**: IUnknown, 참조 카운팅, 스마트 포인터
- **[WASAPI Deep Dive](docs/details/wasapi-deep-dive.md)**: 6단계 초기화, 에러 처리, 최적화
- **[PoC Lessons](docs/details/poc-lessons.md)**: ProcessLoopbackCapture 패턴 5가지

### 트러블슈팅

- **[build-errors.md](docs/build-errors.md)**: 빌드 에러 해결 (regsvr32, 링커 에러 등)

---

## 📊 진행 현황

### 전체 진행률

```
[████████████░░░░] 70% (18h / 50h)

Week 0: ████████████ 100% (6h)
Week 1: ████████████ 100% (8h / 14h) ⭐ 완료!
Week 2: ░░░░░░░░░░░░ 0% (0h / 22h)
Week 3: ░░░░░░░░░░░░ 0% (0h / 10h)
```

### 마일스톤

| 마일스톤              | 상태         | 날짜           |
| --------------------- | ------------ | -------------- |
| Week 0 PoC 완성       | ✅ 완료      | 2025-11-16     |
| Day 1 학습 완료       | ✅ 완료      | 2025-11-17     |
| Day 2 PID 캡처        | ✅ 완료      | 2025-11-18     |
| **Week 1 COM 브리지** | ✅ **완료**  | **2025-11-18** |
| Week 2 Electron 연동  | ⏳ 진행 예정 | 2025-11-19     |
| Week 3 MVP 완성       | 📅 예정      | 2025-12-04     |

### 시간 효율

| 항목             | 계획 | 실제 | 차이         |
| ---------------- | ---- | ---- | ------------ |
| Week 0           | 14h  | 6h   | **-8h** ✨   |
| Week 1 (Day 1)   | 6h   | 4h   | **-2h** ✨   |
| Week 1 (Day 2)   | 8h   | 2h   | **-6h** ✨   |
| Week 1 (Day 3-4) | 11h  | 6h   | **-5h** ✨   |
| **누적 절감**    | 39h  | 18h  | **-21h** 🎉  |
| **남은 예산**    | 50h  | 32h  | -            |
| **효율성**       | -    | -    | **54% 향상** |

---

## 🎯 다음 단계 (Week 2)

### Phase 10-12: Electron 연동 (최우선) ⭐

**목표**: COM DLL → Electron Main → Renderer 실시간 전송

**예상 소요**: 18-22시간  
**난이도**: ⭐⭐⭐⭐ 매우 어려움

**핵심 작업**:

1. winax 설치 및 재빌드
2. Electron Main 프로세스에서 COM 객체 생성
3. 이벤트 수신 및 Renderer로 전송
4. IPC 통신 구현
5. E2E 테스트

**테스트 목표**:

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

---

## 🎉 주요 성과

### Week 0 (2025-11-16)

- ✅ PID 기반 오디오 캡처 검증 완료
- ✅ 프로세스 자동 감지 (Discord/Chrome)
- ✅ 16kHz 자동 변환 확인 (SpeexDSP 불필요)
- ✅ 콘솔 PoC 완성

### Day 1 (2025-11-17)

- ✅ Visual Studio 2026 + ATL 환경 구축
- ✅ C++ 포인터/참조 학습 완료
- ✅ COM 기본 개념 습득 (AddRef/Release, HRESULT)
- ✅ **WASAPI 루프백 캡처 성공**
  - 48kHz 스테레오, 240,000 프레임
  - 500개 패킷 실시간 수신
  - 실제 오디오 데이터 검증 완료

### Day 2 (2025-11-18) ⭐

- ✅ **PID 기반 오디오 캡처 성공** (가장 어려운 단계!)
- ✅ `ActivateAudioInterfaceAsync` 비동기 API 구현
- ✅ `IActivateAudioInterfaceCompletionHandler` 완료 핸들러
- ✅ `IAgileObject` 추가로 MTA 안정성 확보
- ✅ Chrome PID (21616)로 선택적 캡처 검증
- ✅ 참조 카운팅 정상 동작 (AddRef: 1→2→3, Release: 2→1→0)
- ✅ 메모리 정리 완료 (핸들러 소멸자 실행)

### Day 3-4 (2025-11-18) ⭐

- ✅ **ATL COM DLL 프로젝트 구조 완성**
- ✅ `IOnVoiceCapture` 인터페이스 3개 메서드 구현
- ✅ `StartCapture(PID)` / `StopCapture()` / `GetCaptureState()` 작동
- ✅ **Phase 8: COM 이벤트 콜백 완성**
  - `_IOnVoiceCaptureEvents` 인터페이스 정의
  - `IConnectionPoint` / `IConnectionPointContainer` 구현
  - GIT 프록시를 통한 스레드 간 안전한 이벤트 전송
  - `Fire_OnAudioData()` 헬퍼 함수 구현
- ✅ **Phase 9: 캡처 엔진 통합**
  - `AudioCaptureEngine` 클래스 구현
  - `ProcessLoopbackCapture` 래핑
  - 실제 WASAPI 캡처 통합
  - 16kHz mono PCM 자동 변환
- ✅ **VBScript 이벤트 테스트 성공**
  - COM 객체 생성 및 이벤트 연결 성공
  - 실시간 오디오 데이터 수신 확인
  - 상태 전환 확인 (Stopped → Starting → Capturing → Stopped)

### E2E 테스트 (2025-11-20) 🎉 신규!

- ✅ **WAV 파일 저장 E2E 테스트 성공**
  - 945개 오디오 청크 수집
  - 총 323,190 bytes 데이터 캡처
  - WAV 파일 저장 및 재생 검증 완료
  - 전체 E2E 테스트 통과!

**검증 결과**:

```
✅ CreateObject("OnVoiceAudioBridge.OnVoiceCapture", "OnVoice_") 성공
✅ StartCapture(PID) → 실제 오디오 캡처 시작
✅ OnVoice_OnAudioData 이벤트 수신 성공
✅ 16kHz mono PCM 데이터 실시간 전송 확인
✅ StopCapture() → 정상 중지
✅ 모든 테스트 통과!
```

**E2E 테스트 결과 (2025-11-20)** 🎉:

```
✅ 945개의 오디오 청크 수집 성공
✅ 총 데이터 크기: 323,190 bytes
✅ WAV 파일 저장 완료: capture_Chrome_2025-11-20_오후_25001.wav
✅ 전체 E2E 테스트 통과!
```

### 학습한 프로젝트 (6개)

1. **HelloCOM** - ATL DLL 템플릿 이해
2. **CppBasics** - 포인터와 참조 실습
3. **COMBasics** - COM 디바이스 정보 가져오기
4. **AudioCapture** - WASAPI 루프백 캡처 (Day 1)
5. **AudioCapturePID** - PID 기반 선택적 캡처 (Day 2) ⭐
6. **OnVoiceAudioBridge** - ATL COM DLL 프로젝트 (Day 3-4) ⭐ 완성!
   - COM 이벤트 콜백 구현
   - 실제 오디오 캡처 엔진 통합

---

## 🔗 참고 자료

### 공식 문서

- [Microsoft WASAPI](https://learn.microsoft.com/en-us/windows/win32/coreaudio/wasapi)
- [ATL COM Tutorial](https://learn.microsoft.com/en-us/cpp/atl/)
- [ProcessLoopbackCapture GitHub](https://github.com/Naseband/ProcessLoopbackCapture)
- [ActivateAudioInterfaceAsync](https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-activateaudiointerfaceasync)

### 프로젝트 문서

**기능별 구현 상세**:
- [COM 인터페이스 구현](docs/phases/com-interface.md) - IOnVoiceCapture 상세
- [이벤트 시스템 구현](docs/phases/event-system.md) - IConnectionPoint, GIT 프록시
- [캡처 엔진 구현](docs/phases/capture-engine.md) - AudioCaptureEngine 상세
- [테스트 및 검증](docs/phases/testing.md) - VBScript 테스트 스크립트

**학습 자료**:
- [learning-notes.md](docs/learning-notes.md) - 빠른 참조 + 핵심 개념 학습
- [phase-progress.md](docs/phase-progress.md) - 기능별 구현 현황
- [COM Deep Dive](docs/details/com-deep-dive.md) - COM 상세
- [WASAPI Deep Dive](docs/details/wasapi-deep-dive.md) - WASAPI 상세

---

**마지막 업데이트**: 2025-11-20 (E2E 테스트 성공)  
**개발자**: 김원 (H.E.A.R. Team)  
**현재 상태**: Week 1 완료 ✅ + E2E 테스트 성공 🎉 → Week 2 (Electron 연동) 준비 중 🚀

---

## 📝 변경 이력

### 2025-11-20 (E2E 테스트 성공) 🎉 신규!

- ✅ **WAV 파일 저장 E2E 테스트 완료!**
- ✅ 945개 오디오 청크 수집 성공
- ✅ 총 323,190 bytes 데이터 캡처
- ✅ WAV 파일 저장 및 검증 완료
- ✅ 전체 E2E 테스트 통과

### 2025-11-18 (Week 1 완료) ⭐

- ✅ **Phase 7-9 완료 - COM 브리지 전체 통합 성공!**
- ✅ Phase 7: OnVoiceAudioBridge ATL COM DLL 프로젝트 생성
- ✅ Phase 8: COM 이벤트 콜백 구현 (IConnectionPoint, GIT 프록시)
- ✅ Phase 9: AudioCaptureEngine 통합 및 실제 WASAPI 캡처
- ✅ 16kHz mono PCM 자동 변환 및 실시간 스트리밍
- ✅ VBScript 이벤트 수신 테스트 성공
- 📊 진행률: 56% → 70%
- ⏱️ 시간 절감: 누적 21시간 (계획 39h → 실제 18h)

### 2025-11-18 (Day 2)

- ✅ **Phase 4 완료 - PID 기반 캡처 성공!**
- ✅ ActivateAudioInterfaceAsync 비동기 API 구현
- ✅ IActivateAudioInterfaceCompletionHandler + IAgileObject
- ✅ VIRTUAL_AUDIO_DEVICE_PROCESS_LOOPBACK 사용
- ✅ Chrome PID 선택적 캡처 검증
- ✅ 참조 카운팅 정상 동작 확인
- 📊 진행률: 40% → 50%
- ⏱️ 시간 절감: 누적 16시간

### 2025-11-17 (Day 1)

- ✅ Visual Studio 2026 + ATL 설정 완료
- ✅ C++ 기초 학습 (포인터, 참조, COM)
- ✅ WASAPI 루프백 캡처 성공
- ✅ 4개 학습 프로젝트 완성
- 📊 진행률: 12% → 40%
- ⏱️ 시간 절감: 누적 10시간

### 2025-11-16 (Week 0)

- ✅ ProcessLoopbackCapture 분석
- ✅ 콘솔 PoC 완성
- ✅ PID 기반 캡처 검증
- ✅ 16kHz 자동 변환 확인
