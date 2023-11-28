#ifndef _MM_INLINE_H_
#define _MM_INLINE_H_

#include "mmio.h"
#include <string.h>

static void __inline _mminline_memcpy_word(void *dst, const void *src, int count)
{
    memcpy(dst, src, count*2);
}


#endif
