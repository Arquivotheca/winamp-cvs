#pragma once
#include "audio/ifc_audioout.h"
#include "nu/LockFreeRingBuffer.h"
#include "nu/PtrDeque.h"
#include "player/ifc_playback_parameters.h"
#include "ifc_audiotrackpro_settings.h"
#include "pcmutils.h"

class AudioThread;
class AudioSession : public ifc_audioout, public nu::PtrDequeNode
{
public:
	AudioSession();
	~AudioSession();

	int Initialize(AudioThread *parent, const ifc_audioout::Parameters *audio_parameters, ifc_playback_parameters *secondary_parameters);

	/* returns bytes written */
	int ReadFloat(float *buffer, size_t bytes_requested, size_t *bytes_written);
	int AccumulateFloat(float *buffer, size_t bytes_requested, size_t *bytes_written);
	double SecondsBuffered();
	void Flush(size_t position);
	void SetError(int error);
	void SetLatency(uint32_t);
	void OnDone();
	void OnStart(bool transition, size_t crossfade_samples); 
	void OnTransition(bool transition, size_t *crossfade_samples);
	bool Done() const;
	const ifc_audioout::Parameters *GetParameters() const;

	/* ifc_audioout implementation */
	int WASABICALL AudioOutput_Output(const void *data, size_t data_size);
	size_t WASABICALL AudioOutput_CanWrite(); // returns number of bytes that you can write
	void WASABICALL AudioOutput_Flush(double seconds);
	void WASABICALL AudioOutput_Pause(int state);
	void WASABICALL AudioOutput_Done();
	void WASABICALL AudioOutput_Stop();
	double WASABICALL AudioOutput_Latency();
	int WASABICALL AudioOutput_Playing();

private:
	enum CrossfadeState
	{
		IDLE,
		FADE_IN,
		PLAYING,
		FADE_OUT,
	};
	ifc_playback_parameters *playback_parameters;
	ifc_audiotrackpro_settings *settings;

	size_t buffer_size_in_samples; /* number of samples that the ring buffer can hold */
	Parameters audio_parameters;
	volatile int error;
	volatile int done_flag;
	LockFreeRingBuffer ring_buffer;
	CrossfadeState crossfade_state;
	double bytes_per_second;
	double volume_adjust;
	double volume_delta;
	size_t samples_per_second;

	AudioThread *parent;
	uint32_t latency;

	size_t pre_gap_bytes;
	size_t current_pre_gap_bytes;
	size_t post_gap_bytes;
	ConverterFunc convert;
	DeinterleaverFunc deinterleave;
	/* Crossfade state */
	bool skip_crossfade;
	size_t fade_samples;
	

	/* returns how many bytes to actually read from the buffer, after adjusting for post-gap stuff */
	int Internal_AdjustRead(size_t bytes_requested, size_t *bytes_to_read) const;

	int Internal_Fill(float *buffer, size_t bytes_requested, size_t *bytes_written);
	int Internal_Accumulate(float *buffer, size_t bytes_requested, size_t *bytes_written);
	int Internal_FadeIn(float *buffer, size_t bytes_requested, size_t *bytes_written);
	int Internal_FadeInAccumulate(float *buffer, size_t bytes_requested, size_t *bytes_written);
	int Internal_FadeOut(float *buffer, size_t bytes_requested, size_t *bytes_written);
	int Internal_FadeOutAccumulate(float *buffer, size_t bytes_requested, size_t *bytes_written);

};