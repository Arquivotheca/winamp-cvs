/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2002-2006 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __AC3_DEC_TABLES_H__
#define __AC3_DEC_TABLES_H__

#include <bfc/platform/types.h>

typedef struct frmsize_s {
  uint16_t bit_rate;
  uint16_t frm_size[3];
} frmsize_t;

extern const frmsize_t FRAMESIZECODE[];
extern const int32_t SAMPLING_RATE[];
extern const uint16_t NFCHANS[];
extern int16_t UNGRP5[];

extern int16_t SLOWDEC[];
extern int16_t FASTDEC[];
extern int16_t SLOWGAIN[];
extern int16_t DBPBTAB[];

extern int16_t FLOORTAB[];
extern int16_t FASTGAIN[];
extern int16_t BNDTAB[];
extern int16_t BNDSZ[];
extern int16_t MASKTAB[];
extern int16_t LATAB[];
extern int16_t HTH[][50];
extern uint8_t BAPTAB[];

extern float phscorFac[8][8][4][5];
extern float phsCorTab[];
extern float WindowTable[];

#endif /* __AC3DEC_TABLES_H__ */

