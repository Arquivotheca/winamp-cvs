/*****************************************************************************

                       (C) copyright Fraunhofer-IIS (1997-2005)
                               All Rights Reserved

   translated by f2c from zufall.f. Some polishing by sdb
   random number generation routines

   This software and/or program is protected by copyright law and
   international treaties. Any reproduction or distribution of this
   software and/or program, or any portion of it, may result in severe
   civil and criminal penalties, and will be prosecuted to the maximum
   extent possible under law.

   $Id: random.c,v 1.1 2009/04/28 20:17:42 audiodsp Exp $

******************************************************************************/

#include "random.h"

/* NORMAL generator routines: */

/*       subroutine normalen(n,g) */
/*       integer n */
/*       double precision g(n) */
/* c returns set of n normals g(1), ..., g(n) such that */
/* c mean <g> = 0, and variance <g**2> = 1. */

/*       subroutine normalsv(normsv) */
/*       double precision normsv(1634) */
/* c saves zufall seed buffer and pointer in normsv */
/* c buffer/pointer for normalen restart also in normsv */

/*       subroutine normalrs(normsv) */
/*       double precision normsv(1634) */
/* c restores zufall seed buffer/pointer and */
/* c buffer/pointer for normalen restart from normsv */
/* ------------------------------------------------------ */

/* POISSON generator routine: */

/*       subroutine fische(n,mu,q) */
/*       integer n,q(n) */
/*       double precision mu */
/* c returns set of n integers q, with poisson */
/* c distribution, density p(q,mu) = exp(-mu) mu**q/q! */
/* c */
/* c USE zufallsv and zufallrs for stop/restart sequence */
/* c */

struct klotz0_1_ {
  double buff[607];
  int ptr;
};

#define klotz0_1 (*(struct klotz0_1_ *) &klotz0_)


#if 0
/* ndf 20050308: At the moment, "klotz1_" only appears in an
"if0"'ed section below and thus causes a "defined but not used" warning.*/
struct klotz1_1_ {
  double xbuff[1024];
  int first, xptr;
};
#define klotz1_1 (*(struct klotz1_1_ *) &klotz1_)
#endif /* #if 0 */


/* Initialized data */

static struct {
  double fill_1[607];
  int e_2;
  int fill_3[1];
  double e_4;
} klotz0_ = { { 0 }, 0, { 0 }, 0.};


#if 0
/* ndf 20050308: pls see above comment */
static struct {
  double fill_1[1024];
  int e_2[2];
  double e_3;
} klotz1_ = { { 0 }, { 0 }, 0. };
#endif


void zufall(int n, double *a)
{
  /* Initialized data */

  static int buffsz = 607;

  /* System generated locals */
  int i__1, i__2;

  /* Local variables */
  int left, aptr, bptr, aptr0, i, k, q;
  double t;
  int nn, vl, qq, k273, k607;


/* portable lagged Fibonacci series uniform random number */
/* generator with "lags" -273 und -607: */

/*       t    = u(i-273)+buff(i-607)  (floating pt.) */
/*       u(i) = t - float(int(t)) */

/* W.P. Petersen, IPS, ETH Zuerich, 19 Mar. 92 */


  /* Parameter adjustments */
  --a;

  /* Function Body */

  aptr = 0;
  nn = n;

L1:

  if (nn <= 0) {
    return;
  }
/* factor nn = q*607 + r */

  q = (nn - 1) / 607;
  left = buffsz - klotz0_1.ptr;

  if (q <= 1) {

/* only one or fewer full segments */

    if (nn < left) {
      i__1 = nn;
      for (i = 1; i <= i__1; ++i) {
	a[i + aptr] = klotz0_1.buff[klotz0_1.ptr + i - 1];
      }
      klotz0_1.ptr += nn;
      return;
    } else {
      i__1 = left;
      for (i = 1; i <= i__1; ++i) {
	a[i + aptr] = klotz0_1.buff[klotz0_1.ptr + i - 1];
      }
      klotz0_1.ptr = 0;
      aptr += left;
      nn -= left;
/*  buff -> buff case */
      vl = 273;
      k273 = 334;
      k607 = 0;
      for (k = 1; k <= 3; ++k) {
	i__1 = vl;
	for (i = 1; i <= i__1; ++i) {
	  t = klotz0_1.buff[k273 + i - 1] + klotz0_1.buff[k607 + i
							  - 1];
	  klotz0_1.buff[k607 + i - 1] = t - (double) ((int) t);
	}
	k607 += vl;
	k273 += vl;
	vl = 167;
	if (k == 1) {
	  k273 = 0;
	}
      }

      goto L1;
    }
  } else {

/* more than 1 full segment */

    i__1 = left;
    for (i = 1; i <= i__1; ++i) {
      a[i + aptr] = klotz0_1.buff[klotz0_1.ptr + i - 1];
    }
    nn -= left;
    klotz0_1.ptr = 0;
    aptr += left;

/* buff -> a(aptr0) */

    vl = 273;
    k273 = 334;
    k607 = 0;
    for (k = 1; k <= 3; ++k) {
      if (k == 1) {
	i__1 = vl;
	for (i = 1; i <= i__1; ++i) {
	  t = klotz0_1.buff[k273 + i - 1] + klotz0_1.buff[k607 + i
							  - 1];
	  a[aptr + i] = t - (double) ((int) t);
	}
	k273 = aptr;
	k607 += vl;
	aptr += vl;
	vl = 167;
      } else {
	i__1 = vl;
	for (i = 1; i <= i__1; ++i) {
	  t = a[k273 + i] + klotz0_1.buff[k607 + i - 1];
	  a[aptr + i] = t - (double) ((int) t);
	}
	k607 += vl;
	k273 += vl;
	aptr += vl;
      }
    }
    nn += -607;

/* a(aptr-607) -> a(aptr) for last of the q-1 segments */

    aptr0 = aptr - 607;
    vl = 607;

    i__1 = q - 2;
    for (qq = 1; qq <= i__1; ++qq) {
      k273 = aptr0 + 334;
      i__2 = vl;
      for (i = 1; i <= i__2; ++i) {
	t = a[k273 + i] + a[aptr0 + i];
	a[aptr + i] = t - (double) ((int) t);
      }
      nn += -607;
      aptr += vl;
      aptr0 += vl;
    }

/* a(aptr0) -> buff, last segment before residual */

    vl = 273;
    k273 = aptr0 + 334;
    k607 = aptr0;
    bptr = 0;
    for (k = 1; k <= 3; ++k) {
      if (k == 1) {
	i__1 = vl;
	for (i = 1; i <= i__1; ++i) {
	  t = a[k273 + i] + a[k607 + i];
	  klotz0_1.buff[bptr + i - 1] = t - (double) ((int) t);
	}
	k273 = 0;
	k607 += vl;
	bptr += vl;
	vl = 167;
      } else {
	i__1 = vl;
	for (i = 1; i <= i__1; ++i) {
	  t = klotz0_1.buff[k273 + i - 1] + a[k607 + i];
	  klotz0_1.buff[bptr + i - 1] = t - (double) ((int) t);
	}
	k607 += vl;
	k273 += vl;
	bptr += vl;
      }
    }
    goto L1;
  }
}				/* zufall_ */


void zufalli(int seed)
{
  /* Initialized data */

  static int kl = 9373;
  static int ij = 1802;

  int i, j, k, l, m;
  double s, t;
  int ii, jj;


/*  generates initial seed buffer by linear congruential */
/*  method. Taken from Marsaglia, FSU report FSU-SCRI-87-50 */
/*  variable seed should be 0 < seed <31328 */


  if (seed != 0) {
    ij = seed;
  }
  i = ij / 177 % 177 + 2;
  j = ij % 177 + 2;
  k = kl / 169 % 178 + 1;
  l = kl % 169;
  for (ii = 1; ii <= 607; ++ii) {
    s = 0.f;
    t = .5f;
    for (jj = 1; jj <= 24; ++jj) {
      m = i * j % 179 * k % 179;
      i = j;
      j = k;
      k = m;
      l = (l * 53 + 1) % 169;
      if (l * m % 64 >= 32) {
	s += t;
      }
      t *= .5f;
    }
    klotz0_1.buff[ii - 1] = s;
  }
  return;
}				/* zufalli_ */

#if 1

void zufallsv(double *svblk)
{
  int i;


/*  saves common blocks klotz0, containing seeds and */
/*  pointer to position in seed block. IMPORTANT: svblk must be */
/*  dimensioned at least 608 in driver. The entire contents */
/*  of klotz0 (pointer in buff, and buff) must be saved. */


  /* Parameter adjustments */
  --svblk;

  /* Function Body */
  svblk[1] = (double) klotz0_1.ptr;
  for (i = 1; i <= 607; ++i) {
    svblk[i + 1] = klotz0_1.buff[i - 1];
  }

  return;
}				/* zufallsv_ */

void zufallrs(double *svblk)
{
  int i;


/*  restores common block klotz0, containing seeds and pointer */
/*  to position in seed block. IMPORTANT: svblk must be */
/*  dimensioned at least 608 in driver. The entire contents */
/*  of klotz0 must be restored. */


  /* Parameter adjustments */
  --svblk;

  /* Function Body */
  klotz0_1.ptr = (int) svblk[1];
  for (i = 1; i <= 607; ++i) {
    klotz0_1.buff[i - 1] = svblk[i + 1];
  }

  return;
}				/* zufallrs_ */
#endif

#if 0
/* Subroutine */ int normalen_(int n, double *x)
{
  /* Initialized data */

  static int buffsz = 1024;

  /* System generated locals */
  int i__1;

  /* Local variables */
  int left, i, nn, ptr;
  extern /* Subroutine */ int normal00_(void);


/* Box-Muller method for Gaussian random numbers */

  /* Parameter adjustments */
  --x;

  /* Function Body */

  nn = n;
  if (nn <= 0) {
    return 0;
  }
  if (klotz1_1.first == 0) {
    normal00_();
    klotz1_1.first = 1;
  }
  ptr = 0;

L1:
  left = buffsz - klotz1_1.xptr;
  if (nn < left) {
    i__1 = nn;
    for (i = 1; i <= i__1; ++i) {
      x[i + ptr] = klotz1_1.xbuff[klotz1_1.xptr + i - 1];
    }
    klotz1_1.xptr += nn;
    return 0;
  } else {
    i__1 = left;
    for (i = 1; i <= i__1; ++i) {
      x[i + ptr] = klotz1_1.xbuff[klotz1_1.xptr + i - 1];
    }
    klotz1_1.xptr = 0;
    ptr += left;
    nn -= left;
    normal00_();
    goto L1;
  }
  return 0;
}				/* normalen_ */

int normal00_(void)
{
  /* Builtin functions */
  double cos(double), sin(double), log(double), sqrt(double);

  /* Local variables */
  int i;
  static double twopi = 6.2831853071795862;
  double r1, r2, t1, t2;
  /* extern int zufall_(int *, double *); */

  zufall_(1024, klotz1_1.xbuff);
  for (i = 1; i <= 1024; i += 2) {
    r1 = twopi * klotz1_1.xbuff[i - 1];
    t1 = cos(r1);
    t2 = sin(r1);
    r2 = sqrt(log(1.f - klotz1_1.xbuff[i]) * -2.f);
    klotz1_1.xbuff[i - 1] = t1 * r2;
    klotz1_1.xbuff[i] = t2 * r2;
  }

  return 0;
}				/* normal00_ */

int normalsv_(double *svbox)
{
  /* Builtin functions */
  int s_wsle(cilist *), do_lio(int *, int *, char *, ftnlen), e_wsle(void);

  /* Local variables */
  extern /* Subroutine */ int zufallsv_(double *);
  int i, k;

  /* Fortran I/O blocks */
  static cilist io___123 =
  {0, 6, 0, 0, 0};



/*  saves common block klotz0 containing buffers */
/*  and pointers. IMPORTANT: svbox must be dimensioned at */
/*  least 1634 in driver. The entire contents of blocks */
/*  klotz0 (via zufallsv) and klotz1 must be saved. */


  /* Parameter adjustments */
  --svbox;

/*  save zufall block klotz0 */

  zufallsv_(&svbox[1]);

  svbox[609] = (double) klotz1_1.first;
  svbox[610] = (double) klotz1_1.xptr;
  k = 610;
  for (i = 1; i <= 1024; ++i) {
    svbox[i + k] = klotz1_1.xbuff[i - 1];
  }

  return 0;
}				/* normalsv_ */

int normalrs_(double *svbox)
{
  /* Builtin functions */
  int s_wsle(cilist *), do_lio(int *, int *, char *, ftnlen), e_wsle(void);

  /* Local variables */
  extern /* Subroutine */ int zufallrs_(double *);
  int i, k;

  /* Fortran I/O blocks */
  static cilist io___126 =
  {0, 6, 0, 0, 0};



/*  restores common blocks klotz0, klotz1 containing buffers */
/*  and pointers. IMPORTANT: svbox must be dimensioned at */
/*  least 1634 in driver. The entire contents */
/*  of klotz0 and klotz1 must be restored. */


/* restore zufall blocks klotz0 and klotz1 */

  /* Parameter adjustments */
  --svbox;

  /* Function Body */
  zufallrs_(&svbox[1]);
  klotz1_1.first = (int) svbox[609];
  if (klotz1_1.first == 0) {
    s_wsle(&io___126);
    do_lio(&c__9, &c__1, " ERROR in normalsv, restoration of unitialized"
	   " block", 52L);
    e_wsle();
  }
  klotz1_1.xptr = (int) svbox[610];
  k = 610;
  for (i = 1; i <= 1024; ++i) {
    klotz1_1.xbuff[i - 1] = svbox[i + k];
/* L1: */
  }

  return 0;
}				/* normalrs_ */

/* Subroutine */ int fische_(int * n, double *mu, int * p)
{
  /* System generated locals */
  int i__1, i__2;

  /* Builtin functions */
  double exp(double);

  /* Local variables */
  int left, indx[1024], i, k;
  double q[1024], u[1024];
  int nsegs, p0;
  double q0;
  int ii, jj;
  extern /* Subroutine */ int zufall_(int *, double *);
  int nl0;
  double pmu;


/* Poisson generator for distribution function of p's: */

/*    q(mu,p) = exp(-mu) mu**p/p! */

/* initialize arrays, pointers */

  /* Parameter adjustments */
  --p;

  /* Function Body */
  if (*n <= 0) {
    return 0;
  }
  pmu = exp(-(*mu));
  p0 = 0;

  nsegs = (*n - 1) / 1024;
  left = *n - (nsegs << 10);
  ++nsegs;
  nl0 = left;

  i__1 = nsegs;
  for (k = 1; k <= i__1; ++k) {

    i__2 = left;
    for (i = 1; i <= i__2; ++i) {
      indx[i - 1] = i;
      p[p0 + i] = 0;
      q[i - 1] = 1.f;
    }

/* Begin iterative loop on segment of p's */

  L1:

/* Get the needed uniforms */

    zufall_(&left, u);

    jj = 0;

    i__2 = left;
    for (i = 1; i <= i__2; ++i) {
      ii = indx[i - 1];
      q0 = q[ii - 1] * u[i - 1];
      q[ii - 1] = q0;
      if (q0 > pmu) {
	++jj;
	indx[jj - 1] = ii;
	++p[p0 + ii];
      }
    }

/* any left in this segment? */

    left = jj;
    if (left > 0) {
      goto L1;
    }
    p0 += nl0;
    nl0 = 1024;
    left = 1024;
  }

  return 0;
}				/* fische_ */
#endif /* #if 0 */
