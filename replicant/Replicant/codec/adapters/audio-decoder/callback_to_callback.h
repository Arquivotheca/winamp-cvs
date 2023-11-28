#pragma once
#include "audio/ifc_audio_decoder_callback.h"
#include "pcmutils.h"
#include "audio/parameters.h"

class AudioDecoderAdapter_CallbackToCallback
{
public:
	static int Create(ifc_audio_decoder_callback **out_decoder, nsaudio::Parameters *parameters, int flags, ifc_audio_decoder_callback *source_decoder, const nsaudio::Parameters *source_parameters);
private:
	static int CreateInterleaver(ifc_audio_decoder_callback **out_decoder, nsaudio::Parameters *parameters, int flags, ifc_audio_decoder_callback *source_decoder, const nsaudio::Parameters *source_parameters);
	static int CreateConverter(ifc_audio_decoder_callback **out_decoder, nsaudio::Parameters *parameters, int flags, ifc_audio_decoder_callback *source_decoder, const nsaudio::Parameters *source_parameters);
	
};