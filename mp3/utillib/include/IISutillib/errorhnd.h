/*****************************************************************************

                       (C) copyright Fraunhofer-IIS (1997-2006)
                               All Rights Reserved

 project:              utility library
 Initial author:       B. Teichman  <tmn@iis.fhg.de>, W.Schildbach <sdb@iis.fhg.de>
 contents/description: - generic error handler header file
                       - part of the interface to utillib
                       - do not include this file, include utillib.h instead

 This software and/or program is protected by copyright law and
 international treaties. Any reproduction or distribution of this
 software and/or program, or any portion of it, may result in severe
 civil and criminal penalties, and will be prosecuted to the maximum
 extent possible under law.

 $Id: errorhnd.h,v 1.1 2009/04/28 20:17:48 audiodsp Exp $

******************************************************************************/

#ifndef __ERRORHND_H
#define __ERRORHND_H

#include <stdio.h>

#include "IISutillib/errtype.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG
#ifdef __GNUC__
#define CDI __FILE__,__LINE__,__FUNCTION__
#else
#define CDI __FILE__,__LINE__,0
#endif
#else
#define CDI "",0,""
#endif

/* helper function for macro WARNIF */
void _warnif(char *condition, const char *file, const int line, const char *function, char *text);

/* helper function for macro ABORT */
void _abort(char *text, const char *file, const int line, const char *function);
#define ABORT(text) _abort(text, CDI)

#ifdef ERROR
#undef ERROR
#endif
/* generate an ERROR_INFO handle from error text */
HANDLE_ERROR_INFO ERROR( const char *file, const int line, const char *function, const char *fmt, ...);

/* hand error through and append stack traceback */
HANDLE_ERROR_INFO _handBack( const char *file, const int line, const char *function, HANDLE_ERROR_INFO e);

/* return the length of this error message */
int errorLength(HANDLE_ERROR_INFO e, const char *fmt, int levels);

/* return the error message in string s */
void errorText(HANDLE_ERROR_INFO e, const char *fmt, char *s, int levels);

/* return the error number of e */
long errorCode(HANDLE_ERROR_INFO e);

void freeErrorTraceback(HANDLE_ERROR_INFO e);

#if (!defined(_NDEBUG) && !defined(NDEBUG)) || defined(DEBUG)

#define WARNIF(condition,text) ((void)( !(condition) || \
 (_warnif(#condition,CDI,(text)),0)))
#define WARN(text) ((void)( _warnif("",CDI,(text)),0))

#else

#define WARNIF(c,t) ((void)0)
#define WARN(c) ((void)0)

#endif

#define WARNALL(text) ((void)( _warnif("",CDI,(text)),0))

#define handBack(e) _handBack(CDI,(e))


#define SAFECALL(e,f) if (noError == e) { if (noError != (e = (f))) { e = handBack(e);}}
  /* ndf 20050509: Use SAFECALL() to simplify error handling in case of just one function call.
     e.g.:
     ***** before:

     HANDLE_ERROR_INFO  error = noError;
     [...]
     if (noError == error) {
       error = function1(a,b,c);
       if (noError != error) {
         error = handBack(error);
       }
     }
     if (noError == error) {
       error = function2(a,b,c);
       if (noError != error) {
         error = handBack(error);
       }
     }
     [...]
     return error;

     ***** after:
     HANDLE_ERROR_INFO  error = noError;
     [...]
     SAFECALL(error,function1(a,b,c));
     SAFECALL(error,function2(a,b,c));
     [...]
     return error;

  */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #define __ERRORHND_H */
