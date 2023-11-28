.text
.align
    
.global lifo_push
.type lifo_push, %function

.global lifo_pop
.type lifo_pop, %function


/*
r0=lifo
r1=new value
*/    
lifo_push:
1:
   ldr     r2, [r0]     /* r2 holds old head (maybe) */
   str     r2, [r1]     /* store old head pointer as new value's "next" */
	 dmb                  /* let the store complete */
   ldrex   r3, [r0]     /* exclusively load head pointer from lifo */
   teq     r2, r3       /* make sure head didn't change out from under us */
   strexeq r3, r1, [r0] /* try to store new value as the head */
   teqeq   r3, #0
   bne     1b           /* loop if we didn't succeeed */
   bx      lr

/*
r0 = lifo
return r0 = popped value
*/
lifo_pop:
   push    {r4}
   mov     r12, r0       /* easier to have this here */
1: 
   ldrd    r0, r1, [r12] /* load the lifo-next pointer (r0) and counter (r1) */
   teq     r0, #0
   beq     2f            /* bail out if next is null */
   ldr     r4, [r0]      /* read head->next */
   ldrexd  r2, r3, [r12] /* load again, but locked  */
   teq     r1, r3        /* see if the counter changed */
   teqeq   r0, r2        /* see if the next pointer changed */
   bne     1b
   add     r3, r3, #1 /* add 1 to the aba count */
   mov     r2, r4 /* because strexd wants consecutive registers */
   strexd  r1, r2, r3, [r12] /* try to store updated next and aba counter */
   teq     r1, #0
   bne     1b /* loop if we didn't succeeed */
2:
   pop   {r4}
   bx    lr
