.686
.mmx
.k3d
.xmm
.model flat,C

.code

;typedef struct
;{
;	float ia,ib,ic,s1;
;	long prev;
;	BOOL ok;
;} CUBIC_DAT;

cubic_calc_3d proc
		shr edx,1	;hack to avoid weirdness with converting uint to float
		movd mm0,edx
		pi2fd mm0,mm0
		movq mm2,mm0
		pfmul mm0,mm0
		movq mm1,mm0
		pfmul mm0,mm2
		pfmul mm2,qword ptr [ecx+8]	;CUBIC_DAT.ic
		pfmul mm1,qword ptr [ecx+4]	;CUBIC_DAT.ib		
		pfmul mm0,qword ptr [ecx]	;CUBIC_DAT.ia
		pfadd mm1,mm2
		pfadd mm1,mm0
		pf2id mm1,mm1
		movd eax,mm1
		femms
		add eax,dword ptr [ecx+16]		;CUBIC_DAT.prev
		ret
cubic_calc_3d endp

;const float h=(float)((INT64S)1<<(FRACBITS-1));///hack

;const_1p2h=1/(2*h), const_h=h, const_h2=h*h, const_1ph2 = 1/(h*h), const_1ph3=1/(h*h*h)
;const_3 = 3
externdef const_h:QWORD
externdef const_1p2h:QWORD
externdef const_h2:QWORD
externdef const_1ph2:QWORD
externdef const_1ph3:QWORD
externdef const_3:QWORD

cubic_advance_3d proc	;dat: ecx s1: edx    s2: [esp+4]   s3: [esp+8]
	mov eax,[esp+8]
	mov dword ptr [ecx+16],edx

	sub eax,edx
	sub edx,[esp+4]
	movd mm3,[ecx+12]
	neg edx
	movd mm0,eax	;s3-s1
	movd mm1,edx	;s2-s1

	pi2fd mm0,mm0
	pi2fd mm1,mm1	;fh

	pfmul mm0,const_1p2h	;new_s1
	movd [ecx+8],mm3	;ic=s1

	movq mm2,mm1
	movd [ecx+12],mm0	;s1=new_s1

	movq mm4,mm3
	;mm0: new_s1, mm1: fh, mm2: fh  mm3: ic  mm4: ic
	pfadd mm3,mm3	;mm3:2*ic

	pfmul mm2,const_3	;mm2: fh*3	
	;mm0: new_s1, mm1: fh, mm2: fh*3  mm3: 2*ic  mm4: ic
	pfadd mm0,mm3

	pfmul mm0,const_h

	pfsub mm2,mm0

	pfmul mm2,const_1ph2
	;mm0: (new_s1+fh*3)*h, mm1: fh, mm2: ib  mm3: 2*ic  mm4: ic
	movd [ecx+4],mm2
	pfmul mm2,const_h

	pfadd mm2,mm4	;mm2: h*(ib+ic)

	pfmul mm2,const_h

	pfsub mm1,mm2

	pfmul mm1,const_1ph3

	movd [ecx],mm1

	mov dword ptr [ecx+20],1
	femms
	ret 8
cubic_advance_3d endp

END