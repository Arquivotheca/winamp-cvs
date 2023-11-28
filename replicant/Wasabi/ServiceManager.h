#pragma once
#include "foundation/guid.h"
#include "service/api_service.h"
#include "nu/AutoLock.h"
#include "nu/PtrList.h"
#include "foundation/guid.h"
#include "nu/ptrmap.h"
#include "service/ifc_servicefactory.h"
#include "component/ifc_component_sync.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <semaphore.h>
#endif

class ServiceManager : public api_service, public ifc_component_sync
{
public:
	ServiceManager();
	~ServiceManager();
	int WASABICALL Dispatchable_QueryInterface(GUID interface_guid, void **object);

	int WASABICALL Service_Register(ifc_serviceFactory *svc);
	int WASABICALL Service_Unregister(ifc_serviceFactory *svc);
	size_t WASABICALL Service_GetServiceCount(GUID svc_type);
	ifc_serviceFactory *WASABICALL Service_EnumService(GUID svc_type, size_t n);
	ifc_serviceFactory *WASABICALL Service_EnumService(size_t n);
	ifc_serviceFactory *WASABICALL Service_GetServiceByGUID(GUID guid);
	void WASABICALL Service_ComponentDone();

private:
	int WASABICALL ComponentSync_Wait(size_t count);
	nu::LockGuard serviceGuard;
	typedef PtrMap<GUID, ifc_serviceFactory> ServiceMap;
	ServiceMap services;
	typedef nu::PtrList<ifc_serviceFactory> ServiceList;	
	PtrMap<GUID, ServiceList> services_by_type;
#ifdef _WIN32
	HANDLE component_wait;
#else
	sem_t component_wait;
#endif
};

extern ServiceManager service_manager;
