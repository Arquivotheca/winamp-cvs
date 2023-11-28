#ifndef NULLSOFT_DROPBOX_PLUGIN_FILEINFO_PLAYLISTFILE_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FILEINFO_PLAYLISTFILE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./fiUnknown.h"

class PlaylistFileInfo : public UnknownFileInfo
{

public:
	static STDMETHODIMP CreateInstance(LPCTSTR filePath, WIN32_FILE_ATTRIBUTE_DATA *attributes, IFileInfo **itemOut);

protected:
	PlaylistFileInfo(LPCTSTR pszFilePath, WIN32_FILE_ATTRIBUTE_DATA *pAttributeData);
	virtual ~PlaylistFileInfo();

public:

	/*** IFileInfo ***/
	STDMETHOD(GetType)(DWORD *pType);
	STDMETHOD(EnumItems)(IFileEnumerator **);

	/*** IFileMeta ***/
	STDMETHOD(GetReader)(METAKEY *, INT, INT, IFileMetaReader **);
	STDMETHOD(CanPlay)(void);
	STDMETHOD(CanRead)(METAKEY);

};


#endif //NULLSOFT_DROPBOX_PLUGIN_FILEINFO_PLAYLISTFILE_HEADER