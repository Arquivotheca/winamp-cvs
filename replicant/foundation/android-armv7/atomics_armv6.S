.text
.align
    
    .global nx_atomic_write
    .type nx_atomic_write, %function
    
    .global nx_atomic_inc
    .type nx_atomic_inc, %function
    .global nx_atomic_dec
    .type nx_atomic_dec, %function

    .global nx_atomic_inc_acquire
    .type nx_atomic_inc_acquire, %function
    .global nx_atomic_dec_release
    .type nx_atomic_dec_release, %function

    .global nx_atomic_add64
    .type nx_atomic_add64, %function

    .global nx_atomic_sub64
    .type nx_atomic_sub64, %function
		
	.global nx_atomic_read64
    .type nx_atomic_read64, %function
		
    .global nx_atomic_add
    .type nx_atomic_add, %function
	.global nx_atomic_sub
    .type nx_atomic_sub, %function
    .global nx_atomic_and
    .type nx_atomic_and, %function
    .global nx_atomic_or
    .type nx_atomic_or, %function
    
    .global nx_atomic_swap
    .type nx_atomic_swap, %function
    
    .global nx_atomic_cmpxchg
    .type nx_atomic_cmpxchg, %function
    
	.global nx_atomic_cmpxchg2
    .type nx_atomic_cmpxchg2, %function
	

/* FIXME: On SMP systems memory barriers may be needed */
#warning  "this file is not safe with SMP systems"


/*
 * ----------------------------------------------------------------------------
 * nx_atomic_write
 * input: r0=value, r1=address
 * output: void
 */

nx_atomic_write:
	/*dmb*/
    str     r0, [r1]
    bx      lr;

/*
 * ----------------------------------------------------------------------------
 * nx_atomic_inc
 * input: r0 = address
 * output: r0 = new value
 */
 
nx_atomic_inc:
    mov     r12, r0
1:  ldrex   r2, [r12]
    add     r0, r2, #1
    strex   r1, r0, [r12]
    cmp     r1, #0
    bxeq    lr
    b       1b

/*
 * ----------------------------------------------------------------------------
 * nx_atomic_dec
 * input: r0=address
 * output: r0 = new value
 */
 
nx_atomic_dec:
    mov     r12, r0
1:  ldrex   r2, [r12]
    sub     r0, r2, #1
    strex   r1, r0, [r12]
    cmp     r1, #0
    bxeq    lr
    b       1b

/*
 * ----------------------------------------------------------------------------
 * nx_atomic_inc_acquire
 * input: r0 = address
 * output: r0 = new value
 */
 
nx_atomic_inc_acquire:
    mov     r12, r0
1:  ldrex   r2, [r12]
    add     r0, r2, #1
    strex   r1, r0, [r12]
    cmp     r1, #0
    bxeq    lr
	dmb     st
    b       1b

/*
 * ----------------------------------------------------------------------------
 * nx_atomic_dec_release
 * input: r0=address
 * output: r0 = new value
 */
 
nx_atomic_dec_release:
	dmb
    mov     r12, r0
1:  ldrex   r2, [r12]
    sub     r0, r2, #1
    strex   r1, r0, [r12]
    cmp     r1, #0
    bxeq    lr
    b       1b

    
/*
 * ----------------------------------------------------------------------------
 * nx_atomic_add
 * input: r0=value, r1=address
 */

nx_atomic_add:
1:
	ldrex   r2, [r1]
	add     r2, r2, r0
	strex   r3, r2, [r1]
	cmp     r3, #0
	bxeq    lr
	b       1b
  
/*
 * ----------------------------------------------------------------------------
 * nx_atomic_sub
 * input: r0=value, r1=address
 */

nx_atomic_sub:
1:  
	ldrex   r2, [r1]
	sub     r2, r2, r0
	strex   r3, r2, [r1]
	cmp     r3, #0
	bxeq    lr
	b       1b
    
		
		/*
 * ----------------------------------------------------------------------------
 * nx_atomic_add64
 * input: r0:r1=value, r2=address
 */

nx_atomic_add64:
	push {r4-r5}
	mov r12, r2
1: 
	ldrexd  r2, r3, [r12]
	adds    r2, r2, r0
	adc     r3, r3, r1
	strexd  r4, r2, r3, [r12]
	cmp     r4, #0
	bne 1b
	pop {r4-r5}
	bx lr
	
/*
 * ----------------------------------------------------------------------------
 * nx_atomic_sub64
 * input: r0=value, r1=address
 */

nx_atomic_sub64:
	push {r4-r5}
	mov r12, r2
1: 
	ldrexd  r2, r3, [r12]
	subs    r2, r2, r0
	sbc     r3, r3, r1
	strexd  r4, r2, r3, [r12]
	cmp     r4, #0
	bne 1b
	pop {r4-r5}
	bx lr
  
/*
 * ----------------------------------------------------------------------------
 * nx_atomic_read64
 * input: r0=address
 * output r0:r1=value
 */	
nx_atomic_read64:
/* on some ARM chips, ldrd is atomic on its own, but we can't really guarantee this */
	ldrexd r0, r1, [r0]
	clrex
	bx lr
/*
 * ----------------------------------------------------------------------------
 * nx_atomic_and
 * input: r0=value, r1=address
 * output: r0 = old value
 */

nx_atomic_and:
    mov     r12, r0
1:  ldrex   r0, [r1]
    and     r2, r0, r12
    strex   r3, r2, [r1]
    cmp     r3, #0
    bxeq    lr
    b       1b

    
/*
 * ----------------------------------------------------------------------------
 * nx_atomic_or
 * input: r0=value, r1=address
 * output: r0 = old value
 */

nx_atomic_or:
    mov     r12, r0
1:  ldrex   r0, [r1]
    orr     r2, r0, r12
    strex   r3, r2, [r1]
    cmp     r3, #0
    bxeq    lr
    b       1b

/*
 * ----------------------------------------------------------------------------
 * nx_atomic_swap
 * input: r0=value, r1=address
 * output: r0 = old value
 */

nx_atomic_swap:
	mov   r2, r0
1:	ldrex r0, [r1]
	strex r3, r2, [r1]
	cmp   r3, #0
	bxeq  lr
	b     1b

/*
 * ----------------------------------------------------------------------------
 * nx_atomic_cmpxchg
 * input: r0=oldvalue, r1=newvalue, r2=address
 */

nx_atomic_cmpxchg:
1:  ldrex   r3, [r2]
    teq     r0, r3
	movne   r0, #0
	bxne    lr
    strex   r0, r1, [r2]
    cmp     r0, #0
	bne     1b
	mov     r0, #1
	bx      lr

/*
 * ----------------------------------------------------------------------------
 * nx_atomic_cmpxchg2
 * input: r0-r1=oldvalue, r2-r3=newvalue, arg4 (on stack)=address

 */
nx_atomic_cmpxchg2:

	ldr      r12, [sp]
    push     { r4, r5, r6, r7 }
1:  ldrexd   r4, r5, [r12] 
    teq      r0, r4  ;
    teqeq    r1, r5
	movne    r0, #0
	bne     2f
    strexd   r7, r2, r3, [r12]
	cmp      r7, #0
	bne      1b
	mov      r0, #1
	2:
	pop      { r4, r5, r6, r7 }
	bx       lr
