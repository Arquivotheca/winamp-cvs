#ifndef NULLSOFT_AGAVE_API_CONFIGGROUP_H
#define NULLSOFT_AGAVE_API_CONFIGGROUP_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>
#include <bfc/platform/guid.h>
#include "api_configitem.h"

class api_configgroup : public Dispatchable
{
protected:
	api_configgroup() {}
	~api_configgroup() {}
public:
	api_configitem *GetItem(const wchar_t *name);
	GUID GetGUID();
public:
	DISPATCH_CODES
	{
		API_CONFIGGROUP_GETITEM = 10,
		API_CONFIGGROUP_GETGUID = 20,
	};
	
};

inline api_configitem *api_configgroup::GetItem(const wchar_t *name)
{
	return _call(API_CONFIGGROUP_GETITEM, (api_configitem *)0, name);
}

inline GUID api_configgroup::GetGUID()
{
	return _call(API_CONFIGGROUP_GETGUID, (GUID)INVALID_GUID);
}
#endif