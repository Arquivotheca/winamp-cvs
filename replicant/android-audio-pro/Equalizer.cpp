#include "Equalizer.h"
#include "foundation/error.h"
#include <math.h>
#include <android/log.h>

static double eq10_freq_iso[EQ10_NOFBANDS]={31,62,125,250,500,1000,2000,4000,8000,16000}; // ISO frequency table

Equalizer::Equalizer()
{
	enabled=0;
	allzero=1;
	eq=0;
	channels=0;
	preamp_gain=1.0f;
}

Equalizer::~Equalizer()
{
	free(eq);
}

int Equalizer::Initialize(unsigned int channels, double sample_rate) 
{
	this->channels = channels;
	eq = (eq10_t *)calloc(channels, sizeof(eq10_t));
	if (!eq)
		return NErr_OutOfMemory;

	for (unsigned int channel = 0; channel < channels; channel++)
	{
		eq10_setup(&eq[channel], sample_rate, eq10_freq_iso); // initialize
		for (unsigned int band=0;band<10;band++)
		{

			eq10_setgain(&eq[channel], band, 0);

		}
	}

	return NErr_Success;
}

void Equalizer::Optimize(double last_value)
{
	if (last_value || preamp_gain != 1.0f)
	{
		allzero=0;
	}
	else
	{
		bool disable_filter=true;
		for (int i=0;i<10 && disable_filter;i++)
		{
			if (eq10_getgain(eq, i))
				disable_filter=false;
		}
		if (disable_filter)
			allzero=1;
		else
			allzero=0;
	}
}

int Equalizer::Equalizer_SetBand(unsigned int band, double dB)
{
	for (unsigned int channel = 0; channel < channels; channel++)
	{
		eq10_setgain(&eq[channel], band, dB);
	}
	Optimize(dB);
	return NErr_Success;
}

int Equalizer::Equalizer_SetPreamp(double dB)
{
	preamp_gain = pow(10.0,dB/20.0);
	Optimize(dB);
	return NErr_Success;
}

int Equalizer::Equalizer_Enable()
{
	enabled=1;
	return NErr_Success;
}

int Equalizer::Equalizer_Disable()
{
	enabled=0;
	return NErr_Success;
}

void Equalizer::ProcessSamples(const float *input, float *output, size_t samples)
{
	size_t sample_frames = samples/channels;
	for (unsigned int channel = 0; channel < channels; channel++)
	{
		eq10_processf(&eq[channel], input+channel, output+channel, sample_frames, channels, 1);
	}
}

void Equalizer::ProcessFrames(const float *input, float *output, size_t sample_frames)
{
	for (unsigned int channel = 0; channel < channels; channel++)
	{
		eq10_processf(&eq[channel], input+channel, output+channel, sample_frames, channels, 1);
	}
}

bool Equalizer::IsEnabled()
{
	if (enabled && !allzero)
		return true;
	else
		return false;
}

float Equalizer::GetGain()
{
	if (enabled)
		return preamp_gain;
	else
		return 1.0f;
}