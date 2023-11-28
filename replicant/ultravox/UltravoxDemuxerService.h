#pragma once
#include "http/svc_http_demuxer.h"
#include "nx/nxstring.h"
#include "nx/nxonce.h"

// {3BEAC214-2AC8-477F-92B1-9247A9771124}
static const GUID ultravox_demuxer_guid = 
{ 0x3beac214, 0x2ac8, 0x477f, { 0x92, 0xb1, 0x92, 0x47, 0xa9, 0x77, 0x11, 0x24 } };


class UltravoxDemuxerService : public svc_http_demuxer
{
public:
	UltravoxDemuxerService();
	~UltravoxDemuxerService();
	static nx_string_t GetServiceName() { return 0; } // TODO!!
	static GUID GetServiceGUID() { return ultravox_demuxer_guid; }

	const char *WASABICALL HTTPDemuxerService_EnumerateAcceptedTypes(size_t i);
	const char *WASABICALL HTTPDemuxerService_GetUserAgent();
	void WASABICALL HTTPDemuxerService_CustomizeHTTP(jnl_http_t http);
	NError WASABICALL HTTPDemuxerService_CreateDemuxer(nx_uri_t uri, jnl_http_t http, ifc_http_demuxer **demuxer, int pass);

private:
	static nx_once_value_t once;
};