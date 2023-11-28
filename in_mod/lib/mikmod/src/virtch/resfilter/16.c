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
 Module: resfilter/16.c

 Description:
  Low-level mixer functions for mixing 16 bit sample data.  Includes normal and
  interpolated mixing, without declicking (no microramps, see vc16nc.c
  for those).

*/

#include "mikmod.h"
#include "..\wrap16.h"

#include "resshare.h"

// =====================================================================================
    static SLONG __inline filter(SLONG sroot, VC_RESFILTER *resfilter)
// =====================================================================================
{
    //TEST
    //
    // rewrite the damned thing!
    //

    /*
    resfilter->speed += (((sroot<<10) - resfilter->pos) * resfilter->cofactor) >> VC_COFACTOR;
    resfilter->pos   += resfilter->speed;

    resfilter->speed *= resfilter->resfactor;
    resfilter->speed >>= VC_RESFACTOR;

    return resfilter->pos>>10;
    */

    return sroot;
}


// =====================================================================================
    static void VC_ResVolcalc16_Mono(VIRTCH *vc, VINFO *vnf)
// =====================================================================================
{
    vnf->vol.front.left  = ((vnf->volume.front.left+vnf->volume.front.right) * (vc->volume.front.left+vc->volume.front.right)) / 3;

    vnf->volinfo.lvolsel = vnf->vol.front.left / BIT16_VOLFAC;

    // declicker: Set us up to volramp!
    if(vc->mode & DMODE_NOCLICK)
    {   if(vnf->vol.front.left != vnf->oldvol.front.left)
        {   if(!vnf->volramp) vnf->volramp = RAMPLEN_VOLUME;
            vnf->volinfo.left.inc = vnf->volinfo.right.inc = ((vnf->vol.front.left - (vnf->volinfo.left.vol = vnf->oldvol.front.left)) / (int)vnf->volramp);
        } else if(vnf->volramp)
        {   vnf->volinfo.left.inc  = ((vnf->vol.front.left - (vnf->volinfo.left.vol = vnf->oldvol.front.left)) / (int)vnf->volramp);
            if(!vnf->volinfo.left.inc) vnf->volramp = 0;
        }
    }

    VC_CalcResonance(vc, vnf);
}


// =====================================================================================
    static void VC_ResVolcalc16_Stereo(VIRTCH *vc, VINFO *vnf)
// =====================================================================================
{
    vnf->vol.front.left  = vnf->volume.front.left  * vc->volume.front.left;
    vnf->vol.front.right = vnf->volume.front.right * vc->volume.front.right;

    vnf->volinfo.lvolsel = vnf->vol.front.left / BIT16_VOLFAC;
    vnf->volinfo.rvolsel = vnf->vol.front.right / BIT16_VOLFAC;

    // declicker: Set us up to volramp!
    if(vc->mode & DMODE_NOCLICK)
    {   if((vnf->vol.front.left != vnf->oldvol.front.left) || (vnf->vol.front.right != vnf->oldvol.front.right))
        {   if(!vnf->volramp) vnf->volramp = RAMPLEN_VOLUME;
            vnf->volinfo.left.inc  = ((vnf->vol.front.left - (vnf->volinfo.left.vol = vnf->oldvol.front.left)) / (int)vnf->volramp);
            vnf->volinfo.right.inc = ((vnf->vol.front.right - (vnf->volinfo.right.vol = vnf->oldvol.front.right)) / (int)vnf->volramp);
        } else if(vnf->volramp)
        {   vnf->volinfo.left.inc  = ((vnf->vol.front.left - (vnf->volinfo.left.vol = vnf->oldvol.front.left)) / (int)vnf->volramp);
            vnf->volinfo.right.inc = ((vnf->vol.front.right - (vnf->volinfo.right.vol = vnf->oldvol.front.right)) / (int)vnf->volramp);
            if(!vnf->volinfo.left.inc && !vnf->volinfo.right.inc) vnf->volramp = 0;
        }
    }

    VC_CalcResonance(vc, vnf);
}


// =====================================================================================
    void __cdecl Res16StereoNormal(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    SLONG  sample;

    for(; todo; todo--)
    {
        sample = filter(srce[himacro(index)], resfilter);
        index += increment;

        *dest++ += volinfo->lvolsel * sample;
        *dest++ += volinfo->rvolsel * sample;
    }
}


// =====================================================================================
    void __cdecl Res16StereoInterp(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   register SLONG  sroot  = srce[himacro(index)];
        sroot = filter((SWORD)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS)), resfilter);

        *dest++ += volinfo->lvolsel * sroot;
        *dest++ += volinfo->rvolsel * sroot;

        index  += increment;
    }
}


// =====================================================================================
    void __cdecl Res16SurroundNormal(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    SLONG  sample;

    for (; todo; todo--)
    {   
			sample = volinfo->lvolsel * filter(srce[himacro(index)], resfilter);
        index += increment;

        *dest++ += sample;
        *dest++ -= sample;
    }
}


// =====================================================================================
    void __cdecl Res16SurroundInterp(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for (; todo; todo--)
    {   
			register SLONG  sroot = srce[himacro(index)];
        sroot = volinfo->lvolsel * filter((SWORD)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS)), resfilter);

        *dest++ += sroot;
        *dest++ -= sroot;
        index   += increment;
    }
}


// =====================================================================================
    void __cdecl Res16MonoNormal(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   
			*dest++ += volinfo->lvolsel * filter(srce[himacro(index)], resfilter);
        index  += increment;
    }
}


// =====================================================================================
    void __cdecl Res16MonoInterp(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   
			register SLONG  sroot = srce[himacro(index)];
        sroot = volinfo->lvolsel * filter((SWORD)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS)), resfilter);
        *dest++ += sroot;
        index  += increment;
    }
}


// =====================================================================================
    void __cdecl Res16StereoNormal_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {
        volinfo->left.vol    += volinfo->left.inc;
        volinfo->right.vol   += volinfo->right.inc;
        {
        register SLONG sample = filter(srce[himacro(index)], resfilter);
        *dest++ += (volinfo->left.vol  / BIT16_VOLFAC) * sample;
        *dest++ += (volinfo->right.vol / BIT16_VOLFAC) * sample;
        }
        index  += increment;
    }
}


// =====================================================================================
    void __cdecl Res16StereoInterp_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   register SLONG  sroot  = srce[himacro(index)];

        volinfo->left.vol    += volinfo->left.inc;
        volinfo->right.vol   += volinfo->right.inc;

        sroot = filter((SWORD)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS)), resfilter);

        *dest++  += (volinfo->left.vol  / BIT16_VOLFAC) * sroot;
        *dest++  += (volinfo->right.vol / BIT16_VOLFAC) * sroot;
        index    += increment;
    }
}


// =====================================================================================
    void __cdecl Res16SurroundNormal_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   register SLONG sample;

        volinfo->left.vol += volinfo->left.inc;
        sample        = (volinfo->left.vol / BIT16_VOLFAC) * filter(srce[himacro(index)], resfilter);

        *dest++ += sample;
        *dest++ -= sample;
        index   += increment;
    }
}


// =====================================================================================
    void __cdecl Res16SurroundInterp_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)];
        volinfo->left.vol += volinfo->left.inc;
        sroot         = (volinfo->left.vol  / BIT16_VOLFAC) * filter((SWORD)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS)), resfilter);

        *dest++ += sroot;
        *dest++ -= sroot;
        index   += increment;
    }
}


// =====================================================================================
    void __cdecl Res16MonoNormal_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    SLONG  sample;

    for(; todo; todo--)
    {   volinfo->left.vol  += volinfo->left.inc;
        sample         = (volinfo->left.vol  / BIT16_VOLFAC) * filter(srce[himacro(index)], resfilter);

        *dest++ += sample;
        index   += increment;
    }
}


// =====================================================================================
    void __cdecl Res16MonoInterp_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)];
        volinfo->left.vol += volinfo->left.inc;
        sroot         = (volinfo->left.vol  / BIT16_VOLFAC) * filter((SWORD)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS)), resfilter);

        *dest++ += sroot;
        index   += increment;
    }
}


static BOOL Check_Mono(uint channels, uint mixmode, uint format, uint flags)
{
    if((channels != MD_MONO) || (format != SF_16BITS)) return 0;
    if(flags & SL_RESONANCE_FILTER) return 1;

    return 0;
}


static BOOL Check_MonoInterp(uint channels, uint mixmode, uint format, uint flags)
{
    if((channels != MD_MONO) || (format != SF_16BITS)) return 0;
    if((flags & SL_RESONANCE_FILTER) && (mixmode & DMODE_INTERP)) return 1;

    return 0;
}


static BOOL Check_Stereo(uint channels, uint mixmode, uint format, uint flags)
{
    if((channels != MD_STEREO) || (format != SF_16BITS)) return 0;
    if(flags & SL_RESONANCE_FILTER) return 1;

    return 0;
}


static BOOL Check_StereoInterp(uint channels, uint mixmode, uint format, uint flags)
{
    if((channels != MD_STEREO) || (format != SF_16BITS)) return 0;
    if((flags & SL_RESONANCE_FILTER) && (mixmode & DMODE_INTERP)) return 1;

    return 0;
}


VMIXER RF_M16_MONO_INTERP =
{   NULL,

    BLAH("Resonance Mono-16 (Mono/Interp) v0.1"),

    Check_MonoInterp,

    NULL,
    NULL,

    VC_ResVolcalc16_Mono,
    VC_Volramp16_Mono,
    Res16MonoInterp_NoClick,
    Res16MonoInterp_NoClick,
    Res16MonoInterp,
    Res16MonoInterp,
};

VMIXER RF_M16_STEREO_INTERP =
{   NULL,

    BLAH("Resonance Mono-16 (Stereo/Interp) v0.1"),

    Check_StereoInterp,

    NULL,
    NULL,

    VC_ResVolcalc16_Stereo,
    VC_Volramp16_Stereo,
    Res16StereoInterp_NoClick,
    Res16SurroundInterp_NoClick,
    Res16StereoInterp,
    Res16SurroundInterp,
};


VMIXER RF_M16_MONO =
{   NULL,

    BLAH("Resonance Mono-16 (Mono) v0.1"),

    Check_Mono,
    
    NULL,
    NULL,

    VC_ResVolcalc16_Mono,
    VC_Volramp16_Mono,
    Res16MonoNormal_NoClick,
    Res16MonoNormal_NoClick,
    Res16MonoNormal,
    Res16MonoNormal,
};


VMIXER RF_M16_STEREO =
{   NULL,

    BLAH("Resonance Mono-16 (Stereo) v0.1"),

    Check_Stereo,
    
    NULL,
    NULL,
    
    VC_ResVolcalc16_Stereo,
    VC_Volramp16_Stereo,
    Res16StereoNormal_NoClick,
    Res16SurroundNormal_NoClick,
    Res16StereoNormal,
    Res16SurroundNormal,
};
