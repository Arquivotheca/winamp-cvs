#include "OggDecoder.h"
#include <vorbis/codec.h>


ns_error_t OggDecoderService::OggDecoder_CreateAudioDecoder(ogg_packet *packet, ifc_oggaudiodecoder **audio_decoder)
{
	if (vorbis_synthesis_idheader(packet) == 1)
	{

		// TODO: return NErr_Success;
		return NErr_NotImplemented;
	}
	return NErr_False;
}

/* -------------- */

OggVorbisDecoder::OggVorbisDecoder()
{
	vorbis_info_init(&info);
	vorbis_comment_init(&comment);
}