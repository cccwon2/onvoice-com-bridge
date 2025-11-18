# OnVoice COM Audio Bridge 🎙️

Windows WASAPI 기반 프로세스별 오디오 캡처 COM 브리지

[![Phase](https://img.shields.io/badge/Phase-Week%201%20Day%203%20Complete-green)]()
[![Progress](https://img.shields.io/badge/Progress-56%25-blue)]()
[![Time](https://img.shields.io/badge/Spent-14h%20%2F%2050h-orange)]()

---

## 🎯 프로젝트 개요

**목표**: Discord, Chrome, Edge의 오디오를 선택적으로 캡처하여 Electron 앱으로 실시간 전송

**핵심 기능**:

- ✅ 기본 WASAPI 루프백 캡처 (Day 1 완료)
- ✅ 자동 프로세스 감지 (Discord/Chrome) (Week 0 완료)
- ✅ 16kHz mono PCM 자동 변환 (Week 0 검증)
- ✅ PID 기반 프로세스별 오디오 격리 (Day 2 완료) ⭐
- ✅ ATL COM DLL 프로젝트 구조 완성 (Day 3 완료) ⭐
- ⏳ COM 이벤트 기반 실시간 스트리밍 (Week 2 예정)

**팀**: H.E.A.R. (OnVoice 프로젝트)  
**개발 기간**: 50시간 스프린트 (Week 0-3)  
**성능 목표**: 평균 latency < 3초

---

## 🏆 최신 업데이트 (2025-11-18, Day 3)

### ✅ Day 3 완료 내용

**Phase 7: ATL COM 프로젝트 완성** ⭐ VBScript 테스트 성공!

```
핵심 달성:
✅ OnVoiceAudioBridge ATL COM DLL 프로젝트 생성
✅ IOnVoiceCapture 인터페이스 3개 메서드 구현
✅ StartCapture(PID) / StopCapture() / GetCaptureState() 작동
✅ 상태 관리 (m_bIsCapturing, m_targetPid) 정상 동작
✅ VBScript 테스트 완벽 통과
✅ COM 객체 생성 및 호출 검증 완료
```

**VBScript 테스트 결과**:

```vbscript
' 1. COM 객체 생성
Set capture = CreateObject("OnVoiceAudioBridge.OnVoiceCapture")

' 2. 초기 상태 확인 → 0 (중지)
WScript.Echo "초기 상태: " & capture.GetCaptureState()

' 3. 캡처 시작 (PID: 12345)
capture.StartCapture(12345)
WScript.Echo "현재 상태: " & capture.GetCaptureState()  ' → 1 (실행 중)

' 4. 캡처 중지
capture.StopCapture()
WScript.Echo "최종 상태: " & capture.GetCaptureState()  ' → 0 (중지)
```

### 📊 Day 3 성과

| 항목                   | 계획    | 실제    | 상태             |
| ---------------------- | ------- | ------- | ---------------- |
| Phase 7 (COM DLL 구현) | 4h      | 2h      | ✅ 초과 달성!    |
| **누적**               | **30h** | **14h** | **-16h 절감** ✨ |

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

#### ✅ Day 3 (T+12-14h) - 완료! ⭐ 신규!

- [x] **Phase 7**: ATL COM DLL 프로젝트
  - [x] OnVoiceAudioBridge 프로젝트 생성
  - [x] IDL 인터페이스 정의 (IOnVoiceCapture)
  - [x] 3개 메서드 구현 (StartCapture, StopCapture, GetCaptureState)
  - [x] 상태 관리 멤버 변수 (m_bIsCapturing, m_targetPid)
  - [x] VBScript 테스트 스크립트 작성
  - [x] COM 객체 생성 및 호출 검증

**실제 소요**: 2시간 (계획 4h 대비 -2h) 🎉

#### 📋 Day 4 (T+14-18h) - 계획

- [ ] **Phase 5**: 리소스 누수 수정 (우선)

  - [ ] MS 샘플 버그 수정 (참조 카운트)
  - [ ] CoTaskMemFree 누락 수정
  - [ ] 100회 시작/중지 테스트

- [ ] **Phase 6**: SpeexDSP 리샘플링 (선택 사항)
  - [ ] 48kHz Float → 16kHz Int16 변환
  - [ ] (Week 0에서 불필요 확인됨 → 건너뛸 수도 있음)

**예상 소요**: 1-2시간

---

### 📋 Week 2: COM DLL 및 Electron 연동 (T+18-40h) - 계획

#### Day 5-6: COM 이벤트 및 캡처 통합

- [ ] **Phase 8**: COM 이벤트 콜백

  - [ ] IDL에 이벤트 인터페이스 정의
  - [ ] `IConnectionPoint` 구현
  - [ ] OnAudioData(SAFEARRAY\*) 이벤트
  - [ ] SAFEARRAY로 오디오 데이터 전송

- [ ] **Phase 9**: 캡처 엔진 통합
  - [ ] Phase 4 코드를 COM DLL로 이식
  - [ ] StartCapture에서 실제 WASAPI 캡처 시작
  - [ ] OnAudioData 이벤트로 데이터 전송
  - [ ] VBScript로 이벤트 수신 테스트

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
│   ├── learning-notes.md           # 빠른 참조 + Day 1-3 학습 내용 ✅
│   ├── details/                    # 상세 문서
│   │   ├── com-deep-dive.md        # COM 상세
│   │   ├── wasapi-deep-dive.md     # WASAPI 상세
│   │   └── poc-lessons.md          # PoC 학습
├── phases/                         # 진행 상황 상세 문서
│   │   ├── week0-poc.md            # PoC 완성 및 검증 상세
│   │   ├── week1-com-bridge.md     # COM WASAPI bridge 상세
│   ├── build-errors.md             # 에러 해결
│   └── phase-progress.md           # 진행 상황 ✅
│
├── phase1-console/                 # 🧪 PoC 및 학습
│   ├── reference/                  # ProcessLoopbackCapture 원본 ✅
│   └── poc/                        # 콘솔 PoC (Week 0 완료) ✅
│
├── phase2-learning/                # 📖 Day 1-2 학습 프로젝트 ✅
│   ├── HelloCOM/                   # ATL DLL 템플릿
│   ├── CppBasics/                  # 포인터/참조 실습
│   ├── COMBasics/                  # COM 기본 실습
│   ├── AudioCapture/               # WASAPI 루프백 캡처 (Day 1)
│   └── AudioCapturePID/            # PID 기반 캡처 (Day 2) ⭐
│
├── phase3-com-dll/                 # 🔧 COM DLL (Week 1 진행 중) ⭐
│   └── OnVoiceAudioBridge/         # ATL COM DLL 프로젝트 (Day 3) ✅
│       ├── OnVoiceAudioBridge.idl  # 인터페이스 정의
│       ├── OnVoiceCapture.h/.cpp   # 구현
│       └── x64/Debug/
│           ├── OnVoiceAudioBridge.dll
│           └── TestOnVoiceCapture.vbs  # VBScript 테스트
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
C:\Windows\System32\cscript.exe //nologo TestOnVoiceCapture.vbs

# 예상 출력:
# ==========================================
# OnVoice COM 브리지 테스트 시작!
# ==========================================
# [1단계] COM 객체 생성 중...
# [OK] COM 객체 생성 성공!
# [2단계] 초기 상태 확인 중...
# 초기 상태: 0 (0=중지, 1=실행 중)
# [OK] 예상대로 중지 상태입니다!
# ...
# 모든 테스트 완료!
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
cd phase1-console/poc
start AudioCaptureTest.sln

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

- **[learning-notes.md](docs/learning-notes.md)**: 빠른 참조 + Day 1-3 학습 내용

### 상세 문서

- **[COM Deep Dive](docs/details/com-deep-dive.md)**: IUnknown, 참조 카운팅, 스마트 포인터
- **[WASAPI Deep Dive](docs/details/wasapi-deep-dive.md)**: 6단계 초기화, 에러 처리, 최적화
- **[PoC Lessons](docs/details/poc-lessons.md)**: ProcessLoopbackCapture 패턴 5가지

### 트러블슈팅

- **[build-errors.md](docs/build-errors.md)**: 빌드 에러 해결 (regsvr32, 링커 에러 등)
- **[phase-progress.md](docs/phase-progress.md)**: Phase별 상세 진행 기록

---

## 📊 진행 현황

### 전체 진행률

```
[███████████░░░░░] 56% (14h / 50h)

Week 0: ████████████ 100% (6h)
Week 1: ██████████░░ 70% (8h / 14h) ⭐
Week 2: ░░░░░░░░░░░░ 0% (0h / 22h)
Week 3: ░░░░░░░░░░░░ 0% (0h / 10h)
```

### 마일스톤

| 마일스톤             | 상태         | 날짜           |
| -------------------- | ------------ | -------------- |
| Week 0 PoC 완성      | ✅ 완료      | 2025-11-16     |
| Day 1 학습 완료      | ✅ 완료      | 2025-11-17     |
| Day 2 PID 캡처       | ✅ 완료      | 2025-11-18     |
| **Day 3 COM DLL**    | ✅ **완료**  | **2025-11-18** |
| Day 4 리소스 관리    | ⏳ 진행 예정 | 2025-11-19     |
| Week 1 COM 기초      | ⏳ 진행 중   | 2025-11-20     |
| Week 2 Electron 연동 | 📅 예정      | 2025-11-27     |
| Week 3 MVP 완성      | 📅 예정      | 2025-12-04     |

### 시간 효율

| 항목           | 계획 | 실제 | 차이         |
| -------------- | ---- | ---- | ------------ |
| Week 0         | 14h  | 6h   | **-8h** ✨   |
| Week 1 (Day 1) | 6h   | 4h   | **-2h** ✨   |
| Week 1 (Day 2) | 8h   | 2h   | **-6h** ✨   |
| Week 1 (Day 3) | 10h  | 2h   | **-8h** ✨   |
| **누적 절감**  | 38h  | 14h  | **-24h** 🎉  |
| **남은 예산**  | 50h  | 36h  | -            |
| **효율성**     | -    | -    | **48% 향상** |

---

## 🎯 다음 단계 (Day 4)

### Phase 8: COM 이벤트 콜백 (최우선) ⭐

**목표**: COM → Electron 이벤트 전송 구현

**예상 소요**: 2-3시간  
**난이도**: ⭐⭐⭐⭐ 매우 어려움 (COM의 가장 복잡한 부분!)

**핵심 작업**:

1. IDL에 이벤트 인터페이스 정의 (`_IOnVoiceCaptureEvents`)
2. `IConnectionPoint` / `IConnectionPointContainer` 구현
3. `FireOnAudioData()` 헬퍼 함수
4. VBScript 이벤트 수신 테스트

**테스트 목표**:

```vbscript
' VBScript에서 이벤트 수신
Set capture = CreateObject("OnVoiceAudioBridge.OnVoiceCapture")
WScript.ConnectObject capture, "OnVoice_"

capture.StartCapture(12345)

' 이벤트 핸들러 (자동 호출됨)
Sub OnVoice_OnAudioData(data, dataSize)
    WScript.Echo "오디오 데이터 수신: " & dataSize & " bytes"
End Sub
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

### Day 3 (2025-11-18) ⭐ 신규!

- ✅ **ATL COM DLL 프로젝트 구조 완성**
- ✅ `IOnVoiceCapture` 인터페이스 3개 메서드 구현
- ✅ `StartCapture(PID)` / `StopCapture()` / `GetCaptureState()` 작동
- ✅ 상태 관리 멤버 변수 정상 동작
- ✅ **VBScript 테스트 완벽 통과**
  - COM 객체 생성 성공
  - 메서드 호출 정상
  - 상태 전환 확인 (0 → 1 → 0)

**검증 결과**:

```
✅ CreateObject("OnVoiceAudioBridge.OnVoiceCapture") 성공
✅ 초기 상태: 0 (중지)
✅ StartCapture(12345) → 상태: 1 (실행 중)
✅ StopCapture() → 상태: 0 (중지)
✅ 모든 테스트 통과!
```

### 학습한 프로젝트 (6개)

1. **HelloCOM** - ATL DLL 템플릿 이해
2. **CppBasics** - 포인터와 참조 실습
3. **COMBasics** - COM 디바이스 정보 가져오기
4. **AudioCapture** - WASAPI 루프백 캡처 (Day 1)
5. **AudioCapturePID** - PID 기반 선택적 캡처 (Day 2) ⭐
6. **OnVoiceAudioBridge** - ATL COM DLL 프로젝트 (Day 3) ⭐ 신규!

---

## 🔗 참고 자료

### 공식 문서

- [Microsoft WASAPI](https://learn.microsoft.com/en-us/windows/win32/coreaudio/wasapi)
- [ATL COM Tutorial](https://learn.microsoft.com/en-us/cpp/atl/)
- [ProcessLoopbackCapture GitHub](https://github.com/Naseband/ProcessLoopbackCapture)
- [ActivateAudioInterfaceAsync](https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-activateaudiointerfaceasync)

### 학습 자료

- [learning-notes.md](docs/learning-notes.md) - 빠른 참조 + Day 1-3 학습
- [COM Deep Dive](docs/details/com-deep-dive.md) - COM 상세
- [WASAPI Deep Dive](docs/details/wasapi-deep-dive.md) - WASAPI 상세

---

**마지막 업데이트**: 2025-11-18 (Day 3 완료)  
**개발자**: 김원 (H.E.A.R. Team)  
**현재 상태**: Week 1 Day 3 완료 ✅ → Day 4 준비 중 🚀

---

## 📝 변경 이력

### 2025-11-18 (Day 3) ⭐ 신규!

- ✅ **Phase 7 완료 - ATL COM DLL 프로젝트 성공!**
- ✅ OnVoiceAudioBridge 프로젝트 생성
- ✅ IOnVoiceCapture 인터페이스 3개 메서드 구현
- ✅ VBScript 테스트 완벽 통과
- ✅ COM 객체 생성 및 상태 관리 검증
- 📊 진행률: 50% → 56%
- ⏱️ 시간 절감: 누적 24시간 (계획 38h → 실제 14h)

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
