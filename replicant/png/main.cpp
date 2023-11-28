#include "api.h"
#include "main.h"
#include "nswasabi/singleton.h"
#include "foundation/export.h"
#include "PNGLoader.h"

static PNGComponent png_component;
api_service *WASABI2_API_SVC=0;
api_application *WASABI2_API_APP=0;

static SingletonService<PNGLoader, svc_imageloader> png_factory;

// {0593C186-1AC2-4F5D-A52B-8AC143272893}
static const GUID png_component_guid = 
{ 0x593c186, 0x1ac2, 0x4f5d, { 0xa5, 0x2b, 0x8a, 0xc1, 0x43, 0x27, 0x28, 0x93 } };

PNGComponent::PNGComponent() : ifc_component(png_component_guid)
{
}

int PNGComponent::Component_RegisterServices(api_service *service)
{
	WASABI2_API_SVC = service;
	
	WASABI2_API_SVC->GetService(&WASABI2_API_APP);

	png_factory.Register(WASABI2_API_SVC);
	
	//WASABI_API_SVC->service_register(&pngWriteFactory);
	return NErr_Success;
}

void PNGComponent::Component_DeregisterServices(api_service *service)
{
	png_factory.Deregister(WASABI2_API_SVC);
	//service->service_deregister(&pngWriteFactory);

	if (WASABI2_API_APP)
		WASABI2_API_APP->Release();
}

extern "C" DLLEXPORT ifc_component *GetWasabi2Component()
{
	return &png_component;
}

