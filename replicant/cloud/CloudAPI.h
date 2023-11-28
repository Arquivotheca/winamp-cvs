#pragma once
#include "foundation/types.h"
#include "foundation/error.h"
#include "ifc_cloudclient.h"
#include "nx/nxstring.h"
#include "api_cloud.h"
#include "nu/AutoLock.h"
#include "nu/ptrmap.h"
#include "sha1.h"
#include "nswasabi/ServiceName.h"
#include "metadata/ifc_metadata.h"

template <>
class PtrMapComp<nx_string_t>
{
public:
	int operator()(const nx_string_t &a, const nx_string_t &b)
	{
		return NXStringKeywordCompare(a, b);
	}
};

class CloudThread;
class CloudAPI : public api_cloud
{
public:
	WASABI_SERVICE_NAME("Cloud API");
	CloudAPI();
	~CloudAPI();

	ns_error_t WASABICALL Cloud_CreateCloudClient(nx_string_t device_token, ifc_cloudclient **out_client);
	ns_error_t WASABICALL Cloud_GetAPIURL(nx_string_t *url, ns_error_t http);
	ns_error_t WASABICALL Cloud_GetCredentials(nx_string_t *username, nx_string_t *authtoken, nx_string_t *provider);
	ns_error_t WASABICALL Cloud_SetCredentials(nx_string_t username, nx_string_t authtoken, nx_string_t provider);
	ns_error_t WASABICALL Cloud_CreateDatabaseConnection(ifc_clouddb **out_connection, nx_string_t device_id);
	void WASABICALL Cloud_SetDevMode(int dev_mode);
private:	
	nu::LockGuard cloud_client_guard;
	PtrMap<nx_string_t, CloudThread> cloud_clients;
	nx_string_t username, authtoken, provider;
	int dev_mode;	// used to determine the url to use
};

extern CloudAPI cloud_api;