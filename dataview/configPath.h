#ifndef _NULLSOFT_WINAMP_DATAVIEW_CONFIG_PATH_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_CONFIG_PATH_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_viewconfig.h"

class ConfigPath
{
protected:
	ConfigPath(const wchar_t *path);
	~ConfigPath();

public:
	static HRESULT CreateInstance(const wchar_t *path,
								  ConfigPath **instance);

public:
	size_t AddRef();
	size_t Release();

	HRESULT GetPath(const wchar_t **pathName, BOOL createMissing);
	HRESULT GetShortPath(const char **pathName, BOOL createMissing);



protected:
	size_t ref;
	wchar_t *path;
	char *shortPath;
};

#endif // _NULLSOFT_WINAMP_DATAVIEW_CONFIG_PATH_HEADER
