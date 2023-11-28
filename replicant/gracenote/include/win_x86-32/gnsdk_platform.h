/*
 * Copyright (c) 2012 Gracenote.
 *
 * This software may not be used in any way or distributed without
 * permission. All rights reserved.
 *
 * Some code herein may be covered by US and international patents.
 */

/*
 * gnsdk_platform.h - Contains platform-specific definitions.
 */

/*
 * USE THIS FILE TO OVERRIDE THE DEFINITIONS IN gnsdk_defines.h
 */

/*
 * Windows Flavor
 */

#ifndef _GNSDK_PLATFORM_H_
#define _GNSDK_PLATFORM_H_


#define GNSDK_WINDOWS

#define GNSDK_EXPORTED_API	__declspec(dllexport)
#define GNSDK_IMPORTED_API	__declspec(dllimport)
#define	GNSDK_CALLBACK_API	__cdecl
#define GNSDK_API			__stdcall

#include <assert.h>
#define GNSDK_ASSERT(exp)	assert(exp)

#define GNSDK_INT64_T
typedef __int64 gnsdk_int64_t;

#define GNSDK_UINT64_T
typedef unsigned __int64 gnsdk_uint64_t;

#include <stddef.h>


#endif /* _GNSDK_PLATFORM_H_ */

