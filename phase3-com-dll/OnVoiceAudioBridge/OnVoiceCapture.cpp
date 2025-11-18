// OnVoiceCapture.cpp : COnVoiceCapture 구현

#include "pch.h"
#include "OnVoiceCapture.h"
#include <stdio.h>  // ⭐ printf 사용을 위해 추가

// ========================================
// 소멸자: AudioCaptureEngine 삭제 (⭐ 새로 추가)
// ========================================
COnVoiceCapture::~COnVoiceCapture()
{
    printf("[COnVoiceCapture] 소멸자 시작\n");

    // 엔진 삭제
    if (m_pEngine) {
        delete m_pEngine;
        m_pEngine = nullptr;
        printf("[COnVoiceCapture] AudioCaptureEngine 삭제\n");
    }

    printf("[COnVoiceCapture] ✅ 소멸 완료\n");
}

// ========================================
// IOnVoiceCapture 인터페이스 구현
// ========================================

// 캡처 시작 (⭐ 수정됨)
STDMETHODIMP COnVoiceCapture::StartCapture(LONG processId)
{
    printf("[COnVoiceCapture] StartCapture 호출 (PID: %ld)\n", processId);

    // ⭐ 엔진이 없으면 지금 생성 (지연 생성)
    if (!m_pEngine) {
        printf("[COnVoiceCapture] AudioCaptureEngine 생성 중...\n");
        m_pEngine = new AudioCaptureEngine();

        if (!m_pEngine) {
            printf("[COnVoiceCapture] ❌ 엔진 생성 실패!\n");
            return E_OUTOFMEMORY;
        }

        printf("[COnVoiceCapture] ✅ AudioCaptureEngine 생성 성공\n");
    }

    // 2. 이미 실행 중이면 에러
    if (m_bIsCapturing || m_pEngine->IsCapturing()) {
        printf("[COnVoiceCapture] ⚠️ 이미 캡처 중입니다\n");
        return E_FAIL;
    }

    // 3. PID 저장
    m_targetPid = processId;

    // 4. 엔진 시작 (this를 콜백으로 전달) (⭐ 핵심!)
    HRESULT hr = m_pEngine->Start((DWORD)processId, this);

    if (SUCCEEDED(hr)) {
        m_bIsCapturing = TRUE;
        printf("[COnVoiceCapture] ✅ 캡처 시작 성공\n");
    }
    else {
        printf("[COnVoiceCapture] ❌ 캡처 시작 실패 (HR: 0x%X)\n", hr);
    }

    return hr;
}

// 캡처 중지 (⭐ 수정됨)
STDMETHODIMP COnVoiceCapture::StopCapture()
{
    printf("[COnVoiceCapture] StopCapture 호출\n");

    // 1. 엔진이 없으면 무시
    if (!m_pEngine) {
        printf("[COnVoiceCapture] 엔진이 없습니다 (정상)\n");
        m_bIsCapturing = FALSE;
        return S_OK;
    }

    // 2. 실행 중이 아니면 무시
    if (!m_bIsCapturing) {
        printf("[COnVoiceCapture] 이미 중지 상태입니다\n");
        return S_OK;
    }

    // 3. 엔진 중지 (⭐ 추가)
    HRESULT hr = m_pEngine->Stop();

    if (SUCCEEDED(hr)) {
        m_bIsCapturing = FALSE;
        m_targetPid = 0;
        printf("[COnVoiceCapture] ✅ 캡처 중지 성공\n");
    }
    else {
        printf("[COnVoiceCapture] ❌ 캡처 중지 실패 (HR: 0x%X)\n", hr);
    }

    return hr;
}

// 상태 확인 (⭐ 수정됨)
STDMETHODIMP COnVoiceCapture::GetCaptureState(LONG* pState)
{
    // 1. NULL 포인터 체크 (중요!)
    if (pState == NULL) {
        return E_POINTER;  // 잘못된 포인터
    }

    // 2. 엔진 상태 반환 (⭐ 수정)
    if (m_pEngine) {
        *pState = (LONG)m_pEngine->GetState();
    }
    else {
        *pState = 0; // STATE_STOPPED
    }

    printf("[COnVoiceCapture] GetCaptureState = %ld\n", *pState);
    return S_OK;
}

// ========================================
// IAudioDataCallback 구현 (⭐ 새로 추가)
// - 엔진에서 오디오 데이터를 받으면 호출됨
// ========================================
void COnVoiceCapture::OnAudioData(BYTE* pData, UINT32 dataSize)
{
    printf("[COnVoiceCapture] OnAudioData 호출 (크기: %u 바이트)\n", dataSize);

    // SAFEARRAY로 변환
    SAFEARRAY* pSA = SafeArrayCreateVector(VT_UI1, 0, dataSize);
    if (!pSA) {
        printf("[COnVoiceCapture] ❌ SAFEARRAY 생성 실패!\n");
        return;
    }

    // 데이터 복사
    void* pArrayData = nullptr;
    HRESULT hr = SafeArrayAccessData(pSA, &pArrayData);
    if (SUCCEEDED(hr)) {
        memcpy(pArrayData, pData, dataSize);
        SafeArrayUnaccessData(pSA);

        // COM 이벤트 발생! (⭐ VBScript로 전달)
        HRESULT hrEvent = Fire_OnAudioData(pSA);

        if (SUCCEEDED(hrEvent)) {
            printf("[COnVoiceCapture] ✅ OnAudioData 이벤트 발생 성공\n");
        }
        else {
            printf("[COnVoiceCapture] ❌ OnAudioData 이벤트 발생 실패 (HR: 0x%X)\n", hrEvent);
        }
    }
    else {
        printf("[COnVoiceCapture] ❌ SAFEARRAY 접근 실패!\n");
    }

    // SAFEARRAY 정리
    SafeArrayDestroy(pSA);
}

// ========================================
// 이벤트 헬퍼: 16kHz PCM 오디오 데이터를 구독자에게 브로드캐스트
// ========================================
HRESULT COnVoiceCapture::Fire_OnAudioData(SAFEARRAY* psaAudio)
{
    if (psaAudio == nullptr)
    {
        return E_POINTER;
    }

    // VARIANT 설정: SAFEARRAY(BYTE) → VT_ARRAY | VT_UI1
    CComVariant varAudio;
    varAudio.vt = VT_ARRAY | VT_UI1;
    varAudio.parray = psaAudio;  // 소유권은 호출자에 유지됨

    const int nConnections = m_vec.GetSize();
    HRESULT hr = S_OK;

    // OnAudioData의 DISPID는 IDL에서 [id(1)]으로 정의했으므로 1
    const DISPID dispidOnAudioData = 1;

    for (int i = 0; i < nConnections; ++i)
    {
        CComPtr<IUnknown> spUnk = m_vec.GetAt(i);
        if (!spUnk)
        {
            continue;
        }

        CComQIPtr<IDispatch> spDispatch = spUnk;
        if (!spDispatch)
        {
            continue;
        }

        DISPPARAMS dp = {};
        dp.rgvarg = &varAudio;  // 인자 1개
        dp.cArgs = 1;

        HRESULT hrInvoke = spDispatch->Invoke(
            dispidOnAudioData,
            IID_NULL,
            LOCALE_USER_DEFAULT,
            DISPATCH_METHOD,
            &dp,
            nullptr,    // 결과값 없음
            nullptr,    // 예외 정보 사용 안 함
            nullptr     // 인자 오류 인덱스 사용 안 함
        );

        // 첫 번째 실패 코드만 hr에 저장 (참고용)
        if (FAILED(hrInvoke) && SUCCEEDED(hr))
        {
            hr = hrInvoke;
        }
    }

    return hr;
}