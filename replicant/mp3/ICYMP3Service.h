#pragma once
#include "icy/svc_icy_playback.h"
#include "nswasabi/ServiceName.h"
// {98E21240-9BC2-4fd7-9AA5-BA05BDE5DFF7}
static const GUID icy_mp3_service_guid = 
{ 0x98e21240, 0x9bc2, 0x4fd7, { 0x9a, 0xa5, 0xba, 0x5, 0xbd, 0xe5, 0xdf, 0xf7 } };

class ICYMP3Service : public svc_icy_playback
{
public:
	WASABI_SERVICE_NAME("MP3 SHOUTcast Playback");
	static GUID GetServiceGUID() { return icy_mp3_service_guid; }
	const char *WASABICALL ICYPlaybackService_EnumerateAcceptedTypes(size_t i);
	NError WASABICALL ICYPlaybackService_CreateDemuxer(jnl_http_t http, ifc_icy_playback **playback, int pass);
};