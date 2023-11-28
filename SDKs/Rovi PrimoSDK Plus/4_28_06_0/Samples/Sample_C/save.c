//-----------------------------------------------------------------------------
// save.c
// Copyright (c) Sonic Solutions.  All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// PROJECT:         PrimoSDK C Sample
//
// DESCRIPTION:     Save-to-File Functions
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
#include "misc.h"


//-----------------------------------------------------------------------------
//
//  Read from the CD and write a  Global Image file
//
//   Param: szImageName - file name of the destination image file.
//
//   Notes: Uses the following globals:
//
//          dwUnitSource - address of source drive
//
//  Return: None.
//
//-----------------------------------------------------------------------------
VOID BuildGI(TCHAR *szImageName, PGLOBAL g)
{
	DWORD dwErr, dwRepStat;
	DWORD dwSize, dwTotal;

	if (!LockAndBlockSource(g->dwUnitSource,TRUE,g->dwHandle))
	{
		EnableDisable(FALSE, g);
		///////
		return;
		///////
	}

	//
	// BUILD
	//

	//
	// Start up the progress engine.
	//

	StartProgress(&g->progress,0,0,g->dwUnitSource,TRUE,g);


	//
	// Start reading the disc to the GI file.
	//

#ifdef _UNICODE
	{
		size_t NameLen = wcslen(szImageName);
		if (NameLen > 3 && wcsncmp(&szImageName[NameLen-3], L"ISO", 3) == 0)
			dwErr = PrimoSDK_MakeOtherCDImageWcs(g->dwHandle,&g->dwUnitSource,szImageName,0);
		else
			dwErr = PrimoSDK_MakeGIWcs(g->dwHandle,&g->dwUnitSource,szImageName,PRIMOSDK_COPYPREGAP);
	}
#else
	dwErr = PrimoSDK_ReadGI(g->dwHandle,&g->dwUnitSource,szImageName,PRIMOSDK_COPYPREGAP);
#endif

	if (dwErr != PRIMOSDK_OK)
	{

		//
		// An error happened creating the temporary.
		//

		DisplayError(dwErr,_T("PrimoSDK_ReadGI"),NULL);

		//
		// Delete the file, just in case it was still there.
		//

		DeleteFile(szImageName);
		EnableDisable(FALSE, g);
	}
	else
	{
		WaitForProcessComplete(g);

		//
		// Cleanup
		//

		LockAndBlockSource(g->dwUnitSource,FALSE,g->dwHandle);

		//
		// Determine if we succeeded or not.
		//

		dwRepStat = PrimoSDK_RunningStatus(g->dwHandle,g->bStop?PRIMOSDK_ABORT:PRIMOSDK_GETSTATUS,
										   &dwSize,&dwTotal);

		if (g->DeviceErrorDetected)
		{
			TCHAR Error[256] = {0};
			_stprintf(Error, _T("Error reported from drive: SC/SQ=%.2x/%.2x\n"),g->SenseCode, g->SenseQual);
			MessageBox(NULL, Error, _T("Device Error"), MB_OK);
			g->DeviceErrorDetected = FALSE;
		}
		else if (dwRepStat == PRIMOSDK_OK)
		{
			PxMessage(IDS_BUILDGIOK);
		}
		else // Error during operation or aborted operation
		{
			DisplayError(dwRepStat,_T("PrimoSDK_RunningStatus"),NULL);
		}
	}
}






//-----------------------------------------------------------------------------
//
//  Copy a list of data files onto a track image file (.ISO)
//
//   Param: szImageName - file name of destination the image file.
//
//          pszFiles[] - array of pointers to filenames. List terminated by NULL.
//
//          szVolumeName - Volume name to write to the image file
//
//          dwFSMode - one of PRIMOSDK_JOLIET, PRIMOSDK_UDF or PRIMOSDK_ISOLEVEL1 -
//                     format of the image file.
//
//   Notes: None.
//
//  Return: None.
//
//-----------------------------------------------------------------------------
VOID SaveData(TCHAR * szImageName, TCHAR *Files[], TCHAR *szVolumeName, DWORD dwFSMode, PGLOBAL g)
{
	UINT i;
	DWORD dwCDMode = PRIMOSDK_MODE1;
	DWORD dwDateMode = PRIMOSDK_ORIGDATE;
	DWORD dwCloseDisc = PRIMOSDK_CLOSEDISC;
	DWORD dwLoad = 0;
	DWORD dwErr, dwTotal;
	DWORD dwRepStat, dwSize;
	BYTE szBuf2[2048];
	TCHAR FormattedMsgStr[2048], MsgStr[2048];
	DWORD dwTotalFiles;

	GetTempPathA(128,szBuf2);

	//
	// Make the new image for the data disc (Add the flag PRIMOSDK_CHECKDUPLI
	//  to have every PrimoSDK_AddFolder and PrimoSDK_AddFile checked.)
	//

	g->dwUnitsRec[0] = NO_DRIVE;
#ifdef _UNICODE
	dwErr = PrimoSDK_NewImageWCS(g->dwHandle,g->dwUnitsRec,szVolumeName,dwLoad,
								 dwFSMode|dwCDMode|dwDateMode|dwCloseDisc|PRIMOSDK_CHECKDUPLI,
								 0,szBuf2);
#else
	dwErr = PrimoSDK_NewImage(g->dwHandle,g->dwUnitsRec,szVolumeName,dwLoad,
							  dwFSMode|dwCDMode|dwDateMode|dwCloseDisc|PRIMOSDK_CHECKDUPLI,
							  0,szBuf2);
#endif

	if (dwErr != PRIMOSDK_OK)
	{
		DisplayError(dwErr,_T("PrimoSDK_NewImage"),NULL);
		EnableDisable(FALSE, g);
		///////
		return;
		///////
	}

	//
	// Load the files into the image.
	//

	SetResultText(IDS_ADDING, g); // Update text in result window

	for (i=0, dwTotalFiles = 0; Files[i] != NULL ; i++)
	{
		dwErr = FileAdder(Files[i],&dwTotalFiles,MsgStr,FALSE,g);
		if (dwErr != PRIMOSDK_OK)
			break;
	}

	if (dwErr != PRIMOSDK_OK)
	{
		DisplayError(dwErr,_T("PrimoSDK_AddFolder or PrimoSDK_AddFile"),MsgStr);
		PrimoSDK_CloseImage(g->dwHandle);
		EnableDisable(FALSE, g);
		///////
		return;
		///////
	}

	SetResultText(IDS_PREMASTERING, g); // Update text in result window

	ProcessMessages();
	Sleep(1000);

	//
	// Write the image to file!
	//

#ifdef _UNICODE
	if ((dwErr = PrimoSDK_SaveImageWcs(g->dwHandle,szImageName,&dwTotal)) != PRIMOSDK_OK)
#else
	if ((dwErr = PrimoSDK_SaveImage(g->dwHandle,szImageName,&dwTotal)) != PRIMOSDK_OK)
#endif
	{
		//
		// An error happened immediately, display it and destroy the image
		//

		DisplayError(dwErr,_T("PrimoSDK_SaveImage"),NULL);
		PrimoSDK_CloseImage(g->dwHandle);
		EnableDisable(FALSE, g);
	}
	else
	{
		//
		// Show number of needed sectors.
		//

		PxLoadString(IDS_SECTORSNEEDED,MsgStr,sizeof(MsgStr));
		wsprintf(FormattedMsgStr,MsgStr,dwTotal,dwTotalFiles);
		SetResultTextStr(FormattedMsgStr, g); // Update text in result window

		ProcessMessages();

		WaitForProcessComplete(g);

		//
		//Cleanup
		//

		//
		// Determine the operation status.
		//

		dwRepStat = PrimoSDK_RunningStatus(g->dwHandle,g->bStop?PRIMOSDK_ABORT:PRIMOSDK_GETSTATUS,
										   &dwSize,&dwTotal);
		if (dwRepStat == PRIMOSDK_OK)
		{
			PxMessage(IDS_SAVEDATAOK);
		}
		else // Error during operation or aborted operation
		{
			DisplayError(dwRepStat,_T("PrimoSDK_RunningStatus"),NULL);
		}

		PrimoSDK_CloseImage(g->dwHandle);
	}
}

//-----------------------------------------------------------------------------
//
//  Extract one track from an audio CD to a .WAV file.
//
//   Param: szFileName - name of the destination .WAV file
//
//          dwTrack - Audio track to extract.
//
//   Notes: None.
//
//  Return: None.
//
//-----------------------------------------------------------------------------
VOID AudioExtract(TCHAR *szFilename, DWORD dwTrack, PGLOBAL g)
{
	DWORD dwErr, dwTotalSize = 0;
	DWORD dwRepStat, dwSize, dwTotal;
	TCHAR MsgStr[2048];

#ifdef _UNICODE
	dwErr = PrimoSDK_ExtractAudioTrackWcs(g->dwHandle,&g->dwUnitSource,dwTrack,szFilename,&dwTotalSize);
#else
	dwErr = PrimoSDK_ExtractAudioTrack(g->dwHandle,&g->dwUnitSource,dwTrack,szFilename,&dwTotalSize);
#endif

	if (dwErr != PRIMOSDK_OK)
	{
		//
		// An error happened immediately, display it and destroy the file
		//

		DisplayError(dwErr,_T("PrimoSDK_ExtractAudioTrack"),NULL);
	}
	else
	{
		//
		// The writing started ok, let the system go controlled by the timer
		//

		EnableDisable(TRUE, g);

		wsprintf(MsgStr,_T("%d sectors to extract..."),dwTotalSize);
		SetResultTextStr(MsgStr, g); // Update text in result window

		WaitForProcessComplete(g);

		//
		//Cleanup
		//

		LockAndBlockSource(g->dwUnitSource,FALSE,g->dwHandle);

		//
		// Determine the operation status.
		//

		dwRepStat = PrimoSDK_RunningStatus(g->dwHandle,g->bStop?PRIMOSDK_ABORT:PRIMOSDK_GETSTATUS,
										   &dwSize,&dwTotal);
		if (dwRepStat == PRIMOSDK_OK)
		{
			PxMessage(IDS_EXTRACTOK);
		}
		else // Error during operation or aborted operation
		{
			DisplayError(dwRepStat,_T("PrimoSDK_RunningStatus"),NULL);
		}
	}
}




//-----------------------------------------------------------------------------
//
//  Record a list of data files onto a Global Image
//
//   Param: szFileName - name of the destination Global Image file
//
//          szVolumeName - Name to write on the CD image
//
//          pszFiles - array of pointers to filenames. List terminated by NULL.
//
//          dwFSMode, dwCDMode ,dwDateMode, dwCloseDisc - see PrimoSDK_NewImage()
//                     dwFlags parameter.
//
//   Notes: None.
//
//  Return: None.
//
//-----------------------------------------------------------------------------
VOID SaveGI(TCHAR *szFilename, TCHAR *szVolumeName, TCHAR *Files[], DWORD dwFlags, PGLOBAL g)
{
	UINT i;
	BYTE szBuf2[2048];
	TCHAR FormattedMsgStr[2048], MsgStr[2048];
	DWORD dwErr, dwTotal;
	DWORD dwRepStat, dwSize;
	DWORD dwUnit = NO_DRIVE;
	DWORD dwTotalFiles;


	GetTempPathA(128,szBuf2);

#ifdef _UNICODE
	dwErr = PrimoSDK_NewImageWCS(g->dwHandle,&dwUnit,szVolumeName,0,
								 dwFlags|PRIMOSDK_CHECKDUPLI,
								 0,szBuf2);
#else
	dwErr = PrimoSDK_NewImage(g->dwHandle,&dwUnit,szVolumeName,0,
							  dwFlags|PRIMOSDK_CHECKDUPLI,
							  0,szBuf2);
#endif

	if (dwErr != PRIMOSDK_OK)
	{
		DisplayError(dwErr,_T("PrimoSDK_NewImage"),NULL);
		EnableDisable(FALSE, g);
		return;
	}

	//
	// Add each file into the the image
	//

	SetResultText(IDS_ADDING, g); // Update text in result window

	for (i=0, dwTotalFiles = 0; Files[i] != NULL ; i++)
	{
		dwErr = FileAdder(Files[i],&dwTotalFiles,MsgStr,FALSE,g);
		if (dwErr != PRIMOSDK_OK)
			break;
	}

	if (dwErr != PRIMOSDK_OK)
	{
		DisplayError(dwErr,_T("PrimoSDK_AddFolder or PrimoSDK_AddFile"),MsgStr);
		PrimoSDK_CloseImage(g->dwHandle);
		EnableDisable(FALSE, g);
		///////
		return;
		///////
	}


	ProcessMessages();

	Sleep(1000); //Sleep 1 second to allow user to see the new string

	//
	// WRITE!
	//

#ifdef _UNICODE
	if ((dwErr =  PrimoSDK_SaveGIWcs(g->dwHandle, szFilename, &dwSize)) != PRIMOSDK_OK)
#else
	if ((dwErr =  PrimoSDK_SaveGI(g->dwHandle, szFilename, &dwSize)) != PRIMOSDK_OK)
#endif
	{
		//
		// An error happened immediately. Display it and destroy the image.
		//

		DisplayError(dwErr,_T("PrimoSDK_SaveGI"),NULL);
		EnableDisable(FALSE, g);
		PrimoSDK_CloseImage(g->dwHandle);
	}
	else //SaveGI call started ok.
	{
		//
		// Show number of bytes needed.
		//

		PxLoadString(IDS_SECTORSNEEDED,MsgStr,sizeof(MsgStr));
		wsprintf(FormattedMsgStr,MsgStr,dwSize,dwTotalFiles);
		SetResultTextStr(FormattedMsgStr, g);

		ProcessMessages();

		//
		// The writing started ok, let the system go controlled by the timer.
		//

		WaitForProcessComplete(g);

		//
		//Cleanup
		//

		//
		// Determine the operation status.
		//

		dwRepStat = PrimoSDK_RunningStatus(g->dwHandle,g->bStop?PRIMOSDK_ABORT:PRIMOSDK_GETSTATUS,
										   &dwSize,&dwTotal);
		if (dwRepStat == PRIMOSDK_OK)
		{
			PxMessage(IDS_WRITEOK);
		}
		else // Error during operation or aborted operation
		{
			DisplayError(dwRepStat,_T("PrimoSDK_RunningStatus"),NULL);
		}

		PrimoSDK_CloseImage(g->dwHandle);
	}
}

