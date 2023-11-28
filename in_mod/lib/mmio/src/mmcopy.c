/*
 Mikmod Portable Memory Allocation

 By Jake Stine of Divine Entertainment (1996-2000) and
    Jean-Paul Mikkers (1993-1996).

 File: mmcopy.c

 Description:

*/

#include "mmio.h"
#include <string.h>

// ======================================================================================
    CHAR *_mm_strdup(MM_ALLOC *allochandle, const CHAR *src)
// ======================================================================================
// Same as Watcom's _strdup function - allocates memory for a string
// and makes a copy of it.  Ruturns NULL if failed.
{
    CHAR *buf;

    if(!src) return NULL;
    if((buf = (CHAR *)MikMod_malloc(allochandle, strlen(src)+1)) == NULL) return NULL;

    strcpy(buf,src);
    return buf;
}
