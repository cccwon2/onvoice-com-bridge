# Week 0: PoC 완성 및 검증 ✅

T+0-6h (6시간 완료)

**진행률**: 100%  
**상태**: ✅ 완료  
**완료 날짜**: 2025-11-16

---

## 📊 전체 진행 상황

```
Week 0 타임라인:
[████████████] 100% (6h / 14h 계획)

Phase 0:   ████ 완료 (0.5h)
Phase 0.5: ████ 완료 (1.5h)
Phase 0.6: ████ 완료 (1h)
Phase 0.7: ████ 완료 (1h)
Phase 0.8: ████ 완료 (2h) ⭐

시간 절감: -8시간! (계획 14h → 실제 6h)
```

---

## 🎯 Week 0 목표

**ProcessLoopbackCapture 레포 발견으로 PoC 단계 추가**

원래 계획했던 Phase 3-6 (WASAPI 직접 구현, 14시간)을 건너뛰고, 참조 구현을 분석하여 PoC를 빠르게 완성함으로써 **8시간 절감!**

---

## ✅ 완료한 Phase

### Phase 0: 프로젝트 초기화 (0.5h)

- [x] GitHub 레포 생성 (onvoice-com-bridge)
- [x] 문서 구조 세팅
- [x] 학습 노트 시스템 구축

---

### Phase 0.5: COM 기초 개념 학습 (1.5h)

**학습 내용**:

- IUnknown 3가지 메서드
- HRESULT 에러 체계
- 참조 카운팅 (AddRef/Release)

**실생활 비유**:

- COM = 아파트 관리 규칙
- IUnknown = 모든 COM 객체의 DNA
- 참조 카운팅 = 도서관 공유 책 시스템

---

### Phase 0.6: WASAPI 개념 학습 (1h)

**핵심 컴포넌트 4가지**:

1. IMMDeviceEnumerator (디바이스 목록)
2. IMMDevice (특정 디바이스)
3. IAudioClient (오디오 세션)
4. IAudioCaptureClient (실제 데이터)

**중요 발견**:

- ✅ Process-Specific Loopback으로 앱별 오디오 격리 가능
- ✅ 16kHz 변환이 Windows 엔진에서 자동 처리 (SpeexDSP 불필요!)

---

### Phase 0.7: ProcessLoopbackCapture 코드 리뷰 (1h)

**발견한 핵심 패턴 5가지**:

1. **비동기 활성화 패턴**
   - Completion Handler + 이벤트 동기화
2. **캡처 스레드 패턴**
   - Static 함수 + this 포인터 전달
   - 우선순위 상승 (AvSetMmThreadCharacteristics)
3. **3-Layer 에러 처리**
   - Layer 1: 사용자 친화적 enum
   - Layer 2: HRESULT 저장
   - Layer 3: 에러 텍스트 변환
4. **스레드 안전성**
   - `std::atomic<eCaptureState>` 사용
5. **Lock-Free Queue**
   - moodycamel::ReaderWriterQueue

---

### Phase 0.8: 콘솔 PoC 개발 (2h) ⭐

**완료 사항**:

- [x] ProcessLoopbackCapture 통합
- [x] PID 기반 캡처 검증
- [x] 16kHz mono PCM 캡처
- [x] Chrome 프로세스 자동 감지
- [x] Discord 프로세스 자동 감지
- [x] WAV 파일 저장

**Chrome 프로세스 자동 감지**:

```cpp
// --type= 플래그가 없는 chrome.exe = 브라우저 프로세스
std::wstring cmdLine = GetProcessCommandLine(pid);
if (cmdLine.find(L"--type=") == std::wstring::npos) {
    return pid;  // 브라우저 프로세스!
}
```

**검증 결과**:

```
✅ Chrome 브라우저 프로세스 자동 감지 성공
✅ Discord 프로세스 자동 감지 성공
✅ 16kHz mono 캡처 검증 완료
✅ 타 앱 소리 격리 확인 (Discord만 캡처, Windows 알림 무시)
✅ WAV 파일 재생 확인
✅ 총 캡처 데이터: ~0.32 MB (10초)
```

---

## 📊 Week 0 요약

### 시간 효율

| 항목                    | 원래 계획 | 실제 | 차이       |
| ----------------------- | --------- | ---- | ---------- |
| Phase 3-6 (WASAPI 구현) | 14h       | -    | 건너뜀     |
| Week 0 (PoC)            | -         | 6h   | **추가**   |
| **절감**                | -         | -    | **-8h** ✨ |

### 주요 성과

1. **PID 기반 오디오 캡처 검증 완료** ⭐
   - Chrome/Discord 선택적 캡처
   - 프로세스 자동 감지
2. **16kHz 자동 변환 확인**
   - SpeexDSP 불필요!
   - Windows 엔진에서 자동 처리
3. **콘솔 PoC 완성**
   - ProcessLoopbackCapture 통합
   - WAV 파일 저장 성공

### 배운 점

- ✅ 기존 레퍼런스 활용으로 시간 절감 가능
- ✅ PID 기반 캡처가 실제로 작동함 (가장 중요!)
- ✅ Chrome 멀티프로세스 아키텍처 이해
- ✅ ProcessLoopbackCapture의 5가지 핵심 패턴

---

## 🎯 Week 0의 가치

**Week 0 PoC 덕분에**:

- ✅ 기술적 불확실성 제거 (PID 캡처 검증)
- ✅ 8시간 시간 절약
- ✅ Week 1 개발 속도 가속화 (참조 코드 확보)

**없었다면**:

- ❌ WASAPI 직접 구현 시행착오 (14시간)
- ❌ PID 캡처 실패 위험
- ❌ 전체 일정 지연

---

**[← 전체 요약으로](../phase-progress.md)**  
**[다음: Week 1 →](week1-com-bridge.md)**
