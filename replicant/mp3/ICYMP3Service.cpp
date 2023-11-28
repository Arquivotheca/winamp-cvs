#include "ICYMP3Service.h"
#include "ICYMP3.h"
#include "jnetlib/jnetlib.h"
#include "nswasabi/ReferenceCounted.h"
#include <new>

const char *ICYMP3Service::ICYPlaybackService_EnumerateAcceptedTypes(size_t i)
{
	switch(i)
	{
	case 0:
		return "audio/mpeg";
	default:
		return 0;
	}
}

NError ICYMP3Service::ICYPlaybackService_CreateDemuxer(jnl_http_t http, ifc_icy_playback **playback, int pass)
{
	const char *content_type = jnl_http_getheader(http, "content-type");
	if (content_type && !strcmp(content_type, "audio/mpeg"))
	{
		ICYMP3 *icy_mp3 = new (std::nothrow) ReferenceCounted<ICYMP3>;
		if (!icy_mp3)
			return NErr_OutOfMemory;
		int ret = icy_mp3->Initialize(http);
		if (ret != NErr_Success)
		{
			delete icy_mp3;
			return ret;
		}
		*playback = icy_mp3;
		return NErr_Success;
	}
	return NErr_False;
}