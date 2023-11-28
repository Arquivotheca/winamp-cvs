

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0365 */
/* Compiler settings for msnetobj.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __msnetobj_h__
#define __msnetobj_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IRMGetLicense_FWD_DEFINED__
#define __IRMGetLicense_FWD_DEFINED__
typedef interface IRMGetLicense IRMGetLicense;
#endif 	/* __IRMGetLicense_FWD_DEFINED__ */


#ifndef __RMGetLicense_FWD_DEFINED__
#define __RMGetLicense_FWD_DEFINED__

#ifdef __cplusplus
typedef class RMGetLicense RMGetLicense;
#else
typedef struct RMGetLicense RMGetLicense;
#endif /* __cplusplus */

#endif 	/* __RMGetLicense_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

/* interface __MIDL_itf_msnetobj_0000 */
/* [local] */ 

//=========================================================================
//
// Microsoft Windows Media Technologies
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
//=========================================================================


extern RPC_IF_HANDLE __MIDL_itf_msnetobj_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msnetobj_0000_v0_0_s_ifspec;

#ifndef __IRMGetLicense_INTERFACE_DEFINED__
#define __IRMGetLicense_INTERFACE_DEFINED__

/* interface IRMGetLicense */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IRMGetLicense;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9EA69F99-F8FF-415E-8B90-35D6DFAF160E")
    IRMGetLicense : public IDispatch
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetLicenseFromURL( 
            /* [in] */ BSTR bstrXMLDoc,
            /* [in] */ BSTR bstrURL) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetSystemInfo( 
            /* [retval][out] */ BSTR *pbstrXMLDoc) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE StoreLicense( 
            /* [in] */ BSTR bstrXMLDoc) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDRMVersion( 
            /* [retval][out] */ BSTR *pbstrDRMVersion) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDRMSecurityVersion( 
            /* [retval][out] */ BSTR *pbstrDRMSecurityVersion) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetLicenseFromURLAsync( 
            /* [in] */ BSTR bstrXMLDoc,
            /* [in] */ BSTR bstrURL) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetAsyncCallStatus( 
            /* [out][in] */ VARIANT *pvarStatus,
            /* [out][in] */ VARIANT *pvarHResult) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRMGetLicenseVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRMGetLicense * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRMGetLicense * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRMGetLicense * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRMGetLicense * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRMGetLicense * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRMGetLicense * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRMGetLicense * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetLicenseFromURL )( 
            IRMGetLicense * This,
            /* [in] */ BSTR bstrXMLDoc,
            /* [in] */ BSTR bstrURL);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetSystemInfo )( 
            IRMGetLicense * This,
            /* [retval][out] */ BSTR *pbstrXMLDoc);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *StoreLicense )( 
            IRMGetLicense * This,
            /* [in] */ BSTR bstrXMLDoc);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetDRMVersion )( 
            IRMGetLicense * This,
            /* [retval][out] */ BSTR *pbstrDRMVersion);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetDRMSecurityVersion )( 
            IRMGetLicense * This,
            /* [retval][out] */ BSTR *pbstrDRMSecurityVersion);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetLicenseFromURLAsync )( 
            IRMGetLicense * This,
            /* [in] */ BSTR bstrXMLDoc,
            /* [in] */ BSTR bstrURL);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetAsyncCallStatus )( 
            IRMGetLicense * This,
            /* [out][in] */ VARIANT *pvarStatus,
            /* [out][in] */ VARIANT *pvarHResult);
        
        END_INTERFACE
    } IRMGetLicenseVtbl;

    interface IRMGetLicense
    {
        CONST_VTBL struct IRMGetLicenseVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRMGetLicense_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRMGetLicense_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRMGetLicense_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRMGetLicense_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRMGetLicense_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRMGetLicense_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRMGetLicense_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRMGetLicense_GetLicenseFromURL(This,bstrXMLDoc,bstrURL)	\
    (This)->lpVtbl -> GetLicenseFromURL(This,bstrXMLDoc,bstrURL)

#define IRMGetLicense_GetSystemInfo(This,pbstrXMLDoc)	\
    (This)->lpVtbl -> GetSystemInfo(This,pbstrXMLDoc)

#define IRMGetLicense_StoreLicense(This,bstrXMLDoc)	\
    (This)->lpVtbl -> StoreLicense(This,bstrXMLDoc)

#define IRMGetLicense_GetDRMVersion(This,pbstrDRMVersion)	\
    (This)->lpVtbl -> GetDRMVersion(This,pbstrDRMVersion)

#define IRMGetLicense_GetDRMSecurityVersion(This,pbstrDRMSecurityVersion)	\
    (This)->lpVtbl -> GetDRMSecurityVersion(This,pbstrDRMSecurityVersion)

#define IRMGetLicense_GetLicenseFromURLAsync(This,bstrXMLDoc,bstrURL)	\
    (This)->lpVtbl -> GetLicenseFromURLAsync(This,bstrXMLDoc,bstrURL)

#define IRMGetLicense_GetAsyncCallStatus(This,pvarStatus,pvarHResult)	\
    (This)->lpVtbl -> GetAsyncCallStatus(This,pvarStatus,pvarHResult)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRMGetLicense_GetLicenseFromURL_Proxy( 
    IRMGetLicense * This,
    /* [in] */ BSTR bstrXMLDoc,
    /* [in] */ BSTR bstrURL);


void __RPC_STUB IRMGetLicense_GetLicenseFromURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRMGetLicense_GetSystemInfo_Proxy( 
    IRMGetLicense * This,
    /* [retval][out] */ BSTR *pbstrXMLDoc);


void __RPC_STUB IRMGetLicense_GetSystemInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRMGetLicense_StoreLicense_Proxy( 
    IRMGetLicense * This,
    /* [in] */ BSTR bstrXMLDoc);


void __RPC_STUB IRMGetLicense_StoreLicense_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRMGetLicense_GetDRMVersion_Proxy( 
    IRMGetLicense * This,
    /* [retval][out] */ BSTR *pbstrDRMVersion);


void __RPC_STUB IRMGetLicense_GetDRMVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRMGetLicense_GetDRMSecurityVersion_Proxy( 
    IRMGetLicense * This,
    /* [retval][out] */ BSTR *pbstrDRMSecurityVersion);


void __RPC_STUB IRMGetLicense_GetDRMSecurityVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRMGetLicense_GetLicenseFromURLAsync_Proxy( 
    IRMGetLicense * This,
    /* [in] */ BSTR bstrXMLDoc,
    /* [in] */ BSTR bstrURL);


void __RPC_STUB IRMGetLicense_GetLicenseFromURLAsync_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IRMGetLicense_GetAsyncCallStatus_Proxy( 
    IRMGetLicense * This,
    /* [out][in] */ VARIANT *pvarStatus,
    /* [out][in] */ VARIANT *pvarHResult);


void __RPC_STUB IRMGetLicense_GetAsyncCallStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRMGetLicense_INTERFACE_DEFINED__ */



#ifndef __MSNETOBJLib_LIBRARY_DEFINED__
#define __MSNETOBJLib_LIBRARY_DEFINED__

/* library MSNETOBJLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_MSNETOBJLib;

EXTERN_C const CLSID CLSID_RMGetLicense;

#ifdef __cplusplus

class DECLSPEC_UUID("A9FC132B-096D-460B-B7D5-1DB0FAE0C062")
RMGetLicense;
#endif
#endif /* __MSNETOBJLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


