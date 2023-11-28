#include "callback_to_callback_interleave_decimate.h"
#include <stdlib.h>

AudioDecoderAdapter_CallbackToCallback_InterleaveShift::AudioDecoderAdapter_CallbackToCallback_InterleaveShift()
{
	memset(&parameters, 0, sizeof(parameters));
	interleaver=0;
	source_decoder=0;
	shift_bits=0;
	interleave_buffer=0;
	frame_size=0;
	audio_callback=0;
}

AudioDecoderAdapter_CallbackToCallback_InterleaveShift::~AudioDecoderAdapter_CallbackToCallback_InterleaveShift()
{
	if (source_decoder)
		source_decoder->Release();
	free(interleave_buffer);
}

int AudioDecoderAdapter_CallbackToCallback_InterleaveShift::Initialize(const nsaudio::Parameters *parameters, ifc_audio_decoder_callback *source_decoder, InterleaverShift interleaver, unsigned int shift_bits)
{
	memcpy(&this->parameters, parameters, sizeof(nsaudio::Parameters));
	this->source_decoder = source_decoder;
	
	this->interleaver = interleaver;
	this->shift_bits = shift_bits;

	/* build buffer for interleaving */
	if (source_decoder->GetFrameSize(&frame_size) == NErr_Success && frame_size)
	{
		size_t new_size = parameters->number_of_channels * parameters->bytes_per_sample * frame_size;
		if (new_size < frame_size)
			return NErr_IntegerOverflow;
		interleave_buffer = malloc(new_size);
		if (!interleave_buffer)
			return NErr_OutOfMemory;
	}
	
	source_decoder->Retain();
	return NErr_Success;
}

int AudioDecoderAdapter_CallbackToCallback_InterleaveShift::AudioDecoderCallback_Decode(ifc_audio_decoder_callback::callback *callback)
{
	audio_callback=callback;
	return source_decoder->Decode(this);
}

int AudioDecoderAdapter_CallbackToCallback_InterleaveShift::AudioDecoderCallback_DecodeStep(ifc_audio_decoder_callback::callback *callback)
{
	audio_callback=callback;
	return source_decoder->DecodeStep(this);	
}

int AudioDecoderAdapter_CallbackToCallback_InterleaveShift::AudioDecoderCallback_GetFrameSize(size_t *frame_size)
{
	if (this->frame_size)
	{
		*frame_size = this->frame_size;
		return NErr_Success;
	}
	return NErr_NotImplemented;
}

int AudioDecoderAdapter_CallbackToCallback_InterleaveShift::AudioDecoderCallback_GetMetadata(ifc_metadata **metadata)
{
	return source_decoder->GetMetadata(metadata);
}

int AudioDecoderAdapter_CallbackToCallback_InterleaveShift::AudioDecoderCallback_OnAudio(const void *buffer, size_t buffer_frames)
{
	/* resize buffer if necessary */
	if (buffer_frames > frame_size)
	{
		size_t new_size = parameters.number_of_channels * parameters.bytes_per_sample * buffer_frames;
		if (new_size < buffer_frames)
			return NErr_IntegerOverflow;
		void *new_buffer = realloc(interleave_buffer, new_size);
		if (!new_buffer)
			return NErr_OutOfMemory;

		frame_size = buffer_frames;
		interleave_buffer = new_buffer;
	}

	interleaver(interleave_buffer, (const void **)buffer, parameters.number_of_channels, buffer_frames, shift_bits);
	return audio_callback->OnAudio(interleave_buffer, buffer_frames);
}