#pragma once
#include "../Winamp/gen.h"
#include "../Winamp/wa_ipc.h"
#include <time.h>

#define VERSION_MAJOR 0
#define VERSION_MINOR 2 /* can be two digits or one */

#define CSTR_INVARIANT		MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

#ifndef ARRAYSIZE
#define ARRAYSIZE(blah) (sizeof(blah)/sizeof(*blah))
#endif

/* main.cpp */
extern winampGeneralPurposePlugin plugin;
extern int winampVersion;
extern __time64_t session_expiration;