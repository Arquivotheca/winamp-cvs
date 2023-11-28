#if 0
#include "main.h"
#include "adts_aacp.h"
#include "FactoryHelper.h"
#include "../winamp/wa_ipc.h"
#include "api.h"
#include "config.h"
#include "../nsutil/pcm.h"
#include <assert.h>

//#define USE_LIMITER
#define AAC_MAX_CHANNELS 6
#define TEMP_BUFFER_SAMPLES 2048
#define AAC_READ_BYTES 8192
#define AAC_SYNC_PASSES 10

ADTS_AACP::ADTS_AACP()
		: checkit(0), decoder(0), pDesc(0), pConf(0), readpos(0),
		outputFrameSize(0), inputread(0), gain(1.0f), channels(0)
{
	predelay = 0;

}


int ADTS_AACP::Initialize(bool forceMono, bool reverse_stereo, bool allowSurround, int maxBits, bool _allowRG, bool _useFloat)
{
	allowRG = _allowRG;
	useFloat = _useFloat;
	if (!decoder)
		ServiceBuild(decoder, aacPlusDecoderGUID);

	if (decoder)
	{
		AACPLUSDEC_OUTPUTFORMAT sampleFormat;
		int maxChannels;

		if (_useFloat)
		{
			sampleFormat = AACPLUSDEC_OUTPUTFORMAT_FLOAT;
			bitsPerSample = 32;
		}
		else if (maxBits >= 24)
		{
			bitsPerSample = 24;
		}
		else
		{
			bitsPerSample = 16;
		}

		if (allowSurround)
			maxChannels = AAC_MAX_CHANNELS;
		else if (AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"mono", false))
			maxChannels = 1;
		else
			maxChannels = 2;

		decoder->EasyOpen(AACPLUSDEC_OUTPUTFORMAT_FLOAT, AAC_MAX_CHANNELS);

		/* get access to the decoder settings */
		pConf = decoder->GetDecoderSettingsHandle();

		/* set a maximum number of channels */
		pConf->nDmxOutputChannels = maxChannels;

		/* enable the built-in limiter for higher quality at low bitrates */
#ifdef USE_LIMITER
		pConf->bEnableOutputLimiter = 1;
#else
		pConf->bEnableOutputLimiter = 0;
#endif

		/* enable upsampling, offers upsampling by a factor of 2 for plain AAC
		* bitstreams at or below 24 kHz */
		pConf->bDoUpsampling = 1;

		/* reinitialize the decoder with the altered settings */
		decoder->SetDecoderSettings();

		pDesc = decoder->GetStreamPropertiesHandle() ;
	}
	return !decoder;
}

bool ADTS_AACP::Open(ifc_mpeg_stream_reader *file)
{
	if (allowRG)
		gain = file->MPEGStream_Gain();

	if (!decoder)
		return false;



	memset((AACPLUSDEC_BITSTREAMBUFFERINFO*)&bitbufInfo, 0, sizeof(AACPLUSDEC_BITSTREAMBUFFERINFO));
	memset((AACPLUSDEC_AUDIOBUFFERINFO*)&audiobufInfo, 0, sizeof(AACPLUSDEC_AUDIOBUFFERINFO));
	memset((AACPLUSDEC_DATASTREAMBUFFERINFO*)&datastreamInfo, 0, sizeof(AACPLUSDEC_DATASTREAMBUFFERINFO));

	return true;
}

void ADTS_AACP::Close()
{
	if (decoder)
	{
		decoder->Close();
		ServiceRelease(decoder, aacPlusDecoderGUID);
		decoder = 0;
	}
}

void ADTS_AACP::GetOutputParameters(size_t *numBits, int *numChannels, int *sampleRate)
{
	*sampleRate = checkit->nOutputSamplingRate;
	*numChannels = channels;
	*numBits = bitsPerSample;
}

void ADTS_AACP::CalculateFrameSize(int *frameSize)
{
	*frameSize = outputFrameSize;
	if (*frameSize > 576*(bitsPerSample / 8)* channels)
		*frameSize = 576 * (bitsPerSample / 8) * channels;
}

void ADTS_AACP::Flush(ifc_mpeg_stream_reader *file)
{
	decoder->Restart();
	readpos = 0;
	inputread = 0;
}

size_t ADTS_AACP::GetCurrentBitrate()
{
	return (pDesc->nBitrate + 500) / 1000;
}

size_t ADTS_AACP::GetDecoderDelay()
{
	return predelay;
}

int ADTS_AACP::Sync(ifc_mpeg_stream_reader *file, unsigned __int8 *output, size_t outputSize, size_t *outputWritten, size_t *bitrate)
{
	// sync to stream
	if (!inputread)
	{
		file->MPEGStream_Read(buffer, AAC_READ_BYTES, &inputread);
		readpos = 0;
		if (file->MPEGStream_EOF())
			return ENDOFFILE;

	}
	if (inputread)
	{
		bitbufInfo.nBytesGivenIn = inputread;
		bitbufInfo.nBitsOffsetIn = 0;
		/* feed the content of the stream buffer into the internal decoder buffer */
		decoder->StreamFeed(buffer + readpos, &bitbufInfo);
		inputread -= bitbufInfo.nBytesReadOut;
		readpos += bitbufInfo.nBytesReadOut;
	}

	float floatTemp[TEMP_BUFFER_SAMPLES*AAC_MAX_CHANNELS];
	float *data = useFloat ? (float *)output : floatTemp;
	audiobufInfo.nBytesBufferSizeIn = useFloat ? outputSize : sizeof(floatTemp);
	datastreamInfo.nBytesBufferSizeIn = sizeof(dataStreamBuffer);

	AACPLUSDEC_ERROR result = decoder->StreamDecode(data,
	                          &audiobufInfo,
	                          (unsigned char *) & dataStreamBuffer,
	                          &datastreamInfo) ;

	if (result == AACPLUSDEC_OK)
	{
		if (datastreamInfo.nBytesWrittenOut >= 4 && dataStreamBuffer.magicWord == PRE_PADDING_MAGIC_WORD) // yay!
		{
			if (datastreamInfo.nBytesWrittenOut >= 6)
				predelay = dataStreamBuffer.padding; // should we make any effort to valid this?
		}

		pDesc = decoder->GetStreamPropertiesHandle();
		checkit = &(pDesc->programProperties[pDesc->nCurrentProgram]);
		channels = checkit->nOutputChannels;

		outputFrameSize = audiobufInfo.nBytesWrittenOut;
		size_t numSamples=outputFrameSize /sizeof(float);

		if (!useFloat)
		{
			nsutil_pcm_FloatToInt_Interleaved_Gain(output, data, bitsPerSample, numSamples, gain);
		}
		else
		{
			for (size_t i=0;i<numSamples;i++)
				data[i]=data[i]*gain;
		}

		*outputWritten = numSamples * (bitsPerSample/8);
		*bitrate = (pDesc->nBitrate + 500) / 1000;
		if (!*bitrate) *bitrate = MulDiv(bitbufInfo.nBytesReadOut, checkit->nAacSamplingRate, 128000);
		return SUCCESS;
	}
	else if (result == AACPLUSDEC_ERROR_NEEDMOREDATA)
	{
		// not enough bytes in buffer to do a decode
		//pass++;
		return NEEDMOREDATA;
	}
	else if (result == AACPLUSDEC_ERROR_ENDOFSTREAM)
	{
		// no more input left, we're done
		return ENDOFFILE;
	}
	else
	{
		// some bug?
		return FAILURE;
	}
}

int ADTS_AACP::Decode(ifc_mpeg_stream_reader *file, unsigned __int8 *output, size_t outputSize, size_t *outputWritten, size_t *bitrate, size_t *endCut)
{

	if (!inputread)
	{
		file->MPEGStream_Read(buffer, AAC_READ_BYTES, &inputread);
		readpos = 0;
	}
	if (inputread)
	{
		bitbufInfo.nBytesGivenIn = inputread;
		bitbufInfo.nBitsOffsetIn = 0;
		/* feed the content of the stream buffer into the internal decoder buffer */
		decoder->StreamFeed(buffer + readpos, &bitbufInfo);
		inputread -= bitbufInfo.nBytesReadOut;
		readpos += bitbufInfo.nBytesReadOut;
	}

	assert(channels);
	float floatTemp[TEMP_BUFFER_SAMPLES*AAC_MAX_CHANNELS];
	float *data = useFloat ? (float *)output : floatTemp;
	audiobufInfo.nBytesBufferSizeIn = useFloat ? outputSize : sizeof(float)*TEMP_BUFFER_SAMPLES*channels;

	datastreamInfo.nBytesBufferSizeIn = sizeof(dataStreamBuffer);

	AACPLUSDEC_ERROR	result = decoder->StreamDecode(data,
	                          &audiobufInfo,
	                          (unsigned char *) & dataStreamBuffer,
	                          &datastreamInfo);
	if (result == AACPLUSDEC_OK)
	{

		if (datastreamInfo.nBytesWrittenOut >= 4 && dataStreamBuffer.magicWord == POST_PADDING_MAGIC_WORD) // yay!
		{
			if (datastreamInfo.nBytesWrittenOut >= 6)
				*endCut = dataStreamBuffer.padding; // should we make any effort to valid this?

		}

		checkit = &(pDesc->programProperties[pDesc->nCurrentProgram]);
		size_t numSamples=audiobufInfo.nBytesWrittenOut/sizeof(float);

		if (!useFloat)
		{
						nsutil_pcm_FloatToInt_Interleaved_Gain(output, data, bitsPerSample, numSamples, gain);
		}
		else
		{
			for (size_t i=0;i<numSamples;i++)
				data[i]=data[i]*gain;
		}

		*outputWritten = numSamples * (bitsPerSample/8);

		size_t thisBitrate = (pDesc->nBitrate + 500) / 1000;
		if (!thisBitrate) // AAC+ decoder sometimes returns 0 for VBR
			thisBitrate = MulDiv(bitbufInfo.nBytesReadOut, checkit->nAacSamplingRate, 128000); // so we need to calculate it manually
		*bitrate = thisBitrate;
		return adts::SUCCESS;
	}
	else if (result == AACPLUSDEC_ERROR_ENDOFSTREAM)
	{
		return adts::ENDOFFILE;
	}
	else if (file->MPEGStream_EOF())
	{
		return adts::ENDOFFILE;
	}
	else if (result == AACPLUSDEC_ERROR_NEEDMOREDATA)
	{
		*bitrate = pDesc->nBitrate / 1000;
		return adts::NEEDMOREDATA;
	}
	else if (result && (result != AACPLUSDEC_ERROR_NEEDMOREDATA && result != AACPLUSDEC_ERROR_ENDOFSTREAM && result != AACPLUSDEC_OK))
	{
		return adts::FAILURE;
	}
	else
		return adts::SUCCESS;

}


#endif