#include "ogg_vorbis_decoder.h"
#include <string.h>

ifc_oggdecoder *OggDecoderFactory::CreateDecoder(const ogg_packet *packet)
{
	if (packet && packet->packet && packet->bytes >= 30)
	{
		if (!memcmp(packet->packet + 1, "vorbis", 6))
		{
			OggVorbisDecoder *decoder = new OggVorbisDecoder();
			if (decoder)
			{
				vorbis_info_init(&decoder->info);
				vorbis_comment_init(&decoder->comment);
				
				if (vorbis_synthesis_headerin(&decoder->info, &decoder->comment, const_cast<ogg_packet *>(packet)))
				{
					delete decoder;
					return 0;
				}
				return decoder;
			}
		}
	}
	return 0;
}

#define CBCLASS OggDecoderFactory
START_DISPATCH;
CB(DISP_CREATEDECODER, CreateDecoder)
END_DISPATCH;
#undef CBCLASS



OggVorbisDecoder::OggVorbisDecoder()
{
	bps=16; // TODO
}

#define CBCLASS OggVorbisDecoder
START_DISPATCH;
END_DISPATCH;
#undef CBCLASS
