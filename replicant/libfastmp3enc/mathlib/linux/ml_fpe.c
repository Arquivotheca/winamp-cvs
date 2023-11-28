/******************************** MPEG Audio Encoder **************************

                       (C) copyright Fraunhofer-IIS (1997)
                               All Rights Reserved

   $Id: ml_fpe.c,v 1.1 2007/05/29 16:02:34 audiodsp Exp $
   Initial author:       W. Schildbach
   contents/description: Mathlib V2.0 functions to enable/disable floating
                         point exceptions on linux systems

    This software and/or program is protected by copyright law and
    international treaties. Any reproduction or distribution of this 
    software and/or program, or any portion of it, may result in severe 
    civil and criminal penalties, and will be prosecuted to the maximum 
    extent possible under law.
 
******************************************************************************/

#ifdef linux

#include <math.h>
#include <stdlib.h>
#include <float.h>

#include "mathlib.h"
#include "controlfp.h"

void enableFPEs(void)
{
  _controlfp((unsigned short)_EM_INEXACT|_EM_DENORMAL|_EM_UNDERFLOW,(unsigned short)_MCW_EM);
}

void disableFPEs(void)
{
  _controlfp((unsigned short)_MCW_EM,(unsigned short)_MCW_EM);
}

#endif /* ifdef linux */
