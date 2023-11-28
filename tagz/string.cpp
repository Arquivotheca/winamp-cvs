#include "string.h"
#include <windows.h>
#include <strsafe.h>
using namespace tagz_;

string::string()
{
	data = 0;
	size = 0;
	used = 0;
}

void string::AddDBChar(LPTSTR c)
{
	LPTSTR end = CharNext(c);
	while (c != end)
		AddChar(*c++);
}

void string::AddChar(TCHAR c)
{
	if (!data)
	{
		size=512;
		data = (LPTSTR)malloc(size * sizeof(TCHAR));
		data[0]=0;
		used = 0;
	}
	else if (size == used)
	{
		size <<= 1;
		LPTSTR newData = (LPTSTR)realloc((LPTSTR)data, size * sizeof(TCHAR));
		if (!newData)
		{
			free(data);
			data = (LPTSTR)malloc(size * sizeof(TCHAR));
		}
		else
			data = newData;
	}
	if (data) 
		data[used++] = c;
}

void string::AddInt(int i)
{
	TCHAR simpleInt[16];
	StringCchPrintf(simpleInt, 16, TEXT("%i"), i);
	AddString(simpleInt);
}

void string::AddString(LPCTSTR z)
{
	while (*z)
	{
		AddChar(*z);
		z++;
	}
}

void string::AddString(string & s)
{
	AddString(s.Peek());
}

string::~string()
{
	if (data) free(data);
}

LPTSTR string::GetBuf()
{
	if (!data)
		return ::_wcsdup(L"");
	LPTSTR r = (LPTSTR)realloc(data, (used + 1) * sizeof(TCHAR));
	if (!r)
	{
		free(data);
		r = (LPTSTR)malloc( (used + 1) * sizeof(TCHAR));
	}
	r[used] = 0;
	data = 0;
	return r;
}

TCHAR string::operator[](size_t i)
{
	if (!data || i >= used) 
		return 0;
	else 
		return data[i];
}

size_t string::Len()
{
	return data ? used : 0;
}

void string::Reset()
{
	if (data) {free(data);data = 0;}
}

LPCTSTR string::Peek()
{
	AddChar(0);
	used--;
	return data;
}

