/******************************************************************************

                       (C) copyright Fraunhofer-IIS (1997-2006)
                               All Rights Reserved

 project:              utility library
 Initial author:       W. Schildbach
 contents/description: - Memory allocator wrapper
                       - part of the interface to utillib
                       - do not include this file, include utillib.h instead

 This software and/or program is protected by copyright law and
 international treaties. Any reproduction or distribution of this
 software and/or program, or any portion of it, may result in severe
 civil and criminal penalties, and will be prosecuted to the maximum
 extent possible under law.

 $Id: ngsalloc.h,v 1.1 2009/04/28 20:17:48 audiodsp Exp $

******************************************************************************/

#ifndef _ngsalloc_h
#define _ngsalloc_h

#include <stdlib.h>

#if defined __ICL
#define ALIGN_16_BYTE __declspec(align(16))

#elif defined _MSC_VER
#define ALIGN_16_BYTE __declspec(align(16))

#elif defined __GNUC__ && !defined __sparc__ && !defined __sparc_v9__ /* mul: fixing problems with gcc 3.0.4 on SunOS 5.7 */
#define ALIGN_16_BYTE  __attribute__((aligned(16)))

#else
#define ALIGN_16_BYTE
#endif

/* memory alignment for SSE */
#define MEM_ALIGNMENT 16

#ifdef __cplusplus
extern "C" {
#endif
extern void ngsInitAllocationCheck(void);
extern void ngsAllocationCheck(void);
#ifdef __cplusplus
           }
#endif

#if !defined (MEMDEBUG) && !defined (MEM_ALIGNMENT)

/* map ngsAlloc functions to their stdlib counterparts. */
/* only for compatibility */
#define ngsMalloc(s) malloc(s)
#define ngsCalloc(s,t) calloc(s,t)
#define ngsFree(p) free(p)

/* new name */
#define iisMalloc(s) malloc(s)
#define iisCalloc(s,t) calloc(s,t)
#define iisFree(p) free(p)

#else

#if defined NDEBUG && !defined (MEMDEBUG)
/* use our own wrapper functions */
/* only for compatibility */
#define ngsMalloc(s)   iisMalloc_mem(s,  "",0)
#define ngsCalloc(s,t) iisCalloc_mem(s,t,"",0)
#define ngsFree(p)     iisFree_mem(p,    "",0)

/* new name */
#define iisMalloc(s) iisMalloc_mem(s,    "",0)
#define iisCalloc(s,t) iisCalloc_mem(s,t,"",0)
#define iisFree(p) iisFree_mem(p,        "",0)
#else
/* use our own wrapper functions */
/* only for compatibility */
#define ngsMalloc(s) iisMalloc_mem(s,__FILE__,__LINE__)
#define ngsCalloc(s,t) iisCalloc_mem(s,t,__FILE__,__LINE__)
#define ngsFree(p) iisFree_mem(p,__FILE__,__LINE__)

/* new name */
#define iisMalloc(s) iisMalloc_mem(s,__FILE__,__LINE__)
#define iisCalloc(s,t) iisCalloc_mem(s,t,__FILE__,__LINE__)
#define iisFree(p) iisFree_mem(p,__FILE__,__LINE__)
#endif

#ifdef __cplusplus
extern "C" {
#endif
extern void *iisMalloc_mem(size_t s, char *file, int line);
extern void *iisCalloc_mem(size_t s, size_t t, char *file, int line);
extern void iisFree_mem(void *p, char *file, int line);
#ifdef __cplusplus
           }
#endif

#endif /* ifdef MEMDEBUG */

#endif /* ifndef _ngsalloc_h */
