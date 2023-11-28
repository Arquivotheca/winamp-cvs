.text
.align
    
.global nx_atomic_write
.type nx_atomic_write, %function
    
    .global nx_atomic_inc
    .type nx_atomic_inc, %function
    .global nx_atomic_dec
    .type nx_atomic_dec, %function
    
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
	
/*
 * ----------------------------------------------------------------------------
 * nx_atomic_write
 * input: r0=value, r1=address
 * output: void
 */

nx_atomic_write:
    str     r0, [r1]
    bx      lr;

/*
 * ----------------------------------------------------------------------------
 * nx_atomic_swap
 * input: r0=value, r1=address
 * output: r0 = old value
 */

nx_atomic_swap:
	swp r0, r0, [r1]
	bx  lr
