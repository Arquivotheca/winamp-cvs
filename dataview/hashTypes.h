#ifndef _NULLSOFT_WINAMP_DATAVIEW_HASH_TYPES_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_HASH_TYPES_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./klib/khash.h"

#ifdef  _WIN64
 KHASH_MAP_INIT_INT64(sizet_map, size_t);
 KHASH_SET_INIT_INT64(sizet_set);
#else
 KHASH_MAP_INIT_INT(sizet_map, size_t);
 KHASH_SET_INIT_INT(sizet_set);
#endif


#endif //_NULLSOFT_WINAMP_DATAVIEW_HASH_TYPES_HEADER