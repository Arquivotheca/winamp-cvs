#pragma once
#include <bfc/platform/types.h>
/* updates str, data_len and str_cch */
int ParseDescription(const char *&str, size_t &data_len, size_t &str_cch);
int ParseDescription(const wchar_t *&str, size_t &data_len, size_t &str_cch, uint8_t &str_encoding);