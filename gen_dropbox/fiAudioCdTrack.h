#ifndef NULLSOFT_DROPBOX_PLUGIN_FILEINFO_AUDIOCDTRACK_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FILEINFO_AUDIOCDTRACK_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./fiAudio.h"

class AudioCdTrackInfo : public AudioFileInfo
{

public:
	static STDMETHODIMP CreateInstance(LPCTSTR filePath, WIN32_FILE_ATTRIBUTE_DATA *attributes, IFileInfo **itemOut);

protected:
	AudioCdTrackInfo(LPCTSTR pszFilePath, WIN32_FILE_ATTRIBUTE_DATA *pAttributeData);
	virtual ~AudioCdTrackInfo();

public:
	/*** IFileInfo ***/
	STDMETHOD(GetType)(DWORD *pType);
};


#endif //NULLSOFT_DROPBOX_PLUGIN_FILEINFO_AUDIOCDTRACK_HEADER