/*
* Copyright (c) 2011 Gracenote.
*
* This software may not be used in any way or distributed without
* permission. All rights reserved.
*
* Some code herein may be covered by US and international patents.
*/

/*
* gnsdk_error_codes.h - Package code definitions and string access.
*/

/*
 * Note: all values are expressed in hexadecimal form.
 */

#ifndef	_GNSDK_ERROR_CODES_H_
#define _GNSDK_ERROR_CODES_H_

/*
* Dependencies
*/


#ifdef __cplusplus
extern "C"{
#endif


/*
* Constants
*/

/* Package/library identifiers. */
#define GNSDKERR_BASE_PKG_ID					(0x80)
#define GNSDKERR_MAX_PKG_ID						(GNSDKERR_BASE_PKG_ID+0x7D)
#define GNSDKERR_RSV_PKG_IDS					(GNSDKERR_BASE_PKG_ID+0x3f)
#define GNSDKERR_APP_PKG_IDS					(GNSDKERR_BASE_PKG_ID+0x5f)

/* Register your application package ID starting with this value */
#define GNSDKPKG_ID_APP_START					(GNSDKERR_APP_PKG_IDS+0x01)

/* GNSDK defined packages */
#define GNSDKPKG_SDKManager						(GNSDKERR_BASE_PKG_ID+0x00)
#define GNSDKPKG_MusicID						(GNSDKERR_BASE_PKG_ID+0x01)
#define GNSDKPKG_MusicID_File					(GNSDKERR_BASE_PKG_ID+0x02)
#define GNSDKPKG_Link							(GNSDKERR_BASE_PKG_ID+0x04)
#define GNSDKPKG_VideoID						(GNSDKERR_BASE_PKG_ID+0x05)
#define GNSDKPKG_Submit							(GNSDKERR_BASE_PKG_ID+0x06)
#define GNSDKPKG_StreamID						(GNSDKERR_BASE_PKG_ID+0x07)
#define GNSDKPKG_eMMS							(GNSDKERR_BASE_PKG_ID+0x08)
#define GNSDKPKG_Playlist						(GNSDKERR_BASE_PKG_ID+0x09)
#define GNSDKPKG_Helper							(GNSDKERR_BASE_PKG_ID+0x19)
#define GNSDKPKG_SQLite							(GNSDKERR_BASE_PKG_ID+0x20)
#define GNSDKPKG_DSP							(GNSDKERR_BASE_PKG_ID+0x21)
#define GNSDKPKG_MusicID_Match					(GNSDKERR_BASE_PKG_ID+0x22)

/* Error Codes */
#define GNSDKERR_BASE_CODE						(0x0)

/* General Errors */
#define GNSDKERR_NoError						(GNSDKERR_BASE_CODE)
#define GNSDKERR_InvalidArg						(GNSDKERR_BASE_CODE+0x0001)
#define GNSDKERR_NoMemory						(GNSDKERR_BASE_CODE+0x0002)
#define GNSDKERR_NotFound						(GNSDKERR_BASE_CODE+0x0003)
#define GNSDKERR_IOError						(GNSDKERR_BASE_CODE+0x0004)
#define GNSDKERR_ReadOnly						(GNSDKERR_BASE_CODE+0x0005)
#define GNSDKERR_Busy							(GNSDKERR_BASE_CODE+0x0006)
#define GNSDKERR_NotInited						(GNSDKERR_BASE_CODE+0x0007)
#define GNSDKERR_BufferTooSmall					(GNSDKERR_BASE_CODE+0x0008)
#define GNSDKERR_InvalidFormat					(GNSDKERR_BASE_CODE+0x0009)
#define GNSDKERR_InitFailed						(GNSDKERR_BASE_CODE+0x000A)
#define GNSDKERR_UnsupportedFunctionality		(GNSDKERR_BASE_CODE+0x000B)
#define GNSDKERR_InvalidData					(GNSDKERR_BASE_CODE+0x000C)
#define GNSDKERR_InvalidCall					(GNSDKERR_BASE_CODE+0x000D)
#define GNSDKERR_Unexpected						(GNSDKERR_BASE_CODE+0x003C)
#define GNSDKERR_IckyError						(GNSDKERR_BASE_CODE+0x003D)
#define GNSDKERR_Unknown						(GNSDKERR_BASE_CODE+0x003E)
#define GNSDKERR_LibraryNotLoaded				(GNSDKERR_BASE_CODE+0x003F)

/* File System Subsystem */
#define GNSDKERR_FileInvalidAccess				(GNSDKERR_BASE_CODE+0x0040)
#define GNSDKERR_FileNotFound					(GNSDKERR_BASE_CODE+0x0041)
#define GNSDKERR_FileExists						(GNSDKERR_BASE_CODE+0x0042)
#define GNSDKERR_FileNoSpace					(GNSDKERR_BASE_CODE+0x0043)
#define GNSDKERR_FileTooManyOpen				(GNSDKERR_BASE_CODE+0x0044)
#define GNSDKERR_FileInvalidHandle				(GNSDKERR_BASE_CODE+0x0045)
#define GNSDKERR_FileInvalidName				(GNSDKERR_BASE_CODE+0x0046)
#define GNSDKERR_FileInvalidFolder				(GNSDKERR_BASE_CODE+0x0047)
#define GNSDKERR_FileTooLarge					(GNSDKERR_BASE_CODE+0x0048)
#define GNSDKERR_EndOfFile						(GNSDKERR_BASE_CODE+0x0049)

/* Remote Communications Management */
#define GNSDKERR_NoMoreConnections				(GNSDKERR_BASE_CODE+0x0080)
#define GNSDKERR_CommInvalidAddress				(GNSDKERR_BASE_CODE+0x0081)
#define GNSDKERR_CommInvalidHandle				(GNSDKERR_BASE_CODE+0x0082)
#define GNSDKERR_CommHostDown					(GNSDKERR_BASE_CODE+0x0083)
#define GNSDKERR_Timeout						(GNSDKERR_BASE_CODE+0x0084)
#define GNSDKERR_HTTPClientError				(GNSDKERR_BASE_CODE+0x0085)
#define GNSDKERR_HTTPServerError				(GNSDKERR_BASE_CODE+0x0086)
#define GNSDKERR_HTTPCancelled					(GNSDKERR_BASE_CODE+0x0087)
#define GNSDKERR_ConnectionRefused				(GNSDKERR_BASE_CODE+0x0088)
#define GNSDKERR_HTTPInvalidHeaderFormat		(GNSDKERR_BASE_CODE+0x0089)
#define GNSDKERR_HTTPMovedError					(GNSDKERR_BASE_CODE+0x008A)

/* Crytographic Subsystem */
#define GNSDKERR_UnsupportedAlg					(GNSDKERR_BASE_CODE+0x00C0)

#define GNSDKERR_BadXMLFormat					(GNSDKERR_BASE_CODE+0x00C1)
#define GNSDKERR_UnknownVersion					(GNSDKERR_BASE_CODE+0x00C2)
#define GNSDKERR_DataError						(GNSDKERR_BASE_CODE+0x00C3)
#define	GNSDKERR_InvalidTOC						(GNSDKERR_BASE_CODE+0x00C4)

/* Micro XML Package */
#define GNSDKERR_SyntaxError					(GNSDKERR_BASE_CODE+0x0120)
#define GNSDKERR_IllegalCharacter				(GNSDKERR_BASE_CODE+0x0121)
#define GNSDKERR_UnexpectedEndOfInput			(GNSDKERR_BASE_CODE+0x0122)
#define GNSDKERR_NothingAtPath					(GNSDKERR_BASE_CODE+0x0123)
#define GNSDKERR_DisallowedStructure			(GNSDKERR_BASE_CODE+0x0124)

/* Online Protocol */
#define GNSDKERR_WrongServerPublicKey			(GNSDKERR_BASE_CODE+0x0160)
#define GNSDKERR_WrongClientPublicKey			(GNSDKERR_BASE_CODE+0x0161)
#define GNSDKERR_ServerError					(GNSDKERR_BASE_CODE+0x0162)
#define GNSDKERR_ServerEncryptionError			(GNSDKERR_BASE_CODE+0x0163)
#define GNSDKERR_MaxRedirects					(GNSDKERR_BASE_CODE+0x0164)
#define GNSDKERR_InvalidClientID				(GNSDKERR_BASE_CODE+0x0165)
#define GNSDKERR_InvalidUserID					(GNSDKERR_BASE_CODE+0x0166)
#define GNSDKERR_QuotaExceeded					(GNSDKERR_BASE_CODE+0x0167)

#define GNSDKERR_Aborted						(GNSDKERR_BASE_CODE+0x01A0)
#define GNSDKERR_NotReady						(GNSDKERR_BASE_CODE+0x01A1)
#define GNSDKERR_NothingToDo					(GNSDKERR_BASE_CODE+0x01A2)
#define GNSDKERR_InvalidInputObject				(GNSDKERR_BASE_CODE+0x01A3)
#define GNSDKERR_InsufficientInputData			(GNSDKERR_BASE_CODE+0x01A4)

/* GCSP Transport */
#define GNSDKERR_CorruptEncryptData				(GNSDKERR_BASE_CODE+0x0200)
#define GNSDKERR_CorruptCompressData			(GNSDKERR_BASE_CODE+0x0201)
#define GNSDKERR_CommandError					(GNSDKERR_BASE_CODE+0x0202)
#define GNSDKERR_ClientCompression				(GNSDKERR_BASE_CODE+0x0203)
#define GNSDKERR_ClientEncryption				(GNSDKERR_BASE_CODE+0x0204)
#define GNSDKERR_GEMPError						(GNSDKERR_BASE_CODE+0x0205)

#define GNSDKERR_UnknownResponseCode			(GNSDKERR_BASE_CODE+0x0206)
#define GNSDKERR_ClientError					(GNSDKERR_BASE_CODE+0x0207)
#define GNSDKERR_GeneralError					(GNSDKERR_BASE_CODE+0x0208)
#define GNSDKERR_IncorrectUsage					(GNSDKERR_BASE_CODE+0x0209)

/* Compression */
#define GNSDKERR_UnknownCompressionType			(GNSDKERR_BASE_CODE+0x0240)

/* Thread */
#define GNSDKERR_TooMany						(GNSDKERR_BASE_CODE+0x0280)
#define GNSDKERR_Deadlock						(GNSDKERR_BASE_CODE+0x0281)
#define GNSDKERR_NoResources					(GNSDKERR_BASE_CODE+0x0282)
#define GNSDKERR_Interrupted					(GNSDKERR_BASE_CODE+0x0283)
#define GNSDKERR_PermissionError				(GNSDKERR_BASE_CODE+0x0284)
#define GNSDKERR_ThreadTerminated				(GNSDKERR_BASE_CODE+0x0285)

/* MediaVOCS */
#define GNSDKERR_TranscriptionLangNotSet		(GNSDKERR_BASE_CODE+0x02A0)

/* Lists */
#define GNSDKERR_ListInvalidFile				(GNSDKERR_BASE_CODE+0x02D0)
#define GNSDKERR_ListUnavailable				(GNSDKERR_BASE_CODE+0x02D1)
#define GNSDKERR_ListInvalidLevel				(GNSDKERR_BASE_CODE+0x02D2)
#define GNSDKERR_LocaleNotSet				(GNSDKERR_BASE_CODE+0x02D3)

/* Handle Errors */
#define GNSDKERR_HandleObjectInvalid			(GNSDKERR_BASE_CODE+0x0320)	/* handle reference invalid */
#define GNSDKERR_HandleObjectWrongType			(GNSDKERR_BASE_CODE+0x0321)	/* handle passed to incorrect API for object */
#define GNSDKERR_HandleObjectMismatch			(GNSDKERR_BASE_CODE+0x0322)	/* handle can't perform operation with parameter */

/* Submit Errors */
#define GNSDKERR_MissingField					(GNSDKERR_BASE_CODE+0x0340)
#define GNSDKERR_InvalidContents				(GNSDKERR_BASE_CODE+0x0341)
#define GNSDKERR_NotEditable					(GNSDKERR_BASE_CODE+0x0342)

/* DataType Errors */
#define GNSDKERR_DataCorruption					(GNSDKERR_BASE_CODE+0x0360)
#define GNSDKERR_IndexOutOfRange				(GNSDKERR_BASE_CODE+0x0361)
#define GNSDKERR_WrongValueType					(GNSDKERR_BASE_CODE+0x0362)
#define GNSDKERR_BadKeyValue					(GNSDKERR_BASE_CODE+0x0363)
#define GNSDKERR_LateDataUpdate					(GNSDKERR_BASE_CODE+0x0364)
#define GNSDKERR_DoDefaultProcessing			(GNSDKERR_BASE_CODE+0x0365)
#define GNSDKERR_InvalidPath					(GNSDKERR_BASE_CODE+0x0366)
#define GNSDKERR_SetsIncompatible				(GNSDKERR_BASE_CODE+0x0367)

/* String Errors */
#define GNSDKERR_InvalidUTF8					(GNSDKERR_BASE_CODE+0x03A0)
#define GNSDKERR_InvalidUCS2					(GNSDKERR_BASE_CODE+0x03A1)

/* General low level subsystem errors */
#define GNSDKERR_CommunicationsError			(GNSDKERR_BASE_CODE+0x0400)
#define GNSDKERR_QueryError						(GNSDKERR_BASE_CODE+0x0401)
#define GNSDKERR_CDSError						(GNSDKERR_BASE_CODE+0x0402)
#define GNSDKERR_DataEncodeError				(GNSDKERR_BASE_CODE+0x0403)
#define GNSDKERR_CompressionError				(GNSDKERR_BASE_CODE+0x0404)
#define GNSDKERR_EncryptionError				(GNSDKERR_BASE_CODE+0x0405)

/* Crypto errors */
#define GNSDKERR_InvalidKey						(GNSDKERR_BASE_CODE+0x0410)
#define GNSDKERR_InvalidSignature				(GNSDKERR_BASE_CODE+0x0411)

/* License Errors */
#define GNSDKERR_LicenseInvalid					(GNSDKERR_BASE_CODE+0x0420)
#define GNSDKERR_LicenseDisallowed				(GNSDKERR_BASE_CODE+0x0421)
#define GNSDKERR_LicenseExpired					(GNSDKERR_BASE_CODE+0x0422)
#define GNSDKERR_LicenseTrialExpired			(GNSDKERR_BASE_CODE+0x0423)

/* Fingerprint Errors */
#define GNSDKERR_TemporalDiscontinuity			(GNSDKERR_BASE_CODE+0x0440)
#define GNSDKERR_SilentAudio					(GNSDKERR_BASE_CODE+0x0441)

/* Local Storage Errors */
#define GNSDKERR_InvalidSchema					(GNSDKERR_BASE_CODE+0x0500)
#define GNSDKERR_SchemaMismatch					(GNSDKERR_BASE_CODE+0x0501)
#define GNSDKERR_EndOfRecords					(GNSDKERR_BASE_CODE+0x0502)
#define GNSDKERR_DuplicateRecord				(GNSDKERR_BASE_CODE+0x0503)

/* GNServer errors - these may not exist in any SDKs but they are used in the JNIs */
#define GNSDKERR_Ambiguous						(GNSDKERR_BASE_CODE+0x0510)

/* DB errors */
#define GNSDKERR_NestedCall						(GNSDKERR_BASE_CODE+0x0521)
/*#define	GNSDKSERR_PermissionError			(GNSDKERR_BASE_CODE+0x0522)*/
#define GNSDKERR_ReadErr						(GNSDKERR_BASE_CODE+0x0523)
#define GNSDKERR_WriteErr						(GNSDKERR_BASE_CODE+0x0524)
/*#define	GNSDKERR_UnknownVersion				(GNSDKERR_BASE_CODE+0x0525)*/
#define GNSDKERR_InvalidFile					(GNSDKERR_BASE_CODE+0x0526)
#define GNSDKERR_AlreadyAdded					(GNSDKERR_BASE_CODE+0x0527)
#define GNSDKERR_InvalidBlock					(GNSDKERR_BASE_CODE+0x0528)
/*#define	GNSDKERR_Aborted					(GNSDKERR_BASE_CODE+0x0529)*/
#define GNSDKERR_NoSpace						(GNSDKERR_BASE_CODE+0x052A)
#define	GNSDKERR_BadIndex						(GNSDKERR_BASE_CODE+0x052B)
#define GNSDKERR_BadFreeList					(GNSDKERR_BASE_CODE+0x052C)
#define	GNSDKERR_BadRecord						(GNSDKERR_BASE_CODE+0x052D)
/*#define	GNSDKERR_BadBlock					(GNSDKERR_BASE_CODE+0x052E)*/
#define	GNSDKERR_IndexPastEOF					(GNSDKERR_BASE_CODE+0x052F)
#define	GNSDKERR_RecordPastEOF					(GNSDKERR_BASE_CODE+0x0530)
#define	GNSDKERR_IndexInFreeList				(GNSDKERR_BASE_CODE+0x0531)
#define	GNSDKERR_RecordInFreeList				(GNSDKERR_BASE_CODE+0x0532)
#define	GNSDKERR_BlockInFreeList				(GNSDKERR_BASE_CODE+0x0533)
#define GNSDKERR_IncompatibleDBs				(GNSDKERR_BASE_CODE+0x0534)
#define GNSDKERR_KeyNotRead						(GNSDKERR_BASE_CODE+0x0535)
#define GNSDKERR_InvalidByteOrder				(GNSDKERR_BASE_CODE+0x0536)
/*#define GNSDKERR_UnknownFSError				(GNSDKERR_BASE_CODE+0x0537)*/
#define GNSDKERR_InvalidIterator				(GNSDKERR_BASE_CODE+0x0538)
#define GNSDKERR_BadHeader						(GNSDKERR_BASE_CODE+0x0539)
/*#define	GNSDKERR_CriticalSection			(GNSDKERR_BASE_CODE+0x053A)*/
#define	GNSDKERR_ThreadError					(GNSDKERR_BASE_CODE+0x053B)
#define	GNSDKERR_ReadLocked						(GNSDKERR_BASE_CODE+0x053C)
#define	GNSDKERR_WriteLocked					(GNSDKERR_BASE_CODE+0x053D)

/* Playlist Errors */
#define GNSDKERR_SeedRequired					(GNSDKERR_BASE_CODE+0x0600)
#define GNSDKERR_StatementError					(GNSDKERR_BASE_CODE+0x0601)

/***************/
/* ERROR VALUES*/
/***************/

/* Errors returned from the SDK Manager */
#define SDKMGRERR_NoError					GNSDKERR_NoError
#define SDKMGRERR_InvalidArg				GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_InvalidArg)
#define SDKMGRERR_InitFailed				GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_InitFailed)
#define SDKMGRERR_LicenseInvalid			GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_LicenseInvalid)
#define SDKMGRERR_LicenseDisallowed			GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_LicenseDisallowed)
#define SDKMGRERR_LicenseExpired			GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_LicenseExpired)
#define SDKMGRERR_LicenseTrialExpired		GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_LicenseTrialExpired)
#define SDKMGRERR_NotInited					GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_NotInited)
#define SDKMGRERR_NotFound					GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_NotFound)
#define SDKMGRWARN_NotFound					GNSDKERR_MAKE_WARNING(GNSDKPKG_SDKManager, GNSDKERR_NotFound)
#define SDKMGRERR_NoMemory					GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_NoMemory)
#define SDKMGRERR_Unsupported				GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_UnsupportedFunctionality)
#define SDKMGRWARN_Unsupported				GNSDKERR_MAKE_WARNING(GNSDKPKG_SDKManager, GNSDKERR_UnsupportedFunctionality)
#define SDKMGRERR_InvalidFormat				GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_InvalidFormat)
#define SDKMGRERR_HandleObjectInvalid		GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_HandleObjectInvalid)
#define SDKMGRERR_HandleObjectWrongType		GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_HandleObjectWrongType)
#define SDKMGRERR_HandleObjectMismatch		GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_HandleObjectMismatch)
#define SDKMGRERR_QueryError				GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_QueryError)
#define SDKMGRERR_CommunicationsError		GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_CommunicationsError)
#define SDKMGRERR_CDSError					GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_CDSError)
#define SDKMGRERR_DataEncodeError			GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_DataEncodeError)
#define SDKMGRERR_CompressionError			GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_CompressionError)
#define SDKMGRERR_EncryptionError			GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_EncryptionError)
#define SDKMGRERR_IncorrectUsage			GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_IncorrectUsage)
#define SDKMGRERR_IckyError					GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_IckyError)
#define SDKMGRERR_InvalidClientID			GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_InvalidClientID)
#define SDKMGRERR_InvalidUserID				GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_InvalidUserID)
#define SDKMGRERR_QuotaExceeded				GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_QuotaExceeded)
#define SDKMGRERR_InvalidSchema				GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_InvalidSchema)
#define SDKMGRERR_SchemaMismatch			GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_SchemaMismatch)
#define SDKMGRWARN_ListUnavailable			GNSDKERR_MAKE_WARNING(GNSDKPKG_SDKManager, GNSDKERR_ListUnavailable)
#define SDKMGRERR_ListUnavailable			GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_ListUnavailable)
#define SDKMGRERR_ServerError				GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_ServerError)
#define SDKMGRWARN_LocaleNotSet			GNSDKERR_MAKE_WARNING(GNSDKPKG_SDKManager, GNSDKERR_LocaleNotSet)
#define SDKMGRERR_LocaleNotSet			GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_LocaleNotSet)
#define SDKMGRWARN_TranscriptionLangNotSet	GNSDKERR_MAKE_WARNING(GNSDKPKG_SDKManager, GNSDKERR_TranscriptionLangNotSet)
#define SDKMGRERR_TranscriptionLangNotSet	GNSDKERR_MAKE_ERROR(GNSDKPKG_SDKManager, GNSDKERR_TranscriptionLangNotSet)

/* Errors returned from the MusicID-File SDK */
#define MIDFERR_NoError						GNSDKERR_NoError
#define MIDFERR_InvalidArg					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_InvalidArg)
#define MIDFERR_InitFailed					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_InitFailed)
#define MIDFERR_Unsupported					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_UnsupportedFunctionality)
#define MIDFERR_NoMemory					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_NoMemory)
#define MIDFERR_NotReady					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_NotReady)
#define MIDFERR_NotInited					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_NotInited)
#define MIDFERR_NotFound					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_NotFound)
#define MIDFWARN_NotFound					GNSDKERR_MAKE_WARNING(GNSDKPKG_MusicID_File, GNSDKERR_NotFound)
#define MIDFERR_HandleObjectInvalid			GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_HandleObjectInvalid)
#define MIDFERR_HandleObjectWrongType		GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_HandleObjectWrongType)
#define MIDFERR_UnsupportedAlg				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_UnsupportedAlg)
#define MIDFERR_StillProcessing				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_Busy)
#define MIDFERR_NotProcessing				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_Unexpected)
#define MIDFWARN_NotProcessing				GNSDKERR_MAKE_WARNING(GNSDKPKG_MusicID_File, GNSDKERR_Unexpected)
#define MIDFERR_InvalidCall					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_InvalidCall)
#define MIDFERR_Aborted						GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_Aborted)
#define MIDFWARN_Aborted					GNSDKERR_MAKE_WARNING(GNSDKPKG_MusicID_File, GNSDKERR_Aborted)
#define MIDFERR_Timeout						GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_Timeout)
#define MIDFWARN_Timeout					GNSDKERR_MAKE_WARNING(GNSDKPKG_MusicID_File, GNSDKERR_Timeout)
#define MIDFERR_IdentExists					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_FileExists)
#define MIDFERR_InvalidFormat				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_InvalidFormat)
#define MIDFERR_IncompatibleDBs				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_IncompatibleDBs)
#define MIDFERR_FileNotFound   				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_FileNotFound)
#define MIDFERR_NothingToDo					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_NothingToDo)
#define MIDFERR_LicenseDisallowed			GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_LicenseDisallowed)
#define MIDFERR_LicenseExpired				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_LicenseExpired)
#define MIDFERR_LicenseTrialExpired			GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_LicenseTrialExpired)
#define MIDFERR_InvalidClientID				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_InvalidClientID)
#define MIDFERR_InvalidUserID				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_InvalidUserID)
#define MIDFERR_QuotaExceeded				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_QuotaExceeded)
#define MIDFERR_QueryError					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_QueryError)
#define MIDFERR_CDSError					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_CDSError)
#define MIDFERR_CommunicationsError			GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_CommunicationsError)
#define MIDFERR_DataEncodeError				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_DataEncodeError)
#define MIDFERR_CompressionError			GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_CompressionError)
#define MIDFERR_EncryptionError				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_EncryptionError)
#define MIDFERR_ThreadTerminated			GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_ThreadTerminated)
#define MIDFWARN_ThreadTerminated			GNSDKERR_MAKE_WARNING(GNSDKPKG_MusicID_File, GNSDKERR_ThreadTerminated)
#define MIDFERR_LibraryNotLoaded			GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_LibraryNotLoaded)
#define MIDFERR_LocaleNotSet				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_LocaleNotSet)
#define MIDFERR_InvalidUTF8					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_File, GNSDKERR_InvalidUTF8)

/* Errors returned from the MusicID SDK */
#define MIDERR_NoError						GNSDKERR_NoError
#define MIDERR_NotInited					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_NotInited)
#define MIDERR_InvalidArg					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_InvalidArg)
#define MIDERR_InitFailed					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_InitFailed)
#define MIDERR_NoMemory						GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_NoMemory)
#define MIDERR_Unsupported					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_UnsupportedFunctionality)
#define MIDERR_HandleObjectInvalid			GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_HandleObjectInvalid)
#define MIDERR_HandleObjectWrongType		GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_HandleObjectWrongType)
#define MIDERR_QueryError					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_QueryError)
#define MIDERR_CDSError						GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_CDSError)
#define MIDERR_CommunicationsError			GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_CommunicationsError)
#define MIDERR_NotFound						GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_NotFound)
#define MIDWARN_NotFound					GNSDKERR_MAKE_WARNING(GNSDKPKG_MusicID, GNSDKERR_NotFound)
#define MIDERR_OutOfRange					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_IndexOutOfRange)
#define MIDERR_InvalidInputObject			GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_InvalidInputObject)
#define MIDERR_BadXMLFormat					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_BadXMLFormat)
#define MIDERR_UnknownResponseCode			GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_UnknownResponseCode)
#define MIDERR_InvalidTOC					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_InvalidTOC)
#define MIDERR_DataEncodeError				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_DataEncodeError)
#define MIDERR_CompressionError				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_CompressionError)
#define MIDERR_EncryptionError				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_EncryptionError)
#define MIDERR_InsufficientInputData		GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_InsufficientInputData)
#define MIDERR_LicenseDisallowed			GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_LicenseDisallowed)
#define MIDERR_LicenseExpired				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_LicenseExpired)
#define MIDERR_LicenseTrialExpired			GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_LicenseTrialExpired)
#define MIDERR_Busy							GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_Busy)
#define MIDERR_InvalidClientID				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_InvalidClientID)
#define MIDERR_InvalidUserID				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_InvalidUserID)
#define MIDERR_QuotaExceeded				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_QuotaExceeded)
#define MIDERR_LibraryNotLoaded				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_LibraryNotLoaded)
#define MIDERR_LocaleNotSet				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_LocaleNotSet)
#define MIDERR_InvalidUTF8					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_InvalidUTF8)
#define MIDERR_InvalidData					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID, GNSDKERR_InvalidData)

/* Errors returned from the Link SDK */
#define LINKERR_NoError						GNSDKERR_NoError
#define LINKERR_NotInited					GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_NotInited)
#define LINKERR_InvalidArg					GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_InvalidArg)
#define LINKERR_InitFailed					GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_InitFailed)
#define LINKERR_NoMemory					GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_NoMemory)
#define LINKERR_Unsupported					GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_UnsupportedFunctionality)
#define LINKERR_HandleObjectInvalid			GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_HandleObjectInvalid)
#define LINKERR_QueryError					GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_QueryError)
#define LINKERR_CDSError					GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_CDSError)
#define LINKERR_CommunicationsError			GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_CommunicationsError)
#define LINKERR_IckyError					GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_IckyError)
#define LINKWARN_NotFound					GNSDKERR_MAKE_WARNING(GNSDKPKG_Link, GNSDKERR_NotFound)
#define LINKERR_NotFound					GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_NotFound)
#define LINKERR_HandleObjectWrongType		GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_HandleObjectWrongType)
#define LINKERR_InvalidInputObject			GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_InvalidInputObject)
#define LINKERR_InsufficientInputData		GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_InsufficientInputData)
#define LINKERR_IncorrectUsage				GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_IncorrectUsage)
#define LINKERR_DataEncodeError				GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_DataEncodeError)
#define LINKERR_CompressionError			GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_CompressionError)
#define LINKERR_EncryptionError				GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_EncryptionError)
#define LINKERR_LicenseDisallowed			GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_LicenseDisallowed)
#define LINKERR_LicenseExpired				GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_LicenseExpired)
#define LINKERR_LicenseTrialExpired			GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_LicenseTrialExpired)
#define LINKERR_Busy						GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_Busy)
#define LINKERR_InvalidClientID				GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_InvalidClientID)
#define LINKERR_InvalidUserID				GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_InvalidUserID)
#define LINKERR_QuotaExceeded				GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_QuotaExceeded)
#define LINKERR_LocaleNotSet				GNSDKERR_MAKE_ERROR(GNSDKPKG_Link, GNSDKERR_LocaleNotSet)

/* Errors returned from the VideoID SDK */
#define VIDERR_NoError						GNSDKERR_NoError
#define VIDERR_NotInited					GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_NotInited)
#define VIDERR_InvalidArg					GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_InvalidArg)
#define VIDERR_InvalidData					GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_InvalidData)
#define VIDERR_InitFailed					GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_InitFailed)
#define VIDERR_NoMemory						GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_NoMemory)
#define VIDERR_HandleObjectInvalid			GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_HandleObjectInvalid)
#define VIDERR_HandleObjectWrongType		GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_HandleObjectWrongType)
#define VIDERR_QueryError					GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_QueryError)
#define VIDERR_CDSError						GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_CDSError)
#define VIDERR_CommunicationsError			GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_CommunicationsError)
#define VIDERR_NotFound						GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_NotFound)
#define VIDWARN_NotFound					GNSDKERR_MAKE_WARNING(GNSDKPKG_VideoID, GNSDKERR_NotFound)
#define VIDERR_InvalidInputObject			GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_InvalidInputObject)
#define VIDERR_BadXMLFormat					GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_BadXMLFormat)
#define VIDERR_UnknownResponseCode			GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_UnknownResponseCode)
#define VIDERR_InvalidTOC					GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_InvalidTOC)
#define VIDERR_DataEncodeError				GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_DataEncodeError)
#define VIDERR_CompressionError				GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_CompressionError)
#define VIDERR_EncryptionError				GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_EncryptionError)
#define VIDERR_InsufficientInputData		GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_InsufficientInputData)
#define VIDERR_ClientError					GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_ClientError)
#define VIDERR_ServerError					GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_ServerError)
#define VIDERR_IckyError					GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_IckyError)
#define VIDERR_Unsupported					GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_UnsupportedFunctionality)
#define VIDERR_OutOfRange					GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_IndexOutOfRange)
#define VIDERR_LicenseDisallowed			GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_LicenseDisallowed)
#define VIDERR_LicenseExpired				GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_LicenseExpired)
#define VIDERR_LicenseTrialExpired			GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_LicenseTrialExpired)
#define VIDERR_Busy							GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_Busy)
#define VIDERR_InvalidClientID				GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_InvalidClientID)
#define VIDERR_InvalidUserID				GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_InvalidUserID)
#define VIDERR_InvalidTOCVerion				GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_UnknownVersion)
#define VIDERR_QuotaExceeded				GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_QuotaExceeded)
#define VIDERR_LocaleNotSet				GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_LocaleNotSet)
#define VIDERR_InvalidUTF8					GNSDKERR_MAKE_ERROR(GNSDKPKG_VideoID, GNSDKERR_InvalidUTF8)

/* Errors returned from the Submit SDK */
#define SUBMITERR_NoError					GNSDKERR_NoError
#define SUBMITERR_NotInited					GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_NotInited)
#define SUBMITERR_InvalidArg				GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_InvalidArg)
#define SUBMITERR_InitFailed				GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_InitFailed)
#define SUBMITERR_NoMemory					GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_NoMemory)
#define SUBMITERR_Unsupported				GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_UnsupportedFunctionality)
#define SUBMITERR_HandleObjectInvalid		GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_HandleObjectInvalid)
#define SUBMITERR_QueryError				GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_QueryError)
#define SUBMITERR_CommunicationsError		GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_CommunicationsError)
#define SUBMITERR_ClientError				GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_ClientError)
#define SUBMITERR_ServerError				GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_ServerError)
#define SUBMITERR_IckyError					GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_IckyError)
#define SUBMITERR_NotFound					GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_NotFound)
#define SUBMITWARN_NotFound					GNSDKERR_MAKE_WARNING(GNSDKPKG_Submit, GNSDKERR_NotFound)
#define SUBMITERR_HandleObjectWrongType		GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_HandleObjectWrongType)
#define SUBMITERR_InvalidInputObject		GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_InvalidInputObject)
#define SUBMITERR_InsufficientInputData		GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_InsufficientInputData)
#define SUBMITERR_IncorrectUsage			GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_IncorrectUsage)
#define SUBMITERR_DataEncodeError			GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_DataEncodeError)
#define SUBMITERR_CompressionError			GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_CompressionError)
#define SUBMITERR_EncryptionError			GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_EncryptionError)
#define SUBMITERR_LicenseDisallowed			GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_LicenseDisallowed)
#define SUBMITERR_LicenseExpired			GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_LicenseExpired)
#define SUBMITERR_LicenseTrialExpired		GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_LicenseTrialExpired)
#define SUBMITERR_Busy						GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_Busy)
#define SUBMITERR_InvalidClientID			GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_InvalidClientID)
#define SUBMITERR_InvalidUserID				GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_InvalidUserID)
#define SUBMITERR_QuotaExceeded				GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_QuotaExceeded)
#define SUBMITERR_MissingField				GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_MissingField)
#define SUBMITERR_InvalidContents			GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_InvalidContents)
#define SUBMITWARN_NotEditable				GNSDKERR_MAKE_WARNING(GNSDKPKG_Submit, GNSDKERR_NotEditable)
#define SUBMITERR_NotEditable				GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_NotEditable)
#define SUBMITERR_InvalidTOC				GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_InvalidTOC)
#define SUBMITERR_OutOfRange				GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_IndexOutOfRange)
#define SUBMITERR_LibraryNotLoaded			GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_LibraryNotLoaded)
#define SUBMITERR_TooMany					GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_TooMany)
#define SUBMITERR_Insufficient				GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_InsufficientInputData)
#define SUBMITERR_Aborted					GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_Aborted)
#define SUBMITERR_Timeout					GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_Timeout)
#define SUBMITERR_SilentAudio				GNSDKERR_MAKE_ERROR(GNSDKPKG_Submit, GNSDKERR_SilentAudio)

/* Errors returned from StreamID SDK */
#define STREAMIDERR_NoError					GNSDKERR_NoError
#define STREAMIDERR_NoMemory				GNSDKERR_MAKE_ERROR(GNSDKPKG_StreamID, GNSDKERR_NoMemory)
#define STREAMIDERR_InvalidArg				GNSDKERR_MAKE_ERROR(GNSDKPKG_StreamID, GNSDKERR_InvalidArg)
#define STREAMIDERR_InitFailed				GNSDKERR_MAKE_ERROR(GNSDKPKG_StreamID, GNSDKERR_InitFailed)
#define STREAMIDERR_NotInited				GNSDKERR_MAKE_ERROR(GNSDKPKG_StreamID, GNSDKERR_NotInited)
#define STREAMIDERR_NotFound				GNSDKERR_MAKE_ERROR(GNSDKPKG_StreamID, GNSDKERR_NotFound)
#define STREAMIDWARN_NotFound				GNSDKERR_MAKE_WARNING(GNSDKPKG_StreamID, GNSDKERR_NotFound)
#define STREAMIDERR_HandleObjectWrongType	GNSDKERR_MAKE_ERROR(GNSDKPKG_StreamID, GNSDKERR_HandleObjectWrongType)
#define STREAMIDERR_Unsupported				GNSDKERR_MAKE_ERROR(GNSDKPKG_StreamID, GNSDKERR_UnsupportedFunctionality)
#define STREAMIDERR_HandleObjectInvalid		GNSDKERR_MAKE_ERROR(GNSDKPKG_StreamID, GNSDKERR_HandleObjectInvalid)
#define STREAMIDERR_OutOfRange				GNSDKERR_MAKE_ERROR(GNSDKPKG_StreamID, GNSDKERR_IndexOutOfRange)
#define STREAMIDERR_Busy					GNSDKERR_MAKE_ERROR(GNSDKPKG_StreamID, GNSDKERR_Busy)
#define STREAMIDERR_Timeout					GNSDKERR_MAKE_ERROR(GNSDKPKG_StreamID, GNSDKERR_Timeout)
#define STREAMIDERR_LicenseDisallowed		GNSDKERR_MAKE_ERROR(GNSDKPKG_StreamID, GNSDKERR_LicenseDisallowed)
#define STREAMIDERR_LicenseExpired			GNSDKERR_MAKE_ERROR(GNSDKPKG_StreamID, GNSDKERR_LicenseExpired)
#define STREAMIDERR_LicenseTrialExpired		GNSDKERR_MAKE_ERROR(GNSDKPKG_StreamID, GNSDKERR_LicenseTrialExpired)
#define STREAMIDWARN_TemporalDiscontinuity	GNSDKERR_MAKE_WARNING(GNSDKPKG_StreamID, GNSDKERR_TemporalDiscontinuity)
#define STREAMIDERR_InvalidClientID			GNSDKERR_MAKE_ERROR(GNSDKPKG_StreamID, GNSDKERR_InvalidClientID)
#define STREAMIDERR_InvalidUserID			GNSDKERR_MAKE_ERROR(GNSDKPKG_StreamID, GNSDKERR_InvalidUserID)
#define STREAMIDERR_QuotaExceeded			GNSDKERR_MAKE_ERROR(GNSDKPKG_StreamID, GNSDKERR_QuotaExceeded)
#define STREAMIDERR_LocaleNotSet			GNSDKERR_MAKE_ERROR(GNSDKPKG_StreamID, GNSDKERR_LocaleNotSet)
#define STREAMIDERR_LibraryNotLoaded		GNSDKERR_MAKE_ERROR(GNSDKPKG_StreamID, GNSDKERR_LibraryNotLoaded)
#define STREAMIDERR_IncorrectUsage			GNSDKERR_MAKE_ERROR(GNSDKPKG_StreamID, GNSDKERR_IncorrectUsage)

/* Errors returned from the eMMS SDK */
#define EMMSERR_NoError						GNSDKERR_NoError
#define EMMSERR_NotInited					GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_NotInited)
#define EMMSERR_InvalidArg					GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_InvalidArg)
#define EMMSERR_InitFailed					GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_InitFailed)
#define EMMSERR_InvalidFormat				GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_InvalidFormat)
#define EMMSERR_NoMemory					GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_NoMemory)
#define EMMSERR_Unsupported					GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_UnsupportedFunctionality)
#define EMMSWARN_Unsupported				GNSDKERR_MAKE_WARNING(GNSDKPKG_eMMS, GNSDKERR_UnsupportedFunctionality)
#define EMMSERR_HandleObjectInvalid			GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_HandleObjectInvalid)
#define EMMSWARN_Aborted					GNSDKERR_MAKE_WARNING(GNSDKPKG_eMMS, GNSDKERR_Aborted)
#define EMMSERR_QueryError					GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_QueryError)
#define EMMSERR_CDSError					GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_CDSError)
#define EMMSERR_CommunicationsError			GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_CommunicationsError)
#define EMMSWARN_NotFound					GNSDKERR_MAKE_WARNING(GNSDKPKG_eMMS, GNSDKERR_NotFound)
#define EMMSERR_NotFound					GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_NotFound)
#define EMMSERR_HandleObjectWrongType		GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_HandleObjectWrongType)
#define EMMSERR_InvalidInputObject			GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_InvalidInputObject)
#define EMMSERR_InsufficientInputData		GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_InsufficientInputData)
#define EMMSERR_IncorrectUsage				GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_IncorrectUsage)
#define EMMSERR_DataEncodeError				GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_DataEncodeError)
#define EMMSERR_CompressionError			GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_CompressionError)
#define EMMSERR_EncryptionError				GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_EncryptionError)
#define EMMSERR_LicenseDisallowed			GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_LicenseDisallowed)
#define EMMSERR_LicenseExpired				GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_LicenseExpired)
#define EMMSERR_LicenseTrialExpired			GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_LicenseTrialExpired)
#define EMMSERR_Busy						GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_Busy)
#define EMMSERR_InvalidClientID				GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_InvalidClientID)
#define EMMSERR_InvalidUserID				GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_InvalidUserID)
#define EMMSERR_QuotaExceeded				GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_QuotaExceeded)
#define EMMSERR_UnknownVersion				GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_UnknownVersion)
#define EMMSERR_InvalidData					GNSDKERR_MAKE_ERROR(GNSDKPKG_eMMS, GNSDKERR_InvalidData)

/* Errors returned from SQLite SDK */
#define SQLITEERR_NoError					GNSDKERR_NoError
#define SQLITEERR_NoMemory					GNSDKERR_MAKE_ERROR(GNSDKPKG_SQLite, GNSDKERR_NoMemory)
#define SQLITEERR_InitFailed				GNSDKERR_MAKE_ERROR(GNSDKPKG_SQLite, GNSDKERR_InitFailed)
#define SQLITEERR_NotInited					GNSDKERR_MAKE_ERROR(GNSDKPKG_SQLite, GNSDKERR_NotInited)
#define SQLITEERR_NotFound					GNSDKERR_MAKE_ERROR(GNSDKPKG_SQLite, GNSDKERR_NotFound)
#define SQLITEWARN_NotFound					GNSDKERR_MAKE_WARNING(GNSDKPKG_SQLite, GNSDKERR_NotFound)
#define SQLITEERR_Busy						GNSDKERR_MAKE_ERROR(GNSDKPKG_SQLite, GNSDKERR_Busy)
#define SQLITEERR_HandleObjectInvalid		GNSDKERR_MAKE_ERROR(GNSDKPKG_SQLite, GNSDKERR_HandleObjectInvalid)
#define SQLITEERR_InvalidArg				GNSDKERR_MAKE_ERROR(GNSDKPKG_SQLite, GNSDKERR_InvalidArg)
#define SQLITEERR_InvalidStorageLocation	GNSDKERR_MAKE_ERROR(GNSDKPKG_SQLite, GNSDKERR_FileInvalidFolder)
#define SQLITEERR_InvalidSchema				GNSDKERR_MAKE_ERROR(GNSDKPKG_SQLite, GNSDKERR_InvalidSchema)
#define SQLITEERR_SchemaMismatch			GNSDKERR_MAKE_ERROR(GNSDKPKG_SQLite, GNSDKERR_SchemaMismatch)
#define SQLITEERR_StatementError			GNSDKERR_MAKE_ERROR(GNSDKPKG_SQLite, GNSDKERR_InvalidFormat)
#define SQLITEERR_EndOfRecords				GNSDKERR_MAKE_ERROR(GNSDKPKG_SQLite, GNSDKERR_EndOfRecords)
#define SQLITEWARN_EndOfRecords				GNSDKERR_MAKE_WARNING(GNSDKPKG_SQLite, GNSDKERR_EndOfRecords)
#define SQLITEERR_DuplicateRecord			GNSDKERR_MAKE_ERROR(GNSDKPKG_SQLite, GNSDKERR_DuplicateRecord)
#define SQLITEWARN_DuplicateRecord			GNSDKERR_MAKE_WARNING(GNSDKPKG_SQLite, GNSDKERR_DuplicateRecord)
#define SQLITEERR_StorageFull				GNSDKERR_MAKE_ERROR(GNSDKPKG_SQLite, GNSDKERR_FileTooLarge)
#define SQLITEWARN_StorageFull				GNSDKERR_MAKE_WARNING(GNSDKPKG_SQLite, GNSDKERR_FileTooLarge)
#define SQLITEERR_ReadOnly					GNSDKERR_MAKE_ERROR(GNSDKPKG_SQLite, GNSDKERR_ReadOnly)
#define SQLITEERR_IOError					GNSDKERR_MAKE_ERROR(GNSDKPKG_SQLite, GNSDKERR_IOError)
#define SQLITEERR_Unknown					GNSDKERR_MAKE_ERROR(GNSDKPKG_SQLite, GNSDKERR_Unknown)

/* Errors returned from DSP SDK */
#define DSPERR_NoError						GNSDKERR_NoError
#define DSPERR_NoMemory						GNSDKERR_MAKE_ERROR(GNSDKPKG_DSP, GNSDKERR_NoMemory)
#define DSPERR_InitFailed					GNSDKERR_MAKE_ERROR(GNSDKPKG_DSP, GNSDKERR_InitFailed)
#define DSPERR_NotInited					GNSDKERR_MAKE_ERROR(GNSDKPKG_DSP, GNSDKERR_NotInited)
#define DSPERR_NotFound						GNSDKERR_MAKE_ERROR(GNSDKPKG_DSP, GNSDKERR_NotFound)
#define DSPWARN_NotFound					GNSDKERR_MAKE_WARNING(GNSDKPKG_DSP, GNSDKERR_NotFound)
#define DSPERR_Busy							GNSDKERR_MAKE_ERROR(GNSDKPKG_DSP, GNSDKERR_Busy)
#define DSPERR_HandleObjectInvalid			GNSDKERR_MAKE_ERROR(GNSDKPKG_DSP, GNSDKERR_HandleObjectInvalid)
#define DSPERR_HandleObjectWrongType		GNSDKERR_MAKE_ERROR(GNSDKPKG_DSP, GNSDKERR_HandleObjectWrongType)
#define DSPERR_InvalidArg					GNSDKERR_MAKE_ERROR(GNSDKPKG_DSP, GNSDKERR_InvalidArg)
#define DSPERR_Unknown						GNSDKERR_MAKE_ERROR(GNSDKPKG_DSP, GNSDKERR_Unknown)
#define DSPERR_Unsupported					GNSDKERR_MAKE_ERROR(GNSDKPKG_DSP, GNSDKERR_UnsupportedFunctionality)
#define DSPERR_IckyError					GNSDKERR_MAKE_ERROR(GNSDKPKG_DSP, GNSDKERR_IckyError)
#define DSPWARN_InsufficientInputData		GNSDKERR_MAKE_WARNING(GNSDKPKG_DSP, GNSDKERR_InsufficientInputData)

/* Errors returned from the Playlist SDK */
#define PLERR_NoError						GNSDKERR_NoError
#define PLERR_NotInited						GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_NotInited)
#define PLERR_InvalidArg					GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_InvalidArg)
#define PLERR_InitFailed					GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_InitFailed)
#define PLERR_NoMemory						GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_NoMemory)
#define PLERR_Unsupported					GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_UnsupportedFunctionality)
#define PLERR_HandleObjectInvalid			GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_HandleObjectInvalid)
#define PLERR_HandleObjectWrongType			GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_HandleObjectWrongType)
#define PLERR_QueryError					GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_QueryError)
#define PLERR_CommunicationsError			GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_CommunicationsError)
#define PLERR_NotFound						GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_NotFound)
#define PLWARN_NotFound						GNSDKERR_MAKE_WARNING(GNSDKPKG_Playlist, GNSDKERR_NotFound)
#define PLERR_DataEncodeError				GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_DataEncodeError)
#define PLERR_CompressionError				GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_CompressionError)
#define PLERR_EncryptionError				GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_EncryptionError)
#define PLERR_InsufficientInputData			GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_InsufficientInputData)
#define PLERR_LicenseDisallowed				GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_LicenseDisallowed)
#define PLERR_LicenseExpired				GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_LicenseExpired)
#define PLERR_LicenseTrialExpired			GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_LicenseTrialExpired)
#define PLERR_QuotaExceeded					GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_QuotaExceeded)
#define PLERR_LibraryNotLoaded				GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_LibraryNotLoaded)
#define PLERR_LocaleNotSet					GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_LocaleNotSet)
#define PLERR_InvalidUTF8					GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_InvalidUTF8)
#define PLERR_InvalidData					GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_InvalidData)
#define PLERR_InvalidSchema					GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_InvalidSchema)
#define PLERR_InvalidCall					GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_InvalidCall)
#define PLERR_OutOfRange					GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_IndexOutOfRange)
#define PLERR_Unexpected					GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_Unexpected)
#define PLERR_BufferTooSmall				GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_BufferTooSmall)
#define PLERR_SeedRequired					GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_SeedRequired)
#define PLERR_StatementError				GNSDKERR_MAKE_ERROR(GNSDKPKG_Playlist, GNSDKERR_StatementError)


/* Errors returned from the Helper SDK */
#define HELPERERR_NoError					GNSDKERR_NoError
#define HELPERERR_NotInited					GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_NotInited)
#define HELPERERR_InvalidArg				GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_InvalidArg)
#define HELPERERR_InitFailed				GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_InitFailed)
#define HELPERERR_NoMemory					GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_NoMemory)
#define HELPERERR_Unsupported				GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_UnsupportedFunctionality)
#define HELPERERR_HandleObjectInvalid		GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_HandleObjectInvalid)
#define HELPERERR_HandleObjectWrongType		GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_HandleObjectWrongType)
#define HELPERERR_QueryError				GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_QueryError)
#define HELPERERR_CDSError					GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_CDSError)
#define HELPERERR_CommunicationsError		GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_CommunicationsError)
#define HELPERERR_NotFound					GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_NotFound)
#define HELPERWARN_NotFound					GNSDKERR_MAKE_WARNING(GNSDKPKG_Helper, GNSDKERR_NotFound)
#define HELPERERR_OutOfRange				GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_IndexOutOfRange)
#define HELPERERR_InvalidInputObject		GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_InvalidInputObject)
#define HELPERERR_UnknownResponseCode		GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_UnknownResponseCode)
#define HELPERERR_DataEncodeError			GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_DataEncodeError)
#define HELPERERR_CompressionError			GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_CompressionError)
#define HELPERERR_EncryptionError			GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_EncryptionError)
#define HELPERERR_InsufficientInputData		GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_InsufficientInputData)
#define HELPERERR_LicenseDisallowed			GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_LicenseDisallowed)
#define HELPERERR_LicenseExpired			GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_LicenseExpired)
#define HELPERERR_LicenseTrialExpired		GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_LicenseTrialExpired)
#define HELPERERR_Busy						GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_Busy)
#define HELPERERR_InvalidClientID			GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_InvalidClientID)
#define HELPERERR_InvalidUserID				GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_InvalidUserID)
#define HELPERERR_QuotaExceeded				GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_QuotaExceeded)
#define HELPERERR_LibraryNotLoaded			GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_LibraryNotLoaded)
#define HELPERERR_LocaleNotSet				GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_LocaleNotSet)
#define HELPERERR_InvalidUTF8				GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_InvalidUTF8)
#define HELPERERR_InvalidData				GNSDKERR_MAKE_ERROR(GNSDKPKG_Helper, GNSDKERR_InvalidData)


#define MIDMERR_NoError						GNSDKERR_NoError
#define MIDMERR_NotInited					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_NotInited)
#define MIDMERR_InvalidArg					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_InvalidArg)
#define MIDMERR_InitFailed					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_InitFailed)
#define MIDMERR_NoMemory					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_NoMemory)
#define MIDMERR_Unsupported					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_UnsupportedFunctionality)
#define MIDMERR_HandleObjectInvalid			GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_HandleObjectInvalid)
#define MIDMERR_HandleObjectWrongType		GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_HandleObjectWrongType)
#define MIDMERR_QueryError					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_QueryError)
#define MIDMERR_Aborted						GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_Aborted)
#define MIDMERR_CommunicationsError			GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_CommunicationsError)
#define MIDMERR_NotFound					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_NotFound)
#define MIDMWARN_NotFound					GNSDKERR_MAKE_WARNING(GNSDKPKG_MusicID_Match, GNSDKERR_NotFound)
#define MIDMERR_NotProcessed				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_Unexpected)
#define MIDMERR_Busy						GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_Busy)
#define MIDMERR_InvalidInputObject			GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_InvalidInputObject)
#define MIDMERR_DataEncodeError				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_DataEncodeError)
#define MIDMERR_CompressionError			GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_CompressionError)
#define MIDMERR_EncryptionError				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_EncryptionError)
#define MIDMERR_InsufficientInputData		GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_InsufficientInputData)
#define MIDMERR_LicenseDisallowed			GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_LicenseDisallowed)
#define MIDMERR_LicenseExpired				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_LicenseExpired)
#define MIDMERR_LicenseTrialExpired			GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_LicenseTrialExpired)
#define MIDMERR_InvalidClientID				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_InvalidClientID)
#define MIDMERR_InvalidUserID				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_InvalidUserID)
#define MIDMERR_QuotaExceeded				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_QuotaExceeded)
#define MIDMERR_LibraryNotLoaded			GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_LibraryNotLoaded)
#define MIDMERR_LocaleNotSet				GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_LocaleNotSet)
#define MIDMERR_InvalidUTF8					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_InvalidUTF8)
#define MIDMERR_InvalidData					GNSDKERR_MAKE_ERROR(GNSDKPKG_MusicID_Match, GNSDKERR_InvalidData)

#ifdef __cplusplus
}
#endif

#endif /* _GNSDK_ERROR_CODES_H_ */

