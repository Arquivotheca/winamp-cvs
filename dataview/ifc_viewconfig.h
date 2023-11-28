#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_CONFIG_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_CONFIG_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {181E8A2E-1BD2-4853-B5ED-A8F88287ADA1}
static const GUID IFC_ViewConfig = 
{ 0x181e8a2e, 0x1bd2, 0x4853, { 0xb5, 0xed, 0xa8, 0xf8, 0x82, 0x87, 0xad, 0xa1 } };


#include <bfc/dispatch.h>

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewconfig : public Dispatchable
{

protected:
	ifc_viewconfig() {}
	~ifc_viewconfig() {}

public:
	HRESULT QueryGroup(const char *name, ifc_viewconfig **group);
	
	HRESULT DeleteGroup();
	HRESULT DeleteKey(const char *key);
	
	int ReadInt(const char *key, int defaultValue);
	size_t ReadString(const char *key, const char *defaultValue, char *buffer, size_t bufferSize);
	BOOL ReadBool(const char *key, BOOL defaultValue);

	HRESULT WriteInt(const char *key, int value);
	HRESULT WriteString(const char *key, const char *value);
	HRESULT WriteBool(const char *key, BOOL value);

public:
	DISPATCH_CODES
	{
		API_QUERYGROUP = 10,
		API_DELETEGROUP = 20,
		API_DELETEKEY = 30,
		API_READINT = 40,
		API_READSTRING = 50,
		API_READBOOL = 60,
		API_WRITEINT = 70,
		API_WRITESTRING = 80,
		API_WRITEBOOL = 90,
	};
};

inline HRESULT ifc_viewconfig::QueryGroup(const char *name, ifc_viewconfig **group)
{
	return _call(API_QUERYGROUP, (HRESULT)E_NOTIMPL, name, group);
}

inline HRESULT ifc_viewconfig::DeleteGroup()
{
	return _call(API_DELETEGROUP, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_viewconfig::DeleteKey(const char *key)
{
	return _call(API_DELETEKEY, (HRESULT)E_NOTIMPL, key);
}

inline int ifc_viewconfig::ReadInt(const char *key, int defaultValue)
{
	return _call(API_READINT, (int)defaultValue, key, defaultValue);
}

inline size_t ifc_viewconfig::ReadString(const char *key, const char *defaultValue, char *buffer, size_t bufferSize)
{
	return _call(API_READSTRING, (unsigned int)0, key, defaultValue, buffer, bufferSize);
}

inline BOOL ifc_viewconfig::ReadBool(const char *key, BOOL defaultValue)
{
	return _call(API_READBOOL, (BOOL)defaultValue, key, defaultValue);
}

inline HRESULT ifc_viewconfig::WriteInt(const char *key, int value)
{
	return _call(API_WRITEINT, (HRESULT)E_NOTIMPL, key, value);
}

inline HRESULT ifc_viewconfig::WriteString(const char *key, const char *value)
{
	return _call(API_WRITESTRING, (HRESULT)E_NOTIMPL, key, value);
}

inline HRESULT ifc_viewconfig::WriteBool(const char *key, BOOL value)
{
	return _call(API_WRITEBOOL, (HRESULT)E_NOTIMPL, key, value);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_CONFIG_INTERFACE_HEADER