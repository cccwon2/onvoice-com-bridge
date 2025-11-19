// OnVoiceCapture.cpp : COnVoiceCapture 구현

#include "pch.h"
#include "OnVoiceCapture.h"
#include "ProcessHelper.h"

#include <stdio.h>
#include <atlcomcli.h>
#include <OleAuto.h>

// ========================================
// 소멸자: AudioCaptureEngine / GIT 정리
// ========================================
COnVoiceCapture::~COnVoiceCapture()
{
    printf("[COnVoiceCapture] Destructor start\n");

    // GIT에 등록된 이벤트 싱크 정리
    if (!m_gitSinks.empty())
    {
        printf("[COnVoiceCapture] Clearing GIT sinks (count=%zu)\n",
            m_gitSinks.size());
        // 각 CComGITPtr<IDispatch>의 소멸자가 Revoke()를 호출함
        m_gitSinks.clear();
    }

    if (m_pEngine)
    {
        printf("[COnVoiceCapture] AudioCaptureEngine Stop() 호출\n");
        m_pEngine->Stop();
        Sleep(200);

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
        printf("[COnVoiceCapture] ❌ 잘못된 PID (%ld)\n", processId);
        return E_INVALIDARG;
    }

    // 프로세스 존재 여부 검증
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, static_cast<DWORD>(processId));
    if (hProcess == NULL)
    {
        DWORD dwError = GetLastError();
        printf("[COnVoiceCapture] ❌ 프로세스 열기 실패 (PID=%ld, Error=%lu)\n", processId, dwError);
        return HRESULT_FROM_WIN32(dwError);
    }
    CloseHandle(hProcess);
    printf("[COnVoiceCapture] ✅ 프로세스 존재 확인 (PID=%ld)\n", processId);

    if (m_state == CaptureState::Starting || m_state == CaptureState::Capturing)
    {
        printf("[COnVoiceCapture] ❌ 이미 시작 중이거나 캡처 중 (state=%ld)\n",
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

    m_ownerThreadId = GetCurrentThreadId();

    // 🔥 현재 연결된 이벤트 싱크들을 GIT에 등록
    m_gitSinks.clear();
    {
        const int nConnections = m_vec.GetSize();
        printf("[COnVoiceCapture] Preparing GIT sinks, raw connections=%d\n", nConnections);

        for (int i = 0; i < nConnections; ++i)
        {
            IUnknown* pUnk = m_vec.GetAt(i);
            printf("  [GIT prep src %d] IUnknown*=%p\n", i, pUnk);
            if (!pUnk)
                continue;

            CComQIPtr<IDispatch> spDisp(pUnk);
            printf("  [GIT prep src %d] IDispatch*=%p\n", i, (IDispatch*)spDisp);
            if (!spDisp)
                continue;

            CComGITPtr<IDispatch> git;
            HRESULT hrGit = git.Attach(spDisp);
            printf("  [GIT prep src %d] Attach HR=0x%08X\n", i, hrGit);

            if (SUCCEEDED(hrGit))
            {
                m_gitSinks.push_back(git);  // ✅ Attach 성공한 것만 저장
            }
        }

        printf("[COnVoiceCapture] GIT sink count=%zu\n", m_gitSinks.size());
    }

    m_targetPid = processId;
    m_state = CaptureState::Starting;
    printf("[COnVoiceCapture] state = Starting\n");

    // AudioCaptureEngine 에게 PID, 콜백(this) 전달
    HRESULT hr = m_pEngine->Start(static_cast<DWORD>(processId), this);

    if (SUCCEEDED(hr))
    {
        m_state = CaptureState::Capturing;
        printf("[COnVoiceCapture] ✅ StartCapture 성공 -> state = Capturing\n");
    }
    else
    {
        printf("[COnVoiceCapture] ❌ StartCapture 실패 (HR=0x%08X) -> state = Stopped\n", hr);
        m_state = CaptureState::Stopped;
        m_targetPid = 0;
        m_gitSinks.clear();  // 실패 시 GIT도 정리
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
            printf("[COnVoiceCapture] ✅ AudioCaptureEngine Stop() 성공\n");
            Sleep(200);
        }
        else
        {
            printf("[COnVoiceCapture] ⚠️  AudioCaptureEngine Stop() 실패 (HR=0x%08X)\n", hr);
        }
    }
    else
    {
        printf("[COnVoiceCapture] 엔진 없음\n");
    }

    // GIT 싱크 정리
    if (!m_gitSinks.empty())
    {
        printf("[COnVoiceCapture] Clearing GIT sinks on StopCapture (count=%zu)\n",
            m_gitSinks.size());
        m_gitSinks.clear();
    }

    m_state = CaptureState::Stopped;
    m_targetPid = 0;
    printf("[COnVoiceCapture] state = Stopped\n");

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

// Chrome 브라우저 프로세스 찾기
STDMETHODIMP COnVoiceCapture::FindChromeProcess(LONG* pPid)
{
    if (!pPid)
        return E_POINTER;

    printf("[COnVoiceCapture] FindChromeProcess 호출\n");

    DWORD pid = FindChromeBrowserProcess();
    *pPid = static_cast<LONG>(pid);

    if (pid == 0)
    {
        printf("[COnVoiceCapture] ⚠️  Chrome 프로세스를 찾지 못했습니다.\n");
    }
    else
    {
        printf("[COnVoiceCapture] ✅ Chrome 프로세스 발견: PID %lu\n", pid);
    }

    return S_OK;
}

// Discord 프로세스 찾기
STDMETHODIMP COnVoiceCapture::FindDiscordProcess(LONG* pPid)
{
    if (!pPid)
        return E_POINTER;

    printf("[COnVoiceCapture] FindDiscordProcess 호출\n");

    DWORD pid = ::FindDiscordProcess();
    *pPid = static_cast<LONG>(pid);

    if (pid == 0)
    {
        printf("[COnVoiceCapture] ⚠️  Discord 프로세스를 찾지 못했습니다.\n");
    }
    else
    {
        printf("[COnVoiceCapture] ✅ Discord 프로세스 발견: PID %lu\n", pid);
    }

    return S_OK;
}

// ========================================
// IAudioDataCallback 구현
//  - AudioCaptureEngine → OnVoiceCapture 로 PCM 전달
// ========================================
void COnVoiceCapture::OnAudioData(BYTE* pData, UINT32 dataSize)
{
    if (!pData || dataSize == 0)
        return;

    // 너무 많은 로그 방지 (처음 10개만)
    static int callCount = 0;
    if (callCount < 10)
    {
        printf("[COnVoiceCapture] OnAudioData (size=%u bytes) [#%d]\n", dataSize, callCount);
        callCount++;
    }

    HRESULT hrEvent = Fire_OnAudioData(pData, dataSize);

    if (callCount <= 10)
    {
        if (SUCCEEDED(hrEvent))
        {
            printf("[COnVoiceCapture] Fire_OnAudioData OK\n");
        }
        else
        {
            printf("[COnVoiceCapture] Fire_OnAudioData FAILED (HR=0x%08X)\n", hrEvent);
        }
    }
}

// ========================================
// 이벤트 헬퍼
//  - PCM → SAFEARRAY(VT_UI1) → OnAudioData(Byte[]) 호출
// ========================================
HRESULT COnVoiceCapture::Fire_OnAudioData(BYTE* pData, UINT32 dataSize)
{
    if (!pData || dataSize == 0)
        return S_OK;

    // 디버그 로그 (처음 10개만)
    static int fireCount = 0;
    bool shouldLog = (fireCount < 10);

    if (shouldLog)
    {
        DWORD currentTid = GetCurrentThreadId();
        printf("[COnVoiceCapture] Fire_OnAudioData enter (size=%u, ownerTid=%lu, currentTid=%lu) [#%d]\n",
            dataSize, m_ownerThreadId, currentTid, fireCount);
        fireCount++;
    }

    // SAFEARRAY(VT_UI1) 생성
    SAFEARRAYBOUND sab;
    sab.lLbound = 0;
    sab.cElements = dataSize;

    SAFEARRAY* psa = SafeArrayCreate(VT_UI1, 1, &sab);
    if (!psa)
    {
        printf("[COnVoiceCapture] ❌ SafeArrayCreate failed\n");
        return E_OUTOFMEMORY;
    }

    BYTE* pArrayData = nullptr;
    HRESULT hr = SafeArrayAccessData(psa, reinterpret_cast<void**>(&pArrayData));
    if (FAILED(hr))
    {
        printf("[COnVoiceCapture] ❌ SafeArrayAccessData failed (HR=0x%08X)\n", hr);
        SafeArrayDestroy(psa);
        return hr;
    }

    memcpy(pArrayData, pData, dataSize);
    SafeArrayUnaccessData(psa);

    VARIANT varAudio;
    VariantInit(&varAudio);
    varAudio.vt = VT_ARRAY | VT_UI1;
    varAudio.parray = psa;

    const DISPID dispidOnAudioData = 1;

    const int nConnections = static_cast<int>(m_gitSinks.size());

    if (shouldLog)
    {
        printf("[COnVoiceCapture] Fire_OnAudioData: nConnections=%d\n", nConnections);
    }

    HRESULT hrAll = S_OK;

    for (int i = 0; i < nConnections; ++i)
    {
        CComPtr<IDispatch> spDispatch;

        // GIT에서 이 스레드에서 사용할 IDispatch 프록시 꺼내기
        HRESULT hrGit = m_gitSinks[i].CopyTo(&spDispatch);

        if (shouldLog)
        {
            printf("  [conn %d] GIT CopyTo HR=0x%08X, IDispatch*=%p\n",
                i, hrGit, (IDispatch*)spDispatch);
        }

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

        if (shouldLog)
        {
            printf("  [conn %d] Invoke HR=0x%08X, argErr=%u\n", i, hrInvoke, argErr);
        }

        if (FAILED(hrInvoke))
        {
            if (ex.bstrSource || ex.bstrDescription)
            {
                wprintf(L"  [conn %d] ❌ EXCEP Source=%s, Desc=%s\n",
                    i,
                    ex.bstrSource ? ex.bstrSource : L"(null)",
                    ex.bstrDescription ? ex.bstrDescription : L"(null)");
            }
        }

        if (ex.bstrSource)    SysFreeString(ex.bstrSource);
        if (ex.bstrDescription) SysFreeString(ex.bstrDescription);
        if (ex.bstrHelpFile)  SysFreeString(ex.bstrHelpFile);

        if (FAILED(hrInvoke) && SUCCEEDED(hrAll))
        {
            hrAll = hrInvoke;
        }
    }

    VariantClear(&varAudio);   // SAFEARRAY도 같이 해제됨

    if (shouldLog)
    {
        printf("[COnVoiceCapture] Fire_OnAudioData leave (HR=0x%08X)\n", hrAll);
    }

    return hrAll;
}
