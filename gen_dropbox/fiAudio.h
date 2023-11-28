#ifndef NULLSOFT_DROPBOX_PLUGIN_FILEINFO_AUDIOFILE_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FILEINFO_AUDIOFILE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./fiUnknown.h"
#include "./fileMetaInterface.h"

class AudioFileInfo : public UnknownFileInfo
{

public:
	static STDMETHODIMP CreateInstance(LPCTSTR filePath, WIN32_FILE_ATTRIBUTE_DATA *attributes, IFileInfo **itemOut);
	
protected:
	AudioFileInfo(LPCTSTR pszFilePath, WIN32_FILE_ATTRIBUTE_DATA *pAttributeData);
	virtual ~AudioFileInfo();

	
public:

	/*** IFileInfo ***/
	STDMETHOD(GetType)(DWORD *pType);

	/*** IFileMeta ***/
	STDMETHOD(GetReader)(METAKEY *, INT, INT, IFileMetaReader **);
	STDMETHOD(CanPlay)(void);
	STDMETHOD(CanRead)(METAKEY);
		

protected:
};



#endif //NULLSOFT_DROPBOX_PLUGIN_FILEINFO_AUDIOFILE_HEADER