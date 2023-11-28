#ifndef _NULLSOFT_WINAMP_DATAVIEW_CONFIG_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_CONFIG_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_viewconfig.h"
#include "./configPath.h"

class Config : public ifc_viewconfig
{

protected:
	Config(ConfigPath *path, const char *section);
	~Config();

public:
	static HRESULT CreateInstance(const wchar_t *path, 
								  const char *section, 
								  Config **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_viewconfig */
	HRESULT QueryGroup(const char *name, ifc_viewconfig **group);
	HRESULT DeleteGroup();
	HRESULT DeleteKey(const char *key);
	int ReadInt(const char *key, int defaultValue);
	size_t ReadString(const char *key, const char *defaultValue, char *buffer, size_t bufferSize);
	BOOL ReadBool(const char *key, BOOL defaultValue);
	HRESULT WriteInt(const char *key, int value);
	HRESULT WriteString(const char *key, const char *value);
	HRESULT WriteBool(const char *key, BOOL value);

protected:
	HRESULT WriteStringInternal(const char *key, const char *value);

protected:
	size_t ref;
	ConfigPath *path;
	char *section;

protected:
	RECVS_DISPATCH;
};

#endif //_NULLSOFT_WINAMP_DATAVIEW_CONFIG_HEADER