#pragma once
#include "api_mediaserver.h"
#include "nswasabi/ServiceName.h"
#include "nx/nxthread.h"

class MediaServer : public api_mediaserver
{
public:
	MediaServer();
	~MediaServer(); 
	WASABI_SERVICE_NAME("Media Server API");
	int WASABICALL MediaServer_SetDestinationDirectory(nx_uri_t directory);
	int WASABICALL MediaServer_SetIPv4Address(int address);
	int WASABICALL MediaServer_Start();
	int WASABICALL MediaServer_Stop();

	nx_uri_t destination;
	int ip;
	nx_thread_t server_thread;
};
