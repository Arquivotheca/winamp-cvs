#pragma once
#include "foundation/guid.h"
/* Android implementation */

#define NX_API __attribute__ ((visibility("default")))

/* increment this any time that the NX API changes in a non-backwards-compatible way (preferably rarely) */
static const int nx_api_version = 2;

// {B333D71B-738B-43CE-B10E-6A32309948A9}
static const GUID nx_platform_guid = 
{ 0xb333d71b, 0x738b, 0x43ce, { 0xb1, 0xe, 0x6a, 0x32, 0x30, 0x99, 0x48, 0xa9 } };
