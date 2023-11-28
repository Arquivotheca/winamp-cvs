#include "UltravoxAACService.h"
#include "UltravoxAAC.h"
#include "jnetlib/jnetlib.h"

NError UltravoxAACService::UltravoxPlaybackService_CreateDemuxer(jnl_http_t http, unsigned int classtype, ifc_ultravox_playback **playback)
{
	if (classtype == 0x8001 || classtype == 0x8003)
	{
		*playback = new UltravoxAAC(http);
		if (*playback)
			return NErr_Success;
		else
			return NErr_OutOfMemory;
	}
	return NErr_False;
}