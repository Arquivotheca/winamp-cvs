#include "./main.h"
#include "./plugin.h"
#include "./supportedExtensions.h"
#include "./fileInfoInterface.h"
#include "./itemTypeInterface.h"
#include "./wasabiApi.h"
#include "./resource.h"
#include "../nu/vector.h"
#include "../playlist/api_playlisthandler.h"
#include "../playlist/api_playlistmanager.h"
#include <api/service/waservicefactorybase.h>
#include <api/service/services.h>

#include <shlwapi.h>
#include <strsafe.h>

#define COMPARE_STR_INVARIANT_I(__str1, __str2)\
	((NULL == (__str1) || NULL == (__str2)) ? ((INT)(ULONG_PTR)((__str1) - (__str2))) :\
	(CompareString(CSTR_INVARIANT, NORM_IGNORECASE, (__str1), -1, (__str2), -1) - 2))


typedef struct _FILETYPEREC
{
	TCHAR		szExtension[8]; // must be first
	IItemType	*type;
	TCHAR		szFamily[96];
} FILETYPEREC;

typedef Vector<FILETYPEREC, 64> SupportedExtList;



__inline static int __cdecl InvariantStringComparer(const void *elem1, const void *elem2)
{
	return COMPARE_STR_INVARIANT_I((LPCTSTR)elem1, (LPCTSTR)elem2);
}

static HANDLE hSupportedExtensions = NULL;


static void CALLBACK UninitializeDefaultExtensions(void)
{
	ReleaseSupportedExtensions(hSupportedExtensions);
	hSupportedExtensions = NULL;
}

static BOOL GetPLExtensionFamily(LPCWSTR pszExt, LPWSTR pszDest, INT cchDest)
{	
	BOOL result(FALSE);
	int n(0);
    waServiceFactory *sf = 0;
	LPCWSTR ext;

	while (NULL != (sf = WASABI_API_SVC->service_enumService(WaSvc::PLAYLISTHANDLER, n++)) && !result)
	{
		api_playlisthandler * handler = static_cast<api_playlisthandler *>(sf->getInterface());
		if (handler)
		{
			int k(0);
			while (NULL != (ext = handler->EnumerateExtensions(k++)))
			{
				if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, pszExt, -1, ext, -1))
				{
					result = (S_OK == StringCchCopyEx(pszDest, cchDest, handler->GetName(), NULL, NULL, STRSAFE_IGNORE_NULLS));
					if (result && CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, pszExt, -1, TEXT("M3U8"), -1)) // ugly...
							result = (S_OK == StringCchCat(pszDest, cchDest, TEXT(" (Unicode)")));
					break;
				}
			}
			sf->releaseInterface(handler);
		}
	}
	return result;
}

static BOOL RegisterTypeExtenstion(SupportedExtList *psel, BYTE typeId, LPCTSTR typeExtension, INT familyResource)
{
	FILETYPEREC ftr;
	ftr.type = PLUGIN_REGTYPES->FindById(IItemType::itemTypeAudioCdTrack);

	if (NULL == ftr.type || 
		FAILED(StringCchCopyW(ftr.szExtension, ARRAYSIZE(ftr.szExtension), typeExtension)))
	{
		return FALSE;
	}
	
	WASABI_API_LNGSTRINGW_BUF(familyResource, ftr.szFamily, ARRAYSIZE(ftr.szFamily));
	
	BOOL replacedOk = FALSE;
	size_t index = psel->size();
	while(index--)
	{
		if (0 == COMPARE_STR_INVARIANT_I(typeExtension, psel->at(index).szExtension))
		{
			psel->at(index) = ftr;
			replacedOk = TRUE;
			break;
		}
	}
	if (!replacedOk)
	psel->push_back(ftr);
	
	return TRUE;
}

static void ReadSupportedExtensions(SupportedExtList *psel)
{
	INT i;
	LPWSTR pszTypes, p;
	FILETYPEREC ftr;

	pszTypes = (LPWSTR)SENDWAIPC(plugin.hwndParent, IPC_GET_EXTLISTW, 0);
	if (pszTypes)
	{
		for(i = 0, p = pszTypes; *p != 0 && *(p+1) != 0; p += lstrlenW(p) + 1, i++)
		{
			if (S_OK == StringCchCopy(ftr.szExtension, ARRAYSIZE(ftr.szExtension), p))
			{
				ftr.type = NULL;
				psel->push_back(ftr);
			}
		}
		GlobalFree(pszTypes);
	}

	if (WASABI_API_SVC)
	{
		api_playlistmanager *plMngr;
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(api_playlistmanagerGUID);
		if (factory) plMngr = (api_playlistmanager*) factory->getInterface();
		if (plMngr)
		{
			ftr.type = PLUGIN_REGTYPES->FindById(IItemType::itemTypePlaylistFile);
			size_t playlistEnum = 0;
			LPCWSTR playlistExt = NULL;
			while (NULL != (playlistExt = plMngr->EnumExtension(playlistEnum++))) 
			{ 
				if (S_OK ==StringCchCopyW(ftr.szExtension, ARRAYSIZE(ftr.szExtension), playlistExt))
				{
					GetPLExtensionFamily(ftr.szExtension, ftr.szFamily, ARRAYSIZE(ftr.szFamily));
					psel->push_back(ftr);
				}
			}
			factory->releaseInterface(plMngr);
		}
	}

	RegisterTypeExtenstion(psel, IItemType::itemTypeLinkFile, L"lnk", IDS_LINKFILE_EXTENSION_FAMILY);
	RegisterTypeExtenstion(psel, IItemType::itemTypeAudioCdTrack, L"cda", IDS_AUDIOCDTRACK_EXTENSION_FAMILY);
}

static IItemType *ReadFileTypeFamilyInfo(FILETYPEREC *pftr)
{
	WCHAR szTest[MAX_PATH], szResult[MAX_PATH];

	if (NULL == pftr)
		return NULL;
	
	pftr->szFamily[0] = TEXT('\0');
	BYTE typeId = IItemType::itemTypeUnknown;
	
	if (S_OK == StringCchPrintfW(szTest, ARRAYSIZE(szTest), L"test.%s", pftr->szExtension))
	{	
		szResult[0] = L'\0';
		extendedFileInfoStructW efis = { szTest, L"type", szResult, ARRAYSIZE(szResult), };
		if (SENDWAIPC(plugin.hwndParent, IPC_GET_EXTENDED_FILE_INFOW, (WPARAM)&efis))
		{
			
			switch(szResult[0])
			{
				case L'0':	typeId = IItemType::itemTypeAudioFile; break;
				case L'1':	typeId = IItemType::itemTypeVideoFile; break;
			}
						
			
			switch(typeId)
			{
				case IItemType::itemTypeAudioFile:
				case IItemType::itemTypeVideoFile:
					extendedFileInfoStructW efis = { szTest, L"family", pftr->szFamily, ARRAYSIZE(pftr->szFamily), };
					SENDWAIPC(plugin.hwndParent, IPC_GET_EXTENDED_FILE_INFOW, (WPARAM)&efis);
					break;
			}

		}
	}
	pftr->type = PLUGIN_REGTYPES->FindById(typeId);
	return pftr->type;
}


HANDLE InitializeSupportedExtensions()
{
	SupportedExtList *psel = new SupportedExtList();
	if (NULL == psel) return NULL;
	
	ReadSupportedExtensions(psel);
	
	if (psel->size() > 0)
		qsort(psel->begin(), psel->size(), sizeof(FILETYPEREC), InvariantStringComparer);
	
	return (HANDLE)psel;
}



void ReleaseSupportedExtensions(HANDLE hSupportedExt)
{	
	if (NULL != hSupportedExt)
	{
		if (hSupportedExt == hSupportedExtensions)
			hSupportedExtensions = NULL;
		
		delete ((SupportedExtList*)hSupportedExt);
	}
}

HANDLE GetDefaultSupportedExtensionsHandle()
{	
	if (NULL == hSupportedExtensions)
	{
		hSupportedExtensions = InitializeSupportedExtensions();
		if (NULL != hSupportedExtensions)
			Plugin_RegisterUnloadCallback(UninitializeDefaultExtensions);
	}
	return hSupportedExtensions;
}

IItemType *GetTypeByExtension(HANDLE hSupportedExt, LPCTSTR pszExtension, LPCTSTR *ppszFamily)
{
	FILETYPEREC *pftr = NULL;
	if (NULL != hSupportedExt)
	{
		SupportedExtList *psel = (SupportedExtList*)hSupportedExt;
		pftr = (FILETYPEREC*)bsearch(pszExtension, psel->begin(), psel->size(), sizeof(FILETYPEREC), InvariantStringComparer);
		if (NULL != pftr && NULL == pftr->type)
			ReadFileTypeFamilyInfo(pftr);
	}
		
	if (NULL != ppszFamily)
		*ppszFamily = (NULL != pftr) ? pftr->szFamily : TEXT("");
	
	return (NULL != pftr) ? pftr->type : NULL;

}
