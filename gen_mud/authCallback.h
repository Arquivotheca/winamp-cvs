#ifndef NULLSOFT_ORGLER_AUTH_CALLBACK_HEADER
#define NULLSOFT_ORGLER_AUTH_CALLBACK_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <api/syscb/callbacks/syscb.h>
#include <api/syscb/callbacks/authcb.h>

class api_auth;

class OpenAuthCallback : public SysCallback
{
protected:
	OpenAuthCallback();
	~OpenAuthCallback();

public:
	static HRESULT CreateInstance(OpenAuthCallback **instance);

public:
	/*** Dispatchable ***/
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/*** SysCallback ***/
	FOURCC GetEventType();
    int Notify(int msg, intptr_t param1, intptr_t param2);

protected:
	void CredentialsAboutToChange(api_auth *auth, const GUID *realm);
	void CredentialsChanged(api_auth *auth, const GUID *realm);

protected:
	size_t ref;

protected:
	RECVS_DISPATCH;
};


#endif //NULLSOFT_ORGLER_AUTH_CALLBACK_HEADER
