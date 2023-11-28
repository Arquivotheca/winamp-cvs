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
 Module: nc16.c

 Description:
  Mix 16 bit *STEREO* (oh yea!) data with volume ramping.  These functions 
  use special global variables that differ fom the ones used by the normal 
  mixers, so that they do not interfere with the true volume of the sound.

  (see v16 extern in wrap16.h)

*/

#include "mikmod.h"
#include "wrap16.h"


BOOL nc16ss_Check_Mono(uint channels, uint mixmode, uint format, uint flags)
{
    if((channels != MD_MONO) || (format != (SF_16BITS | SF_STEREO))) return 0;

    return 1;
}


BOOL nc16ss_Check_MonoInterp(uint channels, uint mixmode, uint format, uint flags)
{
    if((channels != MD_MONO) || (format != (SF_16BITS | SF_STEREO))) return 0;
    if(mixmode & DMODE_INTERP) return 1;

    return 0;
}


BOOL nc16ss_Check_Stereo(uint channels, uint mixmode, uint format, uint flags)
{
    if((channels != MD_STEREO) || (format != (SF_16BITS | SF_STEREO))) return 0;
    return 1;
}


BOOL nc16ss_Check_StereoInterp(uint channels, uint mixmode, uint format, uint flags)
{
    if((channels != MD_STEREO) || (format != (SF_16BITS | SF_STEREO))) return 0;
    if(mixmode & DMODE_INTERP) return 1;

    return 0;
}



// =====================================================================================
    void __cdecl Mix16StereoSS_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   volinfo->left.vol    += volinfo->left.inc;
        volinfo->right.vol   += volinfo->right.inc;
        *dest++ += (volinfo->left.vol  / BIT16_VOLFAC) * srce[(himacro(index)*2)];
        *dest++ += (volinfo->right.vol / BIT16_VOLFAC) * srce[(himacro(index)*2)+1];
        index  += increment;
    }
}


// =====================================================================================
    void __cdecl Mix16StereoSSI_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
	for(; todo; todo--)
    {   register SLONG  sroot;

        volinfo->left.vol   += volinfo->left.inc;
        volinfo->right.vol  += volinfo->right.inc;

        sroot = srce[(himacro(index)*2)];
        *dest++  += (volinfo->left.vol  / BIT16_VOLFAC) * (SWORD)(sroot + ((((SLONG)srce[(himacro(index)*2) + 2] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS));

        sroot = srce[(himacro(index)*2) + 1];
        *dest++  += (volinfo->right.vol / BIT16_VOLFAC) * (SWORD)(sroot + ((((SLONG)srce[(himacro(index)*2) + 3] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS));


        index   += increment;
    }
}


// =====================================================================================
    void __cdecl Mix16MonoSS_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   volinfo->left.vol     += volinfo->left.inc;
        *dest++          += ((volinfo->left.vol  / BIT16_VOLFAC) * (srce[himacro(index)] + srce[(himacro(index)*2)+1])) / 2;
        index   += increment;
    }
}


// =====================================================================================
    void __cdecl Mix16MonoSSI_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)*2], crap;
        volinfo->left.vol += volinfo->left.inc;
        crap  = (sroot + ((((SLONG)srce[(himacro(index)*2) + 2] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS));

        sroot = srce[(himacro(index)*2) + 1];        
        *dest++ += (volinfo->left.vol  / BIT16_VOLFAC) * (crap + (sroot + ((((SLONG)srce[(himacro(index)*2) + 2] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS))) / 2;

        index   += increment;
    }
}
