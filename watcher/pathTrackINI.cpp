#include ".\pathTrackINI.h"
#include <strsafe.h>

PathTrackINI::PathTrackINI(void)
{
	record.Allocate(1024);
}

PathTrackINI::~PathTrackINI(void)
{
	DeleteFile(TRUE);
}

HRESULT PathTrackINI::SetTracker(const wchar_t *file, const wchar_t *watcherID)
{
	SetWrokingINI(file);
	SetWorkingSection(watcherID);
	if (!IsFileExist()) CreateUnicode(TRUE);
	return S_OK;
}

HRESULT PathTrackINI::WritePathInfo(PATHINFO *pathInfo)
{	
	wchar_t hash[9];
	if (S_OK != StringCchPrintfW(hash, 9, L"%08I32X", pathInfo->hPath)) return S_FALSE;				
	SetValue(hash, pathInfo->path.GetBuffer());
	return S_OK;
}

HRESULT PathTrackINI::GetPathInfo(PATHINFO *pathInfo)
{	
	wchar_t hash[9];
	StringCchPrintfW(hash, 9, L"%08I32X", pathInfo->hPath);				
	GetStringValue(hash, &pathInfo->path, L"");
	return S_OK;
}

unsigned __int32* PathTrackINI::GetPathHashList(unsigned int *size)
{
	MLString buffer(8192);
	GetKeys(&buffer);
	if (buffer.GetLength() == 0) return NULL;
	wchar_t *keys = buffer.GetBuffer();
	*size = 0;
	unsigned int index = 0;
	while(*keys != 0) {	index++; keys += 9; }
	unsigned __int32 *hashList = (unsigned __int32*)HeapAlloc(GetProcessHeap(), NULL, index*sizeof(unsigned __int32));
	keys = buffer.GetBuffer();
	index = 0;
	while(*keys != 0) 
	{
	
		swscanf(keys, L"%08I32X", &hashList[index++]);
		keys += 9;
	}
	*size = index;
	return hashList;
}

HRESULT PathTrackINI::DeletePathRec(unsigned __int32 pathHash)
{
	HRESULT retCode = S_FALSE;
	wchar_t hash[9];
	if (S_OK == StringCchPrintfW(hash, 9, L"%08I32X", pathHash))
	{
		retCode = DeleteKey(hash);
	}
	return retCode;
}

void  PathTrackINI::FreeHashList(void* data)
{
	HeapFree(GetProcessHeap(), NULL, data);
}