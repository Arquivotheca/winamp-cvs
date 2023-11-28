/***********************************************************************************************
*                                   MPEG Layer3-Audio Encoder                                  *
*                                                                                              *
*                                 © 1999-2005 by Fraunhofer IIS                                *
*                                       All Rights Reserved                                    *
*                                                                                              *
*   This software and/or program is protected by copyright law and international treaties.     *
*   Any reproduction or distribution of this software and/or program, or any portion of it,    *
*   may result in severe civil and criminal penalties, and will be prosecuted to the           *
*   maximum extent possible under law.                                                         *
*                                                                                              *
*   $Id: quantize.h,v 1.1 2007/05/29 16:02:31 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef _QUANTIZE_H_
#define _QUANTIZE_H_

/* quantizing */
#include "psy_const.h" 

#define MAX_QUANT 8191

void mp3QuantizeSpectrum (const int    sfbActive,   /* # of sfbs containing energy. */
			  const int   *sfbOffset,
			  const float *mdctSpectrum,
			  const float *threshold,
			  const float *sfbMaxSpec,
			  const int    globalGain,
			  const int    blockType,
			  const int    subBlockGain[TRANS_FAC],
			  const int    preEmphasisFlag,
			  const int    scfScale,
			  const int   *maxScf,
			  const int    fullPsych,
			  int         *scalefactors,
			  int         *quantizedSpectrum);

void mp3InvQuantizeSpectrum(const int    sfbCnt, 
                            const int   *sfbOffset, 
                            const int   *quaSpectrum,
                            const int    globalGain,
                            const int    blockType,
                            const int    subBlockGain[TRANS_FAC],
                            const int    preEmphasisFlag,
                            const int    scfScale,
                            const int   *scalefactors,
                            float       *invQuantizedSpectrum);

#endif /* _QUANTIZE_H_ */
