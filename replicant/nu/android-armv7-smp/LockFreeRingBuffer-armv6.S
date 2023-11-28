.text
.align
		
.global ringbuffer_write
.type ringbuffer_write, %function

.global ringbuffer_read
.type ringbuffer_read, %function

/* memory layout
[0-3] available <--- this is our atomic synchronization point!
[4-7] size
[8-11] write head
[12-15] read head
[16-] buffer
*/

/* r0 = ringbuffer pointer
	 r1 = memory address
	 r2 = byte count
	 return (r0) = byte count written
*/
ringbuffer_write:
	ldr r3, [r0, #0] /* r3: available */
	push {r4-r9}
	
	ldr r4, [r0, #4] /* r4: buffer size */
	ldr r5, [r0, #8] /* r5: write head */
	mov r6, #0 /* will hold our written count */
	
	/* bounds check */
	cmp r2, r3
	movhi r2, r3	/* if the byte count was higher than the available size in the buffer, change the byte count */

1:		
	sub r12, r4, r5 /* number of bytes until end of the buffer */
	
	cmp r2, r12 /* can we fit the entire write? */
	movhi r12, r2 
	
	/* copy memory */
	mov r7, r12 /* r7: bytes left to do */
	add r8, r0, r5 /* r8: destination buffer */
	add r8, r0, #16
2:
	cmp r7, #0
	beq 3f
	ldrh r9, [r1], #2
	strh r9, [r8], #2
	b 2b
	/* TODO: this could be optimized HEAVILY */
3:
	add r6, r6, r12 /* update the return value */	
	add r5, r5, r12 /* update the write head */
	cmp r4, r5 /* check for buffer wraparound */
	moveq r5, #0
	
	subs r2, r2, r12 /* subtract out how much we wrote */
	bne 1b /* do the second half of the write if we need to */

	str r5, [r0, #8] /* store the new write head */
	dmb /* let any pending stores from the memcpy finish */
4:
	ldrex r3, [r0] /* re-load available */
	sub r3, r3, r6 /* subtract out how much we wrote */
	strex r1, r3, [r0] /* store the new available count */
	cmp r1, #0
	bne 4b

	mov r0, r6 /* set the return value */
	pop {r4-r9}
	bx lr	

/* r0 = ringbuffer pointer
	 r1 = memory address
	 r2 = byte count
	 return (r0) = byte count read
*/
ringbuffer_read:
	ldr r3, [r0, #0] /* r3: available
	push {r4-r9}
	
	ldr r4, [r0, #4] /* r4: buffer size */
	ldr r5, [r0, #12] /* r5: read head */
	mov r6, #0 /* will hold our read count */
	sub r3, r4, r3 /* since available is how many bytes can be written, size-available is how many bytes can be read */
	
	/* bounds check */
	cmp r2, r3
	movhi r2, r3	/* if the byte count was higher than the available size in the buffer, change the byte count */
	
1:		
	sub r12, r4, r5 /* number of bytes until end of the buffer */
	
	cmp r2, r12 /* can we fit the entire read? */
	movhi r12, r2 

/* copy memory */
	mov r7, r12 /* r7: bytes left to do */
	add r8, r0, r5 /* r8: source buffer */
	add r8, r0, #16
2:
	cmp r7, #0
	beq 3f
	ldrh r9, [r8], #2
	strh r9, [r1], #2
	b 2b
	/* TODO: this could be optimized HEAVILY */
3:

	add r6, r6, r12 /* update the return value */	
	add r3, r3, r12 /* mark that there's more available */
	add r5, r5, r12 /* update the read head */
	cmp r4, r5 /* check for buffer wraparound */
	moveq r5, #0

	subs r2, r2, r12 /* subtract out how much we read */
	bne 1b /* do the second half of the read if we need to */
	
	str r5, [r0, #12] /* store the new read head */
	dmb /* let any pending reads from the memcpy finish */
4:
	ldrex r3, [r0] /* re-load available */
	add r3, r3, r6 /* subtract out how much we wrote */
	strex r1, r3, [r0] /* store the new available count */
	cmp r1, #0
	bne 4b

	mov r0, r6 /* set the return value */
	pop {r4-r9}
	bx lr	
