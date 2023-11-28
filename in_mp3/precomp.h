#ifndef NULLSOFT_IN_MP3_PRECOMP_H
#define NULLSOFT_IN_MP3_PRECOMP_H

/* Windows stuff */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>

/* nu */
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"

/* ID3v2 parser */
#include "../id3v2/id3_tag.h"

/* MP3 Stuff */
#ifndef NO_MP3SURROUND
#include "../mp3/bccDecLinklib/include/bccDecLink.h" // Binaural Cue Coding (aka mp3 surround)
#endif
#include "../mp3/mp3dec/mp3ssc.h"
#include "../mp3/mp3dec/mpgadecoder.h"

#include "giofile.h"

/* Winamp stuff */
#include "../winamp/in2.h"
#include "../winamp/wa_ipc.h"

#include "api.h"
#include "config.h"
#include "adts.h"


#endif