#include "JSAPI2_MUDAPI.h"
#include "main.h"
#include "config.h"
#include "../Winamp/JSAPI.h"
#include "api.h"
#include "../Winamp/JSAPI_CallbackParameters.h"
#include "JSAPI2_CallbackManager.h"

JSAPI2::MUDAPI::MUDAPI(const wchar_t *_key, JSAPI::ifc_info *_info)
{
	info = _info;
	key = _key;
	refCount = 1;
}

JSAPI2::MUDAPI::~MUDAPI()
{
		// just in case someone forgot
	JSAPI2::callbackManager.Deregister(this);
}

#define DISP_TABLE \
	CHECK_ID(enabled)\
	CHECK_ID(OpenOptions)\
	CHECK_ID(Login)\
	CHECK_ID(Logout)\
	CHECK_ID(login_status)\
	CHECK_ID(version)\
	CHECK_ID(RegisterForEvents)\
	CHECK_ID(UnregisterFromEvents)\

#define CHECK_ID(str) JSAPI_DISP_ENUMIFY(str),
enum { 
	DISP_TABLE 
};

#undef CHECK_ID
#define CHECK_ID(str) 		if (wcscmp(rgszNames[i], L## #str) == 0)	{		rgdispid[i] = JSAPI_DISP_ENUMIFY(str); continue; }
HRESULT JSAPI2::MUDAPI::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		DISP_TABLE

			rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;

	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT JSAPI2::MUDAPI::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::MUDAPI::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::MUDAPI::enabled(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (wFlags & DISPATCH_PROPERTYPUT)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
		JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BOOL, puArgErr);
		if (AGAVE_API_JSAPI2_SECURITY->GetActionAuthorization(L"MUD", L"enabled", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
		{
			config_collect = JSAPI_PARAM(pdispparams, 1).boolVal == VARIANT_TRUE;
			Config_SyncEnabled();
		}
		return S_OK;
	}
	else if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_BOOL;
		V_BOOL(pvarResult) = config_collect?VARIANT_TRUE:VARIANT_FALSE;
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;

	return E_FAIL;	
}

HRESULT JSAPI2::MUDAPI::OpenOptions(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	if (AGAVE_API_JSAPI2_SECURITY->GetActionAuthorization(L"MUD", L"openoptions", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		PostMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&g_prefsItem, IPC_OPENPREFSTOPAGE);
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
	}
	else
	{
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
	}

	return S_OK;
}

HRESULT JSAPI2::MUDAPI::Login(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	if (AGAVE_API_JSAPI2_SECURITY->GetActionAuthorization(L"MUD", L"login", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		if (GetLoginStatus() != LOGIN_LOGGEDIN)
		{
			::Login(NULL, session_key, 512, token_a, 512);
			Config_SyncLogin();
		}
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
	}
	else
	{
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
	}

	return S_OK;
}

HRESULT JSAPI2::MUDAPI::Logout(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	if (AGAVE_API_JSAPI2_SECURITY->GetActionAuthorization(L"MUD", L"logout", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		if (GetLoginStatus() == LOGIN_LOGGEDIN)
		{
			::Logout();
			Config_SyncLogin();
		}
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
	}
	else
	{
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
	}

	return S_OK;
}

HRESULT JSAPI2::MUDAPI::login_status(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_I4;
		V_I4(pvarResult) = GetLoginStatus();
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;

	return E_FAIL;	
}

HRESULT JSAPI2::MUDAPI::version(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_I4;
		LONG version = VERSION_MAJOR * 100;
		if (VERSION_MINOR < 10)
			version += VERSION_MINOR*10;
		else
			version += VERSION_MINOR;
		V_I4(pvarResult) = version;
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;

	return E_FAIL;	
}

#undef CHECK_ID
#define CHECK_ID(str) 		case JSAPI_DISP_ENUMIFY(str): return str(wFlags, pdispparams, pvarResult, puArgErr);
HRESULT JSAPI2::MUDAPI::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
		DISP_TABLE
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP JSAPI2::MUDAPI::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG JSAPI2::MUDAPI::AddRef(void)
{
	return InterlockedIncrement(&refCount);
}


ULONG JSAPI2::MUDAPI::Release(void)
{
	LONG lRef = InterlockedDecrement(&refCount);
	if (lRef == 0) delete this;
	return lRef;
}

void JSAPI2::MUDAPI::OnStatusChange(int new_status)
{
	for (size_t i=0;i!=events.size();i++)
	{
		IDispatch *invokee = events[i];
		JSAPI::CallbackParameters *parameters = new JSAPI::CallbackParameters ;
		parameters->AddString(L"event", L"OnStatusChange");
		parameters->AddLong(L"login_status", new_status);
try
{
		JSAPI::InvokeEvent(parameters, invokee);
}
catch(...)
{
	events.eraseindex(i--);
	if (events.empty())
		JSAPI2::callbackManager.Deregister(this);
}
	}
}

HRESULT JSAPI2::MUDAPI::RegisterForEvents(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_DISPATCH, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	switch (AGAVE_API_JSAPI2_SECURITY->GetActionAuthorization(L"MUD", L"events", key, info, JSAPI2::api_security::ACTION_PROMPT))
	{
	case JSAPI2::api_security::ACTION_DISALLOWED:
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
		break;
	case JSAPI2::api_security::ACTION_ALLOWED:
		{
			/** if this is the first time someone is registering an event
			** add ourselves to the callback manager
			*/
			if (events.empty())
				JSAPI2::callbackManager.Register(this);
			IDispatch *event = JSAPI_PARAM(pdispparams, 1).pdispVal;
			// TODO: benski> not sure, but we might need to: event->AddRef();    
			events.push_back(event);
			JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
		}
		break;
	}
	return S_OK;
}

HRESULT JSAPI2::MUDAPI::UnregisterFromEvents(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_DISPATCH, puArgErr);

	IDispatch *event = JSAPI_PARAM(pdispparams, 1).pdispVal;
	// TODO: benski> not sure, but we might need to: event->Release(); 
	events.eraseAll(event);
	/** if we don't have any more event listeners
	** remove ourselves from the callback manager
	*/
	if (events.empty())
		JSAPI2::callbackManager.Deregister(this);

	return S_OK;
}
