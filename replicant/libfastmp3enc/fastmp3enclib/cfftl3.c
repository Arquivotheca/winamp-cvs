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
*   $Id: cfftl3.c,v 1.1 2007/05/29 16:02:27 audiodsp Exp $                             *
*   author:   W. Schildbach                                                                    *
*   contents/description:  provides FFT routines specifically tailored for layer-3             *
                           transform length.                                                   *
*                                                                                              *
************************************************************************************************/

#include "mconfig.h"
#include "cfftl3.h"

static const float c31 = -0.86602540f;
static const float c32 = -1.5f;

static const float c92 =  0.93969262f;
static const float c93 = -0.17364818f;
static const float c94 =  0.76604444f;
static const float c95 = -0.5f;
static const float c96 = -0.34202014f;
static const float c97 = -0.98480775f;
static const float c98 = -0.64278761f;

typedef struct
{
  int factor;
  int Ndivfactor;
  int remainder;
} fftfac ;

void fastcfft(float *x, int N, int M, const fftfac factor[]);
static void cfft18(float *x);
static void cfft6(float *x);

void cfft_l3(float *x, int N)
{
  switch(N)
  {
  case  6: cfft6(x);  break;
  case 18: cfft18(x); break;
  }
}

/* a "general" routine for N that can be factored into 2,3,4,9 */

void fastcfft(float *x, int N, int M, const fftfac factor[])
{
  int k ;

  for (k = 0; k < M; k++)
  {
    int n1 = factor[k].factor ;
    int n2 = factor[k].Ndivfactor; /* N / n1 ; */

    int l = 0 ;
    int j ;
    int n3 = factor[k].remainder;  /* n2 % n1; */
    int i[16];
    int ip[16];
    int lp[16] ;

    for (j = 1; j < n1; j++)
    {
      l += n3;
      if (l >= n1) l -= n1;

      lp[j] = l;
    }

    for (j = 0; j < N; j += n1)
    {
      int it = j ;
      i[0]   = 2*j ;
      ip[0]  = 2*j ;
      for (l = 1; l < n1; l++)
      {
        it += n2;
        if (it >= N) it -= N;
        i[l]      = 2*it;
        ip[lp[l]] = 2*it;
      }

      switch(n1)
      {
      case 1:
        break;

      case 2:
        {
          float r1;

          r1 = x[i[0]];
          x[i[0]] = r1 + x[i[1]];
          x[i[1]] = r1 - x[i[1]];

          r1 = x[1+i[0]];
          x[1+ip[0]] = r1 + x[1+i[1]];
          x[1+ip[1]] = r1 - x[1+i[1]];
        }
        break;

      case 3:
        {
          float s1,s2;
          float r1,r2;

          r2     = (x[i[1]] - x[i[2]]) * c31;
          r1     =  x[i[1]] + x[i[2]];
          x[i[0]] = x[i[0]] + r1;
          r1      = x[i[0]] + r1 * c32;

          s2        =(x[1+i[1]] - x[1+i[2]]) * c31;
          s1        = x[1+i[1]] + x[1+i[2]];
          x[1+i[0]] = x[1+i[0]] + s1;
          s1        = x[1+i[0]] + s1 * c32;

          x[ip[1]] = r1 - s2;
          x[ip[2]] = r1 + s2;
          x[1+ip[1]] = s1 + r2;
          x[1+ip[2]] = s1 - r2;
        }
        break;

      case 4:
        {
          float r1,r2;
          float t1,t2;
          r1 = x[i[0]] + x[i[2]];
          t1 = x[i[0]] - x[i[2]];
          r2 = x[i[1]] + x[i[3]];
          x[ip[0]] = r1 + r2;
          x[ip[2]] = r1 - r2;

          r1 = x[1+i[0]] + x[1+i[2]];
          t2 = x[1+i[0]] - x[1+i[2]];
          r2 = x[1+i[1]] + x[1+i[3]];
          x[1+ip[0]] = r1 + r2;
          x[1+ip[2]] = r1 - r2;

          r1 = x[i[1]] - x[i[3]];
          r2 = x[1+i[1]] - x[1+i[3]];

          x[ip[1]] = t1 + r2;
          x[ip[3]] = t1 - r2;
          x[1+ip[1]] = t2 - r1;
          x[1+ip[3]] = t2 + r1;
        }
        break;

      case 9:
        {
          float r1,r2,r3,r4,r5,r6,r7,r8,r9;
          float t0,t1,t2,t3,t4,t5,t6,t7,t8,t9;

          r1 = x[i[1]] + x[i[8]];
          r2 = x[i[1]] - x[i[8]];
          r3 = x[i[2]] + x[i[7]];
          r4 = x[i[2]] - x[i[7]];
          r5 = x[i[3]] + x[i[6]];
          t8 =(x[i[3]] - x[i[6]]) * c31;
          r7 = x[i[4]] + x[i[5]];
          r8 = x[i[4]] - x[i[5]];
          t0 = x[i[0]] + r5;
          t7 = x[i[0]] + r5 * c95;
          r5 = r1 + r3 + r7;
          x[i[0]] = t0 + r5;
          t5 = t0 + r5 * c95;
          t3 = (r3 - r7) * c92;  
          r7 = (r1 - r7) * c93;
          r3 = (r1 - r3) * c94;
          t1 = t7 + t3 + r3;
          t3 = t7 - t3 - r7;
          t7 = t7 + r7 - r3;
          t6 = (r2 - r4 + r8) * c31;
          t4 = (r4 + r8) * c96;
          r8 = (r2 - r8) * c97;
          r2 = (r2 + r4) * c98;
          t2 = t8 + t4 + r2;
          t4 = t8 - t4 - r8;
          t8 = t8 + r8 - r2;
          
          r1 = x[1+i[1]] + x[1+i[8]] ;
          r2 = x[1+i[1]] - x[1+i[8]] ;
          r3 = x[1+i[2]] + x[1+i[7]] ;
          r4 = x[1+i[2]] - x[1+i[7]] ;
          r5 = x[1+i[3]] + x[1+i[6]] ;
          r6 =(x[1+i[3]] - x[1+i[6]]) * c31 ;
          r7 = x[1+i[4]] + x[1+i[5]] ;
          r8 = x[1+i[4]] - x[1+i[5]] ;
          t0 = x[1+i[0]] + r5 ;
          t9 = x[1+i[0]] + r5 * c95 ;
          r5 = r1 + r3 + r7 ;
          x[1+i[0]] = t0 + r5 ;
          r5 = t0 + r5 * c95 ;
          t0 = (r3 - r7) * c92 ;
          r7 = (r1 - r7) * c93 ;
          r3 = (r1 - r3) * c94 ;
          r1 = t9 + t0 + r3 ;
          t0 = t9 - t0 - r7 ;
          r7 = t9 + r7 - r3 ;
          r9 = (r2 - r4 + r8) * c31 ;
          r3 = (r4 + r8) * c96 ;
          r8 = (r2 - r8) * c97 ;
          r4 = (r2 + r4) * c98 ;
          r2 = r6 + r3 + r4 ;
          r3 = r6 - r8 - r3 ;
          r8 = r6 + r8 - r4 ;

          x[ip[1]] = t1 - r2 ;
          x[ip[8]] = t1 + r2 ;
          x[1+ip[1]] = r1 + t2 ;
          x[1+ip[8]] = r1 - t2 ;
          x[ip[2]] = t3 + r3 ;
          x[ip[7]] = t3 - r3 ;
          x[1+ip[2]] = t0 - t4 ;
          x[1+ip[7]] = t0 + t4 ;
          x[ip[3]] = t5 - r9 ;
          x[ip[6]] = t5 + r9 ;
          x[1+ip[3]] = r5 + t6 ;
          x[1+ip[6]] = r5 - t6 ;
          x[ip[4]] = t7 - r8 ;
          x[ip[5]] = t7 + r8 ;
          x[1+ip[4]] = r7 + t8 ;
          x[1+ip[5]] = r7 - t8 ;
        }
      }
    }
  }
}

/* specifically optimized for N=6 */

static void cfft6(float *x)
{
  int k ;
  static const int N = 6;
  static const int M = 2;
  static const int n1_tab[] = {2,3};
  static const int n2_tab[] = {3,2};

  for (k = 0; k < M; k++)
  {
    int n1 = n1_tab[k] ;
    int n2 = n2_tab[k] ;

    int j ;

    for (j = 0; j < N; j += n1)
    {
      int l ;
      int it = j ;
      int i[3];

      i[0] = 2*j ;
      for (l = 1; l < n1; l++)
      {
        it += n2;
        if (it >= N) it -= N;
        i[l] = 2*it;
      }

      switch(n1)
      {
      case 2:
        {
          float r1;

          r1 = x[i[0]];
          x[i[0]] = r1 + x[i[1]];
          x[i[1]] = r1 - x[i[1]];

          r1 = x[1+i[0]];
          x[1+i[0]] = r1 + x[1+i[1]];
          x[1+i[1]] = r1 - x[1+i[1]];
        }
        break;

      case 3:
        {
          float s1,s2;
          float r1,r2;

          r2     = (x[i[1]] - x[i[2]]) * c31;
          r1     =  x[i[1]] + x[i[2]];
          x[i[0]] = x[i[0]] + r1;
          r1      = x[i[0]] + r1 * c32;

          s2        =(x[1+i[1]] - x[1+i[2]]) * c31;
          s1        = x[1+i[1]] + x[1+i[2]];
          x[1+i[0]] = x[1+i[0]] + s1;
          s1        = x[1+i[0]] + s1 * c32;

          x[i[2]] = r1 - s2;
          x[i[1]] = r1 + s2;
          x[1+i[2]] = s1 + r2;
          x[1+i[1]] = s1 - r2;
        }
        break;
      }
    }
  }
}

/* specifically optimized for N=18 */

static void cfft18(float *x)
{
  int k ;
  static const int N = 18;
  static const int M = 2;
  static const int n1_tab[] = {2,9};
  static const int n2_tab[] = {9,2};

  for (k = 0; k < M; k++)
  {
    int n1 = n1_tab[k] ;
    int n2 = n2_tab[k] ;
    int j ;

    for (j = 0; j < N; j += n1)
    {
      int l ;
      int it = j ;
      int i[9];

      i[0] = 2*j ;
      for (l = 1; l < n1; l++)
      {
        it += n2;
        if (it >= N) it -= N;
        i[l] = 2*it;
      }

      switch(n1)
      {
      case 2:
        {
          float r1;

          r1 = x[i[0]];
          x[i[0]] = r1 + x[i[1]];
          x[i[1]] = r1 - x[i[1]];

          r1 = x[1+i[0]];
          x[1+i[0]] = r1 + x[1+i[1]];
          x[1+i[1]] = r1 - x[1+i[1]];
        }
        break;

      case 9:
        {
          float r1,r2,r3,r4,r5,r6,r7,r8,r9;
          float t0,t1,t2,t3,t4,t5,t6,t7,t8,t9;

          r1 = x[i[1]] + x[i[8]];
          r2 = x[i[1]] - x[i[8]];
          r3 = x[i[2]] + x[i[7]];
          r4 = x[i[2]] - x[i[7]];
          r5 = x[i[3]] + x[i[6]];
          t8 =(x[i[3]] - x[i[6]]) * c31;
          r7 = x[i[4]] + x[i[5]];
          r8 = x[i[4]] - x[i[5]];
          t0 = x[i[0]] + r5;
          t7 = x[i[0]] + r5 * c95;
          r5 = r1 + r3 + r7;
          x[i[0]] = t0 + r5;
          t5 = t0 + r5 * c95;
          t3 =(r3 - r7) * c92;  
          r7 =(r1 - r7) * c93;
          r3 =(r1 - r3) * c94;
          t1 = t7 + t3 + r3;
          t3 = t7 - t3 - r7;
          t7 = t7 + r7 - r3;
          t6 =(r2 - r4 + r8) * c31;
          t4 =(r4 + r8) * c96;
          r8 =(r2 - r8) * c97;
          r2 =(r2 + r4) * c98;
          t2 = t8 + t4 + r2;
          t4 = t8 - t4 - r8;
          t8 = t8 + r8 - r2;
          
          r1 = x[1+i[1]] + x[1+i[8]] ;
          r2 = x[1+i[1]] - x[1+i[8]] ;
          r3 = x[1+i[2]] + x[1+i[7]] ;
          r4 = x[1+i[2]] - x[1+i[7]] ;
          r5 = x[1+i[3]] + x[1+i[6]] ;
          r6 =(x[1+i[3]] - x[1+i[6]]) * c31 ;
          r7 = x[1+i[4]] + x[1+i[5]] ;
          r8 = x[1+i[4]] - x[1+i[5]] ;
          t0 = x[1+i[0]] + r5 ;
          t9 = x[1+i[0]] + r5 * c95 ;
          r5 = r1 + r3 + r7 ;
          x[1+i[0]] = t0 + r5 ;
          r5 = t0 + r5 * c95 ;
          t0 =(r3 - r7) * c92 ;
          r7 =(r1 - r7) * c93 ;
          r3 =(r1 - r3) * c94 ;
          r1 = t9 + t0 + r3 ;
          t0 = t9 - t0 - r7 ;
          r7 = t9 + r7 - r3 ;
          r9 =(r2 - r4 + r8) * c31 ;
          r3 =(r4 + r8) * c96 ;
          r8 =(r2 - r8) * c97 ;
          r4 =(r2 + r4) * c98 ;
          r2 = r6 + r3 + r4 ;
          r3 = r6 - r8 - r3 ;
          r8 = r6 + r8 - r4 ;

          x[i[5]]   = t1 - r2 ;
          x[i[4]]   = t1 + r2 ;
          x[1+i[5]] = r1 + t2 ;
          x[1+i[4]] = r1 - t2 ;
          x[i[1]]   = t3 + r3 ;
          x[i[8]]   = t3 - r3 ;
          x[1+i[1]] = t0 - t4 ;
          x[1+i[8]] = t0 + t4 ;
          x[i[6]]   = t5 - r9 ;
          x[i[3]]   = t5 + r9 ;
          x[1+i[6]] = r5 + t6 ;
          x[1+i[3]] = r5 - t6 ;
          x[i[2]]   = t7 - r8 ;
          x[i[7]]   = t7 + r8 ;
          x[1+i[2]] = r7 + t8 ;
          x[1+i[7]] = r7 - t8 ;
        }
      }
    }
  }
}


void cfft_3(float *x)
{
  
  float s1,s2;
  float r1,r2;
  
  r2     = (x[2] - x[4]) * c31;
  r1     =  x[2] + x[4];
  x[0]   = x[0] + r1;
  r1     = x[0] + r1 * c32;
  
  s2      =(x[3] - x[5]) * c31;
  s1      = x[3] + x[5];
  x[1]    = x[1] + s1;
  s1      = x[1] + s1 * c32;
  
  x[2] = r1 - s2;
  x[4] = r1 + s2;
  x[3] = s1 + r2;
  x[5] = s1 - r2;

}

  
  
void cfft_9(float *x)
{

  float r1,r2,r3,r4,r5,r6,r7,r8,r9;
  float t0,t1,t2,t3,t4,t5,t6,t7,t8,t9;
	
  r1 = x[2] + x[16];
  r2 = x[2] - x[16];
  r3 = x[4] + x[14];
  r4 = x[4] - x[14];
  r5 = x[6] + x[12];
  t8 =(x[6] - x[12]) * c31;
  r7 = x[8] + x[10];
  r8 = x[8] - x[10];
  t0 = x[0] + r5;
  t7 = x[0] + r5 * c95;
  r5 = r1 + r3 + r7;
  x[0] = t0 + r5;
  t5 = t0 + r5 * c95;
  t3 = (r3 - r7) * c92;  
  r7 = (r1 - r7) * c93;
  r3 = (r1 - r3) * c94;
  t1 = t7 + t3 + r3;
  t3 = t7 - t3 - r7;
  t7 = t7 + r7 - r3;
  t6 = (r2 - r4 + r8) * c31;
  t4 = (r4 + r8) * c96;
  r8 = (r2 - r8) * c97;
  r2 = (r2 + r4) * c98;
  t2 = t8 + t4 + r2;
  t4 = t8 - t4 - r8;
  t8 = t8 + r8 - r2;
	
  r1 = x[3] + x[17] ;
  r2 = x[3] - x[17] ;
  r3 = x[5] + x[15] ;
  r4 = x[5] - x[15] ;
  r5 = x[7] + x[13] ;
  r6 =(x[7] - x[13]) * c31 ;
  r7 = x[9] + x[11] ;
  r8 = x[9] - x[11] ;
  t0 = x[1] + r5 ;
  t9 = x[1] + r5 * c95 ;
  r5 = r1 + r3 + r7 ;
  x[1] = t0 + r5 ;
  r5 = t0 + r5 * c95 ;
  t0 = (r3 - r7) * c92 ;
  r7 = (r1 - r7) * c93 ;
  r3 = (r1 - r3) * c94 ;
  r1 = t9 + t0 + r3 ;
  t0 = t9 - t0 - r7 ;
  r7 = t9 + r7 - r3 ;
  r9 = (r2 - r4 + r8) * c31 ;
  r3 = (r4 + r8) * c96 ;
  r8 = (r2 - r8) * c97 ;
  r4 = (r2 + r4) * c98 ;
  r2 = r6 + r3 + r4 ;
  r3 = r6 - r8 - r3 ;
  r8 = r6 + r8 - r4 ;
	
  x[2] = t1 - r2 ;
  x[16] = t1 + r2 ;
  x[3] = r1 + t2 ;
  x[17] = r1 - t2 ;
  x[4] = t3 + r3 ;
  x[14] = t3 - r3 ;
  x[5] = t0 - t4 ;
  x[15] = t0 + t4 ;
  x[6] = t5 - r9 ;
  x[12] = t5 + r9 ;
  x[7] = r5 + t6 ;
  x[13] = r5 - t6 ;
  x[8] = t7 - r8 ;
  x[10] = t7 + r8 ;
  x[9] = r7 + t8 ;
  x[11] = r7 - t8 ;
}
  
  


