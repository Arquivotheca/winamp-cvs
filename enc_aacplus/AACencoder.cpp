#include "Encoders.h"

AudioCoderAAC::AudioCoderAAC(int nch, int srate, int bps, AACplusConfig *conf)
: AudioCoderCommon(nch, srate, bps, conf->format, IMPLICIT)
{
	if (m_err) // if parent class reported an error, bail out
		return;

	aacPlusEncOutputFormat myFormat;
	Populate(&myFormat, conf); // fill in common format settings
	
	// fill in settings specific to LC-AAC
	myFormat.sbrMode = SBR_OFF;

	if(aacPlusEncSetOutputFormat (m_handle, &myFormat))
	{
		m_err=1;
		return;
	}

	ConfigureBitstream();
	ConfigureEncoder();

	
}


MP4CoderAAC::MP4CoderAAC(int nch, int srate, int bps, AACplusConfig *conf)
: MP4Coder(nch, srate, bps, conf->format, conf->signallingMode)
{
	if (m_err) // if parent class reported an error, bail out
		return;

	aacPlusEncOutputFormat myFormat;
	Populate(&myFormat, conf); // fill in common format settings
	
	// fill in settings specific to LC-AAC
	myFormat.sbrMode = SBR_OFF;

	if(aacPlusEncSetOutputFormat (m_handle, &myFormat))
	{
		m_err=1;
		return;
	}

	ConfigureBitstream();
	ConfigureEncoder(conf->pns);
	Start();

	if (myFormat.sampleRate != srate)
		resampling=true;
}