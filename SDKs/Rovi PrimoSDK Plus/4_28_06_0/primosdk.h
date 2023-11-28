//-----------------------------------------------------------------------------
// primosdk.h
// Copyright (c) Sonic Solutions.  All rights reserved.
//-----------------------------------------------------------------------------


////////////////////////////////////////////////////////////////////
//                                                                //
//                                                                //
//                   P  r  i  m  o  S  D  K                       //
//                  ========================                      //
//                                                                //
//                                                                //
// CD and DVD mastering API for Windows 9x, ME, NT, 2000 and XP.  //
//                                                                //
// The use of this SDK must be done only under signed License     //
// Agreement with Sonic Solutions.                                //
//                                                                //
// This computer program is protected by copyright law and        //
// international treaties. Unauthorized reproduction or           //
// distribution of this program, or any portion of it, may result //
// in severe civil and criminal penalties.                        //
//                                                                //
// Copyright (c) Sonic Solutions                                  //
//                                                                //
////////////////////////////////////////////////////////////////////


#ifdef __cplusplus
extern "C"
{
#endif


typedef HRESULT(__cdecl *PrimoSDK_CreateWmaReader)(DWORD  dwRights,           // Rights param (see WMA Documentation)
		void*  pReader);         // Pointer to the receiving pointer

// REPLY CODES
//
//    (FOR EXPLANATION OF EACH REPLY CODE, PLEASE SEE BELOW IN THE DOCUMENTATION OF
//     THE FUNCTION THAT IS RETURNING THE REPLY YOU LOOK FOR)
//
#define  PRIMOSDK_OK             0
#define  PRIMOSDK_CMDSEQUENCE    1
#define  PRIMOSDK_NOASPI         2
#define  PRIMOSDK_NO_DRIVER      2
#define  PRIMOSDK_INTERR         3
#define  PRIMOSDK_BADPARAM       4
#define  PRIMOSDK_ALREADYEXIST   6
#define  PRIMOSDK_NOTREADABLE    7
#define  PRIMOSDK_NOSPACE        8
#define  PRIMOSDK_INVALIDMEDIUM  9
#define  PRIMOSDK_RUNNING        10
#define  PRIMOSDK_BUR            11
#define  PRIMOSDK_SCSIERROR      12
#define  PRIMOSDK_UNITERROR      13
#define  PRIMOSDK_NOTREADY       14
#define  PRIMOSDK_INVALIDSOURCE  16
#define  PRIMOSDK_INCOMPATIBLE   17
#define  PRIMOSDK_FILEERROR      18
#define  PRIMOSDK_ITSADEMO       23
#define  PRIMOSDK_USERABORT      24
#define  PRIMOSDK_BADHANDLE      25
#define  PRIMOSDK_BADUNIT        26
#define  PRIMOSDK_ERRORLOADING   27
#define  PRIMOSDK_NOAINCONTROL   29
#define  PRIMOSDK_READERROR      30
#define  PRIMOSDK_WRITEERROR     31
#define  PRIMOSDK_TMPOVERFLOW    32
#define  PRIMOSDK_DVDSTRUCTERROR 33
#define  PRIMOSDK_FILETOOLARGE   34
#define  PRIMOSDK_CACHEFULL      35
#define  PRIMOSDK_FEATURE_NOT_SUPPORTED 36
#define  PRIMOSDK_FEATURE_DISABLED 37
#define  PRIMOSDK_CALLBACK_ERROR   38         // returned from the callers callback function to terminate stream
#define  PRIMOSDK_PROTECTEDWMA     39
#define  PRIMOSDK_LIMITEXPIRED       40
#define  PRIMOSDK_INVALIDPROPERTY    41
#define  PRIMOSDK_NEEDFULLERASE      42

// Parameter used for PrimoSDK_InitStreamFileSystem
#define  PRIMOSDK_INIT_AUTO_DETECT_DRIVER 0x00000001     // similar in behavior to PrimoSDK_Init
#define  PRIMOSDK_INIT_SELECT_BEST_DRIVER 0x00000002     // pick only the best driver, reports PRIMOSDK_NO_DRIVER otherwise
#define  PRIMOSDK_INIT_SELECT_IMAPI2      0x00000004     // Force usage of IMAPI2, reports PRIMOSDK_NO_DRIVER if IMAPI2 is not available.
#define  PRIMOSDK_INIT_SELECT_SPTI         0x00000008     // Force usage of SPTI (or ASPI on Win9x)
#define  PRIMOSDK_INIT_STREAM_FILE_SYSTEM 0x80000000     // select stream file system (This flag is always selected for calls to PrimoSDK_InitStreamFileSystem)

//
// UNIT TYPES AND MEDIA TYPES
//
#define  PRIMOSDK_CDROM          0x00000201
#define  PRIMOSDK_CDR            0x00000202
#define  PRIMOSDK_CDRW           0x00000203
#define  PRIMOSDK_DVDR           0x00000204
#define  PRIMOSDK_DVDROM         0x00000205
#define  PRIMOSDK_DVDRAM         0x00000206
#define  PRIMOSDK_DVDRW          0x00000207
#define  PRIMOSDK_DVDPRW         0x00000209
#define  PRIMOSDK_DVDPR          0x00000210
#define  PRIMOSDK_DDCDROM        0x00000211
#define  PRIMOSDK_DDCDR          0x00000212
#define  PRIMOSDK_DDCDRW         0x00000213
#define  PRIMOSDK_DVDPR9         0x00000214    // dual-layer DVD+R
#define  PRIMOSDK_DVDR9          0x00000215    // dual-layer DVD-R
#define  PRIMOSDK_BDRE           0x00000216    // rewritable BD.
#define  PRIMOSDK_BDR            0x00000217    // Write-Once BD.
#define  PRIMOSDK_BDROM          0x00000218
#define  PRIMOSDK_HDDVDRW        0x00000219    // rewritable HD media
#define  PRIMOSDK_HDDVDROM       0x00000221
#define  PRIMOSDK_HDDVDR         0x00000222    // Write-Once HD
#define  PRIMOSDK_HDDVDR_DL      0x00000223    // Dual layer Write-Once HD
#define  PRIMOSDK_DVDRW9         0x00000224    // Dual layer DVD-RW

//
#define  PRIMOSDK_ROBOTICS       0x00000208
#define  PRIMOSDK_OTHER          0x00000220

//
// DISC TYPES
//
//    (FOR EXPLANATION, PLEASE SEE BELOW IN THE DOCUMENTATION OF THE PrimoSDK_DiscInfo)
//
#define  PRIMOSDK_SILVER         0x00000301
#define  PRIMOSDK_COMPLIANTGOLD  0x00000302
#define  PRIMOSDK_OTHERGOLD      0x00000303
#define  PRIMOSDK_BLANK          0x00000304

//
// BUS TYPES
//
//
#define  PRIMOSDK_BUS_UNKNOWN    0
#define  PRIMOSDK_ATAPI          1
#define  PRIMOSDK_SCSI           2
#define  PRIMOSDK_1394           3
#define  PRIMOSDK_USB            4
#define  PRIMOSDK_USB2           5

//
// DISC FORMAT CODING
//
//    (FOR EXPLANATION, PLEASE SEE BELOW IN THE DOCUMENTATION OF THE PrimoSDK_DiscInfo)
//
#define  PRIMOSDK_GENERICCD      0x000000C1
#define  PRIMOSDK_B1             0x000000B1
#define  PRIMOSDK_D1             0x000000D1
#define  PRIMOSDK_D2             0x000000D2
#define  PRIMOSDK_D3             0x000000D3
#define  PRIMOSDK_D4             0x000000D4
#define  PRIMOSDK_D5             0x000000D5
#define  PRIMOSDK_D6             0x000000D6
#define  PRIMOSDK_D7             0x000000D7
#define  PRIMOSDK_D8             0x000000D8
#define  PRIMOSDK_D9             0x000000D9
#define  PRIMOSDK_A1             0x000000A1
#define  PRIMOSDK_A2             0x000000A2
#define  PRIMOSDK_A3             0x000000A3
#define  PRIMOSDK_A4             0x000000A4
#define  PRIMOSDK_A5             0x000000A5
#define  PRIMOSDK_M1             0x000000E1
#define  PRIMOSDK_M2             0x000000E2
#define  PRIMOSDK_M3             0x000000E3
#define  PRIMOSDK_M4             0x000000E4
#define  PRIMOSDK_M5             0x000000E5
#define  PRIMOSDK_M6             0x000000E6
#define  PRIMOSDK_F1             0x000000F1
#define  PRIMOSDK_F2             0x000000F2
#define  PRIMOSDK_F3             0x000000F3
#define  PRIMOSDK_F4             0x000000F4
#define  PRIMOSDK_F5             0x000000F5
#define  PRIMOSDK_F8             0x000000F8
#define  PRIMOSDK_FA             0x000000FA

//
// TRACK TYPES
//
//

#define PRIMOSDK_AUDIO_TRACK   0
#define PRIMOSDK_MODE1_TRACK   1
#define PRIMOSDK_MODE2_TRACK   2


//
// FLAGS
//
//    (FOR EXPLANATION, PLEASE SEE BELOW IN THE DOCUMENTATION OF THE VARIOUS FUNCTIONS
//     THAT LIST EACH FLAG THEY USE)
//
#define  PRIMOSDK_OPENTRAYEJECT           0x00000001
#define  PRIMOSDK_CLOSETRAY               0x00000002
#define  PRIMOSDK_LOCK                    0x00000004
#define  PRIMOSDK_UNLOCK                  0x00000008
#define  PRIMOSDK_TEST                    0x00000010
#define  PRIMOSDK_WRITE                   0x00000020
#define  PRIMOSDK_IMMEDIATE               0x00000040
#define  PRIMOSDK_BURNPROOF               0x00000080
#define  PRIMOSDK_HIGHDENSITY             0x80000000    // used to be 0x100 but that conflicts with ISOLEVEL1 below.
#define  PRIMOSDK_COPYRIGHT               0x00000200
#define  PRIMOSDK_EMPHASIS                0x00000400
#define  PRIMOSDK_ALLOW_NONSTANDARD_LAYER 0x00008000 // PrimoSDK_WriteImage flag to allow non-compliant layer break on DVD Video
#define  PRIMOSDK_FORCE_REFRESH           0x00010000
#define  PRIMOSDK_FAST_WRITE              0x80000000 // PrimoSDK_WriteImage flag to make drive write file data without read-after-write
#define  PRIMOSDK_VNR_WRITE               0x00000008 // write to disc faster by using "Verify Not Required" mode (when supported)

//
// Mastering flags
//
#define  PRIMOSDK_ISOLEVEL1               0x00000100
#define  PRIMOSDK_JOLIET                  0x00000200
#define  PRIMOSDK_UDF                     0x00000400
#define  PRIMOSDK_DVDPRQUICK              0x00000800
#define  PRIMOSDK_ORIGDATE                0x00001000
#define  PRIMOSDK_USERTIMESET             0x00001000 // use for streamed files since they don't have an "original date"
#define  PRIMOSDK_SETNOW                  0x00002000
#define  PRIMOSDK_MODE1                   0x00004000
#define  PRIMOSDK_MODE2                   0x00008000 // ignored when writing UDF file system and writes Mode 1
#define  PRIMOSDK_CLOSEDISC               0x00010000
#define  PRIMOSDK_COPYPREGAP              0x00020000
#define  PRIMOSDK_NOPREGAP                0x00040000
#define  PRIMOSDK_RESETDRIVES             0x00080000
#define  PRIMOSDK_UDF201                  0x00100000
#define  PRIMOSDK_ISOLEVEL2               0x00200000
#define  PRIMOSDK_ISOLEVEL3               0x00400000 //aka ISO Level 2 long (long filenames)
#define  PRIMOSDK_SAO                     0x00800000
#define  PRIMOSDK_TAO                     0x01000000
#define  PRIMOSDK_VIDEOCD                 0x02000000
#define  PRIMOSDK_CHECKDUPLI              0x04000000
#define  PRIMOSDK_DVDIMAGE                0x08000000
#define  PRIMOSDK_DVDRWCOMPAT             0x08000000 // Allow SAO and long close on RO.
#define  PRIMOSDK_VERSIONLESS_ISO         0x10000000 //for ETFSBOOT.COM boot CDs only
#define  PRIMOSDK_BAD_ISOLEVEL1_NOVERSION 0x10000000 //for ETFSBOOT.COM boot CDs only
#define  PRIMOSDK_PRESERVE_ISO_VARIATIONS 0x20000000 //only for appending to media that is out
//of ISO spec and you want to preserve
//the existing file system as is.
#define  PRIMOSDK_UDF250                  0x40000000
#define  PRIMOSDK_UDF260                  0x00000080

//
#define  PRIMOSDK_IMAGE_M1_2048  0x00100000
#define  PRIMOSDK_IMAGE_M2_2336  0x00200000
#define  PRIMOSDK_IMAGE_M2_2352  0x00400000
//
#define  PRIMOSDK_GETSTATUS      0x01000000
#define  PRIMOSDK_ABORT          0x02000000
//
#define  PRIMOSDK_SSCLASS        0x00001F40
//
#define  PRIMOSDK_MAX            0x00000000
#define  PRIMOSDK_MEDIUM         0xFFFFF000
#define  PRIMOSDK_MIN            0xFFFFFF00
#define  PRIMOSDK_BEST           0xFFFFFFF0
//
#define  PRIMOSDK_DEMOVERSION    0x00000401
#define  PRIMOSDK_CDDVDVERSION   0x00000404
//
#define  PRIMOSDK_ERASEQUICK     0x00000001
#define  PRIMOSDK_ERASEFULL      0x00000002
#define  PRIMOSDK_ERASELAST      0x00000004
//
#define  PRIMOSDK_FLOPPY144      144
#define  PRIMOSDK_FLOPPY288      288
#define  PRIMOSDK_FLOPPY12       0x04000000
#define  PRIMOSDK_HD             0x02000000
#define  PRIMOSDK_NOEMULATION    0x01000000
#define  PRIMOSDK_NOEMULATION_WITH_SET_BOOT_INFO 0x11000000
//
#define  PRIMOSDK_NODATA         0x00000000
#define  PRIMOSDK_DATAIN         0x00000001
#define  PRIMOSDK_DATAOUT        0x00000002
//
#define  PRIMOSDK_DVDUNKNOWN         0x00000000
#define  PRIMOSDK_DVDDATA            0x00000001
#define  PRIMOSDK_DVDAUDIO           0x00000002
#define  PRIMOSDK_DVDVIDEO           0x00000004
#define  PRIMOSDK_DVDVR              0x00000008
#define  PRIMOSDK_DVDSTREAM          0x00000010
#define  PRIMOSDK_DEFECTMAPPING      0x00000020
//
#define  PRIMOSDK_PACKETWRITTEN  0x00000001

#define  PRIMOSDK_AUDIO_PREEMPHASIS 0x00000001
#define  PRIMOSDK_AUDIO_COPYRIGHT   0x00000002

#define  PRIMOSDK_STARTTRACK     0x00000001

// UnitInfo2 features
#define  PRIMOSDK_UNITFEATURE_DAP    0x00000001 // supports Digital Audio Play of CDDA tracks


// Function typedef for streaming callback function.
//	Used in advanced functionality PrimoSDK_AddAudioStream and PrimoSDK_AddFileStreamWCS
// This function will be called repeatedly until all file data has been read.
//
// pBuffer          - buffer containing the user data of the stream
// dwBytesRequested - the engine requesting that number of bytes to be filled in pBuffer.
// pdwBytesWritten  - pointer to the numbers of bytes provided by the client of this SDK.
//
// NOTE: The value dwBytesRequested must equal to dwBytesWritten otherwise it shall be
// considered an error.
//
typedef DWORD (__cdecl *PrimoSDK_StreamCallback)(PBYTE pBuffer, DWORD dwBytesRequested,
		PDWORD pdwBytesWritten, PVOID pContext);


DWORD WINAPI PrimoSDK_Trace(DWORD dwTrace);
//          ================
//
// Set the trace mode.
//
// This function can be called at any time to enable or
//  disable the PrimoSDK trace.
//
//   Param: dwTrace sets the debug trace mode. If not 0 all the calls to
//          PrimoSDK are logged in the text file <appname>.DBG
//
//   Notes: It could be helpful to introduce a backdoor in your
//          application to activate the trace at user level.
//          In case of need, the customer can activate the
//          trace and then send the log file to you for debug.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_End(VOID);
//          ==============
//
// Terminate PrimoSDK.
//
// This function must be called after all the other PrimoSDK calls
// have been terminated. It frees the internal structures.
//
//   Notes: You should call this function only once, when your
//          application or module terminates.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_INTERR if an internal error occurred
//



DWORD WINAPI PrimoSDK_GetHandle(PDWORD pdwHandle);
//          ====================
//
// Obtains an handle to use in all the PrimoSDK call.
//
// The PrimoSDK_GetHandle function returns an handle that must be used
// in any call to PrmoSDK. An handle is released calling PrimoSDK_ReleaseHandle.
//
//   Param: pdwHandlde points to a DWORD that receives the new handle
//
//   Notes: You can obtain as many handle as you need, to perform
//          different simultaneous operations on different drives.
//
//  Return: PRIMOSDK_OK if no error, the handle has been created
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_INTERR if an internal error occurred
//



DWORD WINAPI PrimoSDK_ReleaseHandle(DWORD dwHandle);
//          ========================
//
// Release an handle that has been obtained with PrimoSDK_GetHandle.
//
//   Param: dwHandle is the handle to release
//
//   Notes: None.
//
//  Return: PRIMOSDK_OK if no error, the handle has been created
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occurred
//



DWORD WINAPI PrimoSDK_UnitInfo(DWORD dwHandle, PDWORD pdwUnit, PDWORD pdwType,
							   PBYTE szDescr, PDWORD pdwReady);
//          ===================
//
// Retrieve information about a unit.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification.
//                The caller can identify a unit by drive letter, as "D", or
//                SCSI Host/ID/LUN triple, as "130". If the least significant byte of the
//                DWORD is not 00 or 20 (blank), then it should contain the letter. If it
//                is 00 or 20, the other 3 bytes contain the triple. For instance:
//                to call for unit D:, pdwUnit will point to 0x00000044; instead
//                to call for unit at Host 1, ID 3, Lun 0, pdwUnit will
//                point to 0x01030000. When the function returns, the field is
//                completed by PrimoSDK, therefore, in both the previous examples,
//                it will become 0x01030044.
//                If the least significant byte is 20, instead of 00,
//                it will still go for triple, but the letter is not forced
//                by PrimoSDK inside the field that, therefore, remains unchanged.
//                For recorders of SCSI Type 4, that do not have an assigned
//                drive letter from the system, like the Yamaha CDR100,
//                or when the drives letter setting (especially under NT/2000) is
//                not completely or correctly seen by the ASPI layer,
//                the call must be done by triple only, and the last byte
//                will always be 00.
//                This format is used for any PrimoSDK call that specifies a unit.
//
//          pdwType returns the unit type:
//                PRIMOSDK_CDROM    if the unit is a CD-ROM
//                PRIMOSDK_CDR      if the unit is a supported CD-R
//                PRIMOSDK_CDRW     if the unit is a supported CD-RW
//                PRIMOSDK_DVDROM   if the unit is a DVD-ROM
//                PRIMOSDK_DVDR     if the unit is a supported DVD-R
//                PRIMOSDK_DVDRW    if the unit is a supported DVD-RW
//                                  Note: for drives that support DVD-RW and DVD+RW,
//                                        PRIMOSDK_DVDRW is returned.
//                PRIMOSDK_DVDPRW   if the unit is a supported DVD+RW
//                PRIMOSDK_DVDPR9   for DVD+R9
//                PRIMOSDK_DVDRAM   if the unit is a supported DVD-RAM
//                PRIMOSDK_ROBOTICS if the unit is a robotics
//                PRIMOSDK_OTHER    if the unit is of another type
//
//          szDescr returns the Vendor, Product and Firmware Version of
//                the unit (if not NULL)
//
//          pdwReady is set to 1 if the unit is ready (if not NULL)
//
//   Notes: The szDescr must be at least 50 bytes in length. The last 4 chars.
//          of szDescr contain always the firmware version of the drive.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the requested drive does not exist
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_UnitInfo2(DWORD dwHandle, PDWORD pdwUnit, PDWORD pdwTypes,
								PDWORD pdwClass, PDWORD pdwBusType, PDWORD pdwFeatures);
//          ====================
//
// Retrieve information about a unit.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification.
//                (see PrimoSDK_UnitInfo for field format)
//
//          pdwTypes returns a vector containing all the medium type managed by the unit,
//                closed by 0xFFFFFFFF. For instance, for a normal CD-RW, pdwTypes will
//                return:
//                PRIMOSDK_CDROM,PRIMOSDK_CDR,PRIMOSDK_CDRW,0xFFFFFFFF
//                For a combo CD-RW it will be:
//                PRIMOSDK_CDROM,PRIMOSDK_CDR,PRIMOSDK_CDRW,PRMOSDK_DVDROM,0xFFFFFFFF
//                and so on.
//
//          pdwClass is the class identifier assigned to this drive. Different brand drives
//                can have the same class. If the drive is taken by the "Silent Select"
//                mechanism, which means that it is not in the drive table but it is
//                still managed, this value would be the special value PRIMOSDK_SSCLASS
//
//          pdwBusType is the bus type that the device is connected to.
//
//          pdwFeatures is a DWORD bitfield that identifies features and capabilities
//                of the drive.
//
//   Notes: remember to allow enough DWORD's under pdwTypes as some combo units can
//          handle several type of media.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the requested drive does not exist
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_UnitSpeeds(DWORD dwHandle, PDWORD pdwUnit, PDWORD pdwCDSpeeds,
								 PDWORD pdwDVDSpeeds, PDWORD pdwCapabilities);
//          =====================
//
// This API has been deprecated.  Drive speeds are no longer supported, use only
// current media speeds obtained with PrimoSDK_GetDiscSpeedList.
//
// Retrieve the various speeds of a unit.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification.
//                (see PrimoSDK_UnitInfo for field format)
//
//          pdwCDSpeeds returns the unit speeds for CD. There are 3 set of values:
//                the reading speeds, the CD-R speeds, and the CD-RW, each set terminated by
//                0xFFFFFFFF. Some speeds can be empty (just the terminator) when not managed.
//                For instance in the case of a drive which reads at 24x and records only on
//                CD-R at 2x and 4x, this parameter will return (hex):
//                0x00000018,0xFFFFFFFF,0x00000002,0x00000004,0xFFFFFFFF,0xFFFFFFFF
//                The caller needs to be sure that this paramter is pointed to a vector of at
//                least 66 DWORD, as some drive can operate on many speeds.
//
//          pdwDVDSpeeds is as above, but for DVD-ROM, DVD-R and DVD-RW. If the drive
//                does not support DVD, this parameter will point to:
//                0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF
//
//          pdwCapabilities will point to a DWORD with some drive capabilities
//                1st bit (0x00000001) is set if the drive supports BURN-Proof
//                2nd bit (0x00000002) is set if the drive handles CD Text
//                3rd bit (0x00000004) is set if the drive support AWS (better speed for the
//                                     media and therefore accept PRIMOSDK_BEST as a speed)
//                4th bit (0x00000008) is set if the drive support reading DVD-ROM discs
//                5th bit (0x00000010) is set if the drive needs to open the tray after Test
//                6th bit (0x00000020) is set if the drive needs to open the tray after Record
//                7th bit (0x00000040) is set if the drive supports High-density recording.
//
//   Notes: the reading speed, for both CD and DVD is always represented by a single
//          value, the declared maximum.
//
//          To support silent select drives, caller should call PrimoSDK_UnitInfo2 to determine
//          the drive class.
//          If the drive class is managed by Silent Select (see PrimoSDK_UnitInfo2) then only
//          the faster speed is return and not recommended for display.
//          The caller should use PRIMOSDK_MAX, PRIMOSDK_MEDIUM or PRIMOSDK_MIN speeds then.
//          If the caller uses PRIMOSDK_BEST on a drive that does not support AWS, then
//          the speed will revert to PRIMOSDK_MAX
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the requested drive does not exist
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_UnitReady(DWORD dwHandle, PDWORD pdwUnit);
//          ====================
//
// Test if the unit is ready.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification
//                (see PrimoSDK_UnitInfo for field format)
//
//   Notes: This function performs the bare minimum elaboration and is the one
//          to call in timer loop while waiting for a drive to become ready.
//          This function must be called only on CD or DVD drives. Calling this
//          function on Hard Disk, boards or other units that you can still
//          identify by SCSI triple could have unpredictable results.
//
//  Return: PRIMOSDK_OK if no error, the unit is ready
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_NOTREADY if the unit is not ready
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_DiscInfoEx(DWORD dwHandle, PDWORD pdwUnit, DWORD dwFlags,
								 PDWORD pdwMediumType, PDWORD pdwMediumFormat,
								 PDWORD pdwErasable, PDWORD pdwTracks,
								 PDWORD pdwUsed, PDWORD pdwFree);
//          ===================
//
// Retrieve information about the medium inside pdwUnit.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification.
//                (see PrimoSDK_UnitInfo for field format)
//
//          dwFlags is either 0 or PRIMOSDK_TAO
//                PRMIOSDK_TAO sets the drive mode to incremental or TAO
//                   instead of DAO before retrieving disc information to
//                   return the correct available sectors in that mode.
//
//          pdwMediumType returns the type of the medium (if not NULL):
//                PRIMOSDK_SILVER if the disc is not recordable
//                PRIMOSDK_COMPLIANTGOLD if the disc is recordable by PrimoSDK
//                PRIMOSDK_OTHERGOLD if the disc is recordable but not usable
//                                 by PrimoSDK (other type of unclosed disc)
//                PRIMOSDK_BLANK if the disc is a blank recordable medium
//
//          pdwErasable returns 1 if the medium is erasable (an erasable disc
//                inside a CD-RW drive) and 0 otherwise
//
//          pdwMediumFormat returns the format of the medium (if not NULL):
//                PRIMOSDK_B1 - Blank Disc
//                PRIMOSDK_D1 - Data Mode 1 DAO (like the MSVC++ or a typical DOS game)
//                PRIMOSDK_D2 - Kodak Photo CD - Data multis. Mode 2 TAO
//                PRIMOSDK_D3 - Gold Data Mode 1 - Data multis. Mode 1, closed
//                PRIMOSDK_D4 - Gold Data Mode 2 - Data multis. Mode 2, closed
//                PRIMOSDK_D5 - Data Mode 2 DAO (silver mastered from Corel or Toast gold)
//                PRIMOSDK_D6 - CDRFS - Fixed packet (from Sony packet writing solution)
//                PRIMOSDK_D7 - Packet writing
//                PRIMOSDK_D8 - Gold Data Mode 1 - Data multis. Mode 1, open
//                PRIMOSDK_D9 - Gold Data Mode 2 - Data multis. Mode 2, open
//                PRIMOSDK_A1 - Audio DAO Silver, like almost any music disc, or Closed Golg
//                PRIMOSDK_A2 - Audio Gold disc not closed (TAO or SAO)
//                PRIMOSDK_A3 - First type of Enhanced CD (aborted)
//                PRIMOSDK_A4 - CD Extra, Blue Book standard
//                PRIMOSDK_A5 - Audio TAO tracks with session not closed, the (HP way)
//                PRIMOSDK_M1 - First track Data and other audio
//                PRIMOSDK_M2 - Gold TAO (like the ones made with Easy-CD 16 or 32 versions)
//                PRIMOSDK_M3 - Kodak Portfolio (as the Kodak standard)
//                PRIMOSDK_M4 - Video CD (as the White Book standard)
//                PRIMOSDK_M5 - CD-i (as the Green Book standard)
//                PRIMOSDK_M6 - PlayStation (Sony games)
//                PRIMOSDK_F1 - DVD-ROM
//                PRIMOSDK_F3 - Recordable DVD-R, closed
//                PRIMOSDK_F4 - Appendable (not-closed) disc
//                PRIMOSDK_F5 - Layer Jump DVD-R9 disc
//                PRIMOSDK_F8 - Recordable DVD-R, open
//                PRIMOSDK_FA - DVD-RAM cartridge
//                PRIMOSDK_GENERICCD - Other
//
//          pdwTracks number of tracks in the disc (if not NULL)
//                (always valid, 0 if PRIMOSDK_BLANK)
//
//          pdwUsed total number of used sectors in the disc (if not NULL)
//
//          pdwFree total number of free sectors in the disc (if not NULL)
//                (1 sector is 2048 bytes for the PRIMOSDK_COMPLIANTGOLD)
//
//   Notes: Before starting any recording operation, the caller should check
//          the presence of blank discs in all the engaged recorders using
//          this function.
//
//          For DVD+RW, DVD+RW media will always be reported as PRIMOSDK_F8 because of
//          the nature of the media.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_UNITERROR if the unit reported a reading error
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_DiscInfo(DWORD dwHandle, PDWORD pdwUnit,  PDWORD pdwMediumType,
							   PDWORD pdwMediumFormat, PDWORD pdwErasable, PDWORD pdwTracks,
							   PDWORD pdwUsed, PDWORD pdwFree);
//          ===================
//
// Retrieve information about the medium inside pdwUnit.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification.
//                (see PrimoSDK_UnitInfo for field format)
//
//          pdwMediumType returns the type of the medium (if not NULL):
//                PRIMOSDK_SILVER if the disc is not recordable
//                PRIMOSDK_COMPLIANTGOLD if the disc is recordable by PrimoSDK
//                PRIMOSDK_OTHERGOLD if the disc is recordable but not usable
//                                 by PrimoSDK (other type of unclosed disc)
//                PRIMOSDK_BLANK if the disc is a blank recordable medium
//
//          pdwErasable returns 1 if the medium is erasable (an erasable disc
//                inside a CD-RW drive) and 0 otherwise
//
//          pdwMediumFormat returns the format of the medium (if not NULL):
//                PRIMOSDK_B1 - Blank Disc
//                PRIMOSDK_D1 - Data Mode 1 DAO (like the MSVC++ or a typical DOS game)
//                PRIMOSDK_D2 - Kodak Photo CD - Data multis. Mode 2 TAO
//                PRIMOSDK_D3 - Gold Data Mode 1 - Data multis. Mode 1, closed
//                PRIMOSDK_D4 - Gold Data Mode 2 - Data multis. Mode 2, closed
//                PRIMOSDK_D5 - Data Mode 2 DAO (silver mastered from Corel or Toast gold)
//                PRIMOSDK_D6 - CDRFS - Fixed packet (from Sony packet writing solution)
//                PRIMOSDK_D7 - Packet writing
//                PRIMOSDK_D8 - Gold Data Mode 1 - Data multis. Mode 1, open
//                PRIMOSDK_D9 - Gold Data Mode 2 - Data multis. Mode 2, open
//                PRIMOSDK_A1 - Audio DAO Silver, like almost any music disc, or Closed Golg
//                PRIMOSDK_A2 - Audio Gold disc not closed (TAO or SAO)
//                PRIMOSDK_A3 - First type of Enhanced CD (aborted)
//                PRIMOSDK_A4 - CD Extra, Blue Book standard
//                PRIMOSDK_A5 - Audio TAO tracks with session not closed, the (HP way)
//                PRIMOSDK_M1 - First track Data and other audio
//                PRIMOSDK_M2 - Gold TAO (like the ones made with Easy-CD 16 or 32 versions)
//                PRIMOSDK_M3 - Kodak Portfolio (as the Kodak standard)
//                PRIMOSDK_M4 - Video CD (as the White Book standard)
//                PRIMOSDK_M5 - CD-i (as the Green Book standard)
//                PRIMOSDK_M6 - PlayStation (Sony games)
//                PRIMOSDK_F1 - DVD-ROM
//                PRIMOSDK_F3 - Recordable DVD-R, closed
//                PRIMOSDK_F4 - Appendable (not-closed) disc
//                PRIMOSDK_F5 - Layer Jump DVD-R9 disc
//                PRIMOSDK_F8 - Recordable DVD-R, open
//                PRIMOSDK_FA - DVD-RAM cartridge
//                PRIMOSDK_GENERICCD - Other
//
//          pdwTracks number of tracks in the disc (if not NULL)
//                (always valid, 0 if PRIMOSDK_BLANK)
//
//          pdwUsed total number of used sectors in the disc (if not NULL)
//
//          pdwFree total number of free sectors in the disc (if not NULL)
//                (1 sector is 2048 bytes for the PRIMOSDK_COMPLIANTGOLD)
//
//   Notes: Before starting any recording operation, the caller should check
//          the presence of blank discs in all the engaged recorders using
//          this function.
//
//          For DVD+RW, DVD+RW media will always be reported as PRIMOSDK_F8 because of
//          the nature of the media.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_UNITERROR if the unit reported a reading error
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_DiscInfo2(DWORD dwHandle, PDWORD pdwUnit,  PDWORD pdwMedium,
								PDWORD pdwProtectedDVD, PDWORD pdwFlags,
								PDWORD pdwMediumEx, PDWORD pdwRFU3);
//          ====================
//
// Retrieve additional information about the medium inside pdwUnit.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification.
//                (see PrimoSDK_UnitInfo for field format)
//
//          pdwMedium returns the physical type of the medium (if not NULL):
//                PRIMOSDK_CDROM    for CD-ROM, DDCD-ROM
//                PRIMOSDK_CDR      for CD-R, DDCD-R
//                PRIMOSDK_CDRW     for CD-RW, DDCD-RW
//                PRIMOSDK_DVDROM   for DVD-ROM (any type)
//                PRIMOSDK_DVDR     for DVD-R
//                PRIMOSDK_DVDRW    for DVD-RW
//                PRIMOSDK_DVDPR    for DVD+R
//                PRIMOSDK_DVDPRW   for DVD+RW
//                PRIMOSDK_DVDRAM   for DVD-RAM
//                PRIMOSDK_DVDPR9   for DVD+R9
//                PRIMOSDK_DVDR9    for DVD-R9
//                PRIMOSDK_BDRE     for BD-RE
//                PRIMOSDK_BDR      for BD-R
//                PRIMOSDK_OTHER    for other types
//
//          pdwProtectedDVD returns 1 if the medium is a protected DVD and 0 otherwise
//                (if not NULL)
//
//          pdwFlags returns any of the following values (if not NULL):
//                PRIMOSDK_PACKETWRITTEN if the media is formatted by packet writing
//                PRIMOSDK_HIGHDENSITY if the media was written in high-density mode
//                   software (currently only set for DVD-RW)
//
//          pdwMediumEx returns the physical type of the medium (if not NULL):
//                PRIMOSDK_CDROM    for CD-ROM
//                PRIMOSDK_CDR      for CD-R
//                PRIMOSDK_CDRW     for CD-RW
//                PRIMOSDK_DDCDROM  for DDCD-ROM
//                PRIMOSDK_DDCDR    for DDCD-R
//                PRIMOSDK_DDCDRW   for DDCD-RW
//                PRIMOSDK_DVDROM   for DVD-ROM (any type)
//                PRIMOSDK_DVDR     for DVD-R
//                PRIMOSDK_DVDRW    for DVD-RW
//                PRIMOSDK_DVDPR    for DVD+R
//                PRIMOSDK_DVDPRW   for DVD+RW
//                PRIMOSDK_DVDRAM   for DVD-RAM
//                PRIMOSDK_BDRE     for BD-RE
//                PRIMOSDK_BDR      for BD-R
//                PRIMOSDK_DVDPR9   for DVD+R9
//                PRIMOSDK_DVDR9    for DVD-R9
//                PRIMOSDK_OTHER    for other types
//
//          pdwRFU3 is reserved for future use
//
//   Notes: Before calling this function you must have already called PrimoSDK_DiscInfo.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_UNITERROR if the unit reported a reading error
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//

DWORD WINAPI PrimoSDK_GetDiscSpeedList(DWORD dwHandle, PDWORD pdwUnit,
									   DWORD dwNumSpeeds,
									   PDWORD pdwNumSpeeds,
									   PDWORD pdwSpeedList100thX);
//          =======================
//
// Ask and retrieve the the list of speeds of a disc, in 100th of X.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification.
//                (see PrimoSDK_UnitInfo for field format)
//
//          dwNumSpeeds is number of DWORDs that is allocated by the client of the SDK.
//
//          pdwNumSpeeds is the actual number of speeds returned.
//
//          pdwSpeedList100thX is the list containing the list of speeds in 100th of X
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_UNITERROR if the unit reported an error
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//


DWORD WINAPI PrimoSDK_GetDiscSpeed(DWORD dwHandle, PDWORD pdwUnit,
								   DWORD dwRequestedSpeed100thX, LPDWORD pdwGottenSpeed100thX);
//          =======================
//
// Ask and retrieve the exact speed of a disc, in 100th of X.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification.
//                (see PrimoSDK_UnitInfo for field format)
//
//          pwRequestedSpeed100thX is the requested speed in 100th of x.
//
//          pdwGottenSpeed100thX is where is returned the actual speed the drive agreed
//                on the disc for the speed request
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_UNITERROR if the unit reported an error
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_GetDVDType(DWORD dwHandle, PDWORD pdwUnit, LPDWORD pdwType,
								 LPDWORD pdwRFU);
//          =====================
//
// Retrieve type of DVD media inside pdwUnit
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification.
//                (see PrimoSDK_UnitInfo for field format)
//
//          pdwType returns the type of DVD media
//                PRIMOSDK_DVDUNKNOWN if unknown or not a DVD
//                PRIMOSDK_DVDDATA if is DVD data
//                PRIMOSDK_DVDAUDIO if disc has DVD-Audio
//                PRIMOSDK_DVDVIDEO if disc has DVD-Video
//                PRIMOSDK_DVDSTREAM if DVD Stream Recording
//                PRIMOSDK_DEFECTMAPPING if the disc is contains defect mapping sectors (i.e Mt. Rainer)
//
//          pdwRFU is reserved for future use
//
//   Notes: This function will not work correctly if the drive is locked via
//          PrimoSDK_UnitAIN()
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_UNITERROR if the unit reported a reading error
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_GIInfo(DWORD dwHandle, PBYTE szGIFileName, PDWORD pdwMediumFormat,
							 PDWORD pdwTracks, PDWORD pdwUsed);
//          =================
//
// Retrieve information about a Global Image file (.GI).(Use PrimoSDK_GIInfoEx instead.)
//
//   Param: dwHandle is the operation handle
//
//          szGIFileName is the Global Image file name
//
//          pdwMediumFormat returns the disc format(if not NULL)
//                (see PrimoSDK_DiscInfo)
//
//          pdwTracks number of tracks in the disc (if not NULL)
//
//          pdwUsed total number of used sectors in the disc (if not NULL)
//
//   Notes: Before starting any recording operation, the caller should check
//          the presence of blank discs in all engaged recorders.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_FILEERROR if szGIFileName is not found
//          PRIMOSDK_INVALIDSOURCE if the file is not a valid GI
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//


DWORD WINAPI PrimoSDK_GIInfoEx(DWORD dwHandle, PBYTE szGIFileName, PDWORD pdwMediumFormat,
							   PDWORD pdwTracks, PDWORD pdwUsed, PDWORD pdwMedium,
							   PDWORD pdwMediumEx);
DWORD WINAPI PrimoSDK_GIInfoExWcs(DWORD dwHandle, WCHAR *wcsGIFileName, PDWORD pdwMediumFormat,
								  PDWORD pdwTracks, PDWORD pdwUsed, PDWORD pdwMedium,
								  PDWORD pdwMediumEx);
//          =================
//
// Retrieve information about a Global Image file (.GI).
//
//   Param: dwHandle is the operation handle
//
//          szGIFileName is the Global Image file name
//
//          pdwMediumFormat returns the disc format(if not NULL)
//                (see PrimoSDK_DiscInfo)
//
//          pdwTracks number of tracks in the disc (if not NULL)
//
//          pdwUsed total number of used sectors in the disc (if not NULL)
//
//          pdwMedium media type GI was created from (if not NULL)
//                (see PrimoSDK_DiscInfo2)
//
//          pdwMediumEx returns the physical type of the medium (if not NULL):
//                (see PrimoSDK_DiscInfo2)
//
//   Notes: Before starting any recording operation, the caller should check
//          the presence of blank discs in all engaged recorders.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_FILEERROR if szGIFileName is not found
//          PRIMOSDK_INVALIDSOURCE if the file is not a valid GI
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//


DWORD WINAPI PrimoSDK_TrackInfo(DWORD dwHandle, DWORD dwTrackNumber,
								PDWORD pdwSessionNumber, PDWORD pdwTrackType,
								PDWORD pdwPreGap, PDWORD pdwStart, PDWORD pdwLength);
//          ====================
//
// Retrieve the information about a track.
//
// After a PrimoSDK_DiscInfo or a PrimoSDK_GIInfo has been performed, this
// function will retrieve the type and some geometry information of a track.
//
//   Param: dwHandle is the operation handle used to perform the PrimoSDK_DiscInfo
//                or the PrimoSDK_GIInfo
//
//          dwTrackNumber is the number of the track to get info from, 1 is the first
//
//          pdwSessionNumber returns the number of the session that contains the track
//
//          pdwTrackType returns the type of the session the track, 0 if is Audio,
//                1 if Data Mode1, 2 if data Mode2
//
//          pdwPreGap returns the pre-gap in sector of the track
//
//          pdwStart returns the start position in sector of the track
//
//          pdwLength returns the length in sector of the track
//
//   Notes: This function must be called only after having issued a PrimoSDK_DiscInfo
//          or a PrimoSDK_GIInfo.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or PrimoSDK_DiscInfo/PrimoSDK_GIInfo
//                has not been called yet
//          PRIMOSDK_BADPARAM if dwTrackNumber is incorrect
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_CDTextInfo(DWORD dwHandle, PDWORD pdwUnit,
								 PBYTE szTitle, PBYTE szPerformer, PBYTE szComposer);
//          =====================
//
// Retrieve the (English) CD Text information about a disc.
//
// After a PrimoSDK_DiscInfo or a PrimoSDK_GIInfo has been performed, this
// function will retrieve the CD Text information for the entire disc.
//
//   Param: dwHandle is the operation handle used to perform the PrimoSDK_DiscInfo
//                or the PrimoSDK_GIInfo
//
//          pdwUnit points to a DWORD containing the unit identification.
//                (see PrimoSDK_UnitInfo for field format)
//
//          szTitle points to a multiline string, where each line is divided by <CR><LF>;
//                the 1st line is the Title for the disc, the 2nd for the 1st track, the
//                3rd line is the title of the 2nd track, and so on
//
//          szPerformer is similar to szTitle, but it is for the Performer
//
//          szComposer is similar to szTitle, but it is for the Composer
//
//   Notes: This function must be called only after having issued a PrimoSDK_DiscInfo
//          or a PrimoSDK_GIInfo. Please point to enough room in szTitle, szPerformer
//          and szComposer. If this 3 fields return empty ("") then the disc does not
//          have CD Text.
//          A safe size for the buffers pointed by szTitle, szPerformer and szComposer
//          is 2000 bytes each.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no Audio CD started
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if thye drive does not support CD Text
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_CDTextInfoEJ(DWORD dwHandle, PDWORD pdwUnit,
								   PBYTE szTitleE, PBYTE szPerformerE, PBYTE szComposerE,
								   PBYTE szTitleJ, PBYTE szPerformerJ, PBYTE szComposerJ);
//          =======================
//
// Retrieve the (English and Japanese) CD Text information about a disc.
//
// After a PrimoSDK_DiscInfo or a PrimoSDK_GIInfo has been performed, this
// function will retrieve the CD Text information for the entire disc.
//
//   Param: dwHandle is the operation handle used to perform the PrimoSDK_DiscInfo
//                or the PrimoSDK_GIInfo
//
//          pdwUnit points to a DWORD containing the unit identification.
//                (see PrimoSDK_UnitInfo for field format)
//
//          szTitleE points to a multiline string, where each line is divided by <CR><LF>;
//                the 1st line is the Title for the disc, the 2nd for the 1st track, the
//                3rd line is the title of the 2nd track, and so on. This is the English
//                information
//
//          szPerformerE is similar to szTitleE, but it is for the Performer
//
//          szComposerE is similar to szTitleE, but it is for the Composer
//
//          szTitleJ is similar to szTitleE, but it is for the Composer in Japanese
//
//          szPerformerJ is similar to szTitleJ, but it is for the Performer
//
//          szComposerJ is similar to szTitleJ, but it is for the Composer
//
//   Notes: This function must be called only after having issued a PrimoSDK_DiscInfo
//          or a PrimoSDK_GIInfo. Please point to enough room in szTitle, szPerformer
//          and szComposer. If this 3 fields return empty ("") then the disc does not
//          have CD Text. If the Japanese CD Text information is not present, then the
//          relatine strings are returned empty.
//          The Japanese strings are returned in Shift JIS (Double byte).
//          A safe size for the buffers pointed by szTitle, szPerformer and szComposer
//          is 2000 bytes each.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no Audio CD started
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if thye drive does not support CD Text
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_MoveMedium(DWORD dwHandle, PDWORD pdwUnit, DWORD dwFlags);
//          =====================
//
// Open/Close the tray or eject the caddy.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification.
//                (see PrimoSDK_UnitInfo for field format)
//
//          dwFlags is PRIMOSDK_OPENTRAYEJECT to open/eject the tray/caddy and
//                PRIMOSDK_CLOSETRAY to close the tray (does nothing if caddy).
//                Use also PRIMOSDK_IMMEDIATE if you want to have back the control
//                without waiting that the drive completes the operation
//
//   Notes: If no PRIMOSDK_OPENTRAYEJECT nor PRIMOSDK_CLOSETRAY is selected, this
//          function will report an error
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_NOTREADY if the unit is not ready to move the tray
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_UnitAIN(DWORD dwHandle, PDWORD pdwUnit, DWORD dwFlags);
//          ==================
//
// Block/Unblock the Auto Insert Notification and the File System activity.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification.
//                (see PrimoSDK_UnitInfo for field format)
//
//          dwFlags is PRIMOSDK_LOCK to block or PRIMOSDK_UNLOCK to unblock the
//                file system activity
//                PRIMOSDK_FORCE_REFRESH can also be specified to force
//                the OS to refresh the media after the lock/unlock
//
//    Note: when the activity is blocked on a drive, every other user operations
//          return that the unit is not ready. The only operations allowed
//          by this filtering are the ones made by PrimoSDK.
//          It is mandatory that the Auto Insert Notification is stopped on the
//          recording drives, as well as any other kind of access not performed
//          by PrimoSDK.
//          The blocking is actuated by the PxHelper driver; this function
//          will return an error when running under WinASPI.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_NOAINCONTROL if the AIN control did not activated; this
//                usually happen when running under WinASPI instead of PxHelper
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_UnitVxBlock(DWORD dwHandle, PDWORD pdwUnit, DWORD dwFlags,
								  PBYTE szAppName);
//          ======================
//
// Inquiry or Reserve/Release a drive using the VxBlock.dll mechanism.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification.
//                (see PrimoSDK_UnitInfo for field format)
//
//          dwFlags can be PRIMOSDK_TEST to Inquiry the VxBlock status of the drive,
//                or PRIMOSDK_LOCK to Reserve it or PRIMOSDK_UNLOCK to Release it
//
//          szAppName will receive the string set by the application that reserved the
//                drive if dwFlags was PRIMOSDK_TEST and the result is PRIMOSDK_NOTREADY.
//                If dwFlags is PRIMOSDK_LOCK or PRIMOSDK_UNLOCK this string is used to
//                pass the string that the application sets while reserving the drive.
//                Use NULL to either not receive or not pass the string.
//
//    Note: this works only when the VxBlock.dll has been installed in the system.
//          Remember to pass again at PRIMOSDK_UNLOCK the very same string that
//          has been used at PRIMOSDK_LOCK. szAppName must be long enough when
//          Inquiring (256 bytes suggested)
//
//  Return: PRIMOSDK_OK if dwFlags was PRIMOSDK_TEST and the drive is not already
//                reserved, or if dwFlags was PRIMOSDK_LOCK and drive was reserved
//                successfully (szAppName should be loaded with the string
//                identifying who is reserving when dwFlags is PRIMOSDK_LOCK or
//                PRIMOSDK_UNLOCK)
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_INCOMPATIBLE if VxBlock.dll is not installed
//          PRIMOSDK_NOTREADY if dwFlags was PRIMOSDK_TEST or PRIMOSDK_LOCK and the drive
//                was already reserved (szAppName is loaded with the string of who reserved)
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_UnitLock(DWORD dwHandle, PDWORD pdwUnit, DWORD dwFlags);
//          ===================
//
// Lock/Unlock the tray or the caddy.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification.
//                (see PrimoSDK_UnitInfo for field format)
//
//          dwFlags is PRIMOSDK_LOCK to lock or PRIMOSDK_UNLOCK to unlock
//
//    Note: a good implementation should lock the units before starting to record
//          and it remembers to unlock when the operation finishes, wathever path
//          in the code is taken.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_EraseMedium(DWORD dwHandle, PDWORD pdwUnit, DWORD dwFlags);
//          ======================
//
// Erase a rewritable disc.
//
// This function returns immediately and continues asynchronously. The caller
// should use the PrimoSDK_RunningStatus to control the operations.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification.
//                (see PrimoSDK_UnitInfo for field format)
//
//          dwFlags is PRIMOSDK_ERASEQUICK for a fast erase (only the TOC) or
//                PRIMOSDK_ERASEFULL for a complete erase or
//                PRIMOSDK_ERASELAST to erase the last session, where available
//
//   Notes: None.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_CopyDisc(DWORD dwHandle, PDWORD pdwUnits, PDWORD pdwUnitSource,
							   DWORD dwFlags, DWORD dwSpeed);
//          ===================
//
// Copies an entire disc, from a source disc to one or more recorders.
//
// This function returns immediately and continues asynchronously. The caller
// should use the PrimoSDK_RunningStatus to control the operations.
// The pdwUnits units must contain a blank disc and the pdwUnitSource unit must
// contain a valid disc to copy.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnits points to a vector of DWORD containing the units identification,
//                (see PrimoSDK_UnitInfo for field format) terminated by 0xFFFFFFFF
//
//          pdwUnitSource points the source unit identification
//                (see PrimoSDK_UnitInfo for field format)
//
//          dwFlags is (OR the following values if more than one):
//                PRIMOSDK_WRITE for real recording or PRIMOSDK_TEST for test
//                PRIMOSDK_COPYPREGAP to copy the Pre-gaps of audio tracks or
//                PRIMOSDK_NOPREGAP to not copy the Pre-gaps
//                PRIMOSDK_BURNPROOF if enable the BURN-Proof support if available
//                PRIMOSDK_HIGHDENSITY enable writing in high-density mode (valid if the source is high-density)
//                PRIMOSDK_CLOSEDISC if the disc must be closed anyway
//                PRIMOSDK_VNR_WRITE to write to disc using Verify Not Required mode if supported by the drive
//
//          dwSpeed defines the speed to use for recording:
//                PRIMOSDK_MAX or n (like 8 for 8x) or PRIMOSDK_BEST if the drive supports AWS
//
//   Notes: The caller must check the number of free blocks on the destination disc
//          against the total blocks of the source, using PrimoSDK_DiscInfo, before
//          calling this function. PrimoSDK will try to overburn if you try to copy
//          more sectors than the available declared in the medium.
//          If you select a speed that is not supported by the selected recorder
//          the closest slower one is set.
//          This function will return PRIMOSDK_BADPARAM when trying to test on a
//          DVD+R, DVD+RW or DVD+RAM.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or if another operation
//                is already in progress using the same handle
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_NOTREADY if the recorder or the source is not ready
//          PRIMOSDK_INVALIDSOURCE if the source does not contain a valid
//                disc to copy
//          PRIMOSDK_INCOMPATIBLE if the type of disc is not compatible with
//                the capabilities of the recorder
//          PRIMOSDK_ITSADEMO if trying to record more than permitted with a Demo version
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_ReadGI(DWORD dwHandle, PDWORD pdwUnitSource,
							 PBYTE szGIFileName, DWORD dwFlags);
DWORD WINAPI PrimoSDK_MakeGIWcs(DWORD dwHandle, PDWORD pdwUnitSource,
								WCHAR *wcsGIFileName, DWORD dwFlags);
//          =================
//
// Copies an entire disc, from a source unit to a Global Image.
//
// This function returns immediately and continues asynchronously. The caller
// should use the PrimoSDK_RunningStatus to control the operations.
// The unit must contain a valid disc to copy.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnitSource points the source unit identification
//                (see PrimoSDK_UnitInfo for field format)
//
//          szGIFileName is the file name where to store the GI
//
//          dwFlags is:
//                PRIMOSDK_COPYPREGAP to read the Pre-gaps of audio tracks or
//                PRIMOSDK_NOPREGAP to not read the Pre-gaps
//
//   Notes: None.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or if another operation
//                is already in progress using the same handle
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_NOTREADY if the source unit is not ready
//          PRIMOSDK_INVALIDSOURCE if the source does not contain a valid
//                disc to read
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_WriteGI(DWORD dwHandle, PDWORD pdwUnits, PBYTE szGIFileName,
							  DWORD dwFlags, DWORD dwSpeed);
DWORD WINAPI PrimoSDK_WriteGIWcs(DWORD dwHandle, PDWORD pdwUnits, WCHAR *wcsGIFileName,
								 DWORD dwFlags, DWORD dwSpeed);
//          ==================
//
// Writes a Global Image to one or more recorders.
//
// This function returns immediately and continues asynchronously. The caller
// should use the PrimoSDK_RunningStatus to control the operations.
// The units must contain a blank disc.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnits points to a vector of DWORD containing the units identification,
//                (see PrimoSDK_UnitInfo for field format) terminated by 0xFFFFFFFF
//
//          szGIFileName is the file name where is stored the GI
//
//          dwFlags can be PRIMOSDK_WRITE for real recording or PRIMOSDK_TEST for test
//                PRIMOSDK_BURNPROOF if enable the BURN-Proof support if available
//                PRIMOSDK_HIGHDENSITY enable writing in high-density mode
//                PRIMOSDK_VNR_WRITE to write to disc using Verify Not Required mode if supported by the drive
//
//          dwSpeed defines the speed to use for recording:
//                PRIMOSDK_MAX or n (like 8 for 8x) or PRIMOSDK_BEST if the drive supports AWS
//
//   Notes: This function will return PRIMOSDK_BADPRAM when trying to test on a
//          DVD+R, DVD+RW or DVD+RAM.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or if another operation
//                is already in progress using the same handle
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_NOTREADY if the recorder is not ready
//          PRIMOSDK_FILEERROR if szGIFileName is not found
//          PRIMOSDK_INVALIDSOURCE if the file is not a valid GI
//          PRIMOSDK_INVALIDMEDIUM if the target disc is not blank
//          PRIMOSDK_INCOMPATIBLE if the type of disc store in the GI
//                is not compatible with the capabilities of the recorder
//          PRIMOSDK_ITSADEMO if trying to record more than permitted with a Demo version
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_MakeOtherCDImageWcs(DWORD dwHandle, PDWORD pdwUnitSource,
		WCHAR *wcsFileName, DWORD dwFlags);
//          =================
//
// Extracts the first data track to an ISO file.
//
// This function returns immediately and continues asynchronously. The caller
// should use the PrimoSDK_RunningStatus to control the operations.
// The unit must contain a valid disc to copy.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnitSource points the source unit identification
//                (see PrimoSDK_UnitInfo for field format)
//
//          wcsFileName is the file name where to store the GI
//
//          dwFlags is currently unused
//
//   Notes: None.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or if another operation
//                is already in progress using the same handle
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_NOTREADY if the source unit is not ready
//          PRIMOSDK_INVALIDSOURCE if the source does not contain a valid
//                disc to read
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_WriteOtherCDImage(DWORD dwHandle, PDWORD pdwUnits, PBYTE szFileName,
										DWORD dwFlags, DWORD dwSpeed);
DWORD WINAPI PrimoSDK_WriteOtherCDImageWcs(DWORD dwHandle, PDWORD pdwUnits, WCHAR *wcsFileName,
		DWORD dwFlags, DWORD dwSpeed);
//          ============================
//
// Writes a generic CD image to one or more recorders.
//
// This function returns immediately and continues asynchronously. The caller
// should use the PrimoSDK_RunningStatus to control the operations.
// The units must contain a blank disc.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnits points to a vector of DWORD containing the units identification,
//                (see PrimoSDK_UnitInfo for field format) terminated by 0xFFFFFFFF
//
//          szFileName is the file name where is stored the CD image.
//
//          dwFlags can be PRIMOSDK_WRITE for real recording or PRIMOSDK_TEST for test
//                PRIMOSDK_SAO : (default) if the image should be written SAO mode.
//                PRIMOSDK_TAO : if the image should be written TAO mode; you may use this flag
//                                  to append more ISO tracks to a disc, if the disc is not closed.
//                PRIMOSDK_BURNPROOF if enable the BURN-Proof support if available
//                PRIMOSDK_HIGHDENSITY enable writing in high-density mode
//                PRIMOSDK_VNR_WRITE to write to disc using Verify Not Required mode if supported by the drive
//                PRIMOSDK_CLOSEDISC if the disc must be closed so no other session could
//                                  be added
//                PRIMOSDK_IMAGE_M1_2048 to record an image in Mode 1 with a block
//                                  length of 2048 bytes, or
//                PRIMOSDK_IMAGE_M2_2336 to record an image in Mode 2 with a block
//                                  length of 2336 bytes, or
//                PRIMOSDK_IMAGE_M2_2352 to record an image in Mode 2 with a block
//                                  length of 2352 bytes
//
//          dwSpeed defines the speed to use for recording:
//                PRIMOSDK_MAX or n (like 8 for 8x) or PRIMOSDK_BEST if the drive supports AWS
//
//   Notes: Use this function to record an ISO image made with PrimoSDK_SaveGI, passing
//          the parameters PRIMOSDK_IMAGE_M1_2048.
//          This image must NOT be in a Global Image (.GI) file (use the PrimoSDK_WriteGI
//          function to record GIs) but in any other format.
//          Pay attention that the use of wrong parameters, not fitting the
//          image, can lead to unusable discs.
//          This function will return PRIMOSDK_BADPRAM when trying to test on a
//          DVD+R, DVD+RW or DVD+RAM.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or if another operation
//                is already in progress using the same handle
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_NOTREADY if the recorder is not ready
//          PRIMOSDK_FILEERROR if szFileName is not found
//          PRIMOSDK_INVALIDMEDIUM if the target disc is not blank
//          PRIMOSDK_ITSADEMO if trying to record more than permitted with a Demo version
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_VerifyDisc(DWORD dwHandle, PDWORD pdwUnits,
								 PDWORD pdwUnitSource, DWORD dwSpeed);
//          =====================
//
// Verifies an entire source disc against one or more written discs.
//
// The pdwUnits units must contain the disc that must be verified against the discs
// in pdwUnitSource.
//
// This function returns immediately and continues asynchronously. The caller
// should use the PrimoSDK_RunningStatus to control the operations.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnits points to a vector of DWORD containing the units identification,
//                (see PrimoSDK_UnitInfo for field format) terminated by 0xFFFFFFFF
//
//          pdwUnitSource points the source unit identification
//                (see PrimoSDK_UnitInfo for field format)
//
//          dwSpeed defines the speed to use for recording:
//                PRIMOSDK_MAX or n (like 8 for 8x)
//
//   Notes: The discs geometry is verified first. Then the verification continues byte
//          per byte on every track; for the audio tracks where, because of the
//          absence of error correction in the Red Book standard an absolute verification
//          is impossible, a special proprietary algorithm is applied.
//          The PrimoSDK_UnitStatus reports PRIMOSDK_UNITERROR if a drive fails the
//          verification; the other parametrs of PrimoSDK_UnitStatus, as pCommnad,
//          pSense, pASC and pASCQ are all set to 0.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or if another operation
//                is already in progress using the same handle
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_NOTREADY if the recorder or the source are not ready or
//                if the recorders contain blanks
//          PRIMOSDK_INVALIDSOURCE if the source does not contain a valid
//                disc to verify
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_VerifyGI(DWORD dwHandle, PDWORD pdwUnits,
							   PBYTE szGIFileName, DWORD dwSpeed);
DWORD WINAPI PrimoSDK_VerifyGIWcs(DWORD dwHandle, PDWORD pdwUnits,
								  WCHAR *wcsGIFileName, DWORD dwSpeed);
//          ===================
//
// Verifies a Global Image file (.GI) against one or more written discs.
// The szFileName contain the image of the disc that must be verified against the
// CD in pdwUnitSource.
//
// This function returns immediately and continues asynchronously. The caller
// should use the PrimoSDK_RunningStatus to control the operations.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnits points to a vector of DWORD containing the units identification,
//                (see PrimoSDK_UnitInfo for field format) terminated by 0xFFFFFFFF
//
//          szGIFileName is the file name where is stored the GI
//
//          dwSpeed defines the speed to use for verifing:
//                PRIMOSDK_MAX or n (like 8 for 8x)
//
//   Notes: The discs geometry is verified first. Then the verification continues
//          byte by byte on every sector of the data tracks; for the audio tracks
//          (where, because of the absence of error correction in the Red Book standard,
//          a total verification is impossible) a special proprietary algorithm is
//          applied.
//          The PrimoSDK_UnitStatus reports PRIMOSDK_UNITERROR if a drive fails the
//          verification; the other parametrs of PrimoSDK_UnitStatus, as pCommnad,
//          pSense, pASC and pASCQ are all set to 0.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or if another operation
//                is already in progress using the same handle
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_NOTREADY if the recorder or the source are not ready or
//                if the recorders contain blanks
//          PRIMOSDK_FILEERROR if szGIFileName is not found
//          PRIMOSDK_INVALIDSOURCE if the file is not a valid GI
//          PRIMOSDK_INCOMPATIBLE if the type of disc is not compatible with
//                the capabilities of the recorder
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_VerifyOtherCDImage(DWORD dwHandle, PDWORD pdwUnits,
		PBYTE szFileName, DWORD dwFlags, DWORD dwSpeed);
DWORD WINAPI PrimoSDK_VerifyOtherCDImageWcs(DWORD dwHandle, PDWORD pdwUnits,
		WCHAR *wcsFileName, DWORD dwFlags, DWORD dwSpeed);
//          =============================
//
// Verifies a generic CD image against one or more recorders.
//
// This function returns immediately and continues asynchronously. The caller
// should use the PrimoSDK_RunningStatus to control the operations.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnits points to a vector of DWORD containing the units identification,
//                (see PrimoSDK_UnitInfo for field format) terminated by 0xFFFFFFFF
//
//          szFileName is the file name where is stored the CD image.
//
//          dwFlags can be
//                PRIMOSDK_IMAGE_M1_2048 to verify an image in Mode 1 with a block
//                                  length of 2048 bytes, or
//                PRIMOSDK_IMAGE_M2_2336 to verify an image in Mode 2 with a block
//                                  length of 2336 bytes, or
//                PRIMOSDK_IMAGE_M2_2352 to verify an image in Mode 2 with a block
//                                  length of 2352 bytes
//
//          dwSpeed defines the speed to use for verifing:
//                PRIMOSDK_MAX or n (like 8 for 8x)
//
//   Notes: The PrimoSDK_UnitStatus reports PRIMOSDK_UNITERROR if a drive fails the
//          verification; the other parametrs of PrimoSDK_UnitStatus, as pCommnad,
//          pSense, pASC and pASCQ are all set to 0.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or if another operation
//                is already in progress using the same handle
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_NOTREADY if the recorder or the source are not ready or
//                if the recorders contain blanks
//          PRIMOSDK_FILEERROR if szFileName is not found
//          PRIMOSDK_INCOMPATIBLE if the type of disc is not compatible with
//                the capabilities of the recorder
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_CloseImage(DWORD dwHandle);
//          =====================
//
// Close and destroy the "CD Image".
//
// This function must always be called after a PrimoSDK_NewImage, to terminate
// and free the allocated structures.
//
//   Param: dwHandle is the operation handle
//
//   Notes: None.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no CD Image started
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_AddFolder(DWORD dwHandle, PBYTE szFolder);
//          ====================
//
// Add a folder to the "CD Image".
//
// The folder must always be fully specified. Therefore, when adding a subfolder also
// the parent is specified. This function adds only one level at a time.
// For example, to create the path "\My Folder\My Sub Folder" and the path
// "\My Folder\My Second Sub Folder", that is:
//
// \My Folder
//      \My Sub Folder
//      \My Second Sub Folder
//
// The calls to this function should be:
//
// PrimoSDK_AddFolder(dwHandle,"\My Folder");
// PrimoSDK_AddFolder(dwHandle,"\My Folder\My Sub Folder");
// PrimoSDK_AddFolder(dwHandle,"\My Folder\My Second Sub Folder");
//
//   Param: dwHandle is the operation handle
//
//          szFolder specifies the folder name in MBCS
//
//   Notes: If PRIMOSDK_ISOLEVEL1 is specified, the folder must not have
//          any extension; it can be just up to 8 char.
//          The first backslash is mandatory. A trailer backslash
//          is optional. There is no need to create the root folder.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or, CD Image started or
//                if (only if PrimoSDK_NewImage was called with  PRIMOSDK_CHECKDUPLI)
//                the nested folders have not been added yet
//          PRIMOSDK_BADPARAM if incorrect or too long folder name
//          PRIMOSDK_ALREADYEXIST if the folder has been already added
//                (only if PrimoSDK_NewImage was called with  PRIMOSDK_CHECKDUPLI)
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_NOSPACE if the internal tables went in overflow (too many
//                files for the system memory)
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_AddFile(DWORD dwHandle, PBYTE szFileOnCD, PBYTE szSourceFile);
//          ==================
//
// Add szSourceFile to the "CD Image" with the name szFileOnCD.
//
// Both files must be fully specified, and the folder must already exist.
// For example:
//
// PrimoSDK_AddFile(dwHandle,"\My File.Txt","D:\Source\My File.Txt")
//   adds the file "My File.Txt" that is stored in "D:\Source"
//    to the CD root, with the same name.
//
// PrimoSDK_AddFile(dwHandle,"\My Folder\Your File.Txt","D:\Source\My File.Txt")
//   adds the same file to the "\My Folder" on the CD, with
//    the new name "Your File.Txt". "\My Folder" must have been already
//     added to the CD Image with a previous call to PrimoSDK_AddFolder.
//
//   Param: dwHandle is the operation handle
//
//          szFileOnCD specifies the file name on CD in MBCS
//
//          szSourceFile specifies the source file name
//
//   Notes: None.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized, no CD Image started or
//                if (only if PrimoSDK_NewImage was called with  PRIMOSDK_CHECKDUPLI)
//                the nested folders have not been added yet
//          PRIMOSDK_BADPARAM if incorrect or too long file name
//          PRIMOSDK_ALREADYEXIST if szFileOnCD name has been already added
//                (only if PrimoSDK_NewImage was called with PRIMOSDK_CHECKDUPLI)
//          PRIMOSDK_NOTREADABLE if the source file is not found or not readable
//          PRIMOSDK_FILEERROR if a file that was added is invalid
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_NOSPACE if the internal tables went in overflow (too many
//                files for the system memory)
//          PRIMOSDK_INTERR if an internal error occured
//          PRIMOSDK_FILETOOLARGE if a file that was added is bigger than 9.99 GB for UDF
//                                or bigger than 4 GB for ISO.
//



DWORD WINAPI PrimoSDK_AddBootable(DWORD dwHandle, PBYTE szBootImageFile, DWORD dwFlags);
//          ======================
//
// Add the bootable "El Torito" standard feature.
//
// This funcxtion must be called after having added all the folders and files, providing
// the floppy boot image.
//
//   Param: dwHandle is the operation handle
//
//          szBootImageFile specifies the floppy disc boot image
//
//          dwFlags can be the value PRIMOSDK_FLOPPY144 for 1.44 floppy images
//                or the value PRIMOSDK_FLOPPY288 for 2.88 floppy images
//                or the value PRIMOSDK_FLOPPY12 for 1.2 floppy images
//                or the value PRIMOSDK_HD for Hard Drive images
//                or the value PRIMOSDK_NOEMULATION for a special boot image
//                which uses a private loader
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no CD Image started
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_NOTREADABLE if the boot image file is not found or not readable
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//

DWORD WINAPI PrimoSDK_AddBootableEx(DWORD dwHandle, PBYTE szBootImageFile, DWORD dwFlags,
									DWORD dwSize,  DWORD reserved);
DWORD WINAPI PrimoSDK_AddBootableExWcs(DWORD dwHandle, WCHAR *wcsBootImageFile, DWORD dwFlags,
									   DWORD dwSize,  DWORD reserved);
//          ========================
//
// Add the bootable "El Torito" standard feature. Additional parameters for the Boot Info Table
//
// This function must be called after having added all the folders and files, providing
// the floppy boot image.
//
//   Param: dwHandle is the operation handle
//
//          szBootImageFile specifies the boot image
//
//          dwFlags can be the value PRIMOSDK_FLOPPY144 for 1.44 floppy images
//                or the value PRIMOSDK_FLOPPY288 for 2.88 floppy images
//                or the value PRIMOSDK_FLOPPY12 for 1.2 floppy images
//                or the value PRIMOSDK_HD for Hard Drive images
//                or the value PRIMOSDK_NOEMULATION for a special boot image
//                or the value PRIMOSDK_NOEMULATION_WITH_SET_BOOT_INFO for a special boot image
//                which uses a private loader. Also writes the Boot Info table into boot image.
//
//         dwSize is the size of the boot image file (for use with
//                PRIMOSDK_NOEMULATION_WITH_SET_BOOT_INFO).
//                If dwSize is zero, then boot size will be calculated from the length of the
//                boot image file specified by szBootImageFile.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no CD Image started
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_NOTREADABLE if the boot image file is not found or not readable
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//

DWORD WINAPI PrimoSDK_WriteImage(DWORD dwHandle, DWORD dwFlags,
								 DWORD dwSpeed, PDWORD pdwSize);
//          =====================
//
// Start the write (or test) of the "CD Image".
//
// This function returns immediately and continues asynchronously. The caller
// should use the PrimoSDK_RunningStatus to control the operations.
//
//   Param: dwHandle is the operation handle
//
//          dwFlags can be PRIMOSDK_WRITE for real recording or PRIMOSDK_TEST for test
//                PRIMOSDK_BURNPROOF if enable the BURN-Proof support if available
//                PRIMOSDK_HIGHDENSITY enable writing in high-density mode
//                PRIMOSDK_DVDPRQUICK to not force 30mm Lead Out when recording DVD+R disc
//                PRIMOSDK_ALLOW_NONSTANDARD_LAYER to allow non-compliant layer break on DVD Video
//                PRIMOSDK_FAST_WRITE to make drive write file data without read-after-write
//                PRIMOSDK_VNR_WRITE to write to disc using Verify Not Required mode if supported by the drive
//                PRIMOSDK_DVDRWCOMPAT to close DVD-RW discs in most compatible mode
//
//          dwSpeed defines the speed to use for recording:
//                PRIMOSDK_MAX or n (like 8 for 8x) or PRIMOSDK_BEST if the drive supports AWS
//
//          pdwSize returns the total sectors required for this CD Image
//
//   Notes: This function can be called more than once, to generate many CD copies,
//          after the files have been added, and before calling the PrimoSDK_CloseImage.
//          This function will return PRIMOSDK_BADPRAM when trying to test on a
//          DVD+R, DVD+RW or DVD+RAM.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no CD Image started
//                or the CD Image is empty
//          PRIMOSDK_BADPARAM if incorrect parameters to this function or if
//                no units have been passed to PrimoSDK_NewImage
//          PRIMOSDK_NOTREADY if the unit is not ready
//          PRIMOSDK_TMPOVERFLOW if a temporary file goes in overflow
//          PRIMOSDK_NOSPACE if the medium does not have enough free sectors
//          PRIMOSDK_DVDSTRUCTERROR if the passed VIDEO_TS or AUDIO_TS structure
//                do not respectthe DVD-Video or DVD-Audio rules (DVD version only)
//          PRIMOSDK_ITSADEMO if trying to record more than permitted with a Demo version
//          PRIMOSDK_FILEERROR if a file that was added is no longer found
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//
//
//



DWORD WINAPI PrimoSDK_VerifyImage(DWORD dwHandle, DWORD dwSpeed);
//          =====================
//
// Start the verify of the "CD Image".
//
// This function returns immediately and continues asynchronously. The caller
// should use the PrimoSDK_RunningStatus to control the operations.
//
//   Param: dwHandle is the operation handle
//
//          dwSpeed defines the speed to use for verifying:
//                PRIMOSDK_MAX or n (like 8 for 8x)
//
//   Notes: This function can be called after a PrimoSDK_WriteImage
//          and before calling the PrimoSDK_CloseImage.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no CD Image started
//                or the CD Image is empty
//          PRIMOSDK_BADPARAM if incorrect parameters to this function or if
//                no units have been passed to PrimoSDK_NewImage
//          PRIMOSDK_NOTREADY if the unit is not ready
//          PRIMOSDK_TMPOVERFLOW if a temporary file goes in overflow
//          PRIMOSDK_DVDSTRUCTERROR if the passed VIDEO_TS or AUDIO_TS structure
//                do not respectthe DVD-Video or DVD-Audio rules (DVD version only)
//          PRIMOSDK_FILEERROR if a file that was added is no longer found
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//
//
//



DWORD WINAPI PrimoSDK_SaveGI(DWORD dwHandle, PBYTE szGIFileName, PDWORD pdwSize);
DWORD WINAPI PrimoSDK_SaveGIWcs(DWORD dwHandle, WCHAR *wcsGIFileName, PDWORD pdwSize);
//          =================
//
// Writes the CD Image to a Global Image file.
//
// This function builds a Global Image corresponding to the files added.
// The image can later be recorded using PrimoSDK_WriteGI to a blank disc.
// This function returns immediately and continues asynchronously. The caller
// should use the PrimoSDK_RunningStatus to control the operations.
//
//   Param: dwHandle is the operation handle
//
//          szGIFileName is the file name where to write the Global Image
//
//          pdwSize returns the total sectors required for this CD Image
//                (the file will be slightly bigger than that, around 100K more)
//
//   Notes: This function does not engage any recorder.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no CD Image started
//                or the CD Image is empty
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_TMPOVERFLOW if a temporary file goes in overflow
//          PRIMOSDK_DVDSTRUCTERROR if the passed VIDEO_TS or AUDIO_TS structure
//                do not respectthe DVD-Video or DVD-Audio rules (DVD version only)
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_SaveImage(DWORD dwHandle, PBYTE szFileName, PDWORD pdwSize);
DWORD WINAPI PrimoSDK_SaveImageWcs(DWORD dwHandle, WCHAR *wcsFileName, PDWORD pdwSize);
//          ====================
//
// Writes the CD Image to an ISO Image file.
//
// This function builds an ISO or UDF image (UDF only if PrimoSDK for DVD).
// The image can later be recorded using PrimoSDK_WriteOtherCDImage to a
// blank disc.
// This function returns immediately and continues asynchronously. The caller
// should use the PrimoSDK_RunningStatus to control the operations.
//
//   Param: dwHandle is the operation handle
//
//          szFileName is the file name where to write the image
//
//          pdwSize returns the total sectors required for this CD Image
//
//   Notes: This function does not engage any recorder.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no CD Image started
//                or the CD Image is empty
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_TMPOVERFLOW if a temporary file goes in overflow
//          PRIMOSDK_DVDSTRUCTERROR if the passed VIDEO_TS or AUDIO_TS structure
//                do not respectthe DVD-Video or DVD-Audio rules (DVD version only)
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_NewAudio(DWORD dwHandle, PDWORD pdwUnits);
//          ===================
//
// Start a new Audio CD.
//
// An Audio CD is made by a list of tracks that represent the compilation.
// The programmer must call this function, add an audio file for each track using
// PrimoSDK_AddAudioTrack, then write (or test) the audio disc with PrimoSDK_WriteAudio.
// A call to PrimoSDK_CloseAudio will destroy the structure.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnits points to a vector of DWORD containing the units identification,
//                (see PrimoSDK_UnitInfo for field format) terminated by 0xFFFFFFFF
//
//   Notes: Commercial implementations of MPEG-1 and MPEG-2 decoders are subject
//          to royalty fees to patent holders. Many of these patents are general
//          enough such that they are unavoidable regardless of the implementation
//          design, and some may also apply to private implementations.
//          Therefore, PrimoSDK supports only Wave files in its current status.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_NOTREADY if the unit is not ready
//          PRIMOSDK_INVALIDMEDIUM if the target disc are not blank CD-R or CD-RW
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//


DWORD WINAPI PrimoSDK_CloseAudio(DWORD dwHandle);
//          =====================
//
// Close and destroy the Audio CD.
//
// This function must always be called after a PrimoSDK_NewAudio, to terminate
// and free the allocated structures.
//
//   Param: dwHandle is the operation handle
//
//   Notes: None.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no Audio CD started
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_AddAudioTrack(DWORD dwHandle, PBYTE szTrack,
									DWORD dwPreGap, PDWORD pdwSize);
DWORD WINAPI PrimoSDK_AddAudioTrackWcs(DWORD dwHandle, WCHAR *wcsTrack,
									   DWORD dwPreGap, PDWORD pdwSize);
//          ========================
//
// Add a track to the Audio CD.
//
//   Param: dwHandle is the operation handle
//
//          szTrack specifies an audio file
//
//          dwPregap the gap, in sectors, of the track. The first
//                track added (first song) will have a Pre-gap
//                of 150 block, no matter what value is passed
//
//          pdwSize points where is returned the size in sector
//                of the audio file
//
//   Notes: None.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no Audio CD started
//          PRIMOSDK_BADPARAM if incorrect file name or if the audio file
//                is not valid
//          PRIMOSDK_FILEERROR if the file is not found
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_AddCDText(DWORD dwHandle, DWORD dwFlags,
								PBYTE szTitle, PBYTE szPerformer, PBYTE szComposer);
//          ====================
//
// Add the (English) CD Text for the disc and all the added tracks.
//
//   Param: dwHandle is the operation handle
//
//          dwFlags is for future use and must be 0 now
//
//          szTitle points to a multiline string, where each line is divided by <CR><LF>;
//                the 1st line is the Title for the disc, the 2nd for the 1st track, the
//                3rd line is the title of the 2nd track, and so on
//
//          szPerformer is similar to szTitle, but it is for the Performer
//
//          szComposer is similar to szTitle, but it is for the Composer
//
//   Notes: This function must be called after all the tracks has been added, but before
//          calling PrimoSDK_WriteAudio. This function adds in just one call the CD Text
//          info of Title, Performer and Composer for the discs and for all the tracks.
//          The maximum size of the sum of the buffers pointed by szTitle, szPerformer and
//          szComposer is 6000 bytes and each one should be 2000 byte in maximum size.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no Audio CD started
//          PRIMOSDK_BADPARAM if the passed parametrs are not valid
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_AddCDTextEJ(DWORD dwHandle, DWORD dwFlags,
								  PBYTE szTitleE, PBYTE szPerformerE, PBYTE szComposerE,
								  PBYTE szTitleJ, PBYTE szPerformerJ, PBYTE szComposerJ);
//          ======================
//
// Add the (English and Japanese) CD Text for the disc and all the added tracks.
//
//   Param: dwHandle is the operation handle
//
//          dwFlags is for future use and must be 0 now
//
//          szTitleE points to a multiline string, where each line is divided by <CR><LF>;
//                the 1st line is the Title for the disc, the 2nd for the 1st track, the
//                3rd line is the title of the 2nd track, and so on, in English
//
//          szPerformerE is similar to szTitleE, but it is for the Performer
//
//          szComposerE is similar to szTitleE, but it is for the Composer
//
//          szTitleJ is similar to szTitleE, but in Japanese
//
//          szPerformerJ is similar to szTitleJ, but it is for the Performer
//
//          szComposerJ is similar to szTitleJ, but it is for the Composer
//
//   Notes: This function must be called after all the tracks has been added, but before
//          calling PrimoSDK_WriteAudio. This function adds in just one call the CD Text
//          info of Title, Performer and Composer for the discs and for all the tracks.
//          The Japanese strings must be passed in Shift JIS (Double byte).
//          The maximum size of the sum of the buffers pointed by szTitle, szPerformer and
//          szComposer is 6000 bytes and each one should be 2000 byte in maximum size.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no Audio CD started
//          PRIMOSDK_BADPARAM if the passed parametrs are not valid
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_GetAudioTrackInfo(DWORD dwHandle, DWORD dwTrack, DWORD dwInfoType,
										DWORD dwResultDwords, PDWORD pdwResult);
//          ======================
//
// Get information about an audio track.  Since this can be a long operation, this does
// not always return information on the first call (can return PRIMOSDK_RUNNING), so it
// is intended to be called in the following method:
//
//   while ((rc = PrimoSDK_GetAudioTrackInfo(...)) == PRIMOSDK_RUNNING)
//   {
//     // sleep or peek message
//   }
//   if (rc == PRIMOSDK_OK)
//   {
//     // info is valid
//   }
//
//   Param: dwHandle is the operation handle
//
//          dwTrack is the number of an Audio track previously added with PrimoSDK_AddAudioTrack(Ex)
//                  or PrimoSDK_AddAudioStream (1 = the first track added)
//
//          dwInfoType is one of the following:
//            PRIMOSDK_PEAK - the peak level for the track
//                            dwResultDwords must be at least 1
//                            pdwResult will be filled with a single dword that is the peak level
//                                      expressed as a number from 0 - 10000.  10000 indicates that
//                                      the maximum level is 100.00% of the possible level for the track.
//
//          dwResultDwords is the size (in dwords) that the caller supplies for the pdwResult array
//
//          pdwResult is an array of dwords that is the result of the info type
//
//   Notes: This function must be called after the specified track has been added, but before
//          calling PrimoSDK_WriteAudio.  The results can be used to modify audio using
//          the PrimoSDK_AddAudioEffect API.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_RUNNING if it is still calculating the requested info
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no Audio CD started
//          PRIMOSDK_BADPARAM if the passed parametrs are not valid
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_FILEERROR if an error in calculation occured
//
#define PRIMOSDK_PEAK 1

DWORD WINAPI PrimoSDK_WriteAudio(DWORD dwHandle, DWORD dwFlags, DWORD dwSpeed);
//          =====================
//
// Writes the Audio CD to one or more recorders.
//
// When all the tracks have been added to the audio disc, this functions
// writes (or test) the Audio CD.
// This function returns immediately and continues asynchronously. The caller
// should use the PrimoSDK_RunningStatus to control the operations.
//
//   Param: dwHandle is the operation handle
//
//          dwFlags is (OR them if more than one):
//                PRIMOSDK_WRITE for the real recording or PRIMOSDK_TEST for test
//                PRIMOSDK_BURNPROOF if enable the BURN-Proof support if available
//                PRIMOSDK_CLOSEDISC if the disc must be closed
//
//          dwSpeed defines the speed to use for recording:
//                PRIMOSDK_MAX or n (like 8 for 8x) or PRIMOSDK_BEST if the drive supports AWS
//
//   Notes: Use PRIMOSDK_CLOSEDISC for normal audio discs.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no Audio CD started
//                or the Audio CD is empty
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_NOTREADY if one or more units are not ready
//          PRIMOSDK_ITSADEMO if trying to record more than permitted with a Demo version
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_WriteAudioTrack(DWORD dwHandle, PDWORD pdwUnits, PBYTE szTrack,
									  DWORD dwFlags, DWORD dwSpeed);
DWORD WINAPI PrimoSDK_WriteAudioTrackWcs(DWORD dwHandle, PDWORD pdwUnits, WCHAR *wcsTrack,
		DWORD dwFlags, DWORD dwSpeed);
//          ==========================
//
// Writes a single track to an Audio CD on one or more recorders, in TAO.
// This function should be used to directly write, track by track, Audio CD, without
// using PrimoSDK_NewAudio, then a series of calls to PrimoSDK_AddAudioTrack, then
// PrimoSDK_WriteAudio and finally PrimoSDK_CloseAudio. With this function you need
// only to call this API, track by track, and every time wait for the writing to finish
// with PrimoSDK_RunningStatus. The disc could be blank (the first time) or have
// already some audio track. The call to write the last track must have the
// PRIMOSDK_CLOSEDISC flag set, to finalize the disc.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnits points to a vector of DWORD containing the units identification,
//                (see PrimoSDK_UnitInfo for field format) terminated by 0xFFFFFFFF
//
//          szTrack specifies an audio file
//
//          dwFlags is (OR them if more than one):
//                PRIMOSDK_WRITE for the real recording or PRIMOSDK_TEST for test
//                PRIMOSDK_BURNPROOF if enable the BURN-Proof support if available
//                PRIMOSDK_CLOSEDISC for the last track to finalize the disc
//
//          dwSpeed defines the speed to use for recording:
//                PRIMOSDK_MAX or n (like 8 for 8x) or PRIMOSDK_BEST if the drive supports AWS
//
//   Notes: The disc won't be playable on CD-ROM or consumer Audio CD player until
//          it is closed.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no Audio CD started
//                or the Audio CD is empty
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_NOTREADY if one or more units are not ready
//          PRIMOSDK_ITSADEMO if trying to record more than permitted with a Demo version
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//


DWORD WINAPI PrimoSDK_NewVideoCD(DWORD dwHandle, PDWORD pdwUnits,
								 PBYTE szTemp, PBYTE szVideoCDTemplate);
//          =====================
//
// Start a new Video CD.
//
// A Video CD is made by a list of MPEG1 stream files in the proper format.
// The programmer must call this function, add the stream files using
// PrimoSDK_AddVideoCDStream, then write (or test) the audio disc with PrimoSDK_WriteVideoCD.
// A call to PrimoSDK_CloseVideoCD will destroy the structure.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnits points to a vector of DWORD containing the units identification,
//                (see PrimoSDK_UnitInfo for field format) terminated by 0xFFFFFFFF
//
//          szTemp is a user directory where to generate the conversion of the track files
//                just before the recording starts. The space needed in this temporary folder
//                is the sum of the sectors returned into pdwSize by all the calls to
//                PrimoSDK_AddVideoCDStream multiplied by 2336
//
//          szVideoCDTemplate is the accessory file Vcd.dta which is provided with PrimoSDK
//                and that must be just passed from wherever it has been stored.
//
//   Notes:  The file passed as szVideoCDTemplate is used only creating Video CD and,
//           because of its size, PrimoSDK does not include it as a resource in the DLL.
//           Therefore, all the PrimoSDK implementations that do not create Video CD do not
//           need to carry along this extra space. If you create Video CD with PrimoSDK just
//           inlude this file with your installation and pass its pathname to this function.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_NOTREADY if the unit is not ready
//          PRIMOSDK_INVALIDMEDIUM if the target disc are not blank CD-R or CD-RW
//          PRIMOSDK_FILEERROR if szVideoCDTemplate is not found
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_CloseVideoCD(DWORD dwHandle);
//          =======================
//
// Close and destroy the Video CD.
//
// This function must always be called after a PrimoSDK_NewVideoCD, to terminate
// and free the allocated structures.
//
//   Param: dwHandle is the operation handle
//
//   Notes: None.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no Video CD started
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_AddVideoCDStream(DWORD dwHandle, PBYTE szStream, PDWORD pdwSize);
DWORD WINAPI PrimoSDK_AddVideoCDStreamWcs(DWORD dwHandle, WCHAR *wcsStream, PDWORD pdwSize);
//          ===========================
//
// Add a stream to the Video CD.
//
//   Param: dwHandle is the operation handle
//
//          szStream specifies an MPEG1 stream file. The proper stream format is
//                - NTSC 352x240 29.97 Hz. or
//                - PAL 352x288 25 Hz. or
//                - Film 352x240 29.97
//                Also, the audio inside the MPEG must be 44.1 KHz, Stereo
//
//          pdwSize points where is returned the size in sector of the track
//                that will represent that stream
//
//   Notes: The maximum number of streams allowed by PrimoSDK in a Video CD is 32.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no Audio CD started
//          PRIMOSDK_BADPARAM if the stream file is not valid
//          PRIMOSDK_FILEERROR if the file is not found
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_WriteVideoCD(DWORD dwHandle, DWORD dwFlags, DWORD dwSpeed);
//          =======================
//
// Writes the Video CD to one or more recorders.
//
// When all the streams (tracks) have been added to the disc, this functions
// writes (or test) the Video CD.
// This function returns immediately and continues asynchronously. The caller
// should use the PrimoSDK_RunningStatus to control the operations.
//
//   Param: dwHandle is the operation handle
//
//          dwFlags is (OR them if more than one):
//                PRIMOSDK_WRITE for the real recording or PRIMOSDK_TEST for test
//                PRIMOSDK_BURNPROOF if enable the BURN-Proof support if available
//                PRIMOSDK_HIGHDENSITY enable writing in high-density mode
//
//          dwSpeed defines the speed to use for recording:
//                PRIMOSDK_MAX or n (like 8 for 8x) or PRIMOSDK_BEST if the drive supports AWS
//
//   Notes: A Video CD disc will always be closed.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no Audio CD started
//                or the Audio CD is empty
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_NOTREADY if one or more units are not ready
//          PRIMOSDK_ITSADEMO if trying to record more than permitted with a Demo version
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_ExtractAudioTrack(DWORD dwHandle, PDWORD pdwUnit, DWORD dwNum,
										PBYTE szWaveFile, PDWORD pdwSize);
DWORD WINAPI PrimoSDK_ExtractAudioTrackWcs(DWORD dwHandle, PDWORD pdwUnit, DWORD dwNum,
		WCHAR *wWaveFile, PDWORD pdwSize);
//          ============================
//
// Extract a track from an Audio CD, as a Wave file.
//
// This function returns immediately and continues asynchronously. The caller
// should use the PrimoSDK_RunningStatus to control the operations.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification.
//                (see PrimoSDK_UnitInfo for field format)
//
//          dwNum is the track to extract (1 is the first)
//
//          szWaveFile is the name where the wave file is stored
//
//          pdwSize returns the total sectors of the track
//
//   Notes: If the operation is aborted by the calling program, the portion
//          of the file written until that moment is not deleted.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_NOTREADY if the unit is not ready
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_RunningStatus(DWORD dwHandle, DWORD dwFlags,
									PDWORD pdwCurSector, PDWORD pdwTotSector);
//          ========================
//
// When PrimoSDK is testing, recording, extracting or erasing, this functions controls
//  the status of the operation.
//
//   Param: dwHandle is the operation handle
//
//          dwFlags is PRIMOSDK_GETSTATUS to get the status only or
//                PRIMOSDK_ABORT to abort the operation
//
//          pdwCurSector returns the currently recording sector
//
//          pdwTotSector returns the total sectors to record
//
//   Notes: PrimoSDK_RunningStatus does not
//          tell you if the operation terminated successfully, but only
//          if it is still going or not.  Please use PrimoSDK_UnitStatus for detailed error from the drive.
//
//   Notes: This function should be called every 1, 2 or 3 seconds (in a timer
//          loop), to not clog the system. After having called with the
//          PRIMOSDK_ABORT flag do not expect that this function will
//          immediately return with a PRIMOSDK_USERABORT reply. Some recording
//          situations take time to conclude and it is possible to see
//          this function keep returning PRIMOSDK_RUNNING for
//          a while, before it aknowledges the requested abort.
//          When mastering data (using PrimoSDK_WriteImage) this
//          function will initially return pdwTotSector as 0, until the
//          premastering is completed and it becomes known how many sectors
//          are required exactly. The caller can assume that between PrimoSDK_WriteImage is
//          launched and PrimoSDK_RunningStatus starts returning a value
//          greater than 0 in pdwTotSector the engine is in the pre-mastering
//          phase.
//          When called after PrimoSDK_WriteImage, and after pdwTotSector
//          becomes greater than 0, the mastering stays paused for 2 seconds, giving
//          time to the caller to aknowledge the real size of the recording
//          and aborting the operation in time to not start any real recording.
//          This is particularly useful in close-to-full situations, as the
//          PrimoSDK_WriteImage function, when launched, returns in pdwSize only
//          an initial approximation of the needed total number of seconds.
//
//  Return: PRIMOSDK_OK if the operation terminated OK
//          PRIMOSDK_RUNNING if OK and still running
//          PRIMOSDK_USERABORT if we aborted because of a call
//                with the PRIMOSDK_ABORT flag
//          PRIMOSDK_FILEERROR if error writing a file during extraction or while
//                creating an image
//          PRIMOSDK_CMDSEQUENCE if nothing is running
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//          PRIMOSDK_FEATURE_NOT_SUPPORTED if the device does not support the feature requested.
//



DWORD WINAPI PrimoSDK_UnitStatus(DWORD dwHandle, PDWORD pdwUnit, PDWORD pdwCommand,
								 PDWORD pdwSense, PDWORD pdwASC, PDWORD pdwASCQ);
//          =====================
//
// Returns the status of a drive and, if a unit error happened, returns also
//  the last Sense, ASC and ASCQ. Please, refer to the SCSI or ATAPI specs of the
//   unit for the meaning of Sense, ASC and ASCQ.
// This function can be called at any time, but its main scope is to let
//  the main application control the status of every unit during multiple
//   drives operations.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification
//                (see PrimoSDK_UnitInfo for field format)
//
//          pdwCommand points where the last command code is returned
//                when a unit error occurs
//
//          pdwSense points where the Sense code is returned
//                when a unit error occurs
//
//          pdwASC points where the Additional Sense Code is returned
//                when a unit error occurs
//
//          pdwASCQ points where the Additional Sense Code Qualifiers
//                is returned when a unit error occurs
//
//   Notes: Always use this function to control the status and the final
//          result of your operation. PrimoSDK_RunningStatus does not
//          tell you if the operation terminated successfully, but only
//          if it is still going or not.
//
//  Return: PRIMOSDK_OK if the unit is not in error, i.e. the last operation
//          terminated OK.
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BUR if the unit went in buffer under run;
//                pCommand, pSense, pASC and pASCQ are set
//          PRIMOSDK_READERROR if the unit reported a reading error;
//                pCommand, pSense, pASC and pASCQ are set
//          PRIMOSDK_WRITEERROR if the unit reported a recording error;
//                pCommand, pSense, pASC and pASCQ are set
//          PRIMOSDK_UNITERROR if the unit reported another error;
//                pCommand, pSense, pASC and pASCQ are set
//          PRIMOSDK_SCSIERROR if a communication error occurred
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_ListSupportedUnits(PBYTE szList);
//          =============================
//
// This API has been deprecated.  There are many drives supported by this
// SDK that are not specifically returned by this API.
//
// Returns a list of the supported drives, one each line, separated by
// <CR><LF> and terminated by <00>. Each line contains 3 fields separated
// by comma, the Vendor ID, the Product ID and the Px class.
//
//   Param: szList is the receiving buffer
//
//   Notes: szList must be at least 10000 bytes in length.
//          The Vendor ID and the Product are returned exactly as they are spelled
//          by the unit. If the vendor ID is all blank (some manufacturer are making
//          model that way) then the Vendor ID is returned as "[no brand]".
//
//  Return: number of the entries in the list.
//



DWORD WINAPI PrimoSDK_Command(DWORD dwHandle, PDWORD pdwUnit, DWORD dwCmdLen, PBYTE pCmd,
							  DWORD dwDataLen, PBYTE pData, DWORD dwFlag, DWORD dwTimeOut,
							  PDWORD pdwSense, PDWORD pdwASC, PDWORD pdwASCQ);
//          ==================
//
// Sends a bare SCSI command to the specified SCSI or ATAPI unit. Even if the target can be
// a CD or DVD drive, this function is provided mostly to safely send commands to SCSI
// robotics or other special devices, still using the same PrimoSDK ASPI layer/control,
// and not having to install another layer in the system.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification (see ISORep_UnitInfo
//                for field format). When addressing robotics the lower byte must be 00, and
//                it will still be 00 when the function returns, because the robotics
//                does not have any drive letter assigned. Therefore, the robotics
//                can be addresses only by SCSI triple
//
//          dwCmdLen is the length of the SCSI command (usually from 6 to
//                12 bytes)
//
//          pCmd points to the actual SCSI command
//
//          dwDataLen is the length of the data buffer, if any
//
//          pDataLen points to where the data transferred by the command
//                is read or written; can be NULL if the command has no data
//
//          dwFlag can be PRIMOSDK_NODATA, PRIMOSDK_DATAIN or
//                PRIMOSDK_DATAOUT depending how and if the command has data
//
//          dwTimeOut is the command time-out in millisecond
//
//          pdwSense points where the Sense code is returned
//
//          pdwASC points where the Additional Sense Code is returned
//
//          pdwASCQ points where the Additional Sense Code Qualifiers
//                is returned
//
//   Notes: This function must be used carefully and by programmers with
//          experience sending SCSI commands, as the effects using wrong
//          target drives or wrong commands could be fatal to the system.
//
//  Return: PRIMOSDK_OK if the command completes successfully
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_SCSIERROR if a communication error occurred
//                or if the command went in time-out
//          PRIMOSDK_UNITERROR if the command returned a check
//                condition, then pSense, pASC and pASCQ point
//                to the corresponding error triple
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//


DWORD WINAPI PrimoSDK_ReadDisc(DWORD dwHandle, PDWORD pdwUnit, DWORD dwTrack,
							   DWORD dwSector, DWORD dwSectorCount, PBYTE pBf);
//          ===================
//
// Reads sectors directly off the disc.
//
// The programmer must call PrimoSDK_OpenReadDisc, read one or more areas of the disc
// with PrimoSDK_ReadDisc, and finally close the session with PrimoSDK_CloseReadDisc.
//
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification
//                (see PrimoSDK_UnitInfo for field format)
//
//          dwTrack is the track number that will be read from
//
//          dwSector is the sector index to start reading
//
//          dwSectorCount is the number of sectors to read
//
//          pBf is the buffer to receive the data
//
//
//   Notes: The parameters pdwUnit and dwTrack must be the same as in the previous
//          call to PrimoSDK_OpenReadDisc.
//
//          This call does not return until the data is read off the disc.
//
//  Return: PRIMOSDK_OK if the command completes successfully
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters (example: dwTrack out of range 1..n)
//          PRIMOSDK_INTERR if an internal error occurred
//


DWORD WINAPI PrimoSDK_OpenReadDisc(DWORD dwHandle, PDWORD pdwUnit, DWORD dwTrack);
DWORD WINAPI PrimoSDK_OpenReadDiscEx(DWORD dwHandle, PDWORD pdwUnit, DWORD dwTrack, DWORD dwSpeed);
//          ===================
//
// Prepare to read sectors off the disk with PrimoSDK_ReadDisc.
//
// The programmer must call PrimoSDK_OpenReadDisc, read one or more areas of the disc
// with PrimoSDK_ReadDisc, and finally close the session with PrimoSDK_CloseReadDisc.
//
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification
//                (see PrimoSDK_UnitInfo for field format)
//
//          dwTrack is the track number that will be read from
//
//          dwSpeed (Ex version only) is the read speed requested
//
//   Notes: None.
//
//
//  Return: PRIMOSDK_OK if the command completes successfully
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters (example: dwTrack out of range 1..n)
//          PRIMOSDK_INTERR if an internal error occurred
//


DWORD WINAPI PrimoSDK_CloseReadDisc(DWORD dwHandle, PDWORD pdwUnit, DWORD dwTrack);
//          ========================
//
// Close and destroy a ReadDisc session.
//
// This function must always be called after a PrimoSDK_OpenReadDisc, to terminate
// and free the allocated structures.
//
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification
//                (see PrimoSDK_UnitInfo for field format)
//
//          dwTrack is the track number that will be read from
//
//   Notes: The parameters pdwUnit and dwTrack must be the same as in the previous
//          call to PrimoSDK_OpenReadDisc.
//
//
//  Return: PRIMOSDK_OK if the command completes successfully
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters (example: dwTrack out of range 1..n)
//          PRIMOSDK_INTERR if an internal error occurred
//


DWORD WINAPI PrimoSDK_PlayAudio(DWORD dwHandle, PDWORD pdwUnit, DWORD StartLba, DWORD Length);
//          ===================
//
// Play Audio from the a given StartLba for a given length (in sectors)
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification
//                (see PrimoSDK_UnitInfo for field format)
//
//          StartLba: Start sector (logical block Address)
//
//          Length : Length (number of sectors)
//
//  Return: PRIMOSDK_OK if the command completes successfully
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_SCSIERROR if a communication error occurred
//                or if the command went in time-out
//                to the corresponding error triple
//          PRIMOSDK_UNITERROR if the command returned a check
//                condition.
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//

DWORD WINAPI PrimoSDK_PauseResumeAudio(DWORD dwHandle, PDWORD pdwUnit, BOOL bResume);
//          ===================
//
//   Pause or Resume Audio
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification
//                (see PrimoSDK_UnitInfo for field format)
//
//          bResume : TRUE = Continue playing, FALSE = pause audio
//
//  Return: PRIMOSDK_OK if the command completes successfully
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_SCSIERROR if a communication error occurred
//                or if the command went in time-out
//          PRIMOSDK_UNITERROR if the command returned a check
//                condition.
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//

DWORD WINAPI PrimoSDK_StopAudio(DWORD dwHandle, PDWORD pdwUnit);
//          ===================
//
//   Stop Audio from Playing
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification
//                (see PrimoSDK_UnitInfo for field format)
//
//  Return: PRIMOSDK_OK if the command completes successfully
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_SCSIERROR if a communication error occurred
//                or if the command went in time-out
//          PRIMOSDK_UNITERROR if the command returned a check
//                condition.
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//

DWORD WINAPI PrimoSDK_GetPositionAudio(DWORD dwHandle, PDWORD pdwUnit,
									   PDWORD pdwRelPosition,
									   PDWORD pdwAbsPosition);
//          ===================
//
//   Gets the Position of the Audio CD
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification
//                (see PrimoSDK_UnitInfo for field format)
//
//          pdwRelPosition points to a DWORD containing the Relative position in sectors
//
//          pdwAbsPosition points to a DWORD containing the Absolute position in sectors
//
//  Return: PRIMOSDK_OK if the command completes successfully
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_SCSIERROR if a communication error occurred
//                or if the command went in time-out
//          PRIMOSDK_UNITERROR if the command returned a check
//                condition.
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//          PRIMOSDK_BADPARAM if the pointers for the positions are NULL


DWORD WINAPI PrimoSDK_AddFolderWCS(DWORD dwHandle, WCHAR *szFolder);
//          ====================
//
// Add a folder to the "CD Image".
//
// Same as PrimoSDK_AddFolder except that the names are Unicode (Wide Characters)
//
// The folder must always be fully specified. Therefore, when adding a subfolder also
// the parent is specified. This function adds only one level at a time.
// For example, to create the path "\My Folder\My Sub Folder" and the path
// "\My Folder\My Second Sub Folder", that is:
//
// \My Folder
//      \My Sub Folder
//      \My Second Sub Folder
//
// The calls to this function should be:
//
// PrimoSDK_AddFolder(dwHandle,"\My Folder");
// PrimoSDK_AddFolder(dwHandle,"\My Folder\My Sub Folder");
// PrimoSDK_AddFolder(dwHandle,"\My Folder\My Second Sub Folder");
//
//   Param: dwHandle is the operation handle
//
//          szFolder specifies the folder name in MBCS
//
//   Notes: If PRIMOSDK_ISOLEVEL1 is specified, the folder must not have
//          any extension; it can be just up to 8 char.
//          The first backslash is mandatory. A trailer backslash
//          is optional. There is no need to create the root folder.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or, CD Image started or
//                if (only if PrimoSDK_NewImage was called with  PRIMOSDK_CHECKDUPLI)
//                the nested folders have not been added yet
//          PRIMOSDK_BADPARAM if incorrect or too long folder name
//          PRIMOSDK_ALREADYEXIST if the folder has been already added
//                (only if PrimoSDK_NewImage was called with  PRIMOSDK_CHECKDUPLI)
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_NOSPACE if the internal tables went in overflow (too many
//                files for the system memory)
//          PRIMOSDK_INTERR if an internal error occured
//


DWORD WINAPI PrimoSDK_AddFileWCS(DWORD dwHandle, PWORD szFileOnCD, PWORD szSourceFile);
//          ==================
//
// Add szSourceFile to the "CD Image" with the name szFileOnCD.
// Same as PrimoSDK_AddFile except that the names are Unicode (Wide Characters)
//
// Both files must be fully specified, and the folder must already exist.
// For example:
//
// PrimoSDK_AddFile(dwHandle,"\My File.Txt","D:\Source\My File.Txt")
//   adds the file "My File.Txt" that is stored in "D:\Source"
//    to the CD root, with the same name.
//
// PrimoSDK_AddFile(dwHandle,"\My Folder\Your File.Txt","D:\Source\My File.Txt")
//   adds the same file to the "\My Folder" on the CD, with
//    the new name "Your File.Txt". "\My Folder" must have been already
//     added to the CD Image with a previous call to PrimoSDK_AddFolder.
//
//   Param: dwHandle is the operation handle
//
//          szFileOnCD specifies the file name on CD in Unicode
//
//          szSourceFile specifies the source file name in Unicode
//
//   Notes: None.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized, no CD Image started or
//                if (only if PrimoSDK_NewImage was called with  PRIMOSDK_CHECKDUPLI)
//                the nested folders have not been added yet
//          PRIMOSDK_BADPARAM if incorrect or too long file name
//          PRIMOSDK_ALREADYEXIST if szFileOnCD name has been already added
//                (only if PrimoSDK_NewImage was called with PRIMOSDK_CHECKDUPLI)
//          PRIMOSDK_NOTREADABLE if the source file is not found or not readable
//          PRIMOSDK_FILEERROR if a file that was added is invalid
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_NOSPACE if the internal tables went in overflow (too many
//                files for the system memory)
//          PRIMOSDK_INTERR if an internal error occured
//          PRIMOSDK_FILETOOLARGE if a file that was added is bigger than 9.99 GB for UDF
//                                or bigger than 4 GB for ISO.
//

DWORD WINAPI PrimoSDK_GetSpaceUsed(DWORD dwHandle, PDWORD pdwSize, PDWORD pdwLastSector);
//          ========================
//
// Gets the space used for the current data set for the next data job.
//
// pdwSize: returns the number of sectors used (meta data + data)
//
// pdwLastSector: last sector that will be written to.
//
// Notes: use to subtract from DiscInfo's reported capacity for space left on media
// pdwSize gives full size of Meta Data + Data for session / (no media = assumed blank)
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured



DWORD WINAPI PrimoSDK_NewImageWCS(DWORD dwHandle, PDWORD pdwUnits, PWORD szVolumeNameWCS,
								  DWORD dwTrackToLoad, DWORD dwFlags, DWORD dwSwapThreshold,
								  PBYTE szTemp);
//          ======================
//
// Start a new Data "CD Image". (Same as PrimoSDK_NewImage but with wide character volume name string
//
// The programmer must call this function, next add directory and files with
// PrimoSDK_AddFolder and PrimoSDK_AddFile, then call the PrimoSDK_WriteImage to
// record (or test) the image. A call to PrimoSDK_CloseImage will destroy the structure.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnits points to a vector of DWORD containing the units identification,
//                (see PrimoSDK_UnitInfo for field format) terminated by 0xFFFFFFFF.
//                If the caller is starting a new image to just build an ISO or UDF
//                image file using PrimoSDK_SaveImage, or a Global Image with
//                PrimoSDK_SaveGI, no unit is required; in this case just pass the
//                0xFFFFFFFF in the first position
//
//          dwSwapThreshold sets the size (in KB) of the files that are copied in the temporary.
//                For example, 4 means that all the files under 4096 bytes in size will be
//                copied by PrimoSDK in a swap file generated in szTemp. This file will be
//                destroyed by PrimoSDK_CloseImage. A value of 0 means that no swap file is
//                generated. The caching maximum value is 256.
//                The value 0xFFFFFFFF means that all files up to 2 GB will be cached. If
//                all files are less than 2GB, then  a complete image will exist on the
//                hard disk before writing to CD/DVD.
//
//          szTemp is a user directory where to generate the swap files.
//
//          szVolumeNameWCS specifies the volume label of the CD and some optional PVD fields in
//                one of the following formats:
//                   \nVolume[\nPublisher[\nDataPreparer[\nApplicationID[\nSystemID]]]]
//                   Volume[,Publisher[,DataPreparer[,ApplicationID]]]
//                The max lengths are 32 for the Volume and 128 for the other three fields,
//                in characters.
//
//          dwTrackToLoad specifies if and which track should be loaded (linked)
//                to this. If 0 no track is loaded and this will be like a new
//                volume in the free space.
//                To append to DVD+RW or DVD-RW media, this value should always be 1.
//                For all other media, a value of 1 loads the first track,
//                2 the second and so on. The caller should check to ask for a track
//                that already exists on the disc, otherwise a PRIMOSDK_BADPARAM
//                is returned. To create an incremental disc this field should
//                always specify the last track present on the disc.
//                To append to BD-RE or BD-R (POW) media, If 0, no track will be loaded. A non-zero
//                value shall indicate appending to the last session.
//
//          dwFlags is a combination of the following values:
//
//                Set only ONE of the following values:
//
//                PRIMOSDK_ISOLEVEL1  for an ISO 9660 Level 1 compliant CD with
//                                  folder and file names in the 8+3 format
//                                  using only the A-:-Z, 0-:-9 and "_"
//                                  char. set
//                PRIMOSDK_ISOLEVEL2  for an ISO 9660 Level 2 compliant CD
//                                  (DOS OEM characters)
//                PRIMOSDK_ISOLEVEL3  for an ISO 9660 Level 3 compliant CD
//                                  (DOS OEM characters, long file names)
//                PRIMOSDK_JOLIET for a Microsoft Joliet compliant CD with
//                                  filenames up to 106 chars in all the Windows
//                                  legal charset, double chars. included (MBCS)
//                                  [Technical spec limits at 64, readers generally supports more]
//                PRIMOSDK_UDF for a UDF 1.02 bridge file system
//                                  (bridge includes Joliet/ISO, which limits
//                                  filenames to 106 chars)
//                PRIMOSDK_UDF201 for a UDF 2.01 bridge file system
//                                  (bridge includes Joliet/ISO, which limits
//                                  filenames to 106 chars)
//
//           OR one of the values above with any of the following:
//
//                PRIMOSDK_ORIGDATE to mantain the original date/time for all files
//                                  (this flag is mutually exclusive with PRIMOSDK_SETNOW)
//                PRIMOSDK_USERTIMESET to supply a date/time for all streamed files
//                                  (this flag is mutually exclusive with PRIMOSDK_SETNOW)
//                PRIMOSDK_SETNOW to have all files set to the time/date at which the
//                                  PrimoSDK_NewImageWCS is called.
//                                  (this flag is mutually exclusive with PRIMOSDK_ORIGDATE
//                                  and PRIMOSDK_USERTIMESET)
//                PRIMOSDK_MODE1 to make a CD-ROM Mode 1 disc or a DVD
//                                  (this flag is mutually exclusive with PRIMOSDK_MODE2)
//                PRIMOSDK_MODE2 to make a CD-ROM Mode 2 XA disc
//                                  (this flag is mutually exclusive with PRIMOSDK_MODE1)
//                PRIMOSDK_SAO if the disc must be written Session At Once. This flag, combined
//                                  with the following PRIMOSDK_CLOSEDISC let you make any
//                                  type of recording. When PRIMOSDK_SAO is not specified
//                                  the recording is done Track At Once
//                                  Note: for media larger than 2GB (DVD/BD) this will
//                                  create closed media by default even if PRIMOSDK_CLOSEDISC
//                                  is not specified.
//                PRIMOSDK_TAO if the disc is written Track At Once. Required for DVD
//                                  multi-border (DVD-RW, DVD-R, DVD+R). Optional for
//                                  CD-R/RW, DDCD-R/RW.  Must not be combined with
//                                  PRIMOSDK_SAO or PRIMOSDK_BADPARAM will be returned.
//                PRIMOSDK_CLOSEDISC if the disc must be closed so no other session can
//                                  be added.
//                PRIMOSDK_CHECKDUPLI if the caller prefers that all the subsequent calls to
//                                  PrimoSDK_AddFolder and to PrimoSDK_AddFile will do a
//                                  check before adding. It will be checked that the file
//                                  or folder does not already exist (PRIMOSDK_ALREADYEXIST
//                                  is returned then) and that the nested path has been
//                                  already created (PRIMOSDK_CMDSEQUENCE in that case).
//                                  The use of this flag could slow down the time needed to add
//                                  all the files to the image.
//                PRIMOSDK_RESETDRIVES is a special value that must be passed alone in dwFlags
//                                  to redefines the units and volume name before a
//                                  PrimoSDK_WriteImage of an additional round
//                PRIMOSDK_VIDEOCD enables the engine recognition of the VCD 2.0/SVCD directories
//                PRIMOSDK_DVDIMAGE set this if the image is to be written to a DVD. This flag is
//                                  ignored if there are units in pdwUnits.
//
//   Notes: Only the first 11 characters of the volume label are usually visible.
//          The caller should use PRIMOSDK_RESETDRIVES when a round has been completed and
//          there is the need to make another round, on different units or with different
//          volume names, without rebuilding again the image.
//          The optimal value of dwSwapThreshold greatly changes from machine to machine.
//          A fast PC can work without swap, but usually a swap from 2KB to 8KB  is fine
//          when in presence of a large number of files. Avoid using large values of
//          dwSwapThreshold, as it could cause the cache file to exceed available hard disk
//          space.
//          Use the flag PRIMOSDK_CHECKDUPLI sparingly, as it could slow down the time needed to
//          add all the files to the image. It is suggested to use this flag only during the
//          application debug, until you are sure that the call sequence is completely correct.
//          If writing to a DVD the PRIMOSDK_MODE2 should not be used. If it is,
//          PRIMOSDK_MODE1 will be automatically used instead.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_NOTREADY if the unit is not ready
//          PRIMOSDK_ERRORLOADING if there is an error while loading a previous track
//          PRIMOSDK_INVALIDMEDIUM if the target disc are not blank nor appendable
//                or if you are recording on several appendable discs but they
//                are not equal
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//


DWORD WINAPI PrimoSDK_SetProperty(DWORD dwHandle, DWORD Property, DWORD BufferSize, void *PropertyBuffer,
								  DWORD ExtendedInfo, void *ExtendedData);
//          ======================
//
// Set a property controlling the specified PrimoSDK handle.  This property will be
// in effect for all APIs that use a PrimoSDK dwHandle parameter.
//
//   Param: dwHandle is the operation handle
//
//          Property is the property to set.  General properties may be added and
//                defined below.  Additional properties may also exist in advanced
//                API sections below.  Properties will be listed with the header:
//                "PrimoSDK_SetProperty Property values"
//
//          BufferSize is the size of the PropertyBuffer supplied by the client.
//
//          PropertyBuffer is the property data to set.  Varies depending on the property
//                value.
//
//          ExtendedInfo is reserved for future use, should be 0.
//
//          ExtendedData is reserved for future use, should be NULL.
//
//  Return: PRIMOSDK_OK if no error.
//          PRIMOSDK_INVALIDPROP if an invalid property is specified
//



DWORD WINAPI PrimoSDK_Init(PDWORD pdwRelease);
//          ===============
//
// Initialize PrimoSDK.
//
// This function must be called before any other PrimoSDK call, excluding PrimoSDK_Trace,
// to initialize the internal structures.
//
//   Param: pdwRelease points to a DWORD that receives the PRIMOSDK.DLL
//                version number 0xRRSSBBBB
//                RR = Version number
//                SS = Sub Version number
//                BBBB = Build number
//
//   Notes: You should call this function only once, when your
//          application or module starts.
//          PrimoSDK needs an ASPI layer to communicate with both
//          ATAPI or SCSI units. A compatible WinASPI layer
//          or PxHelper (recommended) can be used.
//
//  Return: PRIMOSDK_CDDVDVERSION if OK, this the SDK retail CD and DVD version
//          PRIMOSDK_DEMOVERSION if OK, this the SDK demo version
//          PRIMOSDK_CMDSEQUENCE if already initialized
//          PRIMOSDK_NOASPI if the ASPI layer is not loading or is in error
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_NewImage(DWORD dwHandle, PDWORD pdwUnits, PBYTE szVolumeName,
							   DWORD dwTrackToLoad, DWORD dwFlags, DWORD dwSwapThreshold,
							   PBYTE szTemp);
//          ===================
//
// Start a new Data "CD Image".
//
// The programmer must call this function, next add directory and files with
// PrimoSDK_AddFolder and PrimoSDK_AddFile, then call the PrimoSDK_WriteImage to
// record (or test) the image. A call to PrimoSDK_CloseImage will destroy the structure.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnits points to a vector of DWORD containing the units identification,
//                (see PrimoSDK_UnitInfo for field format) terminated by 0xFFFFFFFF.
//                If the caller is starting a new image to just build an ISO or UDF
//                image file using PrimoSDK_SaveImage, or a Global Image with
//                PrimoSDK_SaveGI, no unit is required; in this case just pass the
//                0xFFFFFFFF in the first position
//
//          dwSwapThreshold sets the size (in KB) of the files that are copied in the temporary.
//                For example, 4 means that all the files under 4096 bytes in size will be
//                copied by PrimoSDK in a swap file generated in szTemp. This file will be
//                destroyed by PrimoSDK_CloseImage. A value of 0 means that no swap file is
//                generated. The caching maximum value is 256.
//                The value 0xFFFFFFFF means that all files up to 2 GB will be cached. If
//                all files are less than 2GB, then  a complete image will exist on the
//                hard disk before writing to CD/DVD.
//
//          szTemp is a user directory where to generate the swap files.
//
//          szVolumeName specifies the volume label of the CD and some optional PVD fields in
//                one of the following formats:
//                   \nVolume[\nPublisher[\nDataPreparer[\nApplicationID[\nSystemID]]]]
//                   Volume[,Publisher[,DataPreparer[,ApplicationID]]]
//                The max lengths are 32 for the Volume and 128 for the other three fields,
//                in bytes.
//
//          dwTrackToLoad specifies if and which track should be loaded (linked)
//                to this. If 0 no track is loaded and this will be like a new
//                volume in the free space.
//                To append to DVD+RW or DVD-RW media, this value should always be 1.
//                For all other media, a value of 1 loads the first track,
//                2 the second and so on. The caller should check to ask for a track
//                that already exists on the disc, otherwise a PRIMOSDK_BADPARAM
//                is returned. To create an incremental disc this field should
//                always specify the last track present on the disc.
//                To append to BD-RE or BD-R (POW) media, If 0, no track will be loaded. A non-zero
//                value shall indicate appending to the last session.
//
//
//          dwFlags is a combination of the following values:
//
//                Set only ONE of the following values:
//
//                PRIMOSDK_ISOLEVEL1  for an ISO 9660 Level 1 compliant CD with
//                                  folder and file names in the 8+3 format
//                                  using only the A-:-Z, 0-:-9 and "_"
//                                  char. set
//                PRIMOSDK_ISOLEVEL2  for an ISO 9660 Level 2 compliant CD
//                                  (DOS OEM characters)
//                PRIMOSDK_ISOLEVEL3  for an ISO 9660 Level 3 compliant CD
//                                  (DOS OEM characters, long file names)
//                PRIMOSDK_JOLIET for a Microsoft Joliet compliant CD with
//                                  filenames up to 106 chars in all the Windows
//                                  legal charset, double chars. included (MBCS)
//                PRIMOSDK_UDF for a UDF 1.02 bridge file system
//                PRIMOSDK_UDF201 for a UDF 2.01 bridge file system
//
//           OR one of the values above with any of the following:
//
//                PRIMOSDK_ORIGDATE to mantain the original date/time for all files
//                                  (this flag is mutually exclusive with PRIMOSDK_SETNOW)
//                PRIMOSDK_SETNOW to have all files set to the time/date at which the
//                                  PrimoSDK_NewImage is called.
//                                  (this flag is mutually exclusive with PRIMOSDK_ORIGDATE
//                                  and PRIMOSDK_USERTIMESET)
//                PRIMOSDK_MODE1 to make a CD-ROM Mode 1 disc or a DVD
//                                  (this flag is mutually exclusive with PRIMOSDK_MODE2)
//                PRIMOSDK_MODE2 to make a CD-ROM Mode 2 XA disc
//                                  (this flag is mutually exclusive with PRIMOSDK_MODE1)
//                PRIMOSDK_SAO if the disc must be written Session At Once. This flag, combined
//                                  with the following PRIMOSDK_CLOSEDISC let you make any
//                                  type of recording. When PRIMOSDK_SAO is not specified
//                                  the recording is done Track At Once
//                                  Note: for media larger than 2GB (DVD/BD) this will
//                                  create closed media by default even if PRIMOSDK_CLOSEDISC
//                                  is not specified.
//                PRIMOSDK_TAO if the disc is written Track At Once. Required for DVD
//                                  multi-border (DVD-RW, DVD-R, DVD+R). Optional for
//                                  CD-R/RW, DDCD-R/RW.  Must not be combined with
//                                  PRIMOSDK_SAO or PRIMOSDK_BADPARAM will be returned.
//                PRIMOSDK_CLOSEDISC if the disc must be closed so no other session can
//                                  be added.
//                PRIMOSDK_CHECKDUPLI if the caller prefers that all the subsequent calls to
//                                  PrimoSDK_AddFolder and to PrimoSDK_AddFile will do a
//                                  check before adding. It will be checked that the file
//                                  or folder does not already exist (PRIMOSDK_ALREADYEXIST
//                                  is returned then) and that the nested path has been
//                                  already created (PRIMOSDK_CMDSEQUENCE in that case).
//                                  The use of this flag could slow down the time needed to add
//                                  all the files to the image.
//                PRIMOSDK_RESETDRIVES is a special value that must be passed alone in dwFlags
//                                  to redefines the units and volume name before a
//                                  PrimoSDK_WriteImage of an additional round
//                PRIMOSDK_VIDEOCD enables the engine recognition of the VCD 2.0/SVCD directories
//                PRIMOSDK_DVDIMAGE set this if the image is to be written to a DVD. This flag is
//                                  ignored if there are units in pdwUnits.
//
//   Notes: Only the first 11 characters of the volume label are usually visible.
//          The caller should use PRIMOSDK_RESETDRIVES when a round has been completed and
//          there is the need to make another round, on different units or with different
//          volume names, without rebuilding again the image.
//          The optimal value of dwSwapThreshold greatly changes from machine to machine.
//          A fast PC can work without swap, but usually a swap from 2KB to 8KB  is fine
//          when in presence of a large number of files. Avoid using large values of
//          dwSwapThreshold, as it could cause the cache file to exceed available hard disk
//          space.
//          Use the flag PRIMOSDK_CHECKDUPLI sparingly, as it could slow down the time needed to
//          add all the files to the image. It is suggested to use this flag only during the
//          application debug, until you are sure that the call sequence is completely correct.
//          If writing to a DVD the PRIMOSDK_MODE2 should not be used. If it is,
//          PRIMOSDK_MODE1 will be automatically used instead.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_NOTREADY if the unit is not ready
//          PRIMOSDK_ERRORLOADING if there is an error while loading a previous track
//          PRIMOSDK_INVALIDMEDIUM if the target disc are not blank nor appendable
//                or if you are recording on several appendable discs but they
//                are not equal
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//




/////////////////////////////////////////////////////////////////////
//                                                                 //
//                                                                 //
//                   Advanced Audio API section                    //
//                ==================================               //
//                                                                 //
//   The following APIs are additional APIs included in this       //
//   SDK version that allow sourcing audio for the disc            //
//   from a stream.  Also includes other advanced audio            //
//   features.                                                     //
//                                                                 //
/////////////////////////////////////////////////////////////////////

// PrimoSDK_SetProperty Property values
#define PRIMOSDK_PROPERTY_ENABLE_DAP   1  // PropertyBuffer = DWORD, set to one of the following:
//  1 = Use DAP for PrimoSDK_ReadDisc commands if the
//      drive and track type supports it.
//  0 = Use standard PrimoSDK_ReadDisc commands


DWORD WINAPI PrimoSDK_AddAudioEffect(DWORD dwHandle, DWORD dwTrack, DWORD dwEffectType,
									 DWORD dwParameterDwords, PDWORD pdwParameters);
//          ======================
//
// Apply an effect to an audio track.
//
//   Param: dwHandle is the operation handle
//
//          dwTrack is the number of an Audio track previously added with PrimoSDK_AddAudioTrack(Ex)
//                  or PrimoSDK_AddAudioStream (1 = the first track added)
//
//          dwEffectType is one of the following:
//            PRIMOSDK_NORMALIZE - normalize the audio for the track
//                            dwParameterDwords must be 2 (indicating that the pdwParameters array
//                                              contains 2 dwords)
//                            pdwParameters points to a 2 dword array containing the following dwords:
//                              [0] - The peak level for this track that was returned from
//                                    PrimoSDK_GetAudioTrackInfo (0 - 10000)
//                              [1] - The percent of the peak level to scale the audio to from 0 - 10000.
//                                    10000 means that the audio is scaled so that the peak value of the
//                                    track is scaled to 100.00% of the maximum sample value.
//
//          dwParameterDwords is the size (in dwords) that the caller supplies for the pdwParameters array
//
//          pdwParameters is an array of dwords that is the parameters for the effect type specified
//                        (see dwEffectType for definitions)
//
//   Notes: This function must be called after the specified track has been added, but before
//          calling PrimoSDK_WriteAudio.  Some effects may require info that can be retrieved
//          with PrimoSDK_GetAudioTrackInfo.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no Audio CD started
//          PRIMOSDK_BADPARAM if the passed parametrs are not valid
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//
#define PRIMOSDK_NORMALIZE 1



DWORD WINAPI PrimoSDK_AddAudioTrackEx(DWORD  dwHandle, PBYTE szTrack,
									  DWORD  dwPreGapSilence,
									  DWORD  dwPreGapAudio,
									  DWORD  dwFlags,
									  PDWORD pdwSize,
									  PBYTE  pISRC,
									  DWORD  dwIndexCount,
									  PDWORD pdwIndexArray);
DWORD WINAPI PrimoSDK_AddAudioTrackExWcs(DWORD  dwHandle, WCHAR *wcsTrack,
		DWORD  dwPreGapSilence,
		DWORD  dwPreGapAudio,
		DWORD  dwFlags,
		PDWORD pdwSize,
		PBYTE  pISRC,
		DWORD  dwIndexCount,
		PDWORD pdwIndexArray);
//          ========================
//
// Add a track to the Audio CD. User shall call PrimoSDK_NewAudio first before call this function.
//
//   Param: dwHandle is the operation handle
//
//          szTrack specifies an audio file
//
//          dwPregap the gap, in sectors, of the track. The first
//                track added (first song) will have a Pre-gap
//                of 150 block, no matter what value is passed
//
//          dwFlags
//                PRIMOSDK_EMPHASIS for enable emphasis recording.
//                PRIMOSDK_COPYRIGHT for setting the copyright bit.
//
//          pdwSize points where is returned the size in sector
//                of the audio file
//
//          dwPreGapSilence defines as the number of sectors or frames to remain silence in the pregap
//
//          dwPreGapAudio (> 0) defines as the number of sectors or frames containing audio from the current track.
//
//          pISRC defines as the international standard recording code.
//
//          dwIndexCount defines as the number of track indices to be recorded and in the array below (max 98)
//
//          pdwIndexArray defines as the array of indices for the trace.  (Starting from Index 2)
//
//   Notes: pregap = dwPregapAudio + dwPreGapSilence
//          dwPregapAudio = 0 and dwPreGapSilence = 0, means no pregap
//          extended pregap is not supported (> 225 sectors).
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured



DWORD WINAPI PrimoSDK_WriteAudioTrackStream(DWORD dwHandle, PDWORD pdwUnits,
		PrimoSDK_StreamCallback  pFillerFn,
		PVOID pContext, DWORD dwSize, DWORD dwFlags,
		DWORD dwSpeed);
//          ==========================
//
// See the description of PrimoSDK_WriteAudioTrack above.  This acts exactly
// the same except instead of reading the audio from a file, the audio is
// supplied by the callback function specified in pFillerFn.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnits points to a vector of DWORD containing the units identification,
//                (see PrimoSDK_UnitInfo for field format) terminated by 0xFFFFFFFF
//
//          pFillerFn is a pointer to the function to call to fill buffers with audio
//                 to be written to the CD.  Must be 44100Hz sample rate, 16-bits per
//                 sample, stereo (2 channel)
//
//          pContext is a value to be passed back to the callback function.
//                 Useful to identify to the callback function which stream data
//                 is being requested.  PrimoSDK does not use this parameter, it
//                 just passes this value to the callback.
//
//          dwSize is the size in sectors of the audio for this track.
//
//          dwFlags is (OR them if more than one):
//                PRIMOSDK_WRITE for the real recording or PRIMOSDK_TEST for test
//                PRIMOSDK_BURNPROOF if enable the BURN-Proof support if available
//                PRIMOSDK_CLOSEDISC for the last track to finalize the disc *Note:
//                                   this MUST be specified for the last track on the
//                                   disc to be compatible with CD players.
//
//          dwSpeed defines the speed to use for recording:
//                PRIMOSDK_MAX or n (like 8 for 8x) or PRIMOSDK_BEST if the drive supports AWS
//
//   Notes: The disc won't be playable on CD-ROM or consumer Audio CD player until
//          it is closed.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no Audio CD started
//                or the Audio CD is empty
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_NOTREADY if one or more units are not ready
//          PRIMOSDK_ITSADEMO if trying to record more than permitted with a Demo version
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//


DWORD WINAPI PrimoSDK_AddAudioStream(DWORD dwHandle, PrimoSDK_StreamCallback pFillerFn,
									 PVOID pContext, DWORD dwPreGap, DWORD dwSize);
//          ========================
//
// Adds an Audio Stream to a PrimoSDK_NewAudio job.  The callback function will
// be called when the engine is ready to accept data to be written to the CD.
//
//   Param: dwHandle is the operation handle
//
//          pFillerFn is a pointer to the function to call to fill buffers
//             with audio data to be written to the CD.  Must be 44100Hz
//             sample rate, 16-bits per sample, stereo (2 channel)
//
//          pContext is a value to be passed to the callback function.
//             Useful to identify to the callback function which stream data
//             is being requested.  PrimoSDK does not use this parameter, it
//             just passes this value to the callback.
//
//          dwPreGap is the number of sectors of pregap to be recorded before
//             the track (will be audio silence).
//
//          dwSize is the size in sectors of the audio for this track.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_BADHANDLE if dwHandle is not valid


DWORD WINAPI PrimoSDK_ExtractAudioToBuffer(DWORD dwHandle, PDWORD pdwUnit,
		DWORD dwStartSector, DWORD dwTotalSectors, DWORD dwReadSpeed,
		DWORD dwReserved1, DWORD dwReserved2, DWORD dwReserved3);
//          ===================
//
// Initializes extract audio track to user buffers.  This function returns
// immediatly.  This call must be followed by calls to
// PrimoSDK_NextExtractAudioBuffer to provide buffers to fill.  Progress
// is monitored by calling PrimoSDK_RunningStatus.
//
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification
//                (see PrimoSDK_UnitInfo for field format)
//
//          dwStartSector is the first sector to read from
//
//          dwTotalSectors is the number of sectors to read
//
//          dwReadSpeed is the read speed (PRIMOSDK_MAX = maximum speed)
//
//          dwReserved1
//          dwReserved2
//          dwReserved3 are reserved for future use.  Pass 0 for these parameters.
//
//  Return: PRIMOSDK_OK if the command completes successfully
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or called out of order
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_INTERR if an internal error occurred
//          PRIMOSDK_USERABORT if the track was aborted (need to specify
//                             PRIMOSDK_STARTTRACK to restart the operation)
//


DWORD WINAPI PrimoSDK_NextExtractAudioBuffer(DWORD dwHandle,
		PBYTE pBuffer, DWORD dwBufSize, PDWORD pExpectedSize,
		PDWORD pTargetSector);
//          ===================
//
// The buffer is filled when the curSector returned from PrimoSDK_RunningStatus
// is larger than TargetSector returned from this function.
// To allow effecient read streaming, you can call this function multiple times
// while waiting for buffers to be filled from previous calls.
//
//
//   Param: dwHandle is the operation handle
//
//          pBuffer points to the buffer to read data into
//
//          dwBufSize is the size of pBuffer (must be a multiple of 2352 - audio
//                    sector size)
//
//          pExpectedSize is filled with the amount of data that we expect to receive
//                        in pBuffer when it is filled.  This should equal dwBufSize
//                        except for the last read (which can be 0).
//
//          pTargetSector is filled with the sector number that we must be past
//                        for pBuffer to be valid.
//
//  Return: PRIMOSDK_OK if the command completes successfully
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or called out of order
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_BADPARAM if incorrect parameters (example: Buffer size not multiple of 2352)
//          PRIMOSDK_INTERR if an internal error occurred
//          PRIMOSDK_USERABORT if the track was aborted (need to specify
//                             PRIMOSDK_STARTTRACK to restart the operation)
//


DWORD WINAPI PrimoSDK_GetISRC(DWORD dwHandle, PDWORD pdwUnit, DWORD dwTrack, PBYTE pISRC);
//          ========================
//
// Returns the International Standard Recording Code for an audio track
//
//   Param: dwHandle is the operation handle
//
//          pdwUnit points to a DWORD containing the unit identification.
//                (see PrimoSDK_UnitInfo for field format)
//
//          dwTrack is the track to retrieve the ISRC from
//
//          pISRC points to a 12 byte buffer that receives the ISRC information
//             The buffer is NOT NULL terminated.  It will be filled with 0
//             if no ISRC information is available for the track.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_BADPARAM if dwTrack is not valid
//          PRIMOSDK_READERROR if the drive reports an error reading the ISRC



DWORD WINAPI PrimoSDK_CheckAudioPlaylist(DWORD dwHandle, DWORD dwBufferSizeBytes, PDWORD pdwResults);
//          =====================
//
// Check Audio Playlist to make sure that there are enough rights or access to write the audio playlist.
//
// This function must always be called after a PrimoSDK_NewAudioPlaylist to terminate
// and free the allocated structures.
//
//   Param: dwHandle is the operation handle
//          dwBufferSizeBytes is the buffer size of pdwResults.  Normally, sizeof(DWORD) * Number of Tracks.
//          pdwResults is the pointer to an array of DWORDs, each DWORD represents the result of the
//          each track in the playlist.  Possible errors are PRIMOSDK_PROTECTEDWMA for protected file and
//          PRIMOSDK_OK for no errors.
//
//   Notes: None.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no Audio CD started
//          PRIMOSDK_PROTECTEDWMA if one of the files in the playlist is protected.
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//


DWORD WINAPI PrimoSDK_NewAudioPlaylist(DWORD dwHandle, PDWORD pdwUnits);
//          ===================
//
// Start a new Audio CD Playlist.  Use only for DRM Protected Playlist audio operations.
// Use either this or PrimoSDK_NewAudio, but not both.  Normally clients will use PrimoSDK_NewAudio.
//
// The programmer must call this function, add an audio file for each track using
// PrimoSDK_AddAudioTrack, then write (or test) the audio disc with PrimoSDK_WriteAudio.
// A call to PrimoSDK_CloseAudio will destroy the structure.
//
//   Param: dwHandle is the operation handle
//
//          pdwUnits points to a vector of DWORD containing the units identification,
//                (see PrimoSDK_UnitInfo for field format) terminated by 0xFFFFFFFF
//
//   Notes: Commercial implementations of MPEG-1 and MPEG-2 decoders are subject
//          to royalty fees to patent holders. Many of these patents are general
//          enough such that they are unavoidable regardless of the implementation
//          design, and some may also apply to private implementations.
//          Therefore, PrimoSDK supports only Wave files in its current status.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized
//          PRIMOSDK_BADUNIT if the unit does not exist
//          PRIMOSDK_NOTREADY if the unit is not ready
//          PRIMOSDK_INVALIDMEDIUM if the target disc are not blank CD-R or CD-RW
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//


DWORD WINAPI PrimoSDK_WriteAudioEx(DWORD dwHandle, DWORD dwFlags, DWORD dwSpeed, PBYTE pMCN);
//          =====================
//
// Writes the Audio CD to one or more recorders.
//
// When all the tracks have been added to the audio disc, this functions
// writes (or test) the Audio CD.
// This function returns immediately and continues asynchronously. The caller
// should use the PrimoSDK_RunningStatus to control the operations.
// User shall call PrimoSDK_CloseAudio when PrimoSDK_RunningStatus reports completed (Successful or otherwise).2
//
//   Param: dwHandle is the operation handle
//
//          dwFlags is (OR them if more than one):
//                PRIMOSDK_WRITE for the real recording or PRIMOSDK_TEST for test
//                PRIMOSDK_BURNPROOF if enable the BURN-Proof support if available
//                PRIMOSDK_CLOSEDISC if the disc must be closed
//
//          dwSpeed defines the speed to use for recording:
//                PRIMOSDK_MAX or n (like 8 for 8x) or PRIMOSDK_BEST if the drive supports AWS
//
//          pMCN defines as the Media Catalog Number (13 bytes will be BCD encoded to be placed into the Q channel)
//
//   Notes: Use PRIMOSDK_CLOSEDISC for normal audio discs.
//
//  Return: PRIMOSDK_OK if no error
//          PRIMOSDK_CMDSEQUENCE if not yet initialized or no Audio CD started
//                or the Audio CD is empty
//          PRIMOSDK_BADPARAM if incorrect parameters
//          PRIMOSDK_NOTREADY if one or more units are not ready
//          PRIMOSDK_ITSADEMO if trying to record more than permitted with a Demo version
//          PRIMOSDK_BADHANDLE if dwHandle is not valid
//          PRIMOSDK_INTERR if an internal error occured
//



DWORD WINAPI PrimoSDK_SetAudioLibraryCallback(DWORD dwFlags, PrimoSDK_CreateWmaReader pCallback);

//          ===================
//
//   Allows the client to set up the required Microsoft WNA DRM libraries.
//   This is called anytime after PrimoSDK_Init and before PrimoSDK_NewAudio.
//   This call is applicable for non-stream audio operations.
//
//   Param: dwFlags: set to 0 - reserved for now.
//
//          Pointer to PrimoSDK CreateWmaReader callback routine.  Below is a sample of the routine.
//
//                     HRESULT PrimoSDKLibSetupCallback(DWORD dwRights, void **ppReader)
//                     {
//                        return WMCreateReader( NULL, dwRights,(IWMReader**) ppReader);
//                     }

//
//  Return: PRIMOSDK_OK if the command completes successfully
//          PRIMOSDK_ERRORLOADING cannot open the library
//          PRIMOSDK_FEATURE_NOT_SUPPORTED if not using the PXSDK Plus package.
//
//  Notes: Sample Call
//         if (PrimoSDK_SetAudioLibraryCallback(0, PrimoSDKLibSetupCallback) == PRIMOSDK_OK) ...



/////////////////////////////////////////////////////////////////////
//                                                                 //
//               END Advanced Audio API section                    //
//            ======================================               //
//                                                                 //
/////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif
