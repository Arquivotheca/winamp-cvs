#ifndef NULLSOFT_DROPBOX_PLUGIN_FILEINFO_FOLDERFILE_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FILEINFO_FOLDERFILE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./fiUnknown.h"
#include "./fileEnumInterface.h"

class FolderFileInfo : public UnknownFileInfo
{

public:
	static STDMETHODIMP CreateInstance(LPCTSTR filePath, WIN32_FILE_ATTRIBUTE_DATA *attributes, IFileInfo **itemOut);

protected:
	FolderFileInfo(LPCTSTR pszFilePath, WIN32_FILE_ATTRIBUTE_DATA *pAttributeData);
	virtual ~FolderFileInfo();

public:

	/*** IFileInfo ***/
	STDMETHOD(GetExtension)(LPCTSTR *pszBuffer);
	STDMETHOD(GetType)(DWORD *pType);
	STDMETHOD(GetSize)(ULONGLONG *pSize);
	STDMETHOD(EnumItems)(IFileEnumerator **);
    
};


#endif //NULLSOFT_DROPBOX_PLUGIN_FILEINFO_FOLDERFILE_HEADER