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
*   $Id: bitres.h,v 1.1 2007/05/29 16:02:27 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef BITRES_H
#define BITRES_H

/* Bit reservoir  */

struct BITRESERVOIR_INIT
{
  int bitrate;
  int sampleRate;
  int sceCpe;
  int nChannels;
};

struct BITRESERVOIR_STATE
{
  int fillLevel[2];
  /* ... */
};

int InitBitReservoir(struct BITRESERVOIR_INIT *init);
int AdvanceBitReservoir(
  struct BITRESERVOIR_STATE *state,
  int bitsAvail[2]           /* out, number of bits avail. for this frame */
  );
int UpdateBitreservoir(struct BITRESERVOIR_STATE *state, int bitsUsed[2]);

#endif
