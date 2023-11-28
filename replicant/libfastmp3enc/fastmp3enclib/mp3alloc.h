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
*   $Id: mp3alloc.h,v 1.1 2007/05/29 16:02:29 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/
#ifndef _MP3ALLOC_H_
#include "utillib.h"

#if !defined NDEBUG || defined MEMDEBUG
#define mp3Alloc(a)    iisMalloc_mem(a,__FILE__,__LINE__)
#define mp3Calloc(a,b) iisCalloc_mem(a,b,__FILE__,__LINE__)
#define mp3Free(a)     iisFree_mem(a,__FILE__,__LINE__)
#else
#define mp3Alloc(a)    iisMalloc_mem(a,"",0)
#define mp3Calloc(a,b) iisCalloc_mem(a,b,"",0)
#define mp3Free(a)     iisFree_mem(a,"",0)
#endif

#endif  /* #ifndef _MP3ALLOC_H_ */
