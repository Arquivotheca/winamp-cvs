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
*   $Id: sf_estim.h,v 1.1 2007/05/29 16:02:31 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef _SF_ESTIM_H
#define _SF_ESTIM_H

#include "mconfig.h"

#include "psy_const.h"
#include "scf.h"
#include "mp3alloc.h"

struct PSY_OUT_CHANNEL;

extern void
(*CalcFormFactorChannel) (float * restrict sfbFormFactor,
                          float * restrict sfbMaxSpec,
                          const struct PSY_OUT_CHANNEL *psyOutChan);

void
EstimateScaleFactorsChannel(const struct PSY_OUT_CHANNEL *psyOutChan,
                            int *scf,
                            float *sfbFormFactor,
                            float *sfbMaxSpec);

void
AdaptScfToMp3Channel(/*const struct PSY_OUT_CHANNEL *psyOutChan,*/
                     const int     sfbActive,
                     const int     blockType,
                     /*const int     mpegVersion,*/
                     const int     fullPsych,
                     int           scf[MAX_GROUPED_SFB],
                     int          *globalGain,
                     int           subBlockGain[TRANS_FAC],
                     unsigned int *scfScale,
                     /*unsigned int *scfCompress,*/
                     const int     scfCntPerPartition[SCF_PARTITIONS],      
                     const int     scfBitsPerPartition[SCF_PARTITIONS],
                     const int     allowPreEmphasis,
                     int          *preEmphasisFlag,
		     int          *maxScf,
                     float        *sfbMaxSpec);

#endif /* _SF_ESTIM_H */
