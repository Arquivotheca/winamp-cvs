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
*   $Id: mp3crc.h,v 1.1 2007/05/29 16:02:29 audiodsp Exp $                             *
*   author: Werner / Schildbach                                                                *
*   contents/description:       routines to calculate a CRC checksum                           *
*                                                                                              *
************************************************************************************************/

#ifndef _CRC_H
#define _CRC_H

extern void mp3CrcAdvance(unsigned short crcPoly, unsigned short crcMask,
                          unsigned long *crc, unsigned char byte);

#endif /* _CRC_H */
