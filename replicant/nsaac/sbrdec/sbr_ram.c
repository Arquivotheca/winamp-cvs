/*
  Memory layout
  This module declares all static and dynamic memory spaces
*/
#include <stdio.h>
#include "sbrdec/sbrdecoder.h"
#include "sbr_ram.h"
#include "math/FloatFR.h"
extern float WorkBufferCore[];

/*
  \name DynamicSbrData
  Dynamic memory areas, might be reused in other algorithm sections,
  e.g. the core decoder
*/
/* @{ */
/*  The work buffer #WorkBufferCore of the aac-core (see aac_ram.cpp)
  will be reused as #WorkBuffer2 in the SBR part. Minimum size of
  #WorkBufferCore must be #MAX_COLS * #NO_SYNTHESIS_CHANNELS.
  #WorkBuffer2 is the second half of the SBR work buffer. */
float WorkBuffer2[2*1024];

/*  This buffer stores half of the reconstructed left time data signal
  until the right channel is completely finished */
float InterimResult[MAX_FRAME_SIZE];
/* @} */

