#define FUNC_NAME MAKENAME(StereoCubic)
#define FOO_BLOCK	\
	_asm mov edx,eax		\
	_asm imul edx,lvolsel	\
	_asm imul eax,rvolsel	\
	_asm add dword ptr [edi],edx	\
	_asm add dword ptr [edi+4],eax	\
	_asm add edi,8

#include "cub_inc.h"

#undef FUNC_NAME
#undef FOO_BLOCK

#define FUNC_NAME MAKENAME(SurroundCubic)
#define FOO_BLOCK \
	_asm imul eax,lvolsel	\
	_asm add dword ptr [edi],eax	\
	_asm sub dword ptr [edi+4],eax	\
	_asm add edi,8

#include "cub_inc.h"

#undef FUNC_NAME
#undef FOO_BLOCK

#define FUNC_NAME MAKENAME(MonoCubic)
#define FOO_BLOCK	\
	_asm imul eax,lvolsel	\
	_asm add dword ptr [edi],eax	\
	_asm add edi,4

#include "cub_inc.h"

#undef FUNC_NAME
#undef FOO_BLOCK


#ifdef CUBIC_8BIT
#define DO_VOL(X)
#else
#define DO_VOL(X) _asm sar X,7
#endif

#define FUNC_NAME MAKENAME(StereoCubic_NoClick)

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


#include "cub_inc.h"

#undef FUNC_NAME
#undef FOO_BLOCK


#define FUNC_NAME MAKENAME(SurroundCubic_NoClick)

#define FOO_BLOCK	\
	_asm mov ecx,vxx.left.vol	\
	_asm add ecx,vxx.left.inc	\
	_asm mov vxx.left.vol,ecx	\
	_asm imul ecx,eax	\
	DO_VOL(ecx)	\
	_asm add dword ptr [edi],ecx	\
	_asm sub dword ptr [edi+4],ecx	\
	_asm add edi,8

#include "cub_inc.h"

#undef FUNC_NAME
#undef FOO_BLOCK

#define FUNC_NAME MAKENAME(MonoCubic_NoClick)

#define FOO_BLOCK	\
	_asm mov ecx,vxx.left.vol	\
	_asm add ecx,vxx.left.inc	\
	_asm mov vxx.left.vol,ecx	\
	_asm imul ecx,eax	\
	DO_VOL(ecx)	\
	_asm add dword ptr [edi],ecx	\
	_asm add edi,4

#include "cub_inc.h"

#undef FUNC_NAME
#undef FOO_BLOCK

#undef DO_VOL