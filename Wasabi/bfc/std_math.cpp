#include <precomp.h>

#include "std_math.h"

unsigned long BSWAP_C(unsigned long input) {
#if defined(_WIN32) && !defined(_WIN64)
  _asm {
    mov eax, input
    bswap eax
    mov input, eax
  };
  return input;
#else
#  ifdef GCC
  __asm__ volatile (
		    "\tmov %0, %%eax\n"
		    "\tbswap %%eax\n"
		    : "=m" (input)
		    : 
		    : "%eax" );
#  else
  unsigned char ret[4];
  unsigned char *s = reinterpret_cast<unsigned char *>(&input);
  ret[0] = s[3];
  ret[1] = s[2];
  ret[2] = s[1];
  ret[3] = s[0];
  return *reinterpret_cast<unsigned long *>(ret);
#  endif
#endif
}

void premultiplyARGB32(ARGB32 *words, int nwords) 
{
  for (; nwords > 0; nwords--, words++) 
	{
    unsigned char *pixel = (unsigned char *)words;
    unsigned int alpha = pixel[3];
    if (alpha == 255) continue;
    pixel[0] = (pixel[0] * alpha) >> 8;	// blue
    pixel[1] = (pixel[1] * alpha) >> 8;	// green
    pixel[2] = (pixel[2] * alpha) >> 8;	// red
  }
}

