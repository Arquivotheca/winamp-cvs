#include "UltravoxMP3Service.h"
#include "UltravoxMP3.h"
#include "jnetlib/jnetlib.h"
#include <new>

NError UltravoxMP3Service::UltravoxPlaybackService_CreateDemuxer(jnl_http_t http, unsigned int classtype, ifc_ultravox_playback **playback)
{
	if (classtype == 0x7000 || classtype == 0x7001)
	{
		*playback = new (std::nothrow) UltravoxMP3(http);
		if (*playback)
			return NErr_Success;
		else
			return NErr_OutOfMemory;
	}
	return NErr_False;
}