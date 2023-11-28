#pragma once
#include "foundation/types.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef NALIGN(8) uint8_t lockfree_reference_t[8];

void lockfree_reference_init(lockfree_reference_t ref, void *ptr);
void lockfree_reference_retain(lockfree_reference_t ref, void **ptr);
// returns zero if you have to free it
int lockfree_reference_release(lockfree_reference_t ref);
#ifdef __cplusplus
}
#endif