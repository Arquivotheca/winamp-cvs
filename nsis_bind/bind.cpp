#include <windows.h>
#include <shlwapi.h>
#include <imagehlp.h>
#include "exdll.h"
#include "../nu/AutoCharFn.h"

#define RECURSE_LIMIT 10
static int recurse=0;
static void BindRecursive(const char *path, const char *root_path /*for finding DLLs*/)
{
	if (recurse >= RECURSE_LIMIT)
		return;
	recurse++;
	char mask[MAX_PATH];
	char file[MAX_PATH];
	WIN32_FIND_DATAA f;
	PathCombineA(mask, path, "*");
	HANDLE hFind = FindFirstFileA(mask, &f);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) // if it's a directory
				&& lstrcmpA(f.cFileName, ".") && lstrcmpA(f.cFileName, "..") // but not . or ..
				)
			{
				PathCombineA(file, path, f.cAlternateFileName[0]?f.cAlternateFileName:f.cFileName);
				BindRecursive(file, root_path);
			}
			else if (!(f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				char *ext = PathFindExtensionA(f.cFileName);
				if (!lstrcmpiA(ext, ".exe") || !lstrcmpiA(ext, ".dll") || !lstrcmpiA(ext,
					".w5s") || !lstrcmpiA(ext, ".wac"))
				{
					PathCombineA(file, path,
						f.cAlternateFileName[0]?f.cAlternateFileName:f.cFileName);
					BindImageEx(BIND_ALL_IMAGES|BIND_CACHE_IMPORT_DLLS, 
						file, 0, 0, 0);
				}
			}
		} while (FindNextFileA(hFind, &f));
		recurse--;
	}
}


static stack_t *pop(stack_t **stacktop)
{
	stack_t *th;
	if (!stacktop || !*stacktop) 
		return 0;

	th=*stacktop;
	*stacktop = th->next;
	return th;
}

extern "C"
{
	void __declspec(dllexport) add(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop,	struct extra_parameters *extra)
	{
		stack_t *root_path = *stacktop;
		AutoCharFn root_pathA(root_path->text);
		BindRecursive(root_pathA, root_pathA);
	}

	BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
	{
		return TRUE;
	}
}