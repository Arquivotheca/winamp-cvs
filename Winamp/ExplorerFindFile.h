#ifndef NULLSOFT_WINAMP_EXPLORERFINDFILE_H
#define NULLSOFT_WINAMP_EXPLORERFINDFILE_H

#include "../Agave/ExplorerFindFile/api_explorerfindfile.h"
#include "../nu/PtrMap.h"
#include "../nu/Map.h"

class ExplorerFindFile : public api_explorerfindfile
{
public:
	ExplorerFindFile();
	~ExplorerFindFile();
	static const char *getServiceName() { return "ExplorerFindFile API"; }
	static const GUID getServiceGuid() { return ExplorerFindFileApiGUID; }
	BOOL AddFile(wchar_t* file);
	BOOL ShowFiles();
	void Reset();
protected:
	RECVS_DISPATCH;

	typedef HRESULT (STDAPICALLTYPE *SHOPENFOLDERANDSELECTITEMS)(PCIDLIST_ABSOLUTE pidlFolder, UINT cidl, PCUITEMID_CHILD_ARRAY apidl, DWORD dwFlags);
	SHOPENFOLDERANDSELECTITEMS g_SHOpenFolderAndSelectItems;

	typedef nu::Map<LPITEMIDLIST, Vector<LPCITEMIDLIST>> PIDLListMap;
	PIDLListMap pidlList;
};

extern ExplorerFindFile *explorerFindFileManager;

#endif