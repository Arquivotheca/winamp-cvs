#pragma once
#include "nx/nxapi.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

NX_API int NXSleep(unsigned int milliseconds);
NX_API int NXSleepYield(void);

#ifdef __cplusplus
}
#endif
