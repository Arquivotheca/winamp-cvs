#pragma once
#include <windows.h>
#include "obj_primo.h"
class PrimoObject : public obj_primo
{
public:
	PrimoObject();
	~PrimoObject();

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
	DWORD DiscInfoEx(PDWORD, DWORD, PDWORD, PDWORD, PDWORD, PDWORD, PDWORD, PDWORD);
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

private:
	DWORD handle;

protected:
	RECVS_DISPATCH;
};