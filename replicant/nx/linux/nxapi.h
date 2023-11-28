#pragma once

#ifdef __ANDROID__
#error you should be using android/nxapi.h instead!
#endif

#include "foundation/guid.h"
/* Linux implementation */

#define NX_API __attribute__ ((visibility("default")))

/* increment this any time that the NX API changes in a non-backwards-compatible way (preferably rarely) */
static const int nx_api_version = 1;

// {FBC843D7-EA24-4822-A027-2DD8634F9A89}
static const GUID nx_platform_guid = 
{ 0xfbc843d7, 0xea24, 0x4822, { 0xa0, 0x27, 0x2d, 0xd8, 0x63, 0x4f, 0x9a, 0x89 } };

