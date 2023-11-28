.text
.align

/*
r0=pointer to once structure
r1=function pointer
r2=parameter
*/    
.global NXOnce
.type NXOnce, %function
NXOnce:
	ldr r3, [r0] /* load value */
	cmp r3, #0
	bxne lr /* bail out if it's non-zero */

	/* save the parameters because _pthread_mutex_lock will eat them */
	push	{r4, r5, r6, lr}	
	mov r4, r0
	mov r5, r1
	mov r6, r2

	add r0, r0, #4
	blx pthread_mutex_lock /* lock the mutex */

	ldr r3, [r4] /* re-load value */
	cmp r3, #0
	bne bail
 
	mov r0, r4 
	mov r1, r6
	mov r2, #0
	blx r5 /* call the user function */

	mov r0, #1 
	str r0, [r4] /* mark that we've done the initialization */
		
 bail:
	add r0, r4, #4
	blx pthread_mutex_unlock

	pop {r4, r5, r6, lr}
	bx lr

