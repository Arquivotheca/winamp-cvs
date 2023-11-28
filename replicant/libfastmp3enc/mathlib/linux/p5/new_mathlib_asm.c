/******************************************************************************

                       (C) copyright Fraunhofer-IIS (1997-2005)
                               All Rights Reserved

   $Id: new_mathlib_asm.c,v 1.1 2007/05/29 16:02:34 audiodsp Exp $
   Initial author:       W. Schildbach
   contents/description: assembly language replacements for parts of mathlib

   This software and/or program is protected by copyright law and
   international treaties. Any reproduction or distribution of this 
   software and/or program, or any portion of it, may result in severe 
   civil and criminal penalties, and will be prosecuted to the maximum 
   extent possible under law.

******************************************************************************/

#include <math.h>
#include <float.h>
#include "mathlib.h"



#if defined(__GNUC__) && defined(i386) && !defined(__x86_64) 

#include "controlfp.h"

unsigned short
_controlfp(unsigned short _newcw, unsigned short _mask)
{
  /* when optimizing, inline assembler does not seem to work correctly
     when accessing arguments from the inline assembly. Therefore, we
     move arguments to local storage before accessing them */

  volatile unsigned short newcw = _newcw, mask = _mask;
  volatile unsigned short cw;

  __asm__ volatile("                 \n\
      wait                           \n\
      fstcw  %0                      \n\
      Mov    %1, %%ax \n" /* newcw */ "\
      mov    %2, %%bx \n" /* mask */  "\
      and    %%bx, %%ax              \n\
      not    %%bx                    \n\
      nop                            \n\
      wait                           \n\
      mov    %0, %%dx \n" /* old control word */ "\
      and    %%bx, %%dx              \n\
      or     %%ax, %%dx              \n\
      mov    %%dx, %0                \n\
      wait                           \n\
      fldcw  %0                "
        : /* outputs */   "=m" (cw)
        : /* inputs */    "m" (newcw), "m" (mask)
        : /* registers */ "ax", "bx", "dx"
  );
  return cw;
}

void quantFLOATtoUINT_Opt(float a, float b, const float X[], unsigned int Z[], int n)
/* Z=floor(a+bX) */
{
  short sav_cw;
  int i;

  /* loop unrolling by 2 */

  i = 0;
  if (n & 1) {
    Z[0] = (unsigned int) (a + b*X[0]);
    i++;
  }
#ifdef DEBUG /* this is the old C code, left here for reference & debugging */
  for (; i < n; i += 2) {
    float t1 = a + b*X[i];
    float t2 = a + b*X[i + 1];
    Z[i] = (unsigned int) (t1);
    Z[i + 1] = (unsigned int) (t2);
  }
#else
  sav_cw = _controlfp(0,0);     /* save old rounding mode */
  _controlfp(_RC_DOWN,_MCW_RC); /* set rounding mode to "down" */

  __asm volatile ("             \n\
    movl %2,%%ebx             \n\
    movl %3,%%ecx             \n\
    movl %4,%%edx             \n\
    movl %5,%%eax             \n\
    sub %%edx,%%eax           \n\
    jz end                    \n\
                              \n\
    flds %0                   \n\
    flds %1                   \n\
                              \n\
loopStart:                    \n\
    flds  (%%ebx,%%edx,4)     \n\
    flds 4(%%ebx,%%edx,4)     \n\
    fmul %%st(2),%%st(0)      \n\
    fxch %%st(1)              \n\
    fmul %%st(2),%%st(0)      \n\
    fxch %%st(1)              \n\
    fadd %%st(3),%%st(0)      \n\
    fxch %%st(1)              \n\
    fadd %%st(3),%%st(0)      \n\
    fistp  (%%ecx,%%edx,4)    \n\
    fistp 4(%%ecx,%%edx,4)    \n\
    addl $2,%%edx             \n\
    subl $2,%%eax             \n\
    jnz loopStart             \n\
                              \n\
    fstp %%st(0)              \n\
    fstp %%st(0)              \n\
end:                          \n"
                : /* outputs */ 
                : /* inputs */ "m" (a), "m" (b), "m" (X), "m" (Z),
                               "m" (i), "m" (n)
                : /* registers */ "ax","bx","cx","dx"
  );

  _controlfp(sav_cw, _MCW_RC); /* set rounding mode to old mode */
#endif
}
#endif /* GNUC && i386 && !defined(__x86_64)*/
