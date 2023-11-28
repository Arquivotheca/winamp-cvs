#ifndef _NULLSOFT_WINAMP_DATAVIEW_STRINGS_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_STRINGS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#define IS_STRING_EMPTY(_string) (NULL == (_string) || L'\0' == *(_string))


wchar_t *
String_Malloc(size_t size);

wchar_t * 
String_ReAlloc(wchar_t *string, 
			   size_t size);

void 
String_Free(wchar_t *string);

wchar_t * 
String_Duplicate(const wchar_t *string);

const wchar_t *
String_CopyTo(wchar_t *buffer, 
			  size_t bufferSize, 
			  const wchar_t *source);

char *
String_ToAnsi(unsigned int codePage, 
			  unsigned long flags, 
			  const wchar_t *string, 
			  int stringLength, 
			  const char *defaultChar, 
			  BOOL *usedDefaultChar);


/*
	Ansi String
*/

char * 
AnsiString_Malloc(size_t size);

char *
AnsiString_ReAlloc(char *string, 
				   size_t size);

void 
AnsiString_Free(char *string);

char * 
AnsiString_Duplicate(const char *string);

wchar_t *
AnsiString_ToUnicode(unsigned int codePage, 
					 unsigned long flags, 
					 const char *string, 
					 int stringLength);


wchar_t* 
ResourceString_Duplicate(const wchar_t *source);

void 
ResourceString_Free(wchar_t *string);


const wchar_t *
ResourceString_CopyTo(wchar_t *buffer, 
					  size_t bufferSize, 
					  const wchar_t *source);

const wchar_t *
String_SkipSpace(const wchar_t *string);

const wchar_t *
String_SkipTheAndSpace(const wchar_t *string);

wchar_t 
String_GetIndexLetter(const wchar_t *string);

const wchar_t *
String_Join(wchar_t *buffer, 
			size_t bufferSize,
			const wchar_t *string1,
			const wchar_t *string2,
			const wchar_t *separator);
int
CompareString_IgnoreCaseTheSpace(LCID localeId, const wchar_t *str1, const wchar_t *str2);

int
CompareString_IgnoreCase(LCID localeId, const wchar_t *str1, const wchar_t *str2);


typedef enum StringParserResponse
{
	StringParserResponse_Continue = 0,
	StringParserResponse_Abort = 1,
	StringParserResponse_KeywordFound = 0x80000000,
} StringParserResponse;
DEFINE_ENUM_FLAG_OPERATORS(StringParserResponse);

typedef StringParserResponse (__stdcall *StringParserCb)(const wchar_t * /*keyword*/,
														 unsigned int /*length*/, 
														 const void * /*context*/);

unsigned int 
String_ParseKeywords(const wchar_t *input, 
					 unsigned int length, 
					 wchar_t separator, 
					 BOOL eatSpace, 
					 StringParserCb callback, 
					 const void *context);


typedef StringParserResponse (__stdcall *AnsiStringParserCb)(const char * /*keyword*/,
														 unsigned int /*length*/, 
														 const void * /*context*/);

unsigned int 
AnsiString_ParseKeywords(const char *input, 
						 unsigned int length, 
						 char separator, 
						 BOOL eatSpace, 
						 AnsiStringParserCb callback, 
						 const void *context);

BOOL
FileString_IsBadChar(const wchar_t testChar);

size_t
FileString_ReplaceInvalidChars(wchar_t *input, const wchar_t replaceChar);

size_t
FileString_RemoveInvalidChars(wchar_t *input);

#endif //_NULLSOFT_WINAMP_DATAVIEW_STRINGS_HEADER