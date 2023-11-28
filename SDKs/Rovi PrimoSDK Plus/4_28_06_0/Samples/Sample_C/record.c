//-----------------------------------------------------------------------------
// record.c
// Copyright (c) Sonic Solutions.  All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// PROJECT:         PrimoSDK C Sample
//
// DESCRIPTION:     Record-to-Disc Functions
//
//-----------------------------------------------------------------------------
// If you are a licensed user of PrimoSDK you are authorized to
// copy part or entirely the code of this example into your
// application.

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
//  Copy from source disk to a list of recorders
//
//   Param: dwSpeed - defines the speed to use for recording:
//                    PRIMOSDK_MAX or n (like 8 for 8x) or PRIMOSDK_BEST
//                    if the drive supports AWS
//
//   Notes: Uses the following globals:
//
//          dwUnitSource - address of source drive
//
//          dwUnitsRec -  list of recorder addresses terminated by NO_DRIVE
//
//
//  Return: None.
//
//-----------------------------------------------------------------------------
VOID CopyDisc(DWORD dwSpeed, PGLOBAL g)
{
	TCHAR Buf[50000], Buf2[2048];
	DWORD dwErr,dwFormatType, dwErasable;
	DWORD    dwMediumType;                     // Disc Type
	DWORD    dwTracks;                         // Total tracks number
	DWORD    dwFree;                           // Total free sectors
	DWORD    dwTotalSize;                      // Size of the image,
	DWORD    dwRepStat, dwSize, dwTotal;


	//
	// Check that at least one reader is present.
	//

	if (!g->dwHowManyReader)
	{
		PxMessage(IDS_NOCDFOUND);

		///////
		return;
		///////
	}

	//
	// Make sure that we have at least one recorder in the list
	//

	if (g->dwUnitsRec[0] == NO_DRIVE)
	{
		//
		// No recorders
		//

		PxMessage(IDS_NOCDRFOUND);
		///////
		return;
		///////
	}

	dwErr = PrimoSDK_DiscInfo(g->dwHandle,&g->dwUnitSource,&dwMediumType,&dwFormatType,
							  &dwErasable,&dwTracks,&dwTotalSize,&dwFree);

	if (dwErr != PRIMOSDK_OK)
	{
		DisplayError(dwErr,_T("PrimoSDK_DiscInfo"),NULL);
		///////
		return;
		///////
	}

	if (dwMediumType == PRIMOSDK_BLANK)
	{
		PxMessage(IDS_BLANKSOURCECD);

		///////
		return;
		///////
	}

	//
	// Display the recording status
	//

	EnableDisable(TRUE, g);

	//
	// Display the number of sectors needed and wait a little while (to let the user see...)
	//

	PxLoadString(IDS_SECTORSTOCOPY,Buf2,sizeof(Buf2));
	wsprintf(Buf,Buf2,dwTotalSize);

	SetResultTextStr(Buf, g);

	ProcessMessages();

	Sleep(1000); //Sleep 1 second to allow user to see the new string

	//
	// Copy! (or test)
	//

	PxLoadString(IDS_STARTINGCOPY,Buf,sizeof(Buf));
	SetResultText(IDS_STARTINGCOPY, g);

	//Start up the progress engine
	StartProgress(&g->progress,dwTotalSize,dwSpeed,g->dwUnitsRec[0],FALSE,g);

	if (!LockAndBlockSource(g->dwUnitSource,TRUE,g->dwHandle))
	{
		EnableDisable(FALSE, g);
		///////
		return;
		///////
	}

	if (!LockAndBlock(g->dwUnitsRec,TRUE,g->dwHandle))
	{
		EnableDisable(FALSE, g);
		///////
		return;
		///////
	}

	dwErr = PrimoSDK_CopyDisc(g->dwHandle,g->dwUnitsRec,&g->dwUnitSource,
							  g->dwAction|PRIMOSDK_COPYPREGAP,dwSpeed);

	if (dwErr != PRIMOSDK_OK)
	{
		//
		// An error happened immediately, display it.
		//

		DisplayError(dwErr,_T("PrimoSDK_CopyDisc"),NULL);
		EnableDisable(FALSE, g);
		LockAndBlockSource(g->dwUnitSource,FALSE,g->dwHandle);
		LockAndBlock(g->dwUnitsRec,FALSE,g->dwHandle);
	}
	else
	{
		WaitForProcessComplete(g);

		//
		//Cleanup
		//

		LockAndBlockSource(g->dwUnitSource,FALSE,g->dwHandle);
		UnlockAndEjectAllDrives(g);

		//
		// Determine if we succeeded or not.
		//
		if (g->DeviceErrorDetected)
		{
			g->DeviceErrorDetected = FALSE;
		}

		dwRepStat = PrimoSDK_RunningStatus(g->dwHandle,g->bStop?PRIMOSDK_ABORT:PRIMOSDK_GETSTATUS,
										   &dwSize,&dwTotal);
		if (dwRepStat == PRIMOSDK_OK)
		{
			PxMessage(g->dwAction==PRIMOSDK_TEST?IDS_TESTOK:IDS_WRITEOK);
		}
		else // error during operation or aborted operation
		{
			DisplayError(dwRepStat,_T("PrimoSDK_RunningStatus"),NULL);
		}
	}

}



//-----------------------------------------------------------------------------
//
//  Record a CD from a Global Image or a other file.
//
//   Param: szImageName - file name of the source image file.
//
//          dwSpeed - defines the speed to use for recording:
//                    PRIMOSDK_MAX or n (like 8 for 8x) or PRIMOSDK_BEST
//                    if the drive supports AWS
//
//   Notes: Determines which method to use (track or GI) based on the
//          extension of the file name. files ending in ".gi" are assumed to be
//          Global Image files, all others are assumed to be ???track data.???
//
//          Uses the following globals:
//
//          dwUnitsRec -  list of recorder addresses terminated by NO_DRIVE
//
//
//  Return: None.
//
//-----------------------------------------------------------------------------
VOID RecordGIorTrack(TCHAR *szImageName, DWORD dwSpeed, PGLOBAL g)
{
	DWORD dwErr;
	DWORD dwRepStat, dwSize, dwTotal;


	//
	// Make sure that we have at least one recorder in the list
	//

	if (g->dwUnitsRec[0] == NO_DRIVE)
	{
		//
		// No recorders
		//

		PxMessage(IDS_NOCDRFOUND);
		///////
		return;
		///////
	}


	if (!LockAndBlock(g->dwUnitsRec,TRUE,g->dwHandle))
	{
		EnableDisable(FALSE, g);
		///////
		return;
		///////
	}

	//
	//Start up the progress engine
	//

	StartProgress(&g->progress,0,dwSpeed,g->dwUnitsRec[0],FALSE,g);

	//
	// If ".ISO" it is an iso image (another image), otherwise it is a gi
	//

	if (!_tcsrchr(szImageName,_T('.')) || _tcsicmp(_tcsrchr(szImageName,_T('.')),_T(".gi")) == 0)
	{
#ifdef _UNICODE
		dwErr = PrimoSDK_WriteGIWcs(g->dwHandle,g->dwUnitsRec,szImageName,g->dwAction,dwSpeed);
#else
		dwErr = PrimoSDK_WriteGI(g->dwHandle,g->dwUnitsRec,szImageName,g->dwAction,dwSpeed);
#endif
	}
	else
	{
#ifdef _UNICODE
		dwErr = PrimoSDK_WriteOtherCDImageWcs(g->dwHandle,g->dwUnitsRec,szImageName,
											  g->dwAction|PRIMOSDK_CLOSEDISC|
											  PRIMOSDK_IMAGE_M1_2048,dwSpeed);
#else
		dwErr = PrimoSDK_WriteOtherCDImage(g->dwHandle,g->dwUnitsRec,szImageName,
										   g->dwAction|PRIMOSDK_CLOSEDISC|
										   PRIMOSDK_IMAGE_M1_2048,dwSpeed);
#endif
	}

	if (dwErr != PRIMOSDK_OK)
	{
		//
		// Display the error.
		//

		DisplayError(dwErr,_T("PrimoSDK_WriteGI"),NULL);
		EnableDisable(FALSE, g);
		LockAndBlock(g->dwUnitsRec,FALSE,g->dwHandle);
	}
	else
	{

		WaitForProcessComplete(g);

		//
		// Cleanup
		//

		UnlockAndEjectAllDrives(g);

		//
		// Determine the operation status.
		//

		dwRepStat = PrimoSDK_RunningStatus(g->dwHandle,g->bStop?PRIMOSDK_ABORT:PRIMOSDK_GETSTATUS,
										   &dwSize,&dwTotal);
		if (dwRepStat == PRIMOSDK_OK)
		{
			PxMessage(g->dwAction==PRIMOSDK_TEST?IDS_TESTOK:IDS_WRITEOK);
		}
		else // Error during operation or aborted operation.
		{
			DisplayError(dwRepStat,_T("PrimoSDK_RunningStatus"),NULL);
		}
	}
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int           NumTracks;
TrackInfo     Tracks[99];
static  char  szName[256];

const char *AUDIO_FILE = "audio.dat" ;
//-----------------------------------------------------------------------------
//
//   Function : AudioFileExists
//
//-----------------------------------------------------------------------------
BOOL AudioFileExists()
{
	FILE *fp;

	if (!GetModuleFileNameA(NULL, szName, MAX_PATH+1))
		return FALSE;

	strcpy(strrchr(szName, '\\') + 1, AUDIO_FILE);

	fp = fopen(szName, "r+t");

	if (fp == NULL)
	{
		printf(" can't read audio.dat file\n");
		return FALSE;
	}

	fclose(fp);
	return TRUE;
}

//-----------------------------------------------------------------------------
//
//   Function : ReadAudioControlFile
//
//   File Name   : audio.dat in the current directory of sample_c.exe
//   File Format :
//                 [Track file name]  [PreEmphasis]  [CopyRight]  [Pregap Silence]  [Pregap Audio]  [Number of Track Indices]    [Index2....IndexN]
//   Example     : C:\\audio.wav       1 (Enable)    0 (Disable)   75 (sectors)      75 (sectors)              2                     50  75
//
//-----------------------------------------------------------------------------
BOOL ReadAudioControlFile()
{
	DWORD dwSilencePregap;
	DWORD dwAudioPregap;
	DWORD dwNumIndices = 0;
	FILE *fp;

	NumTracks = 0;

	if (!GetModuleFileNameA(NULL, szName, MAX_PATH+1))
		return FALSE;

	strcpy(strrchr(szName, '\\') + 1, AUDIO_FILE);

	fp = fopen(szName, "r+t");

	if (fp == NULL)
	{
		printf(" can't read audio.dat file\n");
		return FALSE;
	}

	fseek(fp, 0, SEEK_SET);

	while (!feof(fp))
	{
		DWORD i;
		BOOL  bPreEmp;
		BOOL  bCopy;

		fscanf(fp, "%s %i %i %i %i %i", szName, &bPreEmp, &bCopy, &dwSilencePregap, &dwAudioPregap, &dwNumIndices);

		if (szName[0] == 0)
			break;

		Tracks[NumTracks].FileName[0]     = 0;
#ifdef _UNICODE
		MultiByteToWideChar(GetACP(), MB_PRECOMPOSED, szName, -1, Tracks[NumTracks].FileName,
							sizeof(Tracks[NumTracks].FileName)/sizeof(Tracks[NumTracks].FileName[0]));
#else
		strcpy(Tracks[NumTracks].FileName, szName);
#endif
		Tracks[NumTracks].dwAudioPregap   = dwAudioPregap;
		Tracks[NumTracks].dwSilencePregap = dwSilencePregap;
		Tracks[NumTracks].dwNumIndices    = dwNumIndices;
		Tracks[NumTracks].bCopy           = bCopy;
		Tracks[NumTracks].bPreEmp         = bPreEmp;

		for (i = 0; i < dwNumIndices; i++)
			fscanf(fp, "%i", &Tracks[NumTracks].dwIndexArray[i]);

		NumTracks++;
		szName[0] = 0;
	}
	fclose(fp);
	return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Record a list of movie files onto a disc
//
//   Param: pszFiles[] - array of pointers to filenames. List terminated by NULL.
//
//          dwSpeed - defines the speed to use for recording:
//                     PRIMOSDK_MAX or n (like 8 for 8x) or PRIMOSDK_BEST
//                     if the drive supports AWS
//
//   Notes: Uses the following globals:
//
//          dwUnitsRec -  list of recorder addresses terminated by NO_DRIVE
//
//
//  Return: None.
//
//-----------------------------------------------------------------------------
VOID RecordVideo(TCHAR *Files[], DWORD dwSpeed, PGLOBAL g)
{

	UINT i;
	DWORD dwErr, dwTotalSize = 0;
	DWORD dwRepStat, dwSize, dwTotal;
	BYTE szBuf[2048],szBuf2[2048];
	TCHAR FormattedMsgStr[2048], MsgStr[2048];
	DWORD dwTotalFiles;

	//
	// Make sure that we have at least one recorder in the list
	//

	if (g->dwUnitsRec[0] == NO_DRIVE)
	{
		//
		// No recorders
		//

		PxMessage(IDS_NOCDRFOUND);
		///////
		return;
		///////
	}

	//
	// Make the new Video CD.
	//

	GetTempPathA(128,szBuf2);
	GetModuleFileNameA(NULL,szBuf,sizeof(szBuf));
	strcpy(strrchr(szBuf,'\\')+1,"Vcd.dta");

	if ((dwErr = PrimoSDK_NewVideoCD(g->dwHandle,g->dwUnitsRec,szBuf2,szBuf)) != PRIMOSDK_OK)
	{
		DisplayError(dwErr,_T("PrimoSDK_NewVideoCD"),NULL);
		EnableDisable(FALSE, g);
		///////
		return;
		///////
	}

	//
	// Load the files in the image
	//

	for (i=0, dwTotalFiles = 0; Files[i] != NULL ; i++)
	{
#ifdef _UNICODE
		dwErr = PrimoSDK_AddVideoCDStreamWcs(g->dwHandle,Files[i],&dwSize);
#else
		dwErr = PrimoSDK_AddVideoCDStream(g->dwHandle,Files[i],&dwSize);
#endif
		if (dwErr != PRIMOSDK_OK)
			break;
		dwTotalSize += dwSize;
	}

	if (dwErr != PRIMOSDK_OK)
	{
		DisplayError(dwErr,_T("PrimoSDK_AddVideoCDStream"),Files[i]);
		PrimoSDK_CloseVideoCD(g->dwHandle);
		EnableDisable(FALSE, g);
		///////
		return;
		///////
	}

	//
	// Display the needed sectors and wait a little while.
	//

	PxLoadString(IDS_SECTORSNEEDEDDISC,MsgStr,sizeof(MsgStr));
	wsprintf(FormattedMsgStr,MsgStr,dwTotalSize,g->dwTotEntries);

	SetResultTextStr(FormattedMsgStr, g); // Update text in result window

	Sleep(1000);

	if (!LockAndBlock(g->dwUnitsRec,TRUE,g->dwHandle))
	{
		EnableDisable(FALSE, g);
		PrimoSDK_CloseVideoCD(g->dwHandle);
		///////
		return;
		///////
	}

	//
	//Start up the progress engine
	//

	StartProgress(&g->progress,dwTotalSize,dwSpeed,g->dwUnitsRec[0],FALSE,g);

	//
	// WRITE (OR TEST) !
	//

	dwErr = PrimoSDK_WriteVideoCD(g->dwHandle,g->dwAction,dwSpeed);

	if (dwErr != PRIMOSDK_OK)
	{
		//
		// An error happened immediately, display it and destroy the image
		//

		DisplayError(dwErr,_T("PrimoSDK_WriteVideoCD"),NULL);
		EnableDisable(FALSE, g);
		PrimoSDK_CloseVideoCD(g->dwHandle);
		LockAndBlock(g->dwUnitsRec,FALSE,g->dwHandle);
	}
	else
	{

		WaitForProcessComplete(g);

		//
		//Cleanup
		//

		PrimoSDK_CloseVideoCD(g->dwHandle);
		UnlockAndEjectAllDrives(g);

		//
		// Determine the operation status.
		//

		dwRepStat = PrimoSDK_RunningStatus(g->dwHandle,g->bStop?PRIMOSDK_ABORT:PRIMOSDK_GETSTATUS,
										   &dwSize,&dwTotal);
		if (dwRepStat == PRIMOSDK_OK)
		{
			PxMessage(g->dwAction==PRIMOSDK_TEST?IDS_TESTOK:IDS_WRITEOK);
		}
		else // error during operation or aborted operation
		{
			DisplayError(dwRepStat,_T("PrimoSDK_RunningStatus"),NULL);
		}
	}
}



//-----------------------------------------------------------------------------
//
//  Erase a (rewritable) disc
//
//   Param: dwUnit -  address of recorder unit
//
//   Notes: None.
//
//  Return: None.
//
//-----------------------------------------------------------------------------
VOID EraseDisc(DWORD dwUnit, PGLOBAL g)
{
	DWORD dwErr;
	DWORD dwMediumType;                      // Disc Type
	DWORD dwFormatType;
	DWORD dwErasable;
	DWORD dwTracks;                          // Total tracks number
	DWORD dwUsed;
	DWORD dwFree;                            // Total free sectors
	DWORD dwRepStat, dwSize, dwTotal;

	//
	// Put the unit into a list, since these functions expect lists of drives.
	//

	g->dwUnitsRec[0] = dwUnit;
	g->dwUnitsRec[1] = NO_DRIVE; // end of list

	//
	// Request the disc's info
	//

	dwErr = PrimoSDK_DiscInfo(g->dwHandle,&dwUnit,&dwMediumType,&dwFormatType,&dwErasable,
							  &dwTracks,&dwUsed,&dwFree);

	if (dwErr != PRIMOSDK_OK)
	{
		DisplayError(dwErr,_T("PrimoSDK_DiscInfo"),NULL);
		///////
		return;
		///////
	}

	if (dwErasable == 0)
	{
		PxMessage(IDS_ISNOTERASABLE);
		///////
		return;
		///////
	}

	//
	// Safety message
	//

	if (PxMessageYesNo(IDS_AREYOUSURETOERASE, g->hDlgModeless) != IDYES)
	{
		///////
		return;
		///////
	}


	if (!LockAndBlock(g->dwUnitsRec,TRUE,g->dwHandle))
		///////
		return;
	///////

	dwErr = PrimoSDK_EraseMedium(g->dwHandle,g->dwUnitsRec,PRIMOSDK_ERASEQUICK);

	if (dwErr != PRIMOSDK_OK)
	{
		//
		// An error happened immediately, display it.
		//

		DisplayError(dwErr,_T("PrimoSDK_EraseMedium"),NULL);
		LockAndBlock(g->dwUnitsRec,FALSE,g->dwHandle);
	}
	else
	{
		EnableDisable(TRUE, g);

		WaitForProcessComplete(g);

		//
		//Cleanup
		//

		UnlockAndEjectAllDrives(g);

		//
		// Determine if we succeeded or not.
		//

		dwRepStat = PrimoSDK_RunningStatus(g->dwHandle,g->bStop?PRIMOSDK_ABORT:PRIMOSDK_GETSTATUS,
										   &dwSize,&dwTotal);
		if (dwRepStat == PRIMOSDK_OK)
		{
			PxMessage(IDS_ERASEOK);
		}
		else // Error during operation or aborted operation
		{
			DisplayError(dwRepStat,_T("PrimoSDK_RunningStatus"),NULL);
		}
	}
}

