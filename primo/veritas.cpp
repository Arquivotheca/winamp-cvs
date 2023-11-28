#include "veritas.h"
#include "PrimoSDK.h"


PrimoObject::PrimoObject()
{
	handle=0;
	DWORD ret = PrimoSDK_GetHandle(&handle);
	if (ret != PRIMOSDK_OK)
		handle=0;
}


PrimoObject::~PrimoObject()
{
	if (handle)
	{
		PrimoSDK_ReleaseHandle(handle);
		handle=0;
	}
}

DWORD PrimoObject::RunningStatus(DWORD dwFlags, PDWORD pdwCurSector, PDWORD pdwTotSector)
{
	return PrimoSDK_RunningStatus(handle, dwFlags, pdwCurSector, pdwTotSector);
}


DWORD PrimoObject::UnitLock(PDWORD pdwUnit, DWORD dwFlags)
{
	return PrimoSDK_UnitLock(handle, pdwUnit, dwFlags);
}


DWORD PrimoObject::UnitAIN(PDWORD pdwUnit, DWORD dwFlags)
{
	return PrimoSDK_UnitAIN(handle, pdwUnit, dwFlags);
}


DWORD PrimoObject::UnitVxBlock(PDWORD pdwUnit, DWORD dwFlags, PBYTE szAppName)
{
	return PrimoSDK_UnitVxBlock(handle, pdwUnit, dwFlags, szAppName);
}


DWORD PrimoObject::UnitInfo(PDWORD pdwUnit, PDWORD pdwType, PBYTE szDescr, PDWORD pdwReady)
{
	return PrimoSDK_UnitInfo(handle,  pdwUnit,  pdwType,  szDescr,  pdwReady);
}


DWORD PrimoObject::UnitSpeeds(PDWORD pdwUnit, PDWORD pdwCDSpeeds, PDWORD pdwDVDSpeeds, PDWORD pdwCapabilities)
{
	return PrimoSDK_UnitSpeeds(handle,  pdwUnit,  pdwCDSpeeds,  pdwDVDSpeeds,  pdwCapabilities);
}


DWORD PrimoObject::NextExtractAudioBuffer(PBYTE pBuffer, DWORD dwBufSize, PDWORD pExpectedSize, PDWORD pTargetSector)
{
	return PrimoSDK_NextExtractAudioBuffer(handle,  pBuffer,  dwBufSize,  pExpectedSize,  pTargetSector);
}


DWORD PrimoObject::ExtractAudioToBuffer(PDWORD pdwUnit, DWORD dwStartSector, DWORD dwTotalSectors, DWORD dwReadSpeed, DWORD dwReserved1, DWORD dwReserved2, DWORD dwReserved3)
{
	return PrimoSDK_ExtractAudioToBuffer(handle,  pdwUnit,  dwStartSector,  dwTotalSectors,  dwReadSpeed,  dwReserved1,  dwReserved2,  dwReserved3);
}


DWORD PrimoObject::UnitInfo2(PDWORD pdwUnit, PDWORD pdwTypes, PDWORD pdwClass, PDWORD pdwBusType, PDWORD pdwRFU)
{
	return PrimoSDK_UnitInfo2(handle,  pdwUnit,  pdwTypes,  pdwClass,  pdwBusType,  pdwRFU);
}


DWORD PrimoObject::DiscInfo(PDWORD pdwUnit, PDWORD pdwMediumType, PDWORD pdwMediumFormat, PDWORD pdwErasable, PDWORD pdwTracks, PDWORD pdwUsed, PDWORD pdwFree)
{
	return PrimoSDK_DiscInfo(handle,  pdwUnit,  pdwMediumType,  pdwMediumFormat,  pdwErasable,  pdwTracks,  pdwUsed,  pdwFree);
}


DWORD PrimoObject::TrackInfo(DWORD dwTrackNumber, PDWORD pdwSessionNumber, PDWORD pdwTrackType, PDWORD pdwPreGap, PDWORD pdwStart, PDWORD pdwLength)
{
	return PrimoSDK_TrackInfo(handle,  dwTrackNumber,  pdwSessionNumber,  pdwTrackType,  pdwPreGap,  pdwStart,  pdwLength);
}


DWORD PrimoObject::ListSupportedUnits(PBYTE szList)
{
	return PrimoSDK_ListSupportedUnits(szList);
}


DWORD PrimoObject::UnitStatus(PDWORD pdwUnit, PDWORD pdwCommand, PDWORD pdwSense, PDWORD pdwASC, PDWORD pdwASCQ)
{
	return PrimoSDK_UnitStatus(handle,  pdwUnit,  pdwCommand,  pdwSense,  pdwASC,  pdwASCQ);
}


DWORD PrimoObject::DiscInfoEx(PDWORD pdwUnit, DWORD dwFlags, PDWORD pdwMediumType, PDWORD pdwMediumFormat, PDWORD pdwErasable, PDWORD pdwTracks, PDWORD pdwUsed, PDWORD pdwFree)
{
	return PrimoSDK_DiscInfoEx(handle,  pdwUnit,  dwFlags,  pdwMediumType,  pdwMediumFormat,  pdwErasable,  pdwTracks,  pdwUsed,  pdwFree);
}


DWORD PrimoObject::DiscInfo2(PDWORD pdwUnit, PDWORD pdwMedium, PDWORD pdwProtectedDVD, PDWORD pdwFlags, PDWORD pdwMediumEx, PDWORD pdwRFU3)
{
	return PrimoSDK_DiscInfo2(handle,  pdwUnit,  pdwMedium,  pdwProtectedDVD,  pdwFlags,  pdwMediumEx,  pdwRFU3);
}


DWORD PrimoObject::CDTextInfoEJ(PDWORD pdwUnit, PBYTE szTitleE, PBYTE szPerformerE, PBYTE szComposerE, PBYTE szTitleJ, PBYTE szPerformerJ, PBYTE szComposerJ)
{
	return PrimoSDK_CDTextInfoEJ(handle,  pdwUnit,  szTitleE,  szPerformerE,  szComposerE,  szTitleJ,  szPerformerJ,  szComposerJ);
}


DWORD PrimoObject::CloseImage()
{
	return PrimoSDK_CloseImage(handle);
}


DWORD PrimoObject::NewImageWCS(PDWORD pdwUnits, PWORD szVolumeNameWCS, DWORD dwTrackToLoad, DWORD dwFlags, DWORD dwSwapThreshold, PBYTE szTemp)
{
	return PrimoSDK_NewImageWCS(handle,  pdwUnits,  szVolumeNameWCS,  dwTrackToLoad,  dwFlags,  dwSwapThreshold,  szTemp);
}


DWORD PrimoObject::AddFolderWCS(const wchar_t *szFolder)
{
	return PrimoSDK_AddFolderWCS(handle, (wchar_t*)szFolder);
}


DWORD PrimoObject::AddFileWCS(PWORD szFileOnCD, PWORD szSourceFile)
{
	return PrimoSDK_AddFileWCS(handle,  szFileOnCD,  szSourceFile);
}

DWORD PrimoObject::SaveImage(PBYTE szFileName, PDWORD pdwSize)
{
	return PrimoSDK_SaveImage(handle, szFileName, pdwSize);
}

DWORD PrimoObject::WriteOtherCDImage(PDWORD pdwUnits, PBYTE szFileName, DWORD dwFlags, DWORD dwSpeed)
{
	return PrimoSDK_WriteOtherCDImage(handle, pdwUnits,  szFileName,  dwFlags,  dwSpeed);
}

DWORD PrimoObject::WriteImage(DWORD dwFlags, DWORD dwSpeed, PDWORD pdwSize)
{
	return PrimoSDK_WriteImage(handle, dwFlags, dwSpeed, pdwSize);
}

DWORD PrimoObject::UnitReady(PDWORD pdwUnit)
{
	return PrimoSDK_UnitReady(handle, pdwUnit);
}

DWORD PrimoObject::EraseMedium(PDWORD pdwUnit, DWORD dwFlags)
{
	return PrimoSDK_EraseMedium(handle, pdwUnit, dwFlags);
}

DWORD PrimoObject::MoveMedium(PDWORD pdwUnit, DWORD dwFlags)
{
	return PrimoSDK_MoveMedium(handle, pdwUnit, dwFlags);
}

DWORD PrimoObject::AddAudioStream(PrimoSDK_StreamCallback pFillerFn, PVOID pContext, DWORD dwPreGap, DWORD dwSize)
{
	return PrimoSDK_AddAudioStream(handle, pFillerFn, pContext, dwPreGap, dwSize);
}

DWORD PrimoObject::GetDiscSpeed(PDWORD pdwUnit, DWORD dwRequestedSpeed100thX, LPDWORD pdwGottenSpeed100thX)
{
		return PrimoSDK_GetDiscSpeed(handle, pdwUnit, dwRequestedSpeed100thX, pdwGottenSpeed100thX);
}

DWORD PrimoObject::NewAudio(PDWORD pdwUnits)
{
	return PrimoSDK_NewAudio(handle, pdwUnits);
}

DWORD PrimoObject::CloseAudio()
{
		return PrimoSDK_CloseAudio(handle);
}

DWORD PrimoObject::WriteAudioEx(DWORD dwFlags, DWORD dwSpeed, PBYTE pMCN)
{
	return PrimoSDK_WriteAudioEx(handle,  dwFlags, dwSpeed, pMCN);
}

#define CBCLASS PrimoObject
START_DISPATCH;
CB(PRIMO_RunningStatus, RunningStatus)
CB(PRIMO_UnitLock, UnitLock)
CB(PRIMO_UnitAIN, UnitAIN)
CB(PRIMO_UnitVxBlock, UnitVxBlock)
CB(PRIMO_UnitInfo, UnitInfo)
CB(PRIMO_UnitSpeeds, UnitSpeeds)
CB(PRIMO_NextExtractAudioBuffer, NextExtractAudioBuffer)
CB(PRIMO_ExtractAudioToBuffer, ExtractAudioToBuffer)
CB(PRIMO_UnitInfo2, UnitInfo2)
CB(PRIMO_DiscInfo, DiscInfo)
CB(PRIMO_TrackInfo, TrackInfo)
CB(PRIMO_ListSupportedUnits, ListSupportedUnits)
CB(PRIMO_UnitStatus, UnitStatus)
CB(PRIMO_DiscInfoEx, DiscInfoEx)
CB(PRIMO_DiscInfo2, DiscInfo2)
CB(PRIMO_CDTextInfoEJ, CDTextInfoEJ)
CB(PRIMO_CloseImage, CloseImage)
CB(PRIMO_NewImageWCS, NewImageWCS)
CB(PRIMO_AddFolderWCS, AddFolderWCS)
CB(PRIMO_AddFileWCS, AddFileWCS)
CB(PRIMO_SaveImage, SaveImage)
CB(PRIMO_WriteOtherCDImage, WriteOtherCDImage)
CB(PRIMO_WriteImage, WriteImage)
CB(PRIMO_UnitReady, UnitReady)
CB(PRIMO_EraseMedium, EraseMedium)
CB(PRIMO_MoveMedium, MoveMedium)
CB(PRIMO_AddAudioStream, AddAudioStream)
CB(PRIMO_GetDiscSpeed, GetDiscSpeed)
CB(PRIMO_NewAudio, NewAudio)
CB(PRIMO_CloseAudio, CloseAudio)
CB(PRIMO_WriteAudioEx, WriteAudioEx)
END_DISPATCH;
#undef CBCLASS
