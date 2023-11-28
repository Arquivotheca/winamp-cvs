#include "nscue.h"
#ifdef __linux__
#include <stdio.h>

int main(int argc, char **argv)
{
	CueSheet *sheet;
	int err=CueSheet_Open(argv[1], &sheet);
	if (err == CUESHEET_OK)
	{
		printf("Successfully parsed %s\n", argv[1]);
		CueSheet_Close(sheet);
	}
	else
	{
		printf("Error parsing %s\nError = %d\n", argv[1], err);
		CueSheet_Close(sheet);
	}
	return 0;
}
#else
#include <stdio.h>
#include <windows.h>
#include <shlwapi.h>
unsigned int numsheets = 0;
void TestCueSheet(const wchar_t *filename)
{
	CueSheet *sheet;
	int err=CueSheet_Open(filename, &sheet);
	if (err == CUESHEET_OK)
	{
		numsheets++;
		wprintf(L"Successfully parsed %s\n", filename);
		CueSheet_Close(sheet);
	}
	else
	{
		wprintf(L"Error parsing %s\nError = %d\n", filename, err);
		CueSheet_Close(sheet);
		exit(0);
	}
}

void SearchDir(const wchar_t *dir)
{
	wchar_t filemask[MAX_PATH];
	wchar_t filename[MAX_PATH];
	PathCombine(filemask, dir, L"*");

	WIN32_FIND_DATA data;
	HANDLE h = FindFirstFile(filemask, &data);
	if (h != INVALID_HANDLE_VALUE) 
	{
		do 
		{
			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (wcscmp(data.cFileName, L".") && wcscmp(data.cFileName, L".."))
				{
					PathCombine(filename, dir, data.cFileName);
					SearchDir(filename);
				}
			}
			else if (!_wcsicmp(PathFindExtension(data.cFileName), L".cue"))
			{
				PathCombine(filename, dir, data.cFileName);
				TestCueSheet(filename);
			}
		} while (FindNextFile(h,&data));
	}
}

int wmain(int argc, wchar_t **argv)
{
	SearchDir(argv[1]);
	printf("%u CUE sheets passed.\n", numsheets);
}

#endif