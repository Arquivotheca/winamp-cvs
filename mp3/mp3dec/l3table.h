/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: l3table.h
 *   project : ISO/MPEG-Decoder
 *   author  : Martin Sieler
 *   date    : 1998-05-26
 *   contents/description: HEADER - tables for iso/mpeg-decoding (layer3)
 *
 *
\***************************************************************************/

/*
 * $Date: 2009/04/28 20:17:44 $
 * $Id: l3table.h,v 1.1 2009/04/28 20:17:44 audiodsp Exp $
 */

/*-------------------------------------------------------------------------*/

#ifndef __L3TABLE_H__
#define __L3TABLE_H__

/* ------------------------ includes --------------------------------------*/

/* ------------------------------------------------------------------------*/

typedef struct
{
  int l[23];
  int s[14];
} SF_BAND_INDEX[3][3];

/* ------------------------------------------------------------------------*/

extern const SF_BAND_INDEX sfBandIndex;

/*-------------------------------------------------------------------------*/

#endif
