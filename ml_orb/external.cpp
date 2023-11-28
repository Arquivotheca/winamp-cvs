#include "main.h"
#include "./resource.h"
#include "./external.h"
#include "./wasabi.h"

#include "../winamp/jsapi.h"

#include <strsafe.h>

#define DISPTABLE_CLASS	 ExternalDispatch
DISPTABLE_BEGIN()
	DISPENTRY_ADD(DISPATCH_GETUNIQUEID, L"GetUniqueID", OnGetUniqueId)
	DISPENTRY_ADD(DISPATCH_GETUNIQUEID, L"GetSessionID", OnGetSessionId)
	DISPENTRY_ADD(DISPATCH_GETUNIQUEID, L"PlayUrl", OnPlayUrl)
DISPTABLE_END
#undef DISPTABLE_CLASS

ExternalDispatch::ExternalDispatch() 
	: ref(1)
{
}

ExternalDispatch::~ExternalDispatch()
{
}

LPCWSTR ExternalDispatch::GetName()
{
	return L"Orb";
}

HRESULT ExternalDispatch::CreateInstance(ExternalDispatch **instance)
{
	if (NULL == instance) return E_POINTER;
	
	*instance = new ExternalDispatch();
	if (NULL == *instance) return E_OUTOFMEMORY;
	
	return S_OK;
}


ULONG ExternalDispatch::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}


ULONG ExternalDispatch::Release(void)
{ 
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP ExternalDispatch::QueryInterface(REFIID riid, void **ppvObject)
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

HRESULT ExternalDispatch::OnGetUniqueId(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
	
	if (NULL != pvarResult)
	{
		WCHAR szBuffer[256], *result;
		if (NULL != OMBROWSERMNGR && SUCCEEDED(OMBROWSERMNGR->GetClientId(szBuffer, ARRAYSIZE(szBuffer))))
			result = szBuffer;
		else
			result = L"";

		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_BSTR;
		V_BSTR(pvarResult) = SysAllocString(result);
	}
		
	return S_OK;
}

HRESULT ExternalDispatch::OnGetSessionId(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);

	if (NULL != pvarResult)
	{
		WCHAR szBuffer[256], *result;
		if (NULL != OMBROWSERMNGR && SUCCEEDED(OMBROWSERMNGR->GetSessionId(szBuffer, ARRAYSIZE(szBuffer))))
			result = szBuffer;
		else
			result = L"";

		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_BSTR;
		V_BSTR(pvarResult) = SysAllocString(result);
	}
	
	
	return S_OK;
}

HRESULT ExternalDispatch::OnPlayUrl(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);

	HRESULT hr;
	LPCWSTR url;

	JSAPI_GETSTRING(url, pdispparams, 1, puArgErr);

	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary)
	{
		hr = E_UNEXPECTED;
	}
	else
	{
		WCHAR szBuffer[2048], *end;
		size_t remaining;
		hr = StringCchCopyEx(szBuffer, ARRAYSIZE(szBuffer), url, &end, &remaining, STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE);
	
		if (SUCCEEDED(hr) && NULL != end && remaining > 0)
		{
			end[1] = L'\0'; // double null terminate
			mlSendToWinampStruct send;
			ZeroMemory(&send, sizeof(send));
			send.type = ML_TYPE_STREAMNAMESW;
			send.enqueue = 0;
			send.data = (void*)szBuffer;
					
			SENDMLIPC(hLibrary, ML_IPC_SENDTOWINAMP, (WPARAM)&send);
		}
	}

	if (NULL != pvarResult)
	{
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_BOOL;
		V_BOOL(pvarResult) = (SUCCEEDED(hr) ? VARIANT_TRUE : VARIANT_FALSE);
	}

	return S_OK;
}