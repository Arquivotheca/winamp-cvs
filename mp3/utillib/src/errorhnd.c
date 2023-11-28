/******************************************************************************

                (C) copyright Fraunhofer - IIS (1997-2006)
                         All Rights Reserved

 Initial author:  B.Teichmann <tmn@iis.fhg.de>,
                  W.Schildbach <sdb@iis.fhg.de>
 contents/description: generic error handler

 This software and/or program is protected by copyright law and
 international treaties. Any reproduction or distribution of this
 software and/or program, or any portion of it, may result in severe
 civil and criminal penalties, and will be prosecuted to the maximum
 extent possible under law.

 $Id: errorhnd.c,v 1.1 2009/04/28 20:17:48 audiodsp Exp $

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "IISutillib/ngsalloc.h"
#include "IISutillib/errorhnd.h"

static struct ERROR_INFO ERROR_ERRORHND_NOMEMORY = {
  "errorhnd.c",0,"error()","errorhnd.c: the error handler has run out of memory",0
};

/* print a warning message with a file and line number. If the file contains
   a path, only print the filename portion of it. If an explanatory text is
   given, print it in braces */
void _warnif(char *condition, const char *file, const int line, const char *function, char *text)
{
#ifdef WIN32
  const char *fn = strrchr(file,'\\');
#else
  const char *fn = strrchr(file,'/');
#endif
  if (fn) fn++;
  else fn = file;
  fprintf(stderr,"\x7##########\n#warning: %s in file %s,%d",condition,fn,line);
  if (text && *text) fprintf(stderr," (%s)",text);
  fprintf(stderr,"\n##########\n");
}

/* print a fatal error message and exit */
void _abort(char *text, const char *file, const int line, const char *function)
{
  fprintf(stderr,"\x7##########\nfatal error: %s in file %s, %d",
          text,file,line);
  fprintf(stderr,"\nThis is a program design failure - Aborting."
          "\n##########\n");
  exit(20);
}

static char *mystrdup(const char *c)
{
  char *s;
  unsigned int n;

  if (!c) return 0;
  n = strlen(c);
  s = (char*)ngsMalloc(n+1);
  if (s) strcpy(s,c);
  return s;
}

/*
  generate an error info from error text
*/

HANDLE_ERROR_INFO ERROR(const char *file, const int line, const char *function, const char *fmt, ...)
{
  const char *fn;      /* to parse filename */
  char tmp[512];       /* tmp buffer */
  unsigned int errLen; /* the number of chars we need for the error msg */
  HANDLE_ERROR_INFO e; /* the error info we will pass back */

  va_list ap;
  va_start (ap, fmt);

  /* find out how many characters we need in the error string buffer
   (including terminating zero) */
  vsprintf(tmp, fmt, ap);
  errLen = strlen(tmp) + 1;

  e = (HANDLE_ERROR_INFO)ngsCalloc(sizeof(struct ERROR_INFO),1);
  if ( !e ) {
    return (& ERROR_ERRORHND_NOMEMORY);
  }
  e->errText = (char*)ngsMalloc(errLen);
  if ( !e->errText ) {
    ngsFree(e);
    return (&ERROR_ERRORHND_NOMEMORY);
  }

  /* separate filename from path */
  if (file) {
#ifdef WIN32
    fn = strrchr(file,'\\');
#else
    fn = strrchr(file,'/');
#endif
    if (fn) fn++;
    else fn=file;
  } else {
    fn = file;
  }

  e->module = mystrdup(fn);
  e->lineNo = line;
  e->procedure = mystrdup(function);

  strcpy(e->errText, tmp);

  va_end(ap);

#if (!defined(_NDEBUG) && !defined(NDEBUG)) || defined(DEBUG)
  fprintf(stderr,"\x7unhandled error return?\r");
#endif

  return (e);
}

/*
  hand back the error info given, prepending own info for traceback
*/

HANDLE_ERROR_INFO _handBack(const char *file, const int line, const char *function,
                            HANDLE_ERROR_INFO e)
{
  HANDLE_ERROR_INFO e1;

  if (e == 0) return e;

  e1 = (HANDLE_ERROR_INFO)ngsCalloc(sizeof(struct ERROR_INFO),1);
  if (e1) {
    HANDLE_ERROR_INFO e2;
    const char *fn;

    /* separate filename from path */
    if (file) {
#ifdef WIN32
      fn = strrchr(file,'\\');
#else
      fn = strrchr(file,'/');
#endif
      if (fn) fn++; else fn=file;
    } else {
      fn = file;
    }
    e1->module = mystrdup(fn);
    e1->lineNo = line;
    e1->procedure = mystrdup(function);

    /* find last error message and append outselves */
    e2=e;
    while(e2->traceBack) e2 = e2->traceBack;

    e2->traceBack = e1;
  }
  return e; /* just a dummy right now */
}

/* hash a string into a 16-bit number. Could do better in the future... */
static long hash(char *s)
{
  short x = 0;

  if (!s) return 0;

  while (*s) {
    x <<= 1;
    x ^= *s++ * 257;
  }
  return x;
}

/*
  format an error message according to format *fmt.
  fmt is a sprintf-like format string with special control characters.
  %e translates into the error message, %n into line number, %m into
  module name, %f into function name, %# into error (hash) number,
  %% into %.

  If s==0, only the error string length will be calculated.
*/

static int fmtErr(HANDLE_ERROR_INFO e, const char *fmt, char *s, int levels)
{
  int i;
  int len = 0;

  if (s) *s = 0;
  for (i=0; i<levels && e; i++, e = e->traceBack) {
    long errcode = (hash(e->module)<<16)+e->lineNo;
    const char *fmt1 = fmt;
    char fmt2[3];

    while (*fmt1) {
      /* copy all non-control chars */
      while (*fmt1 && *fmt1 != '%' && *fmt1 != '\\') {
        if (s) *s++ = *fmt1;
        fmt1++;
        len++;
      }

      switch(*fmt1) {

      case '\0':
        continue; /* next level error message */

      case '\\':
        fmt2[0] = fmt1[0]; fmt2[1] = fmt1[1]; fmt2[2]=0;
        if (s) {sprintf(s,fmt2); s += strlen(s);}
        len++;

        fmt1++;
        if (!*fmt1) continue;
        fmt1++;
        break;

      case '%':
        fmt1++;
        switch(*fmt1) {

        case '%':
          if (s) *s++ = '%';
          len++;
          break;
        case '#':
          if (s) {sprintf(s,"%08lx",errcode); s += strlen(s);}
          len += 8;
          break;
        case 'e':
          if (e->errText) {
            if (s) {sprintf(s,"%s",e->errText); s += strlen(s);}
            len += strlen(e->errText);
          }
          break;
        case 'f':
          if (e->procedure) {
            if (s) {sprintf(s,"%s",e->procedure); s += strlen(s);}
            len += strlen(e->procedure);
          }
          break;
        case 'm':
          if (e->module) {
            if (s) {sprintf(s,"%s",e->module); s += strlen(s);}
            len += strlen(e->module);
          }
          break;
        case 'n':
          if (s) {sprintf(s,"%ld",e->lineNo); s += strlen(s);}
          len += 1+(e->lineNo ? (int)floor(log((float)e->lineNo)/log(10.0)):0);
          break;
        case '\0':
          continue; /* next level error message */
        }
        fmt1++;
        break;
      } /* switch on first char */
    } /* while (*fmt1) */
  }
  if (s) *s = 0;
  len++;
  return len;
}

void errorText(HANDLE_ERROR_INFO e, const char *fmt, char *s, int levels)
{
  fmtErr(e, fmt, s, levels);
}

int errorLength(HANDLE_ERROR_INFO e, const char *fmt, int levels)
{
  return fmtErr(e, fmt, 0, levels);
}

void freeErrorTraceback(HANDLE_ERROR_INFO e)
{
  while(e)
  {
    HANDLE_ERROR_INFO e1 = e->traceBack;
    if (e->module) ngsFree(e->module);
    if (e->procedure) ngsFree(e->procedure);
    if (e->errText) ngsFree(e->errText);
    ngsFree(e);
    e = e1;
  }
}


long errorCode(HANDLE_ERROR_INFO e)
{
  long val=(hash(e->module)<<16)+e->lineNo;

  return val;
}
