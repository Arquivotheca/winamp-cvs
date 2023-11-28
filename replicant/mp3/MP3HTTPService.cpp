#include "api.h"
#include "MP3HTTPService.h"
#include "MP3HTTP.h"
#include "jnetlib/jnetlib.h"
#include "service/ifc_servicefactory.h"
#include "nswasabi/ReferenceCounted.h"
#include <new>

static const char *mpeg_audio_mime_types[] = {
	"audio/mp3",
	"audio/mpeg",
	"audio/mpg",
	"audio/x-mp3",
	"audio/x-mpeg",
	"audio/x-mpeg3",
	"audio/x-mpegaudio",
	"audio/x-mpg",
};
MP3HTTPService::MP3HTTPService()
{

}

MP3HTTPService::~MP3HTTPService()
{
}

const char *MP3HTTPService::HTTPDemuxerService_EnumerateAcceptedTypes(size_t i)
{
	if (i < sizeof(mpeg_audio_mime_types) / sizeof(*mpeg_audio_mime_types))
		return mpeg_audio_mime_types[i];
	else
		return 0;

	return 0;
}

const char *MP3HTTPService::HTTPDemuxerService_GetUserAgent()
{
	return 0;
}

void MP3HTTPService::HTTPDemuxerService_CustomizeHTTP(jnl_http_t http)
{
}
static bool AcceptableMIMEType(const char *mime_type)
{
	for(size_t i=0;i< sizeof(mpeg_audio_mime_types) / sizeof(*mpeg_audio_mime_types);i++)
	{
		if (!strcmp(mime_type, mpeg_audio_mime_types[i]))
			return true;
	}
	return false;
}
NError MP3HTTPService::HTTPDemuxerService_CreateDemuxer(nx_uri_t uri, jnl_http_t http, ifc_http_demuxer **demuxer, int pass)
{
	if (pass == 0)
		return NErr_TryAgain;

	if (pass == 1) /* wait for second pass to let shoutcast demuxer have a chance */
	{
		const char *content_type = jnl_http_getheader(http, "content-type");
		if (content_type && AcceptableMIMEType(content_type))
		{
			MP3HTTP *mp3_demuxer = new (std::nothrow) ReferenceCounted<MP3HTTP>;
			if (!mp3_demuxer)
				return NErr_OutOfMemory;
			mp3_demuxer->Initialize(http);
			*demuxer = mp3_demuxer;
			return NErr_Success;
		}
	}

	return NErr_False;
}
