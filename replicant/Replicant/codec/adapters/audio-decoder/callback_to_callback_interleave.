#pragma once
#include "audio/ifc_audio_decoder_callback.h"
#include "pcmutils.h"
#include "audio/parameters.h"

class AudioDecoderAdapter_CallbackToCallback_Interleave : public ifc_audio_decoder_callback, public ifc_audio_decoder_callback::callback
{
public:
	AudioDecoderAdapter_CallbackToCallback_Interleave();
	
	int Initialize(const nsaudio::Parameters *parameters, ifc_audio_decoder_callback *source_decoder, Interleaver interleaver);
protected:
	~AudioDecoderAdapter_CallbackToCallback_Interleave();

	nsaudio::Parameters parameters;
	Interleaver interleaver;
	ifc_audio_decoder_callback *source_decoder;
	double gain;
	void *interleave_buffer;
	size_t frame_size;
	ifc_audio_decoder_callback::callback *audio_callback;

	int WASABICALL AudioDecoderCallback_Decode(ifc_audio_decoder_callback::callback *callback);
	int WASABICALL AudioDecoderCallback_DecodeStep(ifc_audio_decoder_callback::callback *callback);
	int WASABICALL AudioDecoderCallback_GetFrameSize(size_t *frame_size);
	int WASABICALL AudioDecoderCallback_GetMetadata(ifc_metadata **metadata);

	int WASABICALL AudioDecoderCallback_OnAudio(const void *buffer, size_t buffer_frames);
};