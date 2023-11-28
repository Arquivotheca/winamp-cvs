#pragma once
#include "foundation/types.h"
#include "nx/nxapi.h"
#include <android/log.h>

/* Android implementation */

#ifdef __cplusplus
extern "C" {
#endif

	enum
	{
		NXLOG_INFO = ANDROID_LOG_INFO,
	};
NX_API void NXLog(int priority, char *fmt, ...);

#ifdef __cplusplus
}
#endif
