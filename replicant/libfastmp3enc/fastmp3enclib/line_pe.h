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
*   $Id: line_pe.h,v 1.1 2007/05/29 16:02:29 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef __LINE_PE_H
#define __LINE_PE_H

#include "psy_const.h"
#include "mp3alloc.h"

#ifdef OLD_THR_REDUCTION

int CalcPeSfbByLine(const float *mdctSpectrum,
                    const float *sfbEnergy,
                    const float *sfbThreshold, 
                    const int   *sfbOffset,
                    const int   sfbCnt);

#else /* #ifdef OLD_THR_REDUCTION */

typedef struct {
   /* these are calculated by prepareSfbPe */
   ALIGN_16_BYTE float sfbLdEnergy[MAX_GROUPED_SFB]; /* log(sfbEnergy)/log(2) */
   ALIGN_16_BYTE int   sfbNLines[MAX_GROUPED_SFB];   /* number of relevant lines in sfb */
   int   offset;                       /* 0 for long, something for short */
   /* the rest is calculated by calcSfbPe */
   ALIGN_16_BYTE float sfbPe[MAX_GROUPED_SFB];           /* pe for each sfb */
   ALIGN_16_BYTE float sfbConstPart[MAX_GROUPED_SFB];    /* constant part for each sfb */
   ALIGN_16_BYTE int   sfbNActiveLines[MAX_GROUPED_SFB]; /* number of active lines in sfb */
   float pe;                               /* sum of sfbPe */
   float constPart;                        /* sum of sfbConstPart */
   int   nActiveLines;                     /* sum of sfbNActiveLines */
} PE_CHANNEL_DATA;


extern void (*prepareSfbPe) (PE_CHANNEL_DATA *peChanData,
			     const float *sfbEnergy,
			     const float *sfbThreshold,
			     const float *sfbFormFactor,
			     const int   *sfbOffset,
			     const int    sfbCnt,
			     const int    windowSequence);

void calcSfbPe(PE_CHANNEL_DATA *peChanData,
               const float *sfbEnergy,
               const float *sfbThreshold,
               const int    sfbCnt);

#endif /* #ifdef OLD_THR_REDUCTION */


#endif
