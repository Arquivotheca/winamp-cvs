#include "../nsv/nsvlib.h"
#include "../nsv/dec_if.h"
#include "main.h"
#include "nsvmain.h"
#include "nsvAACP.h"
#include "../nsutil/pcm.h"

int MP3_Decoder::decode(void *in, int in_len,
                        void *out, int *out_len,
                        unsigned int out_fmt[8])
{
	int rval = 1;
	if (fused < in_len)
	{
		int l = mp3_dec.GetInputFree();
		if (l > in_len - fused) l = in_len - fused;
		if (l) mp3_dec.Fill((unsigned char *)in + fused, l);
		fused += l;
	}

	if (!pcm_buf_used)
	{
		SSC s = mp3_dec.DecodeFrame(pcm_buf, sizeof(pcm_buf), &pcm_buf_used);
		
		pcm_offs = 0;
	}

	if (pcm_buf_used)
	{
		size_t numSamples = *out_len / 2;
		if (numSamples > (pcm_buf_used/sizeof(float)))
			numSamples = pcm_buf_used/sizeof(float);
			nsutil_pcm_FloatToInt_Interleaved(out, pcm_buf+pcm_offs, 16, numSamples);
		pcm_buf_used -= numSamples*sizeof(float);
		pcm_offs += numSamples;
		*out_len = 2*numSamples;
	}
	else
	{
		if (fused >= in_len) { fused = 0; rval = 0; }
		*out_len = 0;
	}
	int nch = mp3_dec.m_Info.GetEffectiveChannels();
	int srate = mp3_dec.m_Info.GetEffectiveSFreq();
	out_fmt[0] = (nch && srate) ? NSV_MAKETYPE('P', 'C', 'M', ' ') : 0;
	out_fmt[1] = srate;
	out_fmt[2] = nch;
	out_fmt[3] = (nch && srate) ? 16 : 0;
	out_fmt[4] = mp3_dec.m_Info.GetBitrate();

	return rval;
}



extern "C"
{
	__declspec(dllexport) IAudioDecoder *CreateAudioDecoder(unsigned int fmt, IAudioOutput **output)
	{
		switch (fmt)
		{
		case NSV_MAKETYPE('M', 'P', '3', ' '):
			return new MP3_Decoder;

#ifdef AAC_SUPPORT
/*
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
				*/
#endif

		default:
			return NULL;
		}
	}

	__declspec(dllexport) void DeleteAudioDecoder(IAudioDecoder *decoder)
	{
		if (decoder)
			delete decoder;

	}
}
