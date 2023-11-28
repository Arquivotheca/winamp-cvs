/***************************************************************************\
 *                         Fraunhofer IIS 
 *                 (c) 1997 - 2008 Fraunhofer IIS
 *          (c) 2004 Fraunhofer IIS and Agere Systems Inc.
 *                       All Rights Reserved.
 *
 *
 *    This software and/or program is protected by copyright law and
 *    international treaties. Any reproduction or distribution of this
 *    software and/or program, or any portion of it, may result in severe
 *    civil and criminal penalties, and will be prosecuted to the maximum
 *    extent possible under law.
 *
\***************************************************************************/

#ifndef __BCC_NLC_DEC_H__
#define __BCC_NLC_DEC_H__

#include "getstream.h"
#include "bcc_dec_api.h"


typedef enum
{
  NLC_DIFF_TYPE_ERROR = -1,
  DIFF_FREQ = 0x0,
  DIFF_TIME = 0x1

} NLC_DIFF_TYPE;


typedef enum
{
  CODING_SCHEME_ERROR = -1,
  HUFF_2D_FREQ        = 0x0,
  HUFF_2D_TIME        = 0x1,
  PCM                 = 0x2

} CODING_SCHEME;


typedef enum {

  DIRECTION_ERROR = -1,
  BACKWARDS = 0x0,
  FORWARDS  = 0x1

} DIRECTION;


BCC_STAT bcc_cues_nldec( Streamin_state* strm,
                     BCC_dparams*    params,
                     BCC_dstate*     state );

BCC_STAT bcc_info_dec( Streamin_state* strm,
                       BCC_dparams*    params,
                       int             num_info_bits );

#endif
