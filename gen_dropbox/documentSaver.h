#ifndef NULLOSFT_DROPBOX_DOCUMENTSAVER_HEADER
#define NULLOSFT_DROPBOX_DOCUMENTSAVER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../playlist/ifc_playlist.h"

interface IFileInfo;
 
class DocumentSaver : public ifc_playlist
{

public:
	DocumentSaver(LPCTSTR pszDocPath, LPCTSTR pszDocName, IFileInfo **ppFileList, size_t fileCount,	BOOL bRegister);
	virtual ~DocumentSaver();

public:
	/*** ifc_playlist ***/
	size_t GetNumItems();
	size_t GetItem(size_t item, wchar_t *filename, size_t filenameCch);
	size_t GetItemTitle(size_t item, wchar_t *title, size_t titleCch);
	int GetItemLengthMs(size_t item); // TODO: maybe microsecond for better resolution?
	size_t GetItemExtendedInfo(size_t item, const wchar_t *metadata, wchar_t *info, size_t infoCch);

	HRESULT Save();
protected:
	RECVS_DISPATCH;

protected:
	LPTSTR	pszTitle;
    LPTSTR	pszPath;
	IFileInfo **ppItems;
	size_t	count;
	BOOL registerPl;
};


#endif // NULLOSFT_DROPBOX_DOCUMENTSAVER_HEADER


