/*--------------------------------------------------------------------
 *  fullsoft.h
 *
 *  Created: 10/15/97
 *  Author:  Matt Kendall
 *
 *  Copyright (C) 1997-99, Full Circle Software, Inc., All Rights Reserved
 *
 *  Full Circle "Talkback" Application API Definition
 *
 *--------------------------------------------------------------------*/
#if !defined(__FULLSOFT_H)
#define __FULLSOFT_H

/* define NO_FC_API to disable all calls to the Full Circle library */
/* define FC_TRACE to enable the Full Circle TRACE macro */
/* define FC_ASSERT to enable the Full Circle ASSERT macro */
/* define FC_TRACE_PARAM to enable the Full Circle TRACE_PARAM macro */
/* define FC_ASSERT_PARAM to enable the Full Circle ASSERT_PARAM macro */

#if !defined(FCAPI)
#define FCAPI
#endif /* defined FCAPI */

#if !defined(NO_FC_API) && defined(macintosh) && !defined(powerc)
#define NO_FC_API
#endif /* !defined(NO_FC_API) && defined(macintosh) && !defined(powerc) */

typedef const char     *      FC_KEY ;
typedef const char     *      FC_TRIGGER ;
typedef unsigned long         FC_DATE ;
typedef unsigned long         FC_UINT32 ;
typedef       void     *      FC_PVOID ;
typedef const char     *      FC_STRING ;
typedef       void     *      FC_CONTEXT ;
typedef unsigned long         FC_TIMER_ID ;
typedef const char     *      FC_TIMER_NAME ;

#define                       FC_CONTEXT_NONE   ((FC_CONTEXT) -1)
#define                       FC_TIMER_NONE     ((FC_TIMER_ID) 0)

typedef enum {
	FC_DATA_TYPE_BINARY,
	FC_DATA_TYPE_STRING,
	FC_DATA_TYPE_INTEGER,
	FC_DATA_TYPE_DATE,
	FC_DATA_TYPE_COUNTER
} FC_DATA_TYPE ;

typedef enum {
	FC_ERROR_OK = 0,
	FC_ERROR_CANT_INITIALIZE,
	FC_ERROR_NOT_INITIALIZED,
	FC_ERROR_ALREADY_INITIALIZED,
	FC_ERROR_FAILED,
	FC_ERROR_OUT_OF_MEMORY,
	FC_ERROR_INVALID_PARAMETER,
	FC_ERROR_NOT_SUPPORTED
} FC_ERROR ;

/* add bit definition constants for key flags */
#define FC_KEY_OPTION_NON_SHARED_PERSISTENT	0x00010000	/* the key is persistent but not shared */
#define FC_KEY_OPTION_UNIQUE				0x00020000	/* the key values are unique */

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/* define NO_FC_API to disable all calls to the Full Circle library */
	
#if !defined(NO_FC_API)

FC_ERROR FCAPI
FCInitialize( void ) ;

FC_ERROR FCAPI
FCCleanup( void ) ;

FC_ERROR FCAPI
FCCreateKey(
    FC_KEY key,
    FC_DATA_TYPE type,
    FC_UINT32 first_count,
    FC_UINT32 last_count,
    FC_UINT32 max_element_size) ;

FC_ERROR FCAPI
FCSetKeyOptions(
    FC_KEY key,
    FC_UINT32 option_bits) ;

/* depricated API ... use FCCreateCounter or FCCreatePersistentCounter instead */
FC_ERROR FCAPI
FCCreatePersistentKey(
    FC_KEY key,
    FC_DATA_TYPE type,
    FC_UINT32 first_count,
    FC_UINT32 last_count,
    FC_UINT32 max_element_size) ;

FC_ERROR FCAPI
FCCreateCounter(
    FC_KEY key) ;

FC_ERROR FCAPI
FCCreatePersistentCounter(
    FC_KEY key) ;

FC_ERROR FCAPI
FCFlushPersistentCounters( void ) ;

FC_ERROR FCAPI
FCFlushNonSharedPersistentKeys( void ) ;

FC_ERROR FCAPI
FCClearKeys( void ) ;

FC_ERROR FCAPI
FCClearCounters( void ) ;

FC_ERROR FCAPI
FCClearKey( FC_KEY key ) ;

FC_ERROR FCAPI
FCDeleteKey( FC_KEY key ) ;

/* Noop for minidump */
FC_ERROR FCAPI 
FCSetMiniDump(
	FC_UINT32 pcnt,
	FC_UINT32 dmpType);

FC_ERROR FCAPI 
FCRunMemTest(
	FC_UINT32 testnum);

FC_ERROR FCAPI
FCAddDataToKey(
    FC_KEY key,
    FC_PVOID buffer,
    FC_UINT32 data_length) ;

FC_ERROR FCAPI
FCDeleteDataFromKey(
    FC_KEY key,
    FC_PVOID buffer,
    FC_UINT32 data_length,
    FC_UINT32 justMatchFront) ;


FC_ERROR FCAPI
FCAddIntToKey(
    FC_KEY key,
    FC_UINT32 data) ;

FC_ERROR FCAPI
FCDeleteIntFromKey(
    FC_KEY key,
    FC_UINT32 data) ;


FC_ERROR FCAPI
FCAddStringToKey(
    FC_KEY key,
    FC_STRING string) ;

FC_ERROR FCAPI
FCDeleteStringFromKey(
    FC_KEY key,
    FC_STRING string,
	FC_UINT32 justMatchFront) ;


FC_ERROR FCAPI
FCAddDateToKey(
	FC_KEY key,
	FC_DATE date) ;

FC_ERROR FCAPI
FCDeleteDateFromKey(
	FC_KEY key,
	FC_DATE date) ;


FC_ERROR FCAPI
FCSetCounter(
    FC_KEY key,
    FC_UINT32 value) ;

FC_ERROR FCAPI
FCIncrementCounter(
    FC_KEY key,
    FC_UINT32 value) ;

FC_ERROR FCAPI
FCDecrementCounter(
    FC_KEY key,
    FC_UINT32 value) ;

FC_UINT32 FCAPI
FCGetCounter(
    FC_KEY key) ;

FC_ERROR FCAPI
FCRegisterMemory(
    FC_KEY key,
    FC_DATA_TYPE type,
    FC_PVOID buffer,
    FC_UINT32 length,
    FC_UINT32 dereference_count,
    FC_CONTEXT context) ;

FC_ERROR FCAPI
FCUnregisterMemory( FC_CONTEXT context ) ;

FC_ERROR FCAPI
FCTrigger( FC_TRIGGER trigger ) ;

FC_ERROR FCAPI
FCTriggerWithoutUI( FC_TRIGGER trigger ) ;

void FCAPI
FCTrace(FC_STRING fmt, ... ) ;

void FCAPI
FCAssert() ;

void FCAPI
FCTraceParam(
    FC_UINT32 track,
    FC_UINT32 level,
    FC_STRING fmt,
    ... ) ;

void FCAPI
FCAssertParam(
    FC_UINT32 track,
    FC_UINT32 level ) ;

void FCAPI
FCSetDisplay(
	FC_STRING display) ;

FC_ERROR FCAPI
FCCreateSupportIncident( FC_STRING description ) ;

FC_TIMER_ID FCAPI
FCStartTimer( FC_TIMER_NAME name ) ;

FC_ERROR FCAPI
FCHeartbeatTimer( FC_TIMER_ID timer ) ;

FC_ERROR FCAPI
FCEndTimer( FC_TIMER_ID timer ) ;

FC_ERROR FCAPI
FCSetLocale( FC_STRING locale ) ;

/* 
 * Allow caller to retrieve a UUID session identifier string (37 bytes).
 * Caller must first successfully call FCInitialize or FCInitializeWithManifest
 * before calling this function. Talkback will create a key with this string 
 * as part of a successful initialization.
 *
 * Caller should allocate a buffer of size FC_UUID_STRING_LEN, into which 
 * talkback will copy the session id.
 *
 * XXX This has only been implemented for Win32.
 *
 */
#if defined(_WIN32)
#define FC_UUID_STRING_LEN 37
FC_ERROR FCAPI
FCGetSessionUniqueID( char* aSessionID );
#endif /* _WIN32 */

#if defined(_WIN32)
/* This argument is really "EXCEPTION_POINTERS *" but */
/* we do not want to require the users to include "windows.h" in all */
/* files where they do not already do so. */
void FCAPI FCExceptionHandler(
    FC_PVOID pep ) ;

/* The pep argument is really "EXCEPTION_POINTERS *" but */
/* we do not want to require the users to include "windows.h" in all */
/* files where they do not already do so. */
FC_ERROR FCAPI
FCRecordException(
    FC_PVOID pep ) ; /* calls FCExceptionHandler */
#endif /* _WIN32 */

FC_ERROR FCAPI
FCInitializeWithManifest(
    FC_STRING manifest_file_name) ;

/* Add minidump interface on 7/12/04 */
#if defined(_WIN32)
FC_ERROR FCAPI FCSetMiniDump(  FC_UINT32 pcnt, FC_UINT32 dmpType );
#endif /* _WIN32 */

/* Added interface for testing heapcode on 9/12/05 */
#if defined(_WIN32)
FC_ERROR FCAPI FCRunMemTest(  FC_UINT32 testnum );
#endif /* _WIN32 */

#if defined(FC_ASSERT)
#if defined(ASSERT)
#undef ASSERT
#endif /* defined ASSERT */
#define ASSERT(a) { if( !(a) ) FCAssert() ; }
#endif /* FC_ASSERT */

#if defined(FC_TRACE)
#if defined(TRACE)
#undef TRACE
#endif /* defined TRACE */
#define TRACE FCTrace
#endif /* FC_TRACE */

#if defined(FC_ASSERT_PARAM)
#if defined(ASSERT_PARAM)
#undef ASSERT_PARAM
#endif /* defined ASSERT_PARAM */
#define ASSERT_PARAM(a,b,c) { if ( !(c) ) FCAssertParam(a,b) ; }
#endif /* FC_ASSERT_PARAM */

#if defined(FC_TRACE_PARAM)
#if defined(TRACE_PARAM)
#undef TRACE_PARAM
#endif /* defined TRACE_PARAM */
#define TRACE_PARAM FCTraceParam
#endif /* FC_TRACE_PARAM */

#else /* NO_FC_API */

#define FCInitialize()                      FC_ERROR_OK
#define FCCleanup()                         FC_ERROR_OK
#define FCCreateKey(a,b,c,d,e)              FC_ERROR_OK
#define FCSetKeyOptions(a,b)				FC_ERROR_OK
#define FCCreatePersistentKey(a,b,c,d,e)    FC_ERROR_OK
#define FCCreateCounter(a)                  FC_ERROR_OK
#define FCCreatePersistentCounter(a)        FC_ERROR_OK
#define FCFlushPersistentCounters()         FC_ERROR_OK
#define FCFlushNonSharedPersistentKeys()	FC_ERROR_OK
#define FCClearKeys()                       FC_ERROR_OK 
#define FCClearCounters()                   FC_ERROR_OK
#define FCClearKey(a)                       FC_ERROR_OK
#define FCDeleteKey(a)                      FC_ERROR_OK
#define FCAddDataToKey(a,b,c)               FC_ERROR_OK 
#define FCDeleteDataFromKey(a,b,c,d)        FC_ERROR_OK 
#define FCAddIntToKey(a,b)                  FC_ERROR_OK 
#define FCDeleteIntFromKey(a,b)             FC_ERROR_OK 
#define FCAddStringToKey(a,b)               FC_ERROR_OK
#define FCDeleteStringFromKey(a,b,c)        FC_ERROR_OK
#define FCAddDateToKey(a,b)                 FC_ERROR_OK
#define FCDeleteDateFromKey(a,b)            FC_ERROR_OK
#define FCRegisterMemory(a,b,c,d,e,f)       FC_ERROR_OK
#define FCUnregisterMemory(a)               FC_ERROR_OK
#define FCTrigger(a)                        FC_ERROR_OK
#define FCTriggerWithoutUI(a)               FC_ERROR_OK
#define FCSetCounter(a,b)                   FC_ERROR_OK
#define FCIncrementCounter(a,b)             FC_ERROR_OK
#define FCDecrementCounter(a,b)             FC_ERROR_OK
#define FCGetCounter(a)                     0
#define FCSetDisplay(a)                     ((void)0)
#define FCCreateSupportIncident(a)          FC_ERROR_OK
#define FCStartTimer(a)                     0
#define FCHeartbeatTimer(a)                 FC_ERROR_OK
#define FCEndTimer(a)                       FC_ERROR_OK
#define FCSetLocale(a)                      FC_ERROR_OK

#if defined(_WIN32)
#define FC_UUID_STRING_LEN 37
#define FCGetSessionUniqueID(a)             FC_ERROR_OK
#endif /* _WIN32 */

#if defined(_WIN32)
#define FCExceptionHandler(a)               ((void)0)
#define FCRecordException(a)				FC_ERROR_OK
#define FCInitializeWithManifest(a)         FC_ERROR_OK
#endif /* _WIN32 */


#if defined(FC_ASSERT)
#define ASSERT(f)    ((void)0)
#endif /* FC_ASSERT */

#if defined(FC_TRACE)
void FCAPI FCTrace(FC_STRING fmt,...) ;
#define TRACE 1 ? (void)0 : FCTrace
#endif /* FC_TRACE */

#if defined(FC_ASSERT_PARAM)
#define ASSERT_PARAM(a,b,c)    ((void)0)
#endif /* FC_ASSERT_PARAM */

#if defined(FC_TRACE_PARAM)
void FCAPI FCTraceParam(
    FC_UINT32 track,
    FC_UINT32 level,
    FC_STRING fmt,
    ... ) ;

#define TRACE_PARAM 1 ? (void) 0 : FCTraceParam
#endif /* FC_TRACE_PARAM */

#endif /* NO_FC_API */

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* __FULLSOFT_H */
