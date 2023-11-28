/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2002-2007 Intel Corporation. All Rights Reserved.
//
*/

#include "ac3_dec.h"
#include "ac3_dec_own_fp.h"

/********************************************************************/

typedef struct rematrix_band_s {
  Ipp32u  start;
  Ipp32u  end;
} rematrix_band_t;

static const rematrix_band_t rematrix_band[] = {
  {13, 24}, {25, 36}, {37, 60}, {61, 252}
};

/********************************************************************/

void Rematrix(AC3Dec *state)
{
  int32_t num_bands;
  int32_t start;
  int32_t end;
  int32_t minEndmant;
  int32_t i;
  _AudBlk *audblk = &(state->audblk);

  if (!audblk->cplinu)
    num_bands = 4; /* coupling is not in use, num_bands = 4 */
  else {           /* coupling is in use */
    if (audblk->cplbegf > 2)
      num_bands = 4;
    else if (audblk->cplbegf > 0)
      num_bands = 3;
    else
      num_bands = 2;
  }

  minEndmant = IPP_MIN((int32_t)audblk->endmant[0],
                   (int32_t)audblk->endmant[1]);
  for (i = 0; i < num_bands; i++) {
    int32_t j;

    if (!((audblk->rematflg >> (num_bands - i - 1)) & 1))
      continue;

    start = rematrix_band[i].start;
    end =
      (audblk->
       cplinu ? IPP_MIN((int32_t)rematrix_band[i].end,
                    12 * audblk->cplbegf + 36) : rematrix_band[i].end);

    if (end > minEndmant - 1) end = minEndmant - 1;

    for (j = start; j <= end; j++) {
      float a = state->coeffs[0][j];
      float b = state->coeffs[1][j];

      state->coeffs[0][j] = (a + b);
      state->coeffs[1][j] = (a - b);
    }
  }
}

/********************************************************************/

