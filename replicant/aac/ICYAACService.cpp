#include "ICYAACService.h"
#include "ICYAAC.h"
#include "jnetlib/jnetlib.h"
#include "nswasabi/ReferenceCounted.h"

const char *ICYAACService::ICYPlaybackService_EnumerateAcceptedTypes(size_t i)
{
	switch(i)
	{
	case 0:
		return "audio/aac";
	case 1:
		return "audio/aacp";
	default:
		return 0;
	}
}

NError ICYAACService::ICYPlaybackService_CreateDemuxer(jnl_http_t http, ifc_icy_playback **playback, int pass)
{
	const char *content_type = jnl_http_getheader(http, "content-type");
	if (!strcmp(content_type, "audio/aac") || !strcmp(content_type, "audio/aacp"))
	{
		ICYAAC *icy_aac = new (std::nothrow) ReferenceCounted<ICYAAC>;
		if (!icy_aac)
			return NErr_OutOfMemory;

		int ret = icy_aac->Initialize(http);
		if (ret != NErr_Success)
		{
			delete icy_aac;
			return ret;
		}

		*playback = icy_aac;
		return NErr_Success;
	}
	return NErr_False;
}