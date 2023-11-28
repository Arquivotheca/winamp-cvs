#pragma once
#include "http/svc_http_demuxer.h"
#include "nx/nxstring.h"
#include "nx/nxonce.h"
#include "nswasabi/ServiceName.h"
// {5BB9E25C-A0EF-46ce-8898-03AD68752FD7}
static const GUID mp3_demuxer_guid = 
{ 0x5bb9e25c, 0xa0ef, 0x46ce, { 0x88, 0x98, 0x3, 0xad, 0x68, 0x75, 0x2f, 0xd7 } };


class MP3HTTPService : public svc_http_demuxer
{
public:
	MP3HTTPService();
	~MP3HTTPService();
	WASABI_SERVICE_NAME("MP3 HTTP Demuxer");
	static GUID GetServiceGUID() { return mp3_demuxer_guid; }

	const char *WASABICALL HTTPDemuxerService_EnumerateAcceptedTypes(size_t i);
	const char *WASABICALL HTTPDemuxerService_GetUserAgent();
	void WASABICALL HTTPDemuxerService_CustomizeHTTP(jnl_http_t http);
	NError WASABICALL HTTPDemuxerService_CreateDemuxer(nx_uri_t uri, jnl_http_t http, ifc_http_demuxer **demuxer, int pass);
};