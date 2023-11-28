.text
.align

.weak	nsmath_pcm_Convert_F32_S16
.hidden	nsmath_pcm_Convert_F32_S16
.type	nsmath_pcm_Convert_F32_S16, %function
	
.global nsmath_pcm_Convert_F32_S16_NEON
.type nsmath_pcm_Convert_F32_S16_NEON, %function

.global nsmath_pcm_Convert_F32_S16_x4_NEON
.type nsmath_pcm_Convert_F32_S16_x4_NEON, %function

.global nsmath_pcm_Convert_F32_S16_x32_NEON
.type nsmath_pcm_Convert_F32_S16_x32_NEON, %function

/* r0 = destination
   r1 = source (float *)
	 r2 = count
	*/
nsmath_pcm_Convert_F32_S16_NEON:
/* registers in use
  r0 = destination
	r1 = source (float *)
	r2 = sample count
	s0 = total gain
	q1/d2:d3/s4:s7 = total gain in all lanes 
	*/
	flds s0, .const32768  /* s0 = 32768.0f */

1:	
	cmp r2, #4
	blo nsmath_pcm_Convert_F32_S16 /* call this function for leftovers */

	vld1.32 {d4-d5}, [r1]! /* load source samples and increment pointer */
	vmul.f32 q2, q2, d0[0] /* multiply by gain */
	vcvt.s32.f32 q2, q2 /* convert to 32bit integer */
	vqmovn.s32 d1, q2 /* saturate to 16bit integer */
	vst1.16 d1, [r0]! /* store in destination and increment pointer */
	subs r2, #4 /* update count */
	bxeq lr /* if we got exactly the correct count, return */
	b 1b /* loop */
//.const32768:
//.word	0x47000000 /* 32768.0f */

/* r0 = destination
   r1 = source (float *)
	 r2 = count - always a multiple of 4
	*/
nsmath_pcm_Convert_F32_S16_x4_NEON:
/* registers in use
  r0 = destination
	r1 = source (float *)
	r2 = sample count
	s0 = total gain
	*/
	flds s0, .const32768  /* s0 = 32768.0f */

1:	
	subs r2, #4

	vld1.32 {d4-d5}, [r1]! /* load source samples and increment pointer */
	vmul.f32 q2, q2, d0[0] /* multiply by gain */
	vcvt.s32.f32 q2, q2 /* convert to 32bit integer */
	vqmovn.s32 d1, q2 /* saturate to 16bit integer */
	vst1.16 d1, [r0]! /* store in destination and increment pointer */
	bxeq lr /* if we got exactly the correct count, return */
	b 1b /* loop */
//.const32768:
//.word	0x47000000 /* 32768.0f */

/* r0 = destination
   r1 = source (float *)
	 r2 = count - always a multiple of 32
	*/
nsmath_pcm_Convert_F32_S16_x32_NEON:
/* registers in use
  r0 = destination
	r1 = source (float *)
	r2 = sample count
	s0 = total gain
	q1/d2:d3/s4:s7 = total gain in all lanes 
	*/
	flds s0, .const32768  /* s0 = 32768.0f */

1:	
	subs r2, #32

	vldm  r1!, {d2-d17} /* load 32 source samples and increment pointer */
	vmul.f32 q1, q1, d0[0] /* multiply by gain */
	vmul.f32 q2, q2, d0[0] /* multiply by gain */
	vmul.f32 q3, q3, d0[0] /* multiply by gain */
	vmul.f32 q4, q4, d0[0] /* multiply by gain */
	vmul.f32 q5, q5, d0[0] /* multiply by gain */
	vmul.f32 q6, q6, d0[0] /* multiply by gain */
	vmul.f32 q7, q7, d0[0] /* multiply by gain */
	vmul.f32 q8, q8, d0[0] /* multiply by gain */

	vcvt.s32.f32 q1, q1 /* convert to 32bit integer */
	vcvt.s32.f32 q2, q2 /* convert to 32bit integer */
	vcvt.s32.f32 q3, q3 /* convert to 32bit integer */
	vcvt.s32.f32 q4, q4 /* convert to 32bit integer */
	vcvt.s32.f32 q5, q5 /* convert to 32bit integer */
	vcvt.s32.f32 q6, q6 /* convert to 32bit integer */
	vcvt.s32.f32 q7, q7 /* convert to 32bit integer */
	vcvt.s32.f32 q8, q8 /* convert to 32bit integer */

	vqmovn.s32 d1, q1 /* saturate to 16bit integer */
	vqmovn.s32 d2, q2 /* saturate to 16bit integer */
	vqmovn.s32 d3, q3 /* saturate to 16bit integer */
	vqmovn.s32 d4, q4 /* saturate to 16bit integer */
	vqmovn.s32 d5, q5 /* saturate to 16bit integer */
	vqmovn.s32 d6, q6 /* saturate to 16bit integer */
	vqmovn.s32 d7, q7 /* saturate to 16bit integer */
	vqmovn.s32 d8, q8 /* saturate to 16bit integer */

	vstm r0!, {d1-d8} /* store in destination and increment pointer */
	bxeq lr /* if we got exactly the correct count, return */
	b 1b /* loop */
.const32768:
.word	0x47000000 /* 32768.0f */