#ifndef NULLSOFT_PRIMO_OBJ_PRIMO_H
#define NULLSOFT_PRIMO_OBJ_PRIMO_H
#include <windows.h>
#include <bfc/dispatch.h>
#include "PrimoSDK.h"
#include <api/service/services.h>

// {C44425B0-E3B2-4963-90ED-1B976E262867}
static const GUID obj_primoGUID = 
{ 0xc44425b0, 0xe3b2, 0x4963, { 0x90, 0xed, 0x1b, 0x97, 0x6e, 0x26, 0x28, 0x67 } };

class obj_primo : public Dispatchable
{
protected:
	obj_primo(){}
	~obj_primo(){}
public:
	static GUID getServiceGuid() { return obj_primoGUID; }
	static const char *getServiceName() { return "Primo Object"; }
	static FOURCC getServiceType() { return WaSvc::OBJECT; }
	DWORD RunningStatus(DWORD dwFlags, PDWORD pdwCurSector, PDWORD pdwTotSector);
	DWORD UnitLock(PDWORD pdwUnit, DWORD dwFlags);
	DWORD UnitAIN(PDWORD pdwUnit, DWORD dwFlags);
	DWORD UnitVxBlock(PDWORD pdwUnit, DWORD dwFlags, PBYTE szAppName);
	DWORD UnitInfo(PDWORD pdwUnit, PDWORD pdwType, PBYTE szDescr, PDWORD pdwReady);
	DWORD UnitSpeeds(PDWORD pdwUnit, PDWORD pdwCDSpeeds, PDWORD pdwDVDSpeeds, PDWORD pdwCapabilities);
	DWORD NextExtractAudioBuffer(PBYTE pBuffer, DWORD dwBufSize, PDWORD pExpectedSize, PDWORD pTargetSector);
	DWORD ExtractAudioToBuffer(PDWORD pdwUnit, DWORD dwStartSector, DWORD dwTotalSectors, DWORD dwReadSpeed, DWORD dwReserved1, DWORD dwReserved2, DWORD dwReserved3);
	DWORD UnitInfo2(PDWORD pdwUnit, PDWORD pdwTypes, PDWORD pdwClass, PDWORD pdwBusType, PDWORD pdwRFU);
	DWORD DiscInfo(PDWORD pdwUnit, PDWORD pdwMediumType, PDWORD pdwMediumFormat, PDWORD pdwErasable, PDWORD pdwTracks, PDWORD pdwUsed, PDWORD pdwFree);
	DWORD TrackInfo(DWORD dwTrackNumber, PDWORD pdwSessionNumber, PDWORD pdwTrackType, PDWORD pdwPreGap, PDWORD pdwStart, PDWORD pdwLength);
	DWORD ListSupportedUnits(PBYTE szList);
	DWORD UnitStatus(PDWORD pdwUnit, PDWORD pdwCommand, PDWORD pdwSense, PDWORD pdwASC, PDWORD pdwASCQ);
	DWORD DiscInfoEx(PDWORD pdwUnit, DWORD dwFlags, PDWORD pdwMediumType, PDWORD pdwMediumFormat, PDWORD pdwErasable, PDWORD pdwTracks, PDWORD pdwUsed, PDWORD pdwFree);
	DWORD DiscInfo2(PDWORD pdwUnit, PDWORD pdwMedium, PDWORD pdwProtectedDVD, PDWORD pdwFlags, PDWORD pdwMediumEx, PDWORD pdwRFU3);
	DWORD CDTextInfoEJ(PDWORD pdwUnit, PBYTE szTitleE, PBYTE szPerformerE, PBYTE szComposerE, PBYTE szTitleJ, PBYTE szPerformerJ, PBYTE szComposerJ);
	DWORD CloseImage();
	DWORD NewImageWCS(PDWORD pdwUnits, PWORD szVolumeNameWCS, DWORD dwTrackToLoad, DWORD dwFlags, DWORD dwSwapThreshold, PBYTE szTemp);
	DWORD AddFolderWCS(const wchar_t *szFolder);
	DWORD AddFileWCS(PWORD szFileOnCD, PWORD szSourceFile);
	DWORD SaveImage(PBYTE szFileName, PDWORD pdwSize);
	DWORD WriteOtherCDImage(PDWORD pdwUnits, PBYTE szFileName, DWORD dwFlags, DWORD dwSpeed);
	DWORD WriteImage(DWORD dwFlags, DWORD dwSpeed, PDWORD pdwSize);
	DWORD UnitReady(PDWORD pdwUnit);
	DWORD EraseMedium(PDWORD pdwUnit, DWORD dwFlags);
	DWORD MoveMedium(PDWORD pdwUnit, DWORD dwFlags);
	DWORD AddAudioStream(PrimoSDK_StreamCallback pFillerFn, PVOID pContext, DWORD dwPreGap, DWORD dwSize);
	DWORD GetDiscSpeed(PDWORD pdwUnit, DWORD dwRequestedSpeed100thX, LPDWORD pdwGottenSpeed100thX);
	DWORD NewAudio(PDWORD pdwUnits);
DWORD CloseAudio();
DWORD WriteAudioEx(DWORD dwFlags, DWORD dwSpeed, PBYTE pMCN);
	enum
	{
		PRIMO_RunningStatus = 0,
		PRIMO_UnitLock = 1,
		PRIMO_UnitAIN = 2,
		PRIMO_UnitVxBlock = 3,
		PRIMO_UnitInfo = 4,
		PRIMO_UnitSpeeds = 5,
		PRIMO_NextExtractAudioBuffer = 6,
		PRIMO_ExtractAudioToBuffer = 7,
		PRIMO_UnitInfo2 = 8,
		PRIMO_DiscInfo = 9,
		PRIMO_TrackInfo = 10,
		PRIMO_ListSupportedUnits = 11,
		PRIMO_UnitStatus = 12,
		PRIMO_DiscInfoEx = 13,
		PRIMO_DiscInfo2 = 14,
		PRIMO_CDTextInfoEJ = 15,
		PRIMO_CloseImage = 16,
		PRIMO_NewImageWCS = 17,
		PRIMO_AddFolderWCS = 18,
		PRIMO_AddFileWCS = 19,
		PRIMO_SaveImage = 20,
		PRIMO_WriteOtherCDImage = 21,
		PRIMO_WriteImage = 22,
		PRIMO_UnitReady = 23,
		PRIMO_EraseMedium = 24,
		PRIMO_MoveMedium = 25,
		PRIMO_AddAudioStream = 26,
		PRIMO_GetDiscSpeed = 27,
		PRIMO_NewAudio = 28,
		PRIMO_CloseAudio = 29,
		PRIMO_WriteAudioEx = 30,
	};
};

inline DWORD obj_primo::RunningStatus(DWORD dwFlags, PDWORD pdwCurSector, PDWORD pdwTotSector)
{
	return _call(PRIMO_RunningStatus, (DWORD)PRIMOSDK_INTERR, dwFlags, pdwCurSector, pdwTotSector);
}


inline DWORD obj_primo::UnitLock(PDWORD pdwUnit, DWORD dwFlags)
{
	return _call(PRIMO_UnitLock, (DWORD)PRIMOSDK_INTERR, pdwUnit, dwFlags);
}


inline DWORD obj_primo::UnitAIN(PDWORD pdwUnit, DWORD dwFlags)
{
	return _call(PRIMO_UnitAIN, (DWORD)PRIMOSDK_INTERR, pdwUnit, dwFlags);
}


inline DWORD obj_primo::UnitVxBlock(PDWORD pdwUnit, DWORD dwFlags, PBYTE szAppName)
{
	return _call(PRIMO_UnitVxBlock, (DWORD)PRIMOSDK_INTERR, pdwUnit, dwFlags, szAppName);
}


inline DWORD obj_primo::UnitInfo(PDWORD pdwUnit, PDWORD pdwType, PBYTE szDescr, PDWORD pdwReady)
{
	return _call(PRIMO_UnitInfo, (DWORD)PRIMOSDK_INTERR,  pdwUnit,  pdwType,  szDescr,  pdwReady);
}


inline DWORD obj_primo::UnitSpeeds(PDWORD pdwUnit, PDWORD pdwCDSpeeds, PDWORD pdwDVDSpeeds, PDWORD pdwCapabilities)
{
	return _call(PRIMO_UnitSpeeds, (DWORD)PRIMOSDK_INTERR,  pdwUnit,  pdwCDSpeeds,  pdwDVDSpeeds,  pdwCapabilities);
}


inline DWORD obj_primo::NextExtractAudioBuffer(PBYTE pBuffer, DWORD dwBufSize, PDWORD pExpectedSize, PDWORD pTargetSector)
{
	return _call(PRIMO_NextExtractAudioBuffer, (DWORD)PRIMOSDK_INTERR,  pBuffer,  dwBufSize,  pExpectedSize,  pTargetSector);
}


inline DWORD obj_primo::ExtractAudioToBuffer(PDWORD pdwUnit, DWORD dwStartSector, DWORD dwTotalSectors, DWORD dwReadSpeed, DWORD dwReserved1, DWORD dwReserved2, DWORD dwReserved3)
{
	return _call(PRIMO_ExtractAudioToBuffer, (DWORD)PRIMOSDK_INTERR,  pdwUnit,  dwStartSector,  dwTotalSectors,  dwReadSpeed,  dwReserved1,  dwReserved2,  dwReserved3);
}


inline DWORD obj_primo::UnitInfo2(PDWORD pdwUnit, PDWORD pdwTypes, PDWORD pdwClass, PDWORD pdwBusType, PDWORD pdwRFU)
{
	return _call(PRIMO_UnitInfo2, (DWORD)PRIMOSDK_INTERR,  pdwUnit,  pdwTypes,  pdwClass,  pdwBusType,  pdwRFU);
}


inline DWORD obj_primo::DiscInfo(PDWORD pdwUnit, PDWORD pdwMediumType, PDWORD pdwMediumFormat, PDWORD pdwErasable, PDWORD pdwTracks, PDWORD pdwUsed, PDWORD pdwFree)
{
	return _call(PRIMO_DiscInfo, (DWORD)PRIMOSDK_INTERR,  pdwUnit,  pdwMediumType,  pdwMediumFormat,  pdwErasable,  pdwTracks,  pdwUsed,  pdwFree);
}


inline DWORD obj_primo::TrackInfo(DWORD dwTrackNumber, PDWORD pdwSessionNumber, PDWORD pdwTrackType, PDWORD pdwPreGap, PDWORD pdwStart, PDWORD pdwLength)
{
	return _call(PRIMO_TrackInfo, (DWORD)PRIMOSDK_INTERR,  dwTrackNumber,  pdwSessionNumber,  pdwTrackType,  pdwPreGap,  pdwStart,  pdwLength);
}


inline DWORD obj_primo::ListSupportedUnits(PBYTE szList)
{
	return _call(PRIMO_ListSupportedUnits, (DWORD)PRIMOSDK_INTERR, szList);
}


inline DWORD obj_primo::UnitStatus(PDWORD pdwUnit, PDWORD pdwCommand, PDWORD pdwSense, PDWORD pdwASC, PDWORD pdwASCQ)
{
	return _call(PRIMO_UnitStatus, (DWORD)PRIMOSDK_INTERR,  pdwUnit,  pdwCommand,  pdwSense,  pdwASC,  pdwASCQ);
}


inline DWORD obj_primo::DiscInfoEx(PDWORD pdwUnit, DWORD dwFlags, PDWORD pdwMediumType, PDWORD pdwMediumFormat, PDWORD pdwErasable, PDWORD pdwTracks, PDWORD pdwUsed, PDWORD pdwFree)
{
	return _call(PRIMO_DiscInfoEx, (DWORD)PRIMOSDK_INTERR,  pdwUnit,  dwFlags,  pdwMediumType,  pdwMediumFormat,  pdwErasable,  pdwTracks,  pdwUsed,  pdwFree);
}


inline DWORD obj_primo::DiscInfo2(PDWORD pdwUnit, PDWORD pdwMedium, PDWORD pdwProtectedDVD, PDWORD pdwFlags, PDWORD pdwMediumEx, PDWORD pdwRFU3)
{
	return _call(PRIMO_DiscInfo2, (DWORD)PRIMOSDK_INTERR,  pdwUnit,  pdwMedium,  pdwProtectedDVD,  pdwFlags,  pdwMediumEx,  pdwRFU3);
}


inline DWORD obj_primo::CDTextInfoEJ(PDWORD pdwUnit, PBYTE szTitleE, PBYTE szPerformerE, PBYTE szComposerE, PBYTE szTitleJ, PBYTE szPerformerJ, PBYTE szComposerJ)
{
	return _call(PRIMO_CDTextInfoEJ, (DWORD)PRIMOSDK_INTERR,  pdwUnit,  szTitleE,  szPerformerE,  szComposerE,  szTitleJ,  szPerformerJ,  szComposerJ);
}


inline DWORD obj_primo::CloseImage()
{
	return _call(PRIMO_CloseImage, (DWORD)PRIMOSDK_INTERR);
}


inline DWORD obj_primo::NewImageWCS(PDWORD pdwUnits, PWORD szVolumeNameWCS, DWORD dwTrackToLoad, DWORD dwFlags, DWORD dwSwapThreshold, PBYTE szTemp)
{
	return _call(PRIMO_NewImageWCS, (DWORD)PRIMOSDK_INTERR,  pdwUnits,  szVolumeNameWCS,  dwTrackToLoad,  dwFlags,  dwSwapThreshold,  szTemp);
}


inline DWORD obj_primo::AddFolderWCS(const wchar_t *szFolder)
{
	return _call(PRIMO_AddFolderWCS, (DWORD)PRIMOSDK_INTERR, szFolder);
}


inline DWORD obj_primo::AddFileWCS(PWORD szFileOnCD, PWORD szSourceFile)
{
	return _call(PRIMO_AddFileWCS, (DWORD)PRIMOSDK_INTERR,  szFileOnCD,  szSourceFile);
}

inline DWORD obj_primo::SaveImage(PBYTE szFileName, PDWORD pdwSize)
{
	return _call(PRIMO_SaveImage, (DWORD)PRIMOSDK_INTERR,  szFileName,  pdwSize);
}

inline DWORD obj_primo::WriteOtherCDImage(PDWORD pdwUnits, PBYTE szFileName, DWORD dwFlags, DWORD dwSpeed)
{
	return _call(PRIMO_WriteOtherCDImage, (DWORD)PRIMOSDK_INTERR,   pdwUnits,  szFileName,  dwFlags,  dwSpeed);
}

inline DWORD obj_primo::WriteImage(DWORD dwFlags, DWORD dwSpeed, PDWORD pdwSize)
{
	return _call(PRIMO_WriteImage, (DWORD)PRIMOSDK_INTERR,   dwFlags,  dwSpeed,  pdwSize);
}

inline DWORD obj_primo::UnitReady(PDWORD pdwUnit)
{
	return _call(PRIMO_UnitReady, (DWORD)PRIMOSDK_INTERR,   pdwUnit);
}

inline DWORD obj_primo::EraseMedium(PDWORD pdwUnit, DWORD dwFlags)
{
	return _call(PRIMO_EraseMedium, (DWORD)PRIMOSDK_INTERR,   pdwUnit, dwFlags);
}

inline DWORD obj_primo::MoveMedium(PDWORD pdwUnit, DWORD dwFlags)
{
		return _call(PRIMO_MoveMedium, (DWORD)PRIMOSDK_INTERR,   pdwUnit, dwFlags);
}

inline DWORD obj_primo::AddAudioStream(PrimoSDK_StreamCallback pFillerFn, PVOID pContext, DWORD dwPreGap, DWORD dwSize)
{
			return _call(PRIMO_AddAudioStream, (DWORD)PRIMOSDK_INTERR,  pFillerFn,  pContext,  dwPreGap,  dwSize);
}

inline DWORD obj_primo::GetDiscSpeed(PDWORD pdwUnit, DWORD dwRequestedSpeed100thX, LPDWORD pdwGottenSpeed100thX)
{
				return _call(PRIMO_GetDiscSpeed, (DWORD)PRIMOSDK_INTERR,  pdwUnit,  dwRequestedSpeed100thX,  pdwGottenSpeed100thX);
}

inline DWORD obj_primo::NewAudio(PDWORD pdwUnits)
{
	return _call(PRIMO_NewAudio, (DWORD)PRIMOSDK_INTERR,  pdwUnits);
}

inline DWORD obj_primo::CloseAudio()
{
		return _call(PRIMO_CloseAudio, (DWORD)PRIMOSDK_INTERR);
}

inline DWORD obj_primo::WriteAudioEx(DWORD dwFlags, DWORD dwSpeed, PBYTE pMCN)
{
			return _call(PRIMO_WriteAudioEx, (DWORD)PRIMOSDK_INTERR, dwFlags, dwSpeed, pMCN);
}

#endif