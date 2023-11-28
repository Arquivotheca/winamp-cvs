//-----------------------------------------------------------------------------
// functions.c
// Copyright (c) Sonic Solutions.  All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// PROJECT:         PrimoSDK C Sample
//
// DESCRIPTION:     Low level functions
//
// If you are a licensed user of PrimoSDK you are authorized to
// copy part or entirely the code of this example into your
// application.
//-----------------------------------------------------------------------------

#include <windows.h>
#include <commctrl.h>
#include <mbstring.h>
#include <time.h>
#include <stdio.h>
#include <tchar.h>

#include "resource.h"

#include "primosdk.h"
#include "pxsample_c.h"

//-----------------------------------------------------------------------------
//
//  Display info about a Global Image file
//
//   Param: szGIFileName - name of the global image file.
//
//
//   Notes: None.
//
//
//  Return: None.
//
//-----------------------------------------------------------------------------
VOID GIInfo(TCHAR *GIFileName, PGLOBAL g)
{
	DWORD dwMediumFormat;
	DWORD dwTracks;
	DWORD dwUsed;
	DWORD dwErr;
	DWORD dwMedType;
	DWORD dwMedTypeEx;
	TCHAR Buf[2000];
	TCHAR *Format;

#ifdef _UNICODE
	dwErr = PrimoSDK_GIInfoExWcs(g->dwHandle,  GIFileName,  &dwMediumFormat,
								 &dwTracks, &dwUsed, &dwMedType, &dwMedTypeEx);
#else
	dwErr = PrimoSDK_GIInfoEx(g->dwHandle,  GIFileName,  &dwMediumFormat,
							  &dwTracks, &dwUsed, &dwMedType, &dwMedTypeEx);
#endif

	if (dwErr != PRIMOSDK_OK)
	{
		DisplayError(dwErr,_T("PrimoSDK_GIInfo"),NULL);
	}
	else
	{
		switch (dwMediumFormat)
		{
		case PRIMOSDK_B1:
			Format = _T("B1");
			break;
		case PRIMOSDK_D1:
			Format = _T("D1");
			break;
		case PRIMOSDK_D2:
			Format = _T("D2");
			break;
		case PRIMOSDK_D3:
			Format = _T("D3");
			break;
		case PRIMOSDK_D4:
			Format = _T("D4");
			break;
		case PRIMOSDK_D5:
			Format = _T("D5");
			break;
		case PRIMOSDK_D6:
			Format = _T("D6");
			break;
		case PRIMOSDK_D7:
			Format = _T("D7");
			break;
		case PRIMOSDK_D8:
			Format = _T("D8");
			break;
		case PRIMOSDK_D9:
			Format = _T("D9");
			break;
		case PRIMOSDK_A1:
			Format = _T("A1");
			break;
		case PRIMOSDK_A2:
			Format = _T("A2");
			break;
		case PRIMOSDK_A3:
			Format = _T("A3");
			break;
		case PRIMOSDK_A4:
			Format = _T("A4");
			break;
		case PRIMOSDK_A5:
			Format = _T("A5");
			break;
		case PRIMOSDK_M1:
			Format = _T("M1");
			break;
		case PRIMOSDK_M2:
			Format = _T("M2");
			break;
		case PRIMOSDK_M3:
			Format = _T("M3");
			break;
		case PRIMOSDK_M4:
			Format = _T("M4");
			break;
		case PRIMOSDK_M5:
			Format = _T("M5");
			break;
		case PRIMOSDK_M6:
			Format = _T("M6");
			break;
		case PRIMOSDK_F1:
			Format = _T("F1");
			break;
		case PRIMOSDK_F3:
			Format = _T("F3");
			break;
		case PRIMOSDK_F4:
			Format = _T("F4");
			break;
		case PRIMOSDK_F5:
			Format = _T("F5");
			break;
		case PRIMOSDK_F8:
			Format = _T("F8");
			break;
		case PRIMOSDK_FA:
			Format = _T("FA");
			break;
		default:
			Format = _T("Generic CD");
			break;
		}

#ifdef _UNICODE
		wsprintf(Buf,_T("%s\nFormat: %s\nMedium Type:  %x\nTracks: %d\nSectors Used: %d"), GIFileName, Format, dwMedTypeEx, dwTracks, dwUsed);
#else
		wsprintf(Buf,_T("%s\nFormat: %s\nMedium Type:  %x\nTracks: %d\nSectors Used: %d"), GIFileName, Format, dwMedTypeEx, dwTracks, dwUsed);
#endif
		PxMessageStr(Buf, g->hDlgModeless);

	}
}



//-----------------------------------------------------------------------------
//
//  Display info about a disc drive
//
//   Param: dwUnit - disc address (triple)
//
//
//   Notes: None.
//
//
//  Return: None.
//
//-----------------------------------------------------------------------------
VOID DisplayUnitInfo(DWORD dwUnit, DWORD dwHandle)
{
	UINT i;
	TCHAR Buf[50000], Buf2[2048], Buf3[1024], Buf4[64], Buf5[1024], Buf6[1024];
	DWORD dwReady;
	DWORD dwErr;
	DWORD dwCDSpeeds[32], dwDVDSpeeds[32], dwCapa, dwClass;
	DWORD dwType, dwTypes[16];      // Unit type, all types
	TCHAR UnitDescr[64+1];

	EnterCriticalSection(&gCrit);

	{
		BYTE     szUnitDescr[64+1];        // Unit Vendor, Model and FW. version
		dwErr = PrimoSDK_UnitInfo(dwHandle,&dwUnit, &dwType, szUnitDescr, &dwReady);
#ifdef _UNICODE
		MultiByteToWideChar(GetACP(), MB_PRECOMPOSED, szUnitDescr, -1,
							UnitDescr, sizeof(UnitDescr)/sizeof(UnitDescr[0]));
#else
		strncpy(UnitDescr, szUnitDescr, sizeof(UnitDescr));
#endif
	}

	if (dwErr != PRIMOSDK_OK)
	{
		DisplayError(dwErr,_T("PrimoSDK_UnitInfo"),NULL);
		LeaveCriticalSection(&gCrit);
		return;
	}

	dwErr = PrimoSDK_UnitInfo2(dwHandle,&dwUnit, dwTypes, &dwClass, NULL, NULL);

	if (dwErr != PRIMOSDK_OK)
	{
		DisplayError(dwErr,_T("PrimoSDK_UnitInfo2"),NULL);
		LeaveCriticalSection(&gCrit);
		return;
	}

	dwErr = PrimoSDK_UnitSpeeds(dwHandle,&dwUnit, dwCDSpeeds, dwDVDSpeeds, &dwCapa);

	if (dwErr != PRIMOSDK_OK)
	{
		DisplayError(dwErr,_T("PrimoSDK_UnitSpeeds"),NULL);
		LeaveCriticalSection(&gCrit);
		return;
	}

	switch (dwType)
	{

	case PRIMOSDK_CDROM:
		_tcscpy(Buf3, _T("CD-ROM"));
		break;

	case PRIMOSDK_CDR:
		_tcscpy(Buf3, _T("CD-R"));
		break;

	case PRIMOSDK_CDRW:
		_tcscpy(Buf3, _T("CD-RW"));
		break;

	case PRIMOSDK_DVDROM:
		_tcscpy(Buf3, _T("DVD-ROM"));
		break;

	case PRIMOSDK_DVDR:
		_tcscpy(Buf3, _T("DVD-R"));
		break;

	case PRIMOSDK_DVDRW:
		_tcscpy(Buf3, _T("DVD-RW"));
		break;

	case PRIMOSDK_DVDPRW:
		_tcscpy(Buf3, _T("DVD+RW"));
		break;

	case PRIMOSDK_DVDRAM:
		_tcscpy(Buf3, _T("DVD-RAM"));
		break;

	case PRIMOSDK_OTHER:
		PxLoadString(IDS_NOTMANAGEDTYPE,Buf3,sizeof(Buf3));
		break;

	}

	_tcscat(Buf3,_T(" ("));

	for (i=0; dwTypes[i] != 0xFFFFFFFF; i++)
	{
		wsprintf(&Buf3[_tcslen(Buf3)],_T("0x%02X "),dwTypes[i]);
	}

	_tcscpy(&Buf3[_tcslen(Buf3)-1],_T(")  Class: "));

	if (dwClass == PRIMOSDK_SSCLASS)
		_tcscat(Buf3,_T("SS"));
	else
		wsprintf(&Buf3[_tcslen(Buf3)],_T("%d"),dwClass);

	PxLoadString(IDS_UNITINFO,Buf2,sizeof(Buf2));
	PxLoadString(dwReady?IDS_READY:IDS_NOTREADY,Buf4,sizeof(Buf4));

	_tcscpy(Buf5,_T("ROM: "));

	for (i=0; dwCDSpeeds[i] != 0xFFFFFFFF; i++)
		wsprintf(&Buf5[_tcslen(Buf5)],_T("%dx "),dwCDSpeeds[i]);

	_tcscat(Buf5,_T("\n\t R: "));

	for (i++; dwCDSpeeds[i] != 0xFFFFFFFF; i++)
		wsprintf(&Buf5[_tcslen(Buf5)],_T("%dx "),dwCDSpeeds[i]);

	_tcscat(Buf5,_T("\n\t RW: "));

	for (i++; dwCDSpeeds[i] != 0xFFFFFFFF; i++)
		wsprintf(&Buf5[_tcslen(Buf5)],_T("%dx "),dwCDSpeeds[i]);

	_tcscpy(Buf6,_T("ROM: "));

	for (i=0; dwDVDSpeeds[i] != 0xFFFFFFFF; i++)
		wsprintf(&Buf6[_tcslen(Buf6)],_T("%dx "),dwDVDSpeeds[i]);

	_tcscat(Buf6,_T("\n\t R: "));

	for (i++; dwDVDSpeeds[i] != 0xFFFFFFFF; i++)
		wsprintf(&Buf6[_tcslen(Buf6)],_T("%dx "),dwDVDSpeeds[i]);
	_tcscat(Buf6,_T("\n\t RW: "));

	for (i++; dwDVDSpeeds[i] != 0xFFFFFFFF; i++)
		wsprintf(&Buf6[_tcslen(Buf6)],_T("%dx "),dwDVDSpeeds[i]);
	//
	wsprintf(Buf,Buf2,Buf3,HIBYTE(HIWORD(dwUnit)),LOBYTE(HIWORD(dwUnit)),
			 HIBYTE(LOWORD(dwUnit)),LOBYTE(LOWORD(dwUnit)),UnitDescr,Buf5,
			 Buf6,dwCapa,Buf4);

	PxMessageStr(Buf, NULL);

	LeaveCriticalSection(&gCrit);
}



//-----------------------------------------------------------------------------
//
//  Display info about the disc in the disc drive
//
//   Param: dwUnit - disc address (triple)
//
//
//   Notes: None.
//
//
//  Return: None.
//
//-----------------------------------------------------------------------------
VOID DisplayDiscInfo(DWORD dwUnit, DWORD dwHandle)
{
	UINT i;
	TCHAR Buf[50000], Buf2[2048], Buf3[1024], DVDType[64];
	DWORD dwFormatType, dwErasable, dwDVDProtected, dwMedTypeEx;
	DWORD dwSessionNumber, dwTrackType, dwPreGap, dwStart, dwLength;
	DWORD dwUsed;
	DWORD dwErr;
	DWORD dwDIFlags, dwDVDType;
	DWORD dwMediumType, dwMedium;        // Disc Type
	DWORD dwTracks;                      // Total tracks number
	DWORD dwFree;                        // Total free sectors
	int   iRetry = 3;

	EnterCriticalSection(&gCrit);

	GetFileSystem(dwUnit, dwHandle);

	dwErr = PrimoSDK_DiscInfo(dwHandle,&dwUnit,&dwMediumType,&dwFormatType,&dwErasable,
							  &dwTracks,&dwUsed,&dwFree);

	if (dwErr != PRIMOSDK_OK)
	{
		DisplayError(dwErr,_T("PrimoSDK_DiscInfo"),NULL);
		LeaveCriticalSection(&gCrit);
		return;
	}

	do
	{
		dwErr = PrimoSDK_DiscInfo2(dwHandle,&dwUnit,&dwMedium,&dwDVDProtected,&dwDIFlags,&dwMedTypeEx,NULL);

		if (dwErr != PRIMOSDK_OK)
		{
			if (iRetry == 0)
			{
				DisplayError(dwErr,_T("PrimoSDK_DiscInfo2"),NULL);
				LeaveCriticalSection(&gCrit);
				return;
			}
			else
			{
				DisplayError(dwErr,_T("PrimoSDK_DiscInfo2::Retrying..."),NULL);
				iRetry--;
			}
		}
	}
	while (dwErr == PRIMOSDK_UNITERROR);

	if (dwMedTypeEx == PRIMOSDK_DVDROM  ||
		dwMedTypeEx == PRIMOSDK_DVDR    ||
		dwMedTypeEx == PRIMOSDK_DVDRW   ||
		dwMedTypeEx == PRIMOSDK_DVDRW9  ||
		dwMedTypeEx == PRIMOSDK_DVDPR   ||
		dwMedTypeEx == PRIMOSDK_DVDPRW  ||
		dwMedTypeEx == PRIMOSDK_DVDR9   ||
		dwMedTypeEx == PRIMOSDK_DVDPR9  ||
		dwMedTypeEx == PRIMOSDK_DVDRAM)
	{
		dwErr = PrimoSDK_GetDVDType(dwHandle,&dwUnit, &dwDVDType, NULL);

		if (dwErr != PRIMOSDK_OK)
		{
			DisplayError(dwErr,_T("PrimoSDK_GetDVDType"),NULL);
			LeaveCriticalSection(&gCrit);
			return;
		}
	}

	switch (dwMediumType)
	{

	case PRIMOSDK_SILVER:
		PxLoadString(IDS_SILVERORCLOSED,Buf3,sizeof(Buf3));
		break;

	case PRIMOSDK_COMPLIANTGOLD:
		PxLoadString(IDS_VALIDMEDIA,Buf3,sizeof(Buf3));
		break;

	case PRIMOSDK_OTHERGOLD:
		PxLoadString(IDS_NOTVALIDMEDIA,Buf3,sizeof(Buf3));
		break;

	case PRIMOSDK_BLANK:
		PxLoadString(IDS_BLANKCD,Buf3,sizeof(Buf3));
		break;

	}

	PxLoadString(IDS_DISCINFO,Buf2,sizeof(Buf2));
	wsprintf(Buf,Buf2,Buf3,dwMedium,dwMedTypeEx, dwFormatType,dwErasable?_T("Yes"):_T("No"),dwDVDProtected?_T("Yes"):_T("No"),dwTracks,dwUsed,dwFree);

	if (dwTracks > 0)
	{
		char Isrc[128] = "";
		_tcscat(Buf,_T("\r\n"));
		for (i=1; i <= (INT)dwTracks; i++)
		{
			dwErr = PrimoSDK_TrackInfo(dwHandle,i,&dwSessionNumber,&dwTrackType,&dwPreGap,&dwStart,&dwLength);
			if (dwErr != PRIMOSDK_OK)
			{
				DisplayError(dwErr,_T("PrimoSDK_TrackInfo"),NULL);
				break;
			}

			PxLoadString(IDS_TRACKINFO,Buf2,sizeof(Buf2));
			wsprintf(Buf3,Buf2,i,dwSessionNumber,dwTrackType,dwPreGap,dwStart,dwLength);
			_tcscat(Buf,Buf3);

#if 0
			dwErr = PrimoSDK_GetISRC(dwHandle, &dwUnit, i, Isrc);
			if (dwErr == PRIMOSDK_OK)
			{
				_tcscat(Buf,_T(" ISRC:"));
#ifdef _UNICODE
				{
					WCHAR IsrcWcs[64];
					MultiByteToWideChar(GetACP(), MB_PRECOMPOSED, Isrc, -1,
										IsrcWcs, sizeof(IsrcWcs)/sizeof(IsrcWcs[0]));
					wcscat(Buf,IsrcWcs);
				}
#else
				strcat(Buf, Isrc);
#endif
			}
#endif

		}
	}

	/*
	// TEST OF PrimoSDK_CDTextInfo - MAY DISPLAY MORE THAN CAN FIT ON SCREEN
	dwErr = PrimoSDK_CDTextInfo(dwHandle,&dwUnit,szCDTitle,szPerformer,szComposer);

	if (dwErr == PRIMOSDK_OK )
	{
	 PxLoadString(IDS_CDTEXTINFO,szBuf2,sizeof(szBuf2));
	 wsprintf(szCDTextInfo,szBuf2,szCDTitle,szPerformer,szComposer);
	   strcat(szBuf,"\r\n");
	strcat(szBuf,szCDTextInfo);
	}
	*/
	wsprintf(DVDType, _T("\nDVDType: 0x%x"), dwDVDType);
	_tcscat(Buf, DVDType);

	if (dwDIFlags & PRIMOSDK_PACKETWRITTEN)
		_tcscat(Buf, _T("\nPacket Written"));


	// get the list of writable speeds for a given media
	{
		DWORD dwNumSpeeds;
		DWORD Speeds[24];
		DWORD Speed;

		PrimoSDK_GetDiscSpeedList(dwHandle, &dwUnit,
								  sizeof(Speeds) / sizeof(Speeds[0]),
								  &dwNumSpeeds,
								  Speeds);

		_tcscat(Buf, _T("\nSpeeds:"));
		for (Speed = 0; Speed < dwNumSpeeds; Speed++)
		{
			wsprintf(DVDType, _T(" %2d.%1dx"), (int)Speeds[Speed]/ 100, (Speeds[Speed]%100)/10);
			_tcscat(Buf, DVDType);
		}
	}

	LeaveCriticalSection(&gCrit);
	PxMessageStr(Buf, NULL);
}



//-----------------------------------------------------------------------------
// Utility functions
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
//
//   Lock/Unlock drive(s) and block/unblock Windows Auto Insert
//     Notification (AIN) for the drive(s).
//
//   Param: pdwUnit - pointer to a list of drives, terminated by NO_DRIVE
//
//          bLockIt - If TRUE, lock the drive, otherwise unlock it.
//
//   Notes: Uses VxBlock to test if the device is in use. If it is in use,
//            we tell the user and allow them to continue anyway.
//
//          While locking, if the AIN service is not present, we present
//          a message and the user can still abort the operation.
//
//          NOTE: Only unlocks ONE drive (not a list).
//
//  Return: TRUE if the unit is locked/unlocked successfully.
//
//-----------------------------------------------------------------------------
BOOL LockAndBlock(PDWORD pdwUnit, BOOL bLockIt, DWORD dwHandle)
{
	TCHAR Buf[512], MsgStr[512], DrvStr[260], AppName[512];
	DWORD i, dwUnit, dwRet;
	DWORD dwType;                         // Unit type
	TCHAR UnitDescr[64+1];                           // Unit Vendor, Model and FW. version

	//
	// Lock all units in the list.
	//

	for (i=0; pdwUnit[i] != NO_DRIVE; i++)
	{
		dwUnit = pdwUnit[i];

		if (bLockIt) //Locking
		{
			//
			// Test VxBlock to see if we can have the drive
			//

			{
				char szAppName[512]={0};
				dwRet = PrimoSDK_UnitVxBlock(dwHandle,&dwUnit,PRIMOSDK_TEST,szAppName);
#ifdef _UNICODE
				MultiByteToWideChar(GetACP(), MB_PRECOMPOSED, szAppName, -1,
									AppName, sizeof(AppName)/sizeof(AppName[0]));
#else
				strncpy(AppName, szAppName, sizeof(AppName));
#endif
			}

			//
			// The most likely values of dwRed are PRIMOSDK_OK or PRIMOSDK_NOTREADY,
			//

			if (dwRet == PRIMOSDK_OK)
			{
				//
				// Call VxBlock to reserve the drive(s).
				//

				dwRet = PrimoSDK_UnitVxBlock(dwHandle,&dwUnit,PRIMOSDK_LOCK,"PrimoSDKSample");
			}

			if (dwRet == PRIMOSDK_NOTREADY)
			{
				//
				// Unit is in use by another application.
				// Get the information and put up a message to the user
				//

				{
					BYTE  szUnitDescr[64+1];        // Unit Vendor, Model and FW. version
					dwRet = PrimoSDK_UnitInfo(dwHandle,&dwUnit,&dwType,szUnitDescr,NULL);
#ifdef _UNICODE
					MultiByteToWideChar(GetACP(), MB_PRECOMPOSED, szUnitDescr, -1,
										UnitDescr, sizeof(UnitDescr)/sizeof(UnitDescr[0]));
#else
					strncpy(UnitDescr, szUnitDescr, sizeof(UnitDescr));
#endif
				}
				if (dwRet == PRIMOSDK_OK)
				{
					DrvStr[0] = 0x00;

					if (LOBYTE(LOWORD(dwUnit)))
						wsprintf(&DrvStr[_tcslen(DrvStr)],_T("%c: "),LOBYTE(LOWORD(dwUnit)));

					wsprintf(&DrvStr[_tcslen(DrvStr)],_T("%s"),UnitDescr);
				}

				PxLoadString(IDS_DRIVEINUSE,MsgStr,sizeof(MsgStr));
				wsprintf(Buf,MsgStr,DrvStr,AppName);

				if (PxMessageStrYesNo(Buf, NULL) != IDYES)
				{
					//////////////
					return(FALSE); // return failure.
					//////////////
				}
			}
		} // endif bLockIt

		//
		// LOCKING: Now that we have reserved the drive with VxBlock, turn off the Auto Insert Notification for this drive.
		// UNLOCKING: Restore the Auto Insert Notification for this drive.
		//

		dwRet = PrimoSDK_UnitAIN(dwHandle,&dwUnit,bLockIt?PRIMOSDK_LOCK:PRIMOSDK_UNLOCK);

		if (dwRet == PRIMOSDK_NOAINCONTROL && bLockIt)
		{
			//
			// We tried to lock, but we can't control the AIN for this device. Warn the user.
			//

			if (PxMessageYesNo(IDS_NOAIN, NULL) != IDYES)
			{
				//
				//User wants to abort, so undo the VxBlock that we just did and return.
				//

				dwRet = PrimoSDK_UnitVxBlock(dwHandle,&dwUnit,PRIMOSDK_UNLOCK,"PrimoSDKSample");

				//////////////
				return(FALSE);
				//////////////
			}
		}

		//
		// Finally, lock the drive door.
		//

		PrimoSDK_UnitLock(dwHandle,&dwUnit,bLockIt?PRIMOSDK_LOCK:PRIMOSDK_UNLOCK);
	} // end for loop

	//
	// This program  only calls LockAndBlock() to unlock the drive if an operation fails.
	// In this case we need to unlock the VxBlock.
	// Normally the unlocking steps are done in UnlockAndEjectAllDrives().
	//
	// NOTE! This will just unlock one drive. Be careful if there are more than one.
	//

	if (!bLockIt)
		dwRet = PrimoSDK_UnitVxBlock(dwHandle,&dwUnit,PRIMOSDK_UNLOCK,"PrimoSDKSample");

	return(TRUE);
}



//-----------------------------------------------------------------------------
//   Lock/Unlock drive and block/unblock Windows Auto Insert
//     Notification (AIN) for the drive
//
//   Param: pdwUnit - pointer to a list of drives, terminated by NO_DRIVE
//
//          bLockIt - If TRUE, lock the drive, otherwise unlock it.
//
//   Notes: Uses VxBlock to test if the device is in use. If it is in use,
//            we tell the user and allow them to continue anyway.
//
//          While locking, if the AIN service is not present, we present
//          a message and the user can still abort the operation.
//
//          NOTE: Only unlocks ONE drive (not a list).
//
//  Return: TRUE if the unit is locked/unlocked successfully.
//
//-----------------------------------------------------------------------------
BOOL LockAndBlockSource(DWORD dwUnitSource, BOOL bLockIt, DWORD dwHandle)
{
	DWORD dwUnit, dwRet;

	dwUnit = dwUnitSource;

	dwRet = PrimoSDK_UnitAIN(dwHandle,&dwUnit,bLockIt?PRIMOSDK_LOCK:PRIMOSDK_UNLOCK);

	if (dwRet == PRIMOSDK_NOAINCONTROL && bLockIt)
	{
		if (PxMessageYesNo(IDS_NOAIN, NULL) != IDYES)
		{
			return(FALSE);
		}
	}

	PrimoSDK_UnitLock(dwHandle,&dwUnit,bLockIt?PRIMOSDK_LOCK:PRIMOSDK_UNLOCK);

	return(TRUE);
}



//-----------------------------------------------------------------------------
//
//   Unlock drives and unblock Windows Auto Insert Notification (AIN)
//     for the drive.
//
//   Param: None.
//
//   Notes: Uses the following globals:
//
//          dwUnitSource - address of source drive
//
//          dwUnitsRec -  list of recorder addresses terminated by NO_DRIVE
//
//  Return: TRUE if the unit is locked/unlocked successfully.
//
//-----------------------------------------------------------------------------
VOID UnlockAndEjectAllDrives(PGLOBAL g)
{
	UINT i;
	DWORD dwErr;

	//
	// Unlock the drive(s) and open the tray(s)
	//

	for (i=0; g->dwUnitsRec[i] != NO_DRIVE; i++)
	{
		dwErr = PrimoSDK_UnitLock(g->dwHandle,&g->dwUnitsRec[i],PRIMOSDK_UNLOCK);
		dwErr = PrimoSDK_MoveMedium(g->dwHandle,&g->dwUnitsRec[i],PRIMOSDK_OPENTRAYEJECT|PRIMOSDK_IMMEDIATE);
	}

	//
	// Now unlock the drive(s) to allow system activity
	//

	for (i=0; g->dwUnitsRec[i] != NO_DRIVE; i++)
	{
		dwErr = PrimoSDK_UnitAIN(g->dwHandle,&g->dwUnitsRec[i],PRIMOSDK_UNLOCK);
		dwErr = PrimoSDK_UnitVxBlock(g->dwHandle,&g->dwUnitsRec[i],PRIMOSDK_UNLOCK,"PrimoSDKSample");
	}
}


//-----------------------------------------------------------------------------
//
//  Return a pointer that skips past the volume part of a filename.
//
//   Param: p - string containing a filename
//
//   Notes:  Skips past drive letter of servername\volume
//
//  Return: pointer into string p.
//
//
//-----------------------------------------------------------------------------
LPTSTR SkipDriveUNC(LPTSTR p)
{
	if (*(p+1) == '\\')
	{
		// This must be a UNC path like "\\servername\volume\file..."
		p = _tcschr(p+2,'\\'); // skip servername.....^
		p = _tcschr(p+1,'\\'); // skip volume name...........^
	}
	else
	{
		// Assume that this is a string like "C:..."
		p += 2;
	}
	return(p);
}





//
//

#define MAX_SECTOR_SIZE  (2352)

//-----------------------------------------------------------------------------
//
// Scan the disc looking for tags that indicate the file system.
// return FS_ISO, FS_UDF, or FS_UNKNOWN (0) for unknown/error
//
//   Param: dwUnit - drive address triplet
//
//   Notes:  none.
//
//  Return: File system detected. One of:
//
//              FS_ISO, ISO file system
//              FS_UDF  UDF file system
//              FS_UNKNOWN (0) for unknown/error.
//
//
//-----------------------------------------------------------------------------
DWORD GetFileSystem(DWORD dwUnit, DWORD dwHandle)
{
	BOOL bFinish = FALSE;
	BYTE szTag[ MAX_SECTOR_SIZE ];
	DWORD dwSector = 0x10; // start at this sector to look for the tag
	DWORD dwErr = 0;
	BOOL dwFS = FS_UNKNOWN;


	dwErr = PrimoSDK_OpenReadDisc(dwHandle, &dwUnit, 2);

	if (dwErr == PRIMOSDK_OK)
	{
		while (!bFinish)
		{
			// Read one sector
			dwErr = PrimoSDK_ReadDisc(dwHandle, &dwUnit, 2, dwSector, 1, szTag);

			if (dwErr == PRIMOSDK_OK)
			{

				if (strncmp("CD001",(PCHAR) szTag+1,5) == 0)
				{
					dwFS = FS_ISO;
					bFinish = TRUE;
				}
				else if (strncmp("BEA01",(PCHAR) szTag+1,5) == 0)
				{
					dwFS = FS_UDF;
					bFinish = TRUE;
				}
			}
			else
			{
				bFinish = TRUE;
			}

			dwSector++; // try next sector.

			// If we have gotten past sector $40 then something is wrong.
			if (dwSector > 0x40)
				bFinish = TRUE;
		}
	}

	dwErr = PrimoSDK_CloseReadDisc(dwHandle, &dwUnit,2);
	return dwFS;
}


//-----------------------------------------------------------------------------
//
// Test the disc for ISO file system
//
//   Param: dwUnit - drive address triplet
//
//   Notes:  none.
//
//  Return: TRUE if the disc in drive dwUnit has an ISO file system
//
//-----------------------------------------------------------------------------
BOOL IsISO(DWORD dwUnit, DWORD dwHandle)
{
	return GetFileSystem(dwUnit, dwHandle) == FS_ISO;
}


//-----------------------------------------------------------------------------
//
// Test the disc for UDF file system
//
//   Param: dwUnit - drive address triplet
//
//   Notes:  none.
//
//  Return: TRUE if the disc in drive dwUnit has an UDF file system
//
//-----------------------------------------------------------------------------
BOOL IsUDF(DWORD dwUnit, DWORD dwHandle)
{
	return GetFileSystem(dwUnit, dwHandle) == FS_UDF;
}

// Return error code
// Out - all times in seconds
// dwLeadInTime
// dwWriteTime
// dwLeadOutTime
//
// Notes: CD "1x" speed is 150 KB/second or about 75 sectors/second.
//        DVD "1x" speed is 1,275 KB / second or about 638 sectors /second



//-----------------------------------------------------------------------------
//
// Estimate the time required to complete a Write operation
//
//   Param: pProg - The progress record.
//
//          dwSectorsRemaining - number of sectors left to write.
//
//   Notes:  These are estimates only.
//
//  Return: estimated write time in seconds
//
//-----------------------------------------------------------------------------
DWORD EstimateWriteTimes(PPROGRESS pProg, DWORD dwSectorsRemaining, PGLOBAL g)
{
	DWORD dwWriteTime;

	//
	// Initialize return value.
	//

	dwWriteTime = 0;

	switch (pProg->dwMedTypeEx)
	{

	case PRIMOSDK_CDR:
	case PRIMOSDK_CDRW:
	case PRIMOSDK_DDCDR:
	case PRIMOSDK_DDCDRW:


		//
		//Blocks
		//

		dwWriteTime = dwSectorsRemaining / (CD_SPEED * pProg->dwSpeed);


		//LeadOut (assuming only one session)
		dwWriteTime += (115 / pProg->dwSpeed) + 12;

		break;

	case PRIMOSDK_DVDR:
	case PRIMOSDK_DVDRW:

#define DVDMRW_SPEED (9*75)

		if (pProg->dwMedTypeEx == PRIMOSDK_DVDRW)
			dwWriteTime = dwSectorsRemaining / (pProg->dwSpeed * DVDMRW_SPEED);
		else
			dwWriteTime = dwSectorsRemaining / (pProg->dwSpeed * DVD_SPEED);

		if (pProg->dwMedTypeEx != PRIMOSDK_DVDRW)
		{
			//
			// LeadOut (11 minutes)
			// Px engine does not send the Close command if in testing.
			//

			if (g->dwAction!=PRIMOSDK_TEST)
			{

				if (dwWriteTime > (660 / pProg->dwSpeed))
				{
					//
					//If the disc write time is more than 11 mins, the lead out is much less.
					//

					dwWriteTime += (70 / pProg->dwSpeed);
				}
				else
				{
					//
					// Lead out time will cause total to be at least 11 minutes.
					//

					dwWriteTime += dwWriteTime - (660 / pProg->dwSpeed);
				}
			}

			dwWriteTime += (82 / pProg->dwSpeed);
		}
		else
		{
			if (pProg->dwMedTypeEx != PRIMOSDK_DVDRW)
			{
				dwWriteTime += (36 / pProg->dwSpeed);
			}
			else
			{
				dwWriteTime += (110 / pProg->dwSpeed);
			}
		}

		break;

	case PRIMOSDK_DVDPR:
	case PRIMOSDK_DVDPRW:

		dwWriteTime = dwSectorsRemaining / (pProg->dwSpeed * (9 * 75));

		//
		// lead-out
		//

		if (pProg->dwMedTypeEx == PRIMOSDK_DVDPRW)
		{

			dwWriteTime += (11 / pProg->dwSpeed);
		}
		else
		{
			dwWriteTime += (25 / pProg->dwSpeed);
		}

		break;

	case PRIMOSDK_DVDRAM:

		//
		// Blocks
		//

		dwWriteTime = dwSectorsRemaining / (pProg->dwSpeed * DVD_SPEED);

		// Fudge the speed a little.
		dwWriteTime += (6 / pProg->dwSpeed);

		// No lead-in, lead-out.

		break;

		//
		// These remaining cases are not writable, so we must be reading from them.
		//

	case PRIMOSDK_DVDROM:

		dwWriteTime = dwSectorsRemaining / (pProg->dwSpeed * DVD_SPEED);

		break;


	case PRIMOSDK_CDROM:
	case PRIMOSDK_OTHER:
	case PRIMOSDK_DDCDROM:
	default:

		dwWriteTime = dwSectorsRemaining / (CD_SPEED * pProg->dwSpeed);

		break;

	}

	return dwWriteTime;
}


//-----------------------------------------------------------------------------
//
// Estimate the time required to complete a Write operation
//
//   Param: pProg - The progress record.
//
//          dwTotalSectors - number of sectors needed to write.
//
//          dwSpeed - speed (1 = 1x = 75 sectors per second for CD)
//
//          dwUnit - recording device address
//
//   Notes: Call right after we have started an operation.
//
//  Return: error code if we can not get disc information
//
//-----------------------------------------------------------------------------
DWORD StartProgress(PPROGRESS pProg, DWORD dwTotalSectors, DWORD dwSpeed, DWORD dwUnit, BOOL bVerify, PGLOBAL g)
{
	DWORD dwFormatType, dwErasable, dwDVDProtected;
	DWORD dwUsed;
	DWORD dwErr;
	DWORD dwDIFlags;
	DWORD dwMediumType, dwMedium;        // Disc Type
	DWORD dwTracks;                      // Total tracks number
	DWORD dwFree;                        // Total free sectors

	pProg->dwSectorsToWrite = dwTotalSectors;
	pProg->dwSpeed = dwSpeed;
	pProg->dwUnit = dwUnit;
	pProg->dwStartSector = 0;
	pProg->tWriteStart = 0;

	if (bVerify)
		pProg->dwPhase = PROGRESS_VERIFY;
	else
		pProg->dwPhase = PROGRESS_PREWRITE;

	time(&pProg->tStart);

	pProg->dwPercentComplete = 0;


	//
	// Find out about the disc - most important is that we set pProg->dwMedTypeEx, so that
	// EstimateWriteTimes will make estimates appropriate for the media type.
	//


	dwErr = PrimoSDK_DiscInfo(g->dwHandle,&dwUnit,&dwMediumType,&dwFormatType,&dwErasable,
							  &dwTracks,&dwUsed,&dwFree);

	if (dwErr != PRIMOSDK_OK)
	{
		DisplayError(dwErr,_T("PrimoSDK_DiscInfo"),NULL);
		return 0;
	}

	dwErr = PrimoSDK_DiscInfo2(g->dwHandle,&pProg->dwUnit,&dwMedium,&dwDVDProtected,&dwDIFlags,&pProg->dwMedTypeEx,NULL);

	if (dwErr != PRIMOSDK_OK)
	{
		DisplayError(dwErr,_T("PrimoSDK_DiscInfo2"),NULL);
		return 0;
	}

	//
	// If dwSpeed == 0 then we really don't know the speed at this time.
	// We will get called later with a nonzero speed as the operation progresses.
	//

	if (pProg->dwSpeed == 0)
		return 0;

	return EstimateWriteTimes(pProg, dwTotalSectors, g);

}


//-----------------------------------------------------------------------------
//
// Calculate percent complete and time remaining.
//
//   Param: pProg - The progress record.
//
//          dwSectorsProcessed - number of sectors written so far
//
//   Notes: Call during an operation, about once a second.
//
//  Return: value 0-100, percentage of progress.
//
//-----------------------------------------------------------------------------
DWORD EstimateProgress(PPROGRESS pProg, DWORD dwSectorsProcessed, DWORD dwTotalSectors, PGLOBAL g)
{
	DWORD dwOldPercentComplete;
	DWORD dwElapsedTime;


	dwElapsedTime  = (DWORD)(time(NULL) - pProg->tStart);

	//
	// Make sure we pick up new total sectors (it may change as the operation progresses)
	//

	pProg->dwSectorsToWrite = dwTotalSectors;


	//
	// Save old percentage value so we never go backwards.
	//

	dwOldPercentComplete = pProg->dwPercentComplete;

	if (pProg->dwPhase == PROGRESS_VERIFY)
	{
		//
		// During verify, measure progress by sectors instead of time.
		//

		if (dwSectorsProcessed > pProg->dwSectorsToWrite)
		{
			pProg->dwPercentComplete = 100;
		}
		else
		{
			if (pProg->dwSectorsToWrite > 0)
				pProg->dwPercentComplete = (dwSectorsProcessed * 100 / pProg->dwSectorsToWrite);
			else
				pProg->dwPercentComplete = 0;
		}
	}
	else // not Verify, write.
	{
		if (pProg->dwWriteTime == 0)
		{

			//
			// We do not have an estimate of write time yet, use sectors
			//

			if ((dwSectorsProcessed > 0) && (pProg->dwSectorsToWrite > 0))
			{
				pProg->dwPercentComplete = (dwSectorsProcessed * 100 / pProg->dwSectorsToWrite);
			}
		}
		else //pProg->dwWriteTime nonzero
		{
			if (dwElapsedTime > pProg->dwWriteTime)
				pProg->dwPercentComplete = 100;
			else
			{
				if (pProg->dwWriteTime > 0)
					pProg->dwPercentComplete = (dwElapsedTime * 100 / pProg->dwWriteTime);
				else
					pProg->dwPercentComplete = 98;
			}
		}
	}


	//
	// Calculate time remaining, but don't give a negative time.
	//

	if (pProg->dwWriteTime > dwElapsedTime)
		pProg->dwTimeRemaining = pProg->dwWriteTime - dwElapsedTime;
	else
		pProg->dwTimeRemaining = 0;

	//
	// Make sure our percent does not go backwards!
	//

	if (pProg->dwPercentComplete <= dwOldPercentComplete)
		pProg->dwPercentComplete = dwOldPercentComplete;


	if (pProg->dwPercentComplete > 98)
		pProg->dwPercentComplete = 98;


	//
	// At this point we have a basic percentage complete.
	// From here below, we are figuring the time remaining and
	// adjusting the total time expected.


	if (dwSectorsProcessed > 0)
	{
		//
		// SHOW THE PROGRESS
		//

		if (dwSectorsProcessed >= pProg->dwSectorsToWrite)
		{
			if (pProg->dwPhase != PROGRESS_VERIFY)
			{
				pProg->dwPhase = PROGRESS_POSTWRITE;
			}
		}
		else if (dwSectorsProcessed < 1024)
		{
			if (pProg->dwPhase != PROGRESS_VERIFY)
			{
				pProg->dwPhase = PROGRESS_PREWRITE;
			}

			pProg->dwStartSector = 0;
			pProg->tWriteStart = 0;
		}
		else  if ((dwSectorsProcessed >= 1024) && (pProg->dwSectorsToWrite - dwSectorsProcessed > 1024))
		{
			//
			// We are not in the first 1024 sectors nor the last.
			//

			if ((pProg->dwSectorsToWrite > 0) && (pProg->tWriteStart > 0))
			{
				DWORD dwSectorsWrote;
				DWORD dwTimeElapsed;
				DWORD dwSpeed;
				DWORD dwCDSpeed;
				DWORD dwSampleLength;

				dwTimeElapsed = (DWORD)(time(NULL) - pProg->tStart);

				//
				//  Slow down the re-estimates after the burn gets going, so the
				// numbers don't jump around too much.
				//

				if (dwTimeElapsed > 30) //seconds
					dwSampleLength = 10;
				else
					dwSampleLength = 0;

				if (dwTimeElapsed > dwSampleLength)  // every 10 seconds...
				{

					dwSectorsWrote = dwSectorsProcessed - pProg->dwStartSector;

					dwCDSpeed = dwSpeed = (dwSectorsWrote / dwTimeElapsed) / 75;

					//
					// for DVDs adjust to its relative speed (8.5 * CD speed)
					//

					switch (pProg->dwMedTypeEx)
					{
					case PRIMOSDK_DVDR:
					case PRIMOSDK_DVDRW:
					case PRIMOSDK_DVDPR:
					case PRIMOSDK_DVDPRW:
					case PRIMOSDK_DVDRAM:
						dwSpeed = (17 * dwSpeed)/2;
					}

					//
					// cap the minimum speed at 1X
					//

					if (dwCDSpeed > 0)
					{

						pProg->dwSpeed = dwSpeed;

						if (pProg->dwPhase != PROGRESS_VERIFY)
						{
							pProg->dwWriteTime = EstimateWriteTimes(pProg, pProg->dwSectorsToWrite - dwSectorsProcessed, g) + dwElapsedTime;
						}
						else
						{
							pProg->dwWriteTime = ((pProg->dwSectorsToWrite - dwSectorsProcessed) / (dwCDSpeed * 75)) + dwElapsedTime + 5; //fudge a little
						}

						pProg->dwStartSector = dwSectorsProcessed;
						time(&pProg->tWriteStart);
					}
				}
			}
			else if ((dwSectorsProcessed > 2048) && pProg->dwSectorsToWrite > 0)
			{
				//
				// We have begun writing, get the time we started.
				//

				if (pProg->tWriteStart == 0)
				{
					time(&pProg->tWriteStart);
					pProg->dwStartSector = dwSectorsProcessed;
				}
			}
		}
		else if (pProg->dwSectorsToWrite > 0)
		{
			if (pProg->dwPhase != PROGRESS_VERIFY)
				pProg->dwPhase = PROGRESS_PREWRITE;

			pProg->dwStartSector = 0;
			pProg->tWriteStart = 0;
		}
	}
	return pProg->dwPercentComplete;
}

