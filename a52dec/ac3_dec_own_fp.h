/*//////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2002-2007 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __AC3_DEC_OWN_FP_H
#define __AC3_DEC_OWN_FP_H

#include "../tools/staticlib/ipp_px.h"
#include "ipps.h"
#include "ippac.h"
#include "bstream.h"

/* There are some inconsistences between standard and refence realization */
/* basically in the processing of couple channels uder some conditions    */
/* If the following define is set than our decoder will be compatible to  */
/* refenence realization                                                  */
#define REF_DECODER_COMPATIBLE
#define GAINSCALE   65535
#define MAX_CHAN    5

/* Delta bit allocation constants */
enum {
  DELTA_BIT_REUSE,
  DELTA_BIT_NEW,
  DELTA_BIT_NONE,
  DELTA_BIT_RESERVED
};

enum {
  CH_FBW = 1,
  CH_CPL = 2,
  CH_LFE = 4
};

/* audio coding mode */
enum {
  ACMOD_0,
  ACMOD_10,
  ACMOD_20,
  ACMOD_30,
  ACMOD_21,
  ACMOD_31,
  ACMOD_22,
  ACMOD_32
};

/* dual mono downmix mode */
enum {
  DUAL_STEREO,
  DUAL_LEFTMONO,
  DUAL_RGHTMONO,
  DUAL_MIXMONO
};

/* Exponent strategy constants */
enum EXP_STRATEGY {
  EXP_REUSE,
  EXP_D15,
  EXP_D25,
  EXP_D45
};

/* synchronization information */
typedef struct {
  int32_t  fscod;      /* Stream Sampling Rate (kHz) 0 = 48, 1 = 44.1, */
                      /* 2 = 32, 3 = reserved */
  int32_t  frmsizecod; /* Frame size code */
  int32_t  SampleRate;
  int32_t  frame_size; /* Frame size in 16 bit words */
  int32_t  bit_rate;   /* Bit rate in kilobits */
} _SyncInfo;

/* bit stream information */
typedef struct {
  int32_t  bsid;      /* Bit stream identification == 0x8 */
  int32_t  bsmod;     /* Bit stream mode */
  int32_t  acmod;     /* Audio coding mode */
  int32_t  cmixlev;   /* Centre mix level */
  int32_t  surmixlev; /* Surround mix level */
  int32_t  dsurmod;   /* If we're in 2/0 mode then Dolby surround */
                     /* mix level - NOT USED - */
  int32_t  lfeon;     /* Low frequency effects on */
  float  dialnorm;  /* Dialogue normalization */
  int32_t  compre;    /* Compression gain word exists */
  int32_t  compr;     /* Compression gain word */
  int32_t  langcode;  /* Language code exists */
  int32_t  langcod;   /* Language code */
  int32_t  audprodie; /* Audio production info exists */
  int32_t  mixlevel;  /* Mixing level */
  int32_t  roomtyp;   /* Room type */
  float  dialnorm2; /* If we're in dual mono mode (acmod == 0) */
                     /* then extra stuff */
  int32_t  compr2e;   /* compression gain word exists */
  int32_t  compr2;    /* compression gain word */
  int32_t  langcod2e; /* language code exists */
  int32_t  langcod2;  /* language code */
  int32_t  audprodi2e;/* audio production information exists */
  int32_t  mixlevel2; /* mixing level, ch2 */
  int32_t  roomtyp2;  /* room type ch2 */
  int32_t  copyrightb;/* c o p y r i g h t   bit */
  int32_t  origbs;    /* Original bit stream */
  int32_t  timecod1e; /* Timecode 1 exists */
  int32_t  timecod2e; /* Timecode 2 exists */
  int32_t  timecod1;  /* Timecode 1 */
  int32_t  timecod2;  /* Timecode 2*/
  int32_t  addbsie;   /* Additional bit stream info exists */
  int32_t  addbsil;   /* Additional bit stream information */
                     /* length - 1 (in bytes) */
  uint8_t   addbsi[64];
  int32_t  nfchans;   /* Number of channels (excluding LFE) * Derived from acmod */
  int32_t  karaokeMode;
} _BSI;

/* audio block */
typedef struct {
  int32_t  blksw;
  int32_t  dithflag;         /* Dither enable bit indexed by channel num */
  int32_t  dynrnge;          /* Dynamic range gain word exists */
  int32_t  dynrng;           /* Dynamic range gain word */
  int32_t  dynrng2e;         /* If acmod==0 then dynamic range 2 gain exists */
  int32_t  dynrng2;          /* Dynamic range 2 gain */
  int32_t  cplstre;          /* Coupling strategy exists */
  int32_t  cplinu;           /* Coupling in use */
  int32_t  firstCplinu;      /* Coupling in use for the first time? */
  int32_t  chincpl[MAX_CHAN];/* Channel coupled */
  int32_t  firstChincpl[MAX_CHAN];/* Channel coupled for the first time? */
  int32_t  phsflginu;        /* If acmod==2 then phase flags in use */
  int32_t  cplbegf;          /* Coupling begin frequency code */
  int32_t  cplendf;          /* Coupling end frequency code */
  int32_t  cplbndstrc[18];   /* Coupling band structure cplbndstrc[sbnd] */
  int32_t  cplcoe[MAX_CHAN]; /* Coupling coordinares exist cplcoe[ch] */
  int32_t  mstrcplco[MAX_CHAN]; /* Master coupling coordinate */
  float  cplcoord[MAX_CHAN][18]; /* Coupling coordinate */
  int32_t  phsflg[18];       /* Phase flags (only in 2/0 mode) phsflg[bnd] */
  int32_t  rematstr;         /* Rematrixing strategy */
  int32_t  rematflg;         /* Rematrixing flag */
  int32_t  nrematbnds;       /* Number of rematrixing bands */
  int32_t  cplexpstr;        /* Coupling exponent strategy */
  int32_t  chexpstr[MAX_CHAN];/* Channel exponent strategy chexpstr[ch] */
  int32_t  lfeexpstr;        /* Exponent strategy for lfe channel */
  int32_t  chbwcod[MAX_CHAN];/* Channel bandwidth code */
  int32_t  cplabsexp;        /* Absolute coupling exponent */
  int32_t  cplexps[18 * 12 / 3]; /* Coupling channel exponents */
  int32_t  exps[MAX_CHAN][252 / 3 + 1]; /* Fbw channel exponents exps[ch][grp] */
  int32_t  gainrng[MAX_CHAN];/* Channel gain range code gainrng[ch] */
  int32_t  lfeexps[3];       /* Low frequency exponents */
  int32_t  baie;             /* Bit allocation information exists */
  int32_t  sdcycod;          /* Slow decay code */
  int32_t  fdcycod;          /* Fast decay code */
  int32_t  sgaincod;         /* Slow gain code */
  int32_t  dbpbcod;          /* dB per bit code */
  int32_t  floorcod;         /* Masking floor code */
  int32_t  snroffste;        /* SNR offset exists */
  int32_t  csnroffst;        /* Coarse SNR offset */
  int32_t  cplfsnroffst;     /* Coupling fine SNR offset */
  int32_t  cplfgaincod;      /* Coupling fast gain code */
  int32_t  fsnroffst[MAX_CHAN]; /* Fbw fine SNR offset */
  int32_t  fgaincod[MAX_CHAN];  /* Fbw fast gain code */
  int32_t  lfefsnroffst;     /* Lfe fine SNR offset */
  int32_t  lfefgaincod;      /* Lfe fast gain code */
  int32_t  cplleake;         /* Coupling leak initiaization exists */
  int32_t  cplfleak;
  int32_t  cplsleak;         /* Coupling slow leak initialization */
  int32_t  deltbaie;         /* Delta bit allocation information exists */
  int32_t  cpldeltbae;       /* Coupling delta bit allocation exists */
  int32_t  deltbae[MAX_CHAN];/* Fbw delta bit allocation exists */
  int32_t  cpldeltlastbin;
  int32_t  skiple;           /* Skip length exists */
  int32_t  skipl;            /* Skip length */
  int32_t  ncplbnd;          /* Number of combined coupling sub-bands. */
                            /* Derived from ncplsubnd and cplbndstrc */
  int32_t  nchgrps[MAX_CHAN];/* Number of exponent groups by channel. */
                            /* Derived from strmant, endmant */
  int32_t  ncplgrps;         /* Number of coupling exponent groups. */
                            /* Derived from cplbegf, cplendf,cplexpstr */
  int32_t  endmant[MAX_CHAN];/* End mantissa numbers of fbw channels */
  int32_t  cplstrtmant;      /* Start mantissa numbers for the coupling channel */
  int32_t  cplendmant;       /* End mantissa numbers for the coupling channel */
  int32_t  phscor[18];       /* PHSCOR data */
  int32_t  phsoutmod;        /* PHSCOR data */
  int32_t  bitAllocation[MAX_CHAN+1];
  int32_t  CplBitAllocation;
  int32_t  LfeBitAllocation;
} _AudBlk;

typedef struct {
  IppsMDCTInvSpec_32f *pMDCTSpecLong;
  IppsMDCTInvSpec_32f *pMDCTSpecShort;
  uint8_t  *pBufferShort;
  uint8_t  *pBufferLong;
} _AllocationImdct;

typedef struct {
  int32_t m_1_pointer;
  int32_t m_2_pointer;
  int32_t m_4_pointer;

  float *fast_m_1;
  float *fast_m_2;
  float *fast_m_4;

  int16_t dithtemp;
} _MantsTabls;

typedef struct _AC3Dec {
  float  samples[7][512];
  float  coeffs[7][256];
  float  delay[6][256];
  float  cplChannel[256];
  float  fbw_mant[5][256];
  float  cpl_mant[256];
  int32_t  fbw_bap[5][256];
  int32_t  fbw_exp[5][256];
  int32_t  cpl_bap[256];
  int32_t  cpl_exp[256];
  int32_t  lfe_exp[8];
  float  lfe_mant[8];
  int32_t  lfe_bap[8];
  int32_t  deltba[5][64];
  int32_t  cpldeltba[64];
  int32_t  lfedeltba[8];
  float *ShortBuff[2];
  float *temp[6];

  _SyncInfo syncinfo;
  _BSI    bsi;
  _AudBlk audblk;
  _AllocationImdct allocation_imdct;
  _MantsTabls mants_tabls;

  int32_t  nChannelOut;
  int32_t  as_input;
  int32_t  m_frame_number;

  int32_t  dualmonomode;
  int32_t  out_acmod;
  int32_t  outlfeon;
  int32_t  out_compmod;
  int32_t  karaokeCapable;
  int32_t  crc_mute;
  float  drc_scaleLow;
  float  drc_scaleHigh;
  float  gainScale;
};

#ifdef __cplusplus
extern "C" {
#endif

  int32_t BitAllocation(AC3Dec* state);

  void crcInit(AC3Dec *state,
               sBitsreamBuffer *pBS);

  int32_t crcCheck(int32_t num, uint8_t *ptr);

  void  Downmix(int32_t dynrng,
                int32_t dynrng2,
                int32_t before_imdct,
                AC3Dec *state);

  int32_t DecodeExponents(AC3Dec *state);

  void InverseTransform(int32_t before_downmix,
                        AC3Dec *state);

  int32_t InverseTransform_out(AC3Dec *state);

  void  WindowingOverlap(int16_t* pOut,
                         AC3Dec *state);

  int32_t UnpackMantissas(AC3Dec *state,
                         sBitsreamBuffer *pBS);

  int32_t ParseSyncInfo(AC3Dec *state, sBitsreamBuffer *pBS);
  int32_t ParseBsi(AC3Dec *state, sBitsreamBuffer *pBS);
  int32_t ParseAudblk(int32_t nblk, AC3Dec *state, sBitsreamBuffer *pBS);
  int32_t ParseAuxdata(AC3Dec *state, sBitsreamBuffer *pBS, int32_t start_bits);

  void Rematrix(AC3Dec *state);

  AC3Status GetSynch(sBitsreamBuffer *pBS,
                     int32_t inDataSize);

#ifdef __cplusplus
}
#endif

#endif /* __AÑ3_DEC_OWN_FP_H__ */
