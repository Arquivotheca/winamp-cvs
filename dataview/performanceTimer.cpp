#include "main.h"
#include "./performanceTimer.h"

static double performanceCounterFrequency = 0.0;


PerformanceTimer::PerformanceTimer(char *_name)
	: ref(1), activityLock(0), name(_name)
{
	start.QuadPart = 0;
	stop.QuadPart = 0;
}

PerformanceTimer::~PerformanceTimer()
{
	size_t index;
	
	index = eventHandlerList.size();
	while(index--)
	{
		eventHandlerList[index]->Release();
	}

	eventHandlerList.clear();

	AnsiString_Free(name);
}

HRESULT PerformanceTimer::CreateInstance(const char *name, PerformanceTimer **instance)
{
	PerformanceTimer *self;
	char *nameDup;

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (0.0 == performanceCounterFrequency)
	{
		LARGE_INTEGER freq;
		if (FALSE == QueryPerformanceFrequency(&freq))
			RETURN_HRESULT_FROM_LAST_ERROR();

		performanceCounterFrequency = (double)freq.QuadPart;
	}
	
	nameDup = AnsiString_Duplicate(name);
	if (NULL == nameDup && NULL != name)
		return E_OUTOFMEMORY;

	self = new (std::nothrow) PerformanceTimer(nameDup);
	if (NULL == self)
	{
		AnsiString_Free(nameDup);
		return E_OUTOFMEMORY;
	}

	*instance = self;
	return S_OK;
}

size_t PerformanceTimer::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t PerformanceTimer::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int PerformanceTimer::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_PerformanceTimer))
		*object = static_cast<ifc_performancetimer*>(this);
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

const char *PerformanceTimer::GetName()
{
	return name;
}

HRESULT PerformanceTimer::Start()
{
	if (0 != activityLock)
		return S_FALSE;
	
	if (FALSE == QueryPerformanceCounter(&start))
		RETURN_HRESULT_FROM_LAST_ERROR();

	activityLock++;
	Notify_Started();

	return S_OK;
}

HRESULT PerformanceTimer::Stop()
{
	HRESULT hr;

	if (0 == activityLock)
		return S_FALSE;
	
	if (0 != --activityLock)
		return E_PENDING;

	if (FALSE == QueryPerformanceCounter(&stop))
	{
		unsigned long errorCode;
		errorCode = GetLastError();
		stop.QuadPart = start.QuadPart;
		hr = HRESULT_FROM_WIN32(errorCode);
	}
	else
		hr = S_OK;

	Notify_Stopped();

	return hr;
}

HRESULT PerformanceTimer::IsActive()
{
	return (0 != activityLock) ? S_OK : S_FALSE;
}

double PerformanceTimer::GetElapsed()
{
	double result;

	if (0 != activityLock)
	{
		if (FALSE == QueryPerformanceCounter(&stop))
			stop.QuadPart = start.QuadPart;
	}

	result = (double)(stop.QuadPart - start.QuadPart) / performanceCounterFrequency;
	
	return result;
}

HRESULT PerformanceTimer::RegisterEventHandler(ifc_performancetimerevent *eventHandler)
{
	size_t index;

	if (NULL == eventHandler)
		return E_INVALIDARG;

	index = eventHandlerList.size();
	while(index--)
	{
		if (eventHandler == eventHandlerList[index])
			return S_FALSE;
	}
	
	eventHandler->AddRef();
	eventHandlerList.push_back(eventHandler);
	
	return S_OK;
}


HRESULT PerformanceTimer::UnregisterEventHandler(ifc_performancetimerevent *eventHandler)
{
	size_t index;

	if (NULL == eventHandler)
		return E_INVALIDARG;

	index = eventHandlerList.size();
	while(index--)
	{
		if (eventHandler == eventHandlerList[index])
		{
			eventHandlerList.eraseindex(index);
			eventHandler->Release();
			return S_OK;
		}
	}
	
	return S_FALSE;
}


void PerformanceTimer::Notify_Started()
{
	size_t index, count;
	count = eventHandlerList.size();

	for (index = 0; index < count; index++)
	{
		eventHandlerList[index]->PerformanceTimer_Started(this);
	}
}

void PerformanceTimer::Notify_Stopped()
{
		size_t index, count;
	count = eventHandlerList.size();

	for (index = 0; index < count; index++)
	{
		eventHandlerList[index]->PerformanceTimer_Stopped(this);
	}
}

#define CBCLASS PerformanceTimer
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETNAME, GetName)
CB(API_START, Start)
CB(API_STOP, Stop)
CB(API_ISACTIVE, IsActive)
CB(API_GETELAPSED, GetElapsed)
CB(API_REGISTEREVENTHANDLER, RegisterEventHandler)
CB(API_UNREGISTEREVENTHANDLER, UnregisterEventHandler)
END_DISPATCH;
#undef CBCLASS
