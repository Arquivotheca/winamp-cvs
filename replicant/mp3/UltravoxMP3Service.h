#pragma once
#include "ultravox/svc_ultravox_playback.h"
#include "nswasabi/ServiceName.h"

// {747DBD85-9655-49E6-B45F-E582A8DA99C4}
static const GUID ultravox_mp3_service_guid = 
{ 0x747dbd85, 0x9655, 0x49e6, { 0xb4, 0x5f, 0xe5, 0x82, 0xa8, 0xda, 0x99, 0xc4 } };

class UltravoxMP3Service : public svc_ultravox_playback
{
public:
	WASABI_SERVICE_NAME("MP3 Ultravox Playback");
	static GUID GetServiceGUID() { return ultravox_mp3_service_guid; }

	NError WASABICALL UltravoxPlaybackService_CreateDemuxer(jnl_http_t http, unsigned int classtype, ifc_ultravox_playback **playback);
};