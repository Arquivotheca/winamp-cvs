#include "main.h"
#include "./dataViewFactory.h"
#include "./dataView.h"

DataViewFactory::DataViewFactory() 
	: object(NULL)
{
}

DataViewFactory::~DataViewFactory()
{
	SafeRelease(object);
}

FOURCC DataViewFactory::GetServiceType()
{
	return WaSvc::UNIQUE;
}

const char *DataViewFactory::GetServiceName()
{
	return "DataView Interface";
}

GUID DataViewFactory::GetGUID()
{
	return DataViewGUID;
}

void *DataViewFactory::GetInterface(int global_lock)
{	
	api_dataview *instance;
	if (FAILED(GetDataView(&instance)))
		instance = NULL;

	return instance;
}

int DataViewFactory::SupportNonLockingInterface()
{
	return 1;
}

int DataViewFactory::ReleaseInterface(void *ifc)
{
	DataView *object = (DataView*)ifc;
	SafeRelease(object);	
	return 1;
}

const char *DataViewFactory::GetTestString()
{
	return NULL;
}

int DataViewFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

HRESULT DataViewFactory::Register(api_service *service)
{
	if (NULL == service) 
		return E_INVALIDARG;

	service->service_register(this);
	return S_OK;
}

HRESULT DataViewFactory::Unregister(api_service *service)
{
	if (NULL == service) 
		return E_INVALIDARG;

	service->service_deregister(this);
	return S_OK;
}

HRESULT DataViewFactory::GetDataView(api_dataview **instance)
{
	if (NULL == instance)
		return E_POINTER;

	if (NULL == object)
	{
		HRESULT hr;
		
		hr = DataView::CreateInstance(&object);
		if (FAILED(hr))
		{
			object = NULL;
			return hr;
		}
	}

	*instance = object;
	object->AddRef();

	return S_OK;
}

#define CBCLASS DataViewFactory
START_DISPATCH;
CB(WASERVICEFACTORY_GETSERVICETYPE, GetServiceType)
CB(WASERVICEFACTORY_GETSERVICENAME, GetServiceName)
CB(WASERVICEFACTORY_GETGUID, GetGUID)
CB(WASERVICEFACTORY_GETINTERFACE, GetInterface)
CB(WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, SupportNonLockingInterface)
CB(WASERVICEFACTORY_RELEASEINTERFACE, ReleaseInterface)
CB(WASERVICEFACTORY_GETTESTSTRING, GetTestString)
CB(WASERVICEFACTORY_SERVICENOTIFY, ServiceNotify)
END_DISPATCH;
#undef CBCLASS