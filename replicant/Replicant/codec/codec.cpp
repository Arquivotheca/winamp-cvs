#include "api.h"
#include "codec.h"
#include "nswasabi/singleton.h"
#include "DecodeAPI.h"

api_service *WASABI2_API_SVC=0;

static SingletonService<DecodeAPI, api_decode> decode_api_factory;

int Replicant_Codec_Initialize(api_service *service_manager)
{
	WASABI2_API_SVC = service_manager;
	decode_api_factory.Register(WASABI2_API_SVC);
	return NErr_Success;
}

void Replicant_Codec_Shutdown()
{
	decode_api_factory.Deregister(WASABI2_API_SVC);
}