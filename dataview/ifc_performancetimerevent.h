#ifndef _NULLSOFT_WINAMP_DATAVIEW_PERFORMANCE_TIMER_EVENT_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_PERFORMANCE_TIMER_EVENT_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {ECEADEEA-FC6C-4af8-8EA3-F26346D4F174}
static const GUID IFC_PerformanceTimerEvent = 
{ 0xeceadeea, 0xfc6c, 0x4af8, { 0x8e, 0xa3, 0xf2, 0x63, 0x46, 0xd4, 0xf1, 0x74 } };

class ifc_performancetimer;

#include <bfc/dispatch.h>

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_performancetimerevent : public Dispatchable
{
protected:
	ifc_performancetimerevent() {}
	~ifc_performancetimerevent() {}

public:
	void PerformanceTimer_Started(ifc_performancetimer *timer);
	void PerformanceTimer_Stopped(ifc_performancetimer *timer);

public:
	DISPATCH_CODES
	{
		API_PERFORMANCETIMER_STARTED = 10,
		API_PERFORMANCETIMER_STOPPED = 20,
	};
};

inline void ifc_performancetimerevent::PerformanceTimer_Started(ifc_performancetimer *timer)
{
	_voidcall(API_PERFORMANCETIMER_STARTED, timer);
}

inline void ifc_performancetimerevent::PerformanceTimer_Stopped(ifc_performancetimer *timer)
{
	_voidcall(API_PERFORMANCETIMER_STOPPED, timer);
}


#endif //_NULLSOFT_WINAMP_DATAVIEW_PERFORMANCE_TIMER_EVENT_INTERFACE_HEADER