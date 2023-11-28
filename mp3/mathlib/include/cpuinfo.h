/*****************************************************************************

                       (C) copyright Fraunhofer-IIS (1998-2005)
                               All Rights Reserved

   This software and/or program is protected by copyright law and
   international treaties. Any reproduction or distribution of this
   software and/or program, or any portion of it, may result in severe
   civil and criminal penalties, and will be prosecuted to the maximum
   extent possible under law.

   $Id: cpuinfo.h,v 1.1 2009/04/28 20:17:41 audiodsp Exp $

******************************************************************************/

#ifndef _CPUINFO_H
#define _CPUINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/* at first here should detemined the right CPU type */
#if (defined __ICL || defined WIN32 || defined _M_X64 || (defined __GNUC__ && (defined __i386 || __x86_64) )) /* it's very simple, perhaps to simple ? */
#ifdef _MSC_VER /* Microsoft Visual C specific #pragma */
#pragma message(__FILE__": using X86 architecture")
#endif /* _MSC_VER */
#define X86_PLATFORM
#endif

#ifdef _TMS320C6700
#pragma message(__FILE__": using C67xx architecture")
#define C67XX_PLATFORM
#endif

#ifdef ADSP21060
#pragma message(__FILE__": using 2106x architecture")
#define 2106X_PLATFORM
#endif

#if defined  __ppc__
#define PPC_PLATFORM
#endif



  /* X86 cpu info structure */
#if defined X86_PLATFORM
  typedef struct {
    /* Synthesized values */
    unsigned long cpu_vendor;     /* Manufacturer (returns enum CPU_VENDORS) */
    unsigned long cpu_type;       /* CPU type (return enum CPU_TYPES) */
    unsigned long cpu_vendor_string;  /* CPU vendor name string (return const char *) */
    unsigned long cpu_name_string; /* CPU Processor string (extended functions 0x80000002 - 0x80000004, return const char *) */

    /* Processor Features - returned as boolean values */
    unsigned long has_cpuid;      /* Supports CPUID instruction */
    unsigned long has_fpu;        /* FPU present */
    unsigned long has_vme;        /* Virtual Mode Extensions */
    unsigned long has_debug;      /* Debug extensions */
    unsigned long has_pse;        /* Page Size Extensions */
    unsigned long has_tsc;        /* Time Stamp Counter */
    unsigned long has_msr;        /* Model Specific Registers */
    unsigned long has_pae;        /* Page Address Extensions */
    unsigned long has_mce;        /* Machine Check Extensions */
    unsigned long has_cmpxchg8;   /* CMPXCHG8 instruction */
    unsigned long has_apic;       /* APIC */
    unsigned long has_sysenter;   /* SYSENTER/SYSEXIT instruction */
    unsigned long has_mtrr;       /* Memory Type Range Registers */
    unsigned long has_gpe;        /* Global Paging Extensions */
    unsigned long has_mca;        /* Machine Check Architecture */
    unsigned long has_cmov;       /* CMOV instruction */
    unsigned long has_pat;        /* Page Attribue Table */
    unsigned long has_pse36;      /* PSE36 (Page Size Extensions) */

    unsigned long has_mmx_ext;    /* MMX Extensions */
    unsigned long has_mmx;        /* MMX support */
    unsigned long has_fxsave;     /* FXSAVE/FXRSTOR instruction */

    unsigned long has_3dnow_ext;  /* Extended 3DNow! support */
    unsigned long has_3dnow;      /* 3DNow! support */

    unsigned long has_sse_mmx;    /* SSE MMX support (same as HAS_MMXEXT) */
    unsigned long has_sse;        /* SSE */
    unsigned long has_sse2;       /* SSE2 */
    unsigned long has_sse_fp;     /* SSE FP support */
    unsigned long has_sse3;       /* SSE3 */
    unsigned long has_htt;        /* HTT  Hyper-Threading Technology*/ 

    /* Cache parameters (not all values apply to all cpus) */
    unsigned long cpu_l1_dtlb_assoc;      /* L1 Data Cache TLB Associativity */
    unsigned long cpu_l1_dtlb_entries;    /* L1 Data Cache TLB Entries */
    unsigned long cpu_l1_itlb_assoc;      /* L1 Instruction Cache TLB Associativity (0xff = full associativity) */
    unsigned long cpu_l1_itlb_entries;    /* L1 Instruction Cache TLB Entries */

    unsigned long cpu_l1_edtlb_assoc;     /* Extended (2/4 Mbyte) L1 Data Cache TLB Associativity (0xff = full associativity) */
    unsigned long cpu_l1_edtlb_entries;   /* Extended (2/4 Mbyte) L1 Data Cache TLB Entries */
    unsigned long cpu_l1_eitlb_assoc;     /* Extended (2/4 Mbyte) L1 Instruction Cache TLB Associativity */
    unsigned long cpu_l1_eitlb_entries;   /* Extended (2/4 Mbyte) L1 Instruction Cache TLB Entries */

    unsigned long cpu_l1_dcache_size;     /* L1 Data Cache Size (kbytes) */
    unsigned long CPU_L1_DCACHE_ASSOC;    /* L1 Data Cache Associativity (0xff = full associativity) */
    unsigned long cpu_l1_dcache_lines;    /* L1 Data Cache Lines */
    unsigned long cpu_l1_dcache_lsize;    /* L1 Data Cache Line Size (bytes) */

    unsigned long cpu_l1_icache_size;     /* L1 Instruction Cache Size (kbytes) */
    unsigned long cpu_l1_icache_assoc;    /* L1 Instruction Cache Associativity (0xff = full associativity) */
    unsigned long cpu_l1_icache_lines;    /* L1 Instruction Cache Lines */
    unsigned long cpu_l1_icache_isize;    /* L1 Instruction Cache Line Size (bytes) */

    unsigned long cpu_l2_cache_size;      /* L2 Unified Cache Size (Kbytes) */
    unsigned long cpu_l2_cache_assoc;     /* L2 Unified Cache Associativity (0xf = full associativity) */
    unsigned long cpu_l2_cache_lines;     /* L2 Unified Cache Lines (lines per tag) */
    unsigned long cpu_l2_chache_lsize;    /* L2 Unified Cache Line Size (bytes) */

    unsigned long cpu_l2_dtlb_assoc;      /* L2 Data Cache TLB Associativity */
    unsigned long cpu_l2_dtlb_entries;    /* L2 Data Cache TLB Entries */
    unsigned long cpu_l2_utlb_assoc;      /* L2 Instruction or Unified Cache TLB Associativity (0xf = full associativity) */
    unsigned long cpu_l2_utlb_entries;    /* L2 Instruction or Unified Cache TLB Entries */

    unsigned long cpu_l2_edtlb_assoc;     /* Extended (2/4 Mbyte) L2 Data Cache TLB Associativity (0xf = full associativity) */
    unsigned long cpu_l2_edtlb_entries;   /* Extended (2/4 Mbyte) L2 Data Cache TLB Entries */
    unsigned long cpu_l2_eutlb_assoc;     /* Extended (2/4 Mbyte) L2 Instruction or Unified Cache TLB Associativity */
    unsigned long cpu_l2_eutlb_entries;   /* Extended (2/4 Mbyte) L2 Instruction or Unified Cache TLB Entries */
  } CPU_INFO;

#elif defined PPC_PLATFORM

  /* PPC cpu info structure --- NOT really true -> MODIFY !!!!! */
  typedef struct {
    unsigned long has_altivec;
  } CPU_INFO;

#else

  /* Dummy struct for other platforms without special optimizations */
  typedef struct {
    unsigned long has_dummy;
  } CPU_INFO;

#endif

  typedef enum {
    HAS_CPU_FPU,
    HAS_CPU_MMX,
    HAS_CPU_SSE,
    HAS_CPU_SSE2,
    HAS_CPU_SSE3,
    HAS_CPU_ALTIVEC
  } CPU_INFO_CAPS;

  /* prototypes */
  unsigned long GetCPUInfo (CPU_INFO_CAPS caps);


#ifdef __cplusplus
};
#endif

#endif

