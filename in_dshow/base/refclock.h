//------------------------------------------------------------------------------
// File: RefClock.h
//
// Desc: DirectShow base classes - defines the IReferenceClock interface.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#ifndef __BASEREFCLOCK__
#define __BASEREFCLOCK__

#include "dsschedule.h"

const UINT RESOLUTION = 1;                      /* High resolution timer */
const INT ADVISE_CACHE = 4;                     /* Default cache size */
const LONGLONG MAX_TIME = 0x7FFFFFFFFFFFFFFF;   /* Maximum LONGLONG value */

#endif

