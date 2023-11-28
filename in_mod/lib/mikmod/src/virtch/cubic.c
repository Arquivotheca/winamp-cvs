//ph33r d4 cub1c 1nt3rp0l4t10n

#include "mikmod.h"
#include "wrap16.h"
#include "wrap8.h"


typedef struct
{
	float ia,ib,ic,s1;
	long prev;
	BOOL ok;
} CUBIC_DAT;


//#define DO_CALC cubic_calc_ptr(dat,s*lomacro(index))
/*
#define DO_INCREMENT \
        i1=increment;	\
		while(1)	\
		{	\
			INT64S d;\
			DWORD x;\
			d=-s*lomacro(index); \
			if (!d) d=0x100000000;\
			if (i1<d)	\
			{	\
				if (s>0) index+=i1; else index-=i1;	\
				break;	\
			}	\
			if (s>0) index+=d; else index-=d;	\
			i1-=d;	\
			x=himacro(index);	\
			cubic_advance_ptr(dat,srce[x],srce[x+s],srce[x+(s<<1)]);	\
		}
*/

static void _fastcall cubic_advance(CUBIC_DAT * dat,int sample1,int sample2,int sample3)//calculate spline for sample1-sample2 segment
{//from in_tfmx
	{//calc a/b/c
		const float h=(float)((INT64S)1<<(FRACBITS-1));///hack
/*
c=s'(x1)
d=s(x1)=y1
a =  0.4* ( d_y - c*h - d_s'/2 * h ) / h3
b=(s'(x2)-3ah2-c)/2h
*/			
		float new_s1=((float)sample3-(float)sample1)/(2*h);

		float fh=(float)sample2-(float)sample1;

		dat->ic=dat->s1;

		dat->ib = ( 3.0*fh - new_s1*h - 2.0*dat->ic*h ) / (h*h);
		dat->ia = ( fh - dat->ib * h * h - dat->ic * h) / (h*h*h);

		dat->s1=new_s1;
		dat->prev=sample1;
		dat->ok=1;
	}
}

extern void _cdecl cubic_advance_3d();

#define _const_h_ ((float)((INT64S)1<<(FRACBITS-1)))
const float const_h=_const_h_;


const float const_1p2h=1/(2*_const_h_);
const float const_h2=_const_h_*_const_h_;
const float const_1ph2 = 1/(_const_h_*_const_h_);
const float const_1ph3=1/(_const_h_*_const_h_*_const_h_);
const float const_3 = 3.0;

typedef void (_fastcall * CUBIC_ADVANCE)(CUBIC_DAT * dat,int sample1,int sample2,int sample3);
typedef int (_fastcall * CUBIC_CALC)(CUBIC_DAT*,UINT);

_declspec(naked) static int _fastcall cubic_calc(CUBIC_DAT * dat,UINT pos)
{
	_asm
	{
		shr edx,1//hack to avoid weirdness with converting uint to float
		push edx
		fild dword ptr [esp]
		fld st//x,x
		fmul dword ptr [ecx]CUBIC_DAT.ic//cx,x
		fxch//x,cx
		fld st//x,x,cx
		fmul st,st//x2,x,cx
		fxch//x,x2,cx
		fmul st,st(1)//x3,x2,cx
		fmul dword ptr [ecx]CUBIC_DAT.ia//ax3,x2,cx
		fxch//x2,ax3,cx
		fmul dword ptr [ecx]CUBIC_DAT.ib//bx2,ax3,cx
		fadd
		fadd
		fistp dword ptr [esp]
		pop eax
		add eax,dword ptr [ecx]CUBIC_DAT.prev
		ret
	}
/*	float x=(float)(pos>>1);
	return dat->prev+(int)(dat->ia*x*x*x+dat->ib*x*x+dat->ic*x);*/
}

static CUBIC_ADVANCE cubic_advance_ptr=(CUBIC_ADVANCE)cubic_advance;
static CUBIC_CALC cubic_calc_ptr=(CUBIC_CALC)cubic_calc;

extern int _cdecl cubic_calc_3d();//(CUBIC_DAT * dat,UINT pos);

void cubic_init()
{
	uint cpu=_mm_cpudetect();
	if (cpu&CPU_3DNOW)
	{
		cubic_calc_ptr=(CUBIC_CALC)cubic_calc_3d;
		cubic_advance_ptr=(CUBIC_ADVANCE)cubic_advance_3d;
	}
//	else cubic_calc_ptr=cubic_calc;
}

#define SCAST SWORD
#define _SCAST word
#define MAKENAME(X) Mix16##X
#define vxx v16

#include "cubic_inc2.h"

#undef SCAST
#undef _SCAST
#undef MAKENAME
#undef vxx


#define SCAST SBYTE
#define _SCAST byte
#define MAKENAME(X) Mix8##X
#define vxx v8
#define CUBIC_8BIT

#include "cubic_inc2.h"

#undef CUBIC_8BIT
#undef SCAST
#undef _SCAST
#undef MAKENAME
#undef vxx

#pragma warning(disable: 4113)

VMIXER M16_MONO_CUBIC =
{   NULL,

    BLAH("Cubic mixer mono16"),

    nc16_Check_Mono,

    NULL,
    NULL,

    VC_Volcalc16_Mono,
    VC_Volramp16_Mono,
    Mix16MonoCubic_NoClick,
    Mix16MonoCubic_NoClick,
    Mix16MonoCubic,
    Mix16MonoCubic,
};

VMIXER M16_STEREO_CUBIC =
{   NULL,

    BLAH("Cubic mixer stereo16"),

    nc16_Check_Stereo,

    NULL,
    NULL,

    VC_Volcalc16_Stereo,
    VC_Volramp16_Stereo,
    Mix16StereoCubic_NoClick,
    Mix16SurroundCubic_NoClick,
    Mix16StereoCubic,
    Mix16SurroundCubic,
};

VMIXER M8_MONO_CUBIC =
{   NULL,

    BLAH("Cubic mixer mono8"),

    nc8_Check_Mono,

    NULL,
    NULL,

    VC_Volcalc8_Mono,
    VC_Volramp8_Mono,
    Mix8MonoCubic_NoClick,
    Mix8MonoCubic_NoClick,
    Mix8MonoCubic,
    Mix8MonoCubic,
};

VMIXER M8_STEREO_CUBIC =
{   NULL,

    BLAH("Cubic mixer stereo8"),

    nc8_Check_Stereo,

    NULL,
    NULL,

    VC_Volcalc8_Stereo,
    VC_Volramp8_Stereo,
    Mix8StereoCubic_NoClick,
    Mix8SurroundCubic_NoClick,
    Mix8StereoCubic,
    Mix8SurroundCubic,
};
