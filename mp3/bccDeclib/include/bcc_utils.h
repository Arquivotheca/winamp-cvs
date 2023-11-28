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

/* include headers */

#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

/* constants related to computation of partitions */

#define MAXPART      60           /* maximum number of perceptual partitions */
#define MAXPARTWIDTH 200          /* maximum width of a partition */
#define FIRSTPART    200          /* bandwidth of first partition [Hz] */
#define MINPART      1            /* minimum size for a partition [bins] */
#define NWINGEN      20000        /* size of prototype window of partition windows */
#define WINRELWIDTH  2.0          /* relative width of partition windows */

/* function headers */

void init_fft_(int, float**);
void rff2_(float*, float*, float*, int, float**);
void irff2_(float*, float*, float*, int, float**);

float arg_(float, float);
void cmult_(float, float, float, float, float*, float*);
void crosstalkfilt_(float, float, float, float, float, float, float, float,
       float*, float*, float*, float*);
float bcc_min_(float, float);
float bcc_max_(float, float);
void  freeifnotNULL_(void*);
void  bcc_partpow_(/*int,*/ int, int*, float*, float*, float*);
void  bcc_partton(/*int,*/ int, int*, float, float*, float*, float*, float*, float*, float*,
       float*, float*, float*);
float ranfGauss_(int, float);
void  swapint_(int*, int);
int   bcc_makepartitions_(int, int, float, char, int*, int*, int*, float*);
void  bcc_scalepartitions_(int, int, int*, float*, float*, float*, float*, float*, float*);
