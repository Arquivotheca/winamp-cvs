#pragma once
#include "component/ifc_component.h"
#include "Replicant/version.h"

/* A component meant only to run within a Replicant-based framework (And not some other Wasabi-based framework) should inherit from this instead of ifc_component
It will automatically set component_info.framework_guid to the most up-to-date GUID for a Replicant plugin */
class ifc_replicant_component : public ifc_component
{
	protected:
		ifc_replicant_component(GUID component_guid) : ifc_component(component_guid)
		{
			component_info.framework_guid = replicant_platform_guid;
		}
};