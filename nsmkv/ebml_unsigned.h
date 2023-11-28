#pragma once

#include <bfc/platform/types.h>

/* len is passed as uint64_t, but better be 8 or less! */
uint64_t unsigned_read_ptr_len(uint64_t len, const uint8_t *ptr);