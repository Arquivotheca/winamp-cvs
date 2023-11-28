/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: mp3bufferparse.h
 *   project : MPEG Syncer
 *   author  : Martin Sieler
 *   date    : 1998-08-26
 *   contents/description: mp3 buffer parser (C++)
 *
 *
\***************************************************************************/

/*
 * $Date: 2009/04/28 20:17:45 $
 * $Id: mp3bufferparse.h,v 1.1 2009/04/28 20:17:45 audiodsp Exp $
 */

#ifndef __MP3BUFFERPARSE_H__
#define __MP3BUFFERPARSE_H__

/* ------------------------ includes --------------------------------------*/

#ifdef WIN32
  #include <windows.h>
  #include <mmreg.h>
#else
  #include "regtypes.h"
#endif

#include "l3reg.h"

/*-------------------------- defines --------------------------------------*/

#ifndef NULL
  #define NULL 0
#endif

/*-------------------------------------------------------------------------*/

//
// mp3ScanBufferFormat
//
// scan 'cbSize' bytes in 'pBuffer' for MPEG Layer-3 data and fill out
// the MPEGLAYER3WAVEFORMAT structure 'wf', if successfull.
//
// Set 'fForceBlocksize' to 'true' if a value for wf.nBlockSize is needed,
// although it might not be consistent (in this case the average frame size
// is written to wf.nBlockSize).
//
// return value
//    -1  : not an MPEG Layer-3 bitstream
//    else: offset of first MPEG Layer-3 Header (in bits)
//

int mp3ScanBufferFormat
    (
    unsigned char        *pBuffer,
    int                   cbSize,
    MPEGLAYER3WAVEFORMAT &wf,
    bool                  fForceBlocksize = false,
    int                  *pBitrate = NULL          // [OUT] bitrate in [bit/s]
    );

/*-------------------------------------------------------------------------*/
#endif
