#include "main.h"
#include "./component.h"
#include "./wasabi.h"
#include "./dataViewFactory.h"

static DataViewFactory dataViewFactory;

Component::Component()
{
	InitializeCriticalSection(&lock);
}

Component::~Component()
{
	Lock();
	
	ReleaseServices();

	Unlock();

	DeleteCriticalSection(&lock);
}

size_t Component::AddRef()
{
	return 1;
}

size_t Component::Release()
{
	return 1;
}

int Component::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;

	*object = NULL;
	return E_NOINTERFACE;
}

void Component::Lock()
{
	EnterCriticalSection(&lock);
}

void Component::Unlock()
{
	LeaveCriticalSection(&lock);
}

void Component::RegisterServices(api_service *service)
{	
	Lock();

	if (FALSE != Wasabi_Initialize(Plugin_GetInstance(), service))
		Wasabi_LoadDefaultServices();

	dataViewFactory.Register(service);

	Unlock();

	aTRACE_LINE("DataView Service Registered");
}

void Component::DeregisterServices(api_service *service)
{	
	Lock();

	
	dataViewFactory.Unregister(service);

	ReleaseServices();

	Unlock();

	aTRACE_LINE("DataView Service Unregistered");
}

void Component::ReleaseServices()
{
	Wasabi_Release();
}

HRESULT Component::GetDataView(api_dataview **instance)
{
	return dataViewFactory.GetDataView(instance);
}

#define CBCLASS Component
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS