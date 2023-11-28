#include "./lfHeap.h"
#include <winbase.h>

static HANDLE lfHeap = NULL;

BOOL lfh_init()
{
	if (NULL != lfHeap) 
		return TRUE;

	lfHeap = HeapCreate(0, 0, 0);
	
	if (NULL == lfHeap)
		return FALSE;
	
	ULONG  HeapFragValue = 2;

	if (!HeapSetInformation(lfHeap, HeapCompatibilityInformation, 
		&HeapFragValue, sizeof(HeapFragValue)))
	{
	//	MessageBox(NULL, TEXT("d'oh"), TEXT("no lfh support"), MB_OK);
	}

	return TRUE;
}

void lfh_shutdown()
{
	if (NULL != lfHeap)
	{
		HeapDestroy(lfHeap);
		lfHeap = NULL;
	}
}

void lfh_compact()
{
	HeapCompact(lfHeap, 0);
}

void *lfh_malloc(size_t size)
{
	return HeapAlloc(lfHeap, 0, size);
}

void lfh_free(void *mem)
{
	HeapFree(lfHeap, 0, mem);
}

void *lfh_realloc(void* mem, size_t newSize)
{
	return HeapReAlloc(lfHeap, 0, mem, newSize);
}

LPWSTR lfh_strdupW(LPCWSTR orig)
{
	if (NULL == orig)
		return NULL;
	INT cbLen = (lstrlenW(orig) + 1) * sizeof(WCHAR);
	LPWSTR dst = (LPTSTR)lfh_malloc(cbLen);
	if (NULL != dst)
		CopyMemory(dst, orig, cbLen);
	return dst;
}

LPSTR lfh_strdupA(LPCSTR orig)
{
	if (NULL == orig)
		return NULL;
	INT cbLen = (lstrlenA(orig) + 1) * sizeof(CHAR);
	LPSTR dst = (LPSTR)lfh_malloc(cbLen);
	if (NULL != dst)
		CopyMemory(dst, orig, cbLen);
	return dst;
}
