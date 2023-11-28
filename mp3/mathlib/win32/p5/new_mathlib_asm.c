/******************************************************************************

                       (C) copyright Fraunhofer-IIS (1997-2005)
                               All Rights Reserved

   $Id: new_mathlib_asm.c,v 1.1 2009/04/28 20:17:43 audiodsp Exp $
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
/*#include "bastypes.h"*/
#include "mathlib.h"

#if defined _M_IX86 /* Not defined for x64 */

void quantFLOATtoUINT_Opt(float a, float b, const float X[], unsigned int Z[], int n)
/* Z=floor(a+bX) */
{
  short sav_cw,new_cw;
  int i;
  
  /* loop unrolling by 4 */

  i = 0;
  if (n & 1) {
    Z[0] = (unsigned int) (a + b*X[0]);
    i++;
  }
  if (n & 2) {
    float t1 = a + b*X[i];
    float t2 = a + b*X[i + 1];
    Z[i] = (unsigned int) (t1);
    Z[i + 1] = (unsigned int) (t2);
    i += 2;
  }
#ifdef DEBUG /* this is the old C code, left here for reference & debugging */
  for (; i < n; i += 4) {
    float t1 = a + b*X[i];
    float t2 = a + b*X[i + 1];
    float t3 = a + b*X[i + 2];
    float t4 = a + b*X[i + 3];
    Z[i] = (unsigned int) (t1);
    Z[i + 1] = (unsigned int) (t2);
    Z[i + 2] = (unsigned int) (t3);
    Z[i + 3] = (unsigned int) (t4);
  }
#else
__asm
{
  fnstcw sav_cw;
  mov ax,sav_cw;
  or  ax,0x0c00; /* rounding mode to "chop" */
  mov new_cw,ax;
  fldcw new_cw;

  mov ebx,X;
  mov ecx,Z;
  mov edx,i;
  mov eax,n;
  sub eax,edx;
  jz L2;

  fld a;
  fld b;
L1: fld DWORD PTR [ebx + edx*4];
  fmul st(0),st(1)               ; 0* b a
  fld DWORD PTR [ebx + edx*4 + 4];
  fmul st(0),st(2)               ; 1* 0* b a
  fld DWORD PTR [ebx + edx*4 + 8];
  fmul st(0),st(3)               ; 2* 1* 0* b a
  fxch st(2);
  fadd st(0),st(4);
  fxch st(2)                     ; 2* 1* 0*+ b a
  fld DWORD PTR [ebx + edx*4 + 12];
  fmul st(0),st(4); 3* 2* 1* 0*+ b a
  fxch st(2)      ; 1* 2* 3* 0*+ b a
  fadd st(0),st(5); 1*+ 2* 3* 0*+ b a
  fxch st(3)      ; 0*+ 2* 3* 1*+ b a
  fistp DWORD PTR [ecx + edx*4]; 2* 3* 1*+ b a
  fadd st(0),st(4);
  fxch st(2)                   ; 1*+ 3* 2*+ b a
  fistp DWORD PTR [ecx + edx*4 + 4]; 3* 2*+ b a
  fadd st(0),st(3); 3*+ 2*+ b a
  fxch st(1);
  fistp DWORD PTR [ecx + edx*4 + 8];
  fistp DWORD PTR [ecx + edx*4 + 12];

  add edx,4;
  sub eax,4;
  jnz L1;

  fstp st(0);
  fstp st(0);

L2: fldcw sav_cw;
}
#endif

}

/*
  unfortunately, there is no rint() in the MSVC RTL. So we roll
  our own here. The RTL that comes with the Intel compiler appears to have
  one, however. So make sure it is really MSVC we're compiling under.
*/
#if defined(_MSC_VER) && !defined(__ICL)
double rint(double x)
{
  unsigned int t = _controlfp(0,0); /* save old fp-control word */
  _controlfp(_RC_NEAR,_MCW_RC); /* set rounding mode to "nearest" */
  _asm {
    fld qword ptr[x]
    frndint
    fstp qword ptr[x]
  }
  _controlfp(t,0xfffff); /* restore old fp-control word */
  return x;
}
#endif
#endif