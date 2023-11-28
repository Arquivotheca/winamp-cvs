#include "flv_mp3_decoder.h"
#include "main.h"
#include "../nsutil/pcm.h"

int FLVDecoderCreator::CreateAudioDecoder(int stereo, int bits, int sample_rate, int format_type, ifc_flvaudiodecoder **decoder)
{
	if (format_type == FLV::AUDIO_FORMAT_MP3 || format_type == FLV::AUDIO_FORMAT_MP3_8KHZ)
	{
		CMpgaDecoder *ctx = new CMpgaDecoder();
		if (!ctx)
			return CREATEDECODER_FAILURE;
		*decoder = new FLVMP3(ctx);
		return CREATEDECODER_SUCCESS;
	}
	return CREATEDECODER_NOT_MINE;
}

int FLVDecoderCreator::HandlesAudio(int format_type)
{
	if (format_type == FLV::AUDIO_FORMAT_MP3 || format_type == FLV::AUDIO_FORMAT_MP3_8KHZ)
	{
		return CREATEDECODER_SUCCESS;
	}
	return CREATEDECODER_NOT_MINE;
}

#define CBCLASS FLVDecoderCreator
START_DISPATCH;
CB(CREATE_AUDIO_DECODER, CreateAudioDecoder)
CB(HANDLES_AUDIO, HandlesAudio)
END_DISPATCH;
#undef CBCLASS

/* --- */
#define FHG_DELAY 529
FLVMP3::FLVMP3(CMpgaDecoder *mp3) : mp3(mp3)
{
	bits = 16;
	pregap = FHG_DELAY;
	max_channels = 2;
}

int FLVMP3::GetOutputFormat(unsigned int *sample_rate, unsigned int *channels, unsigned int *_bits)
{
	if (mp3)
	{
		*sample_rate = mp3->m_Info.GetEffectiveSFreq();
		*channels = mp3->m_Info.GetEffectiveChannels();
		*_bits = bits;
		return FLV_AUDIO_SUCCESS;
	}
	else
	{
		return FLV_AUDIO_FAILURE;
	}
}

int FLVMP3::DecodeSample(const void *input_buffer, size_t input_buffer_bytes, void *samples, size_t *samples_size_bytes, double *bitrate)
{
	if (!mp3)
		return FLV_AUDIO_FAILURE;

	size_t output_buffer_len = *samples_size_bytes;
	*samples_size_bytes = 0;
	*bitrate = 0;
	const uint8_t *mp3_buffer = (const uint8_t *)input_buffer;
	while (input_buffer_bytes)
	{

		// feed compressed data into the decoder
		int filled = mp3->Fill((const unsigned char *)mp3_buffer, input_buffer_bytes);
		input_buffer_bytes -= filled;
		mp3_buffer += filled;

		// get the decoded data out
		int pcm_buf_used=0;
		float decodeBuf[1152*2*2];
		SSC ret = mp3->DecodeFrame(decodeBuf, sizeof(decodeBuf), &pcm_buf_used);

		if (ret == SSC_W_MPGA_SYNCNEEDDATA || ret == SSC_W_MPGA_SYNCSEARCHED)
		{
			*samples_size_bytes = 0;
			return FLV_AUDIO_NEEDS_MORE_INPUT;
		}
		else if (SSC_SUCCESS(ret))
		{
			// deal with pregap
			int numSamples = pcm_buf_used / sizeof(float);
			int offset = min(numSamples, pregap * mp3->m_Info.GetEffectiveChannels());
			numSamples -= offset;
			pregap -= offset / mp3->m_Info.GetEffectiveChannels();
			float *pcm_buf = decodeBuf + offset;

			// convert to destination sample format
			
			nsutil_pcm_FloatToInt_Interleaved(samples, pcm_buf, bits, numSamples);

			*samples_size_bytes += numSamples * bits / 8;
			samples = (char *)samples + numSamples * bits / 8;

			*bitrate = (double)mp3->GetStreamInfo()->GetBitrate() / 1000.0;
		}
		else
			return FLV_AUDIO_FAILURE;
	}
	return FLV_AUDIO_SUCCESS;
}

void FLVMP3::Flush()
{
	mp3->Reset();
	pregap = FHG_DELAY;
}

void FLVMP3::Close()
{
	delete mp3;
	delete this;
}

void FLVMP3::SetPreferences(unsigned int _max_channels, unsigned int preferred_bits)
{
	if (preferred_bits)
		bits = preferred_bits;

	if (max_channels > _max_channels)
		max_channels = _max_channels;
}

#define CBCLASS FLVMP3
START_DISPATCH;
CB(FLV_AUDIO_GETOUTPUTFORMAT, GetOutputFormat)
CB(FLV_AUDIO_DECODE, DecodeSample)
VCB(FLV_AUDIO_FLUSH, Flush)
VCB(FLV_AUDIO_CLOSE, Close)
VCB(FLV_AUDIO_SETPREFERENCES, SetPreferences)
END_DISPATCH;
#undef CBCLASS
