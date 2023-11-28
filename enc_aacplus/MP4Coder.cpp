#include "Encoders.h"
#include "AncillaryData.h"
#include "../nu/AutoWide.h"
#include <assert.h>

MP4Coder::MP4Coder(int nch, int srate, int bps, bitstreamFormat _bitstream, sbrSignallingMode _signallingMode)
: AudioCoderCommon(nch, srate, bps, BSFORMAT_RAW, _signallingMode),
bytesThisSample(0), padding(0), resampling(false), postpadding(0), totalSamples(0)
{

}

void MP4Coder::Start()
{
	if (bitstream == BSFORMAT_RAW)
	{
		unsigned char  audioSpecConfig[32] = {0};
		unsigned int nConfigBits  = 0;
		unsigned int nConfigBytes = 0;

		aacPlusEncOutputFormat format;
		aacPlusEncGetOutputFormat(m_handle, &format);
		if (format.sbrMode == SBR_NORMAL)// || format.sbrMode == SBR_OVERSAMPLED)
			framesPerSample = 2048;
		else
			framesPerSample =1024;
		aacPlusEncStreamTypePtr myStreamTypePnt = aacPlusEncGetCurrentStreamType(m_handle); 
		mp4Writer.AddAudioTrack(&format, (myStreamTypePnt->bsConfig.asConfig.signallingMode==EXPLICIT_BC));
		aacPlusEncGetMPEG4Config(m_handle, audioSpecConfig, 32, &nConfigBits, TYPE_AUDIO_SPECIFIC_CONFIG);
		nConfigBytes = (nConfigBits+7) / 8;
		mp4Writer.WriteASC((void *)audioSpecConfig, nConfigBytes);		
	}
}

int MP4Coder::Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail)
{
	if (m_err) return -1;  // some class error happened, wtf?

	*in_used = 0;
	unsigned int bytes=0,used=0;
	aacPlusEncIfStatusCode ret = APEI_OK;

	if (in_avail || finished == 1) 
	{
		bytes=out_avail;
		if (first)
		{
			aacPlusEncConfigurationPtr conf = aacPlusEncGetCurrentConfiguration(m_handle);
			padding = conf->codecDelay;
			ret = aacPlusEncEncode(m_handle, in, in_avail, &used, out, &bytes, NULL, NULL, 0);
			first=false;
		}
		else if (finished==1)
		{
			ret = aacPlusEncEncode(m_handle, in, in_avail, &used, out, &bytes, NULL, NULL, 0);
		}
		else
		{
			ret = aacPlusEncEncode(m_handle, in, in_avail, &used, out, &bytes, NULL, NULL, 0);
		}

		*in_used = used;
		bytesThisSample += used;

		if (bytes)
		{
			bytesThisSample/=frameSize;
			totalSamples+=bytesThisSample;			
			if (!resampling)
			{
				if (finished && bytesThisSample != framesPerSample) 
				{
					postpadding+=framesPerSample - bytesThisSample;
				}
				mp4Writer.Write(out, bytes, framesPerSample);
			}
			else
			{
				mp4Writer.Write(out, bytes, -1);
			}
			bytesThisSample=0;
		}

	}
	return bytes;
}

void MP4Coder::Finish(const wchar_t *filename)
{
	if (!resampling)
	{
		assert(postpadding>=padding);
		postpadding = max(postpadding, padding);
		mp4Writer.WriteGaps(padding, postpadding-padding, totalSamples);
	}
	mp4Writer.CloseTo(filename);
}
