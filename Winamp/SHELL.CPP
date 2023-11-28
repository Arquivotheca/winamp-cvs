/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#include "main.h"
#include "../nu/ns_wc.h"
extern "C"
{

	HRESULT ResolveShortCut(HWND hwnd, LPCWSTR pszShortcutFile, LPWSTR pszPath)
	{
		HRESULT hres;
		IShellLinkW *psl;
		wchar_t szGotPath[MAX_PATH];
		WIN32_FIND_DATAW wfd;

		*pszPath = 0;   // assume failure

		hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
		                        IID_IShellLinkW, (void **) & psl);
		if (SUCCEEDED(hres))
		{
			IPersistFile *ppf;

			hres = psl->QueryInterface(IID_IPersistFile, (void **) & ppf); // OLE 2!  Yay! --YO
			if (SUCCEEDED(hres))
			{
				hres = ppf->Load(pszShortcutFile, STGM_READ);
				if (SUCCEEDED(hres))
				{
					hres = psl->Resolve(hwnd, SLR_ANY_MATCH);
					if (SUCCEEDED(hres))
					{
						StringCchCopyW(szGotPath, MAX_PATH, pszShortcutFile);
						hres = psl->GetPath(szGotPath, MAX_PATH, & wfd,
						                    SLGP_SHORTPATH );
						StringCchCopyW(pszPath, MAX_PATH, szGotPath);
					}
				}
				ppf->Release();
			}
			psl->Release();
		}
		return SUCCEEDED(hres);
	}

	/*void CreateShortCut(HWND hwnd, LPCSTR pszShortcutFile, LPCSTR pszExe, LPCSTR pszArg, int iconindex)
	{
		HRESULT hres;
		IShellLink* psl;

		hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
		                        IID_IShellLink, (void **) & psl);
		if (SUCCEEDED(hres))
		{
			IPersistFile* ppf;

			hres = psl->QueryInterface(IID_IPersistFile, (void **) & ppf); // OLE 2!  Yay! --YO
			if (SUCCEEDED(hres))
			{
				wchar_t wsz[MAX_PATH];
				MultiByteToWideCharSZ(CP_ACP, 0, pszShortcutFile, -1, wsz, MAX_PATH);

				hres = psl->SetPath(pszExe);
				if (pszArg)
				{
					psl->SetArguments(pszArg);
				}
				psl->SetIconLocation(pszExe, iconindex);

				if (SUCCEEDED(hres))
				{
					ppf->Save(wsz, TRUE);
				}
				ppf->Release();
			}
			psl->Release();
		}
	}*/

	void Shell_Free(void *p)
	{
		CoTaskMemFree(p);
	}
}