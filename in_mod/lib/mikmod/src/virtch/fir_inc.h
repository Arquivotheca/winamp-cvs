// =====================================================================================
_declspec(naked) static void _cdecl FUNC_NAME2(SCAST *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo)
// =====================================================================================
{
	_asm 
	{	
		push ebx
		push ecx
		push edi
		push esi	//+16
		mov ebx,[esp+44]
		mov edi,[esp+24]
		mov esi,[esp+20]
		movq mm4,[esp+28]
		movq mm5,[esp+36]
_loop:
		movq mm6,mm4
		psrlq mm6,32
		movd eax,mm6
		movd ecx,mm4
#ifdef FIR_8BIT
		add eax,esi
		pxor mm0,mm0
		pxor mm1,mm1
		movq mm2,[eax]
		punpcklbw mm0,mm2
		punpckhbw mm1,mm2
/*
		psraw mm0,8
		psraw mm1,8
*/
#else
		lea eax,[eax*2+esi]
		movq mm0,[eax]
		movq mm1,[eax+8]
#endif

		shr ecx,17
		add ecx,8
		and ecx,0FFF0h

		movq mm2,fir_lut[ecx]
		movq mm3,fir_lut[ecx+8]
		pmaddwd mm0,mm2
		pmaddwd mm1,mm3
		paddd mm0,mm1
		movd eax,mm0
		psrlq mm0,32
		movd ecx,mm0
		add eax,ecx
		sar eax,14

		cmp eax,32767
		jle clip1
		mov eax,32767

clip1:
		cmp eax,-32768
		jge clip2
		mov eax,-32768

clip2:
	
		FOO_BLOCK

/*		paddq mm4,mm5 -how lame, p55c does not support this */
		movd eax,mm4
		movd ecx,mm5
		add eax,ecx
		paddd mm4,mm5
#if 1	/* okay, which is slower, branching, or this? */
		jnc no_carry
		mov eax,1
#else
		setc al
		movzx eax,al
#endif
		movd mm0,eax
		psllq mm0,32
		paddd mm4,mm0
no_carry:

		dec ebx
		jnz _loop
		emms
		pop esi
		pop edi
		pop ecx
		pop ebx
		ret	

	}
}


static void _cdecl FUNC_NAME(SCAST *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo)
{
	vxx = *volinfo;
	FUNC_NAME2(srce, dest, index, increment, todo);
	*volinfo = vxx;
}
