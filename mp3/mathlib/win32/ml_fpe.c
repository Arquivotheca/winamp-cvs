/******************************************************************************

                       (C) copyright Fraunhofer-IIS (1997-2005)
                               All Rights Reserved

   $Id: ml_fpe.c,v 1.1 2009/04/28 20:17:43 audiodsp Exp $
   Initial author:       W. Schildbach
   contents/description: Mathlib V2.0 functions to enable/disable floating
                         point exceptions

   This software and/or program is protected by copyright law and
   international treaties. Any reproduction or distribution of this 
   software and/or program, or any portion of it, may result in severe 
   civil and criminal penalties, and will be prosecuted to the maximum 
   extent possible under law.

******************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <float.h>

#include "mathlib.h"

void enableFPEs(void)
{
  _controlfp(_EM_INEXACT|_EM_DENORMAL|_EM_UNDERFLOW,_MCW_EM);
}

void disableFPEs(void)
{
  _controlfp(_MCW_EM,_MCW_EM);
}
