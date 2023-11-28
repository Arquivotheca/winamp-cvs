#ifndef NULLSOFT_DROPBOX_PLUGIN_FILEINFO_MISSINGFILE_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FILEINFO_MISSINGFILE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./fileInfoInterface.h"
#include "./genericItemMeta.h"

class MissingFileInfo : public IFileInfo, public GenericItemMeta
{

public:
	static STDMETHODIMP CreateInstance(LPCTSTR filePath, WIN32_FILE_ATTRIBUTE_DATA *attributes, IFileInfo **itemOut);

protected:
	MissingFileInfo(LPCTSTR pszFilePath);
	virtual ~MissingFileInfo();


public:
	/*** IUnknown ***/
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/*** IFileInfo ***/
	STDMETHOD(GetPath)(LPCTSTR *pszBuffer);
	STDMETHOD(GetFileName)(LPCTSTR *pszBuffer);
	STDMETHOD(GetExtension)(LPCTSTR *pszBuffer);
	STDMETHOD(GetType)(DWORD *pType);
	STDMETHOD(GetSize)(ULONGLONG *pSize);
	STDMETHOD(GetAttributes)(DWORD *pAttributes);
	STDMETHOD(GetCreationTime)(FILETIME *pTime);
	STDMETHOD(GetModifiedTime)(FILETIME *pTime);

	STDMETHOD(ResetCache)(void);
	STDMETHOD(EnumItems)(IFileEnumerator **);
	STDMETHOD(CanCopy)(void);
	STDMETHOD(CanPlay)(void);

	STDMETHOD(SetExtraInfo)(ULONG_PTR data);
	STDMETHOD(GetExtraInfo)(ULONG_PTR *pData);


protected:
	ULONG ref;
	LPTSTR pszPath;
	ULONG_PTR extraData;

};

#endif //NULLSOFT_DROPBOX_PLUGIN_FILEINFO_VIDEOFILE_HEADER