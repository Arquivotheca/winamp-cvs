/******************************** MPEG Audio Encoder **************************

                       (C) copyright Fraunhofer-IIS (1997-2006)
                               All Rights Reserved

   Initial author:       W. Schildbach
   contents/description: Memory allocator wrapper

   This software and/or program is protected by copyright law and
   international treaties. Any reproduction or distribution of this
   software and/or program, or any portion of it, may result in severe
   civil and criminal penalties, and will be prosecuted to the maximum
   extent possible under law.

   $Id: ngsalloc.c,v 1.2 2009/07/23 20:04:55 audiodsp Exp $

******************************************************************************/

#include "IISutillib/ngsalloc.h"

#if defined(MEMDEBUG) || defined(MEM_ALIGNMENT)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef USE_SSE_MALLOC
#include <xmmintrin.h>
#endif

#ifdef MEMDEBUG
#include "IISutillib/uci.h"

#define NELEM 5000 /* can track no more than NELEM allocations */

struct allocInfo {
  void *p;
  unsigned long size;
  char *file;
  int line;
  int freed;
};

static unsigned long totalMalloc = 0;
static int nTop = 0;
static struct allocInfo plist[NELEM];
static int debugcnt = 0;

static void addToList(void *p, unsigned long size, char *file, int line)
{
  assert(nTop < NELEM);

  plist[nTop].p = p;
  plist[nTop].size = size;
  plist[nTop].line = line;
  plist[nTop].file = _strdup(file);
  plist[nTop].freed = 0;
  nTop++;

  totalMalloc += size;
}

static int removeFromList(void *p, char * file, int line)
{
  int i;
  int err = 0;

  for (i=nTop-1; i>=0; i--) {
    if (plist[i].p == p) {
      if (plist[i].freed == 1) {
        fprintf(stderr,"location %lx was freed in %s, %d\n",
                (unsigned long)plist[i].p, plist[i].file, plist[i].line);
        err = 1;
      } else {
        plist[i].freed = 1;
        plist[i].file = file;
        plist[i].line = line;
      }
      break;
    }
  }
  if (i==-1 || err) return 1;
  totalMalloc -= plist[i].size;
  return 0;
}
#endif /* MEMDEBUG */

void *iisCalloc_mem(size_t s, size_t t, char *file, int line)
{
  void *p;

  p = iisMalloc_mem(s*t,file,line);
  if (p) memset(p,0,s*t);
  return p;
}

void *iisMalloc_mem(size_t s, char *file, int line)
{
  void *p = NULL;

  if (s != 0) {
#ifdef MEM_ALIGNMENT
#ifdef USE_SSE_MALLOC
    p = _mm_malloc(s,16);
#else
    void *p1;
    long modulo;
    s += sizeof(void *)+MEM_ALIGNMENT;
    p = malloc(s);
    if (p) {
      p1 = (void *)((void **)p+1);

      modulo = (unsigned long)((char*)p1) % MEM_ALIGNMENT;
      if (modulo) {
        modulo = MEM_ALIGNMENT - modulo;
        p1 = (void *) ((char *)p1 + modulo);
        *((void **)p1 - 1) = p;
      }
      *((void **)p1 - 1) = p;
      p = p1;
    }
#endif
#else
    p = malloc(s);
#endif
  }

#ifdef MEMDEBUG
  fprintf(stderr,"[%d] ",debugcnt);
  fprintf(stderr,"%s, %d: malloc(%d) ",
          file,line,s);
  if (--debugcnt == 0) {
    fprintf(stderr,"simulated malloc failure...\n");
    return 0;
  }
  addToList(p,s,file,line);
  fprintf(stderr,"(total %ld) = 0x%08lx\n", totalMalloc,(unsigned long)p);
#endif /* MEMDEBUG */

  return p;
}

void iisFree_mem(void *p, char *file, int line)
{
#ifdef MEMDEBUG
  fprintf(stderr,"free(0x%08lx), %s (%d)\n",(unsigned long)p,file,line);
  if (!p) {
    fprintf(stderr,
            "\07***WARNING: %s, %d: free(0)\n",file,line);
    fflush(stderr);
    return;
  }
  if (removeFromList(p,file,line)) {
    fprintf(stderr,
            "\07***WARNING: %s, %d: freeing unallocated memory 0x%08lx\n",file,line,
            (unsigned long)p);
    fflush(stderr);
  }
#endif /* MEMDEBUG */
#ifdef MEM_ALIGNMENT
#ifdef USE_SSE_MALLOC
  _mm_free((char*)p);
  return;
#else
  if (p!=NULL)
    p = *((void **)p-1);
#endif
#endif
  free(p);
}

/*
  only for memory leak checking do we need the following code.
*/
#ifdef MEMDEBUG

#ifdef WIN32
#define CDECL __cdecl
#else
#define CDECL
#endif

void * CDECL operator new(size_t numberofbytes)
{
  return iisMalloc_mem(numberofbytes,__FILE__,__LINE__);
}

void CDECL operator delete(void *p)
{
  iisFree_mem(p,__FILE__,__LINE__);
}

#endif /* MEMDEBUG */

#endif /* defined(MEMDEBUG) || defined(MEM_ALIGNMENT) */

void ngsInitAllocationCheck(void)
{
#ifdef MEMDEBUG
  debugcnt = 0;
  GetIntParameter("allocfail",1,0,5000,"let the i-th iisMalloc() fail",
                  "use this to test proper malloc() return code checking",
                  &debugcnt);
#endif /* MEMDEBUG */
}

void ngsAllocationCheck(void)
{
#ifdef MEMDEBUG
  int i;

  for (i=0; i<nTop && plist[i].freed; i++) ;

  if (i<nTop) {
    printf("\07unfreed allocations:\n");
    for (i=0; i<nTop; i++) {
      if (!plist[i].freed) {
        printf("%s, %d\n",plist[i].file,plist[i].line);
      }
    }
    getchar();
  } else {
    printf("All mallocs() were freed properly -- found no memory leaks.\n");
  }
#endif /* MEMDEBUG */
}
