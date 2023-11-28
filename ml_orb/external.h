#ifndef NULLSOFT_ORB_PLUGIN_EXTERNAL_HEADER
#define NULLSOFT_ORB_PLUGIN_EXTERNAL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../nu/dispatchTable.h"

class ExternalDispatch : public IDispatch
{

public:
	typedef enum
	{
		DISPATCH_GETUNIQUEID = 1969,
		DISPATCH_GETSESSIONID = 1970,
		DISPATCH_PLAY = 1971,
	} DispatchCodes;

protected:
	ExternalDispatch();
	~ExternalDispatch();

public:
	static HRESULT CreateInstance(ExternalDispatch **instance);
	static LPCWSTR GetName();

public:
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

protected:
	DISPTABLE_INCLUDE();
	DISPHANDLER_REGISTER(OnGetUniqueId);
	DISPHANDLER_REGISTER(OnGetSessionId);
	DISPHANDLER_REGISTER(OnPlayUrl);
	
protected:
	ULONG ref;

};


#endif //NULLSOFT_ORB_PLUGIN_EXTERNAL_HEADER