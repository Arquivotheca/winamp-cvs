#include "Encoders.h"

AudioCoderAACPlusHighBitrate::AudioCoderAACPlusHighBitrate(int nch, int srate, int bps, AACplusConfig *conf)
: AudioCoderCommon(nch, srate, bps, conf->format, IMPLICIT)
{
	if (m_err) // if parent class reported an error, bail out
		return;

	aacPlusEncOutputFormat myFormat;
	Populate(&myFormat, conf); // fill in common format settings

	// fill in settings specific to high bitrate aac+
	myFormat.sbrMode = SBR_OVERSAMPLED;

	if(aacPlusEncSetOutputFormat(m_handle, &myFormat))
	{
		m_err=1;
		return;
	}

	ConfigureBitstream();
	ConfigureEncoder();
	
}

MP4CoderAACPlusHighBitrate::MP4CoderAACPlusHighBitrate(int nch, int srate, int bps, AACplusConfig *conf)
: MP4Coder(nch, srate, bps, conf->format, conf->signallingMode)
{
	if (m_err) // if parent class reported an error, bail out
		return;

	aacPlusEncOutputFormat myFormat;
	Populate(&myFormat, conf); // fill in common format settings

	// fill in settings specific to high bitrate aac+
	myFormat.sbrMode = SBR_OVERSAMPLED;

	if(aacPlusEncSetOutputFormat(m_handle, &myFormat))
	{
		m_err=1;
		return;
	}

	ConfigureBitstream();
	ConfigureEncoder();
	Start();
			if (myFormat.sampleRate != srate)
			resampling=true;
}
