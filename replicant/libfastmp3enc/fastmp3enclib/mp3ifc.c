/***********************************************************************************************
*                                   MPEG Layer3-Audio Encoder                                  *
*                                                                                              *
*                                 © 1999-2005 by Fraunhofer IIS                                *
*                                       All Rights Reserved                                    *
*                                                                                              *
*   This software and/or program is protected by copyright law and international treaties.     *
*   Any reproduction or distribution of this software and/or program, or any portion of it,    *
*   may result in severe civil and criminal penalties, and will be prosecuted to the           *
*   maximum extent possible under law.                                                         *
*                                                                                              *
*   $Id: mp3ifc.c,v 1.1 2007/05/29 16:02:29 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description:                                                                      *
*                                                                                              *
************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mconfig.h"
#include "mp3ifc.h"
#include "mp3alloc.h"
#include "mp3enc.h"
#include "psy_const.h"
#include "psy_main.h"
#include "bitenc.h"
#include "interface.h"
#include "qc_main.h"
#include "framctrl.h"
#include "utillib.h"
#include "mathlib.h"
#include "time_buffer.h"
#include "stprepro.h"
#include "cpuinfo.h"


struct MP3_ENCODER
{
  /* private */

  ALIGN_16_BYTE struct MP3ENC_CONFIG config;

  struct QC_STATE *qcKernel;
  struct QC_OUT   *qcOut[2]; /* for MPEG-1, we need two qcOut */

  struct PSY_OUT *hpsyOut;
  struct PSY_INTERNAL *hpsy;

  struct FRAME_CONTROL *frameControl;

  struct BITSTREAM_ENCODER *bsEnc;
 
  struct STEREO_PREPRO *hStereoPrePro;

  CHANNEL_ELEMENT * channelElement ;
  
  /*
   * these are constant over the lifetime of one encoder instance
   */
  int hdSiBits ; /* number of bits for header and sideinfo */
  int granPerFrame ;
  int granPerMetaframe ;
  int cbBufsizeMin ;
  int nDelay ;   /* encoder + decoder delay */

  /*
   * these are variable
   */
  float *pScratch ; /* needed for mono bitstreams */
  int granuleCnt ;
  int modeExtension ;
  int bitRateIndex;
  int paddingByte;
  int frameSize;
  int dynbitsInFrame;
  int nZeroesAppended; /* at end-of-file, count the number of zeroes
                          appended after end of client input */

  int ancBitsPerFrame;        /* desired ancillary bits per frame */ 
  int ancRate;                /* ancillary rate */ 
  mp3_ancillary_mode ancMode; /* ancillary mode */
  int needAncBitsGran[2];     /* number of ancillary bits steal from QC */

  int oflOffset; /* begin of ofl data in byte from file begin */

  int noOfChannelConfigurations;
} ;

enum
{
  ERROR_NSAMPLES = 1, /* wrong # of samples in input buffer */
  ERROR_BUFSIZE,      /* output buffer too small */
  ERROR_WRONGVERSION, /* wrong library version */
  ERROR_PANIC,        /* panic: we wrote more bytes than anticipated */
  ERROR_ANCILLARY     /* avoid negativ numAncDataBytes */
};

/* Prototypes Pentium4 processor version */
extern int mp3encOpen_P4
    (
    struct MP3_ENCODER**        phMp3Enc,/* pointer to an encoder handle, initialized on return */
    const struct MP3ENC_CONFIG* pConfig  /* pre-initialized config struct */
    );

extern void mp3encClose_P4
    (
    struct MP3_ENCODER*        hMp3Enc  /* an encoder handle */
    );

extern void mp3encFlush_P4
    (
    struct MP3_ENCODER*        hMp3enc,  /* an encoder handle */
    unsigned char *bitStreamOutData,
    int *outBytes
    );



extern void MP3ENCAPI mp3encDone_P4
    (
    void
    );

extern int mp3encEncode_P4
    (
    struct MP3_ENCODER * const hMp3Enc,  /* an encoder handle */
    const float* const         pSamples, /* BLOCKSIZE*nChannels audio samples, interleaved */
    const int                  nSamples, /* must be equal to BLOCKSIZE*nChannels */
    unsigned char* const       pOutput,  /* pointer to bitstream buffer; receives complete frames only */
    int                        cbSize,   /* the size of the output buffer; must be large enough to receive all data */
    int* const                 cbOut,    /* number of bytes in bitstream buffer */   
    unsigned char             *ancDataBytes,    /* ancillary data to write */
    int                       *numAncDataBytes, /* number of ancillary data in buffer */
    unsigned int              *writeOflOnce     /* write ofl if volitional in first frame */
    );

extern int mp3encGetInfo_P4
    (
    const struct MP3_ENCODER* hMp3Enc,  /* an encoder handle */
    struct MP3ENC_INFO*       pInfo     /* receives information */
    );

extern const int * mp3encGetFrameStats_P4
    (
    const struct MP3_ENCODER* hMp3Enc   /* an encoder handle */
    );

extern int mp3encGetOflOffset_P4
    (
     const struct MP3_ENCODER* hMp3Enc
    );

extern int mp3encGetGranCnt_P4
    (
     const struct MP3_ENCODER* hMp3Enc
    );
extern void mp3encGetDualPassState_P4
    (
     struct MP3_ENCODER*        hMp3enc,  /* an encoder handle */
     void **peStats
    );

/* Prototypes General CPU processor version */
extern int mp3encOpen_GP
    (
    struct MP3_ENCODER**        phMp3Enc,/* pointer to an encoder handle, initialized on return */
    const struct MP3ENC_CONFIG* pConfig  /* pre-initialized config struct */
    );

extern void mp3encClose_GP
    (
    struct MP3_ENCODER*        hMp3Enc  /* an encoder handle */
    );

extern void mp3encFlush_GP
    (
    struct MP3_ENCODER*        hMp3enc,  /* an encoder handle */
    unsigned char *bitStreamOutData,
    int *outBytes
    );

extern void mp3encDone_GP
    (
    void
    );

extern int mp3encEncode_GP
    (
    struct MP3_ENCODER * const hMp3Enc,  /* an encoder handle */
    const float* const         pSamples, /* BLOCKSIZE*nChannels audio samples, interleaved */
    const int                  nSamples, /* must be equal to BLOCKSIZE*nChannels */
    unsigned char* const       pOutput,  /* pointer to bitstream buffer; receives complete frames only */
    int                        cbSize,   /* the size of the output buffer; must be large enough to receive all data */
    int* const                 cbOut,    /* number of bytes in bitstream buffer */   
    unsigned char             *ancDataBytes,    /* ancillary data to write */
    int                       *numAncDataBytes, /* number of ancillary data in buffer */
    unsigned int              *writeOflOnce     /* write ofl if volitional in first frame */
    );

extern int mp3encGetInfo_GP
    (
    const struct MP3_ENCODER* hMp3Enc,  /* an encoder handle */
    struct MP3ENC_INFO*       pInfo     /* receives information */
    );

extern const int * mp3encGetFrameStats_GP
    (
    const struct MP3_ENCODER* hMp3Enc   /* an encoder handle */
    );

extern int mp3encGetOflOffset_GP
    (
    const struct MP3_ENCODER* hMp3Enc
    );

extern int mp3encGetGranCnt_GP
    (
    const struct MP3_ENCODER* hMp3Enc
    );

extern void mp3encGetDualPassState_GP
    (
     struct MP3_ENCODER*        hMp3enc,  /* an encoder handle */
     void **peStats
    );


/* function pointers show the corresponding processor functions */
static int (*mp3encOpenPtr)
     (   
     struct MP3_ENCODER**        phMp3Enc,/* pointer to an encoder handle, initialized on return */
     const struct MP3ENC_CONFIG* pConfig  /* pre-initialized config struct */
     );

static void (*mp3encClosePtr)
     (
     struct MP3_ENCODER*        hMp3enc  /* an encoder handle */
     );

static void (*mp3encFlushPtr)
	(
	struct MP3_ENCODER*        hMp3enc,  /* an encoder handle */
    unsigned char *bitStreamOutData,
    int *outBytes
	);

static int (*mp3encEncodePtr)
     (
      struct MP3_ENCODER * const hMp3enc,  /* an encoder handle */
      const float* const         pSamples, /* BLOCKSIZE*nChannels audio samples, interleaved */
      const int                  nSamples, /* must be equal to BLOCKSIZE*nChannels */
      unsigned char* const       pOutput,  /* pointer to bitstream buffer; contains complete frames only */
      int                        cbSize,   /* the size of the output buffer; must be large enough to receive all data */
      int* const                 cbOut,    /* number of bytes in bitstream buffer */   
      unsigned char             *ancDataBytes,    /* ancillary data to write */
      int                       *numAncDataBytes, /* number of ancillary data in buffer */
      unsigned int              *writeOflOnce     /* write ofl if volitional in first frame */
      );

static int (*mp3encGetInfoPtr)
     (
     const struct MP3_ENCODER* hMp3Enc,  /* an encoder handle */
     struct MP3ENC_INFO*       pInfo     /* receives information */
     );

static const int *(*mp3encGetFrameStatsPtr)
     (
     const struct MP3_ENCODER* hMp3Enc   /* an encoder handle */
     );

static int (*mp3encGetOflOffsetPtr)
     (
     const struct MP3_ENCODER* hMp3Enc
     );

static int (*mp3encGetGranCntPtr)
     (
     const struct MP3_ENCODER* hMp3Enc
     );

static void (*mp3encGetDualPassStatePtr)
    (
     struct MP3_ENCODER*        hMp3enc,  /* an encoder handle */
     void **peStats
    );


/*---------------------------------------------------------------------------

    functionname:mp3encInit
    description: allocate global resources for the layer-3 library
    returns:     MP3ENC_OK if success

  ---------------------------------------------------------------------------*/

int MP3ENCAPI mp3encInit
    (
    unsigned long needVersion, /* pass MP3ENC_VERSION to check compatibility */
    unsigned long *pVersion    /* contains library version on exit */
    )
{
  InitMathOpt();
#if 1
	/* select correct Processor code */
	if (GetCPUInfo(HAS_CPU_SSE2)) {
		mp3encOpenPtr = mp3encOpen_P4;
		mp3encClosePtr = mp3encClose_P4;
		mp3encFlushPtr = mp3encFlush_P4;
		mp3encEncodePtr = mp3encEncode_P4;
		mp3encGetInfoPtr = mp3encGetInfo_P4;
		mp3encGetFrameStatsPtr = mp3encGetFrameStats_P4;
		mp3encGetOflOffsetPtr = mp3encGetOflOffset_P4;
		mp3encGetGranCntPtr = mp3encGetGranCnt_P4;
		mp3encGetDualPassStatePtr = mp3encGetDualPassState_P4;
	} else	
#endif
	{
		mp3encOpenPtr = mp3encOpen_GP;
		mp3encClosePtr = mp3encClose_GP;
		mp3encFlushPtr = mp3encFlush_GP;
		mp3encEncodePtr = mp3encEncode_GP;
		mp3encGetInfoPtr = mp3encGetInfo_GP;
		mp3encGetFrameStatsPtr = mp3encGetFrameStats_GP;
		mp3encGetOflOffsetPtr = mp3encGetOflOffset_GP;
		mp3encGetGranCntPtr = mp3encGetGranCnt_GP;
		mp3encGetDualPassStatePtr = mp3encGetDualPassState_GP;
	}

	if (pVersion) *pVersion = MP3ENC_VERSION;
		return (MP3ENC_VERSION != needVersion) ? ERROR_WRONGVERSION : MP3ENC_OK;
}

/*---------------------------------------------------------------------------

    functionname:mp3encInitDefaultConfig
    description: initializes the MP3ENC_CONFIG to zero
    returns:     void

  ---------------------------------------------------------------------------*/

void MP3ENCAPI mp3encInitDefaultConfig(struct MP3ENC_CONFIG* pConfig)
{
  memset(pConfig, 0, sizeof(struct MP3ENC_CONFIG));
}


/*---------------------------------------------------------------------------

    functionname:mp3encOpen
    description: allocate and initialize a new encoder instance
    returns:     L3ENC_OK if success

  ---------------------------------------------------------------------------*/
int  MP3ENCAPI mp3encOpen
    (
    struct MP3_ENCODER**        phMp3Enc,/* pointer to an encoder handle, initialized on return */
    const struct MP3ENC_CONFIG* pConfig  /* pre-initialized config struct */
    )
{
#if 1
	/* select correct Processor code */
	if (GetCPUInfo(HAS_CPU_SSE2)) {
		mp3encOpenPtr = mp3encOpen_P4;
		mp3encClosePtr = mp3encClose_P4;
		mp3encFlushPtr = mp3encFlush_P4;
		mp3encEncodePtr = mp3encEncode_P4;
		mp3encGetInfoPtr = mp3encGetInfo_P4;
		mp3encGetFrameStatsPtr = mp3encGetFrameStats_P4;
		mp3encGetOflOffsetPtr = mp3encGetOflOffset_P4;
		mp3encGetGranCntPtr = mp3encGetGranCnt_P4;
		mp3encGetDualPassStatePtr = mp3encGetDualPassState_P4;
	} else	
#endif
	{
		mp3encOpenPtr = mp3encOpen_GP;
		mp3encClosePtr = mp3encClose_GP;
		mp3encFlushPtr = mp3encFlush_GP;
		mp3encEncodePtr = mp3encEncode_GP;
		mp3encGetInfoPtr = mp3encGetInfo_GP;
		mp3encGetFrameStatsPtr = mp3encGetFrameStats_GP;
		mp3encGetOflOffsetPtr = mp3encGetOflOffset_GP;
		mp3encGetGranCntPtr = mp3encGetGranCnt_GP;
		mp3encGetDualPassStatePtr = mp3encGetDualPassState_GP;
	}
		
	return (mp3encOpenPtr(phMp3Enc, pConfig));
}

/*---------------------------------------------------------------------------

    functionname:mp3encGetOflOffset
    description: 
    returns:

  ---------------------------------------------------------------------------*/

int MP3ENCAPI mp3encGetOflOffset(const struct MP3_ENCODER* hMp3Enc) 
{	
  return (mp3encGetOflOffsetPtr(hMp3Enc));
}

/*---------------------------------------------------------------------------

    functionname:mp3encGetGranCnt
    description: 
    returns:

  ---------------------------------------------------------------------------*/

int MP3ENCAPI mp3encGetGranCnt(const struct MP3_ENCODER* hMp3Enc) 
{	
  return (mp3encGetGranCntPtr(hMp3Enc));
}

/*---------------------------------------------------------------------------

    functionname:mp3encClose
    description: deallocate an encoder instance

  ---------------------------------------------------------------------------*/

void MP3ENCAPI mp3encClose
    (
    struct MP3_ENCODER*        hMp3enc  /* an encoder handle */
    )
{
	mp3encClosePtr(hMp3enc);
}




/*---------------------------------------------------------------------------

    functionname: mp3encFlush
    returns:      calls BSFlush

  ---------------------------------------------------------------------------*/

void MP3ENCAPI mp3encFlush
    (
    struct MP3_ENCODER*        hMp3enc,  /* an encoder handle */
    unsigned char *bitStreamOutData,
    int *outBytes
    )

{
	mp3encFlushPtr(hMp3enc, bitStreamOutData, outBytes );
}

/*---------------------------------------------------------------------------

    functionname:mp3encDone
    description: release all global resources

  ---------------------------------------------------------------------------*/

void MP3ENCAPI mp3encDone
    (
    void
    )
{
  /* nothing to do */
}


/*---------------------------------------------------------------------------

    functionname:mp3encGetDualPassState
    description: get the dual pass state

  ---------------------------------------------------------------------------*/

void MP3ENCAPI mp3encGetDualPassState
    (
     struct MP3_ENCODER*        hMp3enc,  /* an encoder handle */
     void **peStats
    )
{
		mp3encGetDualPassStatePtr(hMp3enc,peStats);
}



/* sanity check */
#if MP3ENC_BLOCKSIZE != FRAME_LEN_LONG
#error this code only works for MP3ENC_BLOCKSIZE == FRAME_LEN_LONG
#endif

/*---------------------------------------------------------------------------

    functionname:mp3encEncode
    description: encode BLOCKSIZE*nChannels samples
    returns:     MP3ENC_OK on success

  ---------------------------------------------------------------------------*/

int  MP3ENCAPI mp3encEncode
    (
    struct MP3_ENCODER * const hMp3enc,  /* an encoder handle */
    const float* const         pSamples, /* BLOCKSIZE*nChannels audio samples, interleaved */
    const int                  nSamples, /* must be equal to BLOCKSIZE*nChannels */
    unsigned char* const       pOutput,  /* pointer to bitstream buffer; contains complete frames only */
    int                        cbSize,   /* the size of the output buffer; must be large enough to receive all data */
    int* const                 cbOut,    /* number of bytes in bitstream buffer */   
    unsigned char             *ancDataBytes,    /* ancillary data to write */
    int                       *numAncDataBytes, /* number of ancillary data in buffer */
    unsigned int              *writeOflOnce     /* write ofl if volitional in first frame */
    )
{
  return (mp3encEncodePtr(hMp3enc,
                          pSamples,
                          nSamples,
                          pOutput,
                          cbSize,
                          cbOut,
                          ancDataBytes,
                          numAncDataBytes, 
                          writeOflOnce));
}

/*---------------------------------------------------------------------------

    functionname:mp3encGetInfo
    description: get information about the encoding process
    returns:     MP3ENC_OK on success

  ---------------------------------------------------------------------------*/

int  MP3ENCAPI mp3encGetInfo
    (
    const struct MP3_ENCODER* hMp3Enc,  /* an encoder handle */
    struct MP3ENC_INFO*       pInfo     /* receives information */
    )
{
 	return(mp3encGetInfoPtr(hMp3Enc, pInfo));
}

/*---------------------------------------------------------------------------

    functionname:mp3encGetFrameStats
    description: 
    returns:

  ---------------------------------------------------------------------------*/

const int * MP3ENCAPI mp3encGetFrameStats
    (
    const struct MP3_ENCODER* hMp3Enc   /* an encoder handle */
    )
{
	return(mp3encGetFrameStatsPtr(hMp3Enc));
}
