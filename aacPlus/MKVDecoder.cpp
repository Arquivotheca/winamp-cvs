#include "MKVDecoder.h"
#include "MKVAACDecoder.h"

int MKVDecoder::CreateAudioDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data, unsigned int preferred_bits, unsigned int max_channels,bool floating_point, ifc_mkvaudiodecoder **decoder)
{
	if (!strcmp(codec_id, "A_AAC"))
	{
		MKVAACDecoder *aac_decoder = MKVAACDecoder::Create(track_entry_data, audio_data, preferred_bits, max_channels, floating_point);
		if (aac_decoder)
		{
			*decoder = aac_decoder;
			return CREATEDECODER_SUCCESS;
		}
		return CREATEDECODER_FAILURE;
	}

	return CREATEDECODER_NOT_MINE;
}

#define CBCLASS MKVDecoder
START_DISPATCH;
CB(CREATE_AUDIO_DECODER, CreateAudioDecoder)
END_DISPATCH;
#undef CBCLASS