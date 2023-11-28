#pragma once
#include "audio/ifc_equalizer.h"
#include "nseq/eq_4front.h"

class Equalizer : public ifc_equalizer
{
public:
	Equalizer();
	~Equalizer();
	int Initialize(unsigned int channels, double sample_rate);

	/* Equalizer implementation */
	int WASABICALL Equalizer_SetPreamp(double dB);
	int WASABICALL Equalizer_SetBand(unsigned int band, double dB);
	int WASABICALL Equalizer_Enable();
	int WASABICALL Equalizer_Disable();

	void ProcessSamples(const float *input, float *output, size_t samples);
	void ProcessFrames(const float *input, float *output, size_t sample_frames);
	bool IsEnabled();
	float GetGain();
private:
	eq10_t *eq;
	unsigned int channels;
	volatile int enabled, allzero;
	volatile float preamp_gain;
	void Optimize(double last_value); /* checks if all the bands are zero. pass in the last value you changed (as a performance optimization) */
};
