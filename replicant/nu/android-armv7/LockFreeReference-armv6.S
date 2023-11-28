.text
.align
    
.global lockfree_reference_init
.type lockfree_reference_init, %function

.global lockfree_reference_retain
.type lockfree_reference_retain, %function

.global lockfree_reference_release
.type lockfree_reference_release, %function

/* data structure
[0-3] ptr
[4-7] count
*/

/* r0 = reference pointer 
   r1 = pointer to initialize with
   no return value */
lockfree_reference_init:
	str r1, [r0]
	mov r2, #0
	str r2, [r0, #4]
	bx lr

/* r0 = reference pointer
   r1 = output pointer
*/
lockfree_reference_retain:
1:
	ldrexd r2, r3, [r0] /* r2: ptr r3: count */
	cmp r2, #0
	streq r2, [r1]
	bxeq lr
	add r3, r3, #1
	strexd r12, r2, r3, [r0]
	cmp r12, #0
	bne 1b
	str r2, [r1]
	bx lr

lockfree_reference_release:
1:
	ldrexd r2, r3, [r0]
	subs r3, r3, #1
	moveq r2, #0 /* zero the pointer if we're the last to retrieve it */
	strexd r12, r2, r3, [r0]
	cmp r12, #0
	bne 1b
	mov r0, r3
	bx lr
