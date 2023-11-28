#ifndef NULLSOFT_PODCAST_PLUGIN_RSS_COM_HEADER
#define NULLSOFT_PODCAST_PLUGIN_RSS_COM_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../nu/dispatchTable.h"

class RssCOM : public IDispatch
{
public:
	typedef enum
	{
		DISPATCH_SUBSCRIBE = 0,
	} DispatchCodes;

protected:
	RssCOM();
	~RssCOM();

public:
	static HRESULT CreateInstance(RssCOM **instance);
	static HRESULT SubscribeUrl(BSTR url, VARIANT FAR *result);
	static LPCWSTR GetName();

public:
	/* IUnknown*/
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

protected:
	DISPTABLE_INCLUDE();
	DISPHANDLER_REGISTER(OnSubscribe);

protected:
	ULONG ref;
};

#endif //NULLSOFT_PODCAST_PLUGIN_RSS_COM_HEADER