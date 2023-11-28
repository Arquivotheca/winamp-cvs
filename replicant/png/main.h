#pragma once
#include "component/ifc_component.h"

class PNGComponent : public ifc_component
{
public:
	PNGComponent();
	int WASABICALL Component_RegisterServices(api_service *service);
	void WASABICALL Component_DeregisterServices(api_service *service);

};
