/*

   Android (ARM) implementation
	 
*/
#include "foundation/types.h"

#ifdef __cplusplus
extern "C" {
#endif
/*
 * NOTE: memory shared between threads is synchronized by all atomic operations
 * below, this means that no explicit memory barrier is required: all reads or 
 * writes issued before android_atomic_* operations are guaranteed to complete
 * before the atomic operation takes place.
 */

/*inline static */void nx_atomic_write(size_t value, volatile size_t *addr);// {	*addr = value; }
inline static void nx_atomic_write_pointer(void *value, void* volatile *addr) { nx_atomic_write((size_t)value, (volatile size_t *)addr); }

/*
 * all these atomic operations return the previous value
 */


inline static size_t nx_atomic_inc(volatile size_t *addr)
{
	return __sync_add_and_fetch(addr, 1);
}

inline static size_t nx_atomic_dec(volatile size_t *addr)
{
	return __sync_sub_and_fetch(addr, 1);
}

// Acquire implies a memory barrier *after* the operation
inline static size_t nx_atomic_inc_acquire(volatile size_t *addr)
{
	return __sync_add_and_fetch(addr, 1);
}

// Release implies a memory barrier *before* the operation
inline static size_t nx_atomic_dec_release(volatile size_t *addr)
{
	return __sync_sub_and_fetch(addr, 1);
}


inline static size_t nx_atomic_add(size_t value, volatile size_t* addr)
{
	return __sync_add_and_fetch(addr, value);
}

inline static size_t nx_atomic_sub(size_t value, volatile size_t* addr)
{
	return __sync_sub_and_fetch(addr, value);
}

size_t nx_atomic_and(size_t value, volatile size_t* addr);
size_t nx_atomic_or(size_t value, volatile size_t* addr);

size_t nx_atomic_swap(size_t value, volatile size_t* addr);
inline static void *nx_atomic_swap_pointer(const void *value, void* volatile *addr)
{
	return (void *)nx_atomic_swap((size_t)value, (volatile size_t *)addr);
}

/*
 * NOTE: Two "quasiatomic" operations on the exact same memory address
 * are guaranteed to operate atomically with respect to each other,
 * but no guarantees are made about quasiatomic operations mixed with
 * non-quasiatomic operations on the same address, nor about
 * quasiatomic operations that are performed on partially-overlapping
 * memory.
 */

int64_t nx_atomic_swap2(int64_t value, volatile int64_t* addr);
int64_t nx_atomic_read2(volatile int64_t* addr);
    
/*
 * cmpxchg return a non zero value if the exchange was performed,
 * in other words if oldvalue == *addr
 */

inline static int nx_atomic_cmpxchg(size_t oldvalue, size_t newvalue, volatile size_t *addr)
{
	return __sync_bool_compare_and_swap(addr, oldvalue, newvalue);
}

inline static int nx_atomic_cmpxchg_pointer(const void *oldvalue, const void *newvalue, void* volatile *addr)
{
	return __sync_bool_compare_and_swap(addr, oldvalue, newvalue);
}

#ifdef __cplusplus
}
#endif
