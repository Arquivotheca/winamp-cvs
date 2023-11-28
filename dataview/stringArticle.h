#ifndef _NULLSOFT_WINAMP_DATAVIEW_STRING_ARTICLE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_STRING_ARTICLE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../nu/vector.h"

class StringArticle
{

public:
	StringArticle();
	~StringArticle();


public:
	const wchar_t *Remove(const wchar_t *string, size_t *length);
	
	size_t Register(const wchar_t **article, size_t count);

protected:
	typedef struct Record
	{
		size_t length;
		wchar_t *string;
	} Record;

	typedef Vector<Record> RecordList;

	friend static int __cdecl 
	StringArticle_SearchCb(void *context, const void *key, const void *elem);

	friend static int __cdecl 
	StringArticle_FuzzySearchCb(void *context, const void *key, const void *elem);

	friend static int __cdecl 
	StringArticle_SortCb(void *context, const void *elem1, const void *elem2);

	

protected:
	RecordList list;
	size_t bufferSize;
	wchar_t *buffer;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_STRING_ARTICLE_HEADER