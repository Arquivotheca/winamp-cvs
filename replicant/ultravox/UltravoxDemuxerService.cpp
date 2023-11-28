#include "UltravoxDemuxerService.h"
#include "UltravoxDemuxer.h"
#include "jnetlib/jnetlib.h"
#include "service/ifc_servicefactory.h"
#include "nu/strsafe.h"
#include "api.h"

UltravoxDemuxerService::UltravoxDemuxerService()
{
	
}

UltravoxDemuxerService::~UltravoxDemuxerService()
{
	}

const char *UltravoxDemuxerService::HTTPDemuxerService_EnumerateAcceptedTypes(size_t i)
{
	if (i == 0)
		return "misc/ultravox";

	return 0;
}

const char *UltravoxDemuxerService::HTTPDemuxerService_GetUserAgent()
{
	return "Ultravox/2.1";
}

void UltravoxDemuxerService::HTTPDemuxerService_CustomizeHTTP(jnl_http_t http)
{
	jnl_http_addheader(http, "Ultravox-transport-type: TCP");
}

NError UltravoxDemuxerService::HTTPDemuxerService_CreateDemuxer(nx_uri_t uri, jnl_http_t http, ifc_http_demuxer **demuxer, int pass)
{
	const char *content_type = jnl_http_getheader(http, "content-type");
	if (content_type && !strncmp(content_type, "misc/ultravox", 13))
	{
		*demuxer = new UltravoxDemuxer(http);
		return NErr_Success;
	}

	// TODO: maybe try some other headers like Ultravox-SID
	return NErr_False;
}
