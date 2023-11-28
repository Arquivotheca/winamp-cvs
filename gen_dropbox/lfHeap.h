#ifndef NULLSOFT_DROPBOX_PLUGIN_LOWFRAGMENTATIONHEAP_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_LOWFRAGMENTATIONHEAP_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

BOOL lfh_init();
void lfh_shutdown();
void lfh_compact();

void *lfh_malloc(size_t size);
void lfh_free(void *mem);
void *lfh_realloc(void* mem, size_t newSize);

LPWSTR lfh_strdupW(LPCWSTR orig);
LPSTR lfh_strdupA(LPCSTR orig);

#ifdef UNICODE
#define lfh_strdup	lfh_strdupW
#else
#define lfh_strdup	lfh_strdupA
#endif // !UNICODE

#endif //NULLSOFT_DROPBOX_PLUGIN_LOWFRAGMENTATIONHEAP_HEADER