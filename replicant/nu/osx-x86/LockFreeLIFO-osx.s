//
//  LockFreeLIFO-osx-amd64.s
//  nu
//
//  Created by Ben Allison on 1/4/12.
//  Copyright (c) 2012 Nullsoft, Inc. All rights reserved.
//
.globl _lifo_push, _lifo_pop
.text

_lifo_push:
    mov 4(%esp), %ecx// ecx holds lifo
    mov 8(%esp), %edx // edx holds the new entry
again_push:
    mov (%ecx), %eax // %eax holds the old head
    mov %eax, (%edx) // new node's 'next' is set to the old head
    lock cmpxchg %edx, (%ecx)
    jnz again_push
    ret

_lifo_pop:
    push %esi
    push %ebx
    mov 12(%esp), %esi // esi holds lifo
again_pop:
    mov 4(%esi), %edx // counter 
    mov (%esi), %eax // pointer
    test %eax, %eax
    jz bail

    mov %edx, %ecx // counter
    mov (%eax), %ebx // pointer->next
    inc %ecx

    lock cmpxchg8b (%esi)
    jnz again_pop

bail:
    pop %ebx
    pop %esi
    ret
