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
#include "ac3_dec_tables.h"

#ifndef REF_DECODER_COMPATIBLE

/********************************************************************/

static void CountCplCoeffCom(float *stream_coeffs_cpl,
                             float *stream_coeffs,
                             _AudBlk *audblk,
                             float dpfac,
                             float epfac,
                             int32_t testBsi,
                             int32_t ch)
{
  int32_t i, phsflg, ind;
  float cpl_coord = 0;
  int32_t bnd = 0;
  int32_t bnd1 = 0;

  for (i = audblk->cplstrtmant; i < audblk->cplendmant; i += 12) {
    if (!audblk->cplbndstrc[bnd]) {
      phsflg = audblk->phsflg[bnd1];

      ind = 15 + (int32_t)(audblk->phscor[bnd1] * dpfac + 0.5) -
                 (int32_t)(audblk->phscor[bnd1] * epfac + 0.5);

      cpl_coord = audblk->cplcoord[ch][bnd1] * phsCorTab[ind];
      if      (cpl_coord >  1) cpl_coord =  1;
      else if (cpl_coord < -1) cpl_coord = -1;

      bnd1++;
    }

    if (testBsi && phsflg) {
      ippsMulC_32f(stream_coeffs_cpl+i, -cpl_coord, stream_coeffs+i, 12);
    } else {
      ippsMulC_32f(stream_coeffs_cpl+i, cpl_coord, stream_coeffs+i, 12);
    }
    bnd++;
  }
}

/********************************************************************/
#else
/********************************************************************/

static void CountCplCoeff00(float *stream_coeffs_cpl,
                            float *stream_coeffs,
                            _AudBlk *audblk,
                            int32_t ch)
{
  int32_t i;
  float cpl_coord = 0;
  int32_t bnd = 0;

  for (i = audblk->cplstrtmant; i < audblk->cplendmant; i += 12) {
    int32_t j;
    float *src = stream_coeffs_cpl+i;
    float *dst = stream_coeffs+i;

    cpl_coord = audblk->cplcoord[ch][bnd];

    for (j = 0; j < 12; j++) {
      dst[j] = src[j] * cpl_coord;
    }

    bnd++;
  }
}

/********************************************************************/

static void CountCplCoeff01(float *stream_coeffs_cpl,
                            float *stream_coeffs,
                            _AudBlk *audblk,
                            int32_t ch)
{
  int32_t i, ind;
  float cpl_coord = 0;
  int32_t bnd = 0;

  for (i = audblk->cplstrtmant; i < audblk->cplendmant; i += 12) {
    int32_t j;
    float *src = stream_coeffs_cpl+i;
    float *dst = stream_coeffs+i;

    ind = 15 - audblk->phscor[bnd];

    cpl_coord = audblk->cplcoord[ch][bnd] * phsCorTab[ind];
    if      (cpl_coord >  8) cpl_coord =  8;
    else if (cpl_coord < -8) cpl_coord = -8;

    for (j = 0; j < 12; j++) {
      dst[j] = src[j] * cpl_coord;
    }
    bnd++;
  }
}

/********************************************************************/

static void CountCplCoeff0Com(float *stream_coeffs_cpl,
                              float *stream_coeffs,
                              _AudBlk *audblk,
                              float epfac,
                              int32_t ch)
{
  int32_t i, ind;
  float cpl_coord = 0;
  int32_t bnd = 0;

  for (i = audblk->cplstrtmant; i < audblk->cplendmant; i += 12) {
    int32_t j;
    float *src = stream_coeffs_cpl+i;
    float *dst = stream_coeffs+i;

    ind = 15 - (int32_t)(audblk->phscor[bnd] * epfac + 0.5);

    cpl_coord = audblk->cplcoord[ch][bnd] * phsCorTab[ind];
    if      (cpl_coord >  8) cpl_coord =  8;
    else if (cpl_coord < -8) cpl_coord = -8;

    for (j = 0; j < 12; j++) {
      dst[j] = src[j] * cpl_coord;
    }
    bnd++;
  }
}


/********************************************************************/

static void CountCplCoeff10(float *stream_coeffs_cpl,
                            float *stream_coeffs,
                            _AudBlk *audblk,
                            int32_t ch)
{
  int32_t i, ind;
  float cpl_coord = 0;
  int32_t bnd = 0;

  for (i = audblk->cplstrtmant; i < audblk->cplendmant; i += 12) {
    int32_t j;
    float *src = stream_coeffs_cpl+i;
    float *dst = stream_coeffs+i;

    ind = 15 + audblk->phscor[bnd];

    cpl_coord = audblk->cplcoord[ch][bnd] * phsCorTab[ind];
    if      (cpl_coord >  8) cpl_coord =  8;
    else if (cpl_coord < -8) cpl_coord = -8;

    for (j = 0; j < 12; j++) {
      dst[j] = src[j] * cpl_coord;
    }
    bnd++;
  }
}


/********************************************************************/

static void CountCplCoeff1Com(float *stream_coeffs_cpl,
                              float *stream_coeffs,
                              _AudBlk *audblk,
                              float epfac,
                              int32_t ch)
{
  int32_t i, ind;
  float cpl_coord = 0;
  int32_t bnd = 0;

  for (i = audblk->cplstrtmant; i < audblk->cplendmant; i += 12) {
    int32_t j;
    float *src = stream_coeffs_cpl+i;
    float *dst = stream_coeffs+i;

    ind = 15 + audblk->phscor[bnd] -
          (int32_t)(audblk->phscor[bnd] * epfac + 0.5);

    cpl_coord = audblk->cplcoord[ch][bnd] * phsCorTab[ind];
    if      (cpl_coord >  8) cpl_coord =  8;
    else if (cpl_coord < -8) cpl_coord = -8;

    for (j = 0; j < 12; j++) {
      dst[j] = src[j] * cpl_coord;
    }
    bnd++;
  }
}


/********************************************************************/

static void CountCplCoeffCom0(float *stream_coeffs_cpl,
                             float *stream_coeffs,
                             _AudBlk *audblk,
                             float dpfac,
                             int32_t ch)
{
  int32_t i, ind;
  float cpl_coord = 0;
  int32_t bnd = 0;

  for (i = audblk->cplstrtmant; i < audblk->cplendmant; i += 12) {
    int32_t j;
    float *src = stream_coeffs_cpl+i;
    float *dst = stream_coeffs+i;

    ind = 15 + (int32_t)(audblk->phscor[bnd] * dpfac + 0.5);

    cpl_coord = audblk->cplcoord[ch][bnd] * phsCorTab[ind];
    if      (cpl_coord >  8) cpl_coord =  8;
    else if (cpl_coord < -8) cpl_coord = -8;

    for (j = 0; j < 12; j++) {
      dst[j] = src[j] * cpl_coord;
    }
    bnd++;
  }
}


/********************************************************************/

static void CountCplCoeffCom1(float *stream_coeffs_cpl,
                             float *stream_coeffs,
                             _AudBlk *audblk,
                             float dpfac,
                             int32_t ch)
{
  int32_t i, ind;
  float cpl_coord = 0;
  int32_t bnd = 0;

  for (i = audblk->cplstrtmant; i < audblk->cplendmant; i += 12) {
    int32_t j;
    float *src = stream_coeffs_cpl+i;
    float *dst = stream_coeffs+i;

    ind = 15 + (int32_t)(audblk->phscor[bnd] * dpfac + 0.5) -
                audblk->phscor[bnd];

    cpl_coord = audblk->cplcoord[ch][bnd] * phsCorTab[ind];
    if      (cpl_coord >  8) cpl_coord =  8;
    else if (cpl_coord < -8) cpl_coord = -8;

    for (j = 0; j < 12; j++) {
      dst[j] = src[j] * cpl_coord;
    }
    bnd++;
  }
}


/********************************************************************/

static void CountCplCoeffCom(float *stream_coeffs_cpl,
                             float *stream_coeffs,
                             _AudBlk *audblk,
                             float dpfac,
                             float epfac,
                             int32_t ch)
{
  int32_t i, ind;
  float cpl_coord = 0;
  int32_t bnd = 0;

  for (i = audblk->cplstrtmant; i < audblk->cplendmant; i += 12) {
    int32_t j;
    float *src = stream_coeffs_cpl+i;
    float *dst = stream_coeffs+i;

    ind = 15 + (int32_t)(audblk->phscor[bnd] * dpfac + 0.5) -
               (int32_t)(audblk->phscor[bnd] * epfac + 0.5);

    cpl_coord = audblk->cplcoord[ch][bnd] * phsCorTab[ind];
    if      (cpl_coord >  8) cpl_coord =  8;
    else if (cpl_coord < -8) cpl_coord = -8;

    for (j = 0; j < 12; j++) {
      dst[j] = src[j] * cpl_coord;
    }
    bnd++;
  }
}

/********************************************************************/
#endif
/********************************************************************/

void uncoupleChannel(AC3Dec *state,
                     int32_t ch)
{
  _AudBlk *audblk = &(state->audblk);
  float *stream_coeffs = state->coeffs[ch];
  float *stream_coeffs_cpl = state->cplChannel;
  float dpfac = phscorFac[state->out_acmod][state->bsi.acmod][state->bsi.surmixlev][ch];
  float epfac = phscorFac[audblk->phsoutmod][state->bsi.acmod][state->bsi.surmixlev][ch];
#ifndef REF_DECODER_COMPATIBLE
  int32_t testBsi = 0;

  if (state->bsi.acmod == 0x02 && ch == 1 && audblk->phsflginu)
    testBsi = 1;

  CountCplCoeffCom(stream_coeffs_cpl, stream_coeffs, audblk,
                       dpfac, epfac, testBsi, ch);
#else
  if (dpfac == epfac) {
    CountCplCoeff00(stream_coeffs_cpl, stream_coeffs, audblk, ch);
  } else if (dpfac == 0) {
    if (epfac == 1) {
      CountCplCoeff01(stream_coeffs_cpl, stream_coeffs, audblk, ch);
    } else {
      CountCplCoeff0Com(stream_coeffs_cpl, stream_coeffs, audblk,
                        epfac, ch);
    }
  } else if (dpfac == 1) {
    if (epfac == 0) {
      CountCplCoeff10(stream_coeffs_cpl, stream_coeffs, audblk, ch);
    } else {
      CountCplCoeff1Com(stream_coeffs_cpl, stream_coeffs, audblk,
                        epfac, ch);
    }
  } else {
    if (epfac == 0) {
      CountCplCoeffCom0(stream_coeffs_cpl, stream_coeffs, audblk,
                        dpfac, ch);
    } else if (epfac == 1) {
      CountCplCoeffCom1(stream_coeffs_cpl, stream_coeffs, audblk,
                        dpfac, ch);
    } else {
      CountCplCoeffCom(stream_coeffs_cpl, stream_coeffs, audblk,
                    dpfac, epfac, ch);
    }
  }
#endif
}

/********************************************************************/

