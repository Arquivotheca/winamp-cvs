#include "aacplusdec.h"
#include "../nsv/nsvlib.h"
#include "../nsv/dec_if.h"
#include <string.h>
#include <bfc/platform/export.h>
#include "nsvdec.h"
#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif


EAACP_Decoder::EAACP_Decoder() : aacPlus(0)
{
		aacPlusDecEasyOpen(&aacPlus,AACPLUSDEC_OUTPUTFORMAT_INT16_HOSTENDIAN, 6);
		/* get access to the decoder settings */
		AACPLUSDEC_EXPERTSETTINGS *pConf = aacPlusDecGetDecoderSettingsHandle(aacPlus);
		/* enable the built-in limiter for higher quality at low bitrates */
		pConf->bEnableOutputLimiter = 1;
		/* enable upsampling, offers upsampling by a factor of 2 for plain AAC
		* bitstreams at or below 24 kHz */
		pConf->bDoUpsampling = 1;
		/* reinitialize the decoder with the altered settings */
		aacPlusDecSetDecoderSettings(aacPlus);

		memset((AACPLUSDEC_BITSTREAMBUFFERINFO*)&bitbufInfo, 0, sizeof(AACPLUSDEC_BITSTREAMBUFFERINFO));
		memset((AACPLUSDEC_AUDIOBUFFERINFO*)&audiobufInfo, 0, sizeof(AACPLUSDEC_AUDIOBUFFERINFO));

		pcm_buf_used = 0;
		cbvalid = 0;
		readpos = 0;
}

EAACP_Decoder::~EAACP_Decoder()
{
	if (aacPlus)
	{
		aacPlusDecClose(&aacPlus);
	}
}

void EAACP_Decoder::flush()
{
	if (aacPlus)
		aacPlusDecRestart(aacPlus);
	cbvalid = 0;
	pcm_buf_used = 0;
	readpos = 0;
}

bool EAACP_Decoder::OK()
{
	return aacPlus != 0;
}

void EAACP_Decoder::FillOutputFormat(unsigned int out_fmt[8])
{
	AACPLUSDEC_STREAMPROPERTIES *pDesc = aacPlusDecGetStreamPropertiesHandle(aacPlus);
	AACPLUSDEC_PROGRAMPROPERTIES *checkit = &(pDesc->programProperties[pDesc->nCurrentProgram]);
	if (pDesc->nDecodingState != AACPLUSDEC_DECODINGSTATE_IDLE
		&& checkit)
	{
		out_fmt[0] = NSV_MAKETYPE('P', 'C', 'M', ' ');
		out_fmt[1] = checkit->nOutputSamplingRate;
		out_fmt[2] = checkit->nOutputChannels;
		out_fmt[3] = 16;
	}
	else
	{
		out_fmt[0] = 0;
		out_fmt[1] = 0;
		out_fmt[2] = 0;
		out_fmt[3] = 0;
	}
	if (pDesc)
		out_fmt[4] = pDesc->nBitrate;
}

void EAACP_Decoder::CopyToOutput(void *out, int *out_len)
{
	int l = MIN(pcm_buf_used, *out_len);

	memcpy(out, pcm_buf, l);
	*out_len = l;
	pcm_buf_used -= l;
	if (pcm_buf_used)
		memmove(pcm_buf, pcm_buf + l, pcm_buf_used);
}

int EAACP_Decoder::decode(void *in, int in_len, void *out, int *out_len, unsigned int out_fmt[8])
{
	if (!aacPlus)
		return 0;

	AACPLUSDEC_ERROR result;

	if (pcm_buf_used)
	{
		FillOutputFormat(out_fmt);
		CopyToOutput(out, out_len);
		return 1; // call me again with same data
	}

	if (cbvalid == 0)
	{
		cbvalid = in_len;
		readpos=0;
	}

	if (cbvalid)
	{
		bitbufInfo.nBytesGivenIn = cbvalid;
		bitbufInfo.nBitsOffsetIn = 0;
		/* feed the content of the stream buffer into the internal decoder buffer */

		result = aacPlusStreamFeed(aacPlus,(unsigned char *)in + readpos, &bitbufInfo);
		cbvalid -= bitbufInfo.nBytesReadOut;
		readpos += bitbufInfo.nBytesReadOut;
	}
	audiobufInfo.nBytesBufferSizeIn = (int)65536;

	result = aacPlusStreamDecode(aacPlus,pcm_buf + pcm_buf_used,
		&audiobufInfo,
		0,
		0);

	if (result == AACPLUSDEC_OK)
	{
		pcm_buf_used += audiobufInfo.nBytesWrittenOut;
	}
	else if (result == AACPLUSDEC_ERROR_NEEDMOREDATA)
	{
		// not enough bytes in buffer to do a decode
	}
	else if ( result == AACPLUSDEC_ERROR_ENDOFSTREAM)
	{
		// no more input left, we're done
	}

	FillOutputFormat(out_fmt);
	CopyToOutput(out, out_len);

	if (cbvalid)
		return 1;
	else
		return 0;

}
/*
extern "C"
{
	DLLEXPORT IAudioDecoder *CreateAudioDecoder(unsigned int fmt, IAudioOutput **output)
	{
		switch (fmt)
		{
		case NSV_MAKETYPE('A', 'A', 'C', ' ') :							
		case NSV_MAKETYPE('A', 'A', 'C', 'P'):
		case NSV_MAKETYPE('A', 'P', 'L', ' '):
			{
				EAACP_Decoder *dec = new EAACP_Decoder;
				if (!dec->OK())
				{
					delete dec;
					dec = 0;
				}
				return dec;
			}

		default:
			return NULL;
		}
	}

	DLLEXPORT void DeleteAudioDecoder(IAudioDecoder *decoder)
	{
		if (decoder)
			delete decoder;

	}
}
*/