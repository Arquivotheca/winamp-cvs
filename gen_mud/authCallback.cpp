#include "main.h"
#include "./authCallback.h"
#include "./api.h"
#include "./config.h"
#include "./jsapi2_callbackmanager.h"

OpenAuthCallback::OpenAuthCallback() 
	: ref(1)
{
}

OpenAuthCallback::~OpenAuthCallback()
{
}

HRESULT OpenAuthCallback::CreateInstance(OpenAuthCallback **instance)
{
	if (NULL == instance) return E_POINTER;

	*instance = new OpenAuthCallback();
	if (NULL == instance) return E_OUTOFMEMORY;

	return S_OK;
}

size_t OpenAuthCallback::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t OpenAuthCallback::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int OpenAuthCallback::QueryInterface(GUID interface_guid, void **object)
{
	return E_NOTIMPL;
}

FOURCC OpenAuthCallback::GetEventType()
{
	return SysCallback::AUTH;
}

int OpenAuthCallback::Notify(int msg, intptr_t param1, intptr_t param2)
{
	switch (msg) 
	{
		case AuthCallback::CREDENTIALS_ABOUTTOCHANGE:
			CredentialsAboutToChange(reinterpret_cast<api_auth*>(param1), reinterpret_cast<const GUID *>(param2));
			break;
		case AuthCallback::CREDENTIALS_CHANGED:
			CredentialsChanged(reinterpret_cast<api_auth*>(param1), reinterpret_cast<const GUID *>(param2));
			break;
	}
	return 0;
}

void OpenAuthCallback::CredentialsAboutToChange(api_auth *auth, const GUID *realm)
{
}

void OpenAuthCallback::CredentialsChanged(api_auth *auth, const GUID *realm)
{
	if (IsEqualGUID(*realm, ORGLER_AUTH_REALM))
	{
		auth->GetCredentials(*realm, session_key, sizeof(session_key), token_a, sizeof(token_a), config_username, sizeof(config_username), &session_expiration);
		JSAPI2::callbackManager.OnStatusChange(GetLoginStatus());
	}
}

#define CBCLASS OpenAuthCallback
START_DISPATCH;
  CB(ADDREF, AddRef);
  CB(RELEASE, Release);
  CB(QUERYINTERFACE, QueryInterface);
  CB(SYSCALLBACK_GETEVENTTYPE, GetEventType);
  CB(SYSCALLBACK_NOTIFY, Notify);
END_DISPATCH;
#undef CBCLASS
