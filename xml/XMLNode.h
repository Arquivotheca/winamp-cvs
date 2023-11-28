#pragma once

#include <windows.h>
#include "../nu/Alias.h"
#include "../nu/PtrList.h"
#include "../nu/PtrMap.h"

class PtrMapUnicodeComp
{
public:
	int operator()(const wchar_t *const &str1, const wchar_t *const &str2)
	{
		return CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), 0, str1, -1, str2, -1)-2;
	}
};

class XMLNode
{
public:
	typedef PtrMap<const wchar_t *, wchar_t, PtrMapUnicodeComp> PropMap;
	typedef nu::PtrList<XMLNode> NodeList;
	typedef PtrMap<const wchar_t *, NodeList, PtrMapUnicodeComp> NodeMap;

	XMLNode();
	~XMLNode();
	const XMLNode *Get(const wchar_t *) const;
	const NodeList *GetList(const wchar_t *) const;
	const bool Present(const wchar_t *) const;
	void SetProperty(const wchar_t *prop, const wchar_t *value);
	const wchar_t *GetProperty(const wchar_t *prop) const;
	const wchar_t *GetContent() const;
	void SetContent_Own(wchar_t *new_content);
	void AppendContent(wchar_t *append);
	void AddNode(const wchar_t *name, XMLNode *new_node);
	XMLNode *parent;

private:
	PropMap properties;
	wchar_t *content;
	NodeMap nodes;	
};

