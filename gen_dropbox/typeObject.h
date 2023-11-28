#ifndef NULLSOFT_DROPBOX_PLUGIN_TYPEOBJECT_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_TYPEOBJECT_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./itemTypeInterface.h"


class TypeObject : public IItemType
{
public:
	typedef HRESULT (STDMETHODCALLTYPE *CreatorProc)(LPCTSTR /*pszFiles*/, 
							WIN32_FILE_ATTRIBUTE_DATA* /*attributeData*/, IFileInfo** /*instanceOut*/);

public:
	TypeObject(UINT nId, LPCSTR pszName, UINT nIconId, LPCTSTR pszDisplayName, LPCTSTR pszDescription, UINT nCapabilities, CreatorProc fnCreator);
	virtual ~TypeObject();

public:
	/*** IUnknown ***/
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/*** IItemType ***/
	STDMETHOD_(BYTE, GetId)(void);
	STDMETHOD(GetName)(LPTSTR pszBuffer, INT cchBufferMax);
	STDMETHOD(GetDisplayName)(LPTSTR pszBuffer, INT cchBufferMax);
	STDMETHOD(GetDescription)(LPTSTR pszBuffer, INT cchBufferMax);
	STDMETHOD_(UINT, GetCapabilities)(void);
	STDMETHOD_(UINT, GetIconId)(void);
	STDMETHOD(CreateItem)(LPCTSTR pszFilePath, WIN32_FILE_ATTRIBUTE_DATA *pAttributeData, IFileInfo **ppItem);


protected:
	ULONG ref;
	BYTE id;
	LPSTR name;
	LPTSTR displayName;
	LPTSTR description;
	UINT capabilities;
	UINT iconId;
	CreatorProc createCallback;

};

#endif //NULLSOFT_DROPBOX_PLUGIN_TYPEOBJECT_HEADER