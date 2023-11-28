#pragma once
#include "foundation/dispatch.h"
#include "nx/nxuri.h"

// TODO: benski> not sure if this should be a singleton API or an object you can create


// {E6976A89-5527-428B-BBFF-BAE3F4895FF9}
static const GUID mediaserver_api_service_guid = 
{ 0xe6976a89, 0x5527, 0x428b, { 0xbb, 0xff, 0xba, 0xe3, 0xf4, 0x89, 0x5f, 0xf9 } };

class api_mediaserver : public Wasabi2::Dispatchable
{
protected:
	api_mediaserver() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~api_mediaserver() {}
public:

	static GUID GetServiceType() { return SVC_TYPE_UNIQUE; }
	static GUID GetServiceGUID() { return mediaserver_api_service_guid; }
	int SetIPv4Address(int address) { return MediaServer_SetIPv4Address(address); }
	int SetDestinationDirectory(nx_uri_t directory) { return MediaServer_SetDestinationDirectory(directory); }
	int Start() { return MediaServer_Start(); }
	int Stop() { return MediaServer_Stop(); }

private:
	virtual int WASABICALL MediaServer_SetIPv4Address(int address)=0;
	virtual int WASABICALL MediaServer_SetDestinationDirectory(nx_uri_t directory)=0;	
	virtual int WASABICALL MediaServer_Start()=0;
	virtual int WASABICALL MediaServer_Stop()=0;

	enum
	{
		DISPATCHABLE_VERSION=0,
	};
};
