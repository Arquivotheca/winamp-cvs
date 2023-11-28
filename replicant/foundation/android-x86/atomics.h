#pragma once
#include "foundation/types.h"
inline static void nx_atomic_write(size_t value, volatile size_t *addr) {	*addr = value; }
inline static void nx_atomic_write_pointer(void *value, void* volatile *addr) { nx_atomic_write((size_t)value, (volatile size_t *)addr); }


inline static size_t nx_atomic_inc(volatile size_t *addr)
{
	return __sync_add_and_fetch(addr, 1);
}
inline static size_t nx_atomic_dec(volatile size_t *addr)
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

inline static size_t nx_atomic_swap(size_t value, volatile size_t* addr)
{
	return __sync_lock_test_and_set(addr, value);
}

inline static void *nx_atomic_swap_pointer(const void *value, void* volatile *addr)
{
	return (void *)nx_atomic_swap((size_t)value, (volatile size_t *)addr);
}

inline static int nx_atomic_cmpxchg(size_t oldvalue, size_t newvalue, volatile size_t *addr)
{
	return __sync_bool_compare_and_swap(addr, oldvalue, newvalue);
}

inline static int nx_atomic_cmpxchg_pointer(const void *oldvalue, const void *newvalue, void* volatile *addr)
{
	return __sync_bool_compare_and_swap(addr, oldvalue, newvalue);
}
