# OnVoice COM Audio Bridge 🎙️

Windows WASAPI 기반 프로세스별 오디오 캡처 COM 브리지

[![Phase](https://img.shields.io/badge/Phase-Week%201%20Day%201%20Complete-green)]()
[![Progress](https://img.shields.io/badge/Progress-30%25-blue)]()
[![Time](https://img.shields.io/badge/Spent-10h%20%2F%2050h-orange)]()

---

## 🎯 프로젝트 개요

**목표**: Discord, Chrome, Edge의 오디오를 선택적으로 캡처하여 Electron 앱으로 실시간 전송

**핵심 기능**:

- ✅ 기본 WASAPI 루프백 캡처 (Day 1 완료)
- ✅ 자동 프로세스 감지 (Discord/Chrome) (Week 0 완료)
- ✅ 16kHz mono PCM 자동 변환 (Week 0 검증)
- ⏳ PID 기반 프로세스별 오디오 격리 (Day 2 예정)
- ⏳ COM 이벤트 기반 실시간 스트리밍 (Week 2 예정)

**팀**: H.E.A.R. (OnVoice 프로젝트)  
**개발 기간**: 50시간 스프린트 (Week 0-3)  
**성능 목표**: 평균 latency < 3초

---

## 🏆 최신 업데이트 (2025-11-17, Day 1)

### ✅ Day 1 완료 내용

**1. Visual Studio 2026 + ATL 환경 구축**

```
✅ Visual Studio Community 2026 설치
✅ ATL 구성 요소 (v145 빌드 도구) 설치
✅ HelloCOM ATL 프로젝트 빌드 성공
✅ regsvr32 권한 문제 해결 (사용자 단위 리디렉션)
```

**2. C++ 기초 학습**

```cpp
// 포인터와 참조 실습 (CppBasics)
✅ 포인터, 참조, 이중 포인터 개념 학습
✅ nullptr 사용법 습득
✅ -> 연산자 이해

// COM 기본 실습 (COMBasics)
✅ CoInitialize/CoUninitialize
✅ CoCreateInstance로 COM 객체 생성
✅ IMMDevice로 오디오 디바이스 정보 가져오기
✅ AddRef/Release 참조 카운팅
```

**3. WASAPI 기본 오디오 캡처 성공 ⭐**

```
AudioCapture 프로젝트:
✅ 루프백 모드로 시스템 오디오 캡처
✅ 48kHz 스테레오, 5초간 240,000 프레임 캡처
✅ 500개 패킷 실시간 수신 성공
✅ 실제 오디오 데이터 확인 (DB 6A 81 BC...)
✅ 메모리 누수 없이 정리 완료
```

### 📊 Day 1 성과

| 항목 | 계획 | 실제 | 상태 |
|------|------|------|------|
| Phase 1 (VS 설정) | 2h | 1h | ✅ 초과 달성 |
| Phase 2 (C++ 기초) | 2h | 1.5h | ✅ 완료 |
| Phase 3.1 (기본 캡처) | 2h | 1.5h | ✅ 완료 |
| **합계** | **6h** | **4h** | **-2h 절감!** |

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

### 📋 Week 1: COM 브리지 기초 (T+6-20h) - 진행 중 (40% 완료)

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

**학습 성과**:
- ✅ 4개 프로젝트 완성 (HelloCOM, CppBasics, COMBasics, AudioCapture)
- ✅ 모든 프로젝트 빌드 및 실행 성공
- ✅ 실제 오디오 데이터 캡처 검증

#### 📋 Day 2 (T+10-18h) - 계획

- [ ] **Phase 4**: PID 기반 캡처 ⭐ 핵심!
  - [ ] `AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS` 구조체
  - [ ] `ActivateAudioInterfaceAsync` API
  - [ ] Chrome/Discord PID로 선택적 캡처
  - [ ] 타 앱 소리 격리 검증

- [ ] **Phase 5**: 리소스 누수 수정
  - [ ] MS 샘플 버그 수정 (참조 카운트)
  - [ ] 100회 시작/중지 테스트

- [ ] **Phase 6**: SpeexDSP 리샘플링 (선택 사항)
  - [ ] 48kHz Float → 16kHz Int16 변환
  - [ ] (Week 0에서 불필요 확인됨)

**예상 소요**: 6-8시간

---

### 📋 Week 2: COM DLL 및 Electron 연동 (T+18-40h) - 계획

#### Day 3-4: ATL COM DLL 프로젝트

- [ ] **Phase 7**: ATL COM 프로젝트 생성
  - [ ] OnVoiceAudioBridge DLL 프로젝트
  - [ ] IDL 인터페이스 정의
  - [ ] `IDispatch` 메서드 구현

- [ ] **Phase 8**: 캡처 엔진 통합
  - [ ] Phase 4 코드를 COM DLL로 이식
  - [ ] StartCapture(PID), StopCapture() 메서드
  - [ ] VBScript로 테스트

#### Day 5-6: COM 이벤트 및 Electron 연동

- [ ] **Phase 9-10**: COM 이벤트 콜백
  - [ ] `IConnectionPoint` 구현
  - [ ] OnAudioData(SAFEARRAY*) 이벤트
  - [ ] SAFEARRAY로 오디오 데이터 전송

- [ ] **Phase 11-13**: winax + Electron
  - [ ] winax 설치 및 재빌드
  - [ ] Electron Main 프로세스 연동
  - [ ] Renderer 프로세스로 데이터 전송

**예상 소요**: 18-22시간

---

### 📋 Week 3: 테스트 및 완성 (T+40-50h) - 계획

- [ ] **Phase 14**: 다중 프로세스 테스트
  - [ ] Discord/Chrome/Edge 동시 캡처
  - [ ] 프로세스 격리 검증

- [ ] **Phase 15**: 안정화
  - [ ] 크래시 시나리오 테스트
  - [ ] 메모리 누수 검사 (1시간 연속 실행)
  - [ ] 최종 E2E 테스트

**예상 소요**: 10시간

---

## 📂 프로젝트 구조

```
onvoice-com-bridge/
├── docs/                           # 📚 문서
│   ├── learning-notes.md           # 빠른 참조 + Day 1 학습 내용 ✅
│   ├── details/                    # 상세 문서
│   │   ├── com-deep-dive.md        # COM 상세
│   │   ├── wasapi-deep-dive.md     # WASAPI 상세
│   │   └── poc-lessons.md          # PoC 학습
│   ├── build-errors.md             # 에러 해결
│   └── phase-progress.md           # 진행 상황
│
├── phase1-console/                 # 🧪 PoC 및 학습
│   ├── reference/                  # ProcessLoopbackCapture 원본 ✅
│   └── poc/                        # 콘솔 PoC (Week 0 완료) ✅
│
├── phase2-learning/                # 📖 Day 1 학습 프로젝트 ✅
│   ├── HelloCOM/                   # ATL DLL 템플릿
│   ├── CppBasics/                  # 포인터/참조 실습
│   ├── COMBasics/                  # COM 기본 실습
│   └── AudioCapture/               # WASAPI 루프백 캡처 ⭐
│
├── phase3-com-dll/                 # 🔧 COM DLL (Week 1-2 예정)
│   └── OnVoiceAudioBridge/
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

### Day 1 학습 프로젝트 - 현재 가능 ✅

#### HelloCOM (ATL DLL 템플릿)
```bash
1. Visual Studio 2026 열기
2. phase2-learning/HelloCOM/HelloCOM.sln 열기
3. Ctrl+Shift+B (빌드)
4. x64/Debug/HelloCOM.dll 생성 확인
```

#### CppBasics (포인터/참조 실습)
```bash
1. phase2-learning/CppBasics/CppBasics.sln 열기
2. Ctrl+F5 (실행)
3. 포인터와 참조 출력 확인
```

#### COMBasics (COM 기본 실습)
```bash
1. phase2-learning/COMBasics/COMBasics.sln 열기
2. Ctrl+F5 (실행)
3. 오디오 디바이스 정보 출력 확인
```

#### AudioCapture (WASAPI 캡처) ⭐
```bash
1. phase2-learning/AudioCapture/AudioCapture.sln 열기
2. Ctrl+F5 (실행)
3. 음악 재생 (YouTube, Spotify 등)
4. 5초간 오디오 캡처 진행
5. 결과 확인:
   - 총 패킷 수: 500개
   - 총 프레임 수: 240,000
   - 오디오 데이터: DB 6A 81 BC...
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
- **프로젝트 타입**: ATL COM In-Process DLL (예정)
- **API**: Windows WASAPI (Loopback + Process-Specific)
- **참조 구현**: [ProcessLoopbackCapture](https://github.com/Naseband/ProcessLoopbackCapture)
- **링커 라이브러리**: ole32.lib, oleaut32.lib, mmdevapi.lib, avrt.lib

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

- **[learning-notes.md](docs/learning-notes.md)**: 빠른 참조 + Day 1 학습 내용 (포인터, COM, WASAPI)

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
[████████░░░░░░░░░░] 30% (10h / 50h)

Week 0: ████████████ 100% (6h)
Week 1: ████░░░░░░░░ 40% (4h / 14h)
Week 2: ░░░░░░░░░░░░ 0% (0h / 22h)
Week 3: ░░░░░░░░░░░░ 0% (0h / 10h)
```

### 마일스톤

| 마일스톤 | 상태 | 날짜 |
|----------|------|------|
| Week 0 PoC 완성 | ✅ 완료 | 2025-11-16 |
| **Day 1 학습 완료** | ✅ **완료** | **2025-11-17** |
| Day 2 PID 캡처 | ⏳ 진행 예정 | 2025-11-18 |
| Week 1 COM 기초 | ⏳ 진행 중 | 2025-11-20 |
| Week 2 Electron 연동 | 📅 예정 | 2025-11-27 |
| Week 3 MVP 완성 | 📅 예정 | 2025-12-04 |

### 시간 효율

| 항목 | 계획 | 실제 | 차이 |
|------|------|------|------|
| Week 0 | 14h | 6h | **-8h** ✨ |
| Week 1 (Day 1) | 6h | 4h | **-2h** ✨ |
| **누적 절감** | 20h | 10h | **-10h** 🎉 |
| **남은 예산** | 50h | 40h | - |

---

## 🎯 다음 단계 (Day 2)

### Phase 4: PID 기반 캡처 (최우선)

**목표**: Discord/Chrome만 선택적으로 캡처

**핵심 작업**:
1. `AUDIOCLIENT_PROCESS_LOOPBACK_PARAMS` 구조체 설정
2. `ActivateAudioInterfaceAsync` API 사용
3. Chrome PID로 YouTube만 캡처
4. Discord PID로 Discord만 캡처
5. 타 앱(Spotify, Windows 알림) 격리 검증

**예상 소요**: 2-3시간  
**난이도**: ⭐⭐⭐ 어려움

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
- ✅ **WASAPI 루프백 캡처 성공** ⭐
  - 48kHz 스테레오, 240,000 프레임
  - 500개 패킷 실시간 수신
  - 실제 오디오 데이터 검증 완료

### 학습한 프로젝트

1. **HelloCOM** - ATL DLL 템플릿 이해
2. **CppBasics** - 포인터와 참조 실습
3. **COMBasics** - COM 디바이스 정보 가져오기
4. **AudioCapture** - WASAPI 루프백 캡처 ⭐

---

## 🔗 참고 자료

### 공식 문서

- [Microsoft WASAPI](https://learn.microsoft.com/en-us/windows/win32/coreaudio/wasapi)
- [ATL COM Tutorial](https://learn.microsoft.com/en-us/cpp/atl/)
- [ProcessLoopbackCapture GitHub](https://github.com/Naseband/ProcessLoopbackCapture)

### 학습 자료

- [learning-notes.md](docs/learning-notes.md) - 빠른 참조 + Day 1 학습
- [COM Deep Dive](docs/details/com-deep-dive.md) - COM 상세
- [WASAPI Deep Dive](docs/details/wasapi-deep-dive.md) - WASAPI 상세

---

**마지막 업데이트**: 2025-11-17 (Day 1 완료)  
**개발자**: 김원 (H.E.A.R. Team)  
**현재 상태**: Week 1 Day 1 완료 ✅ → Day 2 준비 중 🚀

---

## 📝 변경 이력

### 2025-11-17 (Day 1)
- ✅ Visual Studio 2026 + ATL 설정 완료
- ✅ C++ 기초 학습 (포인터, 참조, COM)
- ✅ WASAPI 루프백 캡처 성공
- ✅ 4개 학습 프로젝트 완성
- 📊 진행률: 12% → 30%
- ⏱️ 시간 절감: 누적 10시간

### 2025-11-16 (Week 0)
- ✅ ProcessLoopbackCapture 분석
- ✅ 콘솔 PoC 완성
- ✅ PID 기반 캡처 검증
- ✅ 16kHz 자동 변환 확인