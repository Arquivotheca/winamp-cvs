#include "callback_to_callback_convert.h"
#include <stdlib.h>

AudioDecoderAdapter_CallbackToCallback_Convert::AudioDecoderAdapter_CallbackToCallback_Convert()
{
	memset(&parameters, 0, sizeof(parameters));
	converter=0;
	source_decoder=0;
	gain=0;
	convert_buffer=0;
	frame_size=0;
	audio_callback=0;
}

AudioDecoderAdapter_CallbackToCallback_Convert::~AudioDecoderAdapter_CallbackToCallback_Convert()
{
	if (source_decoder)
		source_decoder->Release();
	free(convert_buffer);
}

int AudioDecoderAdapter_CallbackToCallback_Convert::Initialize(const nsaudio::Parameters *parameters, ifc_audio_decoder_callback *source_decoder, ConverterGain converter, double gain)
{
	memcpy(&this->parameters, parameters, sizeof(nsaudio::Parameters));
	this->source_decoder = source_decoder;
	
	this->converter = converter;
	this->gain = gain;

	/* build buffer for interleaving */
	if (source_decoder->GetFrameSize(&frame_size) == NErr_Success && frame_size)
	{
		size_t new_size = parameters->number_of_channels * parameters->bytes_per_sample * frame_size;
		if (new_size < frame_size)
			return NErr_IntegerOverflow;
		convert_buffer = malloc(new_size);
		if (!convert_buffer)
			return NErr_OutOfMemory;
	}
	
	source_decoder->Retain();
	return NErr_Success;
}

int AudioDecoderAdapter_CallbackToCallback_Convert::AudioDecoderCallback_Decode(ifc_audio_decoder_callback::callback *callback)
{
	audio_callback=callback;
	return source_decoder->Decode(this);
}

int AudioDecoderAdapter_CallbackToCallback_Convert::AudioDecoderCallback_DecodeStep(ifc_audio_decoder_callback::callback *callback)
{
	audio_callback=callback;
	return source_decoder->DecodeStep(this);	
}

int AudioDecoderAdapter_CallbackToCallback_Convert::AudioDecoderCallback_GetFrameSize(size_t *frame_size)
{
	if (this->frame_size)
	{
		*frame_size = this->frame_size;
		return NErr_Success;
	}
	return NErr_NotImplemented;
}

int AudioDecoderAdapter_CallbackToCallback_Convert::AudioDecoderCallback_GetMetadata(ifc_metadata **metadata)
{
	return source_decoder->GetMetadata(metadata);
}

int AudioDecoderAdapter_CallbackToCallback_Convert::AudioDecoderCallback_OnAudio(const void *buffer, size_t buffer_frames)
{
	/* resize buffer if necessary */
	if (buffer_frames > frame_size)
	{
		size_t new_size = parameters.number_of_channels * parameters.bytes_per_sample * buffer_frames;
		if (new_size < buffer_frames)
			return NErr_IntegerOverflow;
		void *new_buffer = realloc(convert_buffer, new_size);
		if (!new_buffer)
			return NErr_OutOfMemory;

		frame_size = buffer_frames;
		convert_buffer = new_buffer;
	}

	converter(convert_buffer, buffer, buffer_frames*parameters.number_of_channels, gain);
	return audio_callback->OnAudio(convert_buffer, buffer_frames);
}