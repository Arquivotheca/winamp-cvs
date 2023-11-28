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

#include "bcc_utils.h"

#ifndef PI
#define PI     3.141592654
#endif


/* utility functions
 *******************/

/* arg_ function */

float arg_(float re, float im)
{
      float phase;

      /* phase of complex vector (re,im) */

      if (re >= .0 && im >= .0) {

        /* first quadrant */

		if (re == .0) re = 1e-10f;
        phase = (float)atan(im/re);

      } else

      if (re < .0 && im >= .0) {

        /* second quadrant */

        phase = (float) (PI + atan(im/re));

      } else

      if (re < .0 && im < .0) {

        /* third quadrant */

        phase = (float)(-PI + atan(im/re));

      } else

      if (re >= .0 && im < .0) {

        /* fourth quadrant */

        if (re == .0) re = 1e-10f;
        phase = (float)atan(im/re);

      }

      return phase;
}

/* complex multiplication */

/*inline */
void cmult_(float re1, float im1, float re2, float im2, float* reres, float* imres)
{
    *reres = re1*re2 - im1*im2;
    *imres = re1*im2 + im1*re2;
}

/*inline */
void crosstalkfilt_(
       float re1, float im1,      /* in:  left loudspeaker signal spectral value  */
       float re2, float im2,      /* in:  right loudspeaker signal spectral value */
       float dre, float dim,      /* in:  direct filter spectral value            */
       float cre, float cim,      /* in:  cross filter spectral value             */
       float* ore1, float* oim1,  /* out: left loudspeaker signal spectral value  */
       float* ore2, float* oim2)  /* out: right loudspeaker signal spectral value */
{
    *ore1 = (re1*dre - im1*dim) + (re2*cre - im2*cim);
    *oim1 = (re1*dim + im1*dre) + (re2*cim + im2*cre);
    *ore2 = (re2*dre - im2*dim) + (re1*cre - im1*cim);
    *oim2 = (re2*dim + im2*dre) + (re1*cim + im1*cre);
}

float bcc_min_(float a, float b)
{
  if (a < b) return a;
  return b;
}

float bcc_max_(float a, float b)
{
  if (a > b) return a;
  return b;
}

void freeifnotNULL_(void* mem)
{
  if (mem != NULL)
    free(mem);
  mem = NULL;
}

/* computes the power for each partition */

void bcc_partpow_(
                /*int         specsize,*//* in:     number of spectral coefficients */
                  int         npart,     /* in:     number of partitions */
                  int*        part,      /* in:     partition boundaries */
                  float*      re,        /* in:     real part of spectrum */
                  float*      im,        /* in:     imaginary part of spectrum */
                  float*      pow        /* out:    partition power spectrum */
)
{
  int   i, p, psize;
  float a, b, sum;

  /* compute the power in each partition */

  i = 0;
  for (p = 0; p < npart; p++) {
    sum = 0;
    psize = part[p] - i; /* size of the partition */
    for ( ; i < part[p]; i++) {

      /* sum power of all bins in partition */

      a  = re[i];
      b  = im[i];

      sum += a*a + b*b;
    }
    pow[p] = sum/psize;
  }
}

/* estimates tonality of a signal */

void bcc_partton(
/*     int         specsize, */ /* in:     number of spectral coefficients */
       int         npart,     /* in:     number of partitions */
       int*        part,      /* in:     partition boundaries */
       float       alpha,     /* in:     forgetting factor */
       float*      re,        /* in:     real part of spectrum */
       float*      im,        /* in:     imaginary part of spectrum */
       float*      re2,       /* in:     real part of spectrum */
       float*      im2,       /* in:     imaginary part of spectrum */
       float*      pxx,       /* in/out: state variables */
       float*      pyy,       /* in/out: state variables */
       float*      pxyr,      /* in/out: state variables */
       float*      pxyi,      /* in/out: state variables */
       float*      ton        /* out:    partition correlation estimation */
)
{
  int     p, j;
  float   xas, yas, xyr, xyi, num, den, a, pmax;

  j = 0;
  for (p = 0; p < npart; p++) {
    num  = .0;
    den  = .0;
    for ( ; j < part[p]; j++) {
      xas  = re[j] * re[j] + im[j] * im[j];
      yas  = re2[j] * re2[j] + im2[j] * im2[j];
      xyr  = re[j] * re2[j] + im[j] * im2[j];
      xyi  = im[j] * re2[j] - re[j] * im2[j];

      pxx[j]  = (1-alpha) * pxx[j]  + alpha * xas;
      pyy[j]  = (1-alpha) * pyy[j]  + alpha * yas;
      pxyr[j] = (1-alpha) * pxyr[j] + alpha * xyr;
      pxyi[j] = (1-alpha) * pxyi[j] + alpha * xyi;

      pmax = bcc_max_(pxx[j], pyy[j]);

      a = (float)sqrt(pxyr[j]*pxyr[j]+pxyi[j]*pxyi[j]+1e-20f) / (pmax+1e-20f);

      num += a*(xas+yas);
      den += xas+yas;
    }
    ton[p] = (num+1e-20f)/(den+1e-20f);
  }
}

/* generate Gaussian noise */

#define ranf() ((float) rand() / (float) RAND_MAX)

float ranfGauss_(int m, float s)
{
   static int pass = 0;
   static float y2;
   float x1, x2, w, y1;

   if (pass)
   {
      y1 = y2;
   } else {
      do {
         x1 = 2.0f * ranf() - 1.0f;
         x2 = 2.0f * ranf() - 1.0f;
         w = x1 * x1 + x2 * x2;
      } while (w >= 1.0f) ;

      w = (float)sqrt(-2.0 * log(w) / w);
      y1 = x1 * w;
      y2 = x2 * w;
   }
   pass = !pass;

   return (y1 * s + (float) m);
}

/* byte swap an array of int32 if current machine is big-endian */

void swapint_(int* values, int N) {
      char  *p;
      int   i;
      char  temp[4];
      short s = 0x01 ;

      if (!*((char *) &s)) {
        for (i = 0; i < N; i++) {
          p = (char*)(values+i);

          temp[0]=p[3];
          temp[1]=p[2];
          temp[2]=p[1];
          temp[3]=p[0];

          *(values+i) = *((int*)temp);
        }
      }
}

/* compute partition windows */

int bcc_makepartitions_(
       int    sfreq,    /* in:  sampling frequency */
       int    fftsize,  /* in:  FFT size */
       float  pwidth,   /* in:  partition width [ERB] */
       char   smooth,   /* in:  0: rectangular bands, 1: smoothed bands */
       int*   npart,    /* out: number of partitions */
       int*   part,     /* out: partition boundaries */
       int*   npwins,   /* out: start and lengths of smoothing windows */
       float* pwins     /* out: smoothing windows */
)
{
  int   i, j, k, n, prevn, p, lastpartsize, freqboundn, nwin;
  int   startoffset, winsiden, specsize, winptr, pstart, pstop;
  int   mstart, mstop, startpos, endpos, stopoffset, ptr;
  float mean;
  char  failed;
  float nerb, erbfirstpart, freqbound;
  float *wingen, *win, *factors;

  specsize     = fftsize/2 + 1;

  /* allocate and initialize memory */

  wingen  = NULL;
  win     = NULL;
  factors = NULL;

  if (pwins != NULL) {

    failed  = 0;

    wingen = (float*) malloc(NWINGEN*sizeof(float));
    if (wingen == NULL) failed = 1;

    win = (float*) malloc((int)(WINRELWIDTH*MAXPARTWIDTH*sizeof(float)+.99));
    if (win == NULL) failed = 1;

    factors = (float*) malloc(specsize*sizeof(float));
    if (factors == NULL) failed = 1;

    if (failed == 1) {
#ifndef NDEBUG
      printf("func bcc_makepartitions_: memory allocation problem.\n\n");
#endif
      return(0);
    }
  }

  /* init boundaries for critical bands (pwidth ERB bandwidth) */

  erbfirstpart = 21.4f*(float) log10(.00437f*FIRSTPART+1.f);
  if (erbfirstpart < pwidth)
    erbfirstpart = pwidth;

  nerb         = erbfirstpart;
  p            = 0;
  n            = 0;
  prevn        = 0;
  lastpartsize = 0;
  while (n < specsize) {
    freqbound = (float)((pow(10., nerb / 21.4) - 1.0) / 0.00437);
    n++;
    while ((freqbound>n * sfreq / fftsize) | (n-prevn < MINPART) | (n-prevn < lastpartsize))
      n++;
    lastpartsize = n - prevn;
    if (p == 0) {
      freqbound = (float)((pow(10., pwidth / 21.4) - 1.0) / 0.00437);
      freqboundn = (int)(freqbound/sfreq*fftsize+.5f);
      lastpartsize = freqboundn - prevn;
    }
    prevn = n;
    part[p] = (int)bcc_min_((float)n, (float)specsize);
    p++;
    nerb += pwidth;

    if (p >= MAXPART) {
#ifndef NDEBUG
      printf("Constant MAXPART is too small!\n");
#endif
      return(0);
    }
  }
  *npart = p;
#if 0
  //#ifndef NDEBUG
/* temporary for debugging >>> */
if (pwins != NULL) {
  printf("partitions = \n");
  for (i = 0; i < *npart; i++)
    printf(" %d", part[i]);
  printf("\n");
  printf("npart = %d\n\n", *npart);
}
 /*temporary for debugging <<< */
#endif
  /* computed non-smoothed windows */

  if (smooth == 0) {
    winptr = 0;
    pstart = 0;
    for (j = 0; j < *npart; j++) {
      pstop = part[j];

      /* save start of window data for partition */

      npwins[j*3] = pstart;

      /* save window length for partition */

      npwins[j*3+1] = pstop-pstart;

      /* save start point in window data array for window */

      npwins[j*3+2] = winptr;

      /* save window in window data array */

      for (i = 0; i < pstop-pstart; i++) {
        if (pwins != NULL) pwins[winptr] = 1;
        winptr++;
      }
      pstart = pstop;
    }
  }

  /* compute smoothed partition windows */

  if (smooth == 1) {

    /* init generic window with high resolution */

    if (wingen != NULL) {
      for (i = 0; i < NWINGEN/4; i++)
        wingen[i] = .0;
      j = 1;
      for ( ; i < NWINGEN*3/4; i++) {
        wingen[i] =(float)(sin(j*PI/NWINGEN*2.)*sin(j*PI/NWINGEN*2.));
        j++;
      }
      for ( ; i < NWINGEN; i++)
        wingen[i] = 0;
    }

    /* compute window for each partition */

    winptr = 0;
    pstart = 0;
    for (i = 0; i < specsize; i++) {
      if (factors != NULL) factors[i] = .0;
    }
    for (j = 0; j < *npart; j++) {
      pstop = part[j];

      /* length of window */

      nwin = (int)(WINRELWIDTH*(pstop-pstart));

      /* compute window */

      if ((pstop-pstart) % 2 == 0) {
        for (i = 0; i < nwin; i++) {

          /* compute the mean of the window range corresponding
             to the target STFT bin (this is necessary at low frequencues
             where one bin corresponds to a large part of the window */

          mstart = (int)(i*NWINGEN/2./nwin+.5 + NWINGEN/4);
          mstop  = (int)((i+1)*NWINGEN/2./nwin+.5 + NWINGEN/4);
          mean = .0;
          for (k = mstart; k < mstop; k++)
            if (wingen != NULL) mean += wingen[k];
          mean /= (float) (mstop-mstart);

          /* window value for bin is the computed mean */

          if (win != NULL) win[i] = mean;
        }
      } else {
        for (i = 0; i <= nwin; i++) {

          /* compute the mean of the window range corresponding
             to the target STFT bin (this is necessary at low frequencues
             where one bin corresponds to a large part of the window */

          mstart = (int)(i*NWINGEN/2./nwin+.5 + NWINGEN/4. - NWINGEN/nwin/4.);
          mstop  = (int)((i+1)*NWINGEN/2./nwin+.5 + NWINGEN/4. - NWINGEN/nwin/4.);
          mean = .0;
          for (k = mstart; k < mstop; k++)
            if (wingen != NULL) mean += wingen[k];
          mean /= (float) (mstop-mstart);

          /* window value for bin is the computed mean */

          if (win != NULL) win[i] = mean;
        }
        nwin++;
      }

      /* make sure window does not go out of range of spectrum */

      winsiden = (nwin - pstop + pstart) / 2;

      startpos    = pstart - winsiden;
      startoffset = 0;
      if (startpos < 0) {
        startoffset = -startpos;
        startpos    = 0;
      }

      endpos     = pstop + winsiden;
      stopoffset = 0;
      if (endpos > specsize) {
        stopoffset = endpos - specsize;
        endpos     = specsize;
      }

      /* save start of window data for partition */

      npwins[j*3] = startpos;

      /* save window length for partition */

      npwins[j*3+1] = endpos - startpos;

      /* save start point in window data array for window */

      npwins[j*3+2] = winptr;

      /* save window in window data array */

      for (i = startoffset; i < nwin-stopoffset; i++) {
        if (pwins != NULL) pwins[winptr] = win[i];
        winptr++;
      }
      pstart = pstop;

      /* superimpose all windows (for normalization later) */

      for (i = startpos; i < endpos; i++) {
        if (factors != NULL) factors[i] += win[i-startpos+startoffset];
      }
    }

    /* normalize amplitude of windows */

    for (j = 0; j < *npart; j++) {

      /* get data of window to normalize */

      startpos = npwins[j*3];
      nwin     = npwins[j*3+1];
      ptr      = npwins[j*3+2];

      /* normalize window */

      for (i = 0; i < nwin; i++) {
        if (pwins != NULL) pwins[ptr+i] /= factors[startpos+i];
      }
    }
  }

  /* release memory */

  freeifnotNULL_(wingen);
  freeifnotNULL_(win);
  freeifnotNULL_(factors);
#if 0
  //#ifndef NDEBUG
/* temporary for debugging >>> */
if (pwins != NULL) {
  printf("PARTITION SMOOTHING WINDOWS:\n");
  for (j = 0; j < *npart; j++) {

    /* get data of window */

    startpos = npwins[j*3];
    nwin     = npwins[j*3+1];
    ptr      = npwins[j*3+2];

    /* print data to screen */

    printf("Partition = %d, Startpos = %d, Nwin = %d\n", j, startpos, nwin);
    for (i = 0; i < nwin; i++) {
      printf("  %1.3f", pwins[ptr+i]);
    }
    printf("\n");
  }
}
/* temporary for debugging <<< */
#endif

  return winptr;
}

/* scale partitions (with smoothing) */

void bcc_scalepartitions_(
       int    npart,    /* in:  number of partitions */
       int    specsize, /* in:  spectrum size */
       int*   npwins,   /* in:  start and lengths of smoothing windows */
       float* pwins,    /* in:  smoothing windows */
       float* gains,    /* in:  gain factor for each partition */
       float* re,       /* in:  spectrum to be scaled */
       float* im,       /* in:  spectrum to be scaled */
       float* scre,     /* out: scaled spectrum */
       float* scim      /* out: scaled spectrum */
)
{
  int   i, j, startpos, nwin, winptr;
  float gain;

  for (j = 0; j < specsize; j++) {
    scre[j] = .0;
    scim[j] = .0;
  }

  for (j = 0; j < npart; j++) {

    /* get data of window */

    startpos = npwins[j*3];
    nwin     = npwins[j*3+1];
    winptr   = npwins[j*3+2];

    /* scale partition */

    for (i = 0; i < nwin; i++) {
      gain = pwins[winptr+i]*gains[j];
      scre[startpos+i] += re[startpos+i]*gain;
      scim[startpos+i] += im[startpos+i]*gain;
    }
  }
}

/****************************************************************/
