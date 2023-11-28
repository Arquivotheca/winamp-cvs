/*****************************************************************************

                       (C) copyright Fraunhofer-IIS (1997-2005)
                               All Rights Reserved

   $Id: cfftn.c,v 1.1 2007/05/29 16:02:35 audiodsp Exp $
   Initial author:       M. Werner
   contents/description: Complex FFT core for transforms
                         not necessary a power of two

   This software and/or program is protected by copyright law and
   international treaties. Any reproduction or distribution of this 
   software and/or program, or any portion of it, may result in severe 
   civil and criminal penalties, and will be prosecuted to the maximum 
   extent possible under law.

******************************************************************************/
#include <math.h>
#include <stdlib.h>

#include "mathlib.h"

/*-----------------------------------------------------------------------*
 * int fftradix (REAL Re[], REAL Im[], size_t nTotal, size_t nPass,
 *		 size_t nSpan, int iSign );
 *
 * RE, IM - see above documentation
 *
 * Although there is no limit on the number of dimensions, fftradix() must
 * be called once for each dimension, but the calls may be in any order.
 *
 * NTOTAL = the total number of complex data values
 * NPASS  = the dimension of the current variable
 * NSPAN/NPASS = the spacing of consecutive data values while indexing the
 *	current variable
 * ISIGN - see above documentation
 *
 * example:
 * tri-variate transform with Re[n1][n2][n3], Im[n1][n2][n3]
 *
 *	fftradix (Re, Im, n1*n2*n3, n1,       n1, 1);
 *	fftradix (Re, Im, n1*n2*n3, n2,    n1*n2, 1);
 *	fftradix (Re, Im, n1*n2*n3, n3, n1*n2*n3, 1);
 *
 * single-variate transform,
 *    NTOTAL = N = NSPAN = (number of complex data values),
 *
 *	fftradix (Re, Im, n, n, n, 1);
 *
 * The data can also be stored in a single array with alternating real and
 * imaginary parts, the magnitude of ISIGN is changed to 2 to give correct
 * indexing increment, and data [0] and data [1] used to pass the initial
 * addresses for the sequences of real and imaginary values,
 *
 * example:
 *	REAL data [2*NTOTAL];
 *	fftradix ( &data[0], &data[1], NTOTAL, nPass, nSpan, 2);
 *
 * for temporary allocation:
 *
 * MAX_FACTORS	>= the maximum prime factor of NPASS
 * MAX_PERM	>= the number of prime factors of NPASS.  In addition,
 *	if the square-free portion K of NPASS has two or more prime
 *	factors, then MAX_PERM >= (K-1)
 *
 * storage in FACTOR for a maximum of 15 prime factors of NPASS. if NPASS
 * has more than one square-free factor, the product of the square-free
 * factors must be <= 210 array storage for maximum prime factor of 23 the
 * following two constants should agree with the array dimensions.
 *
 * singleton's mixed radix routine
 *
 */

#define MAX_FACTORS  23
#define MAX_PERM    209
#define NFACTOR	     11
#ifndef M_PI
#define M_PI	3.14159265358979323846264338327950288
#endif
#define SIN60	0.86602540378443865	/* sin(60 deg) */
#define COS72	0.30901699437494742	/* cos(72 deg) */
#define SIN72	0.95105651629515357	/* sin(72 deg) */


static int cfftn(float Re[],
                  float Im[],
                  int  nTotal,
                  int  nPass,
                  int  nSpan,
                  int  iSign)
{
  int ii, mfactor, kspan, ispan, inc;
  int j, jc, jf, jj, k, k1, k2, k3=0, k4, kk, kt, nn, ns, nt;
  
  double radf;
  double c1, c2=0.0, c3=0.0, cd;
  double s1, s2=0.0, s3=0.0, sd;
  float  ak,bk,akp,bkp,ajp,bjp,ajm,bjm,akm,bkm,aj,bj,aa,bb;

  float Rtmp[MAX_FACTORS];      /* temp space for real part*/
  float Itmp[MAX_FACTORS];      /* temp space for imaginary part */
  double Cos[MAX_FACTORS];       /* Cosine values */
  double Sin[MAX_FACTORS];       /* Sine values */
  
  int Perm[MAX_PERM];
  int factor [NFACTOR];
  
  double s60 = SIN60;            /* sin(60 deg) */
  double c72 = COS72;            /* cos(72 deg) */
  double s72 = SIN72;            /* sin(72 deg) */
  double pi2 = M_PI;             /* use PI first, 2 PI later */
  
#ifdef __BOUNDS_CHECKING_ON
  BOUNDS_CHECKING_OFF;
#endif
  /* Parameter adjustments, was fortran so fix zero-offset */
  Re--; /* this dirty and therefore it is NOT bounds check compatible */
  Im--;
  
  if (nPass < 2)
    return 0;
  
    /*
    * Function Body
  */
  inc = iSign;
  if( iSign < 0 ) {
    s72 = -s72;
    s60 = -s60;
    pi2 = -pi2;
    inc = -inc;		/* absolute value */
  }
  
  /* adjust for strange increments */
  nt = inc * nTotal;
  ns = inc * nSpan;
  kspan = ns;
  
  nn = nt - inc;
  jc = ns / nPass;
  radf = pi2 * (double) jc;
  pi2 *= 2.0;			/* use 2 PI from here on */
  
  ii = 0;
  jf = 0;
  /*  determine the factors of n */
  mfactor = 0;
  k = nPass;
  while (k % 16 == 0) {
    mfactor++;
    factor [mfactor - 1] = 4;
    k /= 16;
  }
  j = 3;
  jj = 9;
  do {
    while (k % jj == 0) {
      mfactor++;
      factor [mfactor - 1] = j;
      k /= jj;
    }
    j += 2;
    jj = j * j;
  } while (jj <= k);
  if (k <= 4) {
    kt = mfactor;
    factor [mfactor] = k;
    if (k != 1)
      mfactor++;
  } else {
    if (k - (k / 4 << 2) == 0) {
      mfactor++;
      factor [mfactor - 1] = 2;
      k /= 4;
    }
    kt = mfactor;
    j = 2;
    do {
      if (k % j == 0) {
        mfactor++;
        factor [mfactor - 1] = j;
        k /= j;
      }
      j = ((j + 1) / 2 << 1) + 1;
    } while (j <= k);
  }
  if (kt) {
    j = kt;
    do {
      mfactor++;
      factor [mfactor - 1] = factor [j - 1];
      j--;
    } while (j);
  }
  
  /* test that mfactors is in range */
  if (mfactor > NFACTOR) {
    return(0);
  }
  
  /* compute fourier transform */
  for (;;) {
    sd = radf / (double) kspan;
    cd = sin(sd);
    cd = 2.0 * cd * cd;
    sd = sin(sd + sd);
    kk = 1;
    ii++;
    
    switch (factor [ii - 1]) {
    case 2:
      /* transform for factor of 2 (including rotation factor) */
      kspan /= 2;
      k1 = kspan + 2;
      do {
        do {
          k2 = kk + kspan;
          ak = Re [k2];
          bk = Im [k2];
          Re [k2] = Re [kk] - ak;
          Im [k2] = Im [kk] - bk;
          Re [kk] += ak;
          Im [kk] += bk;
          kk = k2 + kspan;
        } while (kk <= nn);
        kk -= nn;
      } while (kk <= jc);
      if (kk > kspan)
        goto Permute_Results_Label;		/* exit infinite loop */
      do {
        c1 = 1.0 - cd;
        s1 = sd;
        do {
          do {
            do {
              k2 = kk + kspan;
              ak = Re [kk] - Re [k2];
              bk = Im [kk] - Im [k2];
              Re [kk] += Re [k2];
              Im [kk] += Im [k2];
              Re [k2] = (float)(c1 * ak - s1 * bk);
              Im [k2] = (float)(s1 * ak + c1 * bk);
              kk = k2 + kspan;
            } while (kk < nt);
            k2 = kk - nt;
            c1 = -c1;
            kk = k1 - k2;
          } while (kk > k2);
          ak = (float)(c1 - (cd * c1 + sd * s1));
          s1 = sd * c1 - cd * s1 + s1;
          c1 = 2.0 - (ak * ak + s1 * s1);
          s1 *= c1;
          c1 *= ak;
          kk += jc;
        } while (kk < k2);
        k1 += inc + inc;
        kk = (k1 - kspan) / 2 + jc;
      } while (kk <= jc + jc);
      break;
      
    case 4:			/* transform for factor of 4 */
      ispan = kspan;
      kspan /= 4;
      
      do {
        c1 = 1.0;
        s1 = 0.0;
        do {
          do {
            k1 = kk + kspan;
            k2 = k1 + kspan;
            k3 = k2 + kspan;
            akp = Re [kk] + Re [k2];
            akm = Re [kk] - Re [k2];
            ajp = Re [k1] + Re [k3];
            ajm = Re [k1] - Re [k3];
            bkp = Im [kk] + Im [k2];
            bkm = Im [kk] - Im [k2];
            bjp = Im [k1] + Im [k3];
            bjm = Im [k1] - Im [k3];
            Re [kk] = akp + ajp;
            Im [kk] = bkp + bjp;
            ajp = akp - ajp;
            bjp = bkp - bjp;
            if (iSign < 0) {
              akp = akm + bjm;
              bkp = bkm - ajm;
              akm -= bjm;
              bkm += ajm;
            } else {
              akp = akm - bjm;
              bkp = bkm + ajm;
              akm += bjm;
              bkm -= ajm;
            }
            /* avoid useless multiplies */
            if (s1 != 0.0) {
              Re [k1] = (float)(akp * c1 - bkp * s1);
              Re [k2] = (float)(ajp * c2 - bjp * s2);
              Re [k3] = (float)(akm * c3 - bkm * s3);
              Im [k1] = (float)(akp * s1 + bkp * c1);
              Im [k2] = (float)(ajp * s2 + bjp * c2);
              Im [k3] = (float)(akm * s3 + bkm * c3);
            } else {
              Re [k1] = akp;
              Re [k2] = ajp;
              Re [k3] = akm;
              Im [k1] = bkp;
              Im [k2] = bjp;
              Im [k3] = bkm;
            }
            kk = k3 + kspan;
          } while (kk <= nt);
          
          c2 = c1 - (cd * c1 + sd * s1);
          s1 = sd * c1 - cd * s1 + s1;
          c1 = 2.0 - (c2 * c2 + s1 * s1);
          s1 *= c1;
          c1 *= c2;
          /* values of c2, c3, s2, s3 that will get used next time */
          c2 = c1 * c1 - s1 * s1;
          s2 = 2.0 * c1 * s1;
          c3 = c2 * c1 - s2 * s1;
          s3 = c2 * s1 + s2 * c1;
          kk = kk - nt + jc;
        } while (kk <= kspan);
        kk = kk - kspan + inc;
      } while (kk <= jc);
      if (kspan == jc)
        goto Permute_Results_Label;		/* exit infinite loop */
      break;
      
    default:
      k = factor [ii - 1];
      ispan = kspan;
      kspan /= k;
      
      switch (k) {
      case 3:	/* transform for factor of 3 (optional code) */
        do {
          do {
            k1 = kk + kspan;
            k2 = k1 + kspan;
            ak = Re [kk];
            bk = Im [kk];
            aj = Re [k1] + Re [k2];
            bj = Im [k1] + Im [k2];
            Re [kk] = ak + aj;
            Im [kk] = bk + bj;
            ak -= 0.5f * aj;
            bk -= 0.5f * bj;
            aj = (float)((Re [k1] - Re [k2]) * s60);
            bj = (float)((Im [k1] - Im [k2]) * s60);
            Re [k1] = ak - bj;
            Re [k2] = ak + bj;
            Im [k1] = bk + aj;
            Im [k2] = bk - aj;
            kk = k2 + kspan;
          } while (kk < nn);
          kk -= nn;
        } while (kk <= kspan);
        break;
        
      case 5:	/*  transform for factor of 5 (optional code) */
        c2 = c72 * c72 - s72 * s72;
        s2 = 2.0 * c72 * s72;
        do {
          do {
            k1 = kk + kspan;
            k2 = k1 + kspan;
            k3 = k2 + kspan;
            k4 = k3 + kspan;
            akp = Re [k1] + Re [k4];
            akm = Re [k1] - Re [k4];
            bkp = Im [k1] + Im [k4];
            bkm = Im [k1] - Im [k4];
            ajp = Re [k2] + Re [k3];
            ajm = Re [k2] - Re [k3];
            bjp = Im [k2] + Im [k3];
            bjm = Im [k2] - Im [k3];
            aa = Re [kk];
            bb = Im [kk];
            Re [kk] = aa + akp + ajp;
            Im [kk] = bb + bkp + bjp;
            ak = (float)(akp * c72 + ajp * c2 + aa);
            bk = (float)(bkp * c72 + bjp * c2 + bb);
            aj = (float)(akm * s72 + ajm * s2);
            bj = (float)(bkm * s72 + bjm * s2);
            Re [k1] = ak - bj;
            Re [k4] = ak + bj;
            Im [k1] = bk + aj;
            Im [k4] = bk - aj;
            ak = (float)(akp * c2 + ajp * c72 + aa);
            bk = (float)(bkp * c2 + bjp * c72 + bb);
            aj = (float)(akm * s2 - ajm * s72);
            bj = (float)(bkm * s2 - bjm * s72);
            Re [k2] = ak - bj;
            Re [k3] = ak + bj;
            Im [k2] = bk + aj;
            Im [k3] = bk - aj;
            kk = k4 + kspan;
          } while (kk < nn);
          kk -= nn;
        } while (kk <= kspan);
        break;
        
      default:
        if (k != jf) {
          jf = k;
          s1 = pi2 / (double) k;
          c1 = cos(s1);
          s1 = sin(s1);
          if (jf > MAX_FACTORS )
            return(0);
          
          Cos [jf - 1] = 1.0;
          Sin [jf - 1] = 0.0;
          j = 1;
          do {
            Cos [j - 1] = Cos [k - 1] * c1 + Sin [k - 1] * s1;
            Sin [j - 1] = Cos [k - 1] * s1 - Sin [k - 1] * c1;
            k--;
            Cos [k - 1] = Cos [j - 1];
            Sin [k - 1] = -Sin [j - 1];
            j++;
          } while (j < k);
        }
        do {
          do {
            k1 = kk;
            k2 = kk + ispan;
            ak = aa = Re [kk];
            bk = bb = Im [kk];
            j = 1;
            k1 += kspan;
            do {
              k2 -= kspan;
              j++;
              Rtmp [j - 1] = Re [k1] + Re [k2];
              ak += Rtmp [j - 1];
              Itmp [j - 1] = Im [k1] + Im [k2];
              bk += Itmp [j - 1];
              j++;
              Rtmp [j - 1] = Re [k1] - Re [k2];
              Itmp [j - 1] = Im [k1] - Im [k2];
              k1 += kspan;
            } while (k1 < k2);
            Re [kk] = ak;
            Im [kk] = bk;
            k1 = kk;
            k2 = kk + ispan;
            j = 1;
            do {
              k1 += kspan;
              k2 -= kspan;
              jj = j;
              ak = aa;
              bk = bb;
              aj = 0.0f;
              bj = 0.0f;
              k = 1;
              do {
                k++;
                ak += (float)(Rtmp [k - 1] * Cos [jj - 1]);
                bk += (float)(Itmp [k - 1] * Cos [jj - 1]);
                k++;
                aj += (float)(Rtmp [k - 1] * Sin [jj - 1]);
                bj += (float)(Itmp [k - 1] * Sin [jj - 1]);
                jj += j;
                if (jj > jf) {
                  jj -= jf;
                }
              } while (k < jf);
              k = jf - j;
              Re [k1] = ak - bj;
              Im [k1] = bk + aj;
              Re [k2] = ak + bj;
              Im [k2] = bk - aj;
              j++;
            } while (j < k);
            kk += ispan;
          } while (kk <= nn);
          kk -= nn;
        } while (kk <= kspan);
        break;
      }
      /*  multiply by rotation factor (except for factors of 2 and 4) */
      if (ii == mfactor)
        goto Permute_Results_Label;		/* exit infinite loop */
      kk = jc + 1;
      do {
        c2 = 1.0 - cd;
        s1 = sd;
        do {
          c1 = c2;
          s2 = s1;
          kk += kspan;
          do {
            do {
              ak = Re [kk];
              Re [kk] = (float)(c2 * ak - s2 * Im [kk]);
              Im [kk] = (float)(s2 * ak + c2 * Im [kk]);
              kk += ispan;
            } while (kk <= nt);
            ak = (float)(s1 * s2);
            s2 = s1 * c2 + c1 * s2;
            c2 = c1 * c2 - ak;
            kk = kk - nt + kspan;
          } while (kk <= ispan);
          c2 = c1 - (cd * c1 + sd * s1);
          s1 += sd * c1 - cd * s1;
          c1 = 2.0 - (c2 * c2 + s1 * s1);
          s1 *= c1;
          c2 *= c1;
          kk = kk - ispan + jc;
        } while (kk <= kspan);
        kk = kk - kspan + jc + inc;
      } while (kk <= jc + jc);
      break;
    }
  }
  
  /*  permute the results to normal order---done in two stages */
  /*  permutation for square factors of n */
Permute_Results_Label:
  Perm [0] = ns;
  if (kt) {
    k = kt + kt + 1;
    if (mfactor < k)
      k--;
    j = 1;
    Perm [k] = jc;
    do {
      Perm [j] = Perm [j - 1] / factor [j - 1];
      Perm [k - 1] = Perm [k] * factor [j - 1];
      j++;
	     k--;
    } while (j < k);
    k3 = Perm [k];
    kspan = Perm [1];
    kk = jc + 1;
    k2 = kspan + 1;
    j = 1;
    if (nPass != nTotal) {
      /*  permutation for multivariate transform */
Permute_Multi_Label:
    do {
      do {
        k = kk + jc;
        do {
		        /* swap Re [kk] <> Re [k2], Im [kk] <> Im [k2] */
		        ak = Re [kk]; Re [kk] = Re [k2]; Re [k2] = ak;
            bk = Im [kk]; Im [kk] = Im [k2]; Im [k2] = bk;
            kk += inc;
            k2 += inc;
        } while (kk < k);
        kk += ns - jc;
        k2 += ns - jc;
      } while (kk < nt);
      k2 = k2 - nt + kspan;
      kk = kk - nt + jc;
    } while (k2 < ns);
    do {
      do {
        k2 -= Perm [j - 1];
        j++;
        k2 = Perm [j] + k2;
      } while (k2 > Perm [j - 1]);
      j = 1;
      do {
        if (kk < k2)
		        goto Permute_Multi_Label;
        kk += jc;
        k2 += kspan;
      } while (k2 < ns);
    } while (kk < ns);
    } else {
      /*  permutation for single-variate transform (optional code) */
Permute_Single_Label:
    do {
      /* swap Re [kk] <> Re [k2], Im [kk] <> Im [k2] */
      ak = Re [kk]; Re [kk] = Re [k2]; Re [k2] = ak;
      bk = Im [kk]; Im [kk] = Im [k2]; Im [k2] = bk;
      kk += inc;
      k2 += kspan;
    } while (k2 < ns);
    do {
      do {
        k2 -= Perm [j - 1];
        j++;
        k2 = Perm [j] + k2;
      } while (k2 > Perm [j - 1]);
      j = 1;
      do {
        if (kk < k2)
		        goto Permute_Single_Label;
        kk += inc;
        k2 += kspan;
      } while (k2 < ns);
    } while (kk < ns);
    }
    jc = k3;
  }
  
  if ((kt << 1) + 1 >= mfactor)
    return 1;
  ispan = Perm [kt];
  /* permutation for square-free factors of n */
  j = mfactor - kt;
  factor [j] = 1;
  do {
    factor [j - 1] *= factor [j];
    j--;
  } while (j != kt);
  kt++;
  nn = factor [kt - 1] - 1;
  if (nn > MAX_PERM)
    return(0);
  
  j = jj = 0;
  for (;;) {
    k = kt + 1;
    k2 = factor [kt - 1];
    kk = factor [k - 1];
    j++;
    if (j > nn)
      break;				/* exit infinite loop */
    jj += kk;
    while (jj >= k2) {
      jj -= k2;
      k2 = kk;
      k++;
      kk = factor [k - 1];
      jj += kk;
    }
    Perm [j - 1] = jj;
  }
  /*  determine the permutation cycles of length greater than 1 */
  j = 0;
  for (;;) {
    do {
      j++;
      kk = Perm [j - 1];
    } while (kk < 0);
    if (kk != j) {
      do {
        k = kk;
        kk = Perm [k - 1];
        Perm [k - 1] = -kk;
      } while (kk != j);
      k3 = kk;
    } else {
      Perm [j - 1] = -j;
      if (j == nn)
        break;		/* exit infinite loop */
    }
  }
  
  /*  reorder a and b, following the permutation cycles */
  for (;;) {
    j = k3 + 1;
    nt -= ispan;
    ii = nt - inc + 1;
    if (nt < 0)
      break;			/* exit infinite loop */
    do {
      do {
        j--;
      } while (Perm [j - 1] < 0);
      jj = jc;
      do {
        kspan = jj;
        if (jj > MAX_FACTORS * inc) {
          kspan = MAX_FACTORS * inc;
        }
        jj -= kspan;
        k = Perm [j - 1];
        kk = jc * k + ii + jj;
        k1 = kk + kspan;
        k2 = 0;
        do {
          k2++;
          Rtmp [k2 - 1] = Re [k1];
          Itmp [k2 - 1] = Im [k1];
          k1 -= inc;
        } while (k1 != kk);
        do {
          k1 = kk + kspan;
          k2 = k1 - jc * (k + Perm [k - 1]);
          k = -Perm [k - 1];
          do {
            Re [k1] = Re [k2];
            Im [k1] = Im [k2];
            k1 -= inc;
            k2 -= inc;
          } while (k1 != kk);
          kk = k2;
        } while (k != j);
        k1 = kk + kspan;
        k2 = 0;
        do {
          k2++;
          Re [k1] = Rtmp [k2 - 1];
          Im [k1] = Itmp [k2 - 1];
          k1 -= inc;
        } while (k1 != kk);
      } while (jj);
    } while (j != 1);
  }
  return 1;			/* exit point here */
}
/*****************************************************************************

    functionname: CFFTN
    description:  computes complex fourier transform of length len
                  
    returns:      success
    input:        afftData array with time signal data
                  len      transform length
                  isign    sets sign of rotation coefficient:
                           exp(isign*2*pi/len * n*k)
                           i.e. +1=backward (IFFT), -1=forward transform (FFT)
    output:       afftData array with spectral data 

*****************************************************************************/
int CFFTN(float *afftData,int len, int isign)
{
  return(cfftn(afftData,afftData+1,len,len,len,2*isign));
}


/*****************************************************************************

    functionname: CFFTNRI
    description:  computes complex fourier transform of length len
                  
    returns:      success
    input:        afftData array with time signal data
                  len      transform length
                  isign    sets sign of rotation coefficient:
                           exp(isign*2*pi/len * n*k)
                           i.e. +1=backward (IFFT), -1=forward transform (FFT)
    output:       afftData array with spectral data 

*****************************************************************************/
int CFFTNRI(float *afftDataReal,float *afftDataImag,int len, int isign)
{
  return(cfftn(afftDataReal,afftDataImag,len,len,len,isign));
}

/*****************************************************************************

    functionname: CFFTN_NI
    description:  computes complex fourier transform of length len
                  
    returns:      success
    input:        InRealData array with time signal data
                  InImagData array with time signal data
                  OutRealData array with fft signal data
                  OutImagData array with fft signal data

                  len      transform length
                  isign    sets sign of rotation coefficient:
                           exp(isign*2*pi/len * n*k)
                           i.e. +1=backward (IFFT), -1=forward transform (FFT)
    output:       afftData array with spectral data 

*****************************************************************************/
int CFFTN_NI(float *InRealData,
              float *InImagData,
              float *OutRealData,
              float *OutImagData,
              int len, int isign)
{

  copyFLOAT(InRealData, OutRealData, len) ; 
  copyFLOAT(InImagData, OutImagData, len) ; 

  return(cfftn(OutRealData,OutImagData,len,len,len,isign));

}


/*****************************************************************************

    functionname: RFFTN
    description:  computes Fourier transform of a real signal with length len

    returns:      success
    input:        afftData array with real time signal data
                  len      transform length
                  isign    sets sign of rotation coefficient:
                           exp(isign*2*pi/len * n*k)
                           i.e. +1=backward (IFFT), -1=forward transform (FFT)
                  BEWARE OF THE SIGNS !!!!!!
    output:       afftData array with complex spectral data
                  NOTE: X(0) and X(N/2) are both real and stored
                  in afftData[0] (re=X(0),im=X(N/2)) !

*****************************************************************************/
int RFFTN(float *afftData, const float* trigPtr, int len, int isign)
{
  int i;
  int status;
  float tmp1, tmp2, tmp3, tmp4, s, c;
  float scale;

  /* Check that input vector has an even length */
  if(len & 0X01)
    status = 0;
  else
  {
    switch(isign)
    {
      /* Forward FFT */
    case -1 : 
      status = CFFTN(afftData, len/2, isign);
       
      /*
       * Compute the DFT of 2N real input sequence by calculating a N point DFT of a
       * complex input sequence. The 2N real input data are mapped to a complex sequence
       * of size N by interleaving the input data:
       *
       *  Re[t(n)] = v(2n)  , 0<=n<=N-1
       *  Im[t(n)] = v(2n+1), 0<=n<=N-1
       *
       * The complex output values V(k), corresponding to the spectrum of the
       * 2N real input sequence v(n), can be computed from the output values T(k)
       * of the N-point FFT according to:
       *
       * V(k)    = 0.5 * [T(k) + T'(N-k)] - j W(k,N) * 0.5 * [T(k) - T'(N-k)]
       * V'(N-k) = 0.5 * [T(k) + T'(N-k)] + j W(k,N) * 0.5 * [T(k) - T'(N-k)]
       * for 1 < k < N/2, W(k,N) = exp(-j*PI*k/N)
       *
       * V(0) = Re[T(0)] + Im[T(0)]
       * V(N) = Re[T(0)] - Im[T(0)]
       * The operator ' shows the complex conjugate
       *
       */
      
      /*
       * Calculate first V(0) and V(N)
       */
      tmp1 = afftData[0] + afftData[1];
      afftData[1] = afftData[0] - afftData[1];
      afftData[0] = tmp1;
      
      for(i = 1; i <= (len/4+(len%4)/2); ++i)
      {
        tmp1 = afftData[2*i] - afftData[len-2*i];
        tmp2 = afftData[2*i+1] + afftData[len-2*i+1];
        
        s = trigPtr[i];       /* sin(pi*i/(len/2)) */
        c = trigPtr[i+len/4]; /* cos(pi*i/(len/2)) */
        
        tmp3 = s*tmp1 - c*tmp2;  /* real part of j*W(k,N)*[T(k) - T'(N-k)] */
        tmp4 = c*tmp1 + s*tmp2;  /* imag part of j*W(k,N)*[T(k) - T'(N-k)] */
        
        tmp1 = afftData[2*i] + afftData[len-2*i];
        tmp2 = afftData[2*i+1] - afftData[len-2*i+1];
        
        afftData[2*i]       =  0.5f*(tmp1 - tmp3);
        afftData[2*i+1]     =  0.5f*(tmp2 - tmp4);
        afftData[len-2*i]   =  0.5f*(tmp1 + tmp3);
        afftData[len-2*i+1] = -0.5f*(tmp2 + tmp4);
      }
      break;
      
      /* Inverse FFT */
    case +1 :

      /*
       * The computation of the inverse FFT is similar to the forward one, except that the 
       * additionnal processing is performed before the FFT
       */
      scale = 1.0f/len;

      tmp1 = afftData[0] + afftData[1];
      afftData[1] = scale*(afftData[0] - afftData[1]);
      afftData[0] = scale*tmp1;

      for(i = 1; i <= (len/4+(len%4)/2); ++i)
      {
        tmp1 = afftData[2*i] - afftData[len-2*i];
        tmp2 = afftData[2*i+1] + afftData[len-2*i+1];
        
        s = trigPtr[i];       /* sin(pi*i/(len/2)) */
        c = trigPtr[i+len/4]; /* cos(pi*i/(len/2)) */
        
        tmp3 = s*tmp1 + c*tmp2;  /* real part of j*W(k,N)*[T(k) - T'(N-k)] */
        tmp4 = -c*tmp1 + s*tmp2;  /* imag part of j*W(k,N)*[T(k) - T'(N-k)] */
        
        tmp1 = afftData[2*i] + afftData[len-2*i];
        tmp2 = afftData[2*i+1] - afftData[len-2*i+1];
        
        afftData[2*i]       =  scale*(tmp1 - tmp3);
        afftData[2*i+1]     =  scale*(tmp2 - tmp4);
        afftData[len-2*i]   =  scale*(tmp1 + tmp3);
        afftData[len-2*i+1] = -scale*(tmp2 + tmp4);
      }
      
      status = CFFTN(afftData, len/2, isign);

      break;

    default:
      status = 0;
      break;
    }
  }
  return status;
}



float* CreateSineTable(int len)
{
  int i;
  float* trigPtr = (float*)malloc(sizeof(float)*(len/2+1));

  for(i = 0; i < len/2+1; ++i)
    trigPtr[i] = (float)sin(2.0*M_PI*i/len);

  return trigPtr;
}


void DestroySineTable(float* trigPtr)
{
  if(trigPtr != NULL)
    free(trigPtr);
}
