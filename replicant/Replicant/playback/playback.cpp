#include "api.h"
#include "playback.h"
#include "nswasabi/singleton.h"

api_service *WASABI2_API_SVC=0;

int Replicant_Playback_Initialize(api_service *service_manager)
{
	WASABI2_API_SVC = service_manager;
	
	return NErr_Success;
}

void Replicant_Playback_Shutdown()
{
	
}