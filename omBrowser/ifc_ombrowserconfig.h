#ifndef NULLSOFT_WINAMP_OMBROWSER_BROWSER_CONFIG_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_BROWSER_CONFIG_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {41660B13-0547-4b8f-934B-A688306F0D4A}
static const GUID IFC_OmBrowserConfig =
{ 0x41660b13, 0x547, 0x4b8f, { 0x93, 0x4b, 0xa6, 0x88, 0x30, 0x6f, 0xd, 0x4a } };

#define CFGID_BROWSER_CLIENTID		0			//param = (LPCWSTR)pszClientId

#include <bfc/dispatch.h>

class __declspec(novtable) ifc_ombrowserconfig : public Dispatchable
{
protected:
	ifc_ombrowserconfig() {}
	~ifc_ombrowserconfig() {}

public:
	HRESULT GetClientId(LPWSTR pszBuffer, INT cchBufferMax);
	HRESULT SetClientId(LPWSTR pszClientId);

public:
	DISPATCH_CODES
	{
		API_GETCLIENTID		= 10,
		API_SETCLIENTID		= 20,
	};
};

inline HRESULT ifc_ombrowserconfig::GetClientId(LPWSTR pszBuffer, INT cchBufferMax)
{
	return _call(API_GETCLIENTID, (HRESULT)E_NOTIMPL, pszBuffer, cchBufferMax); 
}

inline HRESULT ifc_ombrowserconfig::SetClientId(LPWSTR pszClientId)
{
	return _call(API_SETCLIENTID, (HRESULT)E_NOTIMPL, pszClientId);
}

#endif // NULLSOFT_WINAMP_OMBROWSER_BROWSER_CONFIG_INTERFACE_HEADER