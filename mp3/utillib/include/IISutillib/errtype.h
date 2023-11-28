/*****************************************************************************

                       (C) copyright Fraunhofer-IIS (1997-2006)
                               All Rights Reserved

   Initial author:       W.Schildbach <sdb@iis.fhg.de>
   contents/description: - error handler types
                         - part of the interface to utillib
                         - do not include this file, include utillib.h instead

 This software and/or program is protected by copyright law and
 international treaties. Any reproduction or distribution of this
 software and/or program, or any portion of it, may result in severe
 civil and criminal penalties, and will be prosecuted to the maximum
 extent possible under law.

 $Id: errtype.h,v 1.1 2009/04/28 20:17:48 audiodsp Exp $

******************************************************************************/

#ifndef __ERRTYPE_H
#define __ERRTYPE_H

#ifdef WIN32

#pragma pack(push, 8)

#else

#ifdef macintosh
#if defined(__MRC__) || defined(__MWERKS__)
#pragma options align=mac68k
#endif
#endif

#endif

struct ERROR_INFO {
  char *module;    /* source module where error occured */
  long lineNo;     /* line in which error occured */
  char *procedure; /* procedure where error occured */
  char *errText;   /* the actual error text */
  struct ERROR_INFO *traceBack; /* next module up */
};
typedef struct ERROR_INFO* HANDLE_ERROR_INFO;

#define noError ((HANDLE_ERROR_INFO)0)

#ifdef WIN32
#pragma pack(pop)
#else
#ifdef macintosh
#if defined(__MRC__) || defined(__MWERKS__)
#pragma options align=reset
#endif
#endif
#endif

#endif /* #define __ERRTYPE_H */
