#ifndef NULLSOFT_DROPBOX_PLUGIN_TYPECOLLECTION_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_TYPECOLLECTION_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./itemTypeInterface.h"
#include "../nu/ptrlist.h"


class TypeCollection
{
public:
	typedef BOOL (CALLBACK *TypeEnumProc)(IItemType* /*item*/, ULONG_PTR /*param*/);

public:
	TypeCollection();
	~TypeCollection();

public:
	void RegisterKnownTypes();
	HRESULT CreateItem(LPCTSTR filePath, WIN32_FILE_ATTRIBUTE_DATA *fileAttributes, IFileInfo **ppItem);
	IItemType *FindById(BYTE typeId);
	IItemType *FindByName(LPCTSTR pszName, INT cchName); // cchName can be -1
	INT Count();
	BOOL Enumerate(TypeEnumProc proc, ULONG_PTR param);
	


protected:
	typedef nu::PtrList<IItemType> ITEMTYPELIST;
	
	ITEMTYPELIST typeList;
};

#endif //NULLSOFT_DROPBOX_PLUGIN_TYPECOLLECTION_HEADER