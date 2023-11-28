/** Gracenote SDK: GNSDK Manager Public Header File
  * Author:
  *   Copyright (c) 2012 Gracenote, Inc.
  *   
  *   This software may not be used in any way or distributed without
  *   permission. All rights reserved.
  *   
  *   Some code herein may be covered by USA and international patents.
*/

/** gnsdk_manager.h: Primary interface for the GNSDK Manager.
*/

#ifndef _GNSDK_MANAGER_H_
#define _GNSDK_MANAGER_H_

#include <stdarg.h> /* for va_list */

#ifdef __cplusplus
extern "C"{
#endif


/******************************************************************************
 * Typdefs
 ******************************************************************************/

/** <title gnsdk_manager_handle_t>
  * <toctitle gnsdk_manager_handle_t>
  * 
  * gnsdk_manager_handle_t
  * Summary:
  *   Primary handle to GNSDK Manager. Retrieved from GNSDK Manager's
  *   gnsdk_manager_initialize API. An application must pass this handle to
  *   each Gracenote SDK on initialization.
  *   
  *   This handle does not have to be released.                            
*/
GNSDK_DECLARE_HANDLE( gnsdk_manager_handle_t );

/** <title gnsdk_user_handle_t>
  * <toctitle gnsdk_user_handle_t>
  * 
  * gnsdk_user_handle_t
  * Summary:
  *   Handle to user identification. Created by the
  *   gnsdk_manager_user_register_new or gnsdk_manager_user_deserialize APIs.
  *   An application must create this handle for each unique user that calls
  *   the SDKs.
  *   
  *   This handle must be released by gnsdk_manager_user_release.            
*/
GNSDK_DECLARE_HANDLE( gnsdk_user_handle_t );

/** <title gnsdk_gdo_handle_t>
  * <toctitle gnsdk_gdo_handle_t>
  * 
  * gnsdk_gdo_handle_t
  * Summary:
  *   Gracenote Data Object handle. An application receives this handle from
  *   many GNSDK APIs that return Gracenote data. All Gracenote data is
  *   accessed through the GNSDK Manager <link !!FUNCTIONS_mgr_gdos, GDO APIs>.
  *   
  *   This handle must be released by gnsdk_manager_gdo_release.               
*/
GNSDK_DECLARE_HANDLE( gnsdk_gdo_handle_t );


/** <title gnsdk_locale_handle_t>
  * <toctitle gnsdk_locale_handle_t>
  * <unfinished>
  * 
  * gnsdk_locale_handle_t
  * Summary:
  *   Handle to a locale.           
*/
GNSDK_DECLARE_HANDLE( gnsdk_locale_handle_t );

/******************************************************************************
 * SDK Manager APIs
 ******************************************************************************/

/** gnsdk_manager_initialize
  * Summary:
  *   Initializes the GNSDK Manager and retrieves the GNSDK Manager handle.
  *   You must initialize the GNSDK Manager prior to calling any other GNSDK
  *   library.
  * Parameters:
  *   p_manager_handle:  [out] Pointer to gnsdk_manager_handle_t that receives
  *                      a GNSDK Manager handle
  *   license_data:      [in_opt] Text of a GNSDK license
  *   license_data_len:  [in_opt] Length of text of GNSDK license (in bytes) or
  *                      one of the <link !!MACROS_mgr_initialization_flags, Manager Initialization Flags>.
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_initialize(
	gnsdk_manager_handle_t*		p_manager_handle,
	const gnsdk_char_t*			license_data,
	gnsdk_size_t				license_data_len
	);

/** GNSDK_MANAGER_LICENSEDATA_NULLTERMSTRING
  * Summary:
  *   Indicates the provided license data is a properly null-terminated
  *   string.                                                          
*/
#define GNSDK_MANAGER_LICENSEDATA_NULLTERMSTRING	(gnsdk_size_t)-1
/** GNSDK_MANAGER_LICENSEDATA_FILENAME
  * Summary:
  *   Indicates the provided license data is a relative or absolute path and
  *   filename to the license file.                                         
*/
#define GNSDK_MANAGER_LICENSEDATA_FILENAME			(gnsdk_size_t)-2
/** GNSDK_MANAGER_LICENSEDATA_STDIN
  * Summary:
  *   Indicates license data is provided through standard input (stdin).
*/
#define GNSDK_MANAGER_LICENSEDATA_STDIN				(gnsdk_size_t)-3


/** gnsdk_manager_shutdown
  * Summary:
  *   Shuts down and releases resources used by the GNSDK Manager.
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_shutdown(void);

/** gnsdk_manager_get_version
  * Summary:
  *   Retrieves the GNSDK Manager's version string.
*/
gnsdk_cstr_t GNSDK_API
gnsdk_manager_get_version(void);

/** gnsdk_manager_get_build_date
  * Summary:
  *   Retrieves the GNSDK Manager's build date string.
*/
gnsdk_cstr_t GNSDK_API
gnsdk_manager_get_build_date(void);


/** gnsdk_manager_string_free
  * Summary:
  *   Frees the memory associated with an GNSDK Manager-provided serialized
  *   string or XML string.
  * Parameters:
  *   string:  [in] Pointer to string buffer to free
  *                                                                        
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_string_free(
	gnsdk_str_t				string
	);

/******************************************************************************
 * SDK Manager 'User Handle' APIs
 ******************************************************************************/

/** gnsdk_manager_user_register_new
  * Summary:
  *   Creates a new user and also increments the user's Client ID user count
  *   with the Gracenote Service.
  * Parameters:
  *   client_id:       [in] Client ID that initiates requests with this handle;
  *                    value provided by Gracenote
  *   client_id_tag:   [in] Client ID tag value that matches client ID; value
  *                    provided by Gracenote
  *   client_app_ver:  [in] Client application version; numeric value provided
  *                    by application, and this value is required
  *   p_user_handle:   [out] Pointer to receive new user handle
  *                                                                            
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_user_register_new(
	gnsdk_cstr_t			client_id,
	gnsdk_cstr_t			client_id_tag,
	gnsdk_cstr_t			client_app_ver,
	gnsdk_user_handle_t*	p_user_handle
	);

/** gnsdk_manager_user_deserialize
  * Summary:
  *   Reconstitutes user handle from serialized user handle data.
  * Parameters:
  *   serialized_user:  [in] String of serialized user handle data
  *   p_user_handle:    [out] Pointer to receive user handle
  *                                                               
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_user_deserialize(
	gnsdk_cstr_t				serialized_user,
	gnsdk_user_handle_t*		p_user_handle
	);

/** gnsdk_manager_user_deserialize_legacy
  * Summary:
  *   <label name="conditional"> this function was undoc'd; do we really need
  *   to surface it?</label>
  * Parameters:
  *   legacy_serialized_user:  [in] String of serialized legacy user handle
  *                            data
  *   client_id:               [in] Client ID that initiates requests with this
  *                            handle; value provided by Gracenote
  *   client_id_tag:           [in] Client ID tag value that matches client ID;
  *                            value provided by Gracenote
  *   client_app_ver:          [in] Client application version; numeric value
  *                            provided by application, and this value is
  *                            required
  *   p_user_handle:           [out] Pointer to receive user handle
  *                                                                            
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_user_deserialize_legacy(
	gnsdk_cstr_t				legacy_serialized_user,
	gnsdk_cstr_t				client_id,
	gnsdk_cstr_t				client_id_tag,
	gnsdk_cstr_t				client_app_ver,
	gnsdk_user_handle_t*		p_user_handle
	);

/** gnsdk_manager_user_serialize
  * Summary:
  *   Serializes a user handle into encrypted text that the application can
  *   store locally for future use.
  * Parameters:
  *   user_handle:        [in] User handle to serialize
  *   p_serialized_user:  [out] Pointer to receive serialized user handle
  *                       string
  *                                                                        
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_user_serialize(
	gnsdk_user_handle_t	user_handle,
	gnsdk_str_t*		p_serialized_user
	);

/** gnsdk_manager_user_option_set
  * Summary:
  *   Sets an option for a given user handle.
  * Parameters:
  *   user_handle:   [in] User handle that this option applies to. To apply
  *                  this option globally, set this parameter to GNSDK_NULL
  *   option_name:   [in] A defined <link !!MACROS_mgr_clientopts, User Options>
  *                  name
  *   option_value:  [in] Value to set for the given option
  *                                                                             
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_user_option_set(
	gnsdk_user_handle_t	user_handle,
	gnsdk_cstr_t		option_name,
	gnsdk_cstr_t		option_value
	);

/** gnsdk_manager_user_option_get
  * Summary:
  *   Retrieves an option for a given user handle.
  * Parameters:
  *   user_handle:     [in] User handle to retrieve option from. To retrieve a
  *                    global option, set this parameter to GNSDK_NULL
  *   option_name:     [in] A defined <link !!MACROS_mgr_clientopts, User Options>
  *                    name
  *   p_option_value:  [out] Pointer to value set for the given option
  *                                                                               
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_user_option_get(
	gnsdk_user_handle_t	user_handle,
	gnsdk_cstr_t		option_name,
	gnsdk_cstr_t*		p_option_value
	);


/** GNSDK_USER_OPTION_PROXY_HOST
  * Summary:
  *   Sets host name for proxy to route GNSDK queries through.
*/
#define GNSDK_USER_OPTION_PROXY_HOST			"gnsdk_proxy_host"
/** GNSDK_USER_OPTION_PROXY_USER
  * Summary:
  *   Sets user name for proxy to route GNSDK queries through.
*/
#define GNSDK_USER_OPTION_PROXY_USER			"gnsdk_proxy_username"
/** GNSDK_USER_OPTION_PROXY_PASS
  * Summary:
  *   Sets password for proxy to route GNSDK queries through.
*/
#define GNSDK_USER_OPTION_PROXY_PASS			"gnsdk_proxy_password"
/** GNSDK_USER_OPTION_NETWORK_TIMEOUT
  * Summary:
  *   Sets the network timeout for all GNSDK queries. Option value is in
  *   milliseconds.                                                     
*/
#define GNSDK_USER_OPTION_NETWORK_TIMEOUT		"gnsdk_network_timeout"
/** GNSDK_USER_OPTION_NETWORK_LOADBALANCE
  * Summary:
  *   Enables distributing queries across multiple Gracenote co-location
  *   facilities. When not enabled, queries will generally resolve to a single
  *   co-location.                                                            
*/
#define GNSDK_USER_OPTION_NETWORK_LOADBALANCE	"gnsdk_network_loadbalance"
/** GNSDK_USER_OPTION_LOCATION_ID
  * Summary:
  *   Sets an IP address or country code to represent the location of user
  *   performing requests.                                                  
*/
#define GNSDK_USER_OPTION_LOCATION_ID			"gnsdk_location_id"
/** GNSDK_USER_OPTION_CACHE_EXPIRATION
  * Summary:
  *   Sets the maximum duration an item in the GNSDK query cache is valid for.
  *   This duration is in seconds, and must exceed one day.                   
*/
#define GNSDK_USER_OPTION_CACHE_EXPIRATION	"gnsdk_cache_expiration"

/** gnsdk_manager_user_release
  * Summary:
  *   Releases resources for a user handle.
  * Parameters:
  *   user_handle:  [in] User handle to release
  *                                            
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_user_release(
	gnsdk_user_handle_t	user_handle
	);


/******************************************************************************
 * SDK Manager 'Logging' APIs
 ******************************************************************************/

/** gnsdk_manager_logging_register_package
  * Summary:
  *   Registers a package that writes to the logs.
  * Parameters:
  *   package_id:    [in] ID of package to register
  *   package_name:  [in] Short name or description that appears in the log
  *                  entries for this package.
  *                                                                        
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_logging_register_package(
	gnsdk_uint16_t	package_id,
	gnsdk_cstr_t	package_name
	);

/** gnsdk_manager_logging_enable
  * Summary:
  *   Enables or changes logging settings of GNSDK messages.
  * Parameters:
  *   log_file_path:  [in] Absolute path and file name to log messages to
  *   package_id:     [in] ID of package to filter log messages for this log
  *   filter_mask:    [in] Filter mask for type of message to log
  *   options_mask:   [in] Formatting options for this log
  *   max_size:       [in] Maximum size of log before new log is created. Enter
  *                   a value of zero (0) to always create new log
  *   b_archive:      [in] Specify GNSDK_TRUE for the archive to retain and
  *                   rename old logs, or GNSDK_FALSE to delete old logs
  *                                                                            
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_logging_enable(
	gnsdk_cstr_t	log_file_path,
	gnsdk_uint16_t	package_id,
	gnsdk_uint32_t	filter_mask,
	gnsdk_uint32_t	options_mask,
	gnsdk_uint64_t	max_size,
	gnsdk_bool_t	b_archive
	);

/** gnsdk_manager_logging_callback_fn
  * Summary:
  *   Manually manage Gracenote SDK logging messages.
  * Parameters:
  *   user_data:   [in] Pointer to data passed in to the
  *                gnsdk_manager_logging_enable_callback function through the
  *                callback_userdata parameter. This pointer must be cast from
  *                the gnsdk_void_t type to its original type to be accessed
  *                properly.
  *   package_id:  [in] ID of package sending the log message
  *   mask:        [in] Filter mask of the type of log message
  *   format:      [in] Format string of the log message
  *   argptr:      [in] Set of arguments for the format string
  *                                                                           
*/
typedef gnsdk_void_t
(GNSDK_CALLBACK_API *gnsdk_manager_logging_callback_fn)(
	const gnsdk_void_t*	user_data,
	gnsdk_uint16_t		package_id,
	gnsdk_uint32_t		mask,
	gnsdk_cstr_t		format,
	va_list				argptr
	);

/** gnsdk_manager_logging_enable_callback
  * Summary:
  *   Enables application callback that receives all logging messages.
  * Parameters:
  *   callback:           [in] Callback function for logging message
  *   callback_userdata:  [in_opt] Data that is passed back through calls to
  *                       the callback function
  *   package_id:         [in] ID of package to filter log messages for the
  *                       callback
  *   filter_mask:        [in] Filter mask for type of messages to send to
  *                       callback
  *                                                                         
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_logging_enable_callback(
	gnsdk_manager_logging_callback_fn	callback,
	const gnsdk_void_t*						callback_userdata,
	gnsdk_uint16_t							package_id,
	gnsdk_uint32_t							filter_mask
	);

/** gnsdk_manager_logging_disable
  * Summary:
  *   Disables logging of GNSDK messages.
  * Parameters:
  *   log_file_path:  [in] Absolute path and file name of log to apply this
  *                   disable
  *   package_id:     [in] ID of package to to disable messages for
  *                                                                        
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_logging_disable(
	gnsdk_cstr_t	log_file_path,
	gnsdk_uint16_t	package_id
	);

/** gnsdk_manager_logging_write
  * Summary:
  *   Enables an application to write to a log for its own messages. This API
  *   uses the printf style parameters for the format string.
  * Parameters:
  *   line:        [in_opt] Source line number of this call
  *   filename:    [in_opt] Source file name of this call
  *   package_id:  [in] Package ID of application making call
  *   mask:        [in] Error mask for this logging message
  *   format:      [in] Error message format
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_logging_write(
	gnsdk_int32_t	line,
	gnsdk_cstr_t	filename,
	gnsdk_uint16_t	package_id,
	gnsdk_uint32_t	mask,
	gnsdk_cstr_t	format,
	...
	);

/** gnsdk_manager_logging_vwrite
  * Summary:
  *   Enables an application to write to a log for its own messages. This API
  *   uses the var_args parameter for variables in format string.
  * Parameters:
  *   line:        [in_opt] Source line number of this call
  *   filename:    [in_opt] Source file name of this call
  *   package_id:  [in] Package ID of application making call
  *   mask:        [in] Error mask for this logging message
  *   format:      [in] Error message format
  *   argptr:      [in] Error message format arguments
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_logging_vwrite(
	gnsdk_int32_t	line,
	gnsdk_cstr_t	filename,
	gnsdk_uint16_t	package_id,
	gnsdk_uint32_t	mask,
	gnsdk_cstr_t	format,
	va_list			argptr
	);


/*
 * Logging package IDs
 */

/** GNSDK_LOG_PKG_ALL
  * Summary:
  *   Indicates all package IDs.
*/
#define		GNSDK_LOG_PKG_ALL			0xFF
/** GNSDK_LOG_PKG_GCSL
  * Summary:
  *   Indicates all low level Gracenote Client Standard Library (GCSL) package IDs.
*/
#define		GNSDK_LOG_PKG_GCSL			0x7E
/** GNSDK_LOG_PKG_GNSDK
  * Summary:
  *   Indicates all GNSDK package IDs.
*/
#define		GNSDK_LOG_PKG_GNSDK			0xFE

/*
 * Logging level masks
 */

/** GNSDK_LOG_LEVEL_ERROR
  * Summary:
  *   Log errors         
*/
#define		GNSDK_LOG_LEVEL_ERROR		0x00000001
/** GNSDK_LOG_LEVEL_WARNING
  * Summary:
  *   Log warnings         
*/
#define		GNSDK_LOG_LEVEL_WARNING		0x00000002
/** GNSDK_LOG_LEVEL_INFO
  * Summary:
  *   Log information messages
*/
#define		GNSDK_LOG_LEVEL_INFO		0x00000004
/** GNSDK_LOG_LEVEL_DEBUG
  * Summary:
  *   Log debug messages 
*/
#define		GNSDK_LOG_LEVEL_DEBUG		0x00000008
/** GNSDK_LOG_LEVEL_ALL
  * Summary:
  *   Log all messages 
*/
#define		GNSDK_LOG_LEVEL_ALL			0x0000000F

/*
 * Logging option masks
 */

/** GNSDK_LOG_OPTION_TIMESTAMP
  * Summary:
  *   Includes a log creation timestamp of the format: <c>Wed Jan 30 18:56:37
  *   2008</c>                                                               
*/
#define		GNSDK_LOG_OPTION_TIMESTAMP	0x01000000
/** GNSDK_LOG_OPTION_CATEGORY
  * Summary:
  *   Categorizes the log entries by headings such as ERROR, INFO, and so on.
*/
#define		GNSDK_LOG_OPTION_CATEGORY	0x02000000
/** GNSDK_LOG_OPTION_PACKAGE
  * Summary:
  *   Includes the Package Name, or the Package ID if the name is unavailable.
*/
#define		GNSDK_LOG_OPTION_PACKAGE	0x04000000
/** GNSDK_LOG_OPTION_THREAD
  * Summary:
  *   Includes the Thread ID.
*/
#define		GNSDK_LOG_OPTION_THREAD		0x08000000
/** GNSDK_LOG_OPTION_SOURCEINFO
  * Summary:
  *   Includes the source information in the format: FILE_(_INFO_)
*/
#define		GNSDK_LOG_OPTION_SOURCEINFO	0x10000000
/** GNSDK_LOG_OPTION_NEWLINE
  * Summary:
  *   Includes a trailing newline in the format: rn
*/
#define		GNSDK_LOG_OPTION_NEWLINE	0x20000000
/** GNSDK_LOG_OPTION_NONE
  * Summary:
  *   Indicates to not include any extra information in the log.
*/
#define		GNSDK_LOG_OPTION_NONE		0x00000000
/** GNSDK_LOG_OPTION_ALL
  * Summary:
  *   Includes all log options, except the <i>None</i> option.
*/
#define		GNSDK_LOG_OPTION_ALL		0xFF000000


/******************************************************************************
 * SDK Manager 'Error Info' APIs
 ******************************************************************************/

/** gnsdk_manager_error_string
  * Summary:
  *   Retrieves a string description for a library error code.
  * Parameters:
  *   error_code:  [in] Error code to retrieve string description for
  *                                                                  
*/
gnsdk_cstr_t GNSDK_API
gnsdk_manager_error_string(
	gnsdk_error_t error_code
	);


/** gnsdk_extended_error_info_t
  * Summary:
  *   Extended Error Info structure returned by the
  *   gnsdk_manager_error_extended_info API.
  *   
  *   This structure does not have to be released. 
*/
typedef struct
{
	gnsdk_error_t	error_code;				/** gnsdk_extended_error_info_t@1::error_code
	             	           				  * Summary:
	             	           				  *   Last error code for given handle.      
	             	           				*/
	gnsdk_cstr_t	error_description;		/** gnsdk_extended_error_info_t@1::error_description
	            	                  		  * Summary:
	            	                  		  *   String description for error code.            
	            	                  		*/
	gnsdk_cstr_t	error_extended_info;	/** gnsdk_extended_error_info_t@1::error_extended_info
	            	                    	  * Summary:
	            	                    	  *   Extended error information string.              
	            	                    	*/
} gnsdk_extended_error_info_t;


/** gnsdk_manager_error_extended_info
  * Summary:
  *   Retrieves any available extended error information for the current thread.
  *                                                                         
*/
gnsdk_extended_error_info_t* GNSDK_API
gnsdk_manager_error_extended_info(void);

/*
 * Languages - (based on ISO 639-2 language identifiers - including some locally defined by Gracenote)
 */

/** GNSDK_LANG_ENGLISH
  * Summary:
  *   Language value for English.
*/
#define			GNSDK_LANG_ENGLISH				"eng"
/** GNSDK_LANG_CHINESE_SIMP
  * Summary:
  *   Language value for Simplified Chinese.
*/
#define			GNSDK_LANG_CHINESE_SIMP			"qtb"
/** GNSDK_LANG_CHINESE_TRAD
  * Summary:
  *   Language value for Traditional Chinese.
*/
#define			GNSDK_LANG_CHINESE_TRAD			"qtd"
/** GNSDK_LANG_DUTCH
  * Summary:
  *   Language value for Dutch.
*/
#define			GNSDK_LANG_DUTCH				"dut"
/** GNSDK_LANG_FRENCH
  * Summary:
  *   Language value for French.
*/
#define			GNSDK_LANG_FRENCH				"fre"
/** GNSDK_LANG_GERMAN
  * Summary:
  *   Language value for German.
*/
#define			GNSDK_LANG_GERMAN				"ger"
/** GNSDK_LANG_ITALIAN
  * Summary:
  *   Language value for Italian.
*/
#define			GNSDK_LANG_ITALIAN				"ita"
/** GNSDK_LANG_JAPANESE
  * Summary:
  *   Language value for Japanese.
*/
#define			GNSDK_LANG_JAPANESE				"jpn"
/** GNSDK_LANG_KOREAN
  * Summary:
  *   Language value for Korean.
*/
#define			GNSDK_LANG_KOREAN				"kor"
/** GNSDK_LANG_PORTUGUESE_BRAZIL
  * Summary:
  *   Language value for Brazilian Portuguese.
*/
#define			GNSDK_LANG_PORTUGUESE_BRAZIL	"por"
/** GNSDK_LANG_RUSSIAN
  * Summary:
  *   Language value for Russian.
*/
#define			GNSDK_LANG_RUSSIAN				"rus"
/** GNSDK_LANG_SPANISH
  * Summary:
  *   Language value for Spanish.
*/
#define			GNSDK_LANG_SPANISH				"spa"
/** GNSDK_LANG_SWEDISH
  * Summary:
  *   Language value for Swedish.
*/
#define			GNSDK_LANG_SWEDISH				"swe"
/** GNSDK_LANG_THAI
  * Summary:
  *   Language value for Thai.
*/
#define			GNSDK_LANG_THAI					"tha"
/** GNSDK_LANG_TURKISH
  * Summary:
  *   Language value for Turkish.
*/
#define			GNSDK_LANG_TURKISH				"tur"
/** GNSDK_LANG_POLISH
  * Summary:
  *   Language value for Polish.
*/
#define			GNSDK_LANG_POLISH				"pol"

/** 
  * Summary:
  *   Language value for Farsi.
*/
#define			GNSDK_LANG_FARSI				"per"
/** 
  * Summary:
  *   Language value for Vietnamese.
*/
#define			GNSDK_LANG_VIETNAMESE			"vie"
/** 
  * Summary:
  *   Language value for Hungarian.
*/
#define			GNSDK_LANG_HUNGARIAN			"hun"
/**  
  * Summary:
  *   Language value for Czech.
*/
#define			GNSDK_LANG_CZECH				"cze"
/** 
  * Summary:
  *   Language value for Slovak.
*/
#define			GNSDK_LANG_SLOVAK				"slo"
/** 
  * Summary:
  *   Language value for Romanian.
*/
#define			GNSDK_LANG_ROMANIAN				"rum"
/** 
  * Summary:
  *   Language value for Greek.
*/
#define			GNSDK_LANG_GREEK				"gre"
/** \ \ 
  * Summary:
  *   Language value for Arabic.
*/
#define			GNSDK_LANG_ARABIC				"ara"
/** 
  * Summary:
  *   Language value for Indonesian Bahasa.
*/
#define			GNSDK_LANG_BAHASA_INDONESIA		"ind"
/** 
  * Summary:
  *   Language value for Finnish.
*/
#define			GNSDK_LANG_FINNISH				"fin"
/** 
  * Summary:
  *   Language value for Norwegian.
*/
#define			GNSDK_LANG_NORWEGIAN			"nor"
/** 
  * Summary:
  *   Language value for Croatian.
*/
#define			GNSDK_LANG_CROATIAN				"scr"
/**  
  * Summary:
  *   Language value for Bulgaria.
*/
#define			GNSDK_LANG_BULGARIAN			"bul"
/** 
  * Summary:
  *   Language value for Serbian.
*/
#define			GNSDK_LANG_SERBIAN				"scc"
/** 
  * Summary:
  *   Language value for Danish.
*/
#define			GNSDK_LANG_DANISH				"dan"

/*
 * Regions
 */

/** GNSDK_REGION_DEFAULT
  * Summary:
  *   The default region for a list.
*/
#define			GNSDK_REGION_DEFAULT			GNSDK_NULL
/** <keywords GNSDK default region, locale region/GNSDK default>
  * 
  * GNSDK_REGION_GLOBAL
  * Summary:
  *   The list is not related to any one specific region.
  *   
  *   This key is the GNSDK default locale region.              
*/
#define			GNSDK_REGION_GLOBAL				"gnsdk_region_global"
/** GNSDK_REGION_US
  * Summary:
  *   The list is related to the USA region.
*/
#define			GNSDK_REGION_US					"gnsdk_region_us"
/** GNSDK_REGION_JAPAN
  * Summary:
  *   The list is related to the Japanese region.
*/
#define			GNSDK_REGION_JAPAN				"gnsdk_region_japan"

/** GNSDK_REGION_CHINA
  * Summary:
  *   The list is related to the Chinese region.
*/
#define			GNSDK_REGION_CHINA				"gnsdk_region_china"
/** GNSDK_REGION_TAIWAN
  * Summary:
  *   The list is related to the Taiwanese region.
*/
#define			GNSDK_REGION_TAIWAN				"gnsdk_region_taiwan"
/** GNSDK_REGION_KOREA
  * Summary:
  *   The list is related to the Korean region.
*/
#define			GNSDK_REGION_KOREA				"gnsdk_region_korea"
/** GNSDK_REGION_EUROPE
  * Summary:
  *   The list is related to the European region.
*/
#define			GNSDK_REGION_EUROPE				"gnsdk_region_europe"

/*
 * Descriptors
 */

/** GNSDK_DESCRIPTOR_DEFAULT
  * Summary:
  *   Retrieves default information for the locale.
*/
#define			GNSDK_DESCRIPTOR_DEFAULT		GNSDK_NULL
/** GNSDK_DESCRIPTOR_SIMPLIFIED
  * Summary:
  *   Retrieves simplified information for the locale.
*/
#define			GNSDK_DESCRIPTOR_SIMPLIFIED		"gnsdk_desc_simplified"
/** GNSDK_DESCRIPTOR_DETAILED
  * Summary:
  *   Retrieves detailed information for the locale.
*/
#define			GNSDK_DESCRIPTOR_DETAILED		"gnsdk_desc_detailed"



/******************************************************************************
 * SDK Manager 'List Status' callback definition
 ******************************************************************************/

/** gnsdk_manager_query_status_t
  * Summary:
  *   List callback function status values. An application receives these
  *   values when implementing the gnsdk_manager_query_callback_fn.    
*/
typedef enum
{
	gnsdk_manager_query_status_unknown = 0,			/** gnsdk_manager_query_status_t@1::gnsdk_manager_list_status_unknown
	                                         			  * Summary:
	                                         			  *   Invalid status                                                      
	                                         			*/
	gnsdk_manager_query_status_query_begin,	/** gnsdk_manager_query_status_t@1::gnsdk_manager_list_status_building_list_request
	                                       	  * Summary:
	                                       	  *   Building request to download list from Gracenote                             
	                                       	*/
	gnsdk_manager_query_status_connecting,/** <unfinished>
	                                        * 
	                                        * gnsdk_manager_query_status_t@1::gnsdk_manager_list_status_connecting
	                                        * Summary:
	                                        *   Connecting to the Gracenote Service                               
	                                      */
	
	gnsdk_manager_query_status_sending,				/** gnsdk_manager_query_status_t@1::gnsdk_manager_list_status_sending
	                                     				  * Summary:
	                                     				  *   Sending request                                                
	                                     				*/
	gnsdk_manager_query_status_receiving,				/** gnsdk_manager_query_status_t@1::gnsdk_manager_list_status_receiving
	                                       				  * Summary:
	                                       				  *   Receiving data                                                   
	                                       				*/
	gnsdk_manager_query_status_query_complete /** gnsdk_manager_query_status_t@1::gnsdk_manager_list_status_deleting_list_request
	                                            * Summary:
	                                            *   Done with request                                                            
	                                          */
} gnsdk_manager_query_status_t;

/** gnsdk_manager_query_callback_fn
  * Summary:
  *   Receive status updates as List data is retrieved.
  * Parameters:
  *   user_data:    [in] Pointer to data passed in to various GNSDK libraries'
  *                 functions (see Remarks) through the callback_userdata
  *                 parameter. This pointer must be cast from the gnsdk_void_t
  *                 type to its original type to be accessed properly.
  *   status:       [in] One of gnsdk_manager_query_status_t values
  *   bytes_done:   [in] Current number of bytes transferred. Set to a value
  *                 greater than 0 to indicate progress, or 0 to indicate no
  *                 progress.
  *   bytes_total:  [in] Total number of bytes to be transferred. Set to a
  *                 value greater than 0 to indicate progress, or 0 to indicate
  *                 no progress.
  *   p_abort:      [out] Set dereferenced value to GNSDK_TRUE to abort the
  *                 operation that is calling the callback
  *                                                                            
*/
typedef gnsdk_void_t
(GNSDK_CALLBACK_API *gnsdk_manager_query_callback_fn)(
	const gnsdk_void_t*				user_data,
	gnsdk_manager_query_status_t	status,
	gnsdk_size_t					bytes_done,
	gnsdk_size_t					bytes_total,
	gnsdk_bool_t*					p_abort
	);




/** GNSDK_LOCALE_GROUP_MUSIC
  * Summary:
  *   Locale value for the GNSDK music products. Set this when creating a
  *   locale used with the MusicID and MusicID-File libraries.           
*/
#define GNSDK_LOCALE_GROUP_MUSIC			"gnsdk_locale_music"
/** GNSDK_LOCALE_GROUP_VIDEO
  * Summary:
  *   Locale value for the GNSDK video products. Set this when creating a
  *   locale used with the VideoID or Video Explore libraries (or both). 
*/
#define GNSDK_LOCALE_GROUP_VIDEO			"gnsdk_locale_video"
/** GNSDK_LOCALE_GROUP_PLAYLIST
  * Summary:
  *   Locale value for the GNSDK Playlist product. Set this when creating a
  *   locale used with the Playlist library.                               
*/
#define GNSDK_LOCALE_GROUP_PLAYLIST			"gnsdk_locale_playlist"

/** \ \ 
  * Summary:
  *   Loads a locale object.
  * Parameters:
  *   locale_group:       [in] Locale GNSDK product group defined for locale
  *                       handle
  *   language:           [in] Language defined for locale handle
  *   region:             [in] Region defined for locale handle
  *   descriptor:         [in] Descriptor defined for locale handle
  *   user_handle:        [in] User handle for the user requesting the query
  *   callback_fn:        [in_opt] Callback function for status and progress
  *   callback_userdata:  [in_opt] Data that is passed back through calls to
  *                       the callback function
  *   p_locale_handle:    [out] Pointer to receive locale handle
  *                                                                         
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_locale_load(
	gnsdk_cstr_t					locale_group,
	gnsdk_cstr_t					language,
	gnsdk_cstr_t					region,
	gnsdk_cstr_t					descriptor,
	gnsdk_user_handle_t				user_handle,
	gnsdk_manager_query_callback_fn	callback_fn,
	const gnsdk_void_t*				callback_userdata,
	gnsdk_locale_handle_t*			p_locale_handle
	);

/** \ \ 
  * Summary:
  *   Sets a <link !!MACROS_mgr_locale_groups, global locale group> default
  *   for a locale.
  * Parameters:
  *   locale_handle:  [in] Locale handle to set group default for
  *                                                                        
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_locale_set_group_default(
	gnsdk_locale_handle_t			locale_handle
	);

/** \ \ 
  * Summary:
  *   Releases the default reference to the locale.
  * Parameters:
  *   locale_group:  [in] Locale GNSDK product group to clear from locale handle
  *                                                                             
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_locale_unset_group_default(
	gnsdk_cstr_t					locale_group
	);

/** \ \ 
  * Summary:
  *   Retrieves information about the locale contained in a locale handle.
  * Parameters:
  *   locale_handle:  [in] Locale handle to retrieve information for
  *   p_group:        [out] Pointer to string to receive the locale group
  *   p_language:     [out] Pointer to string to receive the locale language
  *   p_region:       [out] Pointer to string to receive the locale region
  *   p_descriptor:   [out] Pointer to string to receive the locale descriptor
  *                                                                           
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_locale_info(
	gnsdk_locale_handle_t			locale_handle,
	gnsdk_cstr_t*					p_group,
	gnsdk_cstr_t*					p_language,
	gnsdk_cstr_t*					p_region,
	gnsdk_cstr_t*					p_descriptor
	);

/** \ \ 
  * Summary:
  *   Reconstitutes locale from serialized locale data.
  * Parameters:
  *   serialized_locale_data:  [in] String of serialized locale handle data
  *   p_locale_handle:         [out] Pointer to receive handle of deserialized
  *                            locale data
  *                                                                           
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_locale_deserialize(
	gnsdk_cstr_t					serialized_locale_data,
	gnsdk_locale_handle_t*			p_locale_handle
	);

/** \ \ 
  * Summary:
  *   Serializes a locale into encrypted text that the application can store
  *   locally for future use.
  * Parameters:
  *   locale_handle:             [in] Locale handle to serialize
  *   p_serialized_locale_data:  [out] Pointer to string to receive serialized
  *                              locale data
  *                                                                           
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_locale_serialize(
	gnsdk_locale_handle_t			locale_handle,
	gnsdk_str_t*					p_serialized_locale_data
	);

/** \ \ 
  * Summary:
  *   Releases resources for a locale handle.
  * Parameters:
  *   locale_handle:  [in] Locale handle to release
  *                                                
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_locale_release(
	gnsdk_locale_handle_t			locale_handle
	);

/** \ \ 
  * Summary:
  *   Updates a locale.
  * Parameters:
  *   locale_handle:      [in] Locale handle to update
  *   user_handle:        [in] User handle for the user requesting the query
  *   callback_fn:        [in_opt] Callback function for status and progress
  *   callback_userdata:  [in_opt] Data that is passed back through calls to
  *                       the callback function
  *   p_updated:          [out] Pointer to receive boolean value if the locale
  *                       has been updated (GNSDK_TRUE) or not (GNSDK_FALSE)
  *                                                                           
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_locale_update(
	gnsdk_locale_handle_t			locale_handle,
	gnsdk_user_handle_t				user_handle,
	gnsdk_manager_query_callback_fn	callback_fn,
	const gnsdk_void_t*				callback_userdata,
	gnsdk_bool_t*					p_updated
	);

/******************************************************************************
 * SDK Manager local storage management APIs
 ******************************************************************************/

/** gnsdk_manager_storage_cleanup
  * Summary:
  *   Performs maintenance on named local storage.
  * Parameters:
  *   storage_name:  [in] Local storage name
  *   b_async:       [in] Boolean value to indicate whether to perform an
  *                  asynchronous cleanup maintenance in the background
  *                                                                      
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_storage_cleanup(
	gnsdk_cstr_t	storage_name,
	gnsdk_bool_t	b_async
	);

/** gnsdk_manager_storage_flush
  * Summary:
  *   Erases all records from the named local storage.
  * Parameters:
  *   storage_name:  [in] Local storage name
  *   b_async:       [in] Boolean value to indicate whether to perform an
  *                  asynchronous cache flush in the background, on a separate
  *                  thread
  *                                                                           
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_storage_flush(
	gnsdk_cstr_t	storage_name,
	gnsdk_bool_t	b_async
	);

/** GNSDK_MANAGER_STORAGE_QUERYCACHE
  * Summary:
  *   Name of the cache the SDK creates to store queries.
*/
#define GNSDK_MANAGER_STORAGE_QUERYCACHE			"gnsdk_storage_querycache"
/** GNSDK_MANAGER_STORAGE_CONTENTCACHE
  * Summary:
  *   Name of the cache the SDK creates to store content.
*/
#define GNSDK_MANAGER_STORAGE_CONTENTCACHE		"gnsdk_storage_contentcache"


#ifdef __cplusplus
}
#endif

#endif /* _GNSDK_MANAGER_H_ */

