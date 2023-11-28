/*
 * Copyright (c) 2011 Gracenote.
 *
 * This software may not be used in any way or distributed without
 * permission. All rights reserved.
 *
 * Some code herein may be covered by US and international patents.
 */

/*
 * gnsdk.h - header to include all requisite GNSDK headers
 */

#ifndef	_GNSDK_H_
#define _GNSDK_H_

/* GNSDK Types and Macros */
#include "gnsdk_defines.h"          /* REQUIRED */
#include "gnsdk_errors.h"           /* REQUIRED */
#include "gnsdk_error_codes.h"      /* REQUIRED */
#include "gnsdk_version.h"			/* STANDARD */

/* GNSDK APIs */
#include "gnsdk_manager.h"       /* REQUIRED */
#include "gnsdk_manager_gdo.h"   /* STANDARD */
#include "gnsdk_manager_lists.h" /* STANDARD */


/* GNSDKs to be used: Enable defines for all SDKs to be used */
#if ((!defined GNSDK_MUSICID)  && (!defined GNSDK_MUSICID_FILE) && \
	 (!defined GNSDK_VIDEO)	   && (!defined GNSDK_LINK) && \
	 (!defined GNSDK_SQLITE)   && (!defined GNSDK_DSP) && \
	 (!defined GNSDK_SUBMIT)   && (!defined GNSDK_PLAYLIST))

	#define GNSDK_MUSICID		1
	#define GNSDK_MUSICID_FILE	1
	#define GNSDK_MUSICID_MATCH			1
	#define GNSDK_VIDEO			1
	#define GNSDK_LINK			1
	#define GNSDK_SUBMIT		1
	#define GNSDK_SQLITE		1
	#define GNSDK_DSP			1
	#define GNSDK_PLAYLIST		1
#endif


#if GNSDK_MUSICID
	#include "gnsdk_musicid.h"
#endif
#if GNSDK_MUSICID_FILE
	#include "gnsdk_musicid_file.h"
#endif
#if GNSDK_MUSICID_MATCH
	#include "gnsdk_musicid_match.h"
#endif
#if GNSDK_VIDEO
	#include "gnsdk_video.h"
#endif
#if GNSDK_LINK
	#include "gnsdk_link.h"
#endif
#if GNSDK_SUBMIT
	#include "gnsdk_submit.h"
	#include "gnsdk_submit_gdo.h"
#endif
#if GNSDK_SQLITE
	#include "gnsdk_sqlite.h"
#endif
#if GNSDK_DSP
	#include "gnsdk_dsp.h"
#endif
#if GNSDK_PLAYLIST
	#include "gnsdk_playlist.h"
#endif


#endif /* _GNSDK_H_ */

