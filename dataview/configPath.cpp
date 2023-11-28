#include "main.h"
#include "./configPath.h"

ConfigPath::ConfigPath(const wchar_t *_path)
: ref(1), shortPath(NULL), path(NULL)
{
	path = String_Duplicate(_path);
}

ConfigPath::~ConfigPath()
{
	String_Free(path);
	AnsiString_Free(shortPath);
}

HRESULT ConfigPath::CreateInstance(const wchar_t *path, ConfigPath **instance)
{
	if (NULL == instance)
		return E_POINTER;

	if (IS_STRING_EMPTY(path))
		return E_INVALIDARG;

	*instance = new (std::nothrow) ConfigPath(path);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t ConfigPath::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t ConfigPath::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

static HRESULT 
ConfigPath_EnsureFileExist(wchar_t *path, wchar_t *filePart)
{
	HRESULT hr;

	if (NULL == filePart)
		return Plugin_EnsurePathExist(path);
			
	if (filePart > path)
		*(filePart - 1) = L'\0';

	hr = Plugin_EnsurePathExist(path);
	
	if (filePart > path)
		*(filePart - 1) = L'\\';
	
	if (SUCCEEDED(hr))
	{
		HANDLE hFile;
		hFile = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
							CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

		if (INVALID_HANDLE_VALUE == hFile)
		{
			unsigned long errorCode;
			errorCode = GetLastError();

			if (ERROR_FILE_EXISTS != errorCode)
				hr = HRESULT_FROM_WIN32(errorCode);
		}
		else
			CloseHandle(hFile);
	}

	return hr;
}

HRESULT ConfigPath::GetPath(const wchar_t **pathName, BOOL createMissing)
{
	if (NULL == pathName)
		return E_POINTER;
	
	if (FALSE != createMissing)
	{
		WIN32_FILE_ATTRIBUTE_DATA attributeData;
		if (0  == GetFileAttributesEx(path, GetFileExInfoStandard, &attributeData))
		{
			HRESULT hr;
			wchar_t *buffer, *filePart;
			wchar_t stackBuffer[MAX_PATH];
			size_t bufferSize, length;
		
			*pathName = NULL;
		
			hr = S_OK;

			buffer = stackBuffer;
			bufferSize = ARRAYSIZE(stackBuffer);
			
			length = GetFullPathName(path, bufferSize, buffer, &filePart);
			if (0 == length)
				RETURN_HRESULT_FROM_LAST_ERROR();
			
			if (length >= bufferSize)
			{
				bufferSize = length;
				buffer = String_Malloc(length);
				if (NULL == buffer)
					return E_OUTOFMEMORY;
							
				length = GetFullPathName(path, bufferSize, buffer, &filePart);
				if (0 == length)
				{
					unsigned long errorCode;
					errorCode = GetLastError();
					hr = HRESULT_FROM_WIN32(errorCode);
				}
			}

			if(SUCCEEDED(hr))
				hr = ConfigPath_EnsureFileExist(buffer, filePart);
						
			if (buffer != stackBuffer)
				String_Free(buffer);

			if (FAILED(hr))
				return hr;
		}
	}

	*pathName = path;
	return S_OK;
}

HRESULT ConfigPath::GetShortPath(const char **pathName, BOOL createMissing)
{
	if (NULL == pathName)
		return E_POINTER;

	if (NULL != shortPath)
	{
		WIN32_FILE_ATTRIBUTE_DATA attributeData;
		if (0 != GetFileAttributesExA(shortPath, GetFileExInfoStandard, &attributeData))
		{
			*pathName = shortPath;
			return S_OK;
		}
		
		AnsiString_Free(shortPath);
		shortPath = NULL;
	}

	if (NULL == shortPath)
	{		
		HRESULT hr;
		const wchar_t *pathUnicode;
		size_t bufferSize, length;
		wchar_t *buffer;
		wchar_t stackBuffer[MAX_PATH];
	
		*pathName = NULL;

		hr = GetPath(&pathUnicode, createMissing);
		if (FAILED(hr))
			return hr;
	
		buffer = stackBuffer;
		bufferSize = ARRAYSIZE(stackBuffer);
		
		length = GetShortPathName(pathUnicode, buffer, bufferSize);
		if (0 == length)
			RETURN_HRESULT_FROM_LAST_ERROR();
		
		if (length >= bufferSize)
		{
			bufferSize = length;
			buffer = String_Malloc(length);
			if (NULL == buffer)
				return E_OUTOFMEMORY;
				
			length = GetShortPathName(pathUnicode, buffer, bufferSize);
			if (0 == length)
			{
				unsigned long errorCode;
				errorCode = GetLastError();
				hr = HRESULT_FROM_WIN32(errorCode);
			}
		}

		if (SUCCEEDED(hr))
			shortPath = String_ToAnsi(CP_ACP, 0, buffer, -1, NULL, NULL);

		if (buffer != stackBuffer)
			String_Free(buffer);

		if (FAILED(hr))
			return hr;

	}

	*pathName = shortPath;
	return S_OK;
}

