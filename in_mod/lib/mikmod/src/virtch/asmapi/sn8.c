/*
 MikMod Sound System

  By Jake Stine of Divine Entertainment (1996-2000)

 Support:
  If you find problems with this code, send mail to:
    air@divent.org

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 -----------------------------------------
 Module: sn8.c

  Assembly Mixer Plugin : Stereo 8 bit Sample data
  
  Generic all-purpose assembly wrapper for mikmod VMIXER plugin thingie.
  As long as your platform-dependant asm code uses the naming conventions
  below (Asm StereoInterp, SurroundInterp, etc), then you can use this
  module and the others related to it to register your mixer into mikmod.

  See Also:

    mi8.c
    mn8.c
    sn8.c
    si8.c
    mn16.c
    sn16.c
    mi16.c
    si16.c
*/

#include "mikmod.h"
#include "..\wrap8.h"
#include "asmapi.h"

VMIXER ASM_S8_MONO =
{   NULL,

    BLAH("Assembly Stereo-8 (Mono) v0.1"),

    nc8ss_Check_Mono,

    NULL,
    NULL,
    VC_Volcalc8_Mono,
    VC_Volramp8_Mono,
    Mix8MonoSS_NoClick,
    Mix8MonoSS_NoClick,
    AsmMonoSS,
    AsmMonoSS,
};

    
VMIXER ASM_S8_STEREO =
{   NULL,

    BLAH("Assembly Stereo-8 (Stereo) v0.1"),

    nc8ss_Check_Stereo,

    NULL,
    NULL,
    VC_Volcalc8_Stereo,
    VC_Volramp8_Stereo,
    Mix8StereoSS_NoClick,
    Mix8StereoSS_NoClick,
    AsmStereoSS,
    AsmStereoSS,
};
