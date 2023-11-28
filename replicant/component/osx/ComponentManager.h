#pragma once
#include "../ComponentManagerBase.h"

class ComponentManager : public ComponentManagerBase
{
public:
	int AddComponent(nx_uri_t component_uri);
	int AddDirectory(nx_uri_t directory);
	
private:
	void CloseComponent(ifc_component *component);
};