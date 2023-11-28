#ifndef NULLSOFT_DROPBOX_PLUGIN_FILEINFO_LINKFILE_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FILEINFO_LINKFILE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./fiUnknown.h"

class LinkFileInfo : public UnknownFileInfo
{

public:
	static STDMETHODIMP CreateInstance(LPCTSTR filePath, WIN32_FILE_ATTRIBUTE_DATA *attributes, IFileInfo **itemOut);

protected:
	LinkFileInfo(LPCTSTR pszFilePath, WIN32_FILE_ATTRIBUTE_DATA *pAttributeData);
	virtual ~LinkFileInfo();

public:

	/*** IFileInfo ***/
	STDMETHOD(GetType)(DWORD *pType);
	STDMETHOD(EnumItems)(IFileEnumerator **);

	STDMETHOD(CanPlay)(void);
};


#endif //NULLSOFT_DROPBOX_PLUGIN_FILEINFO_LINKFILE_HEADER