/*
  Apple Mac OS X AMD64 implementation
*/

#pragma once
#include <libkern/OSAtomic.h>

/* Mac OS X x86 implementation */


#ifdef __cplusplus
#define NX_ATOMIC_INLINE inline
#else
#define NX_ATOMIC_INLINE
#endif

NX_ATOMIC_INLINE static void nx_atomic_write(size_t value, volatile size_t *addr)
{
    *addr = value; // simple on amd64
}

NX_ATOMIC_INLINE static void nx_atomic_write_pointer(void *value, void* volatile *addr)
{
    *addr = value; // simple on amd64
}

NX_ATOMIC_INLINE static void nx_atomic_write64(uint64_t value, volatile int64_t *addr)
{
    *addr = value; // simple on amd64
}

NX_ATOMIC_INLINE static size_t nx_atomic_inc(volatile size_t *addr)
{
    return __sync_add_and_fetch(addr, 1);
}

NX_ATOMIC_INLINE static size_t nx_atomic_dec(volatile size_t *addr)
{
    return __sync_sub_and_fetch(addr, 1);
}

NX_ATOMIC_INLINE static size_t nx_atomic_dec_release(volatile size_t *addr)
{
	return __sync_sub_and_fetch(addr, 1);
}

NX_ATOMIC_INLINE static void nx_atomic_add(size_t value, volatile size_t* addr)
{
    __sync_add_and_fetch(addr, value);
}

NX_ATOMIC_INLINE static void nx_atomic_sub(size_t value, volatile size_t* addr)
{
    __sync_sub_and_fetch(addr, value);
}

NX_ATOMIC_INLINE static void *nx_atomic_swap_pointer(void *value, void* volatile *addr)
{
    asm volatile("lock;"
                 "xchgq %0, %1"
                 : "=r"(value), "=m"(*addr)
                 : "0"(value), "m"(*addr)
                 : "memory");
    return value;
}

NX_ATOMIC_INLINE static int nx_atomic_cmpxchg_pointer(const void *oldvalue, const void *newvalue, void* volatile *addr)
{
    const void *prev;
    asm volatile("lock;"
                 "cmpxchgq %1, %2;"
                 : "=a"(prev)
                 : "r"(newvalue), "m"(*addr), "a"(oldvalue)
                 : "memory");
    return prev!=oldvalue;
}

NX_ATOMIC_INLINE static int nx_atomic_cmpxchg2(int64_t oldvalue, int64_t newvalue, volatile int64_t *addr)
{
	return OSAtomicCompareAndSwap64(oldvalue, newvalue, addr) == oldvalue;
}