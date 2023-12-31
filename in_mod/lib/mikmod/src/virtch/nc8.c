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
 Module: nc8.c

 Description:
  Mix 8 bit data with volume ramping.  These functions use special global
  variables that differ fom the oes used by the normal mixers, so that they
  do not interfere with the true volume of the sound.

  (see v8 extern in wrap8.h)

*/

#include "mikmod.h"
#include "wrap8.h"


BOOL nc8_Check_Mono(uint channels, uint mixmode, uint format, uint flags)
{
    if((channels != MD_MONO) || (format)) return 0;

    return 1;
}


BOOL nc8_Check_MonoInterp(uint channels, uint mixmode, uint format, uint flags)
{
    if((channels != MD_MONO) || (format)) return 0;
    if(mixmode & DMODE_INTERP) return 1;

    return 0;
}


BOOL nc8_Check_Stereo(uint channels, uint mixmode, uint format, uint flags)
{
    if((channels != MD_STEREO) || (format)) return 0;
    return 1;
}


BOOL nc8_Check_StereoInterp(uint channels, uint mixmode, uint format, uint flags)
{
    if((channels != MD_STEREO) || (format)) return 0;
    if(mixmode & DMODE_INTERP) return 1;

    return 0;
}



// =====================================================================================
    void __cdecl Mix8StereoNormal_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   volinfo->left.vol    += volinfo->left.inc;
        volinfo->right.vol   += volinfo->right.inc;
        *dest++ += (int)(volinfo->left.vol  * srce[himacro(index)]);
        *dest++ += (int)(volinfo->right.vol * srce[himacro(index)]);
        index   += increment;
    }
}


// =====================================================================================
    void __cdecl Mix8StereoInterp_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   register SLONG  sroot  = srce[himacro(index)];

        sroot = (SBYTE)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS));

        volinfo->left.vol    += volinfo->left.inc;
        volinfo->right.vol   += volinfo->right.inc;

        *dest++  += (int)(volinfo->left.vol  * sroot);
        *dest++  += (int)(volinfo->right.vol * sroot);
        index    += increment;
    }
}

// =====================================================================================
    void __cdecl Mix8SurroundNormal_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   register SLONG sample;

        volinfo->left.vol += volinfo->left.inc;
        sample        = (int)(volinfo->left.vol * srce[himacro(index)]);

        *dest++ += sample;
        *dest++ -= sample;
        index   += increment;
    }
}

// =====================================================================================
    void __cdecl Mix8SurroundInterp_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)];
        volinfo->left.vol += volinfo->left.inc;
        sroot         = (int)(volinfo->left.vol * (SBYTE)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS)));

        *dest++ += sroot;
        *dest++ -= sroot;
        index   += increment;
    }
}

// =====================================================================================
    void __cdecl Mix8MonoNormal_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    SLONG  sample;

    for(; todo; todo--)
    {   volinfo->left.vol  += volinfo->left.inc;
        sample        = (int)(volinfo->left.vol * srce[himacro(index)]);

        *dest++ += sample;
        index   += increment;
    }
}


// =====================================================================================
    void __cdecl Mix8MonoInterp_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)];
        volinfo->left.vol += volinfo->left.inc;
        sroot         = (int)(volinfo->left.vol * (SBYTE)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS)));

        *dest++ += sroot;
        index   += increment;
    }
}
