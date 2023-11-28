.text
.align
    
.global mpscq_init
.type mpscq_init, %function
.global mpscq_push
.type mpscq_push, %function
.global mpscq_pop
.type mpscq_pop, %function

/* mpscq data structure
[0-3] head
[4-7] tail
[8-11] stub (dummy entry)
*/

/* r0 = queue pointer 
   no return value */
mpscq_init:
	add	r1, r0, #8 /* r1 = this->stub */
	str	r1, [r0] /* this->head = &this->stub */
	str	r1, [r0, #4] /* this->tail = &this->stub */
	mov	r2, #0 
	str	r2, [r1] /* this->stub.next = 0 */
	bx	lr


/* r0 = queue pointer
   r1 = new_entry 
   returns zero if this push was onto an empty list */
mpscq_push:
	/* terminate new entry before enqueuing it. that let's the pop function detect a pre-empted push */
	mov r2, #0
	str r2, [r1] /* new_entry->next = 0 */
	dmb st /* on SMP we need a memory barrier here */
1:
	/* swap in the new entry as the new head */
	ldrex r2, [r0] /* r2=self->head */
	strex r3, r1, [r0] /* self->head = new_entry */
	cmp r3, #0 /* see if the store-conditional succeeded */
	bne 1b /* try again if we got pre-empted */
	/* [1] if a thread gets pre-empted here, it can cause some weirdness on popping. we deal with this scenario during pop */
	dmb st /* on SMP we *might* need a memory barrier here */
	str r1, [r2], #-8 /* prev->next = new_entry */
	sub r0, r2, r0 /* because of the -8 above, and this sub, r0 will be zero if prev == &stub */
	bx lr


/* r0 = queue pointer
   return r0 = tail entry */
mpscq_pop:
	add r1, r0, #8 /* r1: &this->stub */
	ldr r2, [r0, #4] /* r2: tail=self->tail */

	cmp r2, r1 /* if (tail == &this->stub) - skip the stub if we come across it */
	ldr r3, [r2] /* r3: next=tail->next */
	bne 1f /* skip this block if they're not equal */
	cmp r3, #0 /* check if the list is empty (next == 0) */
	moveq r0, r3 /* if so, return 0 */
	bxeq lr /* and exit */

	str r3, [r0, #4] /* self->tail = next */
	mov r2, r3 /* update local "tail" variable to next */
	ldr r3, [r3] /* update local "next" variable to next->next */

1:
	cmp r3, #0 /* if (next) */
	strne r3, [r0, #4] /* self->tail = next */
	movne r0, r2 /* return tail */
	bxne lr /* exit function */

	/* if we get here, then we have one item in the list - and it's not the stub. 
	we'll push the stub onto the stack so we can safely pop the tail */
	mov r12, #0
	str r12, [r1] /* stub->next = 0 */
	dmb st /* on SMP we need a memory barrier here to prevent a race condition with mpscq_push on another thread */
2:
	ldrex r12, [r0] /* r12: head=self->head */
	cmp r12, r2 /* compare tail and head. if they're different, someone snuck a push in but hasn't finished - they got blocked in [1] */
	movne r0, #1 /* return 1 to signal 'busy' */
	bxne lr /* exit function */
	strex r3, r1, [r0] /* self->head = stub */
	cmp r3, #0
	bne 2b
	
	str r1, [r12] /* prev->next = stub */
	str r1, [r0, #4] /* self->tail = stub */
	mov r0, r2 /* return the (old) tail */
	bx lr
