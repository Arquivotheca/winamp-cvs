#include "main.h"
#include "./stringSortKey.h"

StringSortKey::StringSortKey(LCID _localeId, const void *_value, size_t _size)
	: ref(1), localeId(_localeId), value(_value), size(_size)
{
}

StringSortKey::~StringSortKey()
{
}

HRESULT StringSortKey::CreateInstance(LCID localeId, const wchar_t *string, StringSortKeyFlags flags, StringSortKey **instance)
{
	size_t length, required;
	void *buffer, *value;
	unsigned long mapFlags;
	unsigned short charType;
		
	if (NULL == instance)
		return E_POINTER;

	if (NULL == string)
		length = 0;
	else
	{
		if (0 != (StringSortKey_Trim & flags))
		{
			while(L'\0' != *string && 
				  FALSE != GetStringTypeW(CT_CTYPE1, string, 1, &charType) && 
				  0 != ((C1_SPACE | C1_CNTRL | C1_BLANK | C1_PUNCT) & charType))
			{
				string = CharNext(string);
			}
		}

		length = lstrlen(string);

		if (0 != (StringSortKey_Trim & flags))
		{	
			const wchar_t *end;
			end = string + length;

			for(;;)
			{
				end = CharPrev(string, end);
				if (string == end ||
					FALSE == GetStringTypeW(CT_CTYPE1, end, 1, &charType))
				{
					break;
				}
				if (0 == ((C1_SPACE | C1_CNTRL | C1_BLANK | C1_PUNCT) & charType))
				{
					length = (size_t)(ptrdiff_t)(end - string) + 1;
					break;
				}
			}
		}

		if (0 != (StringSortKey_RemoveArticle & flags))
		{
			const wchar_t *string2;

			string2 = Plugin_GetStringArticle()->Remove(string, &length);
			if (string2 != string && 
				0 != (StringSortKey_Trim & flags))
			{
				string = string2;

				while(L'\0' != *string && 
					  FALSE != GetStringTypeW(CT_CTYPE1, string, 1, &charType) && 
					  0 != ((C1_SPACE | C1_CNTRL | C1_BLANK | C1_PUNCT) & charType))
				{
					string2 = CharNext(string);
					length -= (size_t)(ptrdiff_t)(string2 - string);
					string = string2;
				}
			}
		}
	}	
	
	if (0 != length)
	{
		mapFlags = LCMAP_SORTKEY | SORT_STRINGSORT;
		if (0 != (StringSortKey_IgnoreCase & flags))
			mapFlags |= LINGUISTIC_IGNORECASE;

		required = LCMapString(localeId, mapFlags, string, length, NULL, 0);
		if (0 == required)
			return E_FAIL;

		buffer = malloc(sizeof(StringSortKey) + (sizeof(unsigned char) * required));
		if (NULL == buffer)
			return E_FAIL;
			
		value = (void*)(((BYTE*)buffer) + sizeof(StringSortKey));
		required = LCMapString(localeId, mapFlags, string, length, (wchar_t*)value, required);
		if (0 == required)
		{
			free(buffer);
			return E_FAIL;
		}
	}
	else
	{
		required = 1;
		buffer = malloc(sizeof(StringSortKey) + (sizeof(unsigned char) * required));
		if (NULL == buffer)
			return E_FAIL;
		
		value = (void*)(((BYTE*)buffer) + sizeof(StringSortKey));
		*((unsigned char*)value) = 0x00;
		
	}

	*instance = new(buffer) StringSortKey(localeId, value, required);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;

}

size_t StringSortKey::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t StringSortKey::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int StringSortKey::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_SortKey))
		*object = static_cast<ifc_sortkey*>(this);
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

const void *StringSortKey::GetValue()
{
	return value;
}

size_t StringSortKey::GetSize()
{
	return size;
}

LCID StringSortKey::GetLocaleId()
{
	return localeId;
}

int StringSortKey::Compare(const void *_value, size_t _size)
{	
	int result;
	
	if (NULL == _value || 0 == size)
		return COBJ_ERROR;

	result = memcmp(value, _value, (size < _size) ? size : _size);
	if (0 == result)
		return COBJ_COMPARE(size, _size);
	
	return (result + 2);
}

#define CBCLASS StringSortKey
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETVALUE, GetValue)
CB(API_GETSIZE, GetSize)
CB(API_GETLOCALEID, GetLocaleId)
CB(API_COMPARE, Compare)
END_DISPATCH;
#undef CBCLASS