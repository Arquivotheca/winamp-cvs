/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/

#include "main.h"
#include "strutil.h"

char *SkipX(char *str, int count)
{
	while (count--)
	{
		str = CharNext(str);
	}

	return str;
}

wchar_t *SkipXW(wchar_t *str, int count)
{
	while (count--)
	{
		str = CharNextW(str);
	}

	return str;
}

void CopyChar(char *dest, const char *src)
{
	char *end = CharNext(src);
	ptrdiff_t count = end-src;
	while (count--)
	{
		*dest++=*src++;
	}
}

ptrdiff_t CopyCharW(wchar_t *dest, const wchar_t *src)
{
	wchar_t *end = CharNextW(src);
	ptrdiff_t count = end-src;
	for (ptrdiff_t i=0;i<count;i++)
	{
		*dest++=*src++;
	}
	return count;
}

void MakeRelativePathName(const wchar_t *filename, wchar_t *outFile, const wchar_t *path)
{
	wchar_t outPath[MAX_PATH];
	
	int common = PathCommonPrefixW(path, filename, outPath);
	if (common && common == lstrlenW(path))
	{
		PathAddBackslashW(outPath);
		const wchar_t *p = filename + lstrlenW(outPath);
		lstrcpynW(outFile, p, FILENAME_SIZE);
	}
	else if (!PathIsUNCW(filename) && PathIsSameRootW(filename, path))
	{
		lstrcpynW(outFile, filename+2, FILENAME_SIZE);
	}
}

static int CharacterCompareW(const wchar_t *ch1, const wchar_t *ch2)
{
	wchar_t str1[3]={0,0,0}, str2[3]={0,0,0};

	CopyCharW(str1, ch1);
	CharUpperW(str1);

	CopyCharW(str2, ch2);
	CharUpperW(str2);

	return memcmp(str1, str2, 3*sizeof(wchar_t));
}

static void IncHelperW(LPCWSTR *src, ptrdiff_t *size)
{
	wchar_t *end = CharNextW(*src);
	ptrdiff_t count = end-*src;
	*size-=count;
	*src=end;
}

int FileCompareLogical(const wchar_t *str1, const wchar_t *str2)
{
	if (str1 && str2)
	{
		while (*str1)
		{
			if (!*str2)
				return 1;
			else if (IsCharDigitW(*str1))
			{
				int iStr, iComp;
 
				if (!IsCharDigitW(*str2))
					return -1;

				/* Compare the numbers */
				StrToIntExW(str1, 0, &iStr);
				StrToIntExW(str2, 0, &iComp);

				if (iStr < iComp)
					return -1;
				else if (iStr > iComp)
					return 1;

				/* Skip */
				while (IsCharDigitW(*str1))
					str1=CharNextW(str1);
				while (IsCharDigitW(*str2))
					str2=CharNextW(str2);
			}
			else if (IsCharDigitW(*str2))
				return 1;
			else
			{
				int diff = CharacterCompareW(str1, str2);
				if (diff > 0)
					return 1;
				else if (diff < 0)
					return -1;

				str1=CharNextW(str1);
				str2=CharNextW(str2);
			}
		}
		if (*str2)
			return -1;
	}
	return 0;
}

static int StringLengthNoDigits(LPCWSTR str, LPCWSTR *end)
{
	ptrdiff_t length=0;
	while (*str && !IsCharDigitW(*str))
	{
		IncHelperW(&str, &length);
	}
	*end = str;
	return -length; // IncHelper decrements so we need to negate
}

int CompareStringLogical(LPCWSTR str1, LPCWSTR str2)
{
	if (str1 && str2)
	{
		while (*str1)
		{
			if (!*str2)
				return 1;
			else if (IsCharDigitW(*str1))
			{
				int iStr, iComp;
 
				if (!IsCharDigitW(*str2))
					return -1;
 
				/* Compare the numbers */
				StrToIntExW(str1, 0, &iStr);
				StrToIntExW(str2, 0, &iComp);

				if (iStr < iComp)
					return -1;
				else if (iStr > iComp)
					return 1;

				/* Skip */
				while (IsCharDigitW(*str1))
					str1=CharNextW(str1);
				while (IsCharDigitW(*str2))
					str2=CharNextW(str2);
			}
			else if (IsCharDigitW(*str2))
				return 1;
			else
			{
				LPCWSTR next1, next2;
				int len1 = StringLengthNoDigits(str1, &next1);
				int len2 = StringLengthNoDigits(str2, &next2);

				int comp = CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE|NORM_IGNOREWIDTH, str1, len1, str2, len2);
				if (comp == CSTR_LESS_THAN)
					return -1;
				else if (comp == CSTR_GREATER_THAN)
					return 1;

				str1 = next1;
				str2 = next2;
			}
		}
		if (*str2)
			return -1;
	}
	return 0;
}

int FileCompareLogicalN(LPCWSTR str1, ptrdiff_t str1size, LPCWSTR str2,  ptrdiff_t str2size)
{
	if (str1 && str2)
	{
		while (*str1 && str1size)
		{
			if (!*str2 || !str2size)
				return 1;
			else if (IsCharDigitW(*str1))
			{
				int iStr, iComp;
 
				if (!IsCharDigitW(*str2))
					return -1;

				/* Compare the numbers */
				StrToIntExW(str1, 0, &iStr);
				StrToIntExW(str2, 0, &iComp);

				if (iStr < iComp)
					return -1;
				else if (iStr > iComp)
					return 1;

				/* Skip */
				while (IsCharDigitW(*str1))
					IncHelperW(&str1, &str1size);
				while (IsCharDigitW(*str2))
					IncHelperW(&str2, &str2size);
			}
			else if (IsCharDigitW(*str2))
				return 1;
			else
			{
				int diff = CharacterCompareW(str1, str2);
				if (diff > 0)
					return 1;
				else if (diff < 0)
					return -1;

				IncHelperW(&str1, &str1size);
				IncHelperW(&str2, &str2size);
			}
		}

		if (!str1size && !str2size)
			return 0;
		if (*str2 || str2size < str1size)
			return -1;
		if (*str1 || str1size < str2size)
			return 1;
	}
	return 0;
}

char *GetLastCharacter(char *string)
{
	if (!string || !*string)
		return string;

	return CharPrev(string, string+lstrlenA(string));
}

wchar_t *GetLastCharacterW(wchar_t *string)
{
	if (!string || !*string)
		return string;

	return CharPrevW(string, string+lstrlenW(string));
}

const char *GetLastCharacterc(const char *string)
{
	if (!string || !*string)
		return string;

	for (;;)
	{
		const char *next = CharNext(string);
		if (!*next)
			return string;
		string = next;
	}
}

const wchar_t *GetLastCharactercW(const wchar_t *string)
{
	if (!string || !*string)
		return string;

	return CharPrevW(string, string+lstrlenW(string));
}

wchar_t *scanstr_backW(wchar_t *str, wchar_t *toscan, wchar_t *defval)
{
	wchar_t *s = GetLastCharacterW(str);
	if (!str[0]) return defval;
	if (!toscan || !toscan[0]) return defval; 
	for (;;)
	{
		wchar_t *t = toscan;
		while (*t)
		{
			if (*t == *s) return s;
			t = CharNextW(t);
		}
		t = CharPrevW(str, s);
		if (t == s)
			return defval;
		s = t;
	}
}

const wchar_t *scanstr_backcW(const wchar_t *str, const wchar_t *toscan, const wchar_t *defval)
{
	const wchar_t *s = GetLastCharactercW(str);
	if (!str[0]) return defval;
	if (!toscan || !toscan[0]) return defval; 
	for (;;)
	{
		const wchar_t *t = toscan;
		while (*t)
		{
			if (*t == *s) return s;
			t = CharNextW(t);
		}
		t = CharPrevW(str, s);
		if (t == s)
			return defval;
		s = t;
	}
}

char *scanstr_back(char *str, char *toscan, char *defval)
{
	char *s = GetLastCharacter(str);
	if (!str[0]) return defval;
	if (!toscan || !toscan[0]) return defval; 
	for (;;)
	{
		char *t = toscan;
		while (*t)
		{
			if (*t == *s) return s;
			t = CharNext(t);
		}
		t = CharPrev(str, s);
		if (t == s)
			return defval;
		s = t;
	}
}