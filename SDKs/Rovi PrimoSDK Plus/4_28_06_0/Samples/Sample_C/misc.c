//-----------------------------------------------------------------------------
// misc.c
// Copyright (c) Sonic Solutions.  All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// PROJECT:         PrimoSDK C Sample
//
// DESCRIPTION:     Miscelaneous Functions
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



BOOL IsStreamEnabled(VOID)
{
	return FALSE;
}

DWORD AddFile(PBYTE lpszName, BOOL Unused, PGLOBAL g)
{
	return PrimoSDK_AddFile(g->dwHandle,SkipDriveUNC(lpszName),lpszName);
}


//-----------------------------------------------------------------------------
//
// RECURSIVE PROCEDURE. Adds the files to the disc image.
//
//   Param: szPath is the name of the current file or directory.
//             to add in the image (wildcards ok)
//
//   Notes: Returns immediately if an error occurs.
//
//  Return: Error code.
//
//-----------------------------------------------------------------------------
DWORD FileAdder(LPTSTR Path, PDWORD pdwTotalFiles, LPTSTR FileInError, BOOL UseStreamCallback, PGLOBAL g)
{
	DWORD dwRet = PRIMOSDK_OK;
	WIN32_FIND_DATA FindFileData;
	HANDLE hFindFile;
	TCHAR FileName[512], AddDir[512], Buf[512];
	LPTSTR pChr;

	//
	// Start the Find in this directory.
	//

	if ((hFindFile = FindFirstFile(Path,&FindFileData)) != INVALID_HANDLE_VALUE)
	{

		//
		// Loop for every file/dir in this directory
		//

		do
		{

			//
			// Skip the "." and ".." directories and remove any "*.*"
			//

			if (FindFileData.cFileName[0] == '.')
				continue;

			if (_tcslen(Path) == _tcslen(_tcstok(Path,_T("*"))))
				*((_tcsrchr(Path,'\\'))+1) = 0;


			//
			// What have we found?
			//

			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{

				// It's a directory, increment the scan counter and put in
				// the complete path name with the trailing backslash ('\')
				// into lpszName.

				wsprintf(FileName,_T("%s%s\\"),Path,FindFileData.cFileName);

				//
				// Now add all the directories in the path name, one by one, to the disc image
				//  and exit if an error occurs.
				//

				pChr = SkipDriveUNC(FileName) + 1;
				while (*pChr && _tcschr(pChr,'\\'))
				{
					pChr = _tcschr(pChr,'\\') + 1;
					memmove(AddDir,FileName,(pChr-FileName)*sizeof(*pChr));
					AddDir[pChr-FileName] = 0x00;

#ifdef _UNICODE
					dwRet = PrimoSDK_AddFolderWCS(g->dwHandle,SkipDriveUNC(AddDir));
#else
					dwRet = PrimoSDK_AddFolder(g->dwHandle,SkipDriveUNC(AddDir));
#endif

					if (dwRet != PRIMOSDK_OK && dwRet != PRIMOSDK_ALREADYEXIST)
					{
						_tcscpy(FileInError,SkipDriveUNC(AddDir));
						FindClose(hFindFile);
						return(dwRet);
					}
				}

				//
				// RECURSION! Add a '*.*' to the path name and call myself.
				//

				wsprintf(FileName,_T("%s*.*"),FileName);
				dwRet = FileAdder(FileName, pdwTotalFiles, FileInError, UseStreamCallback, g);     // Recursive call
				if (dwRet != PRIMOSDK_OK)
					break;
			}
			else
			{
				//
				// It's a file. Put the complete path to the file in  lpszName.
				//

				wsprintf(FileName,_T("%s%s"),Path,FindFileData.cFileName);

				//
				// If not aborting, show the name of the file that we are adding.
				//

				if (!g->bStop)
				{
					wsprintf(Buf,_T("Adding (%d): %s"),*pdwTotalFiles+1,FileName);
					SetResultTextStr(Buf, g);
				}

				//
				// Add the file to the disc image
				//  and exit if an error occurred.
				//

				dwRet = AddFile(FileName, UseStreamCallback, g);

				if (dwRet != PRIMOSDK_OK)
				{
					//
					// There was an error adding the file. It may be because the
					// directories that contain the folder don't exist yet.
					// Try to add all the folders one by one to the disc image,
					// Then try to add the file again. If it fails now, then exit.
					//

					pChr = SkipDriveUNC(FileName) + 1;

					while (*pChr && _tcschr(pChr,'\\'))
					{
						pChr = _tcschr(pChr,'\\') + 1;
						memmove(AddDir,FileName,(pChr-FileName)*sizeof(*pChr));
						AddDir[pChr-FileName] = 0x00;

#ifdef _UNICODE
						dwRet = PrimoSDK_AddFolderWCS(g->dwHandle,SkipDriveUNC(AddDir));
#else
						dwRet = PrimoSDK_AddFolder(g->dwHandle,SkipDriveUNC(AddDir));
#endif

						if (dwRet != PRIMOSDK_OK && dwRet != PRIMOSDK_ALREADYEXIST)
						{
							_tcscpy(FileInError,SkipDriveUNC(AddDir));
							FindClose(hFindFile);
							return(dwRet);
						}
					}

					//
					// Now that we have created all the folders, try to add the file again.
					//

					dwRet = AddFile(FileName, UseStreamCallback, g);

					if (dwRet != PRIMOSDK_OK)
					{
						_tcscpy(FileInError,SkipDriveUNC(FileName));
						FindClose(hFindFile);
						return(dwRet);
					}
				}
				(*pdwTotalFiles)++;

				//
				// Let all other processing go on and check if an abort is pending.
				//

				ProcessMessages();

				if (g->bStop)
				{
					FindClose(hFindFile);
					return(PRIMOSDK_USERABORT);
				}
			}
		}
		while (FindNextFile(hFindFile,&FindFileData));

		//
		// Exiting the loop.
		//

		FindClose(hFindFile);
	}
	return(dwRet);
}


//-----------------------------------------------------------------------------
//
//  Record a list of data files onto a disc
//
//   Param: VolumeName - Name to write on the CD
//
//          Files[] - array of pointers to filenames. List terminated by NULL.
//
//          dwLoad - ?????
//
//          dwFSMode, dwCDMode ,dwDateMode, dwCloseDisc - see PrimoSDK_NewImage()
//                     dwFlags parameter.
//
//          dwFileSwap - size of swap file to use
//
//          dwDVDPRQuick is 0 or PRIMOSDK_DVDPRQUICK to not force 30mm Lead Out
//                     when recording DVD+R disc
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
VOID RecordData(TCHAR *VolumeName, TCHAR *Files[], DWORD dwLoad, DWORD dwFSMode, DWORD dwCDMode,
				DWORD dwDateMode, DWORD dwCloseDisc, DWORD dwFileSwap, DWORD dwDVDPRQuick, DWORD dwSpeed,
				BOOL bVerify, BOOL UseStreamCallback, BOOL TotalOnly, PGLOBAL g)
{
	UINT i;
	char TempPath[2048];
	TCHAR MsgBuf[2048], FormattedMsgBuf[2048];
	DWORD dwErr, dwTotal;
	DWORD dwRepStat, dwSize;
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

	GetTempPathA(128,TempPath);

#ifdef _UNICODE
	dwErr = PrimoSDK_NewImageWCS(g->dwHandle,g->dwUnitsRec,VolumeName,dwLoad,
								 dwFSMode|dwCDMode|dwDateMode|dwCloseDisc|PRIMOSDK_CHECKDUPLI,
								 dwFileSwap,TempPath);
#else
	dwErr = PrimoSDK_NewImage(g->dwHandle,g->dwUnitsRec,VolumeName,dwLoad,
							  dwFSMode|dwCDMode|dwDateMode|dwCloseDisc|PRIMOSDK_CHECKDUPLI,
							  dwFileSwap,TempPath);
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
		dwErr = FileAdder(Files[i],&dwTotalFiles,MsgBuf,UseStreamCallback, g);
		if (dwErr != PRIMOSDK_OK)
			break;
	}

	if (dwErr != PRIMOSDK_OK)
	{
		DisplayError(dwErr,_T("PrimoSDK_AddFolder or PrimoSDK_AddFile"),MsgBuf);
		PrimoSDK_CloseImage(g->dwHandle);
		EnableDisable(FALSE, g);
		///////
		return;
		///////
	}

	if (TotalOnly)
	{
		DWORD UsedSectors = 0, NewSectors = 0, FreeSectors = 0;
		DWORD MediumType, MediumFormat, Erasable, Tracks, Used, Free;
		DWORD Unit = g->dwUnitsRec[0];
		EnableDisable(FALSE, g);
		if (PrimoSDK_DiscInfoEx(g->dwHandle, &Unit, 0, &MediumType, &MediumFormat,
								&Erasable, &Tracks, &Used, &Free) == PRIMOSDK_OK)
		{
			DWORD Size, LastSector;
			UsedSectors = Used;
			if (PrimoSDK_GetSpaceUsed(g->dwHandle, &Size, &LastSector) == PRIMOSDK_OK)
			{
				NewSectors = Size;
				FreeSectors = (Used+Free) - LastSector;
			}
		}

		_stprintf(FormattedMsgBuf, _T("Media totals (uncheck \"Total Only\" to burn media)\r\n")
				  _T("%d Sectors existing on media\r\n")
				  _T("%d Sectors to be written\r\n")
				  _T("%d Free sectors remaining\r\n"),
				  UsedSectors,
				  NewSectors,
				  FreeSectors);
		SetResultTextStr(FormattedMsgBuf, g); // Update text in result window
		PrimoSDK_CloseImage(g->dwHandle);
		///////
		return;
		///////
	}


	if (!LockAndBlock(g->dwUnitsRec,TRUE,g->dwHandle))
	{
		PrimoSDK_CloseImage(g->dwHandle);
		EnableDisable(FALSE, g);
		///////
		return;
		///////
	}
	//
	SetResultText(IDS_PREMASTERING, g); // Update text in result window

	ProcessMessages();

	Sleep(1000); //Sleep 1 second to allow user to see the new string

	//
	//Start up the progress engine
	//

	StartProgress(&g->progress,0,dwSpeed,g->dwUnitsRec[0],FALSE,g);

	//
	// WRITE (or test) !
	//

	if ((dwErr = PrimoSDK_WriteImage(g->dwHandle,g->dwAction|dwDVDPRQuick,dwSpeed,
									 &dwTotal)) != PRIMOSDK_OK)
	{
		//
		// An error happened immediately. Display it and destroy the image.
		//

		DisplayError(dwErr,_T("PrimoSDK_WriteImage"),NULL);
		EnableDisable(FALSE, g);
		PrimoSDK_CloseImage(g->dwHandle);
		LockAndBlock(g->dwUnitsRec,FALSE,g->dwHandle);
	}
	else //WriteImage call started ok.
	{
		//
		// Show number of  sectors needed.
		//

		PxLoadString(IDS_SECTORSNEEDED,MsgBuf,sizeof(MsgBuf));
		_stprintf(FormattedMsgBuf,MsgBuf,dwTotal,dwTotalFiles);
		SetResultTextStr(FormattedMsgBuf, g); // Update text in result window

		ProcessMessages();

		//
		// The writing started ok, let the system go controlled by the timer.
		//

		WaitForProcessComplete(g);

		if (bVerify)
		{
			EnableDisable(TRUE, g);

			g->dwFunction = FUNC_VERIFYDATA;

			StartProgress(&g->progress,0,dwSpeed,g->dwUnitsRec[0],TRUE,g);

			if ((dwErr = PrimoSDK_VerifyImage(g->dwHandle,dwSpeed)) != PRIMOSDK_OK)
			{
				//
				// An error happened immediately. Display it and destroy the image.
				//

				DisplayError(dwErr,_T("PrimoSDK_VerifyImage"),NULL);
				EnableDisable(FALSE, g);
				PrimoSDK_CloseImage(g->dwHandle);
				LockAndBlock(g->dwUnitsRec,FALSE,g->dwHandle);
				return;
			}

			WaitForProcessComplete(g);

			g->dwFunction = FUNC_RECORDDATA;
		}
		//
		// Cleanup
		//

		UnlockAndEjectAllDrives(g);

		//
		// Determine the operation status.
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
		else
			if (dwRepStat == PRIMOSDK_OK)
			{
				PxMessage(g->dwAction==PRIMOSDK_TEST?IDS_TESTOK:IDS_WRITEOK);
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
//  Record a list of .WAV files onto a disc
//
//   Param: Files[] - array of pointers to filenames. List terminated by NULL.
//
//          dwCloseDisc  0 or PRIMOSDK_CLOSEDISC if the disc must be closed.
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
VOID RecordAudio(TCHAR *Files[], DWORD dwCloseDisc, DWORD dwSpeed, BOOL UseStreamCallback, PGLOBAL g)
{
	UINT i;
	DWORD dwErr, dwTotalSize = 0;
	DWORD dwRepStat, dwSize, dwTotal;
	BYTE szBuf[2048], szBuf2[2048], szBuf3[2048];
	TCHAR FormattedMsgStr[512], MsgStr[512];
	DWORD dwTotalFiles;
	BOOL  bAudioFileRead = FALSE;

	bAudioFileRead = ReadAudioControlFile();

	if (!bAudioFileRead)   // use the files from the User input if not read from file
	{
		g->NumTracks = 0;

		for (i=0, dwTotalFiles = 0; Files[i] != NULL ; i++)
		{
			_tcscpy(g->Tracks[g->NumTracks].FileName, Files[i]);
			g->NumTracks++;
		}
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

	//
	// Make the new audio disc.
	//

	if ((dwErr = PrimoSDK_NewAudio(g->dwHandle,g->dwUnitsRec)) != PRIMOSDK_OK)
	{
		DisplayError(dwErr,_T("PrimoSDK_NewAudio"),NULL);
		EnableDisable(FALSE, g);
		///////
		return;
		///////
	}

	//
	// Load the files in the image.
	//

	for (i=0, dwTotalFiles = 0; (int)i < g->NumTracks ; i++)
	{
		if (bAudioFileRead == TRUE)    // for advanced Primo SDK Functionality
		{
			DWORD dwFlags = 0;

			if (g->Tracks[i].bPreEmp)
				dwFlags |= PRIMOSDK_EMPHASIS;

			if (g->Tracks[i].bCopy)
				dwFlags |= PRIMOSDK_COPYRIGHT;

			{
#ifdef _UNICODE
				BYTE FileName[512];
				WideCharToMultiByte(GetACP(),0,g->Tracks[i].FileName,-1,(LPSTR)FileName,
									sizeof(FileName),"_",NULL);
#else
				PBYTE FileName = g->Tracks[i].FileName;
#endif
				dwErr = PrimoSDK_AddAudioTrackEx(g->dwHandle,
												 FileName,
												 g->Tracks[i].dwSilencePregap,
												 g->Tracks[i].dwAudioPregap,    // audio pregap
												 dwFlags,                    // flags
												 &dwSize,                    // size (out)
												 NULL,                       // ISRC
												 g->Tracks[i].dwNumIndices,     // Index count
												 g->Tracks[i].dwIndexArray);    // index array
			}
		}
		else
		{
			{
#ifdef _UNICODE
				BYTE FileName[512];
				WideCharToMultiByte(GetACP(),0,g->Tracks[i].FileName,-1,(LPSTR)FileName,
									sizeof(FileName),"_",NULL);
#else
				PBYTE FileName = g->Tracks[i].FileName;
#endif
				dwErr = PrimoSDK_AddAudioTrack(g->dwHandle,FileName,150,&dwSize);
			}
		}

		if (dwErr != PRIMOSDK_OK)
			break;
		dwTotalSize += dwSize;
	}

	if (dwErr != PRIMOSDK_OK)
	{
		DisplayError(dwErr,_T("PrimoSDK_AddAudioTrack"),NULL);
		PrimoSDK_CloseAudio(g->dwHandle);
		EnableDisable(FALSE, g);
		///////
		return;
		///////
	}

	/*
	// TEST OF CD-TEXT, CREATE ARBITRARY CD-TEXT STRINGS AND ADD THEM
	strcpy(szBuf,"SDK - Disc Title\r\n");
	strcpy(szBuf2,"SDK - Disc Performer\n");
	strcpy(szBuf3,"SDK - Disc Composer\n");
	for ( dwCount=0; dwCount < dwTotEntries; dwCount++ ) {
	   sprintf(szBuf+strlen(szBuf),"SDK - Track %d Title\r\n",dwCount+1);
	   sprintf(szBuf2+strlen(szBuf2),"SDK - Track %d Performer\r\n",dwCount+1);
	   sprintf(szBuf3+strlen(szBuf3),"SDK - Track %d Composer\r\n",dwCount+1);
	}
	dwErr = PrimoSDK_AddCDText(dwHandle,0,szBuf,szBuf2,szBuf3);
	if ( dwErr != PRIMOSDK_OK ) {
	   DisplayError(dwErr,"PrimoSDK_AddCDText",NULL);
	   PrimoSDK_CloseAudio(dwHandle);
	   EnableDisable(FALSE, g);
	   break;
	}
	*/
	// TEST OF CD-TEXT, CREATE ARBITRARY CD-TEXT STRINGS AND ADD THEM
	{
		DWORD dwCount;

		strcpy(szBuf,"\r\n");
		strcpy(szBuf2,"\r\n");
		strcpy(szBuf3,"\r\n");
		for (dwCount=0; dwCount < g->dwTotEntries; dwCount++)
		{
			sprintf(szBuf+strlen(szBuf),"\r\n");
			sprintf(szBuf2+strlen(szBuf2),"\r\n");
			sprintf(szBuf3+strlen(szBuf3),"\r\n");
		}
//      dwErr = PrimoSDK_AddCDText(g->dwHandle,0,szBuf,szBuf2,szBuf3);
		dwErr = PrimoSDK_AddCDTextEJ(g->dwHandle,0,szBuf,szBuf2,szBuf3, szBuf, szBuf2, szBuf3);

		if (dwErr != PRIMOSDK_OK)
		{
			DisplayError(dwErr,_T("PrimoSDK_AddCDText"),NULL);
			PrimoSDK_CloseAudio(g->dwHandle);
			EnableDisable(FALSE, g);
			return;
		}
	}

	// Display the needed sectors and wait a little while.
	PxLoadString(IDS_SECTORSNEEDEDDISC,MsgStr,sizeof(MsgStr));
	wsprintf(FormattedMsgStr,MsgStr,dwTotalSize,g->dwTotEntries);
	SetResultTextStr(FormattedMsgStr, g); // Update text in result window

	Sleep(1000);


	if (!LockAndBlock(g->dwUnitsRec,TRUE,g->dwHandle))
	{
		EnableDisable(FALSE, g);
		PrimoSDK_CloseAudio(g->dwHandle);
		///////
		return;
		///////
	}

	//
	//Start up the progress engine
	//

	StartProgress(&g->progress,dwTotalSize,dwSpeed,g->dwUnitsRec[0],FALSE,g);

	//
	// Write (or test) !
	//
	if (bAudioFileRead == TRUE)
		dwErr = PrimoSDK_WriteAudioEx(g->dwHandle,g->dwAction | dwCloseDisc,dwSpeed, NULL);
	else
		dwErr = PrimoSDK_WriteAudio(g->dwHandle,g->dwAction|dwCloseDisc,dwSpeed);

	if (dwErr != PRIMOSDK_OK)
	{
		//
		// An error happened immediately, display it and destroy the image.
		//

		DisplayError(dwErr,_T("PrimoSDK_WriteAudio"),NULL);
		EnableDisable(FALSE, g);
		PrimoSDK_CloseAudio(g->dwHandle);
		LockAndBlock(g->dwUnitsRec,FALSE,g->dwHandle);
	}
	else
	{
		WaitForProcessComplete(g);

		//
		//Cleanup
		//

		PrimoSDK_CloseAudio(g->dwHandle);
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
		else // Error during operation or aborted operation
		{
			DisplayError(dwRepStat,_T("PrimoSDK_RunningStatus"),NULL);
		}
	}
}


//-----------------------------------------------------------------------------
//
//  Record a list of .WAV files onto a disc using track at once method.
//
//   Param: Files[] - array of pointers to filenames. List terminated by NULL.
//
//          dwCloseDisc  0 or PRIMOSDK_CLOSEDISC if the disc must be closed.
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
VOID RecordAudioTrackAtOnce(TCHAR *Files[], DWORD dwCloseDisc, DWORD dwSpeed, BOOL UseStreamCallback, PGLOBAL g)
{
	UINT i;
	DWORD dwErr, dwTotalSize = 0;
	DWORD dwTotalFiles;
	DWORD dwFlags;
	BOOL UseStandardWriteAudioTrack = TRUE;

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
	// Determine whether we write or just test.
	//

	if (g->dwAction==PRIMOSDK_TEST)
		dwFlags = PRIMOSDK_TEST;
	else
		dwFlags = PRIMOSDK_WRITE | PRIMOSDK_BURNPROOF;

	//
	// Write the CD file by file.
	//

	for (i=0, dwTotalFiles = 0; Files[i] != NULL ; i++)
	{

		// reset the busy flag before each track
		g->bBusy = TRUE;

		//
		// On the last track, set the CLOSEDISC flag to complete the disk session (if requested)
		//

		if (Files[i+1] == NULL)
			dwFlags |= dwCloseDisc;

		{
#ifdef _UNICODE
			BYTE FileName[512];
			WideCharToMultiByte(GetACP(),0,Files[i],-1,(LPSTR)FileName,
								sizeof(FileName),"_",NULL);
#else
			PBYTE FileName = Files[i];
#endif
			dwErr = PrimoSDK_WriteAudioTrack(g->dwHandle, g->dwUnitsRec, FileName,
											 dwFlags, dwSpeed);
		}

		if (dwErr == PRIMOSDK_OK)
		{
			WaitForProcessComplete(g);
		}
		else // some error writing audio track
		{
			break; // exit this loop
		}
	}

	if (dwErr != PRIMOSDK_OK)
	{
		if (UseStandardWriteAudioTrack)
			DisplayError(dwErr,_T("PrimoSDK_WriteAudioTrack"),Files[i]);
		else
			DisplayError(dwErr,_T("PrimoSDK_WriteAudioTrackStream"),Files[i]);
		EnableDisable(FALSE, g);
		LockAndBlock(g->dwUnitsRec,FALSE,g->dwHandle);
		///////
		return;
		///////
	}
	else // All files were recorded successfully.
	{

		//
		//Cleanup
		//

		UnlockAndEjectAllDrives(g);
		PxMessage(g->dwAction==PRIMOSDK_TEST?IDS_TESTOK:IDS_WRITEOK);
	}
}


typedef struct
{
	char *pcBuffer;
	DWORD dwTargetSector;
	DWORD dwExpectedSize;
} CACHE_ELEMENT;
#define NUM_ELEMENTS 6
#define BUFFER_SIZE  2352*16


//-----------------------------------------------------------------------------
//
//  Extracts an audio track (dwTrack) from CD and puts the raw data into a
//  file specified by the user (sFilename)
//
//-----------------------------------------------------------------------------
VOID AudioExtractBuffer(PBYTE szFilename, DWORD dwTrack, PGLOBAL g)
{
	DWORD dwStartSector;
	DWORD dwTotalSectors;
	DWORD dwTrackTotalSectors;
	DWORD dwErr;
	DWORD dwSessionNumber;
	DWORD dwTrackType;
	DWORD dwPreGap;
	int   iElement = 0, iCurElementIndex;
	DWORD dwCurSector;
	CACHE_ELEMENT pCache[NUM_ELEMENTS];
	int iFree;

	FILE *pFile = fopen(szFilename, "w+b");

	if (pFile == NULL)
		return;

	fseek(pFile, 0, SEEK_SET);

	dwErr = PrimoSDK_DiscInfo(g->dwHandle, &g->dwUnitSource,NULL,
							  NULL, NULL, NULL,
							  NULL, NULL);

	if (dwErr != PRIMOSDK_OK)
	{
		DisplayError(dwErr,_T("PrimoSDK_DiscInfo"),NULL);
		return;
	}

	dwErr = PrimoSDK_TrackInfo(g->dwHandle,dwTrack,&dwSessionNumber,&dwTrackType,&dwPreGap,&dwStartSector,&dwTrackTotalSectors);

	if (dwErr == PRIMOSDK_OK)
	{
		dwErr = PrimoSDK_ExtractAudioToBuffer(g->dwHandle, &g->dwUnitSource,
											  dwStartSector, dwTrackTotalSectors, PRIMOSDK_MAX, 0, 0, 0);
	}

	EnableDisable(TRUE, g);

	if (dwErr == PRIMOSDK_OK)
	{
		//------------------------------
		// queue up all the requests
		//------------------------------
		for (iElement = 0; iElement < NUM_ELEMENTS; iElement++)
		{
			pCache[iElement].pcBuffer = malloc(BUFFER_SIZE);
			dwErr = PrimoSDK_NextExtractAudioBuffer(g->dwHandle,
													pCache[iElement].pcBuffer, BUFFER_SIZE,
													&pCache[iElement].dwExpectedSize,
													&pCache[iElement].dwTargetSector);
			if (dwErr != PRIMOSDK_OK)
			{
				for (iFree = 0; iFree <= iElement; iFree++)
					free(pCache[iFree].pcBuffer);

				DisplayError(dwErr, _T("PrimoSDK_NextExtractAudioBuffer"), NULL);
			}
		}

		iCurElementIndex = 0;

		SetTimer(g->hDlgModeless,0,1000,NULL); // 1000 milliseconds, no callback.
		g->dwStatusCounter = 0xffffffff;
		//------------------------------
		// Main loop
		//------------------------------
		do
		{
			ProcessMessages();
			dwErr = PrimoSDK_RunningStatus(g->dwHandle, g->bStop?PRIMOSDK_ABORT:PRIMOSDK_GETSTATUS,
										   &dwCurSector, &dwTotalSectors);

			// check for a filled buffer
			if ((dwCurSector >= pCache[iCurElementIndex].dwTargetSector) &&
				(dwCurSector <  dwTrackTotalSectors))
			{
				// to do: the buffer is filled here now so do what you need to
				// pCache[iCurElementIndex.pcBuffer
				int iret = fwrite(pCache[iCurElementIndex].pcBuffer,
								  pCache[iCurElementIndex].dwExpectedSize, 1, pFile);     // dump to a file

				if (iret != 1)
					break;

				// enqueue the buffer to get more from data from the engine.
				dwErr = PrimoSDK_NextExtractAudioBuffer(g->dwHandle,
														pCache[iCurElementIndex].pcBuffer, BUFFER_SIZE,
														&pCache[iCurElementIndex].dwExpectedSize,
														&pCache[iCurElementIndex].dwTargetSector);

				iCurElementIndex = (iCurElementIndex + 1) % NUM_ELEMENTS;
			}
			else
			{
				Sleep(5);    // engine is still busy, sleep to lessen CPU usage for this thread
			}

		}
		while (dwCurSector < dwTrackTotalSectors || dwErr == PRIMOSDK_RUNNING);
		g->dwStatusCounter = 2;

		//------------------------------
		// flush out remaining buffers
		//------------------------------
		if (dwErr == PRIMOSDK_OK)
		{
			for (iFree = 0; iFree < NUM_ELEMENTS; iFree++)
			{
				if (pCache[iCurElementIndex].dwTargetSector <= dwTrackTotalSectors)
				{
					fwrite(pCache[iCurElementIndex].pcBuffer,
						   pCache[iCurElementIndex].dwExpectedSize, 1, pFile);     // dump to a file
				}
				iCurElementIndex = (iCurElementIndex + 1) % NUM_ELEMENTS;
			}
		}
		else
			DisplayError(dwErr,_T("PrimoSDK_RunningStatus"),NULL);

		//------------------------------
		// clean-up
		//------------------------------
		LockAndBlockSource(g->dwUnitSource,FALSE,g->dwHandle);

		for (iFree = 0; iFree < NUM_ELEMENTS; iFree++)
			free(pCache[iFree].pcBuffer);

		dwErr = PrimoSDK_RunningStatus(g->dwHandle, g->bStop?PRIMOSDK_ABORT:PRIMOSDK_GETSTATUS,
									   &dwCurSector, &dwTotalSectors);

		if (dwErr == PRIMOSDK_OK)
			PxMessage(IDS_EXTRACTOK);
		else
			DisplayError(dwErr,_T("PrimoSDK_RunningStatus"),NULL);

	}

	fclose(pFile);
	EnableDisable(FALSE, g);
}

