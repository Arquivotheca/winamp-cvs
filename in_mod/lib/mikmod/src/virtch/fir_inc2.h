/*
HOLY WAR AGAINST QUANTISING 8 BIT SAMPLES TO 8 BIT INSTEAD OF THE
OUTPUT RESOLUTION ... gives them that nasty 8 bit sound even when
interpolated

My solution, though it probably sucks: Convert to 16 bit, then
interpolate. Do not interpolate on 8 bit scale, then use a volume
level that is 256 times louder. (sufficient for conversion, but lame
in my opinion)
*/

#ifdef FIR_8BIT
#define DO_VOL(X) _asm sar X,8
#else
#define DO_VOL(X)
#endif

#define FUNC_NAME MAKENAME(StereoFIR)
#define FUNC_NAME2 MAKENAME(StereoFIR_)
#define FOO_BLOCK	\
	_asm mov edx,eax		\
	_asm imul edx,vxx.lvolsel	\
	_asm imul eax,vxx.rvolsel	\
	DO_VOL(edx) \
	DO_VOL(eax) \
	_asm add dword ptr [edi],edx	\
	_asm add dword ptr [edi+4],eax	\
	_asm add edi,8

#include "fir_inc.h"

#undef FUNC_NAME
#undef FUNC_NAME2
#undef FOO_BLOCK

#define FUNC_NAME MAKENAME(SurroundFIR)
#define FUNC_NAME2 MAKENAME(SurroundFIR_)
#define FOO_BLOCK \
	_asm imul eax,vxx.lvolsel	\
	DO_VOL(eax) \
	_asm add dword ptr [edi],eax	\
	_asm sub dword ptr [edi+4],eax	\
	_asm add edi,8

#include "fir_inc.h"

#undef FUNC_NAME
#undef FUNC_NAME2
#undef FOO_BLOCK

#define FUNC_NAME MAKENAME(MonoFIR)
#define FUNC_NAME2 MAKENAME(MonoFIR_)
#define FOO_BLOCK	\
	_asm imul eax,vxx.lvolsel	\
	DO_VOL(eax) \
	_asm add dword ptr [edi],eax	\
	_asm add edi,4

#include "fir_inc.h"

#undef FUNC_NAME
#undef FUNC_NAME2
#undef FOO_BLOCK

#undef DO_VOL

#ifdef FIR_8BIT
#define DO_VOL(X) _asm sar X,8
#else
#define DO_VOL(X) _asm sar X,7
#endif


#define FUNC_NAME MAKENAME(StereoFIR_NoClick)
#define FUNC_NAME2 MAKENAME(StereoFIR_NoClick_)

#define FOO_BLOCK \
	_asm mov ecx,vxx.left.vol	\
	_asm mov edx,vxx.right.vol	\
	_asm add ecx,vxx.left.inc	\
	_asm add edx,vxx.right.inc	\
	_asm mov vxx.left.vol,ecx	\
	_asm mov vxx.right.vol,edx	\
	_asm imul ecx,eax	\
	_asm imul edx,eax	\
	DO_VOL(ecx)	\
	DO_VOL(edx)	\
	_asm add dword ptr [edi],ecx	\
	_asm add dword ptr [edi+4],edx	\
	_asm add edi,8


#include "fir_inc.h"

#undef FUNC_NAME
#undef FUNC_NAME2
#undef FOO_BLOCK


#define FUNC_NAME MAKENAME(SurroundFIR_NoClick)
#define FUNC_NAME2 MAKENAME(SurroundFIR_NoClick_)

#define FOO_BLOCK	\
	_asm mov ecx,vxx.left.vol	\
	_asm add ecx,vxx.left.inc	\
	_asm mov vxx.left.vol,ecx	\
	_asm imul ecx,eax	\
	DO_VOL(ecx)	\
	_asm add dword ptr [edi],ecx	\
	_asm sub dword ptr [edi+4],ecx	\
	_asm add edi,8

#include "fir_inc.h"

#undef FUNC_NAME
#undef FUNC_NAME2
#undef FOO_BLOCK

#define FUNC_NAME MAKENAME(MonoFIR_NoClick)
#define FUNC_NAME2 MAKENAME(MonoFIR_NoClick_)

#define FOO_BLOCK	\
	_asm mov ecx,vxx.left.vol	\
	_asm add ecx,vxx.left.inc	\
	_asm mov vxx.left.vol,ecx	\
	_asm imul ecx,eax	\
	DO_VOL(ecx)	\
	_asm add dword ptr [edi],ecx	\
	_asm add edi,4

#include "fir_inc.h"

#undef FUNC_NAME
#undef FUNC_NAME2
#undef FOO_BLOCK

#undef DO_VOL