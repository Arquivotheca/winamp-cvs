#include "main.h"
#include "JSAPI2_CloudAPI.h"
#include "../nu/MediaLibraryInterface.h"
#include "../Winamp/JSAPI.h"
#include "api.h"
#include "resource.h"

JSAPI2::CloudAPI::CloudAPI(const wchar_t *_key, JSAPI::ifc_info *_info)
{
	info = _info;
	key = _key;
	refCount = 1;
}

JSAPI2::CloudAPI::~CloudAPI()
{
}

#define DISP_TABLE \
	CHECK_ID(provider)\
	CHECK_ID(authtoken)\
	CHECK_ID(username)\
	CHECK_ID(ShowCloudPrefs)\
	CHECK_ID(ShowSigninPage)\
	CHECK_ID(ShowCloudSources)

#define CHECK_ID(str) JSAPI_DISP_ENUMIFY(str),
enum { 
	DISP_TABLE 
};

#undef CHECK_ID
#define CHECK_ID(str) if (wcscmp(rgszNames[i], L## #str) == 0) { rgdispid[i] = JSAPI_DISP_ENUMIFY(str); continue; }
HRESULT JSAPI2::CloudAPI::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		DISP_TABLE rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;
	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT JSAPI2::CloudAPI::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::CloudAPI::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::CloudAPI::PropertyFromConfig(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr, Config_StringGetter getter, Config_StringSetter setter)
{
	if (wFlags & DISPATCH_PROPERTYPUT)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
		JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr);
		if (AGAVE_API_JSAPI2_SECURITY->GetActionAuthorization(L"Cloud", L"property", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
		{
			setter(JSAPI_PARAM(pdispparams, 1).bstrVal);
		}
		return S_OK;
	}
	else if (wFlags & DISPATCH_PROPERTYGET)
	{
		wchar_t local_string[128];
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_BSTR;
		getter(local_string, 128);
		V_BSTR(pvarResult) = SysAllocString(local_string);
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;
}

HRESULT JSAPI2::CloudAPI::provider(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	HRESULT hr = PropertyFromConfig(wFlags, pdispparams, pvarResult, puArgErr, Config_GetProvider, Config_SetProvider);
	SetCredentials();
	return hr;
}

HRESULT JSAPI2::CloudAPI::authtoken(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	HRESULT hr= PropertyFromConfig(wFlags, pdispparams, pvarResult, puArgErr, Config_GetAuthToken, Config_SetAuthToken);
	SetCredentials();
	return hr;
}

HRESULT JSAPI2::CloudAPI::username(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	HRESULT hr = PropertyFromConfig(wFlags, pdispparams, pvarResult, puArgErr, Config_GetUsername, Config_SetUsername);
	SetCredentials();
	return hr;
}

HRESULT JSAPI2::CloudAPI::ShowCloudPrefs(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	if (AGAVE_API_JSAPI2_SECURITY->GetActionAuthorization(L"Cloud", L"debug_console", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		mediaLibrary.GoToPreferences(preferences._id);
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
	}
	else
	{
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
	}

	return S_OK;
}

HRESULT JSAPI2::CloudAPI::ShowSigninPage(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	if (AGAVE_API_JSAPI2_SECURITY->GetActionAuthorization(L"Cloud", L"debug_console", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		HNAVITEM item = MLNavCtrl_FindItemById(plugin.hwndLibraryParent, (signin_treeItem ? signin_treeItem : cloud_treeItem));
		MLNavItem_Select(plugin.hwndLibraryParent, item);
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
	}
	else
	{
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
	}

	return S_OK;
}

HRESULT JSAPI2::CloudAPI::ShowCloudSources(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	if (AGAVE_API_JSAPI2_SECURITY->GetActionAuthorization(L"Cloud", L"debug_console", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		HNAVITEM item = MLNavCtrl_FindItemById(plugin.hwndLibraryParent, cloud_treeItem);
		MLNavItem_Select(plugin.hwndLibraryParent, item);
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
	}
	else
	{
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
	}

	return S_OK;
}

#undef CHECK_ID
#define CHECK_ID(str) case JSAPI_DISP_ENUMIFY(str): return str(wFlags, pdispparams, pvarResult, puArgErr);
HRESULT JSAPI2::CloudAPI::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
		DISP_TABLE
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP JSAPI2::CloudAPI::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	else if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = (IDispatch *)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = this;
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG JSAPI2::CloudAPI::AddRef(void)
{
	return InterlockedIncrement(&refCount);
}

ULONG JSAPI2::CloudAPI::Release(void)
{
	LONG lRef = InterlockedDecrement(&refCount);
	if (lRef == 0) delete this;
	return lRef;
}