/** Gracenote SDK: SQLite public header file
  * Author:
  *   Copyright (c) 2012 Gracenote, Inc.
  *   
  *   This software may not be used in any way or distributed without
  *   permission. All rights reserved.
  *   
  *   Some code herein may be covered by US and international patents.
*/

#ifndef _GNSDK_SQLITE_H_
/** gnsdk_sqlite.h: primary interface for the SQLite SDK
*/
#define _GNSDK_SQLITE_H_

#ifdef __cplusplus
extern "C"{
#endif

/*
 * gnsdk_sqlite.h:	SQLite implementation of gnsdk_storage_provider_interface_t
 *  
 */

/******************************************************************************
 * Typdefs
 ******************************************************************************/

/******************************************************************************
 * GNSDK SQLite Initialization APIs
 ******************************************************************************/

/** gnsdk_sqlite_initialize
  * Summary:
  *   Initializes the SQLite library.
  * Parameters:
  *   sdkmgr_handle:  [in] Handle from successful gnsdk_manager_initialize
  *                   call
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_sqlite_initialize(
	gnsdk_manager_handle_t	sdkmgr_handle
	);

/** gnsdk_sqlite_shutdown
  * Summary:
  *   Shuts down and releases resources for the SQLite library.
*/
gnsdk_error_t GNSDK_API
gnsdk_sqlite_shutdown(void);

/** gnsdk_sqlite_get_version
  * Summary:
  *   Retrieves the version string of SQLite for GNSDK.
*/
gnsdk_cstr_t GNSDK_API
gnsdk_sqlite_get_version(void);

/** gnsdk_sqlite_get_build_date
  * Summary:
  *   Retrieves the build date string of SQLite for GNSDK.
*/
gnsdk_cstr_t GNSDK_API
gnsdk_sqlite_get_build_date(void);

/** gnsdk_sqlite_get_sqlite_version
  * Summary:
  *   Retrieves version string of SQLite database engine.
*/
gnsdk_cstr_t GNSDK_API
gnsdk_sqlite_get_sqlite_version(void);

/** gnsdk_sqlite_option_set
  * Summary:
  *   Sets an option specific to the SQLite library.
  * Parameters:
  *   option_name:   [in] One of the available <link !!MACROS_sqlite_option_keys, SQLite option keys>
  *   option_value:  [in] Value to set for option name
  *                                                                                                  
*/
gnsdk_error_t GNSDK_API
gnsdk_sqlite_option_set(gnsdk_cstr_t option_name, gnsdk_cstr_t option_value);

/** gnsdk_sqlite_option_get
  * Summary:
  *   Retrieves an option set to the SQLite library.
  * Parameters:
  *   option_name:     [in] One of the available <link !!MACROS_sqlite_option_keys, SQLite option keys>
  *   p_option_value:  [out] Pointer to receive the value set for option name
  *                                                                                                    
*/
gnsdk_error_t GNSDK_API
gnsdk_sqlite_option_get(gnsdk_cstr_t option_name, gnsdk_cstr_t* p_option_value);

/** GNSDK_SQLITE_OPTION_STORAGE_FOLDER
  * Summary:
  *   Sets the folder path where storage file(s) can be created within or
  *   \opened from.                                                      
*/
#define	GNSDK_SQLITE_OPTION_STORAGE_FOLDER		"gnsdk_sqlite_storage_folder"
/** GNSDK_SQLITE_OPTION_CACHE_FILESIZE
  * Summary:
  *   Sets the maximum size the GNSDK cache can grow to; for example <c>“100”</c>
  *   for 100 Kb or <c>“1024”</c> for 1 MB. This limit applies to each cache
  *   that is created.                                                           
*/
#define GNSDK_SQLITE_OPTION_CACHE_FILESIZE		"gnsdk_sqlite_filesize"
/** GNSDK_SQLITE_OPTION_CACHE_MEMSIZE
  * Summary:
  *   Sets the maximum amount of memory SQLite can use to buffer cache data.
*/
#define	GNSDK_SQLITE_OPTION_CACHE_MEMSIZE		"gnsdk_sqlite_memsize"	
/** GNSDK_SQLITE_OPTION_SYNCHRONOUS
  * Summary:
  *   Sets the method that SQLite uses to write to the cache files.
*/
#define	GNSDK_SQLITE_OPTION_SYNCHRONOUS			"gnsdk_sqlite_synchronous"	
/** GNSDK_SQLITE_OPTION_JOURNAL_MODE
  * Summary:
  *   Sets how the SQLite journal file is managed for database transactions.
*/
#define GNSDK_SQLITE_OPTION_JOURNAL_MODE		"gnsdk_sqlite_journalmode"



#ifdef __cplusplus
}
#endif

#endif /* _GNSDK_SQLITE_H_ */

