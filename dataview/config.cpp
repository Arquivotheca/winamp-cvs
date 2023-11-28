#include "main.h"
#include "./config.h"

#include <strsafe.h>

Config::Config(ConfigPath *_path, const char *_section)
	: ref(1), path(_path), section(NULL)
{
	if (NULL != path)
		path->AddRef();

	section = AnsiString_Duplicate(_section);
}

Config::~Config()
{
	SafeRelease(path);
	AnsiString_Free(section);
}

HRESULT Config::CreateInstance(const wchar_t *path, const char *section, Config **instance)
{
	HRESULT hr;
	ConfigPath *configPath;

	if (NULL == instance)
		return E_POINTER;
		
	*instance = NULL;

	if (IS_STRING_EMPTY(path) || IS_STRING_EMPTY(section))
		return E_INVALIDARG;

	hr = ConfigPath::CreateInstance(path, &configPath);
	if (FAILED(hr))
		return hr;

	*instance = new (std::nothrow) Config(configPath, section);

	configPath->Release();

	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t Config::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t Config::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int Config::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_ViewConfig))
		*object = static_cast<ifc_viewconfig*>(this);
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

HRESULT Config::QueryGroup(const char *name, ifc_viewconfig **group)
{

	if (NULL == group)
		return E_POINTER;
		
	*group = NULL;

	if (IS_STRING_EMPTY(section))
		return E_INVALIDARG;

	*group = new (std::nothrow) Config(path, name);
	if (NULL == *group)
		return E_OUTOFMEMORY;

	return S_OK;
}

HRESULT Config::DeleteGroup()
{
	return WriteStringInternal(NULL, NULL);
}

HRESULT Config::DeleteKey(const char *key)
{
	return WriteStringInternal(key, NULL);
}

int Config::ReadInt(const char *key, int defaultValue)
{
	const char *filePath;

	if (FAILED(path->GetShortPath(&filePath, FALSE)))
		return defaultValue;
	
	if (NULL == key)
		key = "";

	return GetPrivateProfileIntA(section, key, defaultValue, filePath);
}

size_t Config::ReadString(const char *key, const char *defaultValue, char *buffer, size_t bufferSize)
{
	const char *filePath;

	if (FAILED(path->GetShortPath(&filePath, FALSE)))
	{
		size_t remaining;
		if (FAILED(StringCchCopyExA(buffer, bufferSize, defaultValue, NULL, &remaining, 
					STRSAFE_IGNORE_NULLS)))
		{
			return bufferSize - 1;
		}

		return bufferSize - remaining;
	}

	return GetPrivateProfileStringA(section, key, defaultValue, buffer, bufferSize, filePath);
}

BOOL  Config::ReadBool(const char *key, BOOL defaultValue)
{
	char buffer[32];
	int length;

	length = (int)ReadString(key, NULL, buffer, ARRAYSIZE(buffer));
	if (length < 1) 
		return defaultValue;
	
	if (1 == length)
	{
		switch(*buffer)
		{
			case '0':
			case 'n':
			case 'f':
				return FALSE;
			case '1':
			case 'y':
			case 't':
				return TRUE;
		}
	}
	else
	{
		if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "yes", -1, buffer, length) ||
			CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "true", -1, buffer, length))
		{
			return TRUE;
		}

		if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "no", -1, buffer, length) ||
			CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "false", -1, buffer, length))
		{
			return FALSE;
		}
	}

	if (FALSE != StrToIntExA(buffer, STIF_SUPPORT_HEX,  &length))
		return (0 != length);

	return defaultValue;
}

HRESULT Config::WriteInt(const char *key, int value)
{
	char buffer[32];
	
	if (FAILED(StringCchPrintfA(buffer, ARRAYSIZE(buffer), "%d", value)))
		return E_FAIL;

	return WriteString(key, buffer);
}

HRESULT Config::WriteString(const char *key, const char *value)
{
	if (NULL == key)
		return E_INVALIDARG;

	if (NULL == value)
		value = "";

	return WriteStringInternal(key, value);
}

HRESULT Config::WriteBool(const char *key, BOOL value)
{
	return WriteString(key, ((FALSE != value) ? "yes" : "no")); 
}

HRESULT Config::WriteStringInternal(const char *key, const char *value)
{
	HRESULT hr;
	const char *fileName;

	hr = path->GetShortPath(&fileName, TRUE);
	if (FAILED(hr))
		return hr;

	if (FALSE == WritePrivateProfileStringA(section, key, value, fileName))
		RETURN_HRESULT_FROM_LAST_ERROR();

	return S_OK;
}



#define CBCLASS Config
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_QUERYGROUP, QueryGroup)
CB(API_DELETEGROUP, DeleteGroup)
CB(API_DELETEKEY, DeleteKey)
CB(API_READINT, ReadInt)
CB(API_READBOOL, ReadBool)
CB(API_READSTRING, ReadString)
CB(API_WRITEINT, WriteInt)
CB(API_WRITESTRING, WriteString)
CB(API_WRITEBOOL, WriteBool)
END_DISPATCH;
#undef CBCLASS