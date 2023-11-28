#include "api.h"
#include "util.h"
api_service *serviceManager=0;

static JNL_AsyncDNS *global_dns=0;
JNL_AsyncDNS *GetGlobalDNS()
{
	if (!global_dns)
	{
		if (JNL::open_socketlib())
			return 0;
		global_dns = new JNL_AsyncDNS;
	}
	return global_dns;
}

void DestroyGlobalDNS()
{
	if (global_dns)
	{
		delete global_dns;
		global_dns=0;
		JNL::close_socketlib();
	}
}