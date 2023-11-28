/*
  Memory layout
*/
#include <stdio.h>
#include "aacdecoder.h"
#include "aac_ram.h"
#include "math/FloatFR.h"

/* Overlap buffer */
float OverlapBuffer[Channels*OverlapBufferSize];

