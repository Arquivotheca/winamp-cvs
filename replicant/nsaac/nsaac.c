#include "nsaac/nsaac.h"
#include "aacdec/aacdecoder.h"
#include "sbrdec/sbrdecoder.h"
#include "bitbuffer.h"
#include "foundation/error.h"
#include "ADTSHeader.h"
#include <stdlib.h>

typedef struct nsaac_decoder_object_t
{
	AACDECODER aac_decoder;
	struct BIT_BUF aac_bitstream;
	SBRDECODER sbr_decoder;
	SBRBITSTREAM sbr_bitstream;
	//float *time_data; /* decoder output */
	uint8_t bit_buffer[8192];
	float TimeDataFloat[4096];
	int adts;
	nsaac_frameinfo_value_t frame_info;
	ADTSHeader adts_header;
} NSAACDEC;

int nsaac_decoder_create(nsaac_decoder_t *out_decoder, int max_channels)
{
	NSAACDEC *decoder = (NSAACDEC *)malloc(sizeof(NSAACDEC));
	if (!decoder)
		return NErr_OutOfMemory;
	CreateBitBuffer(&decoder->aac_bitstream, decoder->bit_buffer, 8192);
	/* limit max channels */
	if (max_channels > 2)
		max_channels = 2;
	/* TODO: might be better to wait until we get the real channel count */
	//decoder->time_data = malloc(sizeof(float) * max_channels * FRAME_SIZE);
	decoder->adts = 0;
	decoder->sbr_decoder = 0;
	*out_decoder = decoder;
	return NErr_Success;
}

int nsaac_decoder_init_from_adts(nsaac_decoder_t decoder, const void *adts_data, size_t data_size)
{
	int ret = nsaac_adts_parse(&decoder->adts_header, (const uint8_t *)adts_data);
	if (ret != NErr_Success)
		return ret;

	decoder->aac_decoder = CAacDecoderOpen(&decoder->aac_bitstream, &decoder->sbr_bitstream, decoder->TimeDataFloat);
	if (!decoder->aac_decoder)
		return NErr_OutOfMemory;
	decoder->adts = 1;

	ret = CAacDecoderInit(decoder->aac_decoder, nsaac_adts_get_samplerate(&decoder->adts_header), 0);
	if (ret != NErr_Success)
	{
		// TODO: CAacDecoderClose(decoder->aac_decoder);
		decoder->aac_decoder = 0;
	}

	return ret;
}

int nsaac_decoder_decode_frame(nsaac_decoder_t decoder, nsaac_frameinfo_t frame_info, const void *data, size_t data_size)
{
	size_t i;

	int frameSize, sampleRate, numChannels;
	char channelMode;
	const uint8_t *data8 = (const uint8_t *)data;

	/* parse the ADTS header if this is an ADTS stream */
	if (decoder->adts)
	{
		int ret;
		ADTSHeader this_adts_header;
		if (data_size < 7)
			return NErr_NeedMoreData;

		ret = nsaac_adts_parse(&this_adts_header, (const uint8_t *)data);
		if (ret != NErr_Success)
			return ret;

		if (nsaac_adts_match(&decoder->adts_header, &this_adts_header) != NErr_True)
			return NErr_Changed;

		if (this_adts_header.protection == 0)
		{
			if (data_size < 9)
				return NErr_NeedMoreData;
			data8 += 9;
			data_size -= 9;
		}
		else
		{
			data8 += 7;
			data_size -= 7;
		}
	}

	// TODO: optimize this
	for (i=0; i<data_size; i++)
		WriteBits(&decoder->aac_bitstream,data8[i], 8);

	decoder->sbr_bitstream.NrElements = 0;
	if (CAacDecoder_DecodeFrame(decoder->aac_decoder,	&frameSize, &sampleRate, &numChannels, &channelMode, 1))
	{
		return NErr_Error;
	}
	else
	{
		decoder->frame_info.channels = numChannels;
		decoder->frame_info.frame_size = frameSize;
		decoder->frame_info.sample_rate = sampleRate;
		decoder->frame_info.channel_mode = channelMode;
	
#if 1
		if (!decoder->sbr_decoder && decoder->sbr_bitstream.NrElements) 
		{
			decoder->sbr_decoder = openSBR (sampleRate,frameSize, 0, 0);
		}
		if (decoder->sbr_decoder) 
		{
			/* apply SBR processing */
			if (applySBR(decoder->sbr_decoder, &decoder->sbr_bitstream, decoder->TimeDataFloat, &numChannels, 1,  0,  0) == SBRDEC_OK)
			{
				decoder->frame_info.channels = numChannels;
				decoder->frame_info.frame_size *= 2;
				decoder->frame_info.sample_rate *= 2;
				// TODO? for parametric stereo? decoder->frame_info.channel_mode = channelMode;		
			}
		}
#endif

		decoder->frame_info.output_data = decoder->TimeDataFloat;
		*frame_info = decoder->frame_info;
		
		return NErr_Success;
		
	}
}