.text
.align

/* armv7 has looser alignment requirements, and we're going to take advantage of that */
    
.global bytereader_init
.type bytereader_init, %function
.global bytereader_advance
.type bytereader_advance, %function
.global bytereader_reset
.type bytereader_reset, %function

.global bytereader_show_n
.type bytereader_show_n, %function
.global bytereader_read_n
.type bytereader_read_n, %function

.global bytereader_show_u8
.type bytereader_show_u8, %function
.global bytereader_read_u8
.type bytereader_read_u8, %function
.global bytereader_show_s8
.type bytereader_show_s8, %function
.global bytereader_read_s8
.type bytereader_read_s8, %function

.global bytereader_show_u16_le
.type bytereader_show_u16_le, %function
.global bytereader_read_u16_le
.type bytereader_read_u16_le, %function
.global bytereader_show_s16_le
.type bytereader_show_s16_le, %function
.global bytereader_read_s16_le
.type bytereader_read_s16_le, %function

.global bytereader_show_u16_be
.type bytereader_show_u16_be, %function
.global bytereader_read_u16_be
.type bytereader_read_u16_be, %function
.global bytereader_show_s16_be
.type bytereader_show_s16_be, %function
.global bytereader_read_s16_be
.type bytereader_read_s16_be, %function

.global bytereader_show_u32_le
.type bytereader_show_u32_le, %function
.global bytereader_read_u32_le
.type bytereader_read_u32_le, %function

.global bytereader_show_u32_be
.type bytereader_show_u32_be, %function
.global bytereader_read_u32_be
.type bytereader_read_u32_be, %function

.global bytereader_show_f32_be
.type bytereader_show_f32_be, %function
.global bytereader_read_f32_be
.type bytereader_read_f32_be, %function

.global bytereader_show_f32_le
.type bytereader_show_f32_le, %function
.global bytereader_read_f32_le
.type bytereader_read_f32_le, %function


/* data structure layout 
[0-3] size
[4-7] current pointer
[8-11] original pointer */

/* r0 = reader
   r1 = pointer
	 r2 = size
	 */
bytereader_init:
	str r2, [r0], #4 /* store size */
	str r1, [r0], #4 /* store pointer */
	str r1, [r0] /* store original pointer */
	bx lr
	/* r0 = reader
	r1 = count 
	*/
bytereader_advance:
	ldm r0, {r2-r3} /* load size (r2) and current pointer (r3) */
	sub r2, r2, r1 /* subtract size */
	add r3, r3, r1 /* advance pointer */
	stm r0, {r2-r3}
	bx lr

/* r0 = reader */	
bytereader_reset:
	ldm r0, {r1-r3} /* load size (r1), current pointer (r2) and original pointer (r3) */
	sub r12, r2, r3 /* r12 is the difference amount */
	add r1, r1, r12 /* add to the size */
	mov r2, r3 /* reset the current pointer */
	stm r0, {r2-r3}
	bx lr

/* --- n byte functions --- */
bytereader_show_n:
	ldr r3, [r0, #4]
	mov r0, r1
	mov r1, r3
	/* to call memcpy, r0 = destination (r1 for us), r1 = source, r2 = size (already there) */
	b memcpy(PLT)

/* r0 = reader
   r1 = destination pointer
   r2 = size */
bytereader_read_n:
    ldr r12, [r0] /* r12 holds the remaining buffer size */
	ldr r3, [r0, #4] /* r3 holds the data pointer */
	sub r12, r12, r2 /* subtract size */
	str r12, [r0] /* and store */
	add r12, r3, r2 /* increment pointer */
	str r12, [r0, #4] /* and store */
	mov r0, r1 /* move destination to r0 for memcpy */
	mov r1, r3 /* move data pointer (pre-increment) to r1 for memcpy */
	/* r2 already holds size for memcpy */
	b memcpy(PLT)

/* --- 1 byte functions --- */
/* r0 = reader */
bytereader_show_u8:
	ldr r1, [r0, #4] /* load current pointer (r1) */
	ldrb r0, [r1] /* load unsigned byte */
	bx lr

/* r0 = reader */	
bytereader_read_u8:
	ldm r0, {r1-r2} /* load size (r1) and current pointer (r2) */
	ldrb r3, [r2], #1 /* load unsigned byte into r3 (we're not done with reader yet so we can't clobber r0)*/
	sub r1, r1, #1 /* subtract 1 from size */
	stm r0, {r1-r2} /* store updated values */
	mov r0, r3 /* move into return value */
	bx lr

/* r0 = reader */
bytereader_show_s8:
	ldr r1, [r0, #4] /* load current pointer (r1) */
	ldrsb r0, [r1] /* load signed byte */
	bx lr

/* r0 = reader */	
bytereader_read_s8:
	ldm r0, {r1-r2} /* load size (r1) and current pointer (r2) */
	ldrsb r3, [r2], #1 /* load signed byte into r3 (we're not done with reader yet so we can't clobber r0) */
	sub r1, r1, #1 /* subtract 1 from size */
	stm r0, {r1-r2} /* store updated values */
	mov r0, r3 /* move into return value */
	bx lr


/* --- 2 byte little endian functions --- */
bytereader_show_u16_le:
 	ldr r1, [r0, #4] /* load current pointer (r1) */
	ldrh r0, [r1] /* load unsigned half-word */
	bx lr
	
bytereader_read_u16_le:
	ldm r0, {r1-r2} /* load size (r1) and current pointer (r2) */
	ldrh r3, [r2], #2 /* load unsigned half-word into r3 (we're not done with reader yet so we can't clobber r0)*/
	sub r1, r1, #2 /* subtract 2 from size */
	stm r0, {r1-r2} /* store updated values */
	mov r0, r3 /* move into return value */
	bx lr
	
bytereader_show_s16_le:
	ldr r1, [r0, #4] /* load current pointer (r1) */
	ldrsh r0, [r1] /* load signed half-word */
	bx lr
	
bytereader_read_s16_le:
	ldm r0, {r1-r2} /* load size (r1) and current pointer (r2) */
	ldrsh r3, [r2], #2 /* load signed half-word into r3 (we're not done with reader yet so we can't clobber r0)*/
	sub r1, r1, #2 /* subtract 2 from size */
	stm r0, {r1-r2} /* store updated values */
	mov r0, r3 /* move into return value */
	bx lr
	
	/* --- 2 byte big-endian functions --- */
bytereader_show_u16_be:
 	ldr r1, [r0, #4] /* load current pointer (r1) */
	ldrh r0, [r1] /* load unsigned half-word */
	rev16 r0, r0 /* swap endian */
	bx lr
	
bytereader_read_u16_be:
	ldm r0, {r1-r2} /* load size (r1) and current pointer (r2) */
	ldrh r3, [r2], #2 /* load unsigned half-word into r3 (we're not done with reader yet so we can't clobber r0)*/
	sub r1, r1, #2 /* subtract 2 from size */
	stm r0, {r1-r2} /* store updated values */
	rev16 r0, r3 /* swap endian */
	bx lr
	
bytereader_show_s16_be:
	ldr r1, [r0, #4] /* load current pointer (r1) */
	ldrsh r0, [r1] /* load signed half-word */
	revsh r0, r0 /* swap endian (signed) */
	bx lr
	
bytereader_read_s16_be:
	ldm r0, {r1-r2} /* load size (r1) and current pointer (r2) */
	ldrsh r3, [r2], #2 /* load signed half-word into r3 (we're not done with reader yet so we can't clobber r0)*/
	sub r1, r1, #2 /* subtract 2 from size */
	stm r0, {r1-r2} /* store updated values */
	revsh r0, r3 /* swap endian (signed) */
	bx lr

/* --- 4 byte little-endian functions --- */
bytereader_show_u32_le:
bytereader_show_f32_le: /* since floats get returned in r0, we can just re-use this */
 	ldr r1, [r0, #4] /* load current pointer (r1) */
	ldr r0, [r1] /* load word */
	bx lr

bytereader_read_u32_le:
bytereader_read_f32_le: /* since floats get returned in r0, we can just re-use this */
	ldm r0, {r1-r2} /* load size (r1) and current pointer (r2) */
	ldr r3, [r2], #4 /* load word into r3 (we're not done with reader yet so we can't clobber r0)*/
	sub r1, r1, #4 /* subtract 4 from size */
	stm r0, {r1-r2} /* store updated values */
	bx lr
	

	
/* --- 4 byte big-endian functions --- */
bytereader_show_u32_be:
bytereader_show_f32_be: /* since floats get returned in r0, we can just re-use this */
 	ldr r1, [r0, #4] /* load current pointer (r1) */
	ldr r0, [r1] /* load word */
	rev r0, r0 /* swap endian */
	bx lr

bytereader_read_u32_be:
bytereader_read_f32_be: /* since floats get returned in r0, we can just re-use this */
	ldm r0, {r1-r2} /* load size (r1) and current pointer (r2) */
	ldr r3, [r2], #4 /* load word into r3 (we're not done with reader yet so we can't clobber r0)*/
	sub r1, r1, #4 /* subtract 4 from size */
	stm r0, {r1-r2} /* store updated values */
	rev r0, r3 /* swap endian */
	bx lr
	
