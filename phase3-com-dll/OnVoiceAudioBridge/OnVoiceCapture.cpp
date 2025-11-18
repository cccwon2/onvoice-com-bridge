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

    // ========================================
    // ★ Phase 8-4: 테스트용 더미 오디오 이벤트 한 번 쏘기
    //    - 나중에 WASAPI 캡처 루프에서 실제 오디오로 교체 예정
    // ========================================
    {
        const LONG dummyLength = 320; // 예시: 320바이트 (짧은 청크)

        SAFEARRAYBOUND bound;
        bound.lLbound = 0;
        bound.cElements = dummyLength;

        SAFEARRAY* psa = SafeArrayCreate(VT_UI1, 1, &bound);
        if (psa != nullptr)
        {
            BYTE* pData = nullptr;
            HRESULT hrAccess = SafeArrayAccessData(psa, (void**)&pData);
            if (SUCCEEDED(hrAccess) && pData != nullptr)
            {
                // 일단 전부 0으로 채움 (무음)
                ZeroMemory(pData, dummyLength * sizeof(BYTE));
                SafeArrayUnaccessData(psa);

                // 이벤트 발사
                HRESULT hrEvent = Fire_OnAudioData(psa);
                if (FAILED(hrEvent))
                {
                    // 디버그 용: 이벤트 실패 시 로그만 남김
                    OutputDebugStringW(L"[OnVoiceCapture] Fire_OnAudioData failed in test block.\n");
                }
            }

            SafeArrayDestroy(psa);
        }
    }

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

// 16kHz PCM 오디오 데이터를 구독자에게 브로드캐스트
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
