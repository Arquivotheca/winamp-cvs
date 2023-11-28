#include "nsmath/pcm.h"
#include <emmintrin.h>

/* This one converts non-interleaved int32_t to int16_t with no change in value.  Useful for e.g. 16-bit FLAC files */

static __declspec(naked) void Convert_S32_S16_stereo_shift_Aligned(int16_t *destination_buffer, const int32_t **source_buffer, size_t channels, size_t count_per_channel, unsigned int decimation_bits)
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
		movd xmm7, DWORD PTR 20[esp] // lower portion of xmm7 holds decimation bits
		

		push esi
		push edi
		mov esi, DWORD PTR 0[ecx] // esi holds left
		mov ecx, DWORD PTR 4[ecx] // ecx holds right
		xor edi, edi
		
		cmp edx, 12	
		jl do1
loop12:
		movaps xmm1, XMMWORD PTR 0[esi+edi*4] // 4 samples of left [L0 L1 L2 L3]
		movaps xmm2, XMMWORD PTR 16[esi+edi*4]
		movaps xmm5, XMMWORD PTR 32[esi+edi*4]

		movaps xmm3, XMMWORD PTR 0[ecx+edi*4] // 4 samples of right [R0 R1 R2 R3]
		movaps xmm4, XMMWORD PTR 16[ecx+edi*4] 
		movaps xmm6, XMMWORD PTR 32[ecx+edi*4] 

		// interleave
		movdqa xmm0, xmm1
		punpckhdq xmm1, xmm3 
		punpckldq xmm0, xmm3 

		movdqa xmm3, xmm2
		punpckhdq xmm3, xmm4
		punpckldq xmm2, xmm4

		movdqa xmm4, xmm5
		punpckhdq xmm5, xmm6
		punpckldq xmm4, xmm6

		// shift right by 8
		psrad xmm0, xmm7
		psrad xmm1, xmm7
		psrad xmm2, xmm7
		psrad xmm3, xmm7
		psrad xmm4, xmm7
		psrad xmm5, xmm7

		// convert to int16, saturing
		packssdw xmm0, xmm1
		packssdw xmm2, xmm3
		packssdw xmm4, xmm5
		
		// store
		movdqa XMMWORD PTR [eax+edi*4], xmm0
		movdqa XMMWORD PTR 16[eax+edi*4], xmm2
		movdqa XMMWORD PTR 32[eax+edi*4], xmm4

		add edi, 12
		sub edx, 12
		cmp edx, 12
		jge loop12
		
do1:
		test edx, edx
		jz done

loop1:
		movd xmm0, DWORD PTR [esi+edi*4] // mm0 holds [L 0]
		movd xmm1, DWORD PTR [ecx+edi*4] // mm1 holds [R 0]

		punpckldq xmm0, xmm1 // xmm0 holds [L R]
		psrad xmm0, xmm7
		packssdw xmm0, xmm0 // convert to int16, saturating
		movd DWORD PTR[eax+edi*4], xmm0 // store lower 32 bits to destination

		add edi, 1
		dec edx
		jnz loop1

done:
		pop edi
		pop esi
		
		ret 0
	}
}

static __declspec(naked) void Convert_S32_S16_stereo_shift_AlignedSource(int16_t *destination_buffer, const int32_t **source_buffer, size_t channels, size_t count_per_channel, unsigned int decimation_bits)
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
		movd xmm7, DWORD PTR 20[esp] // lower portion of xmm7 holds decimation bits
		

		push esi
		push edi
		mov esi, DWORD PTR 0[ecx] // esi holds left
		mov ecx, DWORD PTR 4[ecx] // ecx holds right
		xor edi, edi
		
		cmp edx, 12	
		jl do1
loop12:
		movaps xmm1, XMMWORD PTR 0[esi+edi*4] // 4 samples of left [L0 L1 L2 L3]
		movaps xmm2, XMMWORD PTR 16[esi+edi*4]
		movaps xmm5, XMMWORD PTR 32[esi+edi*4]

		movaps xmm3, XMMWORD PTR 0[ecx+edi*4] // 4 samples of right [R0 R1 R2 R3]
		movaps xmm4, XMMWORD PTR 16[ecx+edi*4] 
		movaps xmm6, XMMWORD PTR 32[ecx+edi*4] 

		// interleave
		movdqa xmm0, xmm1
		punpckhdq xmm1, xmm3 
		punpckldq xmm0, xmm3 

		movdqa xmm3, xmm2
		punpckhdq xmm3, xmm4
		punpckldq xmm2, xmm4

		movdqa xmm4, xmm5
		punpckhdq xmm5, xmm6
		punpckldq xmm4, xmm6

		// shift right by 8
		psrad xmm0, xmm7
		psrad xmm1, xmm7
		psrad xmm2, xmm7
		psrad xmm3, xmm7
		psrad xmm4, xmm7
		psrad xmm5, xmm7

		// convert to int16, saturing
		packssdw xmm0, xmm1
		packssdw xmm2, xmm3
		packssdw xmm4, xmm5
		
		// store
		movdqu XMMWORD PTR [eax+edi*4], xmm0
		movdqu XMMWORD PTR 16[eax+edi*4], xmm2
		movdqu XMMWORD PTR 32[eax+edi*4], xmm4

		add edi, 12
		sub edx, 12
		cmp edx, 12
		jge loop12
		
do1:
		test edx, edx
		jz done

loop1:
		movd xmm0, DWORD PTR [esi+edi*4] // mm0 holds [L 0]
		movd xmm1, DWORD PTR [ecx+edi*4] // mm1 holds [R 0]

		punpckldq xmm0, xmm1 // xmm0 holds [L R]
		psrad xmm0, xmm7
		packssdw xmm0, xmm0 // convert to int16, saturating
		movd DWORD PTR[eax+edi*4], xmm0 // store lower 32 bits to destination

		add edi, 1
		dec edx
		jnz loop1

done:
		pop edi
		pop esi
		
		ret 0
	}
}

static __declspec(naked) void Convert_S32_S16_stereo_shift_AlignedDestination(int16_t *destination_buffer, const int32_t **source_buffer, size_t channels, size_t count_per_channel, unsigned int decimation_bits)
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
		movd xmm7, DWORD PTR 20[esp] // lower portion of xmm7 holds decimation bits
		

		push esi
		push edi
		mov esi, DWORD PTR 0[ecx] // esi holds left
		mov ecx, DWORD PTR 4[ecx] // ecx holds right
		xor edi, edi
		
		cmp edx, 12	
		jl do1
loop12:
		movups xmm1, XMMWORD PTR 0[esi+edi*4] // 4 samples of left [L0 L1 L2 L3]
		movups xmm2, XMMWORD PTR 16[esi+edi*4]
		movups xmm5, XMMWORD PTR 32[esi+edi*4]

		movups xmm3, XMMWORD PTR 0[ecx+edi*4] // 4 samples of right [R0 R1 R2 R3]
		movups xmm4, XMMWORD PTR 16[ecx+edi*4] 
		movups xmm6, XMMWORD PTR 32[ecx+edi*4] 

		// interleave
		movdqa xmm0, xmm1
		punpckhdq xmm1, xmm3 
		punpckldq xmm0, xmm3 

		movdqa xmm3, xmm2
		punpckhdq xmm3, xmm4
		punpckldq xmm2, xmm4

		movdqa xmm4, xmm5
		punpckhdq xmm5, xmm6
		punpckldq xmm4, xmm6

		// shift right by 8
		psrad xmm0, xmm7
		psrad xmm1, xmm7
		psrad xmm2, xmm7
		psrad xmm3, xmm7
		psrad xmm4, xmm7
		psrad xmm5, xmm7

		// convert to int16, saturing
		packssdw xmm0, xmm1
		packssdw xmm2, xmm3
		packssdw xmm4, xmm5
		
		// store
		movdqa XMMWORD PTR [eax+edi*4], xmm0
		movdqa XMMWORD PTR 16[eax+edi*4], xmm2
		movdqa XMMWORD PTR 32[eax+edi*4], xmm4

		add edi, 12
		sub edx, 12
		cmp edx, 12
		jge loop12
		
do1:
		test edx, edx
		jz done

loop1:
		movd xmm0, DWORD PTR [esi+edi*4] // mm0 holds [L 0]
		movd xmm1, DWORD PTR [ecx+edi*4] // mm1 holds [R 0]

		punpckldq xmm0, xmm1 // xmm0 holds [L R]
		psrad xmm0, xmm7
		packssdw xmm0, xmm0 // convert to int16, saturating
		movd DWORD PTR[eax+edi*4], xmm0 // store lower 32 bits to destination

		add edi, 1
		dec edx
		jnz loop1

done:
		pop edi
		pop esi
		
		ret 0
	}
}

static __declspec(naked) void Convert_S32_S16_stereo_shift_Unaligned(int16_t *destination_buffer, const int32_t **source_buffer, size_t channels, size_t count_per_channel, unsigned int decimation_bits)
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
		movd xmm7, DWORD PTR 20[esp] // lower portion of xmm7 holds decimation bits
		

		push esi
		push edi
		mov esi, DWORD PTR 0[ecx] // esi holds left
		mov ecx, DWORD PTR 4[ecx] // ecx holds right
		xor edi, edi
		
		cmp edx, 12	
		jl do1
loop12:
		movups xmm1, XMMWORD PTR 0[esi+edi*4] // 4 samples of left [L0 L1 L2 L3]
		movups xmm2, XMMWORD PTR 16[esi+edi*4]
		movups xmm5, XMMWORD PTR 32[esi+edi*4]

		movups xmm3, XMMWORD PTR 0[ecx+edi*4] // 4 samples of right [R0 R1 R2 R3]
		movups xmm4, XMMWORD PTR 16[ecx+edi*4] 
		movups xmm6, XMMWORD PTR 32[ecx+edi*4] 

		// interleave
		movdqa xmm0, xmm1
		punpckhdq xmm1, xmm3 
		punpckldq xmm0, xmm3 

		movdqa xmm3, xmm2
		punpckhdq xmm3, xmm4
		punpckldq xmm2, xmm4

		movdqa xmm4, xmm5
		punpckhdq xmm5, xmm6
		punpckldq xmm4, xmm6

		// shift right by 8
		psrad xmm0, xmm7
		psrad xmm1, xmm7
		psrad xmm2, xmm7
		psrad xmm3, xmm7
		psrad xmm4, xmm7
		psrad xmm5, xmm7

		// convert to int16, saturing
		packssdw xmm0, xmm1
		packssdw xmm2, xmm3
		packssdw xmm4, xmm5
		
		// store
		movdqu XMMWORD PTR [eax+edi*4], xmm0
		movdqu XMMWORD PTR 16[eax+edi*4], xmm2
		movdqu XMMWORD PTR 32[eax+edi*4], xmm4

		add edi, 12
		sub edx, 12
		cmp edx, 12
		jge loop12
		
do1:
		test edx, edx
		jz done

loop1:
		movd xmm0, DWORD PTR [esi+edi*4] // mm0 holds [L 0]
		movd xmm1, DWORD PTR [ecx+edi*4] // mm1 holds [R 0]

		punpckldq xmm0, xmm1 // xmm0 holds [L R]
		psrad xmm0, xmm7
		packssdw xmm0, xmm0 // convert to int16, saturating
		movd DWORD PTR[eax+edi*4], xmm0 // store lower 32 bits to destination

		add edi, 1
		dec edx
		jnz loop1

done:
		pop edi
		pop esi
		
		ret 0
	}
}

void x86_SSE2_nsmath_pcm_Convert_S32_S16_stereo_decimate(int16_t *destination, const int32_t **source, size_t channels, size_t count_per_channel, unsigned int decimation_bits)
{
	int source_aligned = ((size_t)source[0] & 0xF) == 0 && ((size_t)source[1] & 0xF) == 0;
	int destination_aligned = ((size_t)destination & 0xF) == 0;
	if (destination_aligned && source_aligned)
	{
		Convert_S32_S16_stereo_shift_Aligned(destination, source, channels, count_per_channel, decimation_bits);
	}
	else if (source_aligned)
	{
		Convert_S32_S16_stereo_shift_AlignedSource(destination, source, channels, count_per_channel, decimation_bits);
	}
	else if (destination_aligned)
	{
		Convert_S32_S16_stereo_shift_AlignedDestination(destination, source, channels, count_per_channel, decimation_bits);
	}
	else
	{
		Convert_S32_S16_stereo_shift_Unaligned(destination, source, channels, count_per_channel, decimation_bits);
	}
}

