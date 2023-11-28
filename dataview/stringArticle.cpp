#include "main.h"
#include "./stringArticle.h"

StringArticle::StringArticle()
	: buffer(NULL), bufferSize(0)
{
}

StringArticle::~StringArticle()
{
	Record *record;
	size_t index;

	index = list.size();
	while(index--)
	{
		record = &list[index];
		String_Free(record->string);
	}

	String_Free(buffer);
}

const wchar_t *StringArticle::Remove(const wchar_t *string, size_t *_length)
{
	size_t length, bufferLength;
	LCID localeId;
	Record record, *record_ptr;

	localeId = Plugin_GetUserLocaleId();

	if (NULL == string)
		return NULL;

	if (0 == bufferSize)
		return string;

	if (NULL == buffer)
	{
		buffer = String_Malloc(bufferSize);
		if (NULL == buffer)
			return string;
	}

	length = (NULL != _length && ((int)*_length) > 0) ? *_length : lstrlen(string);
	bufferLength = (length > bufferSize) ? bufferSize : length;
	
	if (0 == LCMapString(localeId, LCMAP_LOWERCASE, string, bufferLength, buffer, bufferSize))
		return string;
	
	record.length = bufferLength;
	record.string = buffer;
	record_ptr = (Record*)bsearch_s(&record, list.begin(), list.size(), sizeof(Record), 
							StringArticle_FuzzySearchCb, (void*)(intptr_t)localeId);
	if (NULL != record_ptr && 
		record_ptr->length < length)
	{
		unsigned short charType;
		const wchar_t *string2;

		string2 = string + record_ptr->length;

		if (0 != GetStringTypeW(CT_CTYPE1, string2, 1, &charType) &&
			 0 != ((C1_SPACE | C1_CNTRL | C1_BLANK/* | C1_PUNCT*/) & charType))
		{
			string = string2 + 1;
			length -= (record_ptr->length + 1);
		}
	}

	if(NULL != _length)
		*_length = length;

	return string;
}

size_t StringArticle::Register(const wchar_t **articles, size_t count)
{
	Record record;
	const wchar_t *article;
	size_t registered, maxLength;
	LCID localeId;

	if (NULL == articles)
		return 0;

	registered = 0;
	maxLength = 0;
	localeId = Plugin_GetUserLocaleId();

	for(size_t index = 0; index < count; index++)
	{
		article = articles[index];
		if (NULL == article)
			continue;

		record.length = LCMapString(localeId, LCMAP_LOWERCASE, article, -1, NULL, 0);
		if (record.length < 1)
			continue;

		record.string = String_Malloc(record.length);
		if (NULL == record.string)
			continue;
		
		record.length = LCMapString(localeId, LCMAP_LOWERCASE, article, 
									record.length, record.string, record.length);
		if (0 == record.length)
		{
			String_Free(record.string);
			continue;
		}
		
		record.length--;

		if (NULL != bsearch_s(&record, list.begin(), list.size() - registered, 
							  sizeof(Record), StringArticle_SearchCb, 
							  (void*)(intptr_t)localeId))
		{
			String_Free(record.string);
			continue;
		}

		list.push_back(record);

		if (record.length > maxLength)
			maxLength = record.length;
				
		registered++;
	}

	if (maxLength > bufferSize)
	{
		bufferSize = maxLength;
		if (NULL != buffer)
		{
			String_Free(buffer);
			buffer = NULL;
		}
	}

	if (0 != registered)
	{
		qsort_s(list.begin(), list.size(), sizeof(Record), 
				StringArticle_SortCb, (void*)(intptr_t)localeId);
	}

	return registered;
}

static int __cdecl
StringArticle_SearchCb(void *context, const void *key, const void *elem)
{
	const StringArticle::Record *record1, *record2;
	LCID localeId;

	record1 = (const StringArticle::Record*)key;
	record2 = (const StringArticle::Record*)elem;
	localeId = (LCID)(intptr_t)context;

	return CompareString(localeId, 0, record1->string, record1->length, 
						 record2->string, record2->length) - 2;
}

static int __cdecl
StringArticle_FuzzySearchCb(void *context, const void *key, const void *elem)
{
	const StringArticle::Record *record1, *record2;
	LCID localeId;
	size_t length;

	record1 = (const StringArticle::Record*)key;
	record2 = (const StringArticle::Record*)elem;
	localeId = (LCID)(intptr_t)context;

	length = (record1->length > record2->length) ? record2->length : record1->length;
	return CompareString(localeId, 0, record1->string, length, 
						 record2->string, record2->length) - 2;
}

static int __cdecl
StringArticle_SortCb(void *context, const void *elem1, const void *elem2)
{
	const StringArticle::Record *record1, *record2;
	LCID localeId;

	record1 = (const StringArticle::Record*)elem1;
	record2 = (const StringArticle::Record*)elem2;
	localeId = (LCID)(intptr_t)context;

	return CompareString(localeId, 0, record1->string, record1->length, 
						 record2->string, record2->length) - 2;
}