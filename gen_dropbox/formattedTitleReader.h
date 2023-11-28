#ifndef NULLSOFT_DROPBOX_PLUGIN_FORMATTED_TITLE_READER_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FORMATTED_TITLE_READER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../tagz/ifc_tagprovider.h"
#include "../tagz/ifc_tagparams.h"

interface IFileInfo;

template<int TITLESIZE = 1024, int CHUNKSIZE = 256>
class FormattedTitleReader : public ifc_tagprovider
{
public:
	FormattedTitleReader(IFileInfo *pFileInfo);
	~FormattedTitleReader(){}
	LPCTSTR GetTitle() 
	{ 
		return szTitle; 
	}
	operator LPCTSTR ()
	{
		return szTitle;
	}

	static void ResetCachedTitleFormatTemplate(void);
protected:
	wchar_t *GetTag(const wchar_t *name, ifc_tagparams *parameters); //return 0 if not found
	void FreeTag(wchar_t *tag);

protected:
	RECVS_DISPATCH;

protected:
	IFileInfo	*pInfo;
	TCHAR szTitle[TITLESIZE];
	TCHAR szChunk[CHUNKSIZE];

};

#endif // NULLSOFT_DROPBOX_PLUGIN_FORMATTED_TITLE_READER_HEADER