#include "main.h"
#include "./performanceTimerDebugOutput.h"

#include <strsafe.h>

PerformanceTimerDebugOutput::PerformanceTimerDebugOutput()
	: ref(1)
{
}

PerformanceTimerDebugOutput::~PerformanceTimerDebugOutput()
{
}

HRESULT PerformanceTimerDebugOutput::CreateInstance(PerformanceTimerDebugOutput **instance)
{
	PerformanceTimerDebugOutput *self;

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	self = new (std::nothrow) PerformanceTimerDebugOutput();
	if (NULL == self)
		return E_OUTOFMEMORY;

	*instance = self;
	return S_OK;

}

size_t PerformanceTimerDebugOutput::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t PerformanceTimerDebugOutput::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int PerformanceTimerDebugOutput::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_PerformanceTimerEvent))
		*object = static_cast<ifc_performancetimerevent*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

	/* ifc_performancetimerevent */
void PerformanceTimerDebugOutput::PerformanceTimer_Started(ifc_performancetimer *timer)
{
}

void PerformanceTimerDebugOutput::PerformanceTimer_Stopped(ifc_performancetimer *timer)
{
	OutputElapsedTime(timer);
}
	
HRESULT PerformanceTimerDebugOutput::OutputElapsedTime(ifc_performancetimer *timer)
{
	const char *timerName;
	char buffer[128];

	if (NULL == timer)
		return E_INVALIDARG;

	timerName = timer->GetName();
	if (NULL == timerName)
		timerName = "";

	StringCchPrintfA(buffer, ARRAYSIZE(buffer), "DataViewPerfTimer (%s): %f sec\r\n", timerName, timer->GetElapsed());
	OutputDebugStringA(buffer);

	return S_OK;
}

#define CBCLASS PerformanceTimerDebugOutput
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
VCB(API_PERFORMANCETIMER_STARTED, PerformanceTimer_Started)
VCB(API_PERFORMANCETIMER_STOPPED, PerformanceTimer_Stopped)
END_DISPATCH;
#undef CBCLASS
