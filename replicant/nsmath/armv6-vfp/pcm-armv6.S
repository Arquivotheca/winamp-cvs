.text
.align
	
.global nsmath_pcm_Convert_F32_S16_armv6_VFP
.type nsmath_pcm_Convert_F32_S16_armv6_VFP, %function


/* r0 = destination
   r1 = source (float *)
	 r2 = count
	*/
nsmath_pcm_Convert_F32_S16_armv6_VFP:
/* registers in use
  r0 = destination
	r1 = source (float *)
	r2 = sample count
	s0 = total gain
	*/
	flds s0, .const32768  /* s0 = 32768.0f */

	cmp r2, #0
	bxeq lr
1:		
	flds s1, [r0] /* load sample */
	add r0, r0, #4 /* update pointer */
	fmuls s1, s1, s0 /* multiply by 32768 */
	ftosis s1, s1 /* convert to integer */
	fmrs r3, s1 /* move to an arm register */
	ssat r3, #16, r3 /* signed saturate to 16 bits */
	strh r3, [r1], #4 /* store signed 16bit value and update pointer */
	subs r2, #1 /* update count */
	beq 1b
	bx lr

.const32768:
.word	0x47000000 /* 32768.0f */