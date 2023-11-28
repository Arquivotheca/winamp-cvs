#pragma once
#include "decode/api_decode.h"
#include "nswasabi/ServiceName.h"
class DecodeAPI : public api_decode
{
public:
	WASABI_SERVICE_NAME("Decode API");
private:
	
	int WASABICALL DecodeAPI_CreateAudioDecoder_Callback(ifc_audio_decoder_callback **decoder, nx_uri_t filename, nsaudio::Parameters *parameters, int flags);
};