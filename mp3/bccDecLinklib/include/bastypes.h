/***************************************************************************\
 *                         Fraunhofer IIS 
 *                 (c) 1997 - 2008 Fraunhofer IIS
 *                       All Rights Reserved.
 *
 *
 *    This software and/or program is protected by copyright law and
 *    international treaties. Any reproduction or distribution of this
 *    software and/or program, or any portion of it, may result in severe
 *    civil and criminal penalties, and will be prosecuted to the maximum
 *    extent possible under law.
 *
\***************************************************************************/

#ifndef __BASTYPES_H
#define __BASTYPES_H
#pragma message(__FILE__":Warning:Should be removed or replaced by something from somewhere")

typedef unsigned char  BYTE;
typedef unsigned short WORD;
#if defined(__alpha__) || defined(__alpha) || defined(__sgi)
typedef unsigned int   DWORD; /* long is 64 bits on these machines */
#else
typedef unsigned long  DWORD;
#endif
typedef int            BOOL;
typedef signed   int   INT;
typedef signed long    LONG;
typedef unsigned long  ULONG;
typedef unsigned int   UINT;
typedef float          FLOAT;
typedef double         DOUBLE;
typedef unsigned char  UCHAR;
typedef char           CHAR;

#define TRUE               1
#define FALSE              0

#ifndef NULL
#define NULL 0
#endif

#define INVALID_HANDLE NULL     

#endif
