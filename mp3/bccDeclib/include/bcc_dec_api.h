/***************************************************************************\
 *                         Fraunhofer IIS 
 *                 (c) 1996 - 2008 Fraunhofer IIS
 *          (c) 2004 Fraunhofer IIS and Agere Systems Inc.
 *                       All Rights Reserved.
 *
 *
 *    This software and/or program is protected by copyright law and
 *    international treaties. Any reproduction or distribution of this
 *    software and/or program, or any portion of it, may result in severe
 *    civil and criminal penalties, and will be prosecuted to the maximum
 *    extent possible under law.
 *
\***************************************************************************/

#ifndef __BCC_DEC_API_H__
#define __BCC_DEC_API_H__


#include "bcc_utils.h"
#include "mod_buf.h"


/* limits [s] for writing diagnostics data */
#define DEBUGSTART  5
#define DEBUGSTOP   8

/* include headers */

#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

/* discard advanced functionality
**********************************/

#define MAX_BACKDELAY 30  /* max 30ms delay for backchannels */


/* audio synthesis modes
   (in bcc_init, mode is passed with one of these values) */

#define ILD              1
#define ITD              2
#define COR              4
#define SEP              8
#define HRTF             16
#define CROSSTALKCANCEL  32

/* constants related to de-correlation */

#define NLEV            49    /* # levels for total ICLD range */
#define LEVCENTER       24    /* = floor(NLEV/2) */
#define NPHASE          1024  /* size of sine and cosine table for ICTD synthesis */
#define NDEL            97    /* # levels for total ICTD range */
#define DELCENTER       48    /* = floor(NDEL/2) */
#define NCORSEPLEVELS   100   /* number of strength values for correlation */
#define CORCHANGE       1500  /* at which frequency [Hz] to change from diffuse sound to ICTD fluctuations */
#define CORICTDRANGE    0e-3  /* range of ICTD spectral fluctuation for de-correlation [s] */
#define CORDIFFFACTOR   1.    /* strength of diffuse sound de-correlation (1 = theoretically correct value) */
#define NSPECMEM        10    /* number of spectra that are stored for generating the diffuse sound */
#define DIFFDECAY       .95   /* decay used when computing diffuse sound */
#define SCLIMIT5to2     10.   /* scaling limit for 5to2 mode [dB] */

/* constants */

#define MAXSOURCES   32           /* maximum number of separated source signals */
#define MAXPBCHAN    8            /* maximum number of playback channels */
#define MAXCUES      MAXPBCHAN-1  /* maximum number of playback channels */
#define ILD2ITD_OFF  1e10         /* value indicating that ild2itd option is off */
#define TC_CHAN_ACT  18.65        /* time constant for channel activity smoothing [ms] */
#define TC_TON       50          /* time constant for tonality estimation [ms] */
#define TAU_FLT      500         /* smoothing filter time constant if tonality is detected */
#define TON_THRESHOLD  .8        /* threshold for tonality detection */

#define MAX_GPF      2            /* maximum number of granules per frame */


typedef enum{

  BCC_OK,
  BCC_ILLEGAL_PARAM,
  BCC_ILLEGAL_CHANNELS,
  BCC_OUT_OF_MEMORY,
  BCC_ILLEGAL_FRAMESIZE,
  BCC_INIT_DECODER,
  BCC_HEADER,
  BCC_CUES,
  BCC_INFO, /*NEW*/
  BCC_BYTE_ALIGN,
  BCC_UNIMPLEMENTED
}BCC_STAT;


/* data structures
 *****************/

typedef struct BCC_dinfo
{
	int		sxFlag;
} BCC_dinfo;

/* data structure for static parameters and data */

typedef struct BCC_dparams
{
       long    framecount;              /* counts the number of processed frames */
       int     clippings;               /* counter for clipping when converting to int16 */
       int     mode;                    /* options */
       int     npbchan;                 /* number of channels */
       int     refchan;                 /* reference channel for the spatial cues */
       int     lfchan;                  /* -1: no lf channel, >=0 lf channel number */
       int     ncues;                   /* number of inter-channel cues */
       int     nsepsrc;                 /* > 0: number of sources, 0: complex audio signal */
       int     sfreq;                   /* sample-frequency of input */
       int     granPerFrame;            /* number of granules per frame */
       int     framesize;               /* number of input samples available at once */
       int     historysize;             /* number of input samples which are stored beyond frame */
       int     fftsize;                 /* size of the transform window (fft size) */
       int     winspan;                 /* non-zero span of the transform window */
       int     winfrontzeros;           /* number of zeros at the beginning of the transform window */
       int     winshift;                /* time-shift of the transform window */
       int     wintype;                 /* in:  0: Hann, 1: Sine, >1: Hann + Protect */
       float*  window;                  /* transform window */
       float*  window2;                 /* memory for protection window */
       int     nspecs;                  /* number of transforms in one frame */
       int     specsize;                /* number of coefficients in one spectrum */
       int     delay;                   /* the system delay is delay*winshift-winfrontzeros */
       int     npart;                   /* number of spectral partitions */
       int     part[MAXPART];           /* partition boundaries */
       int     npwins[MAXPART*3];       /* information for the partition smoothing windows */
       float*  pwins;                   /* memory where the partition smoothing windows are stored */
       int     cuelevels;               /* number of quantizer levels for ICLD and ICTD */
       int     corlevels;               /* number of quantizer levels for correlation */
       int     cuebits;                 /* number of bits for PCM encoding of ICLD and ICTD */
       int     corbits;                 /* number of bits for PCM encoding of ICC */
       int     seplevels;               /* number of quantizer levels for power (-sep2) */
       float   levrange;                /* possible level difference between channels +-levrange dB */
       float   levrange0;               /* possible level difference between channels +-levrange dB */
       float   delrange;                /* possible delays between channels +-delrange samples */
       int     corchange;               /* at which partition to change from diffuse sound to ICTD fluctuations */
       float   corictdrange;            /* amplitude of ICTD modulation over frequency */
       float   cordifffactor;           /* strength of diffuse sound de-correlation */
       float   ildscale;                /* ICLD scaling factor */
       float   ild2itd;                 /* ICLD-to-ICTD conversion factor */
       float   levs[NLEV+1];            /* table with factor corresponding to a range of ICLD */
       float   levspow[NLEV+1];         /* table with power values corresponding to a range of ICLD */
       int     *qlevptr;                /* pointers to the quantizer values */
       int     *qdelptr;                /* pointers to the quantizer values */
       float   ild[MAXSOURCES][MAXCUES];/* ICLDs associated with the sources */
       float   itd[MAXSOURCES][MAXCUES];/* ICTDs associated with the sources */
       float   gain[MAXSOURCES];        /* gains associated with the sources */
       int     qild[MAXCUES][MAXSOURCES];/* indices into the ICLD table */
       int     qitd[MAXCUES][MAXSOURCES];/* indices into the ICTD table */
       float   pild[MAXPBCHAN][MAXSOURCES];/* power distribution between channels for source cues */
       int     hybrid;                  /* hybrid mode: 0=full band BCC, 1: hyrid mode, 2: 5 to 2 mode */
       int     nhpart;                  /* hybrid mode: 0=full band BCC, >1=number of partitions without BCC */
       int     ninchan;                 /* number of input channels (for hybrid mode this is > 1) */
       float*  fftcosin[2];             /* fft constants */
       float   alpha_ton;               /* forgetting factor for tonality estimation */
       float   tau_flt;                 /* smoothing filter time constant if tonality is detected */
       int     timesmoothcues;          /* 1: time smoothing of cues enabled */
       BCC_dinfo infoChunk;

       int     delaySamplesLs;          /* Delay in samples for Ls channel */
       int     delaySamplesRs;          /* Delay in samples for Rs channel */

       HANDLE_MODULO_BUFFER  hModBuffLs;  /* Modulo Buffer for time delay (Ls part ) */
       HANDLE_MODULO_BUFFER  hModBuffRs;  /* Modulo Buffer for time delay (Rs part ) */

       int     maxBackChannelDelaySamples;     /* Max delay in samples for rear channels.
                                                 Calculated once from MAX_BACKDELAY */

} BCC_dparams;

/* data structure for storing the complete state of the
   system */

typedef struct BCC_dstate
{
       /* buffers for quantized cues */
       int*   quant_ild[MAX_GPF][MAXCUES];
       int*   ild_diff[MAX_GPF][MAXCUES];
       int*   quant_ild_hist[MAXCUES];
       int    ioChanActive[MAX_GPF][MAXPBCHAN];


       float* input[MAXPBCHAN];   /* input samples for each channel */
       float* output[MAXPBCHAN];  /* output samples for each channel */
       float* re[MAXPBCHAN];      /* real-values of transformed frame */
       float* im[MAXPBCHAN];      /* im-values of transformed frame */
       char*  ld[MAXCUES];        /* level differences between the channels */
       char*  td[MAXCUES];        /* level differences between the channels */
       char*  cor;                /* correlation estimation between the channels */
       char*  src;                /* index of the source which is dominant in a band */
       float* pow[MAXPBCHAN];     /* power of left right and center (used in hybrid==2 mode) */
       float  chActive[2][MAXPBCHAN];/* indicates whether channel has negligible power */
       float* gain[MAXPBCHAN];    /* gain factor for each channel and partition */
       float* prev_gain[MAXPBCHAN];/* gain factor from previous frame for each channel and partition */
       float* norm5to2[MAXPBCHAN];/* gain factors for base channels for 5to2 mode */
       float* norm;               /* normalization factors of channel power */
       float* buf;                /* only temporary buffer, no state variable ! */

       /* state variables for tonality estimation */

       float* pxx_ton;
       float* pyy_ton;
       float* pxyr_ton;
       float* pxyi_ton;
       float* re_ton;
       float* im_ton;
       float* ton;
} BCC_dstate;


/* function headers
 ******************/

/* BCC signal processing */

BCC_STAT bcc_dinit(int, float, float,
                   int, int ,int, int, int, float, float, int ,int, int,
                   int, int, int, int, int, int, int, int, float, int, int, char, int, BCC_dparams*, BCC_dstate*);
void bcc_ddone(BCC_dparams*, BCC_dstate*);
void bcc_dreset(BCC_dparams*, BCC_dstate*);

BCC_STAT bcc_dprocessframe(BCC_dparams*, BCC_dstate*, void*, void*,
#ifndef NO_MP3S_ADM
			   float,
#endif
			   char, char, char);

BCC_STAT bcc_parameterupdate(char, int,
                             float,
                             float,
                             float, float, BCC_dparams*);

/* bitstream functions */

void bcc_dinitstream(Streamin_state*, FILE*);
BCC_STAT bcc_dinitdecoder(Streamin_state*, int, BCC_dparams*);
BCC_STAT bcc_sideinfo_dec(Streamin_state*, int, int,unsigned char*, int*, BCC_dparams*, BCC_dstate*);
void bcc_dstreamdone(Streamin_state* strm);
void bcc_set_bits(Streamin_state*, unsigned char*, int);


/* internally used functions
 ***************************/
void bcc_dstfft(BCC_dparams*, float*, float*, float*, float*);
void bcc_distfft(BCC_dparams* , float*, float*, float*, float*);

void bcc_icldgains(BCC_dparams*, int, int*, char*,
                   float*, float*, float*, float*, float*);

void bcc_multgains(int, float*, float*);
void bcc_ictdsynth(BCC_dparams*, int, int*, char*, char*, float*, float*);
void bcc_diffusesynth(BCC_dparams* params, char*, float*, float*, float*, float*, float*, float*, float*);


int bcc_set_backchannel_delay( BCC_dparams* params, unsigned int LsDelay, unsigned int RsDelay);

#endif
