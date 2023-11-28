#ifndef NULLSOFT_UTILITY_STRING_H
#define NULLSOFT_UTILITY_STRING_H

#include <malloc.h>
#include <string.h>

class UnicodeString
{
public:
	UnicodeString()
	{
		str=0;
	}
	UnicodeString(const wchar_t *copy)
	{
		if (copy)
			str = _wcsdup(copy);
		else
			str = 0;
	}
	~UnicodeString()
	{
		free(str);
	}

	UnicodeString(const UnicodeString &copy)
	{
		if (copy.str)
			str = _wcsdup(copy.str);
		else
			str=0;
	}

	void operator =(const UnicodeString &copy)
	{
		free(str);
			if (copy.str)
			str = _wcsdup(copy.str);
		else
			str=0;
	}

	bool operator >(const UnicodeString &comp) const
	{
		return wcscmp(str, comp.str) > 0;
	}

	bool operator <(const UnicodeString &comp) const
	{
		return wcscmp(str, comp.str) < 0;
	}

	bool operator ==(const wchar_t *comp) const
	{
		if (!str && !comp)
			return true; // TODO: is this correct?
		if (!str || !comp)
			return false;
		return !wcscmp(str, comp);
	}

	bool operator ==(const UnicodeString &comp) const
	{
		return operator ==(comp.str);
	}
	const wchar_t *c_str() const { return str; }
private:
	wchar_t *str;
};


#endif