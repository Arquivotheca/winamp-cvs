/*
  Memory layout
*/
#ifndef __SBR_RAM_H
#define __SBR_RAM_H
#include "sbrdecsettings.h" /* for MAXNRSBRCHANNELS */
#include "sbrdec/sbrdecoder.h"
#include "env_extr.h"

extern float   WorkBuffer2[];
extern float   InterimResult[MAX_FRAME_SIZE];

#endif
