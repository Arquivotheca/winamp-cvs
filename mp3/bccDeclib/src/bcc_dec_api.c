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

#include "getstream.h"
#include "bcc_utils.h"
#include "bcc_dec_api.h"

#include <assert.h>


#ifndef PI
#define PI     3.141592654
#endif



/****************************************************************/

/* Updates the tables for new values of ildscale, ild2itd,
   and CORICTDRANGE (this code was separated from bcc_init to
   allow real-time tuning of these parameters is possible) */

BCC_STAT bcc_parameterupdate(
       char          firstcall,    /* in:  1: first time this function is called, 0: succesive calls */
       int           mode,         /* in:  operation mode */
       float         ildscale,     /* in:  new value for ildscale */
       float         ild2itd,      /* in:  new value for ildscale */
       float         corictdrange, /* in:  new value for amplitude of ICTD modulation */
       float         cordifffactor,/* in:  new value for strength of diffuse sound de-correlation */
       BCC_dparams*  params        /* out: some tables are updated */
)
{
  int   i, j, offset;
  float level, qvalue, qvaluedB;


  params->ildscale      = ildscale;
  params->ild2itd       = ild2itd;
  params->corictdrange  = corictdrange;
  params->cordifffactor = cordifffactor;
  params->levrange      = params->levrange0;


  /* if option -ild2itd is used to determine resulting ICTD range */

  if (ild2itd != ILD2ITD_OFF) {
    params->delrange = (float) fabs(params->levrange*ild2itd);

#ifndef NDEBUG
    if (firstcall)
      printf("ICLD-to-ICTD factor results in ICTD range of %2.1f ms\n\n",
        fabs(params->levrange*ild2itd/(float)params->sfreq*1000.));
#endif
  }

  /* if option -ildscale is used determine ICLD range */

  if (ildscale != 1.) {
#ifndef NDEBUG
	  if (firstcall)
      printf("ICLD scaling factor %2.1f results in ICLD range of %2.1f dB\n\n", ildscale, params->levrange*ildscale);
#endif
    params->levrange = (float) fabs(params->levrange*ildscale);
  }


  /* init table that stores levels for a channel pair corresponding
     to a range of ICLD */
  for (i = 0; i < NLEV; i++) {
    level             = 2.f*i*params->levrange/(float)(NLEV-1) - params->levrange;
    level             = (float) pow(10, .05*level);

    /* for two channels: normalize for more efficiency */

    if (params->npbchan == 2) {
      params->levs[i] = level / ((float) sqrt(1.+level*level));

    /* for more than two channels store squared levels in table */

    } else {
      params->levs[i]    = level;
      params->levspow[i] = level * level;
    }

  }



  /* zero levels for upper frequencies of LF channel */

  params->levs[NLEV]    = .0;
  params->levspow[NLEV] = .0;


  /* table with indices pointing to the ICLD quantizer values in
     the params->levs table */

  if (!(mode & SEP)) {

    offset = (params->cuelevels-1)/2;

    for (i = 0; i < params->cuelevels; i++) {

      /* ICLD quantizer level in dB */

      qvaluedB = (i-offset)*(params->levrange/(float)offset);

      /* relative index corresponding to quantizer values of ICLDs */

      if (params->levrange > 0)
        params->qlevptr[i] = (int) (.5f * NLEV * qvaluedB / (float) params->levrange);
      else
        params->qlevptr[i] = 0;


      /* for debugging compute exact quantizer value */

      qvalue   = (float)  pow(10, .05*qvaluedB);
      if (params->npbchan == 2)
        qvalue   = qvalue / ((float) sqrt(1.+qvalue*qvalue));

    }


    params->qlevptr[params->cuelevels] = NLEV-LEVCENTER;


  } else {

  /* table with indices pointing to the ICLD source values in
     the params->levs table */

#ifndef NDEBUG
      if (firstcall)
        printf("ICLD indices to source ICLD values:\n");
#endif
      for (i = 0; i < params->nsepsrc; i++) {

#ifndef NDEBUG
		  if (firstcall)
          printf("  Source Number %d:\n", i);
#endif
        for (j = 0; j < params->ncues; j++) {

          /* ICLD source value in dB */

          qvaluedB = params->ild[i][j];
          if (qvaluedB > params->levrange)
            qvaluedB = params->levrange;
          if (qvaluedB < -params->levrange)
            qvaluedB = -params->levrange;

          /* relative index corresponding to quantizer values of ICLDs */

          if (params->levrange > 0)
            params->qild[j][i] = (int) (.5f * NLEV * qvaluedB / (float) params->levrange);
          else
            params->qild[j][i] = 0;

          /* for debugging compute exact quantizer value */

          qvalue   = (float) pow(10, .05*qvaluedB);
          qvalue   = qvalue / ((float) sqrt(1.+qvalue*qvalue));

          /* print information */
#ifndef NDEBUG
          if (firstcall) {
            printf("    %2.2fdB (index=%d), ", qvaluedB, params->qild[j][i]);
            printf("table=%2.2f (ideal=%2.2f)\n", params->levs[params->qild[j][i]+LEVCENTER], qvalue);
          }
#endif
		}

      }

#ifndef NDEBUG
      if (firstcall)
        printf("\n");
#endif

  }


#ifndef NDEBUG
  if (firstcall)
    printf("\n\n");
#endif


  return(BCC_OK);
}


/****************************************************************/

/* initializes the binaural multi-channel processor */

BCC_STAT bcc_dinit(
       int          mode,      /* in:  operating mode */
       float        ildscale,  /* in:  icld scaling factor */
       float        ild2itd,   /* in:  icld-to-ictd conversion factor */
       int          cuelevels, /* in:  number of levels for ICLD and ICTD */
       int          cuebits,   /* in:  number of bits for ICLD and ICTD */
       int          corlevels, /* in:  number of levels for de-correlation */
       int          corbits,   /* in:  number of bits for de-correlation */
       int          seplevels, /* in:  number of levels for power (-sep2) */
       float        levrange,  /* in:  ICLD range [dB] */
       float        delrange,  /* in:  ICTD range [samples] */
       int          npbchan,   /* in:  number of channels */
       int          nsepsrc,   /* in:  > 0: number of sources, 0: complex audio signal */
       int          refchan,   /* in:  reference channel for the ilds and itds */
       int          lfchan,    /* in:  -1: no lf channel, >=0 lf channel number */
       int          sfreq,     /* in:  sample-frequency of input */
       int          granPerFrame, /* in: number of granules per frame */
       int          framesize, /* in:  number of input samples available at once */
       int          fftsize,   /* in:  size of the STFFT transform */
       int          winspan,   /* in:  non-zero time span of the transform window */
       int          winshift,  /* in:  time-shift of the transform window of encoder */
       int          wintype,   /* in:  0: Hann, 1: Sine, >1: Hann + Protect */
       float        pwidth,    /* in:  with of one partition in bark */
       int          nosmooth,  /* in:  0: partition smoothing, 1: no partition smoothing */
       int          tsmooth,   /* in:  0/1: disable/enable time smoothing of quantized cues */
       char         hybrid,    /* in:  0=full band BCC, 1: hybrid mode, 2: 5 to 2 mode */
       int          nhpart,    /* in:  hybrid mode: number of partitions without BCC */
       BCC_dparams* params,    /* out: initialized parameters */
       BCC_dstate*  state      /* out: initialized state variables */
)
{
  int   i, j, p, failed, historysize;
  int   specsize, ncues, nspecs, npart, delay;
  float a, b;
  float *in;

  /* basic variables */

  params->framecount    = 0;
  params->mode          = mode;
  params->npbchan       = npbchan;
  params->ncues         = npbchan - 1;
  params->nsepsrc       = nsepsrc;
  params->refchan       = refchan;
  params->lfchan        = lfchan;
  params->sfreq         = sfreq;
  params->granPerFrame  = granPerFrame;
  params->framesize     = framesize;
  params->fftsize       = fftsize;
  params->winspan       = winspan;
  params->winshift      = winshift;
  params->wintype       = wintype;
  params->nspecs        = framesize / winshift;
  params->specsize      = fftsize/2 + 1;
  params->ildscale      = ildscale;
  params->ild2itd       = ild2itd;
  params->corictdrange  = CORICTDRANGE;
  params->cordifffactor = CORDIFFFACTOR;
  params->cuelevels     = cuelevels;
  params->cuebits       = cuebits;
  params->corlevels     = corlevels;
  params->corbits       = corbits;
  params->seplevels     = seplevels;
  params->levrange      = levrange;
  params->levrange0     = levrange;
  params->delrange      = delrange;
  params->timesmoothcues= tsmooth;
  params->hybrid        = hybrid;
  params->nhpart        = 0;
  params->infoChunk.sxFlag = -1;
  if (hybrid == 1)
    params->nhpart        = nhpart;

  params->ninchan = 1;
  if (hybrid == 1)
    params->ninchan = npbchan;
  if (hybrid == 2)
    params->ninchan = 2;

  if(params->mode & HRTF) {
    if (params->npbchan != 2) {
#ifndef NDEBUG
      printf("HRTFs are only supported for 2 playback channels.\n");
#endif
      return(BCC_ILLEGAL_CHANNELS);
    }
  }

  if(params->mode & CROSSTALKCANCEL) {
    if (params->npbchan != 2) {
#ifndef NDEBUG
      printf("Cross-talk cancellation is only supported for 2 playback channels.\n");
#endif
      return(BCC_ILLEGAL_CHANNELS);
    }
  }

  specsize          = params->specsize;
  nspecs            = params->nspecs;
  ncues             = params->ncues;
  historysize       = params->framesize;

  /* time constant tonality estimation */

  params->alpha_ton = 1.f/(TC_TON*sfreq/1000.f/framesize);

  /* smoothing filter time constant if tonality is detected */

  params->tau_flt = 1.f/(TAU_FLT*sfreq/1000.f/framesize);

  /* set memory pointers to NULL */

  params->window       = NULL;
  params->window2      = NULL;
  params->fftcosin[0]  = NULL;
  params->fftcosin[1]  = NULL;
  state->buf            = NULL;
  state->norm           = NULL;
  state->src            = NULL;
  state->cor            = NULL;
  state->pxx_ton      = NULL;
  state->pyy_ton      = NULL;
  state->pxyr_ton     = NULL;
  state->pxyi_ton     = NULL;
  state->re_ton       = NULL;
  state->im_ton       = NULL;
  state->ton          = NULL;

  for (i = 0; i < params->ninchan; i++)
    state->input[i]          = NULL;

  for (i = 0; i < params->npbchan; i++) {
    state->output[i]    = NULL;
    state->re[i]        = NULL;
    state->im[i]        = NULL;
    state->gain[i]      = NULL;
    state->prev_gain[i]      = NULL;
    state->norm5to2[i]  = NULL;

    state->chActive[0][i]=1.f;
    state->chActive[1][i]=1.f;
  }

  for (i = 0; i < params->ncues; i++) {
    state->ld[i]    = NULL;
    state->td[i]    = NULL;
  }

  for (i = 0; i < params->npbchan; i++) {
    state->pow[i]    = NULL;
  }

  /* make history buffer larger if necessary */

  if (fftsize-winshift > historysize) {
    historysize = fftsize-winshift;
  }

  /* for less than 50% overlap */
  if(fftsize < 2*winshift) {
    historysize = framesize; //fftsize-winshift;
  }

  params->historysize = historysize;

  /* initialize partitions and partition smoothing windows */

  failed = 0;

  if(!(i = bcc_makepartitions_(sfreq, fftsize, pwidth, nosmooth == 0, &npart, params->part, params->npwins, NULL)))
    return(BCC_OUT_OF_MEMORY);

  params->pwins = (float*) malloc(i*sizeof(float));
  if (params->pwins == NULL) failed = 1;

  i = bcc_makepartitions_(sfreq, fftsize, pwidth, nosmooth == 0, &npart, params->part, params->npwins, params->pwins);
  params->npart = npart;

  if (failed == 1) {
#ifndef NDEBUG
    printf("func bcc_dinit: memory allocation problem.");
#endif
    return(BCC_OUT_OF_MEMORY);

  }

  /* specified winshift supported ? */

  failed = 0;

  /* for less than 50% overlap */
  if(fftsize < 2*winshift) {
  }
  else {
    if (.5*winspan/winshift != (int)(.5*winspan/winshift))  failed = 1;
    if (1.*framesize/winshift != (int)(framesize/winshift)) failed = 1;
  }

  if (failed == 1) {
#ifndef NDEBUG
    printf("func bcc_init: specified framesize or winshift not supported.");
#endif
    return(BCC_ILLEGAL_FRAMESIZE);
  }

  /* allocate memory */

  failed = 0;


  /* Init delay for Ls/Rs */

  params->maxBackChannelDelaySamples = (int)(0.001f * MAX_BACKDELAY * params->sfreq );

  params->delaySamplesLs = params->maxBackChannelDelaySamples;
  params->delaySamplesRs = params->maxBackChannelDelaySamples;

  params->hModBuffLs = NULL;
  params->hModBuffRs = NULL;

  /* Allocate Modulo buffer for time delay */

  params->hModBuffLs = CreateFloatModuloBuffer( params->framesize +  params->maxBackChannelDelaySamples);
  if ( params->hModBuffLs  == NULL) failed = 1;

  params->hModBuffRs = CreateFloatModuloBuffer( params->framesize +  params->maxBackChannelDelaySamples);
  if ( params->hModBuffRs == NULL) failed = 1;


  /* Init with zeros */
  ZeroFloatModuloBuffer ( params->hModBuffLs, params->maxBackChannelDelaySamples);
  ZeroFloatModuloBuffer ( params->hModBuffRs, params->maxBackChannelDelaySamples);


  /* Set backchannel delay per default to 27 */

  bcc_set_backchannel_delay( params, 27, 27 );

  params->window = (float*) malloc(fftsize*sizeof(float));
  if (params->window == NULL) failed = 1;

  if (params->wintype > 1) {
    params->window2 = (float*) malloc(fftsize*sizeof(float));
    if (params->window2 == NULL) failed = 1;
  }

  params->fftcosin[0] = (float*) malloc(fftsize*sizeof(float));
  if (params->fftcosin[0] == NULL) failed = 1;

  params->fftcosin[1] = (float*) malloc(fftsize*sizeof(float));
  if (params->fftcosin[1] == NULL) failed = 1;

  state->buf = (float*) malloc((unsigned int)bcc_max_((float)fftsize, (float)(2*specsize)) *sizeof(float));
  if (state->buf == NULL) failed = 1;

  state->norm = (float*) malloc(params->npart*sizeof(float));
  if (state->norm == NULL) failed = 1;

  state->src = (char*) malloc(nspecs*npart*sizeof(char));
  if (state->src == NULL) failed = 1;

  state->pxx_ton = (float*) malloc(specsize*sizeof(float));
  if (state->pxx_ton == NULL) failed = 1;

  state->pyy_ton = (float*) malloc(specsize*sizeof(float));
  if (state->pyy_ton == NULL) failed = 1;

  state->pxyr_ton = (float*) malloc(specsize*sizeof(float));
  if (state->pxyr_ton == NULL) failed = 1;

  state->pxyi_ton = (float*) malloc(specsize*sizeof(float));
  if (state->pxyi_ton == NULL) failed = 1;

  state->re_ton = (float*) malloc(specsize*sizeof(float));
  if (state->re_ton == NULL) failed = 1;

  state->im_ton = (float*) malloc(specsize*sizeof(float));
  if (state->im_ton == NULL) failed = 1;

  state->ton = (float*) malloc(nspecs*npart*sizeof(float));
  if (state->ton == NULL) failed = 1;

  for (i = 0; i < params->ninchan; i++) {
    state->input[i] = (float*) malloc((framesize+historysize)*sizeof(float));
    if (state->input[i] == NULL) failed = 1;
  }

  for (i = 0; i < npbchan; i++) {

    /* these arrays are needed for each channel */

    state->output[i] = (float*) malloc((framesize+historysize)*sizeof(float));
    if (state->output[i] == NULL) failed = 1;

    state->re[i] = (float*) malloc(specsize*nspecs*sizeof(float));
    if (state->re[i] == NULL) failed = 1;

    state->im[i] = (float*) malloc(specsize*nspecs*sizeof(float));
    if (state->im[i] == NULL) failed = 1;

    state->gain[i] = (float*) malloc(npart*nspecs*sizeof(float));
    if (state->gain[i] == NULL) failed = 1;

    /* previous gains for tonality smoothing */

    state->prev_gain[i] = (float*) calloc(npart*nspecs*sizeof(float),1);
    if (state->prev_gain[i] == NULL) failed = 1;

    state->norm5to2[i] = (float*) malloc(npart*nspecs*sizeof(float));
    if (state->norm5to2[i] == NULL) failed = 1;
  }

  for (i = 0; i < ncues; i++) {

    /* ncues inter-channel cues are used */

    state->ld[i] = (char*) malloc(nspecs*npart*sizeof(char));
    if (state->ld[i] == NULL) failed = 1;

    if (ild2itd == ILD2ITD_OFF) {
      state->td[i] = (char*) malloc(nspecs*npart*sizeof(char));
      if (state->td[i] == NULL) failed = 1;
    } else {
      state->td[i] = state->ld[i];
    }
  }


  for (i = 0; i < ncues; i++) {
    for( j = 0; j < granPerFrame; j++ ) {
      state->quant_ild[j][i] = (int*) malloc(npart*sizeof(int));
      if (state->quant_ild[j][i] == NULL) failed = 1;

      state->ild_diff[j][i] = (int*) malloc(npart*sizeof(int));
      if (state->ild_diff[j][i] == NULL) failed = 1;
    }

    {
      state->quant_ild_hist[i] = (int*) calloc(npart,sizeof(int));
      if (state->quant_ild_hist[i] == NULL) failed = 1;
      for( p=0; p<npart; p++ ) {
        state->quant_ild_hist[i][p] = (cuelevels-1)/4;
      }
    }
  }


  state->cor = (char*) malloc(nspecs*npart*sizeof(char));
  if (state->cor == NULL) failed = 1;

  for (i = 0; i < params->npbchan; i++) {
    state->pow[i] = (float*) malloc(nspecs*npart*sizeof(float));
    if (state->pow[i] == NULL) failed = 1;
  }


  /* memory allocation successful ? */

  if (failed == 1) {
#ifndef NDEBUG
    printf("func bcc_init: could not allocate memory.\n");
#endif
    return(BCC_OUT_OF_MEMORY);
  }

  /* initialize history variables with zero */

  for (i = 0; i < params->ninchan; i++) {
    in  = state->input[i];
    for (j = 0; j < (framesize+historysize); j++)
      in[j] = .0;
  }

  for (i = 0; i < npbchan; i++) {
    in = state->output[i];
    for (j = 0; j < (framesize+historysize); j++) {
      in[j] = .0;
    }
  }

  /* init variables for tonality estimation */

  for (j = 0; j < npart*nspecs; j++) {
    state->ton[j]  = .0;
  }

  for (j = 0; j < specsize; j++) {
    state->pxx_ton[j]     = 1e-10f;
    state->pyy_ton[j]     = 1e-10f;
    state->pxyr_ton[j]    = 1e-10f;
    state->pxyi_ton[j]    = 1e-10f;
    state->re_ton[j]      = 1e-10f;
    state->im_ton[j]      = 1e-10f;
  }

  /* initialize the transform window */

  params->winfrontzeros    = (fftsize - winspan)/2;

  if (params->wintype != 1)
    a = 2.f*winshift/winspan;
  else
    a = (float) sqrt(2.*winshift/winspan);

  for (i = 0; i < params->winfrontzeros; i++)
    params->window[i] = .0;

  for ( ; i < winspan+params->winfrontzeros; i++) {
      j = i - params->winfrontzeros;
      b =(float) sin(PI*j/winspan);
      if (params->wintype != 1)
        params->window[i] = a * b * b;
      else
        params->window[i] = a * b;
  }

  for ( ; i < fftsize; i++)
    params->window[i] = .0;

  /* for less than 50% overlap */
  if(fftsize < 2*winshift) {

    if (wintype!=1) failed = 1;

    if (failed == 1) {
#ifndef NDEBUG
      printf("wintype not supported for less than 50%% overlap\n");
#endif
      return(BCC_ILLEGAL_FRAMESIZE);
    }
    else {

      unsigned int overlap=2*(winspan-winshift);

      for (i = 0 ; i < (int)overlap/2; i++) {
        b = (float) sin(PI*i/overlap);
        params->window[i] = b;
        b = (float) (sin(PI*(i+overlap/2)/overlap));
        params->window[winspan-overlap/2+i] = b;
      }
      for (i = (int)(overlap/2) ; i < (int)(winspan-overlap/2); i++) {
        params->window[i] = 1.;
      }
    }
  }

  /* initialize protection window (wintype > 1 mode) */

  if (wintype > 1) {
    for (i = 0; i < wintype; i++) {
      params->window2[i] = (float) sin(PI*i/wintype/2.);;
    }
    for (; i < fftsize-wintype; i++) {
      params->window2[i] = 1.;
    }
    for (; i < fftsize; i++) {
      params->window2[i] = params->window2[fftsize-i-1];
    }
  }

  /* initialize the fft */

  init_fft_(fftsize,params->fftcosin);

  /* the delay of the decoder
     (net delay in samples = delay * params->winshift)
     (choose: delay >= winspan/winshift - 1;     ) */

  delay = winspan/winshift - 1;
  params->delay = delay;

  /* initialize tables */

    if (nsepsrc == 0) {
      params->qlevptr = (int*) malloc((params->cuelevels+1)*sizeof(int));
      if (params->qlevptr == NULL) failed = 1;

      params->qdelptr = (int*) malloc(params->cuelevels*sizeof(int));
      if (params->qdelptr == NULL) failed = 1;
    } else {
      params->qlevptr = (int*) malloc(params->nsepsrc*sizeof(int));
      if (params->qlevptr == NULL) failed = 1;

      params->qdelptr = (int*) malloc(params->nsepsrc*sizeof(int));
      if (params->qdelptr == NULL) failed = 1;
    }

    /* memory allocation successful ? */

    if (failed == 1) {
#ifndef NDEBUG
      printf("func bcc_init: could not allocate memory.\n");
#endif
      return(BCC_OUT_OF_MEMORY);
    }

    /* initialize tables for ICLD, ICTD, and COR synthesis */

    return(bcc_parameterupdate(
      1,
      params->mode,
      params->ildscale,
      params->ild2itd,
      params->corictdrange,
      params->cordifffactor,
      params));
}


void bcc_dreset(BCC_dparams* params, BCC_dstate*  state)
{
  float *tmp;
  int i, j = 0;

  /* reset history variables with zero */
  for (i = 0; i < params->ninchan; i++) 
    {
      tmp  = state->input[i];
      for (j = 0; j < (params->framesize+params->historysize); j++)
        tmp[j] = .0;
    }

  for (i = 0; i < params->npbchan; i++) 
    {
      tmp = state->output[i];
      for (j = 0; j < (params->framesize+params->historysize); j++)
        tmp[j] = .0;

      /* Previous gains for tonality smooting */
      tmp = state->prev_gain[i];
      for (j = 0; j < ( params->npart * params->nspecs ); ++j)
        tmp[j] = 0.0f; 
    }

  if ( params->hModBuffLs )
    { 
      ResetFloatModuloBuffer( params->hModBuffLs );

      /* Restore actual delay */
      ZeroFloatModuloBuffer ( params->hModBuffLs, params->delaySamplesLs);


      ResetFloatModuloBuffer( params->hModBuffRs );

      /* Restore actual delay */
      ZeroFloatModuloBuffer ( params->hModBuffRs, params->delaySamplesRs);
    }
}



/****************************************************************/

/* release memory in the end */

void bcc_ddone(
       BCC_dparams* params,    /* in: memory pointers */
       BCC_dstate*  state      /* in: memory pointers */
)
{
  int i,j;

  /* free memory */

  freeifnotNULL_(params->window);
  freeifnotNULL_(params->window2);
  freeifnotNULL_(params->fftcosin[0]);
  freeifnotNULL_(params->fftcosin[1]);
  freeifnotNULL_(params->qlevptr);
  freeifnotNULL_(params->pwins);
  freeifnotNULL_(state->src);
  freeifnotNULL_(params->qdelptr);
  freeifnotNULL_(state->buf);
  freeifnotNULL_(state->norm);
  freeifnotNULL_(state->pxx_ton);
  freeifnotNULL_(state->pyy_ton);
  freeifnotNULL_(state->pxyr_ton);
  freeifnotNULL_(state->pxyi_ton);
  freeifnotNULL_(state->re_ton);
  freeifnotNULL_(state->im_ton);
  freeifnotNULL_(state->ton);

  for (i = 0; i < params->ninchan; i++)
    freeifnotNULL_(state->input[i]);

  for (i = 0; i < params->npbchan; i++) {
    freeifnotNULL_(state->output[i]);
    freeifnotNULL_(state->re[i]);
    freeifnotNULL_(state->im[i]);
    freeifnotNULL_(state->gain[i]);
    freeifnotNULL_(state->prev_gain[i]);
    freeifnotNULL_(state->norm5to2[i]);
  }

  for (i = 0; i < params->ncues; i++) {
    if (state->td[i] != state->ld[i])
      freeifnotNULL_(state->td[i]);
    freeifnotNULL_(state->ld[i]);
  }
  for (i = 0; i < params->ncues; i++) {
    for (j = 0; j < params->granPerFrame; j++) {
      freeifnotNULL_(state->quant_ild[j][i]);
      freeifnotNULL_(state->ild_diff[j][i]);
    }
    freeifnotNULL_(state->quant_ild_hist[i]);
  }


  for (i = 0; i < params->npbchan; i++) {
    freeifnotNULL_(state->pow[i]);
  }

  freeifnotNULL_(state->cor);

  if ( params->hModBuffLs )
    {
      DeleteFloatModuloBuffer( params->hModBuffLs );
      params->hModBuffLs = NULL;
    }
  if ( params->hModBuffRs )
    {
      DeleteFloatModuloBuffer( params->hModBuffRs );
      params->hModBuffRs = NULL;
    }

}


/****************************************************************/

/* carries out params->nspecs windowed ffts */

void bcc_dstfft(
       BCC_dparams* params,    /* in:  parameters */
       float*      input,     /* in:  time-domain input samples */
       float*      re,        /* out: real part of spectra */
       float*      im,        /* out: im part of spectra */
       float*      buf        /* buffer of size float[fftsize] */
)
{
  int     i, j;
  float*  samples;

  for (i = 0; i < params->nspecs; i++) {

    /* first sample that is transformed */

    samples = input + params->historysize + (i+1)*params->winshift - params->fftsize;

    /* apply window */

    for (j = 0; j < params->fftsize; j++)
      buf[j] = params->window[j]*samples[j];

    /* fft of windowed samples */
    rff2_(
      buf,
      re + i*params->specsize,
      im + i*params->specsize,
      params->fftsize,
      params->fftcosin);

  }
}


/****************************************************************/

/* carries out params->nspecs inverse windowed ffts */

void bcc_distfft(
       BCC_dparams* params,    /* in:  parameters */
       float*      re,        /* in:  real part of spectra */
       float*      im,        /* in:  im part of spectra */
       float*      output,    /* out: time-domain input samples */
       float*      buf        /* buffer of size float[fftsize] */
)
{
  int i, j;
  float*  samples;

  for (i = 0; i < params->nspecs; i++) {

    /* ifft of spectrum */
    irff2_(
      re + i*params->specsize,
      im + i*params->specsize,
      buf,
      params->fftsize,
      params->fftcosin);

    /* first time-domain sample that is transformed back */

    samples = output + params->historysize + (i+1)*params->winshift - params->fftsize;

    /* no window (Hann analysis/no window synthesis, wintype == 0) */

    /* apply window (sine analysis/sine synthesis, wintype == 1) */

    if (params->wintype == 1)
      for (j = 0; j < params->fftsize; j++)
        buf[j] = params->window[j]*buf[j];

    /* apply window (Hann analysis/protection synthesis, wintype == 1) */

    if (params->wintype > 1)
      for (j = 0; j < params->fftsize; j++)
        buf[j] = params->window2[j]*buf[j];

    /* overlap with previous window */

    for (j = 0; j < params->fftsize-params->winshift; j++)
      samples[j] += buf[j];

    /* non-overlapping part */

    for (; j < params->fftsize; j++)
      samples[j] = buf[j];
  }
}


/****************************************************************/

/* modifies the level of a channel according to given ICLD and coherence */

void bcc_icldgains(
       BCC_dparams* params,     /* in:     parameters */
       int         chan,        /* in:     channel number */
       int*        table,       /* in:     lookup table for the lds */
       char*       ld,          /* in:     partition level difference */
       float*      lev,         /* in:     partition level modification (only -sep2 mode) */
       float*      chgains,     /* out:    non-normalized gain factors for channel */
       float*      chgainsPrev, /* in/out: non-normalized gain factors of previous frame (for smoothing) */
       float*      ton,         /* in:     partition tonality (for smoothing) */
       float*      norm         /* in/out: computes the normalization constant
                                         for npbchan > 2 */
)
{
    int   p, start, offset;
    float gain;
    float *levs, *gains;

    levs        = params->levs;
    gains       = params->gain;

    /* initialize array for normalization constants */

    if (params->npbchan > 2 && chan == 0) {
      for (p = 0; p < params->npart; p++)
        norm[p] = .0;
    }

    /* process all partitions */

    for (p = 0; p < params->nhpart; p++) {
      chgains[p] = 1.;
    }
    for ( ; p < params->npart; p++) {

      gain = 1.;
      if (lev != NULL && ld != NULL) {
        if (table == NULL) gain = lev[p];
        else               gain = gains[(int)ld[p]];
      }

      /* start index for levels table */

      if (ld != NULL) {
        if (table != NULL) offset = table[(int)ld[p]];
        else               offset = ld[p];
      } else offset = 0;

      /* for two-channels: apply levels symmetrically */

      if (params->npbchan == 2) {
        if (chan == 0)
          start = LEVCENTER - offset;
        else
          start = LEVCENTER + offset;
      } else {

      /* for more than two channels: same processing for each channel
         (except reference channel) */

          start = LEVCENTER + offset;
      }


      /* get the factor in dB for smoothing */

      if (ld) {
        chgains[p] = (float) params->qlevptr[(int)ld[p]];
      }
      else {
        /* always 0 dB for reference channel */
        chgains[p] = 0.f;
      }

      /* smoothing if tonality is detected */
      if ( ton ) {

        if ( ton[p] > TON_THRESHOLD ) {
          chgains[p] = ( 1 - params->tau_flt)* chgainsPrev[p] + params->tau_flt * chgains[p];
        }
        /* save the factor for next frame (in dB!!) */
        chgainsPrev[p] = chgains[p];
      }

      /* revert the factor to linear scale */

      /* chgains[p] = pow(10, .05*chgains[p]); */

      /* now read values from table !!!! */

      chgains[p] = params->levs[(int)floor(chgains[p]+0.5f)+LEVCENTER];

      /* LFE upper partitions use ild==cuelevels to indicate that they
           should be scaled to zero, see bcc_sideinfo_dec() */

      if (ld && ld[p] >= params->cuelevels) {
        chgains[p] = 0.f;
      }

      /* sum up the powers for computing normalization constant */

      if (params->npbchan > 2) {
        /* norm[p] += params->levspow[start]; */

        /* use smoothed powers for normalization (can't read from table anymore!) */

        norm[p] += chgains[p]*chgains[p];
      }

    }

    /* compute normalization constant */

    if (params->npbchan > 2 && chan == params->npbchan-1) {
      for (p = 0; p < params->nhpart; p++)
        norm[p] = 1.;
      for ( ; p < params->npart; p++)
        norm[p] = (float) (1.f / sqrt(norm[p]));
    }
}



/****************************************************************/

/* multiplies gain factors with factor for each partition */

void bcc_multgains(
       int          npart,    /* in:     number of partitions */
       float*       factors,  /* in:     factors to scale partitions */
       float*       gains     /* in/out: gain factors to be multiplied */
)
{
    int   p;

    for (p = 0; p < npart; p++)
      gains[p] *= factors[p];
}




/****************************************************************/

/* processes one frame of multi-channel audio */

BCC_STAT bcc_dprocessframe(
       BCC_dparams* params,     /* in:     parameters */
       BCC_dstate*  state,      /* in/out: state variables */
       void*       input,       /* in:     input audio */
       void*       output,      /* out:    output processed audio */
#ifndef NO_MP3S_ADM
       float       upmixGain,
#endif
       char        interleaved, /* in:     1: input interleaved, 0: not interleaved */
       char        inisfloat,   /* in:     1: input is float, 0: input is short */
       char        outisfloat   /* in:     1: output is float, 0: output is short */
)
{
  int     i, j, k, l, offset;
  int     mode, nspecs, npart, framesize, specsize, npbchan, fftsize;
  int     historysize, winshift, winfrontzeros, nsepsrc, delay;
  float   a, sc, leftR, leftI, rightR, rightI, scalelimit, normsum;
  float   *in, *out, *re, *im, *pw, *pw1, *pw2;
  short   *sinput, *soutput;
  float   *finput, *foutput, *upmixL, *upmixR;
  char    *icld, *ictd, *cor;

  float    fChActiveScale=1.f;


  /* Parameters for stereo to 5.0 upmix (hybrid==2 mode)
     (Assumed channel assignment: 0=L, 1=R, 2=C, 3=SL, 4=SR,
      if channels assignment is changed, also decoder needs
      modification!) */
  float upmixL_50[] = {1.0, 0.0, 1.0, 1.0, 0.00};
  float upmixR_50[] = {0.0, 1.0, 1.0, 0.00, 1.0};
  /* Parameters for stereo to 5.1 upmix (hybrid==2 mode)
     (Assumed channel assignment: 0=L, 1=R, 2=C, 3=LF, 4=SL,
      5=SR, if channel assignment is changed, also decoder
      needs modification!) */
  float upmixL_51[] = {1.0, 0.0, 1.0, 1.0, 1.0, 0.00};
  float upmixR_51[] = {0.0, 1.0, 1.0, 1.0, 0.00, 1.0};

  /* choose corresponding upmix data */

  if ((params->hybrid == 2) && (params->lfchan == -1) && (params->npbchan == 5)) {
    upmixL = upmixL_50;
    upmixR = upmixR_50;
  }

  if ((params->hybrid == 2) && (params->lfchan != -1) && (params->npbchan == 6)) {
    upmixL = upmixL_51;
    upmixR = upmixR_51;
  }

  /* frequently used variables */

  mode          = params->mode;
  nspecs        = params->nspecs;
  npart         = params->npart;
  framesize     = params->framesize;
  historysize   = params->historysize;
  winshift      = params->winshift;
  winfrontzeros = params->winfrontzeros;
  fftsize       = params->fftsize;
  specsize      = params->specsize;
  delay         = params->delay;
  npbchan       = params->npbchan;
  nsepsrc       = params->nsepsrc;


  /* shift the old input and output data
   *************************************/

  for (i = 0; i < params->ninchan; i++) {
    in   = state->input[i] + framesize;
    out  = state->input[i];
    for (j = 0; j < historysize-winfrontzeros; j++) {
      out[j]  = in[j];
    }
  }

  for (i = 0; i < npbchan; i++) {
    in   = state->output[i] + framesize;
    out  = state->output[i];
    for (j = 0; j < historysize; j++) {
      out[j] = in[j];
    }
  }


  /* get the new input data
   ************************/

  if (inisfloat == 0) {
    if (interleaved == 0) {
      for (i = 0; i < params->ninchan; i++) {
        in     = state->input[i] + params->historysize - params->winfrontzeros;
        sinput = (short*) input;
        sinput += i*params->framesize;
        for (j = 0; j < params->framesize; j++) {
          in[j]  = sinput[j];
        }
      }
    } else {
      for (i = 0; i < params->ninchan; i++) {
        in     = state->input[i] + params->historysize - params->winfrontzeros;
        sinput = (short*) input;
        sinput += i;
        for (j = 0; j < params->framesize; j++) {
          in[j]  = sinput[j*params->ninchan];
        }
      }
    }
  } else {
    if (interleaved == 0) {
      for (i = 0; i < params->ninchan; i++) {
        in     = state->input[i] + params->historysize - params->winfrontzeros;
        finput = (float*) input;
        finput += i*params->framesize;
        for (j = 0; j < params->framesize; j++) {
          in[j]  = finput[j];
        }
      }
    } else {
      for (i = 0; i < params->ninchan; i++) {
        in     = state->input[i] + params->historysize - params->winfrontzeros;
        finput = (float*) input;
        finput += i;
        for (j = 0; j < params->framesize; j++) {
          in[j]  = finput[j*params->ninchan];
        }
      }
    }
  }


  /* windowed ffts for input
   *************************/

  for (i = 0; i < params->ninchan; i++) {
    bcc_dstfft(
      params,
      state->input[i],
      state->re[i],
      state->im[i],
      state->buf);
  }

  /* tonality estimation for sum signal
   * (later tonality is used for determining
   *  the degree of smoothing)
   *****************************************/

  if (params->timesmoothcues == 1) {

    /* compute the partition correlations */

    if (params->hybrid < 2) {
      for (j = 0; j < params->nspecs; j++) {

        /* sum signal is stored here */

        re = state->re[0] + j*params->specsize;
        im = state->im[0] + j*params->specsize;

        /* compute tonality */

        bcc_partton(
                    /*params->specsize,*/
                    params->npart,
                    params->part,
                    params->alpha_ton,
                    state->re_ton,
                    state->im_ton,
                    re,
                    im,
                    state->pxx_ton,
                    state->pyy_ton,
                    state->pxyr_ton,
                    state->pxyi_ton,
                    state->ton + j*params->npart);

        /* save spectrum for next iteration */

        for (i = 0; i < params->specsize; i++) {
          state->re_ton[i] = re[i];
          state->im_ton[i] = im[i];
        }
      }
    } else {

      /* compute the sum signal, as sum of left and right transmitted
         signal */

      for (j = 0; j < params->specsize*params->nspecs; j++) {
        *(state->re[2]+j) = *(state->re[0]+j) + *(state->re[1]+j);
        *(state->im[2]+j) = *(state->im[0]+j) + *(state->im[1]+j);
      }

      for (j = 0; j < params->nspecs; j++) {

        /* sum signal is stored here */

        re = state->re[2] + j*params->specsize;
        im = state->im[2] + j*params->specsize;

        /* compute tonality */

        bcc_partton(
                    /*params->specsize,*/
                    params->npart,
                    params->part,
                    params->alpha_ton,
                    state->re_ton,
                    state->im_ton,
                    re,
                    im,
                    state->pxx_ton,
                    state->pyy_ton,
                    state->pxyr_ton,
                    state->pxyi_ton,
                    state->ton + j*params->npart);

        /* save spectrum for next iteration */

        for (i = 0; i < params->specsize; i++) {
          state->re_ton[i] = re[i];
          state->im_ton[i] = im[i];
        }
      }
    }
  }

  /* processing for 5to2 BCC mode (hybrid==2, 5 channel surround with
     stereo audio transmission)

     Assumed channel assignment: 0=L, 1=R, 2=C, 3=SL, 4=SR
   **************************************************************/

  if (params->hybrid == 2) {

    /* limit for equalization scale factor */

    scalelimit = (float) pow(10.,.05*SCLIMIT5to2);

    /* normalization for computing "sum" signal */
    normsum = 1.f/(.63f*.63f)/(float)params->npbchan;

    /* apply upmixing */

    for (j = 0; j < specsize*nspecs; j++) {
      leftR  = *(state->re[0]+j);
      rightR = *(state->re[1]+j);
      leftI  = *(state->im[0]+j);
      rightI = *(state->im[1]+j);
      for (k = 2; k < params->npbchan; k++) {
        *(state->re[k]+j) = upmixL[k]*leftR + upmixR[k]*rightR;
        *(state->im[k]+j) = upmixL[k]*leftI + upmixR[k]*rightI;
      }
    }

    /* compute power spectra of upmixed channels */


    for (j = 0; j < params->nspecs; j++) {
      offset = j*specsize;
      for (k = 0; k < params->npbchan; k++)
        bcc_partpow_(
                     /*specsize,*/
                     npart,
                     params->part,
                     state->re[k]  + offset,
                     state->im[k]  + offset,
                     state->pow[k] + j*npart);
    }

    /* normalize left, right, and sum signal such that they all
       have same partition power spectrum as the corresponding
       BCC sum signal would have (this is done such that regular
       BCC cue synthesis can be applied to the spectra afterwards) */

    for (j = 0; j < params->nspecs; j++) {
      offset = j*specsize;
      pw1    = state->pow[0] + offset;
      pw2    = state->pow[1] + offset;
      offset = j*npart;
      for (l = 0; l < params->npbchan; l++) {
        pw = state->pow[l] + j*npart;
        for (k = 0; k < npart; k++) {
          a = normsum * (pw1[k] + pw2[k]);
          sc = (float) sqrt(a/(pw[k]+1e-10f));
          if (sc > scalelimit) sc = scalelimit;
#ifndef NO_MP3S_ADM
	  /* Apply downmix gain after limiting the equalization scale to +10dB,
	     to get full scaling range */
	  if(upmixGain!=0.63f) {
	    sc *= 0.63f/upmixGain;
	  }
#endif
          *(state->norm5to2[l] + offset + k) = sc;
        }
      }
    }
  }



  if ((mode & SEP) && (mode & HRTF)) {
#ifndef NDEBUG
    printf("Flexible rendering and HRTFs not yet implemented!");
#endif
    return(BCC_UNIMPLEMENTED);
  }


  /* Type II BCC Synthesis
   ************************************************************************/

  if (!(mode & ILD) && (mode & ITD)) {
#ifndef NDEBUG
    printf("This BCC decoder does not support bitstreams with only ICTD and no ICLD!\n");
#endif
    return(BCC_UNIMPLEMENTED);
  }

  if (mode & ILD) {
    for (k = 0; k < nspecs; k++) {
      j = 0;
      for (i = 0; i < npbchan; i++) {

        /* select the desired cues */

        if (mode & COR)
          cor = state->cor + k*npart;
        else
          cor = NULL;

        if (params->npbchan == 2) {
          icld = state->ld[0] + k*npart;
          ictd = state->td[0] + k*npart;
        } else {
          if (i != params->refchan) {
            icld = state->ld[j] + k*npart;
            ictd = state->td[j] + k*npart;
            j++;
          } else {
            icld = NULL;
            ictd = NULL;
          }
        }

        /* synthesize ICLD, ICTD, and coherence */

        bcc_icldgains(
          params,
          i,
          params->qlevptr,
          icld,
          NULL,
          state->gain[i] + k*npart,
          state->prev_gain[i] + k*npart,
          state->ton,
          state->norm);
       }

       /* normalize gain factors and scale spectra */

       for (i = 0; i < params->npbchan; i++) {

         /* if more than 2 channels: need for normalizing gain factors */

         if (params->npbchan > 2) {
           bcc_multgains(
             npart,
             state->norm,
             state->gain[i] + k*npart);
         }





         /* consider base channel scale factors for 5to2 mode */

         if (params->hybrid == 2) {
           bcc_multgains(
             npart,
             state->norm5to2[i] + k*npart,
             state->gain[i] + k*npart);
         }

         /* calc smooth scaling if channel is not above noise level
          **********************************************************/
         {
           float fTau= (float)(1.f/(TC_CHAN_ACT*params->sfreq/1000.f/params->framesize)); /* smoothing time constant */

           fChActiveScale=(1.f-fTau)*state->chActive[0][i]+fTau*state->chActive[1][i];

           /* switch channel to active instantly */
           if(state->chActive[0][i]>fChActiveScale) {
             fChActiveScale=state->chActive[0][i];
           }

           state->chActive[1][i]=fChActiveScale;

         }
         /* amplify output such that BCC output level equal to original
            signal level (this scaling is necessary because the gain factors
            above are computed by normalizing with power of sum signal */


	 /* avoid sound problems if fChActiveScale gets too small */
         if (fChActiveScale <= 0.001)
           fChActiveScale = 0.0f;

         a = (float) sqrt(npbchan);
         for (j = params->nhpart; j < npart; j++)
           *(state->gain[i] + k*npart + j) *= a * fChActiveScale;


         /* scale spectral coefficients with smoothing */

         bcc_scalepartitions_(
           npart,
           specsize,
           params->npwins,
           params->pwins,
           state->gain[i] + k*npart,
           state->re[i] + k*specsize,
           state->im[i] + k*specsize,
           state->buf,
           state->buf + specsize);

         /* copy scaled spectrum back */

         for (j = 0; j < specsize; j++) {
           *(state->re[i] + k*specsize + j) = state->buf[j];
           *(state->im[i] + k*specsize + j) = state->buf[j+specsize];
         }
       }
    }
  }



  /* inverse windowed ffts for all channels
   ****************************************/

  for (i = 0; i < npbchan; i++) {
    bcc_distfft(
      params,
      state->re[i],
      state->im[i],
      state->output[i],
      state->buf
    );
  }


  /* Add Ls and Rs to modulo buffer */
  AddFloatModuloBufferValues( params->hModBuffLs, state->output[npbchan - 2], params->framesize );
  AddFloatModuloBufferValues( params->hModBuffRs, state->output[npbchan - 1], params->framesize );



  /* Read Ls and Rs from modulo buffer */
  ReadFloatModuloBufferValues( params->hModBuffLs, state->output[npbchan - 2], params->framesize );
  ReadFloatModuloBufferValues( params->hModBuffRs, state->output[npbchan - 1], params->framesize );


  /* write the output out
   **********************/

  if (outisfloat == 0) {
    if (interleaved == 0) {
      for (i = 0; i < npbchan; i++) {
        out   = state->output[i];
        soutput = (short*) output;
        soutput += i*framesize;
        for (j = 0; j < framesize; j++) {
          a = out[j];

          /* proper rounding */

          if(a > 0) {
            a += 0.5f;
          } else if(a < 0) {
            a -= 0.5f;
          }

          /* proper clippling */

          if (a > SHRT_MAX) {
            a = SHRT_MAX;
            params->clippings++;
          }
          if (a < SHRT_MIN) {
            a = SHRT_MIN;
            params->clippings++;
          }
          soutput[j] = (short)a;
        }
      }
    } else {
      for (i = 0; i < npbchan; i++) {
        out   = state->output[i];
        soutput = (short*) output;
        soutput += i;
        for (j = 0; j < framesize; j++) {
          a = out[j];

          /* proper rounding */

          if(a > 0) {
            a += 0.5f;
          } else if(a < 0) {
            a -= 0.5f;
          }

          /* proper clippling */

          if (a > SHRT_MAX) {
            a = SHRT_MAX;
            params->clippings++;
          }
          if (a < SHRT_MIN) {
            a = SHRT_MIN;
            params->clippings++;
          }
          soutput[j*npbchan] = (short)a;
        }
      }
    }
  } else {
    if (interleaved == 0) {
      for (i = 0; i < npbchan; i++) {
        out   = state->output[i];
        foutput = (float*) output;
        foutput += i*framesize;
        for (j = 0; j < framesize; j++) {
          a = out[j];
          foutput[j] = a;
        }
      }
    } else {
      for (i = 0; i < npbchan; i++) {
        out   = state->output[i];
        foutput = (float*) output;
        foutput += i;
        for (j = 0; j < framesize; j++) {
          a = out[j];
          foutput[j*npbchan] = a;
        }
      }
    }
  }

  /* increment frame counter */

  params->framecount++;

  return(BCC_OK);
}


/****************************************************************/

/* init input bitstream */

void bcc_dinitstream(
       Streamin_state* strm, /* state variables for stream */
       FILE* inputfile
)
{
    initinstream(strm, inputfile);
}


/****************************************************************/

/* init side information decoder */

BCC_STAT bcc_dinitdecoder(
       Streamin_state* strm, /* state variables for stream */
       int   hufftrain,      /* 0: normal operation, 1: Huffman training */
       /* these must be initialized to default values (such as after the call to
          bcc_init, these are only used if params->delaycmp > 0 */
       BCC_dparams* params   /* in:  parameters */
)
{
    if(!initdecoder(
        strm,
        params->npart,
        params->ncues,
        params->cuelevels,
        params->corlevels,
        hufftrain))
        return(BCC_INIT_DECODER);

    return(BCC_OK);
}


/****************************************************************/

/* clean-up input bitstream */

void bcc_dstreamdone(Streamin_state* strm)
{
  instreamdone(strm);
}



/****************************************************************/

/* parse and decode BCC side information */

BCC_STAT bcc_sideinfo_dec(
       Streamin_state* strm,            /* state variables for stream */
       int             bitstreamformat, /* in:      bitstream format flags */
       int             granule,         /* in:      number of granule in current frame */
       unsigned char*  bits,            /* in:      buffer with bits from BCC bitstream (when file is used pass NULL) */
       int*            framebits,       /* in/out:  number of in bits/number of bits consumed (when file is used pass NULL) */
       BCC_dparams*    params,          /* in/out:  parameters */
       BCC_dstate*     state            /* in/out:  state variables */
)
{
  int i, j, k, bitsconsumed, lfchanpos;

  int         cuelevels = params->cuelevels;
  /*int         cuebits   = params->cuebits;*/


  /* copy bytes from bitstream */

  if (bits != NULL && framebits != NULL)
     copyinbuf(strm, bits, (*framebits+7)>>3);

  /* hybrid mode: receive transition partition number */

  if (params->hybrid == 1)
    params->nhpart = getbits(strm, 7);

  /* position of lf channel cues */

  lfchanpos = -1;
  if (params->lfchan >= 0) {
    if (params->refchan < params->lfchan)
      lfchanpos = params->lfchan-1;
    else
      lfchanpos = params->lfchan;
  }

  /* read ilds from the bitstream */

  if (bitstreamformat & ILD ) {
    for (j = 0; j < params->nspecs; j++) {
      for (i = 0; i < params->ncues; i++) {
        if (i != lfchanpos) {
          copy_cues(
            state->quant_ild[granule][i],
            params->npart,
            params->nhpart,
            state->ld[i] + j*params->npart);
        /* for LF channel only code cues for lowest partition */

        } else {
          if (params->nhpart == 0)
            copy_cues(
              state->quant_ild[granule][i],
              1, 0,
              state->ld[i] + j*params->npart);
          /* fill cues at higher frequencies of LF channel
             to cuelevels (corresponds to level zero) */
          for (k = 1; k < params->npart; k++)
            *(state->ld[i] + j*params->npart + k) = cuelevels;
        }
      }
    }
  }

  if (framebits != NULL){
    bitsconsumed = framedone(strm);
    if(bitsconsumed < 0)
      return(BCC_BYTE_ALIGN);
    *framebits = bitsconsumed;
  }
  return(BCC_OK);
}



/****************************************************************/

/* copy bytes into bitstream-in buffer (this can be used as opposed to passing
   bytes to function bcc_sideinfo_dec for every frame) */

void bcc_set_bits(
       Streamin_state* strm,      /* state variables for stream */
       unsigned char*  bits,      /* in:      buffer with bits from BCC bitstream (when file is used pass NULL) */
       int             framebits) /* in/out:  number of in bits/number of bits consumed (when file is used pass NULL) */
{

  if (bits != NULL)
     copyinbuf(strm, bits, (int)(framebits/8.+.99));
}

/****************************************************************/



int bcc_set_backchannel_delay( BCC_dparams* params, unsigned int LsDelay, unsigned int RsDelay )
{
  int moveLsPtr = 0;
  int moveRsPtr = 0;

  /* fill buffers with zeros */
  if ( params->hModBuffLs )
    memset(params->hModBuffLs->paBuffer,0,params->hModBuffLs->size*sizeof(float));
  if ( params->hModBuffRs )
    memset(params->hModBuffRs->paBuffer,0,params->hModBuffRs->size*sizeof(float));

  /* Convert to samples */

   LsDelay = (int)(0.001f * LsDelay * params->sfreq);
   RsDelay = (int)(0.001f * RsDelay * params->sfreq);


  /* Ls channel, check range */

  if ( LsDelay > params->maxBackChannelDelaySamples )
    LsDelay = params->maxBackChannelDelaySamples;


  /* Calculate new position of Ls read pointer */
  moveLsPtr = params->delaySamplesLs - LsDelay;

  if ( moveLsPtr )
    {
      /* Move pointer */
      MoveFloatModuloBufferReadPtr( params->hModBuffLs, moveLsPtr );
      params->delaySamplesLs = LsDelay;
    }

  /* Rs channel, check range */

  if ( RsDelay > params->maxBackChannelDelaySamples )
    RsDelay = params->maxBackChannelDelaySamples;

  /* Calculate new position of Rs read pointer */
  moveRsPtr = params->delaySamplesRs - RsDelay;

  if ( moveRsPtr )
    {
      /* Move pointer */
      MoveFloatModuloBufferReadPtr( params->hModBuffRs, moveRsPtr );
      params->delaySamplesRs = RsDelay;
    }
  return 0;
}


