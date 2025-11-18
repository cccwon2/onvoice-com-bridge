// OnVoiceCapture.cpp : COnVoiceCapture 구현

#include "pch.h"
#include "OnVoiceCapture.h"

// ========================================
// IOnVoiceCapture 인터페이스 구현
// ========================================

// 캡처 시작
STDMETHODIMP COnVoiceCapture::StartCapture(LONG processId)
{
    // 1. 이미 실행 중이면 에러
    if (m_bIsCapturing) {
        return E_FAIL;  // "이미 캡처 중입니다"
    }

    // 2. PID 저장
    m_targetPid = processId;

    // 3. 캡처 시작 (나중에 Phase 9에서 실제 캡처 로직 추가)
    // TODO: Phase 9에서 WASAPI 캡처 코드 추가
    m_bIsCapturing = TRUE;

    return S_OK;  // 성공
}

// 캡처 중지
STDMETHODIMP COnVoiceCapture::StopCapture()
{
    // 1. 실행 중이 아니면 무시
    if (!m_bIsCapturing) {
        return S_OK;  // 이미 중지됨
    }

    // 2. 캡처 중지 (나중에 Phase 9에서 실제 중지 로직 추가)
    // TODO: Phase 9에서 정리 코드 추가
    m_bIsCapturing = FALSE;
    m_targetPid = 0;

    return S_OK;
}

// 상태 확인
STDMETHODIMP COnVoiceCapture::GetCaptureState(LONG* pState)
{
    // 1. NULL 포인터 체크 (중요!)
    if (pState == NULL) {
        return E_POINTER;  // 잘못된 포인터
    }

    // 2. 상태 반환 (0=중지, 1=실행 중)
    *pState = m_bIsCapturing ? 1 : 0;

    return S_OK;
}