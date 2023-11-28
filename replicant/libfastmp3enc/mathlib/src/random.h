/*****************************************************************************

                       (C) copyright Fraunhofer-IIS (1997-2005)
                               All Rights Reserved

   This software and/or program is protected by copyright law and
   international treaties. Any reproduction or distribution of this
   software and/or program, or any portion of it, may result in severe
   civil and criminal penalties, and will be prosecuted to the maximum
   extent possible under law.

   $Id: random.h,v 1.1 2007/05/29 16:02:35 audiodsp Exp $

******************************************************************************/

/* README for zufall random number package */
/* ------ --- ------ ------ ------ ------- */
/* This package contains a portable random number generator set */
/* for: uniform (u in [0,1)), normal (<g> = 0, <g^2> = 1), and */
/* Poisson distributions. The basic module, the uniform generator, */
/* uses a lagged Fibonacci series generator: */

/*               t    = u(n-273) + u(n-607) */
/*               u(n) = t - float(int(t)) */

/* where each number generated, u(k), is floating point. Since */
/* the numbers are floating point, the left end boundary of the */
/* range contains zero. This package is nearly portable except */
/* for the following. It is written in lower case. */

/* To compile this beast, note that all floating point numbers */
/* are declared 'double precision'. On Cray X-MP, Y-MP, and C-90 */
/* machines, use the cft77 (cf77) option -dp to run this in 64 */
/* bit mode (not 128 bit double). */

/* External documentation, "Lagged Fibonacci Random Number Generators */
/* for the NEC SX-3," is to be published in the International */
/* Journal of High Speed Computing (1994). Otherwise, ask the */
/* author: */

/*          W. P. Petersen */
/*          IPS, RZ F-5 */
/*          ETHZ */
/*          CH 8092, Zurich */
/*          Switzerland */

/* e-mail:  wpp@ips.ethz.ch. */

/* The package contains the following routines: */

/* ------------------------------------------------------ */
/* UNIFORM generator routines: */

/*       subroutine zufalli(seed) */
/*       integer seed */
/* c initializes common block containing seeds. if seed=0, */
/* c the default value is 1802. */

/*       subroutine zufall(n,u) */
/*       integer n */
/*       double precision u(n) */
/* c returns set of n uniforms u(1), ..., u(n). */

/*       subroutine zufallsv(zusave) */
/*       double precision zusave(608) */
/* c saves buffer and pointer in zusave, for later restarts */

/*       subroutine zufallrs(zusave) */
/*       double precision zusave(608) */
/* c restores seed buffer and pointer from zusave */
/* ------------------------------------------------------ */

#ifndef _random_h
#define _random_h

/*#include "bastypes.h"*/

void zufall(int n, double *a);
void zufalli(int seed);
void zufallsv(double *mem);
void zufallrs(double *mem);

#endif
