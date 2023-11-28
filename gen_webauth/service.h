#pragma once

#include <wtypes.h>
#include "../omBrowser/ifc_omservice.h"

#define SERVICE_ID			123
#define SERVICE_HOMEURL		L"http://www.winamp.com/user/oaweblogin"

class OmService : public ifc_omservice
{

protected:
	OmService();
	~OmService();

public:
	static HRESULT CreateInstance(OmService **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_omservice */
	unsigned int GetId();
	HRESULT GetName(wchar_t *pszBuffer, int cchBufferMax);
	HRESULT GetUrl(wchar_t *pszBuffer, int cchBufferMax);
	HRESULT GetExternal(IDispatch **ppDispatch);

protected:
	RECVS_DISPATCH;

protected:
	ULONG ref;
};

