/*****************************************************************************

                       (C) copyright Fraunhofer-IIS (1997-2005)
                               All Rights Reserved

   This software and/or program is protected by copyright law and
   international treaties. Any reproduction or distribution of this
   software and/or program, or any portion of it, may result in severe
   civil and criminal penalties, and will be prosecuted to the maximum
   extent possible under law.

   $Id: cpuinfo.c,v 1.1 2009/04/28 20:17:42 audiodsp Exp $

******************************************************************************/

#include "cpuinfo.h"

#ifdef X86_PLATFORM
#include "adetect.h"
#endif


/* structure which carry all platform independent information */
CPU_INFO CPU_Info;


/***********************************************************************/
/*	         				GetCPUInfo()							   */
/***********************************************************************/
unsigned long GetCPUInfo (CPU_INFO_CAPS caps)
{
  unsigned long capability = 0;

  /* all X86 features */
#ifdef X86_PLATFORM
  /* - here we can use <detect.c/adetect.h> for feature testing */
  CPU_Info.has_fpu  = GetCPUCaps(HAS_FPU);
  CPU_Info.has_mmx  = GetCPUCaps(HAS_MMX);
  CPU_Info.has_sse  = GetCPUCaps(HAS_SSE);
  CPU_Info.has_sse2 = GetCPUCaps(HAS_SSE2);
  CPU_Info.has_sse3 = GetCPUCaps(HAS_SSE3);

 switch (caps)
    {
    case HAS_CPU_FPU:
      capability = CPU_Info.has_fpu;
      break;

    case HAS_CPU_MMX:
      capability = CPU_Info.has_mmx;
      break;

    case HAS_CPU_SSE:
      capability = CPU_Info.has_sse;
      break;

    case HAS_CPU_SSE2:
      capability = CPU_Info.has_sse2;
      break;

    case HAS_CPU_SSE3:
      capability = CPU_Info.has_sse3;
      break;

    default:
      capability = 0;
      break;
    }
#endif

  /* Power PC hardware platform */
#ifdef PPC_PLATFORM
  extern int IsAltiVecPresent(void);

  CPU_Info.has_altivec = IsAltiVecPresent();

  switch(caps)
  {
    case HAS_CPU_ALTIVEC:
      capability = CPU_Info.has_altivec;
      break;

    default:
      capability = 0;
      break;
  }
#endif
  return (capability);
}
