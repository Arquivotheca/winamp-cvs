#include "Encoders.h"
#include "AncillaryData.h"

void Populate(aacPlusEncOutputFormat *format, AACplusConfig *cfg)
{
	memset(format, 0, sizeof(aacPlusEncOutputFormat)); // clear memory helps, in case this code is compiled against a newer library
	format->sbrMode = UNDEFINED_SBR_MODE;
	format->channelMode = cfg->channelMode;
	format->bitRate     = cfg->bitRate; 
	//format->sampleRate  = cfg->sampleRate; 
	format->signalType  = cfg->speech?SPEECH:DEFAULT_SIGNAL_TYPE;
}

AudioCoderCommon::AudioCoderCommon(int nch, int srate, int bps, bitstreamFormat _bitstream, sbrSignallingMode _signallingMode)
: m_err(0), finished(0), m_handle(0), bitstream(_bitstream), first(true),
padding(0), paddingSize(0), frameSize(0), signallingMode(_signallingMode)
{
	frameSize = (bps/8)*nch;
	if (nch<=6) // benski> I don't think our license doesn't allows for more than 5.1 encoding
	{
		switch(bps)
		{
		case 24:
			m_handle=aacPlusEncOpen(srate,nch,aacPlusEncInput24Packed,1,1);
			break;
		case 16:
			m_handle=aacPlusEncOpen(srate,nch,aacPlusEncInputShort,1,1);
			break;
		}
	}
	if (!m_handle)
		m_err=1;
}

AudioCoderCommon::~AudioCoderCommon()
{
	aacPlusEncClose(m_handle);	  

}

void AudioCoderCommon::PrepareToFinish()
{
	aacPlusEncGetOptimumBufferFeed(m_handle, &paddingSize);
	if (paddingSize)
	{
		padding = (unsigned char *)malloc(paddingSize);
		memset(padding, 0, paddingSize);
	}

	finished = 1;
	return;
}

int AudioCoderCommon::Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail)
{
	if (m_err) return -1;  // some class error happened, wtf?

	*in_used = 0;
	unsigned int bytes=0,used=0;
	aacPlusEncIfStatusCode ret = APEI_OK;

	if (finished==1)
	{
		AncillaryData ancData =  {POST_PADDING_MAGIC_WORD, paddingSize/frameSize};
		unsigned int ancSize = sizeof(ancData);
		bytes=out_avail;
		ret = aacPlusEncEncode(m_handle, padding, paddingSize, &used, out, &bytes, (unsigned char *)&ancData, &ancSize, 0);
		finished=2;
		return bytes;
	}
	if (in_avail || finished == 2) 
	{
		bytes=out_avail;
		if (first)
		{
			first=false;
			aacPlusEncConfigurationPtr conf = aacPlusEncGetCurrentConfiguration(m_handle);
			int codecDelay = conf->codecDelay;
			if (codecDelay)
			{
				AncillaryData ancData =  {PRE_PADDING_MAGIC_WORD, codecDelay};
				unsigned int ancSize = sizeof(ancData);
				ret = aacPlusEncEncode(m_handle, in, in_avail, &used, out, &bytes, (unsigned char *)&ancData, &ancSize, 0);
			}
			else
				ret = aacPlusEncEncode(m_handle, in, in_avail, &used, out, &bytes, NULL, NULL, 0);
		}
		else
		{
			ret = aacPlusEncEncode(m_handle, in, in_avail, &used, out, &bytes, NULL, NULL, 0);
		}

		*in_used = used;
	}
	return bytes;
}

void AudioCoderCommon::ConfigureBitstream()
{
	aacPlusEncStreamTypePtr myStreamTypePnt = aacPlusEncGetCurrentStreamType(m_handle); 
	if (myStreamTypePnt==NULL)
	{ 
		m_err=1;
		return;
	}

	myStreamTypePnt->bsFormat = bitstream; 

	switch(myStreamTypePnt->bsFormat)
	{
	case BSFORMAT_ADTS:
		myStreamTypePnt->bsConfig.asConfig.signallingMode = IMPLICIT;
		break;
	case BSFORMAT_ADIF: // MPEG-2 ADIF
		break;
	case BSFORMAT_LATM: // MPEG-4 LATM (Low-overhead Audio Transport Multiplex)
		break;
	case BSFORMAT_LOAS: // MPEG-4 LOAS (Low-Overhead Audio transport multiplex with Synchronization layer)
		break;
	case BSFORMAT_RAW: // MP4 container
		myStreamTypePnt->bsConfig.asConfig.signallingMode = signallingMode;// EXPLICIT_NON_BC;
		break;
	case BSFORMAT_ADTS_MP4: // MPEG-4 ADTS
		break;
	}

	aacPlusEncSetStreamType(m_handle, myStreamTypePnt);
}

void AudioCoderCommon::ConfigureEncoder(bool allowPNS)
{
	aacPlusEncConfigurationPtr aac_conf=aacPlusEncGetCurrentConfiguration (m_handle);
	if(!aac_conf)
	{
		m_err=1;
		return;
	}
	aac_conf->allowPns=allowPNS?1:0;
	aac_conf->qualityMode=aacPlusEncQualityHighest;
	aacPlusEncSetConfiguration(m_handle,aac_conf);
}