#pragma once

#include <wtypes.h>
#include "../nu/dispatchTable.h"

class WebAuthDispatch : public IDispatch
{

public:
	typedef enum
	{
		DISPATCH_TRANSMIT_TOKEN	= 700,
		DISPATCH_TRANSMIT_STATUS = 701,
	} DispatchCodes;

protected:
	WebAuthDispatch();
	~WebAuthDispatch();

public:
	static HRESULT CreateInstance(WebAuthDispatch **instance);
	static LPCWSTR GetName();

public:
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

protected:
	DISPTABLE_INCLUDE();
	DISPHANDLER_REGISTER(OnTransmitToken);
	DISPHANDLER_REGISTER(OnTransmitStatus);

protected:
	ULONG ref;

};


