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
*   $Id: sf_cmprs.h,v 1.1 2007/05/29 16:02:31 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef _SF_CMPRS_H
#define _SF_CMPRS_H

#include "psy_const.h"
#include "scf.h"
#include "mp3alloc.h"

extern ALIGN_16_BYTE const int scfCntPerPartitionDefault [2][4];
extern ALIGN_16_BYTE const int scfBitsPerPartitionDefault[2][4];

unsigned int
findScfCompressMPEG1(const int blockType,
                     const int scf[MAX_GROUPED_SFB],
                     int       scfCntPerPartition[SCF_PARTITIONS],
                     int       scfBitsPerPartition[SCF_PARTITIONS]);

unsigned int
findScfCompressMPEG2(const int intensityOn, /* 1 if intensity and channel == 1 */
                     const int blockType,
                     const int scf[MAX_GROUPED_SFB],
                     const int preEmphasis,
                     int       scfCntPerPartition[SCF_PARTITIONS],
                     int       scfBitsPerPartition[SCF_PARTITIONS],
                     const int isLimit[TRANS_FAC],
                     int       sfbActive);

#endif /* _SF_CMPRS_H */
