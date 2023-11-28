

// =====================================================================================
   _declspec(naked) static void _cdecl FUNC_NAME(SCAST *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, CUBIC_DAT* dat)
// =====================================================================================
{
	_asm 
	{	
		push ebp
		push ebx
		push edi
		push esi	//+16
		mov ebp,[esp+48]
		mov ebx,[esp+44]
		mov edi,[esp+24]
		mov esi,[esp+20]
		mov eax,[esp+36]
		mov edx,[esp+40]
		cmp edx,0
		mov ecx,1
		jge ldx0
		dec eax
		mov ecx,-1
		sbb edx,0
		xor eax,0xFFFFFFFF
		xor edx,0xFFFFFFFF
ldx0:
		mov [esp+36],eax
		mov [esp+40],edx
		push ecx	//+16+4
		cmp dword ptr [ebp+20],0
		jnz lloop
		mov eax,[esp+4+32]	
#ifdef CUBIC_8BIT
		add eax,esi
		movsx dx,byte ptr [eax+ecx*2]
		movsx edx,dx
		push edx
		movsx dx, byte ptr [eax]
		movsx ax, byte ptr [eax+ecx]
		movsx edx,dx
		movsx eax,ax
		push eax	
#else
		lea eax,[eax*2+esi]
		movsx edx,word ptr [eax+ecx*4]
		push edx
		movsx edx, word ptr [eax]
		movsx eax, word ptr [eax+ecx*2]
		push eax
#endif

		mov ecx,ebp
		call dword ptr cubic_advance_ptr
lloop:
		mov edx,[esp+4+28]
		imul edx,[esp]
		mov ecx,ebp
		call dword ptr [cubic_calc_ptr]

	
		FOO_BLOCK

		mov edx,[esp+4+40]
		mov eax,[esp+4+36]
		push edx	//+16+4+8
		push eax
iloop:
		mov eax,dword ptr [esp+12+28]
		xor edx,edx
		neg eax
		imul eax,[esp+8]
		jnz ld
		mov edx,1
ld:
		cmp dword ptr [esp+4],0
		jnz ld1
		test edx,edx
		jnz ld2
		cmp eax,dword ptr [esp]
		jbe ld1
ld2:
		mov eax,[esp]
		cmp [esp+8],0
		mov edx,[esp+4]
		jl ld3
		add dword ptr [esp+12+28],eax
		adc dword ptr [esp+12+32],edx
		jmp loop_end
ld3:
		sub dword ptr [esp+12+28],eax
		sbb dword ptr [esp+12+32],edx
		jmp loop_end
ld1:
		cmp [esp+8],0
		jl ld4
		add dword ptr [esp+12+28],eax
		adc dword ptr [esp+12+32],edx
		jmp ld5
ld4:
		sub dword ptr [esp+12+28],eax
		sbb dword ptr [esp+12+32],edx
ld5:
		sub dword ptr [esp],eax
		sbb dword ptr [esp+4],edx
		mov eax,dword ptr [esp+12+32]
		mov ecx,dword ptr [esp+8]

#ifdef CUBIC_8BIT
		add eax,esi
		movsx dx,byte ptr [eax+ecx*2]
		movsx edx,dx
		push edx
		movsx dx, byte ptr [eax]
		movsx ax, byte ptr [eax+ecx]
		movsx edx,dx
		movsx eax,ax
		push eax	
#else
		lea eax,[eax*2+esi]
		movsx edx,word ptr [eax+ecx*4]
		push edx
		movsx edx, word ptr [eax]
		movsx eax, word ptr [eax+ecx*2]
		push eax
#endif
		mov ecx,ebp
		call dword ptr cubic_advance_ptr

		jmp iloop
loop_end:
		dec ebx
		lea esp,[esp+8]
		jnz lloop
		pop eax
		pop esi
		pop edi
		pop ebx
		pop ebp
		ret	
	}
}

