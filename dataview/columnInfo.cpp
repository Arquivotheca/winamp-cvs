#include "main.h"
#include "./columnInfo.h"

ColumnInfo::ColumnInfo(const char *_name, CreateColumnCallback _callback)
	: ref(1), name(NULL), displayName(NULL), width(0), widthMin(0), 
	  widthMax(0), alignMode(AlignMode_Default), sortRule(NULL),
	  createColumnCallback(_callback)
{
	name = AnsiString_Duplicate(_name);
}
	

ColumnInfo::~ColumnInfo()
{
	ResourceString_Free(displayName);
	AnsiString_Free(sortRule);
	AnsiString_Free(name);
}

HRESULT ColumnInfo::CreateInstance(const char *name, const wchar_t *displayName,
						   long width, long widthMin, long widthMax,
						   ifc_viewcolumninfo::AlignMode alignMode, const char *sortRule, 
						   CreateColumnCallback createColumnCallback,
						   ColumnInfo **instance)
{
	ColumnInfo *self;

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (IS_STRING_EMPTY(name))
		return E_INVALIDARG;

	if (NULL == createColumnCallback)
		return E_INVALIDARG;

	self = new (std::nothrow) ColumnInfo(name, createColumnCallback);
	if (NULL == self)
		return E_OUTOFMEMORY;
	
	self->SetDisplayName(displayName);
	self->width = width;
	self->widthMax = widthMax;
	self->widthMin = widthMin;
	self->alignMode = alignMode;
	self->SetSortRule(sortRule);


	*instance = self;
	return S_OK;
}

size_t ColumnInfo::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t ColumnInfo::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int ColumnInfo::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_ViewColumnInfo))
		*object = static_cast<ifc_viewcolumninfo*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

const char *ColumnInfo::GetName()
{
	return name;
}

HRESULT ColumnInfo::GetDisplayName(wchar_t *buffer, size_t bufferSize)
{
	if(NULL == ResourceString_CopyTo(buffer, bufferSize, displayName) &&
	   NULL != displayName)
	{
		return E_OUTOFMEMORY;
	}

	return S_OK;
}

HRESULT ColumnInfo::GetWidth(long *width)
{
	if (NULL == width)
		return E_POINTER;

	*width = this->width;
	return S_OK;
}

HRESULT ColumnInfo::GetMinWidth(long *width)
{
	if (NULL == width)
		return E_POINTER;

	*width = widthMin;
	return S_OK;
}

HRESULT ColumnInfo::GetMaxWidth(long *width)
{
	if (NULL == width)
		return E_POINTER;

	*width = widthMax;
	return S_OK;
}

ifc_viewcolumninfo::AlignMode ColumnInfo::GetAlignMode()
{
	return alignMode;
}

const char *ColumnInfo::GetSortRule()
{
	return sortRule;
}

HRESULT ColumnInfo::CreateColumn(ifc_dataprovider *provider, ifc_viewcolumn **instance)
{
	return createColumnCallback(this, provider, instance);
}

HRESULT ColumnInfo::SetDisplayName(const wchar_t *_displayName)
{
	ResourceString_Free(displayName);
	displayName = ResourceString_Duplicate(_displayName);
	if (NULL == displayName && NULL != _displayName)
		return E_OUTOFMEMORY;

	return S_OK;
}

HRESULT ColumnInfo::SetSortRule(const char *_sortRule)
{
	AnsiString_Free(sortRule);
	sortRule = AnsiString_Duplicate(_sortRule);
	if (NULL == sortRule && NULL != _sortRule)
		return E_OUTOFMEMORY;

	return S_OK;
}

#define CBCLASS ColumnInfo
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETNAME, GetName)
CB(API_GETDISPLAYNAME, GetDisplayName)
CB(API_GETWIDTH, GetWidth)
CB(API_GETMINWIDTH, GetMinWidth)
CB(API_GETMAXWIDTH, GetMaxWidth)
CB(API_GETALIGNMODE, GetAlignMode)
CB(API_GETSORTRULE, GetSortRule)
CB(API_CREATECOLUMN, CreateColumn)
END_DISPATCH;
#undef CBCLASS