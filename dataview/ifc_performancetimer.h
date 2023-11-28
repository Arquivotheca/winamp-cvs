#ifndef _NULLSOFT_WINAMP_DATAVIEW_PERFORMANCE_TIMER_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_PERFORMANCE_TIMER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {1F7435CC-E31F-4066-81F1-41C9B9DA1759}
static const GUID IFC_PerformanceTimer = 
{ 0x1f7435cc, 0xe31f, 0x4066, { 0x81, 0xf1, 0x41, 0xc9, 0xb9, 0xda, 0x17, 0x59 } };

#include "./ifc_performancetimerevent.h"

#include <bfc/dispatch.h>


// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_performancetimer : public Dispatchable
{
protected:
	ifc_performancetimer() {}
	~ifc_performancetimer() {}

public:
	const char *GetName();

	HRESULT Start();
	HRESULT Stop();
	HRESULT IsActive();
	double GetElapsed(); //seconds

	HRESULT RegisterEventHandler(ifc_performancetimerevent *eventHandler);
	HRESULT UnregisterEventHandler(ifc_performancetimerevent *eventHandler);

public:
	DISPATCH_CODES
	{
		API_GETNAME = 10,
		API_START = 20,
		API_STOP = 30,
		API_ISACTIVE = 40,
		API_GETELAPSED = 50,
		API_REGISTEREVENTHANDLER = 60,
		API_UNREGISTEREVENTHANDLER = 70,
	};
};

inline const char *ifc_performancetimer::GetName()
{
	return _call(API_GETNAME, (const char *)NULL);
}

inline HRESULT ifc_performancetimer::Start()
{
	return _call(API_START, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_performancetimer::Stop()
{
	return _call(API_STOP, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_performancetimer::IsActive()
{
	return _call(API_STOP, (HRESULT)E_NOTIMPL);
}

inline double ifc_performancetimer::GetElapsed()
{
	return _call(API_GETELAPSED, (double)0.0);
}

inline HRESULT ifc_performancetimer::RegisterEventHandler(ifc_performancetimerevent *eventHandler)
{
	return _call(API_REGISTEREVENTHANDLER, (HRESULT)E_NOTIMPL, eventHandler);
}

inline HRESULT ifc_performancetimer::UnregisterEventHandler(ifc_performancetimerevent *eventHandler)
{
	return _call(API_UNREGISTEREVENTHANDLER, (HRESULT)E_NOTIMPL, eventHandler);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_PERFORMANCE_TIMER_INTERFACE_HEADER