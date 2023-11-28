#include "main.h"
#include "ExplorerFindFile.h"

ExplorerFindFile::ExplorerFindFile()
{
	g_SHOpenFolderAndSelectItems = (SHOPENFOLDERANDSELECTITEMS)GetProcAddress(GetModuleHandle("SHELL32"),"SHOpenFolderAndSelectItems");
}

ExplorerFindFile::~ExplorerFindFile()
{
}

void ExplorerFindFile::Reset()
{
	pidlList.clear();
}

// still need to make this handle zip:// entries natively to get them working as needed
BOOL ExplorerFindFile::AddFile(wchar_t* file)
{
	LPSHELLFOLDER pDesktopFolder = 0;
	if(SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
	{
		LPITEMIDLIST filepidl = 0, folderpidl = 0;
		wchar_t folder[MAX_PATH] = {0},
				param[512] = {0},
				*filews = 0;
		int zip = 0;

		// doing this to handle zip:// entries where there is a valid file
		// path included in the entry so extract it (from FFOD plugin code)
		lstrcpynW(param, file, 7);
		if(!lstrcmpiW(param, L"zip://")) {
			file += 6;
			zip = 1;
		}

		filews = file + lstrlenW(file) - 1;
		while(filews && *filews && (*filews != L'.') && (filews != file))
		{
			filews = CharPrevW(file,filews);
			if(zip && *filews == L',')
			{
				zip = 0;
			}
			if(zip && *filews == L'.')
			{
				zip = 0;
				filews = CharPrevW(file,filews);
			}
		}

		while(filews && *filews &&
			  (*filews != L',' && *filews != L':' && *filews != L'|'))
		{
			filews = CharNextW(filews);
		}
		*filews = 0;

		// doing this to handle foo.rsn\bar.spc so it'll just
		// find foo.rsn (which should then be a valid entry)
		filews = wcsstr(file, L".rsn\\");
		if(filews)
		{
			*(filews+4) = 0;
		}

		// and now get the folder path of the file
		lstrcpynW(folder,file,MAX_PATH);

		if (PathIsRelativeW(folder)) {
			// sort out relative files based against winamp.exe
			wchar_t szTemp[MAX_PATH], szTemp2[MAX_PATH];
			GetModuleFileNameW(hMainInstance, szTemp, sizeof(szTemp));
			PathRemoveFileSpecW(szTemp);
			PathCombineW(szTemp2, szTemp, folder);
			PathCanonicalizeW(folder, szTemp2);
			// and once calculated then we copy the file back
			lstrcpynW(file,folder,MAX_PATH);
		}
		PathRemoveFileSpecW(folder);

		HRESULT hr = pDesktopFolder->ParseDisplayName(NULL,0,folder,0,&folderpidl,0);
		if(FAILED(hr)){ pDesktopFolder->Release(); return FALSE; }

		hr = pDesktopFolder->ParseDisplayName(NULL,0,file,0,&filepidl,0);
		if(FAILED(hr)){ pDesktopFolder->Release(); return FALSE; }

		PIDLListMap::iterator itr=pidlList.begin();
		for (;itr!=pidlList.end();itr++)
		{
			wchar_t tmp[MAX_PATH] = {0};
			SHGetPathFromIDListW(itr->first,tmp);
			// the LPITEMIDLIST isn't consistant for all calls it seems and
			// so it requires doing a physical check against the folder path
			// rather than comparing LPITEMIDLIST values which would be nice
			// TODO - is it worth caching the folder path or just keep this?
			if(!_wcsnicmp(folder,tmp,wcslen(folder)))
			{
				itr->second.push_back(filepidl);
				break;
			}
		}

		// if we got here then there was no match with the currently parsed
		// folder and so we need to add it to the pidllist for use later on
		if(itr == pidlList.end())
		{
			Vector<LPCITEMIDLIST> list;
			list.push_back(filepidl);
			PIDLListMap::MapPair p(folderpidl, list);
			pidlList.insert(p);
		}

		pDesktopFolder->Release();
		return TRUE;
	}
	return FALSE;
}

BOOL ExplorerFindFile::ShowFiles()
{
	if(g_SHOpenFolderAndSelectItems)
	{
		if(pidlList.size())
		{
			BOOL ret = TRUE;
			for (PIDLListMap::iterator itr=pidlList.begin();itr!=pidlList.end();itr++)
			{
				if(FAILED(g_SHOpenFolderAndSelectItems(itr->first,itr->second.size(),itr->second.begin(),NULL))){
					ret = FALSE;
				}
			}
			Reset();
			return ret;
		}
	}
	// fallback for the single item case ie win2k
	else{
		if(pidlList.size())
		{
			// open an explorer window with the file selected in it & if shift is down then open a new window
			// won't reselect if the explorer view is already open and there was a selection (annoying :( )
			wchar_t tmp[1024] = {0}, file[MAX_PATH] = {0};
			SHGetPathFromIDListW(*pidlList.begin()->second.begin(),file);
			StringCchPrintfW(tmp, 1024, L"/select,\"%s\"", file);
			BOOL ret = (ShellExecuteW(hMainWindow, NULL, L"explorer.exe", tmp, NULL, SW_SHOWNORMAL)>(HINSTANCE)32);
			Reset();
			return ret;
		}
	}
	return FALSE;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS ExplorerFindFile
START_DISPATCH;
CB(API_EXPLORERFINDFILE_ADDFILE, AddFile)
CB(API_EXPLORERFINDFILE_SHOWFILES, ShowFiles)
VCB(API_EXPLORERFINDFILE_RESET, Reset)
END_DISPATCH