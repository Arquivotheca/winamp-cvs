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

 * "Acquire" implies a memory barrier *after* the operation
 * "Release" implies a memory barrier *before* the operation
 */

/*inline static */void nx_atomic_write(size_t value, volatile size_t *addr);// {	*addr = value; }
inline static void nx_atomic_write_pointer(void *value, void* volatile *addr) { nx_atomic_write((size_t)value, (volatile size_t *)addr); }

size_t nx_atomic_inc(volatile size_t *addr);
size_t nx_atomic_dec(volatile size_t *addr);

size_t nx_atomic_inc_acquire(volatile size_t *addr);
size_t nx_atomic_dec_release(volatile size_t *addr);

	void nx_atomic_add64(uint64_t value, volatile uint64_t *addr);
	void nx_atomic_sub64(uint64_t value, volatile uint64_t *addr);
	
	uint64_t nx_atomic_read64(volatile uint64_t *addr);

void nx_atomic_add(size_t value, volatile size_t* addr);
void nx_atomic_sub(size_t value, volatile size_t* addr);

size_t nx_atomic_and(size_t value, volatile size_t* addr);
size_t nx_atomic_or(size_t value, volatile size_t* addr);

size_t nx_atomic_swap(size_t value, volatile size_t* addr);
inline static void *nx_atomic_swap_pointer(void *value, void* volatile *addr)
{
	return (void *)nx_atomic_swap((size_t)value, (volatile size_t *)addr);
}

/*
 * cmpxchg return a non zero value if the exchange was NOT performed,
 * in other words if oldvalue != *addr
 */

int nx_atomic_cmpxchg(size_t oldvalue, size_t newvalue, volatile size_t *addr);
inline static int nx_atomic_cmpxchg_pointer(const void *oldvalue, const void *newvalue, void* volatile *addr)
{
	return nx_atomic_cmpxchg((size_t)oldvalue, (size_t)newvalue, (volatile size_t *)addr);
}

int nx_atomic_cmpxchg2(int64_t old, int64_t newvalue, volatile int64_t *addr);

#ifdef __cplusplus
}
#endif
