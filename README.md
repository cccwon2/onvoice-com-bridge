# OnVoice COM Audio Bridge 🎙️

Windows WASAPI 기반 프로세스별 오디오 캡처 COM 브리지

[![Phase](https://img.shields.io/badge/Phase-Week%200%20Complete-green)]()
[![Progress](https://img.shields.io/badge/Progress-12%25-blue)]()
[![Time](https://img.shields.io/badge/Spent-6h%20%2F%2050h-orange)]()

---

## 🎯 프로젝트 개요

**목표**: Discord, Chrome, Edge의 오디오를 선택적으로 캡처하여 Electron 앱으로 실시간 전송

**핵심 기능**:

- ✅ PID 기반 프로세스별 오디오 격리 (검증 완료)
- ✅ 자동 프로세스 감지 (Discord/Chrome)
- ✅ 16kHz mono PCM 자동 변환 (SpeexDSP 불필요)
- ⏳ COM 이벤트 기반 실시간 스트리밍 (개발 중)

**팀**: H.E.A.R. (OnVoice 프로젝트)  
**개발 기간**: 50시간 스프린트 (Week 0-3)  
**성능 목표**: 평균 latency < 3초

---

## 🏆 Week 0 성과 (2025-11-16)

### ✅ 완료된 것

**1. PID 기반 오디오 캡처 검증**

```
Discord (PID 15678) → 16kHz PCM → WAV 파일 ✅
Chrome (PID 12812) → 16kHz PCM → WAV 파일 ✅

격리 확인:
- Discord만 캡처 (Windows 알림 무시) ✅
- Chrome YouTube만 캡처 (Spotify 무시) ✅
```

**2. 프로세스 자동 감지**

```cpp
// Chrome 브라우저 프로세스
FindChromeBrowserProcess() → PID 12812 ✅

// Discord 프로세스
FindDiscordProcess() → PID 15678 ✅
```

**3. 16kHz 자동 변환**

```
앱 출력: 48kHz → Windows 엔진: 자동 변환 → 캡처: 16kHz ✅
→ SpeexDSP 불필요 (10시간 절감!)
```

**4. 콘솔 PoC 완성**

- ProcessLoopbackCapture 레퍼런스 분석 완료
- 실시간 볼륨 모니터링 구현
- WAV 파일 저장 및 재생 검증

### 📊 시간 효율

| 항목             | 계획 | 실제 | 차이       |
| ---------------- | ---- | ---- | ---------- |
| WASAPI 직접 구현 | 14h  | -    | -          |
| PoC 개발         | -    | 6h   | **-8h** ✨ |
| **남은 시간**    | 50h  | 44h  | -          |

---

## 🛠️ 기술 스택

### C++ 레이어

- **개발 환경**: Visual Studio Community 2022/2026
- **프로젝트 타입**: ATL COM In-Process DLL (예정)
- **API**: Windows WASAPI (Process-Specific Loopback Capture)
- **참조 구현**: [ProcessLoopbackCapture](https://github.com/Naseband/ProcessLoopbackCapture)

### Electron 레이어

- **Runtime**: Node.js + React.js
- **COM 클라이언트**: winax (예정)
- **IPC**: Electron Main ↔ Renderer (예정)

### FastAPI 서버

- **STT**: Deepgram + Whisper (fallback)
- **OCR**: PaddleOCR (한국어 특화)
- **AI**: KoELECTRA (유해 표현 탐지)

---

## 📂 프로젝트 구조

```
onvoice-com-bridge/
├── docs/                           # 📚 문서
│   ├── learning-notes.md           # 빠른 참조 + 핵심 개념
│   ├── details/                    # 상세 문서 (새로 추가!)
│   │   ├── com-deep-dive.md        # COM 상세
│   │   ├── wasapi-deep-dive.md     # WASAPI 상세
│   │   └── poc-lessons.md          # PoC 학습
│   ├── build-errors.md             # 에러 해결
│   └── phase-progress.md           # 진행 상황
│
├── phase1-console/                 # 🧪 PoC 및 학습
│   ├── reference/                  # ProcessLoopbackCapture 원본
│   │   └── ProcessLoopbackCapture/
│   └── AudioCaptureTest/           # 콘솔 PoC ✅
│
├── phase2-com-dll/                 # 🔧 COM DLL (예정)
│   └── OnVoiceAudioBridge/
│
├── phase3-electron/                # ⚡ Electron 연동 (예정)
│   └── test-winax/
│
└── scripts/                        # 🤖 유틸리티
    ├── build.bat
    └── get-pid.ps1
```

---

## 🚀 빌드 및 실행

### PoC (콘솔 앱) - 현재 가능 ✅

```bash
# Visual Studio에서 열기
cd phase1-console/AudioCaptureTest
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

### COM DLL - 아직 없음 (Week 1 예정)

```bash
# Phase 1 (Visual Studio 2026 설치) 후 가능
```

---

## 📖 학습 자료

### 빠른 시작

- **[learning-notes.md](docs/learning-notes.md)**: 빠른 참조 + COM/WASAPI 핵심 개념

### 상세 문서

- **[COM Deep Dive](docs/details/com-deep-dive.md)**: IUnknown, 참조 카운팅, 스마트 포인터
- **[WASAPI Deep Dive](docs/details/wasapi-deep-dive.md)**: 6단계 초기화, 에러 처리, 최적화
- **[PoC Lessons](docs/details/poc-lessons.md)**: ProcessLoopbackCapture 패턴 5가지

### 트러블슈팅

- **[build-errors.md](docs/build-errors.md)**: 빌드 에러 해결
- **[phase-progress.md](docs/phase-progress.md)**: Phase별 상세 진행 기록

---

## 🗓️ 개발 로드맵

### ✅ Week 0: PoC 완성 (T+0-6h) - 완료!

- [x] 프로젝트 초기화
- [x] COM 기초 학습
- [x] WASAPI 개념 학습
- [x] ProcessLoopbackCapture 분석
- [x] 콘솔 PoC 개발
- [x] 프로세스 자동 감지

### 📋 Week 1: COM 브리지 기초 (T+6-20h) - 계획

- [ ] Phase 1: Visual Studio 2026 + ATL 설치
- [ ] Phase 2: C++ 기초 (포인터/참조)
- [ ] Phase 7: ATL COM 프로젝트 생성
- [ ] Phase 8: OnVoice 캡처 엔진 통합

### 📋 Week 2: Electron 연동 (T+20-40h) - 계획

- [ ] Phase 9-10: COM 이벤트 콜백
- [ ] Phase 11-13: winax + Electron IPC

### 📋 Week 3: 테스트 및 완성 (T+40-50h) - 계획

- [ ] Phase 14-15: 다중 프로세스 테스트, 안정화

---

## 🔗 참고 자료

### 공식 문서

- [Microsoft WASAPI](https://learn.microsoft.com/en-us/windows/win32/coreaudio/wasapi)
- [ATL COM Tutorial](https://learn.microsoft.com/en-us/cpp/atl/)
- [ProcessLoopbackCapture GitHub](https://github.com/Naseband/ProcessLoopbackCapture)

---

**마지막 업데이트**: 2025-11-16  
**개발자**: 김원 (H.E.A.R. Team)  
**현재 상태**: Week 0 완료 ✅ → Week 1 준비 중 🚀
