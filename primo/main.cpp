#include "api.h"
#include "../Agave/Component/ifc_wa5component.h"
#include "PrimoFactory.h"
#include "PrimoSDK.h"
#include <bfc/platform/export.h>

class WA5_Primo : public ifc_wa5component
{
public:
	void RegisterServices(api_service *service);
	int RegisterServicesSafeModeOk();
	void DeregisterServices(api_service *service);
protected:
	RECVS_DISPATCH;
};

PrimoFactory primoFactory;

api_service *WASABI_API_SVC = 0;

void WA5_Primo::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	WASABI_API_SVC->service_register(&primoFactory);
}

int WA5_Primo::RegisterServicesSafeModeOk()
{
	return 1;
}

void WA5_Primo::DeregisterServices(api_service *service)
{
	service->service_deregister(&primoFactory);
	if (veritas_loaded)
	{
		PrimoSDK_End();
		veritas_loaded=false;
	}
	if (primoDLL)
	{
		FreeLibrary(primoDLL);
		primoDLL=0;
	}
}

static WA5_Primo wa5_primo;
extern "C" DLLEXPORT ifc_wa5component *GetWinamp5SystemComponent()
{
	return &wa5_primo;
}

#define CBCLASS WA5_Primo
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS