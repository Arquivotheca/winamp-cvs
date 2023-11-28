#pragma once
#include "foundation/dispatch.h"
#include "foundation/mkncc.h"
#include "nx/nxstring.h"
#include "foundation/types.h"
#include "foundation/error.h"
#include "service/types.h"
#include "metadata/ifc_metadata.h"

// {12BF0040-3F14-4346-82F7-AEC79C802343}
static const GUID cloud_api_service_guid = 
{ 0x12bf0040, 0x3f14, 0x4346, { 0x82, 0xf7, 0xae, 0xc7, 0x9c, 0x80, 0x23, 0x43 } };


// {933CCA0E-8D4F-4B99-9FAB-5C15A32E186F}
static const GUID cloud_api_syscallback = 
{ 0x933cca0e, 0x8d4f, 0x4b99, { 0x9f, 0xab, 0x5c, 0x15, 0xa3, 0x2e, 0x18, 0x6f } };


class ifc_cloudclient;
class ifc_clouddb;
class api_cloud : public Wasabi2::Dispatchable
{
protected:
	api_cloud() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~api_cloud() {}
public:
	enum 
	{
		// System callbacks for api_cloud
		SYSCALLBACK = MK4CC('c','l','o','d'),	// Unique identifier for mldb_api callbacks
		CLOUD_UPLOAD_START = 10,				// param1 = filename, param2 = (not used), Callback event for when a new file is added to the local mldb
		CLOUD_UPLOAD_DONE = 20,					// param1 = filename, param2 = (not used), Callback event for when a file is removed from the local mldb (before it happens)
	};

	enum 
	{
		SYSCB_LOGGEDIN = 0,
	};

	static GUID GetServiceType() { return SVC_TYPE_UNIQUE; }
	static GUID GetServiceGUID() { return cloud_api_service_guid; }
	static GUID GetSysCallbackGUID() { return cloud_api_syscallback; }

	ns_error_t CreateCloudClient(nx_string_t device_token, ifc_cloudclient **out_client) { return Cloud_CreateCloudClient(device_token, out_client); }
	ns_error_t GetAPIURL(nx_string_t *url, ns_error_t http = NErr_False) { return Cloud_GetAPIURL(url, http); }
	ns_error_t GetCredentials(nx_string_t *username, nx_string_t *authtoken, nx_string_t *provider) { return Cloud_GetCredentials(username, authtoken, provider); }
	ns_error_t SetCredentials(nx_string_t username, nx_string_t authtoken, nx_string_t provider) { return Cloud_SetCredentials(username, authtoken, provider); }
	ns_error_t CreateDatabaseConnection(ifc_clouddb **out_connection, nx_string_t device_id) { return Cloud_CreateDatabaseConnection(out_connection, device_id); }
	void SetDevMode(int dev_mode) { Cloud_SetDevMode(dev_mode); }
private:
	virtual ns_error_t WASABICALL Cloud_CreateCloudClient(nx_string_t device_token, ifc_cloudclient **out_client)=0;
	virtual ns_error_t WASABICALL Cloud_GetAPIURL(nx_string_t *url, ns_error_t http)=0;
	virtual ns_error_t WASABICALL Cloud_GetCredentials(nx_string_t *username, nx_string_t *authtoken, nx_string_t *provider)=0;
	virtual ns_error_t WASABICALL Cloud_SetCredentials(nx_string_t username, nx_string_t authtoken, nx_string_t provider)=0;
	virtual ns_error_t WASABICALL Cloud_CreateDatabaseConnection(ifc_clouddb **out_connection, nx_string_t device_id)=0;
	virtual void WASABICALL Cloud_SetDevMode(int dev_mode)=0;
	enum
	{
		DISPATCHABLE_VERSION=0,
	};
};

