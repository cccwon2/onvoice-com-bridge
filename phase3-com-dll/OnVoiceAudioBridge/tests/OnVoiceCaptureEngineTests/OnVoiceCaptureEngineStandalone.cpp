#include "pch.h"
#include <windows.h>
#include <iostream>
#include <vector>
#include <cassert>

#include "../../OnVoiceCapture.h"
#include "../mocks/MockAudioCaptureEngine.h"

// Minimal fake sink implementing IDispatch so m_vec can accept it if needed
class DummyEventSink : public IDispatch {
public:
    DummyEventSink() : ref(1) {}
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override { return E_NOINTERFACE; }
    STDMETHODIMP_(ULONG) AddRef() override { return ++ref; }
    STDMETHODIMP_(ULONG) Release() override { ULONG r=--ref; if(r==0) delete this; return r; }
    STDMETHODIMP GetTypeInfoCount(UINT* pctinfo) override { return E_NOTIMPL; }
    STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override { return E_NOTIMPL; }
    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) override { return E_NOTIMPL; }
    STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) override {
        // Print size if audio event invoked
        if (pDispParams && pDispParams->cArgs == 1 && pDispParams->rgvarg[0].vt == (VT_ARRAY | VT_UI1)) {
            SAFEARRAY* psa = pDispParams->rgvarg[0].parray;
            BYTE* data = nullptr;
            SafeArrayAccessData(psa, reinterpret_cast<void**>(&data));
            LONG lBound=0, uBound=0;
            SafeArrayGetLBound(psa, 1, &lBound);
            SafeArrayGetUBound(psa, 1, &uBound);
            LONG size = uBound - lBound + 1;
            std::cout << "DummyEventSink::Invoke received audio size=" << size << "\n";
            SafeArrayUnaccessData(psa);
        }
        return S_OK;
    }
private:
    ULONG ref;
};

int main()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (!SUCCEEDED(hr) && hr != RPC_E_CHANGED_MODE) {
        std::cerr << "CoInitializeEx failed: " << std::hex << hr << std::endl;
        return 1;
    }

    CComObject<COnVoiceCapture>* obj = nullptr;
    HRESULT h = CComObject<COnVoiceCapture>::CreateInstance(&obj);
    assert(h == S_OK);
    obj->AddRef();

    // inject mock engine
    MockAudioCaptureEngine* mockEngine = new MockAudioCaptureEngine();
    obj->SetAudioCaptureEngine(mockEngine, false);

    // Test invalid PID
    HRESULT hrStartInvalid = obj->StartCapture(0);
    assert(hrStartInvalid == E_INVALIDARG);
    std::cout << "StartCapture(0) returned E_INVALIDARG as expected" << std::endl;

    // StartCapture with current PID should succeed (OpenProcess will succeed)
    DWORD pid = GetCurrentProcessId();
    HRESULT hrStart = obj->StartCapture(static_cast<LONG>(pid));
    if (hrStart != S_OK) {
        std::cerr << "StartCapture failed: 0x" << std::hex << hrStart << std::endl;
    }
    assert(hrStart == S_OK);
    std::cout << "StartCapture succeeded" << std::endl;

    // Emit fake audio from mock
    std::vector<BYTE> data = {0x11, 0x22, 0x33, 0x44};
    mockEngine->EmitAudio(data);
    std::cout << "Mock emitted audio" << std::endl;

    // Stop
    HRESULT hrStop = obj->StopCapture();
    assert(hrStop == S_OK);
    std::cout << "StopCapture succeeded" << std::endl;

    // Cleanup
    obj->Release();
    delete mockEngine;
    CoUninitialize();

    return 0;
}
