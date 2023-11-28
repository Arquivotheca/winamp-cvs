#include "./main.h"
#include "./fiAudioCdTrack.h"
#include "./itemTypeInterface.h"
#include <strsafe.h>

AudioCdTrackInfo::AudioCdTrackInfo(LPCTSTR pszFilePath, WIN32_FILE_ATTRIBUTE_DATA *pAttributeData)
	: AudioFileInfo(pszFilePath, pAttributeData)
	
{	
	TCHAR szCdaMarker[] = TEXT("cda://");
	if (NULL != pszPath)
	{
		INT cchLen = lstrlen(pszPath);
		if (cchLen > 8 && CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, 
					szCdaMarker, ARRAYSIZE(szCdaMarker) -1, pszPath, ARRAYSIZE(szCdaMarker) - 1))
		{ //convert back to windows format :)
			TCHAR szTemp[MAX_PATH];
			WCHAR driveLetter = pszPath[6];
			int trackNum = _wtoi(&pszPath[8]);
			if (trackNum > 0 && 
				S_OK == StringCchPrintf(szTemp, ARRAYSIZE(szTemp), TEXT("%c:\\Track%02d.cda"), CharUpper((LPTSTR)driveLetter), trackNum))
			{
				free(pszPath);
				INT cbLen = (lstrlen(szTemp) + 1) * sizeof(TCHAR);
				pszPath = (LPTSTR)lfh_malloc(cbLen);
				if (NULL != pszPath)
					CopyMemory(pszPath, szTemp, cbLen);
			}
			
		}

	}
}

AudioCdTrackInfo::~AudioCdTrackInfo()
{	
}

STDMETHODIMP AudioCdTrackInfo::CreateInstance(LPCTSTR filePath, WIN32_FILE_ATTRIBUTE_DATA *attributes, IFileInfo **itemOut)
{
	*itemOut = new AudioCdTrackInfo(filePath, attributes);
	return  (NULL != *itemOut) ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP AudioCdTrackInfo::GetType(DWORD *pType)
{
	if (NULL == pType)
		return E_POINTER;
	*pType = IItemType::itemTypeAudioCdTrack;
	
	return S_OK;
}