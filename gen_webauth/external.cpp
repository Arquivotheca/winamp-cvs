#include "main.h"
#include "./external.h"

#include "../winamp/jsapi.h"
#include "../winamp/jsapi_CallbackParameters.h"

#define DISPTABLE_CLASS	 WebAuthDispatch

DISPTABLE_BEGIN()
	DISPENTRY_ADD(DISPATCH_TRANSMIT_TOKEN, L"transmitTokenToClient", OnTransmitToken)
	DISPENTRY_ADD(DISPATCH_TRANSMIT_STATUS, L"transmitStatusToClient", OnTransmitStatus)
DISPTABLE_END

#undef DISPTABLE_CLASS


WebAuthDispatch::WebAuthDispatch() 
	: ref(1)
{
}

WebAuthDispatch::~WebAuthDispatch()
{
}

HRESULT WebAuthDispatch::CreateInstance(WebAuthDispatch **instance)
{
	if (NULL == instance) return E_POINTER;
	
	*instance = new WebAuthDispatch();
	if (NULL == *instance) return E_OUTOFMEMORY;
	
	return S_OK;
}

LPCWSTR WebAuthDispatch::GetName()
{
	return L"WebAuth";
}

ULONG WebAuthDispatch::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}


ULONG WebAuthDispatch::Release(void)
{ 
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP WebAuthDispatch::QueryInterface(REFIID riid, void **ppvObject)
{
	if (NULL == ppvObject) return E_POINTER;

	if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = static_cast<IDispatch*>(this);
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = static_cast<IUnknown*>(this);
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

HRESULT WebAuthDispatch::OnTransmitToken(WORD wFlags, 
										 DISPPARAMS FAR *pdispparams, 
										 VARIANT FAR *pvarResult, 
										 unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
	
	BSTR token;
	JSAPI_GETSTRING(token, pdispparams, 1, puArgErr);
	
	MessageBox(plugin.hwndParent, token, L"OnTransmitToken", MB_OK);
	JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);

	return S_OK;
}

HRESULT WebAuthDispatch::OnTransmitStatus(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
	
	BSTR token;
	JSAPI_GETSTRING(token, pdispparams, 1, puArgErr);
	
	MessageBox(plugin.hwndParent, token, L"OnTransmitStatus", MB_OK);
	JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);

	return S_OK;
}
