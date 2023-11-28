#ifndef NULLSOFT_DROPBOX_PLUGIN_CONFIGRUATION_INTERFACE_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_CONFIGRUATION_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>


class Profile;

EXTERN_C const IID IID_IConfiguration;

MIDL_INTERFACE("2B8F3FB9-6DD2-4cb9-AF2C-8ACD80BD323B")
IConfiguration : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE CreateInstance(REFIID configUid, Profile *profile, IConfiguration **ppConfig) = 0;
	virtual HRESULT STDMETHODCALLTYPE ResolveKeyString(LPCSTR pszKey, LPCSTR *ppszKnownKey) = 0;
	virtual HRESULT STDMETHODCALLTYPE ReadString(LPCSTR pszKey, LPTSTR pszBufferOut, INT cchBufferMax) = 0;
	virtual HRESULT STDMETHODCALLTYPE ReadInt(LPCSTR pszKey, INT *pnValue) = 0;
	virtual HRESULT STDMETHODCALLTYPE WriteString(LPCSTR pszKey, LPCTSTR pszBuffer) = 0;
	virtual HRESULT STDMETHODCALLTYPE WriteInt(LPCSTR pszKey, INT nValue) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDefaultString(LPCSTR pszKey, LPTSTR pszBufferOut, INT cchBufferMax) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDefaultInt(LPCSTR pszKey, INT *pnValue) = 0;	
	virtual HRESULT STDMETHODCALLTYPE Flush(void) = 0;

};


#endif //NULLSOFT_DROPBOX_PLUGIN_CONFIGRUATION_INTERFACE_HEADER