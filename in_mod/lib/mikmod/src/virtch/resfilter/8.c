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
 Module: vc8.c

  Low-level mixer functions for mixing 8 bit sample data.  Includes normal and
  interpolated mixing, without declicking (no microramps, see vcmix8_noclick.c
  for those).

*/

#include "mikmod.h"
#include "..\wrap8.h"

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

    return resfilter->pos>>10;   //>>VC_COFACTOR;
    */

    return sroot;
}


// =====================================================================================
static void VC_ResVolcalc8_Mono(VIRTCH *vc, VINFO *vnf)
// =====================================================================================
{
	vnf->vol.front.left  = ((vnf->volume.front.left+vnf->volume.front.right) * (vc->volume.front.left+vc->volume.front.right) * BIT8_VOLFAC) / 3;

	vnf->volinfo.lvolsel = vnf->vol.front.left;

	// declicker: Set us up to volramp!
	if(vc->mode & DMODE_NOCLICK)
	{   if(vnf->vol.front.left != vnf->oldvol.front.left)
	{ 
		if(!vnf->volramp) 
			vnf->volramp = RAMPLEN_VOLUME;
		vnf->volinfo.left.inc = ((vnf->vol.front.left - (vnf->volinfo.left.vol = vnf->oldvol.front.left)) / (int)vnf->volramp);
	} 
	else if(vnf->volramp)
	{   
		vnf->volinfo.left.inc  = ((vnf->vol.front.left - (vnf->volinfo.left.vol = vnf->oldvol.front.left)) / (int)vnf->volramp);
		if(!vnf->volinfo.left.inc) 
			vnf->volramp = 0;
	}
	}

	VC_CalcResonance(vc, vnf);
}


// =====================================================================================
    static void VC_ResVolcalc8_Stereo(VIRTCH *vc, VINFO *vnf)
// =====================================================================================
{
    vnf->vol.front.left  = vnf->volume.front.left  * vc->volume.front.left  * BIT8_VOLFAC;
    vnf->vol.front.right = vnf->volume.front.right * vc->volume.front.right * BIT8_VOLFAC;

    vnf->volinfo.lvolsel = vnf->vol.front.left;
    vnf->volinfo.rvolsel = vnf->vol.front.right;

    // declicker: Set us up to volramp!
    if(vc->mode & DMODE_NOCLICK)
    {   
        if((vnf->vol.front.left != vnf->oldvol.front.left) || (vnf->vol.front.right != vnf->oldvol.front.right))
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
    void __cdecl Res8StereoNormal(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    SBYTE  sample;

    for(; todo; todo--)
    {
        sample = filter(srce[himacro(index)], resfilter);
        index += increment;

        *dest++ += volinfo->lvolsel * sample;
        *dest++ += volinfo->rvolsel * sample;
    }
}


// =====================================================================================
    void __cdecl Res8StereoInterp(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   register SLONG  sroot  = srce[himacro(index)];
        sroot = filter((SBYTE)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS)), resfilter);

        *dest++ += volinfo->lvolsel * sroot;
        *dest++ += volinfo->rvolsel * sroot;

        index  += increment;
    }
}


// =====================================================================================
    void __cdecl Res8SurroundNormal(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    SLONG  sample;

    for (; todo; todo--)
    {   sample = volinfo->lvolsel * filter(srce[himacro(index)], resfilter);
        index += increment;

        *dest++ += sample;
        *dest++ -= sample;
    }
}


// =====================================================================================
    void __cdecl Res8SurroundInterp(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for (; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)];
        sroot = volinfo->lvolsel * filter((SBYTE)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS)), resfilter);

        *dest++ += sroot;
        *dest++ -= sroot;
        index  += increment;
    }
}


// =====================================================================================
    void __cdecl Res8MonoNormal(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   *dest++ += volinfo->lvolsel * filter(srce[himacro(index)], resfilter);
        index  += increment;
    }
}


// =====================================================================================
    void __cdecl Res8MonoInterp(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   
			register SLONG  sroot = srce[himacro(index)];
        sroot = volinfo->lvolsel * filter((SBYTE)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS)), resfilter);
        *dest++ += sroot;
        index  += increment;
    }
}


// =====================================================================================
    void __cdecl Res8StereoNormal_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   
        volinfo->left.vol    += volinfo->left.inc;
        volinfo->right.vol   += volinfo->right.inc;
        {
        register SLONG sample = filter(srce[himacro(index)], resfilter);
        *dest++ += volinfo->left.vol   * sample;
        *dest++ += volinfo->right.vol  * sample;
        }
        index  += increment;
    }
}


// =====================================================================================
    void __cdecl Res8StereoInterp_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   register SLONG  sroot  = srce[himacro(index)];
        sroot = filter((SBYTE)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS)), resfilter);

        volinfo->left.vol    += volinfo->left.inc;
        volinfo->right.vol   += volinfo->right.inc;

        *dest++  += volinfo->left.vol  * sroot;
        *dest++  += volinfo->right.vol * sroot;
        index   += increment;
    }
}


// =====================================================================================
    void __cdecl Res8SurroundNormal_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   register SLONG sample;

        volinfo->left.vol += volinfo->left.inc;
        sample       = volinfo->left.vol * filter(srce[himacro(index)], resfilter);

        *dest++ += sample;
        *dest++ -= sample;
        index   += increment;
    }
}


// =====================================================================================
    void __cdecl Res8SurroundInterp_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)];
        volinfo->left.vol += volinfo->left.inc;
        sroot        = volinfo->left.vol  * filter((SBYTE)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS)), resfilter);

        *dest++ += sroot;
        *dest++ -= sroot;
        index  += increment;
    }
}


// =====================================================================================
    void __cdecl Res8MonoNormal_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    SLONG  sample;

    for(; todo; todo--)
    {   volinfo->left.vol  += volinfo->left.inc;
        sample        = volinfo->left.vol  * filter(srce[himacro(index)], resfilter);

        *dest++ += sample;
        index  += increment;
    }
}


// =====================================================================================
    void __cdecl Res8MonoInterp_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
// =====================================================================================
{
    for(; todo; todo--)
    {   register SLONG  sroot = srce[himacro(index)];
        volinfo->left.vol += volinfo->left.inc;
        sroot        = volinfo->left.vol * filter((SBYTE)(sroot + ((((SLONG)srce[himacro(index) + 1] - sroot) * (lomacro(index)>>INTERPBITS)) >> INTERPBITS)), resfilter);

        *dest++ += sroot;
        index   += increment;
    }
}


static BOOL Check_Mono(uint channels, uint mixmode, uint format, uint flags)
{
    if((channels != MD_MONO) || (format)) return 0;
    if(flags & SL_RESONANCE_FILTER) return 1;

    return 0;
}


static BOOL Check_MonoInterp(uint channels, uint mixmode, uint format, uint flags)
{
    if((channels != MD_MONO) || (format)) return 0;
    if((flags & SL_RESONANCE_FILTER) && (mixmode & DMODE_INTERP)) return 1;

    return 0;
}


static BOOL Check_Stereo(uint channels, uint mixmode, uint format, uint flags)
{
    if((channels != MD_STEREO) || (format)) return 0;
    if(flags & SL_RESONANCE_FILTER) return 1;

    return 0;
}


static BOOL Check_StereoInterp(uint channels, uint mixmode, uint format, uint flags)
{
    if((channels != MD_STEREO) || (format)) return 0;
    if((flags & SL_RESONANCE_FILTER) && (mixmode & DMODE_INTERP)) return 1;

    return 0;
}



VMIXER RF_M8_MONO_INTERP =
{   NULL,

    BLAH("Resonance Mono-8 (Mono/Interp) v0.1"),

    Check_MonoInterp,

    NULL,
    NULL,

    VC_ResVolcalc8_Mono,
    VC_Volramp8_Mono,
    Res8MonoInterp_NoClick,
    Res8MonoInterp_NoClick,
    Res8MonoInterp,
    Res8MonoInterp,
};

VMIXER RF_M8_STEREO_INTERP =
{   NULL,

    BLAH("Resonance Mono-8 (Stereo/Interp) v0.1"),

    Check_StereoInterp,

    NULL,
    NULL,

    VC_ResVolcalc8_Stereo,
    VC_Volramp8_Stereo,
    Res8StereoInterp_NoClick,
    Res8SurroundInterp_NoClick,
    Res8StereoInterp,
    Res8SurroundInterp,
};

VMIXER RF_M8_MONO =
{   NULL,

    BLAH("Resonance Mono-8 (Mono) v0.1"),

    Check_Mono,

    NULL,
    NULL,

    VC_ResVolcalc8_Mono,
    VC_Volramp8_Mono,
    Res8MonoNormal_NoClick,
    Res8MonoNormal_NoClick,
    Res8MonoNormal,
    Res8MonoNormal,
};

VMIXER RF_M8_STEREO =
{   NULL,

	BLAH("Resonance Mono-8 (Stereo) v0.1"),

    Check_Stereo,

    NULL,
    NULL,

    VC_ResVolcalc8_Stereo,
    VC_Volramp8_Stereo,
    Res8StereoNormal_NoClick,
    Res8SurroundNormal_NoClick,
    Res8StereoNormal,
    Res8SurroundNormal,
};
