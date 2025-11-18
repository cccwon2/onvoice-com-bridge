// OnVoiceCapture.cpp : COnVoiceCapture 구현

#include "pch.h"
#include "OnVoiceCapture.h"
#include <stdio.h>
#include <atlcomcli.h>
#include <OleAuto.h>

// ========================================
// 소멸자: AudioCaptureEngine 정리
// ========================================
COnVoiceCapture::~COnVoiceCapture()
{
    printf("[COnVoiceCapture] Destructor start\n");

    // GIT 포인터 정리
    m_gitSinks.clear();

    if (m_pEngine)
    {
        printf("[COnVoiceCapture] AudioCaptureEngine Stop() 호출\n");
        m_pEngine->Stop();
        Sleep(100); // 내부 스레드 완전 종료 대기

        delete m_pEngine;
        m_pEngine = nullptr;
        printf("[COnVoiceCapture] AudioCaptureEngine deleted\n");
    }

    printf("[COnVoiceCapture] Destructor end\n");
}

// ========================================
// IOnVoiceCapture 인터페이스 구현
// ========================================

// PID 기반 캡처 시작
STDMETHODIMP COnVoiceCapture::StartCapture(LONG processId)
{
    printf("[COnVoiceCapture] StartCapture (PID=%ld)\n", processId);

    if (processId <= 0)
    {
        printf("[COnVoiceCapture] invalid PID (%ld)\n", processId);
        return E_INVALIDARG;
    }

    if (m_state == CaptureState::Starting || m_state == CaptureState::Capturing)
    {
        printf("[COnVoiceCapture] already starting/capturing (state=%ld)\n",
            static_cast<LONG>(m_state));
        return HRESULT_FROM_WIN32(ERROR_BUSY);
    }

    // AudioCaptureEngine 지연 생성
    if (!m_pEngine)
    {
        printf("[COnVoiceCapture] creating AudioCaptureEngine...\n");
        m_pEngine = new (std::nothrow) AudioCaptureEngine();
        if (!m_pEngine)
        {
            printf("[COnVoiceCapture] AudioCaptureEngine create failed (OOM)\n");
            m_state = CaptureState::Stopped;
            return E_OUTOFMEMORY;
        }
        printf("[COnVoiceCapture] AudioCaptureEngine created\n");
    }

    // 이 객체를 소유한 스레드 ID(대개 cscript main thread) 갱신
    m_ownerThreadId = GetCurrentThreadId();

    // 🔥 현재 연결된 이벤트 싱크들을 GIT에 등록
    //  - m_vec : ATL Connection Point 내부의 sink 리스트
    //  - m_gitSinks : 오디오 스레드에서 사용할 프록시 리스트
    m_gitSinks.clear();
    {
        const int nConnections = m_vec.GetSize();
        printf("[COnVoiceCapture] Preparing GIT sinks, nConnections=%d\n", nConnections);
        m_gitSinks.resize(nConnections);

        for (int i = 0; i < nConnections; ++i)
        {
            IUnknown* pUnk = m_vec.GetAt(i);
            printf("  [GIT prep %d] IUnknown*=%p\n", i, pUnk);
            if (!pUnk)
                continue;

            CComQIPtr<IDispatch> spDisp(pUnk);
            printf("  [GIT prep %d] IDispatch*=%p\n", i, (IDispatch*)spDisp);
            if (!spDisp)
                continue;

            HRESULT hrGit = m_gitSinks[i].Attach(spDisp);
            printf("  [GIT prep %d] Attach HR=0x%08X\n", i, hrGit);
            // 실패해도 일단 넘어감 (그 인덱스는 나중에 CopyTo 실패할 것)
        }
    }

    m_targetPid = processId;
    m_state = CaptureState::Starting;
    printf("[COnVoiceCapture] state = Starting\n");

    // AudioCaptureEngine 에게 PID, 콜백(this) 전달
    HRESULT hr = m_pEngine->Start(static_cast<DWORD>(processId), this);

    if (SUCCEEDED(hr))
    {
        m_state = CaptureState::Capturing;
        printf("[COnVoiceCapture] StartCapture OK -> state = Capturing\n");
    }
    else
    {
        printf("[COnVoiceCapture] StartCapture FAILED (HR=0x%08X) -> state = Stopped\n", hr);
        m_state = CaptureState::Stopped;
        m_targetPid = 0;
        // 실패 시 GIT 포인터도 버려둔다 (다음 StartCapture에서 다시 세팅)
        m_gitSinks.clear();
    }

    return hr;
}

// 캡처 중지
STDMETHODIMP COnVoiceCapture::StopCapture()
{
    printf("[COnVoiceCapture] StopCapture (current state=%ld)\n",
        static_cast<LONG>(m_state));

    if (m_state == CaptureState::Stopped || m_state == CaptureState::Stopping)
    {
        printf("[COnVoiceCapture] already stopped/stopping\n");
        return S_OK;
    }

    m_state = CaptureState::Stopping;
    printf("[COnVoiceCapture] state = Stopping\n");

    HRESULT hr = S_OK;

    if (m_pEngine)
    {
        hr = m_pEngine->Stop();
        if (SUCCEEDED(hr))
        {
            printf("[COnVoiceCapture] AudioCaptureEngine Stop() OK\n");
        }
        else
        {
            printf("[COnVoiceCapture] AudioCaptureEngine Stop() FAILED (HR=0x%08X)\n", hr);
        }
    }
    else
    {
        printf("[COnVoiceCapture] no engine (nothing to stop)\n");
    }

    m_state = CaptureState::Stopped;
    m_targetPid = 0;
    printf("[COnVoiceCapture] state = Stopped\n");

    // 이벤트 싱크용 GIT 포인터 정리
    m_gitSinks.clear();

    return hr;
}

// 상태 확인
STDMETHODIMP COnVoiceCapture::GetCaptureState(LONG* pState)
{
    if (!pState)
        return E_POINTER;

    *pState = static_cast<LONG>(m_state);
    printf("[COnVoiceCapture] GetCaptureState -> %ld\n", *pState);

    return S_OK;
}

// ========================================
// IAudioDataCallback 구현
//  - AudioCaptureEngine → OnVoiceCapture 로 PCM 전달
// ========================================
void COnVoiceCapture::OnAudioData(BYTE* pData, UINT32 dataSize)
{
    printf("[COnVoiceCapture] OnAudioData (size=%u bytes)\n", dataSize);

    HRESULT hrEvent = Fire_OnAudioData(pData, dataSize);
    if (SUCCEEDED(hrEvent))
    {
        printf("[COnVoiceCapture] Fire_OnAudioData OK\n");
    }
    else
    {
        printf("[COnVoiceCapture] Fire_OnAudioData FAILED (HR=0x%08X)\n", hrEvent);
    }
}

// ========================================
// 이벤트 헬퍼
//  - PCM → SAFEARRAY(VT_UI1) → OnAudioData(Byte[]) 호출
//  - 오디오 스레드(MTA) 에서 호출되며, GIT 프록시를 통해
//    스크립트 쪽 아파트먼트로 마샬링됨
// ========================================
HRESULT COnVoiceCapture::Fire_OnAudioData(BYTE* pData, UINT32 dataSize)
{
    if (!pData || dataSize == 0)
        return S_OK;

    DWORD currentTid = GetCurrentThreadId();
    printf("[COnVoiceCapture] Fire_OnAudioData enter (size=%u, ownerTid=%lu, currentTid=%lu)\n",
        dataSize, m_ownerThreadId, currentTid);

    // SAFEARRAY(VT_UI1) 생성
    SAFEARRAYBOUND sab;
    sab.lLbound = 0;
    sab.cElements = dataSize;

    SAFEARRAY* psa = SafeArrayCreate(VT_UI1, 1, &sab);
    if (!psa)
    {
        printf("[COnVoiceCapture] SafeArrayCreate failed\n");
        return E_OUTOFMEMORY;
    }

    BYTE* pArrayData = nullptr;
    HRESULT hr = SafeArrayAccessData(psa, reinterpret_cast<void**>(&pArrayData));
    if (FAILED(hr))
    {
        printf("[COnVoiceCapture] SafeArrayAccessData failed (HR=0x%08X)\n", hr);
        SafeArrayDestroy(psa);
        return hr;
    }

    memcpy(pArrayData, pData, dataSize);
    SafeArrayUnaccessData(psa);

    VARIANT varAudio;
    VariantInit(&varAudio);
    varAudio.vt = VT_ARRAY | VT_UI1;
    varAudio.parray = psa;

    const DISPID dispidOnAudioData = 1;   // IDL에서 [id(1)] OnAudioData(Byte[] data)

    const int nConnections = static_cast<int>(m_gitSinks.size());
    printf("[COnVoiceCapture] Fire_OnAudioData: nConnections=%d\n", nConnections);

    HRESULT hrAll = S_OK;

    for (int i = 0; i < nConnections; ++i)
    {
        CComPtr<IDispatch> spDispatch;

        // 🔥 GIT에서 이 스레드에서 사용할 IDispatch 프록시 꺼내기
        HRESULT hrGit = m_gitSinks[i].CopyTo(&spDispatch);
        printf("  [conn %d] GIT CopyTo HR=0x%08X, IDispatch*=%p\n",
            i, hrGit, (IDispatch*)spDispatch);

        if (FAILED(hrGit) || !spDispatch)
            continue;

        DISPPARAMS dp = {};
        dp.rgvarg = &varAudio;
        dp.cArgs = 1;

        EXCEPINFO ex = {};
        UINT argErr = 0;

        HRESULT hrInvoke = spDispatch->Invoke(
            dispidOnAudioData,
            IID_NULL,
            LOCALE_USER_DEFAULT,
            DISPATCH_METHOD,
            &dp,
            nullptr,
            &ex,
            &argErr
        );

        printf("  [conn %d] Invoke HR=0x%08X, argErr=%u\n", i, hrInvoke, argErr);

        if (FAILED(hrInvoke))
        {
            if (ex.bstrSource || ex.bstrDescription)
            {
                wprintf(L"  [conn %d] EXCEP Source=%s, Desc=%s\n",
                    i,
                    ex.bstrSource ? ex.bstrSource : L"(null)",
                    ex.bstrDescription ? ex.bstrDescription : L"(null)");
            }
        }

        if (ex.bstrSource)    SysFreeString(ex.bstrSource);
        if (ex.bstrDescription)SysFreeString(ex.bstrDescription);
        if (ex.bstrHelpFile)  SysFreeString(ex.bstrHelpFile);

        if (FAILED(hrInvoke) && SUCCEEDED(hrAll))
        {
            hrAll = hrInvoke;
        }
    }

    // VariantClear 가 SAFEARRAY까지 함께 해제해줌
    VariantClear(&varAudio);

    printf("[COnVoiceCapture] Fire_OnAudioData leave (HR=0x%08X)\n", hrAll);
    return hrAll;
}
