#include ".\ini.h"

BaseINI::BaseINI(void)
{
}

BaseINI::~BaseINI(void)
{
}

BaseINI::BaseINI(const wchar_t *file, const wchar_t *section)
{
	fileName = file;
	sectionName = section;
}
void BaseINI::SetWrokingINI(const wchar_t *file)
{
	fileName = file;
}
void BaseINI::SetWorkingSection(const wchar_t *section)
{
	sectionName = section;
}
MLString* BaseINI::GetSections(MLString *sections)
{
	int attempt = 0;
	unsigned int length = sections->GetBufferLength();
	unsigned int retVal = GetPrivateProfileStringW(NULL, NULL, L"", sections->GetBuffer(), length, fileName); 
	while(length - 2 == retVal)
	{
		attempt++;
		sections->Allocate(length + 128*attempt);
		length = sections->GetBufferLength();
		retVal = GetPrivateProfileStringW(NULL, NULL, L"", sections->GetBuffer(), length, fileName); 
	}
	sections->UpdateBuffer(retVal);
	return sections;
}
MLString* BaseINI::GetKeys(MLString *keys)
{
	int attempt = 0;
	unsigned int length = keys->GetBufferLength();
	unsigned int retVal = GetPrivateProfileStringW(sectionName, NULL, L"", keys->GetBuffer(), length, fileName); 
	while(length - 2 == retVal)
	{
		attempt++;
		keys->Allocate(length + 128*attempt);
		length = keys->GetBufferLength();
		retVal = GetPrivateProfileStringW(sectionName, NULL, L"", keys->GetBuffer(), length, fileName); 
	}
	keys->UpdateBuffer(retVal);
	return keys;
}
MLString* BaseINI::GetStringValue(const wchar_t *key, MLString *value, const wchar_t *defVal)
{
	int attempt = 0;
	unsigned int length = value->GetBufferLength();
	if ( 0 == length )
	{
		if (S_OK != value->Allocate(32)) return value;
		length = value->GetBufferLength();
	}
	unsigned int retVal = GetPrivateProfileStringW(sectionName, key, defVal, value->GetBuffer(), length, fileName); 
	while(length - 1 == retVal || length - 2 == retVal)
	{
		attempt++;
		value->Allocate(length + 128*attempt);
		length = value->GetBufferLength();
		retVal = GetPrivateProfileStringW(sectionName, key, defVal, value->GetBuffer(), length, fileName); 
	}
	value->UpdateBuffer(retVal);
	return value;
}

unsigned int BaseINI::GetIntValue(const wchar_t *key, int defVal)
{
	return GetPrivateProfileInt(sectionName, key, defVal, fileName);
}
void BaseINI::SetValue(const wchar_t *key, const wchar_t *value)
{
	WritePrivateProfileString(sectionName, key, value, fileName);
}
void BaseINI::SetValue(const wchar_t *key, unsigned int &value)
{
	wchar_t buffer[64];
	SetValue(key, _ltow(value, buffer, 10));
}

void BaseINI::SetValue(const wchar_t *key, int &value)
{
	wchar_t buffer[32];
	SetValue(key, _itow(value, buffer, 10));
}

HRESULT BaseINI::DeleteSection(void)
{
	return (WritePrivateProfileStringW(sectionName, NULL, NULL, fileName)) ? S_OK : S_FALSE;
}
HRESULT BaseINI::DeleteKey(const wchar_t *key)
{
	return (WritePrivateProfileStringW(sectionName, key, NULL, fileName)) ? S_OK : S_FALSE;
}
void BaseINI::DeleteFile(int onlyEmpty)
{
	if (!IsFileExist()) return;
	if (onlyEmpty)
	{
		HANDLE file = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == file) return;
		int size = GetFileSize(file, NULL);
		if (size > 0)
		{
			if(size != sizeof(WORD) || !HasBOM(file))
			{
				CloseHandle(file);
				return;
			}
		}
		CloseHandle(file);
	}
	::DeleteFile(fileName);
}
void BaseINI::CreateUnicode(int writeSection)
{
	WORD BOM = 0xFEFF;
	DWORD written;
	HANDLE file = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == file) return;

	WriteFile(file, &BOM, sizeof(WORD), &written, NULL);
	if (writeSection)
	{
		MLString tmp(256);
		tmp.Format(L"[%s]", sectionName);
		WriteFile(file, (wchar_t*)tmp, tmp.GetLength()*sizeof(wchar_t), &written, NULL);
	}
	CloseHandle(file);
}

int BaseINI::HasBOM(HANDLE file)
{
	WORD BOM = 0xFEFF;
	WORD buffer;
	DWORD read;
	BOOL retCode = ReadFile(file, &buffer, sizeof(WORD), &read, NULL);
	return (retCode && buffer == BOM);
	
}
int BaseINI::IsFileExist()
{
	DWORD attrib = GetFileAttributes(fileName);
	if (INVALID_FILE_ATTRIBUTES == attrib)
	{
		switch(GetLastError())
		{
			case ERROR_FILE_NOT_FOUND:
			case ERROR_PATH_NOT_FOUND:
				return 0;
			case ERROR_ACCESS_DENIED:
				return 2;
		}
	}
	return 1;
}
