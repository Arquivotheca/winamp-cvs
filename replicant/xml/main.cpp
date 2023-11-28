#include "api.h"
#include "component/ifc_component.h"
#include "service/ifc_servicefactory.h"
#include "foundation/export.h"
#include "nswasabi/ObjectFactory.h"
#include "XMLParser.h"


/* Services that we are going to register */
static CountableObjectFactory<XMLParser, obj_xml> parser_factory;


/* services we need from Wasabi */
api_service *WASABI2_API_SVC=0;
api_application *WASABI2_API_APP=0;

// {C66B570E-0380-4F02-996A-EF7F622ED584}
static const GUID xml_component_guid = 
{ 0xc66b570e, 0x380, 0x4f02, { 0x99, 0x6a, 0xef, 0x7f, 0x62, 0x2e, 0xd5, 0x84 } };

/* Component implementation */
class XMLComponent : public ifc_component
{
public:
	XMLComponent() : ifc_component(xml_component_guid) {}
	int WASABICALL Component_RegisterServices(api_service *service);
	void WASABICALL Component_DeregisterServices(api_service *service);
};

int XMLComponent::Component_RegisterServices(api_service *service)
{
	WASABI2_API_SVC = service;
	
	// get application API
	WASABI2_API_SVC->GetService(&WASABI2_API_APP);

	parser_factory.Register(WASABI2_API_SVC);
	return NErr_Success;
}

void XMLComponent::Component_DeregisterServices(api_service *service)
{
	parser_factory.Deregister(WASABI2_API_SVC);

	if (WASABI2_API_APP)
		WASABI2_API_APP->Release();
}

static XMLComponent xml_component;

extern "C" DLLEXPORT ifc_component *GetWasabi2Component()
{
	return &xml_component;
}
