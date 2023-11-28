#include "main.h"
#include "./strings.h"
#include "./wasabi.h"

#include <strsafe.h>

wchar_t * 
String_Malloc(size_t size)
{
	return (wchar_t *)malloc(sizeof(wchar_t) * size);
}

wchar_t * 
String_ReAlloc(wchar_t *string, size_t size)
{
	return (wchar_t *)realloc(string, sizeof(wchar_t) * size);
}

void 
String_Free(wchar_t *string)
{
	if (NULL != string)
		free(string);
}

wchar_t * 
String_Duplicate(const wchar_t *string)
{
	int length;
	wchar_t *copy;

	if (NULL == string)
		return NULL;

	length = lstrlenW(string) + 1;
		
	copy = String_Malloc(length);
	if (NULL != copy)
		CopyMemory(copy, string, sizeof(wchar_t) * length);
	
	return copy;
}

const wchar_t *
String_CopyTo(wchar_t *buffer, size_t bufferSize, const wchar_t *source)
{
	if (FAILED(StringCchCopyExW(buffer, bufferSize, source, NULL, NULL, STRSAFE_IGNORE_NULLS)))
		return NULL;
	
	return buffer;
}

char *
String_ToAnsi(unsigned int codePage,  unsigned long flags, const wchar_t *string, 
			  int stringLength, const char *defaultChar, BOOL *usedDefaultChar)
{
	char *buffer;
	int bufferSize;

	if (stringLength < 0)
		stringLength = lstrlen(string);
	
	bufferSize = WideCharToMultiByte(codePage, flags, string, stringLength, 
							NULL, 0, defaultChar, usedDefaultChar);
	if (0 == bufferSize) 
		return NULL;
	
	buffer = AnsiString_Malloc(bufferSize + 1);
	if (NULL == buffer) 
		return NULL; 
		
	bufferSize = WideCharToMultiByte(codePage, flags, string, stringLength, 
						buffer, bufferSize, defaultChar, usedDefaultChar);
	if (0 == bufferSize)
	{
		AnsiString_Free(buffer);
		return NULL;
	}
	buffer[bufferSize] = '\0';
	return buffer;
}

char * 
AnsiString_Malloc(size_t size)
{
	return (char*)malloc(sizeof(char) * size);
}

char * 
AnsiString_ReAlloc(char *string, size_t size)
{
	return (char*)realloc(string, sizeof(char) * size);
}

void 
AnsiString_Free(char *string)
{
	if (NULL != string)
		free(string);
}

char * 
AnsiString_Duplicate(const char *string)
{
	char *copy;
	INT length;

	if (NULL == string)
		return NULL;

	length = lstrlenA(string) + 1;
		
	copy = AnsiString_Malloc(length);
	if (NULL != copy)
		CopyMemory(copy, string, sizeof(char) * length);
	
	return copy;
}


wchar_t *
AnsiString_ToUnicode(unsigned int codePage, unsigned long flags, const char* string, INT stringLength)
{
	wchar_t *buffer;
	int buffferSize;
	
	if (NULL == string) 
		return NULL;

	buffferSize = MultiByteToWideChar(codePage, flags, string, stringLength, NULL, 0);
	if (0 == buffferSize) 
		return NULL;
	
	if (stringLength > 0) 
		buffferSize++;
	
	buffer = String_Malloc(buffferSize);
	if (NULL == buffer) 
		return NULL;

	if (0 == MultiByteToWideChar(codePage, flags, string, stringLength, buffer, buffferSize))
	{
		String_Free(buffer);
		return NULL;
	}

	if (stringLength > 0)
		buffer[buffferSize - 1] = L'\0';

	return buffer;
}

wchar_t* 
ResourceString_Duplicate(const wchar_t *source)
{
	return (FALSE != IS_INTRESOURCE(source)) ? 
			(LPWSTR)source : 
			String_Duplicate(source);
}

void 
ResourceString_Free(wchar_t *string)
{
	if (FALSE == IS_INTRESOURCE(string))
		String_Free(string);
}

const wchar_t *
ResourceString_CopyTo(wchar_t *buffer, size_t bufferSize, const wchar_t *source)
{
	if (NULL == buffer)
		return NULL;

	if (NULL == source)
	{
		buffer[0] = L'\0';
		return buffer;
	}

	if (FALSE != IS_INTRESOURCE(source))
	{
		if (NULL == WASABI_API_LNG)
			return NULL;
		
		WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)source, buffer, bufferSize);
		return buffer;
	}

	if (FAILED(StringCchCopyExW(buffer, bufferSize, source, NULL, NULL, STRSAFE_IGNORE_NULLS)))
		return NULL;

	return buffer;
}

const wchar_t *
String_SkipTheAndSpace(const wchar_t *string)
{
	const wchar_t *cursor;
	
	if (NULL == string)
		return NULL;

	cursor = string;

	while ((L' ' == *cursor || L'\t' == *cursor) && L'\0' != *cursor)
		cursor++;

	if ((cursor[0] & ~0x20) == L'T'
		&& (cursor[1] & ~0x20) == L'H'
		&& (cursor[2] & ~0x20) == L'E'
		&& cursor[3] == L' ')
	{
		cursor += 4;
	}
	
	while ((L' ' == *cursor || L'\t' == *cursor) && L'\0' != *cursor)
		cursor++;

	if (L'\0' == *cursor)
		return string;

	return cursor;
}

const wchar_t *
String_SkipSpace(const wchar_t *string)
{	
	if (NULL == string)
		return NULL;

	while (L' ' == *string && L'\0' != *string)
		string++;

	return string;
}

wchar_t 
String_GetIndexLetter(const wchar_t *string) 
{
	const wchar_t *begin;

	begin = String_SkipTheAndSpace(string);
	if (NULL == begin)
	{
		begin = string;
		if (NULL == begin)
			return L'\0';
	}

	return (wchar_t)CharUpperW((wchar_t*)*begin);
}

const wchar_t *
String_Join(wchar_t *buffer, size_t bufferSize,
			const wchar_t *string1, const wchar_t *string2, const wchar_t *separator)
{
	HRESULT hr;
	wchar_t *cursor;
	size_t remaining;

	if (NULL == buffer)
		return NULL;
	
	cursor = buffer;
	remaining = bufferSize;


	if (FALSE == IS_STRING_EMPTY(string1))
	{
		hr = StringCchCopyEx(cursor, remaining, string1, &cursor, &remaining, 0);
		if (FAILED(hr))
			return NULL;
	}
	else
		hr = S_OK;
	
	if (SUCCEEDED(hr) && 
		FALSE == IS_STRING_EMPTY(string2))
	{
		if (FALSE == IS_STRING_EMPTY(separator) && 
			cursor != buffer)
		{
			hr = StringCchCopyEx(cursor, remaining, separator, &cursor, &remaining, 0);
			if (FAILED(hr))
				return NULL;
		}

		hr = StringCchCopyEx(cursor, remaining, string2, &cursor, &remaining, 0);
		if (FAILED(hr))
			return NULL;
	}

	return buffer;
}

int
CompareString_IgnoreCase(LCID localeId, const wchar_t *string1, const wchar_t *string2)
{
	if (NULL == string1 || NULL == string2)
		return (int)(intptr_t)(string1 - string2);

	return CompareStringW(Plugin_GetUserLocaleId(), NORM_IGNORECASE, string1, -1, string2, -1);
}

int
CompareString_IgnoreCaseTheSpace(LCID localeId, const wchar_t *string1, const wchar_t *string2)
{
	string1 = String_SkipTheAndSpace(string1);
	string2 = String_SkipTheAndSpace(string2);

	if (NULL == string1)
		string1 = L"";
	
	if (NULL == string2)
		string2 = L"";
	
	return CompareStringW(localeId, NORM_IGNORECASE, string1, -1, string2, -1);
}

unsigned int 
String_ParseKeywords(const wchar_t *input, unsigned int length, wchar_t separator, 
					 BOOL eatSpace, StringParserCb callback, const void *context)
{
	const wchar_t *end, *block, *last;
	unsigned int found;
	StringParserResponse response;

	if (NULL == input || NULL == callback)
		return 0;

	if ((unsigned int)-1 == length)
		length = lstrlen(input);
	
	if (0 == length)
		return 0;

	end = (input + length);

	if(FALSE != eatSpace) 
	{
		while(input < end && L' ' == *input)
			input++;
	}

	if (L'\0' == *input)
		return 0;

	found = 0;
	
	for (;;)
	{
		block = input;
		while(input < end && separator != *input) 
			input++;
		
		last = (input - 1);
		if (FALSE != eatSpace)
		{
			while(last >= block && L' ' == *last) 
				last--;
		}
		
		if (last >= block)
		{
			response = callback(block, (unsigned int)(intptr_t)(last - block) + 1, context);
			if (0 != (StringParserResponse_KeywordFound & response))
				found++;
			
			if (0 != (StringParserResponse_Abort & response))
				return found;
		}

		if (input >= end || L'\0' == *input) 
			return found;

		input++;

		if (FALSE != eatSpace)
		{
			while(last >= block && L' ' == *last) 
				last--;
		}
	}

	return found;
}

unsigned int 
AnsiString_ParseKeywords(const char *input, unsigned int length, char separator, 
					 BOOL eatSpace, AnsiStringParserCb callback, const void *context)
{
	const char *end, *block, *last;
	unsigned int found;
	StringParserResponse response;

	if (NULL == input || NULL == callback)
		return 0;

	if ((unsigned int)-1 == length)
		length = lstrlenA(input);
	
	if (0 == length)
		return 0;

	end = (input + length);

	if(FALSE != eatSpace) 
	{
		while(input < end && L' ' == *input)
			input++;
	}

	if ('\0' == *input)
		return 0;

	found = 0;
	
	for (;;)
	{
		block = input;
		while(input < end && separator != *input) 
			input++;
		
		last = (input - 1);
		if (FALSE != eatSpace)
		{
			while(last >= block && ' ' == *last) 
				last--;
		}
		
		if (last >= block)
		{
			response = callback(block, (unsigned int)(intptr_t)(last - block) + 1, context);
			if (0 != (StringParserResponse_KeywordFound & response))
				found++;
			
			if (0 != (StringParserResponse_Abort & response))
				return found;
		}

		if (input >= end || '\0' == *input) 
			return found;

		input++;

		if (FALSE != eatSpace)
		{
			while(last >= block && L' ' == *last) 
				last--;
		}
	}

	return found;
}

BOOL
FileString_IsBadChar(const wchar_t testChar)
{

	return (testChar < 32 ||
			L'<' == testChar || 
			L'>' == testChar ||
			L':' == testChar ||
			L'\"' == testChar ||
			L'/' == testChar ||
			L'\\' == testChar ||
			L'|' == testChar ||
			L'?' == testChar ||
			L'*' == testChar);
}

size_t 
FileString_ReplaceInvalidChars(wchar_t *input, const wchar_t replaceChar)
{
	size_t replaced;
	wchar_t *cursor;

	replaced = 0;
	cursor = input;

	if (NULL == input)
		return 0;

	if (FALSE != FileString_IsBadChar(replaceChar))
		return 0;

	while(L'\0' != *cursor)
	{
		if (FALSE != FileString_IsBadChar(*cursor))
		{
			*cursor = replaceChar;
			replaced++;
		}
		
		cursor = CharNext(cursor);
	}

	return replaced;
}

size_t 
FileString_RemoveInvalidChars(wchar_t *input)
{
	size_t removed;
	const wchar_t *cursor;
	wchar_t *insert;

	removed = 0;
	cursor = input;
	insert = input;

	if (NULL == input)
		return 0;

	while(L'\0' != *cursor)
	{
		if (FALSE != FileString_IsBadChar(*cursor))
		{
			removed++;
		}
		else
		{
			*insert = *cursor;
			insert = CharNext(insert);
		}
				
		cursor = CharNext(cursor);
	}

	if (0 != removed)
		*insert = L'\0';

	return removed;
}

