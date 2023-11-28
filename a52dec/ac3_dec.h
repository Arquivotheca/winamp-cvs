/*
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2002-2008 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __AC3_DEC_H__
#define __AC3_DEC_H__

#include <bfc/platform/types.h>
//#include "ipps.h"
#include "ac3_dec_status.h"

#ifdef __cplusplus
extern "C" {
#endif

  typedef enum
  {
    AC3_CHANNEL_FRONT_LEFT      = 0x1,
    AC3_CHANNEL_FRONT_RIGHT     = 0x2,
    AC3_CHANNEL_FRONT_CENTER    = 0x4,
    AC3_CHANNEL_LOW_FREQUENCY   = 0x8,
    AC3_CHANNEL_BACK_LEFT       = 0x10,
    AC3_CHANNEL_BACK_RIGHT      = 0x20,
    AC3_CHANNEL_BACK_CENTER     = 0x100,
  } AC3ChannelMask;

  struct _AC3Dec;
  typedef struct _AC3Dec AC3Dec;

  AC3Status ac3decReset(AC3Dec *state);
  void ac3decUpdateMemMap(AC3Dec *state,
                          int32_t shift);
  AC3Status ac3decInit(AC3Dec *state_ptr,
                       int32_t *sizeAll);
  AC3Status ac3decClose(/* AC3Dec *state */);
  AC3Status ac3decGetFrame(uint8_t *inPointer,
                           int32_t inDataSize,
                           int32_t *decodedBytes,
                           int16_t *outPointer,
                           int32_t outBufferSize,
                           AC3Dec *state);
  AC3Status ac3decGetDuration(float *p_duration,
                              AC3Dec *state);
  AC3Status ac3decGetInfo(unsigned int *sample_rate, unsigned int *channels, unsigned int *bitrate,
                          AC3Dec *state);

  AC3Status ac3decSetOutAcmod(int32_t out_acmod, AC3Dec *state);
  AC3Status ac3decSetOuLfeOn(int32_t outlfeon, AC3Dec *state);
  AC3Status ac3decSetDualMonoMode(int32_t dualmonomode, AC3Dec *state);
  AC3Status ac3decSetDrcScaleLow(float drc_scaleLow, AC3Dec *state);
  AC3Status ac3decSetDrcScaleHigh(float drc_scaleHigh, AC3Dec *state);
  AC3Status ac3decSetOutCompMod(int32_t out_compmod, AC3Dec *state);
  AC3Status ac3decSetKaraokeCapable(int32_t karaokeCapable, AC3Dec *state);
  AC3Status ac3decSetCrcMute(int32_t crc_mute, AC3Dec *state);
  AC3Status ac3decSetGainScale(float gainScale, AC3Dec *state);

  AC3Status ac3decGetNumChannelOut(int32_t *nChannelOut, AC3Dec *state);
  AC3Status ac3decGetSampleFrequency(int32_t *freq, AC3Dec *state);

#ifdef __cplusplus
}
#endif

#endif
