

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 12:14:07 2038
 */
/* Compiler settings for OnVoiceAudioBridge.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0628 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __OnVoiceAudioBridge_i_h__
#define __OnVoiceAudioBridge_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef DECLSPEC_XFGVIRT
#if defined(_CONTROL_FLOW_GUARD_XFG)
#define DECLSPEC_XFGVIRT(base, func) __declspec(xfg_virtual(base, func))
#else
#define DECLSPEC_XFGVIRT(base, func)
#endif
#endif

/* Forward Declarations */ 

#ifndef __IOnVoiceCapture_FWD_DEFINED__
#define __IOnVoiceCapture_FWD_DEFINED__
typedef interface IOnVoiceCapture IOnVoiceCapture;

#endif 	/* __IOnVoiceCapture_FWD_DEFINED__ */


#ifndef ___IOnVoiceCaptureEvents_FWD_DEFINED__
#define ___IOnVoiceCaptureEvents_FWD_DEFINED__
typedef interface _IOnVoiceCaptureEvents _IOnVoiceCaptureEvents;

#endif 	/* ___IOnVoiceCaptureEvents_FWD_DEFINED__ */


#ifndef __OnVoiceCapture_FWD_DEFINED__
#define __OnVoiceCapture_FWD_DEFINED__

#ifdef __cplusplus
typedef class OnVoiceCapture OnVoiceCapture;
#else
typedef struct OnVoiceCapture OnVoiceCapture;
#endif /* __cplusplus */

#endif 	/* __OnVoiceCapture_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "shobjidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IOnVoiceCapture_INTERFACE_DEFINED__
#define __IOnVoiceCapture_INTERFACE_DEFINED__

/* interface IOnVoiceCapture */
/* [unique][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IOnVoiceCapture;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("43a468da-7889-46c9-99de-38cb93e4e649")
    IOnVoiceCapture : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE StartCapture( 
            /* [in] */ LONG processId) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE StopCapture( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCaptureState( 
            /* [retval][out] */ LONG *pState) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IOnVoiceCaptureVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOnVoiceCapture * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOnVoiceCapture * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOnVoiceCapture * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IOnVoiceCapture * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IOnVoiceCapture * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IOnVoiceCapture * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IOnVoiceCapture * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IOnVoiceCapture, StartCapture)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *StartCapture )( 
            IOnVoiceCapture * This,
            /* [in] */ LONG processId);
        
        DECLSPEC_XFGVIRT(IOnVoiceCapture, StopCapture)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *StopCapture )( 
            IOnVoiceCapture * This);
        
        DECLSPEC_XFGVIRT(IOnVoiceCapture, GetCaptureState)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetCaptureState )( 
            IOnVoiceCapture * This,
            /* [retval][out] */ LONG *pState);
        
        END_INTERFACE
    } IOnVoiceCaptureVtbl;

    interface IOnVoiceCapture
    {
        CONST_VTBL struct IOnVoiceCaptureVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOnVoiceCapture_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IOnVoiceCapture_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IOnVoiceCapture_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IOnVoiceCapture_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IOnVoiceCapture_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IOnVoiceCapture_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IOnVoiceCapture_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IOnVoiceCapture_StartCapture(This,processId)	\
    ( (This)->lpVtbl -> StartCapture(This,processId) ) 

#define IOnVoiceCapture_StopCapture(This)	\
    ( (This)->lpVtbl -> StopCapture(This) ) 

#define IOnVoiceCapture_GetCaptureState(This,pState)	\
    ( (This)->lpVtbl -> GetCaptureState(This,pState) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IOnVoiceCapture_INTERFACE_DEFINED__ */



#ifndef __OnVoiceAudioBridgeLib_LIBRARY_DEFINED__
#define __OnVoiceAudioBridgeLib_LIBRARY_DEFINED__

/* library OnVoiceAudioBridgeLib */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_OnVoiceAudioBridgeLib;

#ifndef ___IOnVoiceCaptureEvents_DISPINTERFACE_DEFINED__
#define ___IOnVoiceCaptureEvents_DISPINTERFACE_DEFINED__

/* dispinterface _IOnVoiceCaptureEvents */
/* [hidden][uuid] */ 


EXTERN_C const IID DIID__IOnVoiceCaptureEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("52b4a16b-9f83-4a3e-9240-4dd6676540ea")
    _IOnVoiceCaptureEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _IOnVoiceCaptureEventsVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _IOnVoiceCaptureEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _IOnVoiceCaptureEvents * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _IOnVoiceCaptureEvents * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _IOnVoiceCaptureEvents * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _IOnVoiceCaptureEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _IOnVoiceCaptureEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _IOnVoiceCaptureEvents * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } _IOnVoiceCaptureEventsVtbl;

    interface _IOnVoiceCaptureEvents
    {
        CONST_VTBL struct _IOnVoiceCaptureEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IOnVoiceCaptureEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define _IOnVoiceCaptureEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define _IOnVoiceCaptureEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define _IOnVoiceCaptureEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define _IOnVoiceCaptureEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define _IOnVoiceCaptureEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define _IOnVoiceCaptureEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___IOnVoiceCaptureEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_OnVoiceCapture;

#ifdef __cplusplus

class DECLSPEC_UUID("fe3c62ec-02f2-4c63-8266-d538a86fd7f9")
OnVoiceCapture;
#endif
#endif /* __OnVoiceAudioBridgeLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


