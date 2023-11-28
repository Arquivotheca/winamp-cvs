#include "nsmath/pcm.h"

static __declspec(align(16)) const float const_32768_4[] = { 32768.0f, 32768.0f, 32768.0f, 32768.0f };

// converts 32 floats to int16_t at once
__declspec(naked) static void Convert_F32_S16_Aligned_32(int16_t *destination, const float *source, size_t sample_count, float gain)
{
	__asm
	{
		mov eax, 4[esp] // eax = destination
		mov ecx, 8[esp] // ecx = source
		mov edx, 12[esp] // edx = loop_count

		movss	xmm7, DWORD PTR 16[esp] // load gain into the low portion of xmm7
		shufps	xmm7, xmm7, 0 // move into rest of xmm7

		cmp edx, 32
		jl do16

loop32:
		
		// load 16 consecutive floating point samples (enough for one cache line)
		movaps xmm0, XMMWORD PTR 0[ecx]
		movaps xmm1, XMMWORD PTR 16[ecx]
		movaps xmm2, XMMWORD PTR 32[ecx]
		movaps xmm3, XMMWORD PTR 48[ecx]

		
		// multiply by 32768
		mulps	xmm0, xmm7
		mulps	xmm1, xmm7
		mulps	xmm2, xmm7
		mulps	xmm3, xmm7

		// convert to int32_t with truncation
		cvttps2dq xmm0, xmm0
		cvttps2dq xmm1, xmm1
		cvttps2dq xmm2, xmm2
		cvttps2dq xmm3, xmm3
		
		// convert to int16_t with saturation
		packssdw xmm0, xmm1 // xmm0 = destination[0-7]
		packssdw xmm2, xmm3 // xmm2 = destination[8-15]

		// this frees up xmm1 and xmm3, so let's load another cache line
		movaps xmm1, XMMWORD PTR 64[ecx]
		movaps xmm3, XMMWORD PTR 80[ecx]
		movaps xmm4, XMMWORD PTR 96[ecx]
		movaps xmm5, XMMWORD PTR 112[ecx]

		// multiply by 32768
		mulps	xmm1, xmm7
		mulps	xmm3, xmm7
		mulps	xmm4, xmm7
		mulps	xmm5, xmm7

		// convert to int32_t with truncation
		cvttps2dq xmm1, xmm1
		cvttps2dq xmm3, xmm3
		cvttps2dq xmm4, xmm4
		cvttps2dq xmm5, xmm5

		// convert to int16_t with saturation
		packssdw xmm1, xmm3 // xmm1 = destination[16-23]
		packssdw xmm4, xmm5 // xmm4 = destination[24-31]

		// store destination
		movdqa	XMMWORD PTR 0[eax], xmm0
		movdqa	XMMWORD PTR 16[eax], xmm2
		movdqa	XMMWORD PTR 32[eax], xmm1
		movdqa	XMMWORD PTR 48[eax], xmm4

		add ecx, 128
		add eax, 64

		// see if we're done
		sub edx, 32
		cmp edx, 32
		
		jge loop32

do16:
		cmp edx, 16
		jl do8

		// load 16 consecutive floating point samples (enough for one cache line)
		movaps xmm0, XMMWORD PTR 0[ecx]
		movaps xmm1, XMMWORD PTR 16[ecx]
		movaps xmm2, XMMWORD PTR 32[ecx]
		movaps xmm3, XMMWORD PTR 48[ecx]

		// multiply by 32768
		mulps	xmm0, xmm7
		mulps	xmm1, xmm7
		mulps	xmm2, xmm7
		mulps	xmm3, xmm7

		// convert to int32_t with truncation
		cvttps2dq xmm0, xmm0
		cvttps2dq xmm1, xmm1
		cvttps2dq xmm2, xmm2
		cvttps2dq xmm3, xmm3
		
		// convert to int16_t with saturation
		packssdw xmm0, xmm1 // xmm0 = destination[0-7]
		packssdw xmm2, xmm3 // xmm2 = destination[8-15]
		movdqa	XMMWORD PTR 0[eax], xmm0
		movdqa	XMMWORD PTR 16[eax], xmm2

		add ecx, 64
		add eax, 32

		sub edx, 16

do8:
		cmp edx, 8
		jl do4

		// load 8 consecutive floating point samples
		movaps xmm0, XMMWORD PTR 0[ecx]
		movaps xmm1, XMMWORD PTR 16[ecx]

		// multiply by 32768
		mulps	xmm0, xmm7
		mulps	xmm1, xmm7

		// convert to int32_t with truncation
		cvttps2dq xmm0, xmm0
		cvttps2dq xmm1, xmm1
		
		// convert to int16_t with saturation
		packssdw xmm0, xmm1 // xmm0 = destination[0-7]
		movdqa	XMMWORD PTR 0[eax], xmm0

		add ecx, 32
		add eax, 16

		sub edx, 8
do4:
		test edx, edx
		jz done
		push ebx
loop1:
		// load single float
		movss xmm0, DWORD PTR 0[ecx]

		// multiple by 32768
		mulss xmm0, xmm7

		// convert to 32bit with truncation
		cvttps2dq xmm0, xmm0
		
		// convert to int16_t with saturation
		packssdw xmm0, xmm0

		movd ebx, xmm0
		mov WORD PTR 0[eax], bx

		add ecx, 4
		add eax, 2

		dec edx
		jnz loop1

		pop ebx
done:
		ret 0
	}
}

// converts 32 floats to int16_t at once
__declspec(naked) static void Convert_F32_S16_AlignedSource_32(int16_t *destination, const float *source, size_t sample_count, float gain)
{
	__asm
	{
		mov eax, 4[esp] // eax = destination
		mov ecx, 8[esp] // ecx = source
		mov edx, 12[esp] // edx = loop_count

		movss	xmm7, DWORD PTR 16[esp] // load gain into the low portion of xmm7
		shufps	xmm7, xmm7, 0 // move into rest of xmm7

		cmp edx, 32
		jl do16

loop32:
		
		// load 16 consecutive floating point samples (enough for one cache line)
		movaps xmm0, XMMWORD PTR 0[ecx]
		movaps xmm1, XMMWORD PTR 16[ecx]
		movaps xmm2, XMMWORD PTR 32[ecx]
		movaps xmm3, XMMWORD PTR 48[ecx]

		
		// multiply by 32768
		mulps	xmm0, xmm7
		mulps	xmm1, xmm7
		mulps	xmm2, xmm7
		mulps	xmm3, xmm7

		// convert to int32_t with truncation
		cvttps2dq xmm0, xmm0
		cvttps2dq xmm1, xmm1
		cvttps2dq xmm2, xmm2
		cvttps2dq xmm3, xmm3
		
		// convert to int16_t with saturation
		packssdw xmm0, xmm1 // xmm0 = destination[0-7]
		packssdw xmm2, xmm3 // xmm2 = destination[8-15]

		// this frees up xmm1 and xmm3, so let's load another cache line
		movaps xmm1, XMMWORD PTR 64[ecx]
		movaps xmm3, XMMWORD PTR 80[ecx]
		movaps xmm4, XMMWORD PTR 96[ecx]
		movaps xmm5, XMMWORD PTR 112[ecx]

		// multiply by 32768
		mulps	xmm1, xmm7
		mulps	xmm3, xmm7
		mulps	xmm4, xmm7
		mulps	xmm5, xmm7

		// convert to int32_t with truncation
		cvttps2dq xmm1, xmm1
		cvttps2dq xmm3, xmm3
		cvttps2dq xmm4, xmm4
		cvttps2dq xmm5, xmm5

		// convert to int16_t with saturation
		packssdw xmm1, xmm3 // xmm1 = destination[16-23]
		packssdw xmm4, xmm5 // xmm4 = destination[24-31]

		// store destination
		movdqu	XMMWORD PTR 0[eax], xmm0
		movdqu	XMMWORD PTR 16[eax], xmm2
		movdqu	XMMWORD PTR 32[eax], xmm1
		movdqu	XMMWORD PTR 48[eax], xmm4

		add ecx, 128
		add eax, 64

		// see if we're done
		sub edx, 32
		cmp edx, 32
		
		jge loop32

do16:
		cmp edx, 16
		jl do8

		// load 16 consecutive floating point samples (enough for one cache line)
		movaps xmm0, XMMWORD PTR 0[ecx]
		movaps xmm1, XMMWORD PTR 16[ecx]
		movaps xmm2, XMMWORD PTR 32[ecx]
		movaps xmm3, XMMWORD PTR 48[ecx]

		// multiply by 32768
		mulps	xmm0, xmm7
		mulps	xmm1, xmm7
		mulps	xmm2, xmm7
		mulps	xmm3, xmm7

		// convert to int32_t with truncation
		cvttps2dq xmm0, xmm0
		cvttps2dq xmm1, xmm1
		cvttps2dq xmm2, xmm2
		cvttps2dq xmm3, xmm3
		
		// convert to int16_t with saturation
		packssdw xmm0, xmm1 // xmm0 = destination[0-7]
		packssdw xmm2, xmm3 // xmm2 = destination[8-15]
		movdqu	XMMWORD PTR 0[eax], xmm0
		movdqu	XMMWORD PTR 16[eax], xmm2

		add ecx, 64
		add eax, 32

		sub edx, 16

do8:
		cmp edx, 8
		jl do4

		// load 8 consecutive floating point samples
		movaps xmm0, XMMWORD PTR 0[ecx]
		movaps xmm1, XMMWORD PTR 16[ecx]

		// multiply by 32768
		mulps	xmm0, xmm7
		mulps	xmm1, xmm7

		// convert to int32_t with truncation
		cvttps2dq xmm0, xmm0
		cvttps2dq xmm1, xmm1
		
		// convert to int16_t with saturation
		packssdw xmm0, xmm1 // xmm0 = destination[0-7]
		movdqu	XMMWORD PTR 0[eax], xmm0

		add ecx, 32
		add eax, 16

		sub edx, 8
do4:
		test edx, edx
		jz done
		push ebx
loop1:
		// load single float
		movss xmm0, DWORD PTR 0[ecx]

		// multiple by 32768
		mulss xmm0, xmm7

		// convert to 32bit with truncation
		cvttps2dq xmm0, xmm0
		
		// convert to int16_t with saturation
		packssdw xmm0, xmm0

		movd ebx, xmm0
		mov WORD PTR 0[eax], bx

		add ecx, 4
		add eax, 2

		dec edx
		jnz loop1

		pop ebx
done:
		ret 0
	}
}

__declspec(naked) static void Convert_F32_S16_AlignedDestination_32(int16_t *destination, const float *source, size_t sample_count, float gain)
{
	__asm
	{
		mov eax, 4[esp] // eax = destination
		mov ecx, 8[esp] // ecx = source
		mov edx, 12[esp] // edx = loop_count

		movss	xmm7, DWORD PTR 16[esp] // load gain into the low portion of xmm7
		shufps	xmm7, xmm7, 0 // move into rest of xmm7

		cmp edx, 32
		jl do16

loop32:
		
		// load 16 consecutive floating point samples (enough for one cache line)
		movups xmm0, XMMWORD PTR 0[ecx]
		movups xmm1, XMMWORD PTR 16[ecx]
		movups xmm2, XMMWORD PTR 32[ecx]
		movups xmm3, XMMWORD PTR 48[ecx]

		
		// multiply by 32768
		mulps	xmm0, xmm7
		mulps	xmm1, xmm7
		mulps	xmm2, xmm7
		mulps	xmm3, xmm7

		// convert to int32_t with truncation
		cvttps2dq xmm0, xmm0
		cvttps2dq xmm1, xmm1
		cvttps2dq xmm2, xmm2
		cvttps2dq xmm3, xmm3
		
		// convert to int16_t with saturation
		packssdw xmm0, xmm1 // xmm0 = destination[0-7]
		packssdw xmm2, xmm3 // xmm2 = destination[8-15]

		// this frees up xmm1 and xmm3, so let's load another cache line
		movups xmm1, XMMWORD PTR 64[ecx]
		movups xmm3, XMMWORD PTR 80[ecx]
		movups xmm4, XMMWORD PTR 96[ecx]
		movups xmm5, XMMWORD PTR 112[ecx]

		// multiply by 32768
		mulps	xmm1, xmm7
		mulps	xmm3, xmm7
		mulps	xmm4, xmm7
		mulps	xmm5, xmm7

		// convert to int32_t with truncation
		cvttps2dq xmm1, xmm1
		cvttps2dq xmm3, xmm3
		cvttps2dq xmm4, xmm4
		cvttps2dq xmm5, xmm5

		// convert to int16_t with saturation
		packssdw xmm1, xmm3 // xmm1 = destination[16-23]
		packssdw xmm4, xmm5 // xmm4 = destination[24-31]

		// store destination
		movdqa	XMMWORD PTR 0[eax], xmm0
		movdqa	XMMWORD PTR 16[eax], xmm2
		movdqa	XMMWORD PTR 32[eax], xmm1
		movdqa	XMMWORD PTR 48[eax], xmm4

		add ecx, 128
		add eax, 64

		// see if we're done
		sub edx, 32
		cmp edx, 32
		
		jge loop32

do16:
		cmp edx, 16
		jl do8

		// load 16 consecutive floating point samples (enough for one cache line)
		movups xmm0, XMMWORD PTR 0[ecx]
		movups xmm1, XMMWORD PTR 16[ecx]
		movups xmm2, XMMWORD PTR 32[ecx]
		movups xmm3, XMMWORD PTR 48[ecx]

		// multiply by 32768
		mulps	xmm0, xmm7
		mulps	xmm1, xmm7
		mulps	xmm2, xmm7
		mulps	xmm3, xmm7

		// convert to int32_t with truncation
		cvttps2dq xmm0, xmm0
		cvttps2dq xmm1, xmm1
		cvttps2dq xmm2, xmm2
		cvttps2dq xmm3, xmm3
		
		// convert to int16_t with saturation
		packssdw xmm0, xmm1 // xmm0 = destination[0-7]
		packssdw xmm2, xmm3 // xmm2 = destination[8-15]
		movdqa	XMMWORD PTR 0[eax], xmm0
		movdqa	XMMWORD PTR 16[eax], xmm2

		add ecx, 64
		add eax, 32

		sub edx, 16

do8:
		cmp edx, 8
		jl do4

		// load 8 consecutive floating point samples
		movups xmm0, XMMWORD PTR 0[ecx]
		movups xmm1, XMMWORD PTR 16[ecx]

		// multiply by 32768
		mulps	xmm0, xmm7
		mulps	xmm1, xmm7

		// convert to int32_t with truncation
		cvttps2dq xmm0, xmm0
		cvttps2dq xmm1, xmm1
		
		// convert to int16_t with saturation
		packssdw xmm0, xmm1 // xmm0 = destination[0-7]
		movdqa	XMMWORD PTR 0[eax], xmm0

		add ecx, 32
		add eax, 16

		sub edx, 8
do4:
		test edx, edx
		jz done
		push ebx
loop1:
		// load single float
		movss xmm0, DWORD PTR 0[ecx]

		// multiple by 32768
		mulss xmm0, xmm7

		// convert to 32bit with truncation
		cvttps2dq xmm0, xmm0
		
		// convert to int16_t with saturation
		packssdw xmm0, xmm0

		movd ebx, xmm0
		mov WORD PTR 0[eax], bx

		add ecx, 4
		add eax, 2

		dec edx
		jnz loop1

		pop ebx
done:
		ret 0
	}
}

// converts 32 floats to int16_t at once
__declspec(naked) static void Convert_F32_S16_Unaligned_32(int16_t *destination, const float *source, size_t sample_count, float gain)
{
	__asm
	{
		mov eax, 4[esp] // eax = destination
		mov ecx, 8[esp] // ecx = source
		mov edx, 12[esp] // edx = loop_count

		movss	xmm7, DWORD PTR 16[esp] // load gain into the low portion of xmm7
		shufps	xmm7, xmm7, 0 // move into rest of xmm7

		cmp edx, 32
		jl do16
loop32:
		// load 16 consecutive floating point samples (enough for one cache line)
		movups xmm0, XMMWORD PTR 0[ecx]
		movups xmm1, XMMWORD PTR 16[ecx]
		movups xmm2, XMMWORD PTR 32[ecx]
		movups xmm3, XMMWORD PTR 48[ecx]

		// multiply by 32768
		mulps	xmm0, xmm7
		mulps	xmm1, xmm7
		mulps	xmm2, xmm7
		mulps	xmm3, xmm7

		// convert to int32_t with truncation
		cvttps2dq xmm0, xmm0
		cvttps2dq xmm1, xmm1
		cvttps2dq xmm2, xmm2
		cvttps2dq xmm3, xmm3
		
		// convert to int16_t with saturation
		packssdw xmm0, xmm1 // xmm0 = destination[0-7]
		packssdw xmm2, xmm3 // xmm2 = destination[8-15]

		// this frees up xmm1 and xmm3, so let's load another cache line
		movups xmm1, XMMWORD PTR 64[ecx]
		movups xmm3, XMMWORD PTR 80[ecx]
		movups xmm4, XMMWORD PTR 96[ecx]
		movups xmm5, XMMWORD PTR 112[ecx]

		// multiply by 32768
		mulps	xmm1, xmm7
		mulps	xmm3, xmm7
		mulps	xmm4, xmm7
		mulps	xmm5, xmm7

		// convert to int32_t with truncation
		cvttps2dq xmm1, xmm1
		cvttps2dq xmm3, xmm3
		cvttps2dq xmm4, xmm4
		cvttps2dq xmm5, xmm5

		// convert to int16_t with saturation
		packssdw xmm1, xmm3 // xmm1 = destination[16-23]
		packssdw xmm4, xmm5 // xmm4 = destination[24-31]

		// store destination
		movdqu	XMMWORD PTR 0[eax], xmm0
		movdqu	XMMWORD PTR 16[eax], xmm2
		movdqu	XMMWORD PTR 32[eax], xmm1
		movdqu	XMMWORD PTR 48[eax], xmm4

		add ecx, 128
		add eax, 64

		// see if we're done
		sub edx, 32
		cmp edx, 32
		jge loop32

do16:
		cmp edx, 16
		jl do8

		// load 16 consecutive floating point samples (enough for one cache line)
		movups xmm0, XMMWORD PTR 0[ecx]
		movups xmm1, XMMWORD PTR 16[ecx]
		movups xmm2, XMMWORD PTR 32[ecx]
		movups xmm3, XMMWORD PTR 48[ecx]

		// multiply by 32768
		mulps	xmm0, xmm7
		mulps	xmm1, xmm7
		mulps	xmm2, xmm7
		mulps	xmm3, xmm7

		// convert to int32_t with truncation
		cvttps2dq xmm0, xmm0
		cvttps2dq xmm1, xmm1
		cvttps2dq xmm2, xmm2
		cvttps2dq xmm3, xmm3
		
		// convert to int16_t with saturation
		packssdw xmm0, xmm1 // xmm0 = destination[0-7]
		packssdw xmm2, xmm3 // xmm2 = destination[8-15]
		movdqu	XMMWORD PTR 0[eax], xmm0
		movdqu	XMMWORD PTR 16[eax], xmm2

		add ecx, 64
		add eax, 32

		sub edx, 16

do8:
		cmp edx, 8
		jl do4

		// load 8 consecutive floating point samples
		movups xmm0, XMMWORD PTR 0[ecx]
		movups xmm1, XMMWORD PTR 16[ecx]

		// multiply by 32768
		mulps	xmm0, xmm7
		mulps	xmm1, xmm7

		// convert to int32_t with truncation
		cvttps2dq xmm0, xmm0
		cvttps2dq xmm1, xmm1
		
		// convert to int16_t with saturation
		packssdw xmm0, xmm1 // xmm0 = destination[0-7]
		movdqu	XMMWORD PTR 0[eax], xmm0

		add ecx, 32
		add eax, 16

		sub edx, 8
do4:
		test edx, edx
		jz done
		push ebx
loop1:
		// load single float
		movss xmm0, DWORD PTR 0[ecx]

		// multiple by 32768
		mulss xmm0, xmm7

		// convert to 32bit with truncation
		cvttps2dq xmm0, xmm0
		
		// convert to int16_t with saturation
		packssdw xmm0, xmm0

		movd ebx, xmm0
		mov WORD PTR 0[eax], bx

		add ecx, 4
		add eax, 2

		dec edx
		jnz loop1

		pop ebx
done:
		ret 0
	}
}


void x86_SSE2_nsmath_pcm_Convert_F32_S16(int16_t *destination, const float *source, size_t sample_count, float gain)
{
	if (((size_t)destination & 0xF) == 0 && ((size_t)source & 0xF) == 0)
	{
			Convert_F32_S16_Aligned_32(destination, source, sample_count, gain);
	}
	else if (((size_t)source & 0xF) == 0)
	{
			Convert_F32_S16_AlignedSource_32(destination, source, sample_count, gain);
	}
	else if (((size_t)destination & 0xF) == 0)
	{
		Convert_F32_S16_AlignedDestination_32(destination, source, sample_count, gain);
	}
	else
	{
		Convert_F32_S16_Unaligned_32(destination, source, sample_count, gain);
	}
}
