//-----------------------------------------------------------------------------
// pxsample_c.h
// Copyright (c) Sonic Solutions.  All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// PROJECT:         PrimoSDK C Sample
//
// DESCRIPTION:     Mini mastering application as a C example and a guide to
//                  use the PrimoSDK API.
//
//-----------------------------------------------------------------------------

// DATA TYPES

// Used to keep track of progress of a write operation.
// requires time.h header

// Values for dwPhase
#define PROGRESS_PREWRITE  0
#define PROGRESS_WRITE     1
#define PROGRESS_POSTWRITE 2
#define PROGRESS_VERIFY    3 // special case for verify rather than write

typedef struct _tagProgress
{
	DWORD dwSectorsToWrite;
	DWORD dwSpeed;             //relative speed (1 = 1x = 75 sectors/sec for CD, 637.5 sectors/sec for DVD)
	DWORD dwUnit;
	time_t tStart;
	time_t tWriteStart;
	time_t tWriteEnd;
	DWORD dwWriteTime;
	DWORD dwPhase; //PROGRESS_XXXX constants
	DWORD dwPercentComplete;
	DWORD dwTimeRemaining;
	DWORD dwStartSector;
	DWORD dwMedTypeEx;      //Type of media being written
} PROGRESS, * PPROGRESS;


//
// APPLICATION GLOBAL VARIABLES
//

typedef struct
{
	TCHAR FileName[256];
	DWORD dwSilencePregap;     // in sectors
	DWORD dwAudioPregap;       // in sectors
	DWORD dwNumIndices;
	DWORD dwIndexArray[99];
	BOOL  bPreEmp;
	BOOL  bCopy;
} TrackInfo;

typedef struct
{
	DWORD    dwHandle;                                       // An Handle for the operations
	DWORD    dwFunction;                                     // Current operation code
	DWORD    dwAction;                                       // Test or Write
	DWORD    dwHowManyReader;                                // How many drives to read (recorders count)
	DWORD    dwHowManyRecorder;                              // How many drives to record disc
	DWORD    dwTotEntries;                                   // Lines in the files listbox
	DWORD    dwUnitSource;                                   // Unit for reading from
	DWORD    dwUnitsRec[64+1];                               // Units for recording
	BOOL     bBusy;                                          // TRUE if running
	BOOL     bStop;                                          // TRUE when user stops
	PROGRESS progress;
	HWND     hWndMain;                                       // Main window handle
	HWND     hDlgModeless;                                   // Main modeless dialog handle
	DWORD    dwFileSwap;                                     // File size to put in the swap file
	DWORD    dwStatusCounter;
	BOOL     DeviceErrorDetected;
	DWORD    SenseCode;
	DWORD    SenseQual;
	int      NumTracks;
	TrackInfo Tracks[99];
	FILE    *CurFile;
	TCHAR   *CurFileName;
	UINT     LastIndex;           // init to 0xffffffff
	struct _CALLBACKNODE *CallbackList;
} GLOBAL, *PGLOBAL;

typedef struct _CALLBACKNODE
{
	struct _CALLBACKNODE *Next;
	PGLOBAL g;
	UINT Idx;
	TCHAR FileName[1];
} CALLBACKNODE;


//
// FUNCTION PROTOTYPES
//

// init.c
DWORD InitializeSDK(PDWORD pdwRel, LPTSTR Title);
VOID  UninitializeSDK(VOID);

// pxSample_C.c
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, PSTR pcl, int iCmdShow);
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DlgFnProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
DWORD GetAddressFromCombo(HWND hDlg, UINT uiIDCombo);
LPTSTR SkipDriveUNC(LPTSTR p);
VOID InitMainDialog(HWND hDlg, PGLOBAL g);
VOID UpdateProgress(HWND hDlg, PGLOBAL g);
VOID ExecuteCommand(HWND hDlg, UINT wParam, PGLOBAL g);

VOID DisplayError(DWORD dwError, LPTSTR Function, LPTSTR AdditionalParam);
VOID ProcessMessages(VOID);
VOID EnableDisable(BOOL bBusyParam, PGLOBAL g);
VOID PxMessage(UINT resID);
int  PxMessageYesNo(UINT resID, HWND hDlgModeless);
VOID PxMessageStr(LPTSTR Message, HWND hDlgModeless);
int  PxMessageStrYesNo(LPTSTR Message, HWND hDlgModeless);
VOID SetResultText(int resID, PGLOBAL g);
VOID SetResultTextStr(LPTSTR Text, PGLOBAL g);
VOID PxLoadString(UINT uID, LPTSTR lpBuffer, int nBufferMax);
VOID PxLoadStringA(UINT uID, LPSTR lpBuffer, int nBufferMax);
VOID WaitForProcessComplete(PGLOBAL g);

//functions.c
BOOL LockAndBlock(PDWORD pdwUnit, BOOL bLockIt, DWORD dwHandle);
BOOL LockAndBlockSource(DWORD dwUnitSource, BOOL bLockIt, DWORD dwHandle);
VOID UnlockAndEjectAllDrives(PGLOBAL g);

VOID DisplayUnitInfo(DWORD dwUnit, DWORD dwHandle);
VOID DisplayDiscInfo(DWORD dwUnit, DWORD dwHandle);
VOID GIInfo(TCHAR *GIFileName, PGLOBAL g);

BOOL IsUDF(DWORD dwUnit, DWORD dwHandle);
BOOL IsISO(DWORD dwUnit, DWORD dwHandle);
DWORD GetFileSystem(DWORD dwUnit, DWORD dwHandle);

DWORD EstimateProgress(PPROGRESS pProg, DWORD dwSectorsProcessed, DWORD dwTotalSectors, PGLOBAL g);
DWORD EstimateWriteTimes(PPROGRESS pProg, DWORD dwSectorsRemaining, PGLOBAL g);
DWORD StartProgress(PPROGRESS pProg, DWORD dwTotalSectors, DWORD dwSpeed, DWORD dwUnit, BOOL bVerify, PGLOBAL g);


//record.c
VOID RecordVideo(TCHAR *Files[], DWORD dwSpeed, PGLOBAL g);
VOID RecordGIorTrack(TCHAR *szImageName, DWORD dwSpeed, PGLOBAL g);
VOID CopyDisc(DWORD dwSpeed, PGLOBAL g);
VOID EraseDisc(DWORD dwUnit, PGLOBAL g);
BOOL ReadAudioControlFile();

//verify.c
VOID VerifyDisc(DWORD dwSpeed, PGLOBAL g);
VOID VerifyGlobalImage(TCHAR *szImageName, DWORD dwSpeed, PGLOBAL g);
VOID VerifyOtherImage(TCHAR *szImageName, DWORD dwSpeed, PGLOBAL g);

//save.c
VOID SaveData(TCHAR * szImageName, TCHAR *Files[], TCHAR * szVolumeName, DWORD dwFSMode, PGLOBAL g);
VOID SaveGI(TCHAR *szFilename, TCHAR *szVolumeName, TCHAR *Files[], DWORD dwFlags, PGLOBAL g);
VOID BuildGI(TCHAR *szImageName, PGLOBAL g);
VOID AudioExtract(TCHAR *szFilename, DWORD dwTrack, PGLOBAL g);
VOID AudioExtractBuffer(TCHAR *szFilename, DWORD dwTrack, PGLOBAL g);



//
// OPERATION CODES FOR THIS APPLICATION
//
//                                       Needs    Needs
//                                       Source   Recorder
#define     FUNC_COPY               1  // Yes      Yes
#define     FUNC_BUILDGI            2  // Yes      No
#define     FUNC_RECORDGIORTRACK    3  // No       Yes
#define     FUNC_VERIFYDISC         4  // Yes      Yes
#define     FUNC_VERIFYGI           5  // No       Yes
#define     FUNC_RECORDDATA         6  // No       Yes
#define     FUNC_SAVEDATA           7  // No       No
#define     FUNC_RECORDAUDIO        8  // No       Yes
#define     FUNC_RECORDVIDEO        9  // No       Yes
#define     FUNC_AUDIOEXTRACT       10 // Yes      No
#define     FUNC_ERASEDISC          11 // No       Yes
#define     FUNC_SAVEGI             12 // No       No
#define     FUNC_VERIFYOTHERIMAGE   13 // No       Yes
#define     FUNC_GIINFO             14 // No       No
#define     FUNC_LASTFUNCTION       14

#define     FUNC_VERIFYDATA         0x80000000 // No       Yes

// marker for end of list of drives, or no drive available
#define     NO_DRIVE 0xFFFFFFFF

// Identifiers for file system on CD.
#define FS_UNKNOWN 0
#define FS_ISO 1
#define FS_UDF 2

// default (1x) speeds in sectors per second
#define CD_SPEED (75)
#define DVD_SPEED (638)

// MACROS
// Return true if this function requires  a source drive.
#define NEEDS_SOURCE(fn)  ((fn == FUNC_COPY)       | \
						   (fn == FUNC_BUILDGI)    | \
						   (fn == FUNC_VERIFYDISC) | \
						   (fn == FUNC_AUDIOEXTRACT) )

// Return true if this function requires a recorder drive.
#define NEEDS_RECORDER(fn)((fn == FUNC_COPY)             | \
						   (fn == FUNC_RECORDGIORTRACK)  | \
						   (fn == FUNC_VERIFYDISC)       | \
						   (fn == FUNC_VERIFYGI)         | \
						   (fn == FUNC_RECORDDATA)       | \
						   (fn == FUNC_RECORDAUDIO)      | \
						   (fn == FUNC_RECORDVIDEO)      | \
						   (fn == FUNC_ERASEDISC)        | \
						   (fn == FUNC_VERIFYOTHERIMAGE) )


extern BOOL g_bStreamFsUsed;
extern CRITICAL_SECTION gCrit;

typedef struct
{
	char RiffTag[4];  // "RIFF"
	DWORD WavSize;
	char WavTag[8];   // "WAVEfmt "
	DWORD Unknown1;  // 16 = PCM
	WORD Unknown2;  // 1 = PCM
	WORD Channels;
	DWORD SampleFreq;
	DWORD BytesPerSec;
	WORD BytesByCapture;
	WORD BitsPerSample;
	char DataTag[4];  // "data"
	DWORD DataBytes;
} WAVHEADER;

