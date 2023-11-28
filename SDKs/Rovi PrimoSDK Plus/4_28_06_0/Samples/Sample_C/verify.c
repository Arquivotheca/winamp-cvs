//-----------------------------------------------------------------------------
// verify.c
// Copyright (c) Sonic Solutions.  All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// PROJECT:         PrimoSDK C Sample
//
// DESCRIPTION:     Verify Functions
//
//-----------------------------------------------------------------------------
// If you are a licensed user of PrimoSDK you are authorized to
// copy part or entirely the code of this example into your
// application.

#include <windows.h>
#include <commctrl.h>
#include <mbstring.h>
#include <time.h>
#include <tchar.h>

#include "resource.h"

#include "primosdk.h"
#include "pxsample_c.h"





//-----------------------------------------------------------------------------
//
//  Compare a list of recorded disc to the source disc.
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
VOID VerifyDisc(DWORD dwSpeed, PGLOBAL g)
{
	DWORD    dwErr;
	DWORD    dwMediumType;                      // Disc Type
	DWORD    dwTracks;                          // Total tracks number
	DWORD    dwFree;                            // Total free sectors
	DWORD    dwTotalSize;                       // Size of the image,
	DWORD    dwFormatType;
	DWORD    dwErasable;
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
	//  Check if the source drive contains a valid disc to verify.
	//

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
	// Set the recording status
	//

	EnableDisable(TRUE, g);

	//
	// Verify !
	//

	SetResultText(IDS_STARTINGVERIFY, g); // Update text in result window.

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

	StartProgress(&g->progress,dwTotalSize-dwFree,dwSpeed,g->dwUnitsRec[0],TRUE,g);

	dwErr = PrimoSDK_VerifyDisc(g->dwHandle,g->dwUnitsRec,&g->dwUnitSource,dwSpeed);

	if (dwErr != PRIMOSDK_OK)
	{
		//
		// An error happened immediately, display it.
		//

		DisplayError(dwErr,_T("PrimoSDK_VerifyDisc"),NULL);
		EnableDisable(FALSE, g);
		LockAndBlock(g->dwUnitsRec,FALSE,g->dwHandle);
	}
	else
	{
		//
		// The verify started ok, let the system go controlled by the timer.
		//

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
			PxMessage(IDS_VERIFYOK);
		}
		else // Error during operation or aborted operation
		{
			DisplayError(dwRepStat,_T("PrimoSDK_RunningStatus"),NULL);
		}
	}
}



//-----------------------------------------------------------------------------
//
//  Compare disc in the recorder(s) with a Global Image file
//
//   Param: szImageName - file name of the global image file.
//
//          dwSpeed - defines the speed to use for recording:
//                    PRIMOSDK_MAX or n (like 8 for 8x) or PRIMOSDK_BEST
//                    if the drive supports AWS
//
//   Notes: Uses the following globals:
//
//          dwUnitsRec -  list of recorder addresses terminated by NO_DRIVE
//
//
//  Return: None.
//
//-----------------------------------------------------------------------------
VOID VerifyGlobalImage(TCHAR *szImageName, DWORD dwSpeed, PGLOBAL g)
{

	DWORD    dwErr;
	DWORD    dwRepStat, dwSize, dwTotal;

	SetResultText(IDS_STARTINGVERIFY, g); // Update text in result window


	if (!LockAndBlock(g->dwUnitsRec,TRUE,g->dwHandle))
	{
		EnableDisable(FALSE, g);
		///////
		return;
		///////
	}

	//
	// Start up the progress engine.
	// Use 0 for total size since we don't know it yet.
	//

	StartProgress(&g->progress,0,dwSpeed,g->dwUnitsRec[0],TRUE,g);

#ifdef _UNICODE
	dwErr = PrimoSDK_VerifyGIWcs(g->dwHandle,g->dwUnitsRec,szImageName,dwSpeed);
#else
	dwErr = PrimoSDK_VerifyGI(g->dwHandle,g->dwUnitsRec,szImageName,dwSpeed);
#endif

	if (dwErr != PRIMOSDK_OK)
	{
		//
		// An error happened immediately, display it.
		//

		DisplayError(dwErr,_T("PrimoSDK_VerifyGI"),NULL);
		EnableDisable(FALSE, g);
		LockAndBlock(g->dwUnitsRec,FALSE,g->dwHandle);
	}
	else
	{
		//
		// The verify started ok, let the system go controlled by the timer.
		//

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
			PxMessage(IDS_VERIFYOK);
		}
		else // Error during operation or aborted operation
		{
			DisplayError(dwRepStat,_T("PrimoSDK_RunningStatus"),NULL);
		}
	}
}


//-----------------------------------------------------------------------------
//
//  Compare disc in the recorder(s) with an Image file
//
//   Param: szImageName - name of the image file.
//
//          dwSpeed - defines the speed to use for recording:
//                    PRIMOSDK_MAX or n (like 8 for 8x) or PRIMOSDK_BEST
//                    if the drive supports AWS
//
//   Notes: Uses the following globals:
//
//          dwUnitsRec -  list of recorder addresses terminated by NO_DRIVE
//
//
//  Return: None.
//
//-----------------------------------------------------------------------------
VOID VerifyOtherImage(TCHAR *szImageName, DWORD dwSpeed, PGLOBAL g)
{

	DWORD    dwErr;
	DWORD    dwRepStat, dwSize, dwTotal;

	SetResultText(IDS_STARTINGVERIFY, g);


	if (!LockAndBlock(g->dwUnitsRec,TRUE,g->dwHandle))
	{
		EnableDisable(FALSE, g);
		///////
		return;
		///////
	}

	//
	// Start up the progress engine.
	// Use 0 for total size since we don't know it yet.
	//

	StartProgress(&g->progress,0,dwSpeed,g->dwUnitsRec[0],TRUE,g);

#ifdef _UNICODE
	dwErr = PrimoSDK_VerifyOtherCDImageWcs(g->dwHandle,g->dwUnitsRec,szImageName,PRIMOSDK_IMAGE_M1_2048,dwSpeed);
#else
	dwErr = PrimoSDK_VerifyOtherCDImage(g->dwHandle,g->dwUnitsRec,szImageName,PRIMOSDK_IMAGE_M1_2048,dwSpeed);
#endif

	if (dwErr != PRIMOSDK_OK)
	{
		//
		// An error happened immediately, display it.
		//

		DisplayError(dwErr,_T("PrimoSDK_VerifyGI"),NULL);
		EnableDisable(FALSE, g);
		LockAndBlock(g->dwUnitsRec,FALSE,g->dwHandle);
	}
	else
	{
		//
		// The verify started ok, let the system go controlled by the timer.
		//

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
			PxMessage(IDS_VERIFYOK);
		}
		else // Error during operation or aborted operation
		{
			DisplayError(dwRepStat,_T("PrimoSDK_RunningStatus"),NULL);
		}
	}
}

