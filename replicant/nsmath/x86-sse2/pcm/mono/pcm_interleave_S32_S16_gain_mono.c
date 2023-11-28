#include "nsmath/pcm.h"
#include <emmintrin.h>

static __declspec(naked) void Convert_S32_S16_gain_mono_Unaligned(int16_t *destination_buffer, const int32_t **source_buffer, size_t channels, size_t count_per_channel, float gain)
{
	/*  general structure 
		movss/shufps to load gain into xmm7
		movaps source_left (int32) into xmm0
		movaps source_right (int32) into xmm1
		interleave left and right using punpckhdq/punpckldq 
		cvtdq2ps to convert xmm0 and xmm1 to float
		mulps to multiple by gain
		reconvert to int32 using cvttps2dq
		saturate to int16 using packssdw
		movdqa to store to destination

		TODO: need to think through if we want to use doubles instead of singles for precision
		*/
	__asm
	{
		
		mov eax, DWORD PTR 4[esp] // eax holds destination
		mov	ecx, DWORD PTR 8[esp]
		mov edx, DWORD PTR 16[esp] // edx holds count_per_channel
		movss	xmm7, DWORD PTR 20[esp] // load gain into the low portion of xmm7
		shufps	xmm7, xmm7, 0 // move into rest of xmm7
		mov ecx, DWORD PTR 0[ecx] 

		push edi
		xor edi, edi
		
		cmp edx, 24
		jl do1
loop24:
		movups xmm0, XMMWORD PTR 0[ecx+edi*4] 
		movups xmm1, XMMWORD PTR 16[ecx+edi*4]
		movups xmm2, XMMWORD PTR 32[ecx+edi*4]
		movups xmm3, XMMWORD PTR 48[ecx+edi*4] 
		movups xmm4, XMMWORD PTR 64[ecx+edi*4] 
		movups xmm5, XMMWORD PTR 80[ecx+edi*4] 

		 // convert to float
		cvtdq2ps xmm0, xmm0
		cvtdq2ps xmm1, xmm1
		cvtdq2ps xmm2, xmm2
		cvtdq2ps xmm3, xmm3
		cvtdq2ps xmm4, xmm4
		cvtdq2ps xmm5, xmm5
		
		// multiply by gain
		mulps xmm0, xmm7
		mulps xmm1, xmm7
		mulps xmm2, xmm7
		mulps xmm3, xmm7
		mulps xmm4, xmm7
		mulps xmm5, xmm7
		
		// convert back to int32, with truncation
		cvttps2dq xmm0, xmm0
		cvttps2dq xmm1, xmm1
		cvttps2dq xmm2, xmm2
		cvttps2dq xmm3, xmm3
		cvttps2dq xmm4, xmm4
		cvttps2dq xmm5, xmm5

		// convert to int16, saturing
		packssdw xmm0, xmm1
		packssdw xmm2, xmm3
		packssdw xmm4, xmm5
		
		// store
		movdqu XMMWORD PTR [eax+edi*2], xmm0
		movdqu XMMWORD PTR 16[eax+edi*2], xmm2
		movdqu XMMWORD PTR 32[eax+edi*2], xmm4

		add edi, 24
		sub edx, 24
		cmp edx, 24
		jge loop24
		
do1:
		test edx, edx
		jz done

		push ebx
loop1:
		movd xmm0, DWORD PTR [ecx+edi*4] // mm0 holds [L 0]
		
		cvtdq2ps xmm0, xmm0 // xmm0 holds [R 0 0] as 32bit float
		mulps xmm0, xmm7 // multiply by gain
		cvttps2dq xmm0, xmm0 // convert back to int32, truncating
		packssdw xmm0, xmm0 // convert to int16, saturating
		movd ebx, xmm0 // store lower 32 bits to destination
		mov WORD PTR[eax+edi*2], bx

		add edi, 1
		dec edx
		jnz loop1
		pop ebx
done:
		pop edi
		
		ret 0
	}
}

void x86_SSE2_nsmath_pcm_Convert_S32_S16_gain_mono(int16_t *destination, const int32_t **source, size_t channels, size_t count_per_channel, float gain)
{
	int source_aligned = ((size_t)source[0] & 0xF) == 0;
	int destination_aligned = ((size_t)destination & 0xF) == 0;
	if (destination_aligned && source_aligned)
	{
		Convert_S32_S16_gain_mono_Unaligned(destination, source, channels, count_per_channel, gain);
	}
	else if (source_aligned)
	{
		Convert_S32_S16_gain_mono_Unaligned(destination, source, channels, count_per_channel, gain);
	}
	else if (destination_aligned)
	{
		Convert_S32_S16_gain_mono_Unaligned(destination, source, channels, count_per_channel, gain);
	}
	else
	{
		Convert_S32_S16_gain_mono_Unaligned(destination, source, channels, count_per_channel, gain);
	}
}

