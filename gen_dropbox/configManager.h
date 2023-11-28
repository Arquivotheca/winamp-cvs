#ifndef NULLSOFT_DROPBOX_PLUGIN_CONIFGURATION_MANAGER_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_CONIFGURATION_MANAGER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./configInterface.h"
#include "../nu/ptrlist.h"

class Profile;
class ConfigurationManager
{
public:
	ConfigurationManager();
	~ConfigurationManager();

public:	
	BOOL Add(IConfiguration *pConfig);
	BOOL Remove(IConfiguration *pConfig);
	void Clear();
	HRESULT QueryConfiguration(REFGUID configId, Profile *profile, IConfiguration **ppConfig);

protected:
	typedef nu::PtrList<IConfiguration>  CONFIGLIST;
	CONFIGLIST configList;
};

#endif // NULLSOFT_DROPBOX_PLUGIN_CONIFGURATION_MANAGER_HEADER
