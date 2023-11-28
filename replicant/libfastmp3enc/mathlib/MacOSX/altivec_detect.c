/*****************************************************************************

                          (C) copyright Fraunhofer-IIS (2004-2005)
                                All Rights Reserved

  $Id: altivec_detect.c,v 1.1 2007/05/29 16:02:33 audiodsp Exp $
  Initial author:       E. Allamanche
                        (original code from Ian Ollmann, iano@cco.caltech.edu)
  contents/description: Portable AltiVec detection routine
                        More examples can be found at 
            http://developer.apple.com/hardware/ve/g3_compatibility.html

   This software and/or program is protected by copyright law and
   international treaties. Any reproduction or distribution of this 
   software and/or program, or any portion of it, may result in severe 
   civil and criminal penalties, and will be prosecuted to the maximum 
   extent possible under law.

******************************************************************************/

#if defined (__VEC__)

/* 
 * On MacOSX we could use the Gestalt manager, but this 
 * would not be portable. The following code fragment 
 * only requires a POSIX compliant environment with 
 * a signal handler. This code has been suggested 
 * by I. Ollmann and posted on the SIMDtech.org mailing list
 */

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>


extern int IsAltiVecPresent(void);

static void TryAltiVec(void);
static void sig_ill_handler(int sig);

volatile int gIsAltiVecPresent = -1L;
static sigjmp_buf       gEnv;

/*Hopefully, this will not be optimized to zero
 *even though it doesn't do anything. Any function
 *that uses AltiVec would work fine here.
 */
__attribute__((noinline)) 
static void TryAltiVec(void)
{
  /* 
    This instruction causes an illegal instruction exception
    on CPU's which lack an AltiVec unit
   */
  __asm volatile("vor v0, v0, v0" : /* no output */ : /* no input */ : "v0");
}

static void sig_ill_handler(int sig)
{
  /*Set our flag to 0 to indicate AltiVec is illegal*/
  gIsAltiVecPresent = 0;
  
  /*long jump back to safety*/
  siglongjmp( gEnv, 0);
}

int IsAltiVecPresent( void )
{
  if( -1L == gIsAltiVecPresent )
  {
    sig_t   old_handler;
    
    /*Set AltiVec to ON*/
    gIsAltiVecPresent = 1;
    
    /*swap out the old handler for a new one */
    /*that will set gIsAltiVecPresent to 0 if triggered */
    old_handler = signal( SIGILL, sig_ill_handler );
    
    /*Try AltiVec. If AltiVec is not available,*/
    /*the new signal handler will be triggered */
    if( 0 == sigsetjmp( gEnv, 0) )
    {
      TryAltiVec();
    }
    
    /*Replace the old signal handler*/
    signal( SIGILL, old_handler );
  }
  return gIsAltiVecPresent;
}

#endif /* defined(__VEC__) */