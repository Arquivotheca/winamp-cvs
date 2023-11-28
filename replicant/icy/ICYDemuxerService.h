#pragma once
#include "http/svc_http_demuxer.h"
#include "nx/nxstring.h"
#include "nx/nxonce.h"
#include "nswasabi/ServiceName.h"

// {BC4FEE96-FF26-4dff-8084-D9F237F6DE99}
static const GUID icy_demuxer_guid = 
{ 0xbc4fee96, 0xff26, 0x4dff, { 0x80, 0x84, 0xd9, 0xf2, 0x37, 0xf6, 0xde, 0x99 } };


class ICYDemuxerService : public svc_http_demuxer
{
public:
	ICYDemuxerService();
	~ICYDemuxerService();
	WASABI_SERVICE_NAME("ICY Demuxer");
	static GUID GetServiceGUID() { return icy_demuxer_guid; }

	const char *WASABICALL HTTPDemuxerService_EnumerateAcceptedTypes(size_t i);
	const char *WASABICALL HTTPDemuxerService_GetUserAgent();
	void WASABICALL HTTPDemuxerService_CustomizeHTTP(jnl_http_t http);
	NError WASABICALL HTTPDemuxerService_CreateDemuxer(nx_uri_t uri, jnl_http_t http, ifc_http_demuxer **demuxer, int pass);

private:
	nx_once_value_t once;
};