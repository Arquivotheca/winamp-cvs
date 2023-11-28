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
*   $Id: psy_const.h,v 1.1 2007/05/29 16:02:30 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#ifndef _PSYCONST_H
#define _PSYCONST_H

/* define this if the new psych with one spreading-operation should be used,
   else the normal psych is used */
/*#define LEAN_PSYCH */

/* define this to use the old method for threshold reduction */
/*#define OLD_THR_REDUCTION*/

#define TRUE  1
#define FALSE 0

#define FRAME_LEN_LONG    576
#define TRANS_FAC         3
#define FRAME_LEN_SHORT   (FRAME_LEN_LONG/TRANS_FAC)
#define FFT_LEN            1024

#define PCM_LEVEL         32768.0f
#define NORM_PCM          (PCM_LEVEL/32768.0f)
#define NORM_PCM_ENERGY   (NORM_PCM*NORM_PCM)
#define PCM_CORRECTION    60 /* Has to be 60, if PCM_LEVEL is 32768.0f, if PCM_LEVEL is 1.0f this has to be 0 */


enum
{
  MPEG1 = 0,
  MPEG2 = 1,
  MPEG2_5 = 2
};

enum
{
  STEREO = 0,
  JOINT_STEREO = 1,
  DUAL_CHANNEL = 2,
  SINGLE_CHANNEL = 3
};

/* Block types */

enum
{
  LONG_WINDOW = 0,
  START_WINDOW,
  SHORT_WINDOW,
  STOP_WINDOW
};

#define MAX_CHANNELS 2

#define MAX_PB_SHORT  52
#define MAX_PB_LONG   88
#define MAX_PB        88

#define MAX_SFB_SHORT		13
#define MAX_SFB_LONG		22
#define MAX_SFB	            39
#define MAX_GROUPED_SFB     MAX_SFB

#define POLY_PHASE_DELAY    240
#define FFT_OFFSET              0

#define BLOCK_SWITCHING_DATA_SIZE FRAME_LEN_LONG

#endif /* _PSYCONST_H */
