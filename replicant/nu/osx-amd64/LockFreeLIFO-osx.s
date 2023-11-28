//
//  LockFreeLIFO-osx-amd64.s
//  nu
//
//  Created by Ben Allison on 1/4/12.
//  Copyright (c) 2012 Nullsoft, Inc. All rights reserved.
//
.globl _lifo_push, _lifo_pop
.text

// rdi = lifo
// rsi = new entry
_lifo_push:
    movq (%rdi), %rax // rax holds the old head
    movq %rax, (%rsi) // new node's 'next' is set to the old head
    lock cmpxchgq %rsi, (%rdi) // swap new entry into the head
    ret

// rdi = lifo
_lifo_pop:
    push %rbx
again:
    // on AMD64, re-ordered loads aren't an issue.
    // So we run the risk of getting an OLD counter and a NEW pointer, but will be OK because the old counter will cause the cmpxchg16b to fail
    movq 8(%rdi), %rdx  // rdx holds the counter
    movq (%rdi), %rax // pointer into rax
    testq %rax, %rax
    jz bail

    movq %rdx, %rcx // rdx/rcx = counter
    movq (%rax), %rbx // rbx = next
    incq %rcx

// Compare RDX:RAX register to 128-bit memory location. If equal, set the zero flag (ZF) to 1 and copy the RCX:RBX register to the memory location. Otherwise, copy the memory location to RDX:RAX and clear the zero flag.
    lock cmpxchg16b (%rdi)
    jnz again

bail:
    pop %rbx
    ret
