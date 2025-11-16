# OnVoice COM Audio Bridge 🎙️

Windows WASAPI 기반 프로세스별 오디오 캡처 COM 브리지

## 📋 프로젝트 개요

**목표**: Discord, Chrome, Edge의 오디오를 선택적으로 캡처하여 Electron 앱으로 실시간 전송

**팀**: H.E.A.R. (OnVoice 프로젝트)  
**개발 기간**: 60시간 스프린트 (3주)  
**성능 목표**: 평균 latency < 3초

---

## 🛠️ 기술 스택

### C++ 레이어

- **개발 환경**: Visual Studio Community 2026
- **프로젝트 타입**: ATL COM In-Process DLL
- **API**: Windows WASAPI (Process-Specific Loopback Capture)
- **리샘플링**: SpeexDSP (48kHz Float → 16kHz Int16)
- **인터페이스**: `IDispatch`, `IConnectionPoint` (COM 이벤트)

### Electron 레이어

- **Runtime**: Node.js + React.js
- **COM 클라이언트**: winax
- **IPC**: Electron Main ↔ Renderer

### FastAPI 서버

- **STT**: Deepgram + Whisper (fallback)
- **OCR**: PaddleOCR (한국어 특화)
- **AI 모델**: KoELECTRA (유해 표현 탐지)

---

## 📂 프로젝트 구조

```
onvoice-com-bridge/
├── docs/                        # 📚 문서 및 학습 자료
│   ├── learning-notes.md        # C++ 학습 내용 정리
│   ├── build-errors.md          # 빌드 에러 해결 기록
│   ├── phase-progress.md        # Phase별 진행 상황
│   └── images/                  # 스크린샷, 다이어그램
├── phase1-console/              # 🧪 Phase 1-4: WASAPI 콘솔 앱 (실험)
│   └── AudioCaptureConsole/
├── phase2-com-dll/              # 🔧 Phase 5+: ATL COM DLL (핵심 자산)
│   └── OnVoiceAudioBridge/
├── phase3-electron/             # ⚡ Electron winax 연동 테스트
│   └── test-winax/
└── scripts/                     # 🤖 유틸리티 스크립트
    ├── build.bat                # 빌드 자동화
    └── get-pid.ps1              # 프로세스 PID 조회
```

---

## 🚀 개발 진행 상황

### Week 1: COM 브리지 기초 (T+0-30h)

- [x] 프로젝트 초기화 및 Git 세팅
- [ ] Phase 1: 개발 환경 설정 (T+0-2h)
- [ ] Phase 2: C++ 기초 학습 (T+2-4h)
- [ ] Phase 3: WASAPI 콘솔 앱 V1 (T+4-8h)
- [ ] Phase 4: PID 기반 캡처 (T+8-12h)
- [ ] Phase 5: 리소스 관리 (T+12-14h)
- [ ] Phase 6: 리샘플링 통합 (T+14-18h)
- [ ] Phase 7: ATL COM 프로젝트 생성 (T+18-24h)
- [ ] Phase 8: COM 인터페이스 설계 (T+24-30h)

### Week 2: COM-Electron 통합 (T+30-50h)

- [ ] Phase 9: 이벤트 콜백 구현 (T+30-36h)
- [ ] Phase 10: 캡처 엔진 통합 (T+36-40h)
- [ ] Phase 11: winax 환경 설정 (T+40-42h)
- [ ] Phase 12: Electron 연동 (T+42-46h)
- [ ] Phase 13: E2E 파이프라인 (T+46-50h)

### Week 3: 테스트 및 완성 (T+50-60h)

- [ ] Phase 14: 다중 프로세스 테스트 (T+50-55h)
- [ ] Phase 15: 안정화 (T+55-60h)

---

## 🏗️ 빌드 방법

### Phase 1-4: 콘솔 앱

```bash
# Visual Studio에서 열기
cd phase1-console/AudioCaptureConsole
start AudioCaptureConsole.sln

# 또는 명령줄 빌드
msbuild AudioCaptureConsole.sln /p:Configuration=Debug
```

### Phase 5+: COM DLL

```bash
cd phase2-com-dll/OnVoiceAudioBridge
start OnVoiceAudioBridge.sln

# 빌드 후 등록
regsvr32 Debug\OnVoiceAudioBridge.dll
```

---

## 📖 학습 자료

- **[C++ 학습 노트](docs/learning-notes.md)**: COM, 포인터, WASAPI 개념 정리
- **[빌드 에러 해결](docs/build-errors.md)**: 겪은 에러와 해결 방법
- **[Phase 진행 상황](docs/phase-progress.md)**: 각 Phase별 상세 기록

---

## 🔗 참고 문서

- [OnVoice 프로젝트 협업 지침서 v4.0](../onvoice-collaboration-guide-v4.md)
- [COM Interop 개발 계획서](../COM_Interop_오디오_캡처_브리지_개발_계획.pdf)
- [Microsoft WASAPI Documentation](https://learn.microsoft.com/en-us/windows/win32/coreaudio/wasapi)

---

## 📝 라이선스

TBD (프로젝트 완료 후 결정)

---

**마지막 업데이트**: 2025-11-16  
**개발자**: 김원 (H.E.A.R. Team)
