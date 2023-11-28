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
*   $Id: psy_data.h,v 1.1 2007/05/29 16:02:30 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef _PSY_DATA_H
#define _PSY_DATA_H

#include "psy_types.h"
#include "mp3alloc.h"

#include "block_switch.h"
#include "polyana.h"
#include "transform.h"

struct PSY_DATA {
  BLOCK_SWITCHING_HANDLE  hBlockSwitching;           /* block switching   */
  int windowSequence;
  int oldWindowSequence;

  POLY_PHASE_HANDLE       hPolyPhase;                /* poly phase        */
  ALIGN_16_BYTE TRANSFORM_BUFFER        transformBuffer;           /* mdct input buffer */
  ALIGN_16_BYTE MDCT_SPECTRUM           mdctSpectrum;


  PB_ENERGY pbEnergy;
  PB_THRESHOLD pbThreshold;                          /* threshold       */


  ALIGN_16_BYTE float pbThresholdnm2[MAX_PB];                     /* preEchoControl  */
  ALIGN_16_BYTE float pbThresholdnm1[MAX_PB];

  ALIGN_16_BYTE SFB_THRESHOLD sfbThreshold;                       /* adapt           */
  ALIGN_16_BYTE SFB_ENERGY sfbEnergy;                             /* sfb Energy      */
  ALIGN_16_BYTE SFB_ENERGY sfbEnergyMS;                           

  /* the following we need only in channel 2. Currently, we allocate these
     structures in both channels... Need to clean this up. TODO sdb */
  ALIGN_16_BYTE int isLimit[TRANS_FAC] ;
#ifndef NO_INTENSITY
  ALIGN_16_BYTE IS_POSITION sfbIsPositions;
  ALIGN_16_BYTE IS_DIR_COMPONENT sfbIsDirX;
  ALIGN_16_BYTE IS_DIR_COMPONENT sfbIsDirY;
  ALIGN_16_BYTE SFB_ENERGY       sfbIsCrossProduct;
#endif /* #ifdef NO_INTENSITY */
  int oldMsFlag;

  float dampingFactor ; /* to damp the side channel */
};

#endif /* _PSY_DATA_H */
