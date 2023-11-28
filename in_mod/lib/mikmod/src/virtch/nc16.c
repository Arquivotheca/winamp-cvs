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
  Mix 16 bit data with volume ramping.  These functions use special global
  variables that differ fom the oes used by the normal mixers, so that they
  do not interfere with the true volume of the sound.

  (see v16 extern in wrap16.h)

*/

#include "mikmod.h"
#include "wrap16.h"


BOOL nc16_Check_Mono(uint channels, uint mixmode, uint format, uint flags)
{
    if((channels != MD_MONO) || (format != SF_16BITS)) return 0;

    return 1;
}


BOOL nc16_Check_MonoInterp(uint channels, uint mixmode, uint format, uint flags)
{
    if((channels != MD_MONO) || (format != SF_16BITS)) return 0;
    if(mixmode & DMODE_INTERP) return 1;

    return 0;
}


BOOL nc16_Check_Stereo(uint channels, uint mixmode, uint format, uint flags)
{
    if((channels != MD_STEREO) || (format != SF_16BITS)) return 0;
    return 1;
}


BOOL nc16_Check_StereoInterp(uint channels, uint mixmode, uint format, uint flags)
{
    if((channels != MD_STEREO) || (format != SF_16BITS)) return 0;
    if(mixmode & DMODE_INTERP) return 1;

    return 0;
}


// =====================================================================================
    void __cdecl Mix16StereoNormal_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   volinfo->left.vol    += volinfo->left.inc;
        volinfo->right.vol   += volinfo->right.inc;
        *dest++ += (int)(volinfo->left.vol  * srce[himacro(index)]) / BIT16_VOLFAC;
        *dest++ += (int)(volinfo->right.vol * srce[himacro(index)]) / BIT16_VOLFAC;
        index  += increment;
    }
}


// =====================================================================================
    void __cdecl Mix16StereoInterp_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   register SLONG  sroot  = srce[himacro(index)];

        volinfo->left.vol    += volinfo->left.inc;
        volinfo->right.vol   += volinfo->right.inc;

        sroot = (SWORD)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS));

        *dest++  += (int)(volinfo->left.vol  * sroot) / BIT16_VOLFAC;
        *dest++  += (int)(volinfo->right.vol * sroot) / BIT16_VOLFAC;
        index    += increment;
    }
}


// =====================================================================================
    void __cdecl Mix16SurroundNormal_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   register SLONG sample;

        volinfo->left.vol += volinfo->left.inc;
        sample        = (int)(volinfo->left.vol * srce[himacro(index)]) / BIT16_VOLFAC;

        *dest++ += sample;
        *dest++ -= sample;
        index   += increment;
    }
}


// =====================================================================================
    void __cdecl Mix16SurroundInterp_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)];
        volinfo->left.vol += volinfo->left.inc;
        sroot         = (int)(volinfo->left.vol * (SWORD)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS))) / BIT16_VOLFAC;

        *dest++ += sroot;
        *dest++ -= sroot;
        index  += increment;
    }
}


// =====================================================================================
    void __cdecl Mix16MonoNormal_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    SLONG  sample;

    for(; todo; todo--)
    {   volinfo->left.vol  += volinfo->left.inc;
        sample         = (int)(volinfo->left.vol * srce[himacro(index)])  / BIT16_VOLFAC;

        *dest++ += sample;
        index  += increment;
    }
}


// =====================================================================================
    void __cdecl Mix16MonoInterp_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)];
        volinfo->left.vol += volinfo->left.inc;
        sroot         = (int)(volinfo->left.vol * (SWORD)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS)))  / BIT16_VOLFAC;

        *dest++ += sroot;
        index   += increment;
    }
}
