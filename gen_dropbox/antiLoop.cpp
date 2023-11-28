#include "./main.h"
#include "./antiLoop.h"
#include "./fileInfoInterface.h"

#define ACCUMULATOR_SIZE		8
#define CSTRCMPI(__str1, __str2) (CompareString(CSTR_INVARIANT, NORM_IGNORECASE, (__str1), -1, (__str2), -1))


static int __cdecl AntiLoop_SearchCompare(const void* key, const void* elem)
{
	return CSTRCMPI((LPCTSTR)key, *((LPTSTR*)elem)) - 2;
}

static int __cdecl AntiLoop_SortCompare(const void* elem1, const void* elem2)
{	
	return CSTRCMPI(*((LPTSTR*)elem1), *((LPTSTR*)elem2)) - 2;
}

AntiLoop::AntiLoop()
{
	sorted.reserve(4096);
}

AntiLoop::~AntiLoop()
{
	size_t index;
	index = sorted.size();
	while(index--)
		lfh_free(sorted[index]);

	index = accumulator.size();
	while(index--)
		lfh_free(accumulator[index]);
}

BOOL AntiLoop::Check(IFileInfo *pInfo)
{
	LPCTSTR pszPath;
	if (FAILED(pInfo->GetPath(&pszPath)))
		return TRUE;
	return Check(pszPath);
}

BOOL AntiLoop::Check(LPCTSTR pszPath)
{
	if (NULL == pszPath || TEXT('\0') == pszPath)
		return TRUE;
	size_t index = accumulator.size();
	while(index--)
	{
		if (CSTR_EQUAL == CSTRCMPI(accumulator[index], pszPath))
			return FALSE;
	}
	LPTSTR p = (LPTSTR)bsearch(pszPath, sorted.begin(), sorted.size(), sizeof(LPTSTR), AntiLoop_SearchCompare);
	if (NULL != p) return FALSE;
	
	
	INT cbLen = (lstrlen(pszPath) + 1) * sizeof(TCHAR);
	LPTSTR copy = (LPTSTR)lfh_malloc(cbLen);
	if (NULL == copy)
		return TRUE;
	CopyMemory(copy, pszPath, cbLen);
	if (accumulator.size() == ACCUMULATOR_SIZE)
	{
		sorted.insertBefore(sorted.size(), accumulator.begin(), accumulator.size());
		qsort(sorted.begin(), sorted.size(), sizeof(LPTSTR), AntiLoop_SortCompare);
		accumulator.clear();
	}
	accumulator.push_back(copy);
	return TRUE;
}
