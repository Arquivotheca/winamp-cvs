//-----------------------------------------------------------------------------
// block.h
// Copyright (c) Sonic Solutions.  All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _VX_BLOCK_H_
#define _VX_BLOCK_H_


#ifdef _NTD
	#define OS_CLASS_EXPORT
#elif defined(NTB)
	#define OS_CLASS_EXPORT
#elif defined(WIN32)
	#define OS_CLASS_EXPORT __declspec(dllexport)
#else
	#define OS_CLASS_EXPORT _export
#endif

///////////////////////////////////////////////////////////////////////////////
//
//   function -  LockUnlockCd
//
//   desc     -  Reserve a drive letter as "in use" by your AppName and the
//               current process ID (GetCurrentProcessID). If the calling
//               application is not the locking application, LockUnlockCdEx
//               must be used instead.
//
//   entry    -  Lock     : true = prohibit third party apps.
//               DrvLtr   : letter to lock out other apps from using
//                          Locks all DLA drives because they share the same
//                          driver.
//               AppName  : Your app name.  Query will give "Drive
//                            is locked by (AppName)"
//
//   returns  -  BOOL     : false = could not lock the letter
//
///////////////////////////////////////////////////////////////////////////////
BOOL OS_CLASS_EXPORT LockUnlockCd(BOOL Lock, char DrvLtr, const char *AppName);

///////////////////////////////////////////////////////////////////////////////
//
//   function -  LockUnlockCdEx
//
//   desc     -  Reserve a drive letter as "in use" by a specified AppName and
//               a specified process ID (Pid). This is necessary when the
//               calling application is not the locking application
//               (for example, a COM server that sits between the burn engine
//               and the client application).
//
//   entry    -  Lock     : true = prohibit third party apps.
//               DrvLtr   : letter to lock out other apps from using
//                          Locks all DLA drives because they share the same
//                          driver.
//               AppName  : Your app name.  Query will give "Drive
//                            is locked by (AppName)"
//               Pid      : Process ID of application requesting (un)lock.
//                          Any locks will be released if this Pid is unloaded.
//
//   returns  -  BOOL     : false = could not lock the letter
//
///////////////////////////////////////////////////////////////////////////////
BOOL OS_CLASS_EXPORT LockUnlockCdEx(BOOL Lock, char DrvLtr,
	const char *AppName, DWORD Pid);

///////////////////////////////////////////////////////////////////////////////
//
//   function -  NotifyOfDriverUse
//
//   desc     -  Notify VxBlock a program is using/not using a particular drvr.
//               This need only be used by legacy apps that share DRVMCDB.SYS.
//
//   entry    -  BOOL Lock       : Lock/Unlock the named driver
//               char *DriverName: Driver you are about to lock/release
//               char *AppName   : Caller's application name.
//
//   exit     -  CurrentUser     : if already in use, return FALSE and fill in
//                                 name of program currently using driver
//
//   returns  -  TRUE: Operation completed.  FALSE: driver is not successfully
//                                                  reserved, check CurrentUser
//
///////////////////////////////////////////////////////////////////////////////
BOOL OS_CLASS_EXPORT NotifyOfDriverUse(const char *DriverName, BOOL Lock,
	const char *AppName, char * const CurrentUser);

///////////////////////////////////////////////////////////////////////////////
//
//   function -  QueryCd
//
//   desc     -  Find out if other CD apps are busy with a drive or not using
//               the current process ID (GetCurrentProcessId). If the calling
//               application is not the requesting application, QueryCdEx
//               should be used instead.
//
//   entry    -  DrvLtr    : drive letter to query
//                           queries all DLA drives regardless of letter
//                           because they share the same driver
//               AppName   : If FALSE is returned, AppName contains the name
//                           of the program using the drive.
//
//   returns  -  BOOL      : FALSE = letter is busy.  TRUE = not in use
//
///////////////////////////////////////////////////////////////////////////////
BOOL OS_CLASS_EXPORT QueryCd(char DrvLtr, char * const AppName);

///////////////////////////////////////////////////////////////////////////////
//
//   function -  QueryCdEx
//
//   desc     -  Find out if other CD apps are busy with a drive or not given
//               the process ID (Pid) of the requesting application.
//
//   entry    -  DrvLtr    : drive letter to query
//                           queries all DLA drives regardless of letter
//                           because they share the same driver
//               AppName   : If FALSE is returned, AppName contains the name
//                           of the program using the drive.
//               Pid       : Process ID of application requesting query.
//
//   returns  -  BOOL      : FALSE = letter is busy.  TRUE = not in use
//
///////////////////////////////////////////////////////////////////////////////
BOOL OS_CLASS_EXPORT QueryCdEx(char DrvLtr, char * const AppName, DWORD Pid);

#endif	// _VX_BLOCK_H_
