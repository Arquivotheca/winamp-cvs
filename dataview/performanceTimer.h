#ifndef _NULLSOFT_WINAMP_DATAVIEW_PERFORMANCE_TIMER_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_PERFORMANCE_TIMER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_performancetimer.h"

#include "../nu/ptrlist.h"

class PerformanceTimer : public ifc_performancetimer
{
protected:
	PerformanceTimer(char *name);
	~PerformanceTimer();

public:
	static HRESULT CreateInstance(const char *name, 
								  PerformanceTimer **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_performancetimer */
	const char *GetName();
	HRESULT Start();
	HRESULT Stop();
	HRESULT IsActive();
	double GetElapsed(); // seconds
	HRESULT RegisterEventHandler(ifc_performancetimerevent *eventHandler);
	HRESULT UnregisterEventHandler(ifc_performancetimerevent *eventHandler);
	
protected:
	void Notify_Started();
	void Notify_Stopped();

protected:
	typedef nu::PtrList<ifc_performancetimerevent> EventHandlerList;

protected:
	size_t ref;
	size_t activityLock;
	char *name;
	LARGE_INTEGER start;
	LARGE_INTEGER stop;
	EventHandlerList eventHandlerList;
	
protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_PERFORMANCE_TIMER_HEADER