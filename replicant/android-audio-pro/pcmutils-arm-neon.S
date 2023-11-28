.text
.align

.weak	Float32_To_Int16_Clip
.hidden	Float32_To_Int16_Clip
.type	Float32_To_Int16_Clip, %function
	
.global Float32_To_Int16_Clip_NEON
.type Float32_To_Int16_Clip_NEON, %function

/* r0 = destination
   r1 = source (float *)
	 r2 = count
	 r3 = gain (as a float moved into an ARM register) */
Float32_To_Int16_Clip_NEON:
/* registers in use
  r0 = destination
	r1 = source (float *)
	r2 = sample count
	s0 = total gain
	q1/d2:d3/s4:s7 = total gain in all lanes 
	*/
  fmsr s0, r3 /* s0 = gain */
	flds s1, .const32768  /* s1 = 32768.0f */
	fmuls s0, s0, s1 /* s0 =  gain*32768.0f */

1:	
	cmp r2, #4
	blo Float32_To_Int16_Clip /* call this function for leftovers */

	vld1.32 {d4-d5}, [r1]! /* load source samples and increment pointer */
	vmul.f32 q2, q2, d0[0] /* multiply by gain */
	vcvt.s32.f32 q2, q2 /* convert to 32bit integer */
	vqmovn.s32 d1, q2 /* saturate to 16bit integer */
	vst1.16 d1, [r0]! /* store in destination and increment pointer */
	subs r2, #4 /* update count */
	bxeq lr /* if we got exactly the correct count, return */
	b 1b /* loop */
.const32768:
.word	0x47000000 /* 32768.0f */
/*
   0:	2a03      	cmp	r2, #3
   2:	ee06 3a10 	fmsr	s12, r3
   6:	b470      	push	{r4, r5, r6}
   8:	eddf 6a21 	flds	s13, [pc, #132]
   c:	4605      	mov	r5, r0
   e:	ee66 6a26 	fmuls	s13, s12, s13
  12:	d91d      	bls.n	50 <Float32_To_Int16_Clip+0x50>
  14:	4614      	mov	r4, r2
  16:	460b      	mov	r3, r1
  18:	f963 0a8f 	vld1.32	{d16-d17}, [r3]
  1c:	ee16 6a90 	fmrs	r6, s13
  20:	ee07 6b10 	vmov.32	d7[0], r6
  24:	3c04      	subs	r4, #4
  26:	ffe0 09c7 	vmul.f32	q8, q8, d7[0]
  2a:	3310      	adds	r3, #16
  2c:	fffb 0760 	vcvt.s32.f32	q8, q8
  30:	fff6 02a0 	vqmovn.s32	d16, q8
  34:	f945 074f 	vst1.16	{d16}, [r5]
  38:	3508      	adds	r5, #8
  3a:	2c03      	cmp	r4, #3
  3c:	d8ec      	bhi.n	18 <Float32_To_Int16_Clip+0x18>
  3e:	3a04      	subs	r2, #4
  40:	0893      	lsrs	r3, r2, #2
  42:	f002 0203 	and.w	r2, r2, #3	; 0x3
  46:	3301      	adds	r3, #1
  48:	eb00 05c3 	add.w	r5, r0, r3, lsl #3
  4c:	eb01 1103 	add.w	r1, r1, r3, lsl #4
  50:	b1da      	cbz	r2, 8a <Float32_To_Int16_Clip+0x8a>
  52:	f248 0400 	movw	r4, #32768	; 0x8000
  56:	2300      	movs	r3, #0
  58:	f647 76ff 	movw	r6, #32767	; 0x7fff
  5c:	f6cf 74ff 	movt	r4, #65535	; 0xffff
  60:	eb01 0083 	add.w	r0, r1, r3, lsl #2
  64:	edd0 7a00 	flds	s15, [r0]
  68:	ee66 7aa7 	fmuls	s15, s13, s15
  6c:	eefd 7ae7 	ftosizs	s15, s15
  70:	ee17 0a90 	fmrs	r0, s15
  74:	42b0      	cmp	r0, r6
  76:	bfa8      	it	ge
  78:	4630      	movge	r0, r6
  7a:	42a0      	cmp	r0, r4
  7c:	bfb8      	it	lt
  7e:	4620      	movlt	r0, r4
  80:	f825 0013 	strh.w	r0, [r5, r3, lsl #1]
  84:	3301      	adds	r3, #1
  86:	429a      	cmp	r2, r3
  88:	d1ea      	bne.n	60 <Float32_To_Int16_Clip+0x60>
  8a:	bc70      	pop	{r4, r5, r6}
  8c:	4770      	bx	lr
  8e:	bf00      	nop

*/