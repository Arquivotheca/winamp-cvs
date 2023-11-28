#pragma once
#include "service/api_service.h"
#ifdef __cplusplus
extern "C" {
#endif

	int Replicant_Playback_Initialize(api_service *service_manager);
	void Replicant_Playback_Shutdown();
#ifdef __cplusplus
}
#endif