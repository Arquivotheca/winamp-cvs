#include <stdio.h>
#include "api.h"
#include "CloudAPI.h"
#include "nswasabi/AutoCharNX.h"
#include "CloudThread.h"
#include "main.h"
#include "nswasabi/ReferenceCounted.h"
#include "CloudDB.h"
#include <ctype.h>


CloudAPI::CloudAPI()
{
	username=authtoken=provider=0;
	dev_mode = 0;
}

CloudAPI::~CloudAPI()
{
	NXStringRelease(username);
	NXStringRelease(authtoken);
	NXStringRelease(provider);
}

ns_error_t CloudAPI::Cloud_CreateCloudClient(nx_string_t device_token, ifc_cloudclient **out_client)
{
	nu::AutoLock cloud_lock(cloud_client_guard);
	CloudThread *&cloud_client = cloud_clients[device_token];
	if (!cloud_client)
	{
		cloud_client = new (std::nothrow) ReferenceCounted<CloudThread>;
		if (!cloud_client)
			return NErr_OutOfMemory;

		ns_error_t ret = cloud_client->Initialize(device_token, dev_mode);
		if (ret != NErr_Success)
		{
			delete cloud_client;
			return ret;
		}
	}
	else
	{
		cloud_client->ifc_cloudclient::Retain();
	}

	*out_client = cloud_client;
	return NErr_Success;
}

extern nx_string_t cloud_url_override;
ns_error_t CloudAPI::Cloud_GetAPIURL(nx_string_t *url, ns_error_t http)
{
	if (http == NErr_False && cloud_url_override)
	{
		*url = NXStringRetain(cloud_url_override);
		return NErr_Success;
	}
	else
	{
		const char *cloudurl=0;
		switch(dev_mode)
		{
		case 0: // production
			if (http == NErr_True)
				cloudurl="http://cloud.winamp.com/api/1/";
			else
				cloudurl="https://cloud.winamp.com/api/1/";
			break;

		case 1: // dev
			if (http == NErr_True)
				cloudurl="http://devcloud.winamp.com/api/1/";
			else
				cloudurl="https://devcloud.winamp.com/api/1/";
			break;

		case 2: // qa
			if (http == NErr_True)
				cloudurl="http://qacloud.winamp.com/api/1/";
			else
				cloudurl="https://qacloud.winamp.com/api/1/";
			break;

		case 3: // stage
			if (http == NErr_True)
				cloudurl="http://stagecloud.winamp.com/api/1/";
			else
				cloudurl="https://stagecloud.winamp.com/api/1/";
			break;
		}
		return NXStringCreateWithUTF8(url, cloudurl);
	}
}

ns_error_t CloudAPI::Cloud_GetCredentials(nx_string_t *username, nx_string_t *authtoken, nx_string_t *provider)
{
	if (!this->username || this->username && !this->username->len ||
		!this->authtoken || this->authtoken && !this->authtoken->len ||
		!this->provider || this->provider && !this->provider->len )
		return NErr_Empty;

	if (username && this->username->len > 0)
		*username = NXStringRetain(this->username);
	else if (username)
		return NErr_Empty;

	if (authtoken && this->authtoken->len > 0)
		*authtoken = NXStringRetain(this->authtoken);
	else if (authtoken)
		return NErr_Empty;

	if (provider && this->provider->len > 0)
		*provider = NXStringRetain(this->provider);
	else if (provider)
		return NErr_Empty;

	return NErr_Success;
}

ns_error_t CloudAPI::Cloud_SetCredentials(nx_string_t _username, nx_string_t _authtoken, nx_string_t _provider)
{
	/* TODO: store in database? */
	NXStringRelease(username);
	NXStringRelease(authtoken);
	NXStringRelease(provider);	
	username=NXStringRetain(_username);
	authtoken=NXStringRetain(_authtoken);
	provider=NXStringRetain(_provider);
	return NErr_Success;
}

ns_error_t CloudAPI::Cloud_CreateDatabaseConnection(ifc_clouddb **out_connection, nx_string_t device_id)
{
	ReferenceCountedNXURI filepath;
	ns_error_t ret = CloudThread::CreatePathForDatabase(&filepath, device_id, dev_mode);
	if (ret != NErr_Success)
		return ret;
	Cloud_DBConnection *db;
	ret = Cloud_DBConnection::CreateConnection(&db, filepath, 0, device_id);
	if (ret != NErr_Success)
		return ret;

	db->Info_Populate(device_id);
	*out_connection = db;

	return NErr_Success;
}

void CloudAPI::Cloud_SetDevMode(int dev_mode)
{
	this->dev_mode = dev_mode;
}