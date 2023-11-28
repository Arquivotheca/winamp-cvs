#ifndef NULLSOFT_DROPBOX_PLUGIN_FILEINFO_UNKNOWNFILE_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FILEINFO_UNKNOWNFILE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./fileInfoInterface.h"
#include "./genericItemMeta.h"


class UnknownFileInfo : public IFileInfo, public GenericItemMeta
{

public:
	static STDMETHODIMP CreateInstance(LPCTSTR filePath, WIN32_FILE_ATTRIBUTE_DATA *attributes, IFileInfo **itemOut);

protected:
	UnknownFileInfo(LPCTSTR pszFilePath, WIN32_FILE_ATTRIBUTE_DATA *pAttributeData);
	virtual ~UnknownFileInfo();

public:
	static HRESULT ReadFileAttributes(LPCTSTR pszPath, WIN32_FILE_ATTRIBUTE_DATA *pAttributeData);
	static void FindDataToFileAttribute(WIN32_FIND_DATA *pFindData, WIN32_FILE_ATTRIBUTE_DATA *pAttributeData);
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
	BOOL bInfoRead;
	ULONG_PTR extraData;
	WIN32_FILE_ATTRIBUTE_DATA info;
};


#endif //NULLSOFT_DROPBOX_PLUGIN_FILEINFO_UNKNOWNFILE_HEADER