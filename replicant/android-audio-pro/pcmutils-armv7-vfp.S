.text
.align
	
.global Float32_To_Int16_Clip
.type Float32_To_Int16_Clip, %function


/* r0 = destination
   r1 = source (float *)
	 r2 = count
	*/
Float32_To_Int16_Clip:
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
	flds s1, [r1] /* load sample */
	add r1, r1, #4 /* update pointer */
	fmuls s1, s1, s0 /* multiply by 32768 */	
	ftosizs s1, s1 /* convert to integer */
	subs r2, #1 /* update count */
	fmrs r3, s1 /* move to an arm register */	
	ssat r3, #16, r3 /* signed saturate to 16 bits */	
	strh r3, [r0], #2 /* store signed 16bit value and update pointer */	
	bne 1b
	bx lr

.align 2
.const32768:
.word	0x47000000 /* 32768.0f */