#include "./browserFactory.h"
#include "./browserObject.h"

OmBrowserFactory::OmBrowserFactory() 
{
}

OmBrowserFactory::~OmBrowserFactory()
{
}

FOURCC OmBrowserFactory::GetServiceType()
{
	return WaSvc::UNIQUE;
}

const char *OmBrowserFactory::GetServiceName()
{
	return "OmBrowser Object";
}

GUID OmBrowserFactory::GetGUID()
{
	return OBJ_OmBrowser;
}

void *OmBrowserFactory::GetInterface(int global_lock)
{		
	OmBrowserObject *browserObject;
	HRESULT hr = OmBrowserObject::CreateInstance(&browserObject);
	if (FAILED(hr)) browserObject = NULL;

	return browserObject;
}

int OmBrowserFactory::SupportNonLockingInterface()
{
	return 1;
}

int OmBrowserFactory::ReleaseInterface(void *ifc)
{
	obj_ombrowser *object = (obj_ombrowser*)ifc;
	if (NULL != object) 
		object->Release();
	return 1;
}

const char *OmBrowserFactory::GetTestString()
{
	return NULL;
}

int OmBrowserFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

HRESULT OmBrowserFactory::Register(api_service *service)
{
	if (NULL == service) return E_INVALIDARG;
	service->service_register(this);
	return S_OK;
}

HRESULT OmBrowserFactory::Unregister(api_service *service)
{
	if (NULL == service) return E_INVALIDARG;
	service->service_deregister(this);
	return S_OK;
}

#define CBCLASS OmBrowserFactory
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