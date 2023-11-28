#include "avi_mp3_decoder.h"
#include "main.h"
#include "../nsutil/pcm.h"

int AVIDecoder::CreateAudioDecoder(const nsavi::AVIH *avi_header, 
																	 const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, 
																	 unsigned int preferred_bits, unsigned int max_channels, bool floating_point, 
																	 ifc_aviaudiodecoder **decoder)
{
	nsavi::mp3_format *waveformat = (nsavi::mp3_format *)stream_format;

	if (waveformat->format.format == nsavi::audio_format_mp3
		||waveformat->format.format == nsavi::audio_format_mp2)
	{
		CMpgaDecoder *ctx = new CMpgaDecoder();
		if (!ctx)
			return CREATEDECODER_FAILURE;
		*decoder = new AVIMP3Decoder(ctx, preferred_bits, max_channels, floating_point);
		return CREATEDECODER_SUCCESS;
	}

	return CREATEDECODER_NOT_MINE;

}

#define CBCLASS AVIDecoder
START_DISPATCH;
CB(CREATE_AUDIO_DECODER, CreateAudioDecoder)
END_DISPATCH;
#undef CBCLASS

#define FHG_DELAY 529
AVIMP3Decoder::AVIMP3Decoder(CMpgaDecoder *mp3, unsigned int bps, unsigned max_channels, bool floating_point)
: mp3(mp3), bits(bps?bps:16), max_channels(max_channels?max_channels:2), floating_point(floating_point)
{
	pregap = FHG_DELAY;
}

int AVIMP3Decoder::OutputFrameSize(size_t *frame_size)
{
	*frame_size = max_channels * 1152 * bits/8; 
	if (mp3->GetStreamInfo()->GetLayer() == 1)
		*frame_size *= 3;
	return AVI_SUCCESS;
}

int AVIMP3Decoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
{
	if (mp3)
	{
		*sampleRate = mp3->m_Info.GetEffectiveSFreq();
		*channels = mp3->m_Info.GetEffectiveChannels();
		*bitsPerSample = bits;
		*isFloat = floating_point;
		return AVI_SUCCESS;
	}
	else
	{
		return AVI_FAILURE;
	}
}

int AVIMP3Decoder::DecodeChunk(uint16_t type, void **inputBuffer, size_t *inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	if (!mp3)
		return AVI_FAILURE;

	size_t output_buffer_len = *outputBufferBytes;
	*outputBufferBytes = 0;
	const uint8_t *mp3_buffer = (const uint8_t *)*inputBuffer;
	while (*inputBufferBytes && output_buffer_len >= (1152*2*bits/8))
	{
		// feed compressed data into the decoder
		int filled = mp3->Fill((const unsigned char *)mp3_buffer, *inputBufferBytes);
		*inputBufferBytes -= filled;
		mp3_buffer += filled;

		// get the decoded data out
		int pcm_buf_used=0;
		float decodeBuf[1152*2*2]; 
		// TODO: if floating_point is true and pregap is 0, decode straight into outputBuffer
		SSC ret = mp3->DecodeFrame(decodeBuf, sizeof(decodeBuf), &pcm_buf_used);

		if (ret == SSC_W_MPGA_SYNCNEEDDATA || ret == SSC_W_MPGA_SYNCSEARCHED)
		{
			*outputBufferBytes = 0;
			*inputBuffer = (void *)mp3_buffer;
			return AVI_NEED_MORE_INPUT;
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
			if (floating_point)
			{
				memcpy(outputBuffer, pcm_buf, numSamples*bits/8);
			}
			else
			{
				nsutil_pcm_FloatToInt_Interleaved_Gain(outputBuffer, pcm_buf, bits, numSamples, 1.0);
			}

			*outputBufferBytes += numSamples * bits / 8;
			outputBuffer = (char *)outputBuffer + numSamples * bits / 8;
			output_buffer_len -= numSamples * bits / 8;
		}
		else
			return AVI_FAILURE;
	}
	*inputBuffer = (void *)mp3_buffer;
	return AVI_SUCCESS;
}

void AVIMP3Decoder::Flush()
{
	mp3->Reset();
	pregap = FHG_DELAY;
}

void AVIMP3Decoder::Close()
{
	delete mp3;
	delete this;
}

#define CBCLASS AVIMP3Decoder
START_DISPATCH;
CB(OUTPUT_FRAME_SIZE, OutputFrameSize)
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_CHUNK, DecodeChunk)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
END_DISPATCH;
#undef CBCLASS