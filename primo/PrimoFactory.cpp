#include "api.h"
#include "PrimoFactory.h"
#include "veritas.h"
#include "../nu/AutoLock.h"

HMODULE primoDLL=0;
bool veritas_loaded=false;
using namespace Nullsoft::Utility;
static LockGuard primoGuard;

FOURCC PrimoFactory::GetServiceType()
{
	return obj_primo::getServiceType();
}

const char *PrimoFactory::GetServiceName()
{
	return obj_primo::getServiceName();
}

GUID PrimoFactory::GetGUID()
{
	return obj_primo::getServiceGuid();
}

void *PrimoFactory::GetInterface(int global_lock)
{
	{
		AutoLock autolock(primoGuard);
		if (!veritas_loaded)
		{
			unsigned int prevErrorMode;
			prevErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
			primoDLL = LoadLibrary(L"pxsdkpls.dll");
			SetErrorMode(prevErrorMode);
			
			if (0 != primoDLL)
			{
				DWORD release;
				PrimoSDK_Init(&release);
				veritas_loaded=true;
			}
		}
		if (!primoDLL)
			return 0;
	}

	PrimoObject *obj = new PrimoObject;
	obj_primo *ifc= static_cast<obj_primo *>(obj);
//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return ifc;
}

int PrimoFactory::SupportNonLockingInterface()
{
	return 1;
}

int PrimoFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	obj_primo *obj = static_cast<obj_primo *>(ifc);
	PrimoObject *primo = static_cast<PrimoObject *>(obj);
	delete primo;
	return 1;
}

const char *PrimoFactory::GetTestString()
{
	return NULL;
}

int PrimoFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS PrimoFactory
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