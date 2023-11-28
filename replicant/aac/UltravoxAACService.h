#pragma once
#include "ultravox/svc_ultravox_playback.h"
#include "nswasabi/ServiceName.h"

// {CAEF8CA5-8443-4F96-8925-28B4003A26BD}
static const GUID ultravox_aac_service_guid = 
{ 0xcaef8ca5, 0x8443, 0x4f96, { 0x89, 0x25, 0x28, 0xb4, 0x0, 0x3a, 0x26, 0xbd } };


class UltravoxAACService : public svc_ultravox_playback
{
public:
	WASABI_SERVICE_NAME("Ultravox AAC Playback");
	static GUID GetServiceGUID() { return ultravox_aac_service_guid; }
	
	NError WASABICALL UltravoxPlaybackService_CreateDemuxer(jnl_http_t http, unsigned int classtype, ifc_ultravox_playback **playback);
};