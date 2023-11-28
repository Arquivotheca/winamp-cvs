#pragma once
#include "icy/svc_icy_playback.h"
#include "nswasabi/ServiceName.h"

// {51B7070D-3A55-41F4-AAD2-E34ACFABBCFE}
static const GUID icy_aac_service_guid = 
{ 0x51b7070d, 0x3a55, 0x41f4, { 0xaa, 0xd2, 0xe3, 0x4a, 0xcf, 0xab, 0xbc, 0xfe } };


class ICYAACService : public svc_icy_playback
{
public:
	WASABI_SERVICE_NAME("ICY AAC Playback");
	static GUID GetServiceGUID() { return icy_aac_service_guid; }
	const char *WASABICALL ICYPlaybackService_EnumerateAcceptedTypes(size_t i);
	NError WASABICALL ICYPlaybackService_CreateDemuxer(jnl_http_t http, ifc_icy_playback **playback, int pass);
};