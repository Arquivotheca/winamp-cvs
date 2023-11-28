/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: polyphase.h
 *   project : ISO/MPEG-Decoder
 *   author  : Stefan Gewinner
 *   date    : 1998-05-26
 *   contents/description: polyphase class - HEADER
 *
 *
\***************************************************************************/

/*
 * $Date: 2009/11/29 21:04:32 $
 * $Id: polyphase.h,v 1.2 2009/11/29 21:04:32 audiodsp Exp $
 */

#ifndef __POLYPHASE_H__
#define __POLYPHASE_H__

/* ------------------------ includes --------------------------------------*/

#include "mpeg.h"

/*-------------------------------------------------------------------------*/

#define HAN_SIZE 512

/*-------------------------------------------------------------------------*/

// Class for (inverse) Polyphase calculation.

class CPolyphase
{

public:

  CPolyphase(const MPEG_INFO &_info, int _qual, int _downMix);

  ~CPolyphase() {}

  void Init();
//  short *Apply(POLYSPECTRUM &sample, short *pPcm, int frms=18);
  float *Apply(POLYSPECTRUM &sample, float *pPcm, int frms=18);

protected:

  int   bufOffset;
  float syn_buf[2][HAN_SIZE];

  const MPEG_INFO &info ;      // info-structure
  int              qual;       // quality (full, half, quarter spectrum)
  int              downMix;    // downmix stereo to mono

  //void window_band_m(int bufOffset, short *out_samples, int short_window);
  //void window_band_s(int bufOffset, short *out_samples, int short_window);
  void window_band_m(int bufOffset, float *out_samples, int short_window);
  void window_band_s(int bufOffset, float *out_samples, int short_window);
};

/*-------------------------------------------------------------------------*/
#endif
