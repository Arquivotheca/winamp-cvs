/***************************************************************************\
 *
 *               (C) copyright Fraunhofer - IIS (2004, 2005)
 *                        All Rights Reserved
 *
 *   filename: detect.c
 *   project : math library
 *
 *   This software and/or program is protected by copyright law and
 *   international treaties. Any reproduction or distribution of this
 *   software and/or program, or any portion of it, may result in severe
 *   civil and criminal penalties, and will be prosecuted to the maximum
 *   extent possible under law.
 *
 * $Id: detect.c,v 1.1 2009/04/28 20:17:42 audiodsp Exp $
 *
 \***************************************************************************/

/******************************************************************************

 Copyright (c) 1999 Advanced Micro Devices, Inc.

 LIMITATION OF LIABILITY:  THE MATERIALS ARE PROVIDED *AS IS* WITHOUT ANY
 EXPRESS OR IMPLIED WARRANTY OF ANY KIND INCLUDING WARRANTIES OF MERCHANTABILITY,
 NONINFRINGEMENT OF THIRD-PARTY INTELLECTUAL PROPERTY, OR FITNESS FOR ANY
 PARTICULAR PURPOSE.  IN NO EVENT SHALL AMD OR ITS SUPPLIERS BE LIABLE FOR ANY
 DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF PROFITS,
 BUSINESS INTERRUPTION, LOSS OF INFORMATION) ARISING OUT OF THE USE OF OR
 INABILITY TO USE THE MATERIALS, EVEN IF AMD HAS BEEN ADVISED OF THE POSSIBILITY
 OF SUCH DAMAGES.  BECAUSE SOME JURISDICTIONS PROHIBIT THE EXCLUSION OR LIMITATION
 OF LIABILITY FOR CONSEQUENTIAL OR INCIDENTAL DAMAGES, THE ABOVE LIMITATION MAY
 NOT APPLY TO YOU.

 AMD does not assume any responsibility for any errors which may appear in the
 Materials nor any responsibility to support or update the Materials.  AMD retains
 the right to make changes to its test specifications at any time, without notice.

 NO SUPPORT OBLIGATION: AMD is not obligated to furnish, support, or make any
 further information, software, technical information, know-how, or show-how
 available to you.

 So that all may benefit from your experience, please report  any  problems
 or  suggestions about this software to 3dsdk.support@amd.com

 AMD Developer Technologies, M/S 585
 Advanced Micro Devices, Inc.
 5900 E. Ben White Blvd.
 Austin, TX 78741
 3dsdk.support@amd.com

*******************************************************************************

 DETECT.C

 AMD3D 3D library code: Code to detect for 3DNow! capability.

*******************************************************************************/

#include <string.h>
#include "adetect.h"


/******************************************************************************
  Private data cache
******************************************************************************/
static int   detect_base (void);
static unsigned int  features[2]     = { 0, 0 }; /* [0] hold edx, [1] hold ecx after cpuid with eax set to 0x00000001.*/
static unsigned int  ext_features[3] = { 0, 0, 0 }; /* [0] hold edx, [1] hold ebx [2] hold ecx after cpuid with eax set to 0x80000001, */ 
static unsigned int  processor[2]    = { 0, 0 }; /* [0] hold eax, [1] hold ebx after cpuid with eax set to 0x00000001.*/
static unsigned char proc_idstr[16];             /* hold eax, ecx and abx after cpuid with eax set to 0x00000000.*/
static unsigned char proc_namestr[48];           /* hold processor name, after cpuid with eax set to 0x80000004, 0x80000003, 0x80000002.*/
static unsigned int  proc_cache_l1[4] = { 0, 0, 0, 0 }; /* hold L1 cache info after cpuid with eax set to 0x80000005.*/
static unsigned int  proc_cache_l2[4] = { 0, 0, 0, 0 }; /* hold L2 cache info after cpuid with eax set to 0x80000006.*/

/******************************************************************************
 Routine:   GetCPUCaps
 Input:     Which capability to query (see enum CPUCAPS for an exhaustive list)
 Returns:   Depends on input:
            CPU_TYPE        - enum CPU_TYPES
            CPU_VENDOR      - enum CPU_VENDORS
            CPU_*_STRING    - const char *
            CPU CACHE DATA  - Cache size in KB
            All others      - Boolean
 Comment:   This function returns information about the capabilies of the
            CPU on which it is called.  The input enumeration covers both
            processor feature bits (the HAS_* values) and "synthesized"
            information.
            
            THE HAS_* QUERIES SHOULD ALWAYS BE USED IN PREFERENCE TO DIRECTLY 
            CHECKING THE CPU TYPE WHEN LOOKING FOR FEATURES.  For instance,
            it is *always* better to check for HAS_3DNOW directly, rather
            than rely on checking for a K6_2, K6_3, or Athlon.  Likewise,
            HAS_MMX should always be used in preference to other methods
            of checking for MMX instructions.

            The features bits are checked against either the base feature
            bits (CPUID function 1, edx) or the extended feature bits
            (CPUID function 0x80000001, edx), as appropriate.  The return
            value is 1 for feature present or 0 for feature not present,

            The synthesized information is created by interpreting the CPUID
            results in some way.

            The full set of feature bits (both base and extended) are
            implemented in this version.

            Note that this routine caches the feature bits when first called,
            so checking multiple features is relatively efficient after the
            first invocation.  However, tt is not recommended practice to
            use GetCPUCaps() inside time-critical code.

******************************************************************************/
unsigned int GetCPUCaps (CPUCAPS cap)
{
  static int init = 0;
  int res = -1;
  unsigned int nBrandID;        /* Athlon64 or Opteron*/

   /* Detect CPUID presence once, since all other requests depend on it */
  if (init == 0)
    init = detect_base();

  if (init == -1) {
    /* No CPUID, so no CPUID functions are supported */
    return 0;
  }

  /* Otherwise, perform the requested tests */
  switch (cap) {

    /* Synthesized Capabilities */
  case HAS_CPUID:
    /* Always true if this code gets executed */
    res = 1;
    break;

    /* Detect CPU vendor strings */
  case CPU_VENDOR:
    if (     !strncmp ((char *) proc_idstr, "AuthenticAMD", 12))
      res = VENDOR_AMD;
    else if (!strncmp ((char *) proc_idstr, "GenuineIntel", 12))
      res = VENDOR_INTEL;
    else if (!strncmp ((char *) proc_idstr, "CyrixInstead", 12))
      res = VENDOR_CYRIX;
    else if (!strncmp ((char *) proc_idstr, "CentaurHauls", 12))
      res = VENDOR_CENTAUR;
    else
      res = VENDOR_UNKNOWN;
    break;

  case CPU_TYPE:
    /* Return a member of the CPUTYPES enumeration
       Note: do NOT use this for determining presence of chip features, such
       as MMX and 3DNow!  Instead, use GetCPUCaps (HAS_MMX) and GetCPUCaps (HAS_3DNOW),
       which will accurately detect the presence of these features on all chips which
       support them. */
    res = UNKNOWN;
    switch (GetCPUCaps (CPU_VENDOR)) {   /*detect CPU type*/
    case VENDOR_AMD:
      switch ((processor[0] >> 8) & 0xf) {
        /* extract family code */
      case 4: /* Am486/AM5x86 */
        res = AMD_Am486;
        break;

      case 5: /* K6 */
        switch ((processor[0] >> 4) & 0xf) {
          /* extract model code */
        case 0:
        case 1:
        case 2:
        case 3: res = AMD_K5;       break;
        case 4: /* Not really used */
        case 5: /* Not really used */
        case 6:
        case 7: res = AMD_K6;       break;
        case 8: res = AMD_K6_2;     break;
        case 9: /* K6-III starts here, all subsequent K6 family processors */
        case 10:/* are recognized as K6-III.  If new product releases */
        case 11:/* invalidate this, the new CPU models will be referenced */
        case 12:/* in here */
        case 13:
        case 14:
        case 15:res = AMD_K6_3;     break;
        }
        break;
      
	  case 6: /* Athlon, Duron, AthlonXP */
        switch ((processor[0] >> 4) & 0xf) {
               /* extract model code */
	    case 0:
		case 1:
		case 2: 
		case 4: res = AMD_ATHLON; break;
		case 3: 
		case 7: res = AMD_DURON; break; 
		case 5: /* AthlonXP starts here, all subsequent AthlonXP family processors */
		case 6: /* are recognized as AthlonXP.  If new product releases */
		case 8: /* invalidate this, the new CPU models will be referenced */
		case 9: /* in here */
		case 10: 
		case 11:
		case 12:
		case 13:
		case 14:
		case 15: res = AMD_ATHLON_XP; break;
		}
				 
        break;
      
      
	  case 15:   /* AMD Athlon64 or AMD Opteron Processors*/
			/** get 8-bit BrandID, if there is no 8-bit BrandID, check for 12-bit BrandID.
			One of those must always be available! If not then this CPU is AMD Engineering Sample! */
	    nBrandID = processor[1] & 0xff; 
		if (nBrandID == 0)
		  nBrandID = ext_features[1] & 0xfff;
		else
		  nBrandID = ((nBrandID << 3) & 0x700) | (nBrandID & 0x1f);
		switch (nBrandID >> 6){ /* check 6 MSB from nBrandID*/	
		
		case 4: res = AMD_ATHLON_64; break;
		case 5: res = AMD_ATHLON_64_X2_DualCore; break;
		case 6: res = AMD_ATHLON_64_FX_DualCore; break;
		case 8: 
		case 9: 
		case 10: res = AMD_TURION_64; break;
		case 14: 
		case 15:
		case 16:
		case 18:
		case 19:
		case 20:
		case 22:
		case 23: res = AMD_OPTERON; break;
		case 29:
		case 30:
		case 32: 
		case 33:
		case 34:
		case 35: res = AMD_SEMPRON; break;
		case 36: res = AMD_ATHLON_64_FX; break;
		
		}
		break;
	
	}
	break;



    case VENDOR_INTEL:
      /* extract family code */
      switch ((processor[0] >> 8) & 0xf) {
      case 4:
        /* extract model code */
        switch ((processor[0] >> 4) & 0xf) {
        case 0: res = INTEL_486DX;  break;
        case 1: res = INTEL_486DX;  break;
        case 2: res = INTEL_486SX;  break;
        case 3: res = INTEL_486DX2; break;
        case 4: res = INTEL_486SL;  break;
        case 5: res = INTEL_486SX2; break;
        case 7: res = INTEL_486DX2E;break;
        case 8: res = INTEL_486DX4; break;
        }
        break;

      case 5:
        /* extract model code */
        switch ((processor[0] >> 4) & 0xf) {
        case 1: res = INTEL_Pentium;    break;
        case 2: res = INTEL_Pentium;    break;
        case 3: res = INTEL_Pentium;    break;
        case 4: res = INTEL_Pentium_MMX;break;
        }
        break;

      case 6:
        /* extract model code */
        switch ((processor[0] >> 4) & 0xf) {
        case 1: res = INTEL_Pentium_Pro;break;
        case 3: res = INTEL_Pentium_II; break;
        case 5: res = INTEL_Pentium_II; break;  /* actual differentiation depends on cache settings */
        case 6: res = INTEL_Celeron;    break;
        case 7: res = INTEL_Pentium_III;break;  /* actual differentiation depends on cache settings */
        case 13: res = INTEL_Pentium_M; break;
		}
        break;
   
      case 15:
        
		/* extract model code */
        switch ((processor[0] >> 4) & 0xf) {  
	case 0: res = INTEL_Pentium_IV; break;
	case 1: res = INTEL_Pentium_IV; break;
	case 2: res = INTEL_Pentium_IV; break;
	case 3: res = INTEL_Pentium_IV; break;
	case 4: res = INTEL_Pentium_IV; break; 
	}
	break;
	 
	  
	  }
      break;

 


    case VENDOR_CYRIX:
      res = UNKNOWN;
      break;

    case VENDOR_CENTAUR:
      res = UNKNOWN;
      break;
    }
    break;

    /* Feature Bit Test Capabilities */
  case HAS_FPU:       res = (features[0] >>  0) & 1;     break;  /* bit  0 = FPU */
  case HAS_VME:       res = (features[0] >>  1) & 1;     break;  /* bit  1 = VME */
  case HAS_DEBUG:     res = (features[0] >>  2) & 1;     break;  /* bit  2 = Debugger extensions */
  case HAS_PSE:       res = (features[0] >>  3) & 1;     break;  /* bit  3 = Page Size Extensions */
  case HAS_TSC:       res = (features[0] >>  4) & 1;     break;  /* bit  4 = Time Stamp Counter */
  case HAS_MSR:       res = (features[0] >>  5) & 1;     break;  /* bit  5 = Model Specific Registers */
  case HAS_PAE:       res = (features[0] >>  6) & 1;     break;  /* bit  6 = PAE */
  case HAS_MCE:       res = (features[0] >>  7) & 1;     break;  /* bit  7 = Machine Check Extensions */
  case HAS_CMPXCHG8:  res = (features[0] >>  8) & 1;     break;  /* bit  8 = CMPXCHG8 instruction */
  case HAS_APIC:      res = (features[0] >>  9) & 1;     break;  /* bit  9 = APIC */
  case HAS_SYSENTER:  res = (features[0] >> 11) & 1;     break;  /* bit 11 = SYSENTER instruction */
  case HAS_MTRR:      res = (features[0] >> 12) & 1;     break;  /* bit 12 = Memory Type Range Registers  */
  case HAS_GPE:       res = (features[0] >> 13) & 1;     break;  /* bit 13 = Global Paging Extensions */
  case HAS_MCA:       res = (features[0] >> 14) & 1;     break;  /* bit 14 = Machine Check Architecture */
  case HAS_CMOV:      res = (features[0] >> 15) & 1;     break;  /* bit 15 = CMOV instruction */
  case HAS_PAT:       res = (features[0] >> 16) & 1;     break;  /* bit 16 = Page Attribue Table */
  case HAS_PSE36:     res = (features[0] >> 17) & 1;     break;  /* bit 17 = PSE36 (Page Size Extensions) */
  case HAS_MMX:       res = (features[0] >> 23) & 1;     break;  /* bit 23 = MMX */
  case HAS_FXSAVE:    res = (features[0] >> 24) & 1;     break;  /* bit 24 = FXSAVE/FXRSTOR instruction */
  case HAS_SSE:       res = (features[0] >> 25) & 1;     break;  /* bit 25 = SSE */
  case HAS_SSE2:      res = (features[0] >> 26) & 1;     break;  /* bit 26 = SSE2 */
  case HAS_SSE3:      res = (features[1] >> 0)  & 1;      break; /* bit 0  = SSE3 */
  
  
																 
 /* In relation to the added definition for HTT, CmpLegacy, and LogicalProcessorCount for multi-threading:
    AMD currently does not support multiple threads per CPU core (Sept. 2005). E.g. The number of threads per CPU core is
   always 1. CmpLegacy(bit 1 ecx by calling __cpuid with a 80000001): core multi-processing legacy mode. If you are working on an AMD processor, 
   you should always get a 0 by calling GetCPUCaps(HAS_HTT).

   Intel: In the available Documentation (March 2005), the bit 1 ecx by calling __cpuid with a 80000001, is defined as a reserved bit,
   but seems to work in the same way as by AMD for the new Intel processors. Waiting for the Docu to improve this part of the code (case HAS_HTT).*/
  
  case HAS_HTT:       
    if(GetCPUCaps (CPU_VENDOR) == VENDOR_AMD){  
      res = ((features[0] >> 28) & 1) &&     /* bit 28 = HTT,  */
	  ( ((ext_features[2] >> 1) & 1) == 0)   /* (bit 1 CmpLegacy*/  
	  && ( ((processor[1] >> 16) & 0xff) > 1 );  break;  /* bits [23:16] Logical Processor Count */  
    }
    else{
      res = ((features[0] >> 28) & 1) &&     /* features[0] bit 28 = HTT,  */
	  ( ((processor[1] >> 16) & 0xff) > 1 );  break;  /* processor[1] bits [23:16] Logical Processor Count */  
    }
                                                      
  case HAS_SSE_MMX:   res = (ext_features[0] >> 22) & 1; break;  /* bit 22 (ext) = SSE MMX Extensions */
  case HAS_MMX_EXT:	res = ((features[0] >> 25)&1)
                          | ((ext_features[0]>> 22)&1); break;  /* bits 25|22(ext) = MMX Extensions */
    
  /* AMD extended information */
  case HAS_3DNOW_EXT: res = (ext_features[0] >> 30) & 1; break;  /* bit 30 (ext) = Extended 3DNow! */
  case HAS_3DNOW:     res = (ext_features[0] >> 31) & 1; break;  /* bit 31 (ext) = 3DNow! */

  default:
    /* These are CPU-specific, so guard their access */
    if (GetCPUCaps (CPU_VENDOR) == VENDOR_AMD) {
      /* K5/K6 supports a restricted range */
      switch (cap) {
      case CPU_L1_DTLB_ASSOC:     res = (proc_cache_l1[1] >> 24) & 0xff; break;
      case CPU_L1_DTLB_ENTRIES:   res = (proc_cache_l1[1] >> 16) & 0xff; break;
      case CPU_L1_ITLB_ASSOC:     res = (proc_cache_l1[1] >>  8) & 0xff; break;
      case CPU_L1_ITLB_ENTRIES:   res = (proc_cache_l1[1] >>  0) & 0xff; break;

      case CPU_L1_DCACHE_SIZE:    res = (proc_cache_l1[2] >> 24) & 0xff; break;
      case CPU_L1_DCACHE_ASSOC:   res = (proc_cache_l1[2] >> 16) & 0xff; break;
      case CPU_L1_DCACHE_LINES:   res = (proc_cache_l1[2] >>  8) & 0xff; break;
      case CPU_L1_DCACHE_LSIZE:   res = (proc_cache_l1[2] >>  0) & 0xff; break;
 
      case CPU_L1_ICACHE_SIZE:    res = (proc_cache_l1[3] >> 24) & 0xff; break;
      case CPU_L1_ICACHE_ASSOC:   res = (proc_cache_l1[3] >> 16) & 0xff; break;
      case CPU_L1_ICACHE_LINES:   res = (proc_cache_l1[3] >>  8) & 0xff; break;
      case CPU_L1_ICACHE_LSIZE:   res = (proc_cache_l1[3] >>  0) & 0xff; break;

      case CPU_L2_CACHE_SIZE:     res = (proc_cache_l2[2] >> 16) & 0xffff;  break;
      case CPU_L2_CACHE_ASSOC:    res = (proc_cache_l2[2] >> 12) & 0x0f;    break;
      case CPU_L2_CACHE_LINES:    res = (proc_cache_l2[2] >>  8) & 0x0f;    break;
      case CPU_L2_CACHE_LSIZE:    res = (proc_cache_l2[2] >>  0) & 0xff;    break;

      default: res = 0; break;  
      }

      if (GetCPUCaps (CPU_TYPE) == AMD_ATHLON) {
        /* Athlon supports these additional parameters */
        switch (cap) {
        case CPU_L1_EDTLB_ASSOC:    res = (proc_cache_l1[0] >> 24) & 0xff; break;
        case CPU_L1_EDTLB_ENTRIES:  res = (proc_cache_l1[0] >> 16) & 0xff; break;
        case CPU_L1_EITLB_ASSOC:    res = (proc_cache_l1[0] >>  8) & 0xff; break;
        case CPU_L1_EITLB_ENTRIES:  res = (proc_cache_l1[0] >>  0) & 0xff; break;

        case CPU_L2_DTLB_ASSOC:     res = (proc_cache_l2[0] >> 28) & 0x0f;    break;
        case CPU_L2_DTLB_ENTRIES:   res = (proc_cache_l2[0] >> 16) & 0xfff;   break;
        case CPU_L2_UTLB_ASSOC:     res = (proc_cache_l2[0] >> 12) & 0x0f;    break;
        case CPU_L2_UTLB_ENTRIES:   res = (proc_cache_l2[0] >>  0) & 0xfff;   break;

        case CPU_L2_EDTLB_ASSOC:    res = (proc_cache_l2[1] >> 28) & 0x0f;    break;
        case CPU_L2_EDTLB_ENTRIES:  res = (proc_cache_l2[1] >> 16) & 0xfff;   break;
        case CPU_L2_EUTLB_ASSOC:    res = (proc_cache_l2[1] >> 12) & 0x0f;    break;
        case CPU_L2_EUTLB_ENTRIES:  res = (proc_cache_l2[1] >>  0) & 0xfff;   break;
        default : res = 0; break;
        }
      }
    }
    break;
  }

  return res;
}


/******************************************************************************
 Routine:   detect_base (void)
 Comment:   This routine is separate from GetCPUCaps() for ease of
            comprehension.  It also encapsulates the only parts of the
            algorithm that are compiler specific.
******************************************************************************/

#ifndef __GNUC__

#if (_MSC_VER > 1399)
#pragma intrinsic(__cpuid)
#else
#ifndef WIN64
__inline static void __cpuid(unsigned int *CPUInfo, int code)
{
  __asm {
    push edi ;   save registers used
    push eax ;
    push ebx ;
    push ecx ;
    push edx ;
    mov  edi, CPUInfo   ; 
    mov eax, code       ; initialize eax
    cpuid               ; cpuid
    mov [edi+ 0], eax   ; copy result to CPUInfo
    mov [edi+ 4], ebx   ;
    mov [edi+ 8], ecx   ;
    mov [edi+12], edx   ;
    pop edx;
    pop ecx;
    pop ebx;
    pop eax;
    pop edi;
  }
}

#else
void __cpuid(unsigned int *CPUInfo, int code)
{
  __asm {
    push rdi ;   save registers used
    push rax ;
    push rbx ;
    push rcx ;
    push rdx ;
    mov  rdi, CPUInfo   ; 
    mov eax, DWORD PTR code       ; initialize eax
    cpuid               ; cpuid
    mov DWORD PTR [rdi+ 0], eax   ; copy result to CPUInfo
    mov DWORD PTR [rdi+ 4], ebx   ;
    mov DWORD PTR [rdi+ 8], ecx   ;
    mov DWORD PTR [rdi+12], edx   ;
    pop rdx;     restore registers used
    pop rcx;
    pop rbx;
    pop rax;
    pop rdi;
  }
}
#endif
#endif  /* #if (_MSC_VER > 1400) */

static int CheckCPUID(void) 
{
  unsigned int CPUInfo[4] = {-1};
  __try {
    __cpuid(CPUInfo, 0x00000000);
  }
  __except (1) {
    return -1;
  }
  return 0;
}


#else

#if !defined __amd64 && !defined __x86_64__  

__inline static void __cpuid(unsigned int CPUInfo[4], int InfoType)
{
 __asm volatile(
   "mov %%ebx, %%esi           \n\t"
   "cpuid                                   \n\t"
   "xchg %%ebx, %%esi           \n\t"
     :"=a"(*CPUInfo), "=S"(*(CPUInfo + 1)), "=c"(*(CPUInfo + 2)),"=d"(*(CPUInfo + 3))
     : "0"(InfoType));
} 

static int CheckCPUID()
{
 int haveCPUID = -1; /* Indicate no cpuid support */

 /* Check if the CPU supports the CPUID instruction */
 __asm volatile(
   /* Try to toggle the CPUID bit in the EFLAGS register */
   "pushf                      \n\t"   /* Push the EFLAGS register onto the stack */
   "popl   %%ecx               \n\t"   /* Pop the value into ECX */
   "movl   %%ecx, %%edx        \n\t"   /* Copy ECX to EDX */
   "xorl   $0x00200000, %%ecx  \n\t"   /* Toggle bit 21 (CPUID) in ECX */
   "pushl  %%ecx               \n\t"   /* Push the modified value onto the stack */
   "popf                       \n\t"   /* Pop it back into EFLAGS */
   /* Check if the CPUID bit was successfully toggled */
   "pushf                      \n\t"   /* Push EFLAGS back onto the stack */
   "popl   %%ecx               \n\t"   /* Pop the value into ECX */
   "cmpl   %%ecx, %%edx        \n\t"   /* Compare ECX with EDX */
   "je     .Lno_cpuid%=        \n\t"   /* Jump if they're identical */
   "movl   $0, %0              \n\t"   /* Set haveCPUID to zero to indicate cpuid support */
   ".Lno_cpuid%=:              \n\t"
     : "=r"(haveCPUID)
     :
     : "%ecx", "%edx");
   return haveCPUID;
}

#else  /* #if !defined __amd64 && !defined __x86_64__ */

__inline static void __cpuid(unsigned int CPUInfo[4], int InfoType)
{
 __asm volatile(
   "mov %%rbx, %%rsi           \n\t"
   "cpuid                                  \n\t"
   "xchg %%rbx, %%rsi           \n\t"
     :"=a"(*CPUInfo), "=S"(*(CPUInfo + 1)), "=c"(*(CPUInfo + 2)),"=d"(*(CPUInfo + 3))
     : "0"(InfoType));
}

/* x86_64 cpus support cpuid instruction */
static int CheckCPUID(void) 
{
  return 0;
}

#endif /* #if !defined __amd64 && !defined __x86_64__   */

#endif /* #ifndef __GNUC__ */


static int detect_base (void)
{
  unsigned int CPUInfo[4] = {-1};
  int nIds;

  if(-1 == CheckCPUID())
    return -1;

  __cpuid(CPUInfo, 0x00000000);
  nIds = CPUInfo[0];
  memset(proc_idstr, 0, sizeof(proc_idstr));
  *(int*)(&proc_idstr[0]) = CPUInfo[1];
  *(int*)(&proc_idstr[4]) = CPUInfo[3];
  *(int*)(&proc_idstr[8]) = CPUInfo[2];
  
  if(nIds>0) {
    __cpuid(CPUInfo, 0x00000001);
    processor[0] = CPUInfo[0];
    processor[1] = CPUInfo[1];
    features[0]  = CPUInfo[3];
    features[1]  = CPUInfo[2];
  }
  

  /* detect extended features */
  __cpuid(CPUInfo, 0x80000000);
  nIds = CPUInfo[0];

  if(nIds > 0x80000006) 
    nIds = 0x80000006;
  switch(nIds) {
  case 0x80000006:
    /* L2 Cache Information */
    __cpuid(CPUInfo, 0x80000006);
    proc_cache_l2[0]  = CPUInfo[0];
    proc_cache_l2[1]  = CPUInfo[1];
    proc_cache_l2[2]  = CPUInfo[2];
    proc_cache_l2[3]  = CPUInfo[3];

  case 0x80000005:
    /* L1 Cache Information */
    __cpuid(CPUInfo, 0x80000005);
    proc_cache_l1[0]  = CPUInfo[0];
    proc_cache_l1[1]  = CPUInfo[1];
    proc_cache_l1[2]  = CPUInfo[2];
    proc_cache_l1[3]  = CPUInfo[3];

  case 0x80000004:
    /* Processor Name Part 3 */
    __cpuid(CPUInfo, 0x80000004);
    *(int*)(&proc_namestr[32])  = CPUInfo[0];
    *(int*)(&proc_namestr[36])  = CPUInfo[1];
    *(int*)(&proc_namestr[40])  = CPUInfo[2];
    *(int*)(&proc_namestr[44])  = CPUInfo[3];

  case 0x80000003:
    /* Processor Name Part 2 */
    __cpuid(CPUInfo, 0x80000003);
    *(int*)(&proc_namestr[16])  = CPUInfo[0];
    *(int*)(&proc_namestr[20])  = CPUInfo[1];
    *(int*)(&proc_namestr[24])  = CPUInfo[2];
    *(int*)(&proc_namestr[28])  = CPUInfo[3];

  case 0x80000002:
    /* Processor Name Part 1 */
    __cpuid(CPUInfo, 0x80000002);
    *(int*)(&proc_namestr[ 0])  = CPUInfo[0];
    *(int*)(&proc_namestr[ 4])  = CPUInfo[1];
    *(int*)(&proc_namestr[ 8])  = CPUInfo[2];
    *(int*)(&proc_namestr[12])  = CPUInfo[3];

  case 0x80000001:
    /* Extended Feature Info */
    __cpuid(CPUInfo, 0x80000001);
    ext_features[0] = CPUInfo[3];
    ext_features[1] = CPUInfo[1]; 
    ext_features[2] = CPUInfo[2]; 
    break;
  default:
    /* do nothing */
    break;
  }

  return 1;
}

