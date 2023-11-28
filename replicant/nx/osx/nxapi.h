#pragma once
#include "foundation/guid.h"
#define NX_API __attribute__ ((visibility("default")))

/* increment this any time that the NX API changes in a non-backwards-compatible way (preferably rarely) */
static const int nx_api_version = 1;

// {60929694-CF15-44F7-9157-B4503E43DFFB}
static const GUID nx_platform_guid = 
{ 0x60929694, 0xcf15, 0x44f7, { 0x91, 0x57, 0xb4, 0x50, 0x3e, 0x43, 0xdf, 0xfb } };
