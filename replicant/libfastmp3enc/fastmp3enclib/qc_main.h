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
*   $Id: qc_main.h,v 1.1 2007/05/29 16:02:31 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef _QC_MAIN_H
#define _QC_MAIN_H

#include "qc_data.h"
#include "interface.h"

/* Quantizing & coding stage */

int  QCOutNew(struct QC_OUT **phQC);
void QCOutDelete(struct QC_OUT *hQC);
int  QCOutInit(struct QC_OUT *hQC);

int  QCNew(struct QC_STATE **phQC);
int  QCInit(struct QC_STATE *hQC, struct QC_INIT *init);
void QCDelete(struct QC_STATE *hQC);

int QCMain(struct QC_STATE *hQC,
           /*struct PSY_OUT_LIST *firstPsyOut,*/
           struct PSY_OUT_LIST *qcPsyOut,
           struct PSY_OUT_LIST *lastPsyOut,
           struct QC_OUT *qcOut, 
           CHANNEL_ELEMENT * ce,    /* out */
           int needAncBitsGran
           ); /* returns error code */

int BitReservoir(struct QC_STATE *hQC, int cc);
void updateBitres(struct QC_STATE *hQC, int usedBits, int cc);


void resetDeltaBitres(struct QC_STATE *hQC, int cc);
void updateDeltaBitres(struct QC_STATE *hQC, int dynBitsUsed, int cc);

int needBitsForAnc(int ancBitsPerFrame,
                   int ancMode,
                   int* numAncDataBytes, 
                   unsigned int* writeOflOnce,
                   int granules); 

#endif /* _QC_MAIN_H */
