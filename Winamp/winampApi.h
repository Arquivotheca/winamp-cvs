#ifndef NULLSOFT_WINAMP_API_WINAMP_IMPLEMENTATION_HEADER
#define NULLSOFT_WINAMP_API_WINAMP_IMPLEMENTATION_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./api_winamp.h"

class WinampApi : public api_winamp
{
public:
	WinampApi();
	~WinampApi();

public:
	static HRESULT CreateInstance(WinampApi **instance);
	static const char *getServiceName();
	static const GUID getServiceGuid();

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* api_winamp */
	HWND GetMainWindow(void);
	HWND GetDlgParent(void);
	HRESULT OpenUrl(HWND hwnd, const wchar_t *url);
	int GetRegVer();

protected:
	size_t ref;

protected:
	RECVS_DISPATCH;
};

#endif //NULLSOFT_WINAMP_API_WINAMP_IMPLEMENTATION_HEADER