#ifndef _NULLSOFT_WINAMP_DATAVIEW_PERFORMANCE_TIMER_DEBUG_OUTPUT_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_PERFORMANCE_TIMER_DEBUG_OUTPUT_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_performancetimer.h"
#include "./ifc_performancetimerevent.h"

class PerformanceTimerDebugOutput : public ifc_performancetimerevent
{
protected:
	PerformanceTimerDebugOutput();
	~PerformanceTimerDebugOutput();

public:
	static HRESULT CreateInstance(PerformanceTimerDebugOutput **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_performancetimerevent */
	void PerformanceTimer_Started(ifc_performancetimer *timer);
	void PerformanceTimer_Stopped(ifc_performancetimer *timer);
	
protected:
	HRESULT OutputElapsedTime(ifc_performancetimer *timer);

protected:
	size_t ref;
		
protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_PERFORMANCE_TIMER_DEBUG_OUTPUT_HEADER