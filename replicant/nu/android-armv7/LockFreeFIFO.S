.text
.align

.global fifo_init
.type fifo_init, %function    
.global fifo_push
.type fifo_push, %function
.global fifo_pop
.type fifo_pop, %function

/*
r0=lifo
*/    
fifo_init:
	mov r1, #0
	str r1, [r0]
	str r1, [r0, #4]
	bx lr

/*
r0 = lifo
r1 = new value
*/
fifo_push:
	push {r4}
	mov r2, #0
	str r2, [r1] /* set newvalue->next = 0 */
1:
	ldrexd r2, r3, [r0] /* load head and tail */
	cmp r2, #0 /* if head is empty */
	moveq r2, r1
	beq 2f
	
	ldrex r12, [r3] /* load and lock tail */
	cmp r12, #0 /* make sure tail->next is empty */
	beq 1b /* tail->next isn't empty? try everything again */
	strex r4, r1, [r3] /* tail->next = new_value */
	cmp r4, #1
	beq 1b /* loop again if we can't set tail->next */

2:
	/* mov {r2-r1} to {r2-r3}
	mov r3, r1

	strexd r4, r2, r3, [r0] /* store new head and tail */
	cmp r4, #1
	beq 1b /* loop again if we can't set the head and the tail */

	pop {r4}
	bx lr

/*
r0 = lifo
return r0 = popped value
*/
fifo_pop:
	push {r4}
	mov r4, r0 /* this will make things easier to return data in r0 */
1:
	ldrexd r0, r1, [r4] /* load head and tail */
	cmp r0, #0
	popeq {r4}
	bxeq lr /* if no head, go ahead and return */

	cmp r0, r1 /* if head == tail, then the list only has one item */
	ldr r2, [r0]
	moveq r1, r2

	/* mov {r2-r1} to {r2-r3}
	mov r3, r1

	strexd r1, r2, r3, [r4] /* try to store updated tail and head */
	cmp r1, #1
	beq 1b /* loop if we failed to update */
	pop {r4}
	bx lr /* return if we successfully updated */
	b 1b
	
	

	