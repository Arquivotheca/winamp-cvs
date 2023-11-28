//-----------------------------------------------------------------------------
// misc.h
// Copyright (c) Sonic Solutions.  All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// PROJECT:         PrimoSDK C Sample
//
// DESCRIPTION:     Miscelaneous functions
//
//-----------------------------------------------------------------------------

BOOL IsStreamEnabled(VOID);
DWORD FileAdder(LPTSTR Path, PDWORD pdwTotalFiles, LPTSTR FileInError, BOOL UseStreamCallback, PGLOBAL g);
VOID RecordData(TCHAR *VolumeName, TCHAR *Files[], DWORD dwLoad, DWORD dwFSMode, DWORD dwCDMode,
				DWORD dwDateMode, DWORD dwCloseDisc, DWORD dwFileSwap, DWORD dwDVDPRQuick, DWORD dwSpeed,
				BOOL bVerify, BOOL UseStreamCallback, BOOL TotalOnly, PGLOBAL g);
VOID RecordAudio(TCHAR *Files[], DWORD dwCloseDisc, DWORD dwSpeed, BOOL UseStreamCallback, PGLOBAL g);
VOID RecordAudioTrackAtOnce(TCHAR *Files[], DWORD dwCloseDisc, DWORD dwSpeed, BOOL UseStreamCallback, PGLOBAL g);

