#ifndef NULLSOFT_DROPBOX_PLUGIN_FILEINFO_VIDEOFILE_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FILEINFO_VIDEOFILE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./fiUnknown.h"

class VideoFileInfo : public UnknownFileInfo
{

public:
	static STDMETHODIMP CreateInstance(LPCTSTR filePath, WIN32_FILE_ATTRIBUTE_DATA *attributes, IFileInfo **itemOut);

protected:
	VideoFileInfo(LPCTSTR pszFilePath, WIN32_FILE_ATTRIBUTE_DATA *pAttributeData);
	virtual ~VideoFileInfo();

public:

	/*** IFileInfo ***/
	STDMETHOD(GetType)(DWORD *pType);
	STDMETHOD(CanPlay)(void);
};


#endif //NULLSOFT_DROPBOX_PLUGIN_FILEINFO_VIDEOFILE_HEADER