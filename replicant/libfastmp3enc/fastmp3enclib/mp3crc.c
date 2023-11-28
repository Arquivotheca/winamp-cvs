/***********************************************************************************************
*                                   MPEG Layer3-Audio Encoder                                  *
*                                                                                              *
*                                 © 1997-2005 by Fraunhofer IIS                                *
*                                       All Rights Reserved                                    *
*                                                                                              *
*   This software and/or program is protected by copyright law and international treaties.     *
*   Any reproduction or distribution of this software and/or program, or any portion of it,    *
*   may result in severe civil and criminal penalties, and will be prosecuted to the           *
*   maximum extent possible under law.                                                         *
*                                                                                              *
*   $Id: mp3crc.c,v 1.1 2007/05/29 16:02:29 audiodsp Exp $                             *
*   author:   M.Werner / W.Schildbach                                                          *
*   contents/description:       routines to calculate a CRC checksum                           *
*                                                                                              *
************************************************************************************************/

#include "mconfig.h"
#include "mp3crc.h"

/*****************************************************************************

    functionname:crcAdvance 
    description: calculate a CRC over one byte
    returns:     new CRC
    input:       CRC start value, CRC polynomial bits (excluding leading one),
                 the leading one by itself, right-shifted by 1, and a bitstream
                 to read from.
    globals:     none

    example:     for a polynomial X^16+X^15+X^2+X^0, call this routine with
                 crcPoly = 0x8005, crcMask = 0x8000
*****************************************************************************/

void mp3CrcAdvance(unsigned short crcPoly, unsigned short crcMask,
                   unsigned long *crc, unsigned char byte)
{
  int i;
  for (i=0; i<8; i++)
  {
    unsigned short flag = (*crc) & crcMask ? 1:0;
    flag ^= (byte & 0x80 ? 1 : 0);
    (*crc)<<=1;
    byte <<= 1;
    if(flag)
      (*crc) ^= crcPoly;
  }
}
