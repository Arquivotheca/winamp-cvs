/*****************************************************************************

                       (C) copyright Fraunhofer-IIS (1997-2005)
                               All Rights Reserved

   $Id: controlfp.h,v 1.1 2007/05/29 16:02:34 audiodsp Exp $
   Initial author:       W. Schildbach
   contents/description: function to set fp register control bits

   This software and/or program is protected by copyright law and
   international treaties. Any reproduction or distribution of this 
   software and/or program, or any portion of it, may result in severe 
   civil and criminal penalties, and will be prosecuted to the maximum 
   extent possible under law.

******************************************************************************/

#ifndef _controlfp_h
#define _controlfp_h

#if defined(__GNUC__) && defined(i386)

enum {
  _EM_INVALID    = 1,
  _EM_DENORMAL   = 2,
  _EM_ZERODIVIDE = 4,
  _EM_OVERFLOW   = 8,
  _EM_UNDERFLOW  = 16,
  _EM_PRECISION  = 32,
  _EM_INEXACT    = _EM_PRECISION
};

#define _MCW_EM (_EM_INVALID|_EM_DENORMAL|_EM_ZERODIVIDE|_EM_OVERFLOW|\
                 _EM_UNDERFLOW|_EM_INEXACT)

#define _MCW_RC         0x00000c00              /* Rounding Control */
#define _RC_NEAR        0x00000000              /*   near */
#define _RC_DOWN        0x00000400              /*   down */
#define _RC_UP          0x00000800              /*   up */
#define _RC_CHOP        0x00000c00              /*   chop */

extern unsigned short
_controlfp(unsigned short _newcw, unsigned short _mask);

#endif /* end of "if GNU_C and i386" section */

#endif /* _controlfp_h */
