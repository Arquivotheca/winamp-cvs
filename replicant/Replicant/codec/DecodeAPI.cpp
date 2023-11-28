#include "api.h"
#include "DecodeAPI.h"
#include "decode/svc_decode.h"
#include "foundation/error.h"
#include "adapters/audio-decoder/callback_to_callback.h"

static ns_error_t FindDecoderTryAgain_Callback(ifc_audio_decoder_callback **out_decoder, size_t i, size_t n, nx_uri_t filename, nsaudio::Parameters *parameters, int flags, svc_decode *fallback)
{
	GUID decode_guid = svc_decode::GetServiceType();
	for (;i<n; i++)
	{
		ifc_serviceFactory *sf = WASABI2_API_SVC->EnumService(decode_guid, i);
		if (sf)
		{	
			svc_decode * l = (svc_decode*)sf->GetInterface();
			if (l)
			{
				ns_error_t ret = l->CreateAudioDecoder_Callback(0, out_decoder, filename, parameters, flags);
				if (ret == NErr_Success)
				{
					l->Release();
					return NErr_Success;
				}
				else if (ret == NErr_TryAgain)
				{
					ret = FindDecoderTryAgain_Callback(out_decoder, i+1, n, filename, parameters, flags, l);
					l->Release();
					return ret;
				}
				l->Release();
			}
		}
	}

	if (fallback)
	{
		return fallback->CreateAudioDecoder_Callback(1, out_decoder, filename, parameters, flags);
	}

	return NErr_NoMatchingImplementation;
}

/* retrieve a decoder, but prefer a Callback decoder */
static int GetDecoder_Callback(nx_uri_t filename, nsaudio::Parameters *parameters, int flags, ifc_audio_decoder_callback **out_callback_decoder)
{
	GUID playback_guid = svc_decode::GetServiceType();
	size_t n = WASABI2_API_SVC->GetServiceCount(playback_guid);
	return FindDecoderTryAgain_Callback(out_callback_decoder, 0, n, filename, parameters, flags, 0);
}

int DecodeAPI::DecodeAPI_CreateAudioDecoder_Callback(ifc_audio_decoder_callback **decoder, nx_uri_t filename, nsaudio::Parameters *parameters, int flags)
{
	int ret;
	ifc_audio_decoder_callback *source_decoder=0;
	nsaudio::Parameters decoder_parameters = {0, }; /* this will house the parameters that the decoder will output */

	/* TODO: mask flags so things like 'max samplerate' don't show up */
	ret = GetDecoder_Callback(filename, &decoder_parameters, flags, &source_decoder);
	if (ret == NErr_Success)
	{
		ret = AudioDecoderAdapter_CallbackToCallback::Create(decoder, parameters, flags, source_decoder, &decoder_parameters);
		source_decoder->Release();
		return ret;
	}
	else if (ret == NErr_UnsupportedInterface)
	{
		/* TODO: try a packet decoder (preferred) or a pull decoder (more complicated) and use an adapter */
		return ret;
	}
	else
	{
		return ret;
	}

}