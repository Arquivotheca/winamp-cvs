#include "JSAPI2_Creator.h"
#include "JSAPI2_CloudAPI.h"
#include "api.h"

IDispatch *JSAPI2_Creator::CreateAPI(const wchar_t *name, const wchar_t *key, JSAPI::ifc_info *info)
{
	if (!wcscmp(name, L"Cloud"))
		return new JSAPI2::CloudAPI(key, info);
	else
		return 0;
}

int JSAPI2_Creator::PromptForAuthorization(HWND parent, const wchar_t *group, const wchar_t *action, const wchar_t *authorization_key, JSAPI2::api_security::AuthorizationData *data)
{
	if (group && !wcscmp(group, L"Cloud"))
	{
		return JSAPI2::svc_apicreator::AUTHORIZATION_DENY;
	}
	else
		return JSAPI2::svc_apicreator::AUTHORIZATION_UNDEFINED;
}


#define CBCLASS JSAPI2_Creator
START_DISPATCH;
CB(JSAPI2_SVC_APICREATOR_CREATEAPI, CreateAPI);
CB(JSAPI2_SVC_APICREATOR_PROMPTFORAUTHORIZATION, PromptForAuthorization);
END_DISPATCH;
#undef CBCLASS

static JSAPI2_Creator jsapi2_svc;
static const char serviceName[] = "Cloud Javascript Objects";

// {37259123-E290-4FBF-A914-1335F4CC0479}
static const GUID jsapi2_factory_guid = 
{ 0x37259123, 0xe290, 0x4fbf, { 0xa9, 0x14, 0x13, 0x35, 0xf4, 0xcc, 0x4, 0x79 } };


FOURCC JSAPI2Factory::GetServiceType()
{
	return jsapi2_svc.getServiceType();
}

const char *JSAPI2Factory::GetServiceName()
{
	return serviceName;
}

GUID JSAPI2Factory::GetGUID()
{
	return jsapi2_factory_guid;
}

void *JSAPI2Factory::GetInterface(int global_lock)
{
//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return &jsapi2_svc;
}

int JSAPI2Factory::SupportNonLockingInterface()
{
	return 1;
}

int JSAPI2Factory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	return 1;
}

const char *JSAPI2Factory::GetTestString()
{
	return 0;
}

int JSAPI2Factory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS JSAPI2Factory
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