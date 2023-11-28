#ifndef NULLSOFT_ORB_PLUGIN_SERVICE_HEADER
#define NULLSOFT_ORB_PLUGIN_SERVICE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include <ifc_omservice.h>

class OmService : public ifc_omservice
{

public:
	typedef enum
	{
		flagRoot = 0x00000001,
	} Flags;

protected:
	OmService(UINT nId);
	~OmService();

public:
	static HRESULT CreateInstance(UINT nId, LPCWSTR pszName, LPCWSTR pszIcon, LPCWSTR pszUrl, OmService **instance);

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
	HRESULT GetIcon(wchar_t *pszBuffer, int cchBufferMax);

public:
	HRESULT SetName(LPCWSTR pszName);
	HRESULT SetUrl(LPCWSTR pszUrl);
	HRESULT SetIcon(LPCWSTR pszIcon);

	void SetFlags(UINT mask, UINT newFlags);
	UINT GetFlags(void);

protected:
	RECVS_DISPATCH;

protected:
	ULONG ref;
	UINT id;
	LPWSTR name;
	LPWSTR url;
	LPWSTR icon;
	UINT flags;
};

#endif //NULLSOFT_ORB_PLUGIN_SERVICE_HEADER