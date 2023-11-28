/** Gracenote SDK: MusicID-File public header file
  * Author:
  *   Copyright (c) 2012 Gracenote, Inc.
  *   
  *   This software may not be used in any way or distributed without
  *   permission. All rights reserved.
  *   
  *   Some code herein may be covered by US and international patents.
*/

/** gnsdk_musicid_file.h: primary interface for the MusicID-File SDK
*/

#ifndef _GNSDK_MUSICID_FILE_H_
#define _GNSDK_MUSICID_FILE_H_

#ifdef __cplusplus
extern "C"{
#endif

/*
 * gnsdk_musicid_file.h:	Public interface for the MusicID-File SDK.
 */

/******************************************************************************
 * Typdefs
 ******************************************************************************/

/** <title gnsdk_musicidfile_query_handle_t>
  * <toctitle gnsdk_musicidfile_query_handle_t>
  * 
  * gnsdk_musicidfile_query_handle_t
  * Summary:
  *   Primary handle to <link MusicID-File>. The application must create this
  *   handle to manage a file set designated for <link MusicID-File>
  *   recognition.                                                           
*/
GNSDK_DECLARE_HANDLE( gnsdk_musicidfile_query_handle_t );

/** <title gnsdk_musicidfile_fileinfo_handle_t>
  * <toctitle gnsdk_musicidfile_fileinfo_handle_t>
  * 
  * gnsdk_musicidfile_fileinfo_handle_t
  * Summary:
  *   Handle containing all input and output data for a single file. Each
  *   handle is associated with a <link GNSDK_DECLARE_HANDLE@gnsdk_musicidfile_query_handle_t, gnsdk_musicidfile_query_handle_t>.
*/
GNSDK_DECLARE_HANDLE( gnsdk_musicidfile_fileinfo_handle_t );


/******************************************************************************
 * GNSDK_MUSICIDFILE_QUERY_FLAG_* values
 ******************************************************************************/

/** GNSDK_MUSICIDFILE_QUERY_FLAG_RETURN_SINGLE
  * Summary:
  *   \Returns the single best result for each FileInfo. Do not use in
  *   conjunction with GNSDK_MUSICIDFILE_QUERY_FLAG_RETURN_ALL.
*/
#define	GNSDK_MUSICIDFILE_QUERY_FLAG_RETURN_SINGLE			0x00000001
/** GNSDK_MUSICIDFILE_QUERY_FLAG_RETURN_ALL
  * Summary:
  *   \Returns all possible results for each FileInfo. Do not use in
  *   conjunction with GNSDK_MUSICIDFILE_QUERY_FLAG_RETURN_SINGLE.
*/
#define	GNSDK_MUSICIDFILE_QUERY_FLAG_RETURN_ALL				0x00000010
/** GNSDK_MUSICIDFILE_QUERY_FLAG_AGGRESSIVE
  * Summary:
  *   Indicates use of an aggressive search method when attempting to find
  *   matches.
*/
#define	GNSDK_MUSICIDFILE_QUERY_FLAG_AGGRESSIVE				0x00000020
/** GNSDK_MUSICIDFILE_QUERY_FLAG_ASYNC
  * Summary:
  *   Processes <link MusicID-File> on a separate thread and return
  *   immediately.
*/
#define	GNSDK_MUSICIDFILE_QUERY_FLAG_ASYNC					0x00000200
/** GNSDK_MUSICIDFILE_QUERY_FLAG_NO_THREADS
  * Summary:
  *   Disallows <link MusicID-File> from creating threads for internal
  *   background processing.
*/
#define	GNSDK_MUSICIDFILE_QUERY_FLAG_NO_THREADS				0x00001000
/** GNSDK_MUSICIDFILE_QUERY_FLAG_DEFAULT
  * Summary:
  *   Indicates use of default <link MusicID-File> processing options.
*/
#define	GNSDK_MUSICIDFILE_QUERY_FLAG_DEFAULT				(GNSDK_MUSICIDFILE_QUERY_FLAG_RETURN_SINGLE)


/******************************************************************************
 * GNSDK_MUSICIDFILE_OPTION_* values
 ******************************************************************************/

/** GNSDK_MUSICIDFILE_OPTION_THREADPRIORITY
  * Summary:
  *   Sets or retrieves the thread priority level option.
*/
#define	GNSDK_MUSICIDFILE_OPTION_THREADPRIORITY				"gnsdk_midf_option_thread_priority"
/** GNSDK_MUSICIDFILE_OPTION_BATCH_SIZE
  * Summary:
  *   Sets or retrieves the LibraryID batch size option.
*/
#define	GNSDK_MUSICIDFILE_OPTION_BATCH_SIZE					"gnsdk_midf_option_batch_size"

/** GNSDK_MUSICIDFILE_OPTION_ENABLE_CLASSICAL_DATA
  * Summary:
  *   Indicates whether a response should include any associated classical
  *   music data.
*/
#define GNSDK_MUSICIDFILE_OPTION_ENABLE_CLASSICAL_DATA		"gnsdk_midf_option_enable_classical"
/** GNSDK_MUSICIDFILE_OPTION_ENABLE_MEDIAVOCS_DATA
  * Summary:
  *   Indicates whether a response should include any associated MediaVOCS
  *   data.
*/
#define GNSDK_MUSICIDFILE_OPTION_ENABLE_MEDIAVOCS_DATA		"gnsdk_midf_option_enable_mvocs"
/** GNSDK_MUSICIDFILE_OPTION_ENABLE_DSP_DATA
  * Summary:
  *   Indicates whether a response should include any associated sonic
  *   attribute (DSP) data.
*/
#define GNSDK_MUSICIDFILE_OPTION_ENABLE_DSP_DATA			"gnsdk_midf_option_enable_dsp"
/** GNSDK_MUSICIDFILE_OPTION_ENABLE_PLAYLIST
  * Summary:
  *   Indicates whether a response should include associated attribute data
  *   for GNSDK Playlist.                                                  
*/
#define GNSDK_MUSICIDFILE_OPTION_ENABLE_PLAYLIST			"gnsdk_midf_option_enable_playlist"
/** GNSDK_MUSICIDFILE_OPTION_ENABLE_LINK_DATA
  * Summary:
  *   Indicates whether a response should include any Link data (third-party
  *   metadata).
*/
#define GNSDK_MUSICIDFILE_OPTION_ENABLE_LINK_DATA			"gnsdk_midf_option_enable_link"
/** GNSDK_MUSICIDFILE_OPTION_PREFERRED_LANG
  * Summary:
  *   Specifies the preferred language of the results that are returned.
*/
#define GNSDK_MUSICIDFILE_OPTION_PREFERRED_LANG				"gnsdk_midf_preferred_lang"

/******************************************************************************
 * GNSDK_MUSICIDFILE_OPTION_VALUE_* values
 ******************************************************************************/

/** GNSDK_MUSICIDFILE_OPTION_VALUE_PRIORITY_IDLE
  * Summary:
  *   Sets <link MusicID-File> processing threads to idle priority.
*/
#define	GNSDK_MUSICIDFILE_OPTION_VALUE_PRIORITY_IDLE		"-5"
/** GNSDK_MUSICIDFILE_OPTION_VALUE_PRIORITY_LOW
  * Summary:
  *   Sets <link MusicID-File> processing threads to low priority (default).
*/
#define	GNSDK_MUSICIDFILE_OPTION_VALUE_PRIORITY_LOW			"-3"
/** GNSDK_MUSICIDFILE_OPTION_VALUE_PRIORITY_NORM
  * Summary:
  *   Sets <link MusicID-File> processing threads to normal priority.
*/
#define	GNSDK_MUSICIDFILE_OPTION_VALUE_PRIORITY_NORM		"0"
/** GNSDK_MUSICIDFILE_OPTION_VALUE_PRIORITY_HIGH
  * Summary:
  *   Sets <link MusicID-File> processing threads to high priority.
*/
#define	GNSDK_MUSICIDFILE_OPTION_VALUE_PRIORITY_HIGH		"3"
/** GNSDK_MUSICIDFILE_OPTION_VALUE_PRIORITY_DEFAULT
  * Summary:
  *   Indicates use of default <link MusicID-File> processing thread priority.
*/
#define GNSDK_MUSICIDFILE_OPTION_VALUE_PRIORITY_DEFAULT		GNSDK_MUSICIDFILE_OPTION_VALUE_PRIORITY_LOW
/** GNSDK_MUSICIDFILE_OPTION_VALUE_BATCH_SIZE_DEFAULT
  * Summary:
  *   Sets the LibraryID default batch size.
*/
#define	GNSDK_MUSICIDFILE_OPTION_VALUE_BATCH_SIZE_DEFAULT	"150"

/** GNSDK_MUSICIDFILE_OPTION_VALUE_TRUE
  * Summary:
  *   Sets a value representing a boolean TRUE.
*/
#define GNSDK_MUSICIDFILE_OPTION_VALUE_TRUE					"true"
/** GNSDK_MUSICIDFILE_OPTION_VALUE_FALSE
  * Summary:
  *   Sets a value representing a boolean FALSE.
*/
#define GNSDK_MUSICIDFILE_OPTION_VALUE_FALSE				"false"

/******************************************************************************
 * GNSDK_MUSICIDFILE_FILEINFO_VALUE_* keys
 * These keys are used to get/set data from/to a gnsdk_musicidfile_fileinfo_handle_t
 ******************************************************************************/

/** GNSDK_MUSICIDFILE_FILEINFO_VALUE_IDENT
  * Summary:
  *   Retrieves an identifier string from a FileInfo object.
*/
#define GNSDK_MUSICIDFILE_FILEINFO_VALUE_IDENT					"gnsdk_midf_fileinfo_value_ident"
/** GNSDK_MUSICIDFILE_FILEINFO_VALUE_FILENAME
  * Summary:
  *   Sets or retrieves the path and file name for a FileInfo object.
*/
#define GNSDK_MUSICIDFILE_FILEINFO_VALUE_FILENAME				"gnsdk_midf_fileinfo_value_filename"
/** GNSDK_MUSICIDFILE_FILEINFO_VALUE_TAGID
  * Summary:
  *   Sets or retrieves a Gracenote TagID value for a FileInfo object.
*/
#define GNSDK_MUSICIDFILE_FILEINFO_VALUE_TAGID					"gnsdk_midf_fileinfo_value_tagid"
/** GNSDK_MUSICIDFILE_FILEINFO_VALUE_TUI
  * Summary:
  *   Sets or retrieves the TUI value for a FileInfo object.
*/
#define GNSDK_MUSICIDFILE_FILEINFO_VALUE_TUI					"gnsdk_midf_fileinfo_value_tui"
/** GNSDK_MUSICIDFILE_FILEINFO_VALUE_TUI_TAG
  * Summary:
  *   Sets or retrieves the TUI Tag value for a FileInfo object.
*/
#define GNSDK_MUSICIDFILE_FILEINFO_VALUE_TUI_TAG				"gnsdk_midf_fileinfo_value_tuitag"

/** GNSDK_MUSICIDFILE_FILEINFO_VALUE_CDDB_IDS
  * Summary:
  *   Retrieves or sets a Gracenote CDDB ID value for a FileInfo object.
*/
#define GNSDK_MUSICIDFILE_FILEINFO_VALUE_CDDB_IDS				"gnsdk_midf_fileinfo_value_cddb_ids"
/** GNSDK_MUSICIDFILE_FILEINFO_VALUE_MUI
  * Summary:
  *   Sets or retrieves the Media Unique ID (MUI) value for a FileInfo object.
*/
#define GNSDK_MUSICIDFILE_FILEINFO_VALUE_MUI					"gnsdk_midf_fileinfo_value_mui"
/** GNSDK_MUSICIDFILE_FILEINFO_VALUE_MEDIA_ID
  * Summary:
  *   Sets or retrieves the Media ID value for a FileInfo object.
*/
#define GNSDK_MUSICIDFILE_FILEINFO_VALUE_MEDIA_ID				"gnsdk_midf_fileinfo_value_mediaid"
/** GNSDK_MUSICIDFILE_FILEINFO_VALUE_ALBUMARTIST
  * Summary:
  *   Sets or retrieves the Album Artist value for a FileInfo object.
*/
#define GNSDK_MUSICIDFILE_FILEINFO_VALUE_ALBUMARTIST			"gnsdk_midf_fileinfo_value_albumartist"
/** GNSDK_MUSICIDFILE_FILEINFO_VALUE_ALBUMTITLE
  * Summary:
  *   Sets or retrieves the Album Title value for a FileInfo object.
*/
#define GNSDK_MUSICIDFILE_FILEINFO_VALUE_ALBUMTITLE				"gnsdk_midf_fileinfo_value_albumtitle"
/** GNSDK_MUSICIDFILE_FILEINFO_VALUE_TRACKARTIST
  * Summary:
  *   Sets or retrieves the Track Artist value for a FileInfo object.
*/
#define GNSDK_MUSICIDFILE_FILEINFO_VALUE_TRACKARTIST			"gnsdk_midf_fileinfo_value_trackartist"
/** GNSDK_MUSICIDFILE_FILEINFO_VALUE_TRACKTITLE
  * Summary:
  *   Sets or retrieves the Track Title value for a FileInfo object.
*/
#define GNSDK_MUSICIDFILE_FILEINFO_VALUE_TRACKTITLE				"gnsdk_midf_fileinfo_value_tracktitle"
/** GNSDK_MUSICIDFILE_FILEINFO_VALUE_TRACKNUMBER
  * Summary:
  *   Sets or retrieves the Track Number value for a FileInfo object.
*/
#define GNSDK_MUSICIDFILE_FILEINFO_VALUE_TRACKNUMBER			"gnsdk_midf_fileinfo_value_tracknumber"
/** GNSDK_MUSICIDFILE_FILEINFO_VALUE_DISCNUMBER
  * Summary:
  *   Sets or retrieves the Disc Number value for a FileInfo object.
*/
#define GNSDK_MUSICIDFILE_FILEINFO_VALUE_DISCNUMBER				"gnsdk_midf_fileinfo_value_discnumber"
/** GNSDK_MUSICIDFILE_FILEINFO_VALUE_FINGERPRINT
  * Summary:
  *   Sets or retrieves the Fingerprint value for a FileInfo object.
*/
#define GNSDK_MUSICIDFILE_FILEINFO_VALUE_FINGERPRINT			"gnsdk_midf_fileinfo_value_fingerprint"
/** GNSDK_MUSICIDFILE_FILEINFO_VALUE_TOC_OFFSETS
  * Summary:
  *   Sets or retrieves the TOC offsets value for a FileInfo object.
*/
#define GNSDK_MUSICIDFILE_FILEINFO_VALUE_TOC_OFFSETS			"gnsdk_midf_fileinfo_value_toc_offsets"


/******************************************************************************
 * Fileinfo data source values
 * Values for the possible sources of input information for MusicID
 ******************************************************************************/

/** GNSDK_MUSICIDFILE_FILEINFO_VALUE_SOURCE_APPLICATION
  * Summary:
  *   Indicates the FileInfo value is provided by the application.
*/
#define GNSDK_MUSICIDFILE_FILEINFO_VALUE_SOURCE_APPLICATION		"gnsdk_midf_fileinfo_value_source_application"
/** GNSDK_MUSICIDFILE_FILEINFO_VALUE_SOURCE_FILENAME
  * Summary:
  *   Indicates the FileInfo value is derived from parsing the file.
*/
#define GNSDK_MUSICIDFILE_FILEINFO_VALUE_SOURCE_FILENAME		"gnsdk_midf_fileinfo_value_source_filename"

/******************************************************************************
 * File Info Result values
 * Match values associated with a File Info object.
 ******************************************************************************/

/** gnsdk_musicidfile_fileinfo_status_t@1
  * Summary:
  *   The status value of the current query.
*/
typedef enum
{
	/** gnsdk_musicidfile_fileinfo_status_t@1::gnsdk_musicidfile_fileinfo_unprocessed
	  * Summary:
	  *   FileInfo has not been processed.
	*/
	gnsdk_musicidfile_fileinfo_unprocessed		= 0,
	/** gnsdk_musicidfile_fileinfo_status_t@1::gnsdk_musicidfile_fileinfo_processing
	  * Summary:
	  *   FileInfo is currently being processed.
	*/
	gnsdk_musicidfile_fileinfo_processing		= 1,
	/** gnsdk_musicidfile_fileinfo_status_t@1::gnsdk_musicidfile_fileinfo_error
	  * Summary:
	  *   An error occurred while processing the FileInfo.
	*/
	gnsdk_musicidfile_fileinfo_error			= 2,
	/** gnsdk_musicidfile_fileinfo_status_t@1::gnsdk_musicidfile_fileinfo_result_none
	  * Summary:
	  *   No results were found for FileInfo.
	*/
	gnsdk_musicidfile_fileinfo_result_none		= 3,
	/** gnsdk_musicidfile_fileinfo_status_t@1::gnsdk_musicidfile_fileinfo_result_single
	  * Summary:
	  *   Single preferred response available for FileInfo.
	*/
	gnsdk_musicidfile_fileinfo_result_single	= 4,
	/** gnsdk_musicidfile_fileinfo_status_t@1::gnsdk_musicidfile_fileinfo_result_all
	  * Summary:
	  *   All retrieved results available for FileInfo.
	*/
	gnsdk_musicidfile_fileinfo_result_all		= 5

} gnsdk_musicidfile_fileinfo_status_t;

/*
 * File Info Match values
 * Match values associated with an Album Result GDO.
 */

/** GNSDK_MUSICIDFILE_MATCH_HIGH_CONFIDENCE
  * Summary:
  *   Indicates <link MusicID-File> has flagged this result as high
  *   confidence.
*/
#define GNSDK_MUSICIDFILE_MATCH_HIGH_CONFIDENCE					1
/** <combine GNSDK_MUSICIDFILE_MATCH_HIGH_CONFIDENCE>
  *
  * GNSDK_MUSICIDFILE_MATCH_LOW_CONFIDENCE
  * Summary:
  *   Indicates <link MusicID-File> has not flagged this result as high
  *   confidence.
*/
#define GNSDK_MUSICIDFILE_MATCH_LOW_CONFIDENCE					0

/** GNSDK_MUSICIDFILE_GDO_VALUE_IDENT
  * Summary:
  *   Retrieve an identifier value from matching Track GDO (Track context).
*/
#define GNSDK_MUSICIDFILE_GDO_VALUE_IDENT						"gnsdk_midf_val_ident"
/** GNSDK_MUSICIDFILE_GDO_VALUE_FILENAME
  * Summary:
  *   Retrieve a file name value from matching Track GDO (Track context).
*/
#define GNSDK_MUSICIDFILE_GDO_VALUE_FILENAME					"gnsdk_midf_val_filename"
/** GNSDK_MUSICIDFILE_GDO_VALUE_MATCH_TYPE
  * Summary:
  *   Retrieve a match type value for matching Track (Track context).
*/
#define GNSDK_MUSICIDFILE_GDO_VALUE_MATCH_TYPE					"gnsdk_midf_val_matchtype"
/** GNSDK_MUSICIDFILE_GDO_VALUE_MATCH_SCORE
  * Summary:
  *   Retrieve a match score value for matching Track (Track context).
*/
#define GNSDK_MUSICIDFILE_GDO_VALUE_MATCH_SCORE					"gnsdk_midf_val_matchscore"
/** GNSDK_MUSICIDFILE_GDO_VALUE_MATCH_CONFIDENCE
  * Summary:
  *   Retrieves a match confidence value for a matching Track (Track context).
*/
#define GNSDK_MUSICIDFILE_GDO_VALUE_MATCH_CONFIDENCE			"gnsdk_midf_val_matchconf"
/** GNSDK_MUSICIDFILE_GDO_VALUE_ALBUM_GROUP_ID
  * Summary:
  *   Retrieve a value identifying an Album group a FileInfo is a part of.
*/
#define GNSDK_MUSICIDFILE_GDO_VALUE_ALBUM_GROUP_ID				"gnsdk_midf_val_groupid"
/** GNSDK_MUSICIDFILE_GDO_VALUE_ALBUM_CERTIFIED
  * Summary:
  *   Retrieve a certification value for an Album.
*/
#define GNSDK_MUSICIDFILE_GDO_VALUE_ALBUM_CERTIFIED				"gnsdk_midf_val_cert"

/** gnsdk_musicidfile_handle_status_t
  * Summary:
  *   Status codes retrieved from gnsdk_musicidfile_query_status.
  *
  *
  *
  *
*/
typedef enum
{
	gnsdk_musicidfile_status_unknown, /** gnsdk_musicidfile_handle_status_t@1::gnsdk_musicidfile_status_unknown
	                                    * Summary:
	                                    *   The <link MusicID-File> query handle status is unknown.
	                                  */

	/** gnsdk_musicidfile_handle_status_t@1::gnsdk_musicidfile_status_unprocessed
	  * Summary:
	  *   The <link MusicID-File> query handle has yet to be processed.
	*/
	gnsdk_musicidfile_status_unprocessed			= 1,
	/** gnsdk_musicidfile_handle_status_t@1::gnsdk_musicidfile_status_trackid_processing
	  * Summary:
	  *   The <link MusicID-File> query handle is currently performing TrackID
	  *   processing.
	*/
	gnsdk_musicidfile_status_trackid_processing		= 100,
	gnsdk_musicidfile_status_trackid_complete		= 101,/** gnsdk_musicidfile_handle_status_t@1::gnsdk_musicidfile_status_trackid_complete
	                                         		        * Summary:
	                                         		        *   The <link MusicID-File> query handle has completed TrackID processing.
	                                         		      */

	gnsdk_musicidfile_status_trackid_cancelling		= 102,/** gnsdk_musicidfile_handle_status_t@1::gnsdk_musicidfile_status_trackid_cancelling
	                                           		        * Summary:
	                                           		        *   The <link MusicID-File> query handle is currently cancelling TrackID
	                                           		        *   processing.
	                                           		      */

	gnsdk_musicidfile_status_trackid_cancelled		= 103,/** gnsdk_musicidfile_handle_status_t@1::gnsdk_musicidfile_status_trackid_cancelled
	                                          		        * Summary:
	                                          		        *   The <link MusicID-File> query handle has cancelled TrackID processing.
	                                          		      */

	/** gnsdk_musicidfile_handle_status_t@1::gnsdk_musicidfile_status_albumid_processing
	  * Summary:
	  *   The <link MusicID-File> query handle is currently performing AlbumID
	  *   processing.
	*/
	gnsdk_musicidfile_status_albumid_processing		= 200,
	gnsdk_musicidfile_status_albumid_complete		= 201,/** gnsdk_musicidfile_handle_status_t@1::gnsdk_musicidfile_status_albumid_complete
	                                         		        * Summary:
	                                         		        *   The <link MusicID-File> query handle has completed AlbumID processing.
	                                         		      */

	gnsdk_musicidfile_status_albumid_cancelling		= 202,/** gnsdk_musicidfile_handle_status_t@1::gnsdk_musicidfile_status_albumid_cancelling
	                                           		        * Summary:
	                                           		        *   The <link MusicID-File> query handle is currently cancelling AlbumID
	                                           		        *   processing.
	                                           		      */

	gnsdk_musicidfile_status_albumid_cancelled		= 203,/** gnsdk_musicidfile_handle_status_t@1::gnsdk_musicidfile_status_albumid_cancelled
	                                          		        * Summary:
	                                          		        *   The <link MusicID-File> query handle has cancelled AlbumID processing.
	                                          		      */

	/** gnsdk_musicidfile_handle_status_t@1::gnsdk_musicidfile_status_libraryid_processing
	  * Summary:
	  *   The <link MusicID-File> query handle is currently performing LibraryID
	  *   processing.
	*/
	gnsdk_musicidfile_status_libraryid_processing	= 300,
	gnsdk_musicidfile_status_libraryid_complete		= 301,/** gnsdk_musicidfile_handle_status_t@1::gnsdk_musicidfile_status_libraryid_complete
	                                           		        * Summary:
	                                           		        *   The <link MusicID-File> query handle has completed LibraryID processing.
	                                           		      */

	gnsdk_musicidfile_status_libraryid_cancelling	= 302,/** gnsdk_musicidfile_handle_status_t@1::gnsdk_musicidfile_status_libraryid_cancelling
	                                             	        * Summary:
	                                             	        *   The <link MusicID-File> query handle is currently cancelling LibraryID
	                                             	        *   processing.
	                                             	      */

	gnsdk_musicidfile_status_libraryid_cancelled	= 303/** gnsdk_musicidfile_handle_status_t@1::gnsdk_musicidfile_status_libraryid_cancelled
	                                            	       * Summary:
	                                            	       *   The <link MusicID-File> query handle has cancelled LibraryID processing.
	                                            	     */

	/** The MusicID-File query handle has completed processing.
	*/

} gnsdk_musicidfile_handle_status_t;


/******************************************************************************
 * Callbacks
 *
 * The MusicID-File SDK uses callbacks to report status and lookups results.
 * Use of these callbacks is optional,
 * except when the MUSICIDFILE_OPTION_ASYNC flag is used.
 * In that case, the gnsdk_musicidfile_callback_musicid_complete_fn must be set.
 ******************************************************************************/

/** gnsdk_musicidfile_callback_status_t
  * Summary:
  *   Status codes passed to the gnsdk_musicidfile_callback_status_fn
  *   callback.
  *
  *
  *
  *
*/
typedef enum
{
	/** gnsdk_musicidfile_callback_status_t@1::gnsdk_musicidfile_status_fileinfo_processing_begin
	  * Summary:
	  *   <link MusicID-File> processing is beginning for a given FileInfo.
	*/
	gnsdk_musicidfile_status_fileinfo_processing_begin = 0x100,
	/** gnsdk_musicidfile_callback_status_t@1::gnsdk_musicidfile_status_fileinfo_query
	  * Summary:
	  *   Performing a Gracenote query for given FileInfo.
	*/
	gnsdk_musicidfile_status_fileinfo_query = 0x150,
	/** gnsdk_musicidfile_callback_status_t@1::gnsdk_musicidfile_status_fileinfo_processing_complete
	  * Summary:
	  *   <link MusicID-File> processing is complete for a given FileInfo.
	*/
	gnsdk_musicidfile_status_fileinfo_processing_complete = 0x199,
	gnsdk_musicidfile_status_fileinfo_processing_error = 0x299,/** gnsdk_musicidfile_callback_status_t@1::gnsdk_musicidfile_status_fileinfo_processing_error
	                                                             * Summary:
	                                                             *   <link MusicID-File> processing for a given FileInfo has encountered an
	                                                             *   error.
	                                                           */

	/** gnsdk_musicidfile_callback_status_t@1::gnsdk_musicidfile_status_error
	  * Summary:
	  *   An error in <link MusicID-File> querying or processing has occurred.
	*/
	gnsdk_musicidfile_status_error = 0x999

} gnsdk_musicidfile_callback_status_t;

/** gnsdk_musicidfile_callback_status_fn
  * Summary:
  *   Callback function declaration for <link MusicID-File> progress status.
  * Parameters:
  *   userdata:                  [in] Value passed to the user_data parameter
  *                              of the function this callback was passed to
  *   musicidfile_query_handle:  [in] <link MusicID-File> query handle that the
  *                              Fileinfo handle belongs to
  *   fileinfo:                  [in] <link MusicID-File> FileInfo handle that
  *                              the callback operates on
  *   status:                    [in] One of
  *                              gnsdk_musicidfile_callback_status_t values
  *   current_file:              [in] Current number of the file being
  *                              processed (if applicable to status, otherwise
  *                              0)
  *   total_files:               [in] Total number of files to be processed (if
  *                              applicable to status, otherwise 0)
  *   p_abort:                   [out] Set dereferenced value to GNSDK_TRUE to
  *                              abort the operation that is calling the
  *                              callback
  *
*/
typedef gnsdk_void_t
(GNSDK_CALLBACK_API *gnsdk_musicidfile_callback_status_fn)
	(
	const gnsdk_void_t*					userdata,
	gnsdk_musicidfile_query_handle_t	musicidfile_query_handle,
	gnsdk_musicidfile_fileinfo_handle_t	fileinfo,
	gnsdk_musicidfile_callback_status_t	status,
	gnsdk_uint32_t						current_file,
	gnsdk_uint32_t						total_files,
	gnsdk_bool_t*						p_abort
	);

/** gnsdk_musicidfile_callback_get_fingerprint_fn
  * Summary:
  *   Callback function declaration for a fingerprint generation request.
  * Parameters:
  *   userdata:                  [in] Value passed to the user_data parameter
  *                              of the function this callback was passed to
  *   musicidfile_query_handle:  [in] <link MusicID-File> query handle that the
  *                              FileInfo handle belongs to
  *   fileinfo:                  [in] <link MusicID-File> FileInfo handle that
  *                              the callback operates on
  *   current_file:              [in] Current number of the file being
  *                              processed
  *   total_files:               [in] Total number of files to be processed
  *   p_abort:                   [out] Set the dereferenced value to GNSDK_TRUE
  *                              to abort the operation that calls the callback
  *
*/
typedef gnsdk_void_t
(GNSDK_CALLBACK_API *gnsdk_musicidfile_callback_get_fingerprint_fn)
	(
	const gnsdk_void_t*					userdata,
	gnsdk_musicidfile_query_handle_t	musicidfile_query_handle,
	gnsdk_musicidfile_fileinfo_handle_t	fileinfo,
	gnsdk_uint32_t						current_file,
	gnsdk_uint32_t						total_files,
	gnsdk_bool_t*						p_abort
	);

/** gnsdk_musicidfile_callback_get_metadata_fn
  * Summary:
  *   Callback function declaration for a metadata gathering request.
  * Parameters:
  *   userdata:                  [in] Value passed to the user_data parameter
  *                              of the function this callback was passed to
  *   musicidfile_query_handle:  [in] <link MusicID-File> query handle that the
  *                              FileInfo handle belongs to
  *   fileinfo:                  [in] <link MusicID-File> FileInfo handle that
  *                              the callback operates on
  *   current_file:              [in] Current number of the file being
  *                              processed
  *   total_files:               [in] Total number of files to be processed
  *   p_abort:                   [out] Set dereferenced value to GNSDK_TRUE to
  *                              abort the operation that is calling the
  *                              callback
  *
*/
typedef gnsdk_void_t
(GNSDK_CALLBACK_API *gnsdk_musicidfile_callback_get_metadata_fn)
	(
	const gnsdk_void_t*					userdata,
	gnsdk_musicidfile_query_handle_t	musicidfile_query_handle,
	gnsdk_musicidfile_fileinfo_handle_t	fileinfo,
	gnsdk_uint32_t						current_file,
	gnsdk_uint32_t						total_files,
	gnsdk_bool_t*						p_abort
	);

/** gnsdk_musicidfile_callback_result_available_fn
  * Summary:
  *   Callback function declaration for a result available notification.
  * Parameters:
  *   userdata:                  [in] Value passed to the user_data parameter
  *                              of the function this callback was passed to
  *   musicidfile_query_handle:  [in] <link MusicID-File> query handle that the
  *                              callback operates on
  *   album_response:            [in] GDO handle of an album in the response
  *   current_album:             [in] Current number of the album in this
  *                              response
  *   total_albums:              [in] Total number of albums in this response
  *   p_abort:                   [out] Set the dereferenced value to GNSDK_TRUE
  *                              to abort the operation that calls the callback
  *
*/
typedef gnsdk_void_t
(GNSDK_CALLBACK_API *gnsdk_musicidfile_callback_result_available_fn)
	(
	const gnsdk_void_t*					userdata,
	gnsdk_musicidfile_query_handle_t	musicidfile_query_handle,
	gnsdk_gdo_handle_t					album_response,
	gnsdk_uint32_t						current_album,
	gnsdk_uint32_t						total_albums,
	gnsdk_bool_t*						p_abort
	);

/** gnsdk_musicidfile_callback_result_not_found_fn
  * Summary:
  *   Callback function declaration for a no results notification.
  * Parameters:
  *   userdata:                  [in] Value passed to the user_data parameter
  *                              of the function this callback was passed to
  *   musicidfile_query_handle:  [in] <link MusicID-File> query handle that the
  *                              FileInfo handle belongs to
  *   fileinfo:                  [in] <link MusicID-File> FileInfo handle that
  *                              the callback operates on
  *   current_file:              [in] Current number of the file that is not
  *                              found
  *   total_files:               [in] Total number of files to be processed
  *   p_abort:                   [out] Set dereferenced value to GNSDK_TRUE to
  *                              abort the operation that is calling the
  *                              callback
  *                                                                            
*/
typedef gnsdk_void_t
(GNSDK_CALLBACK_API *gnsdk_musicidfile_callback_result_not_found_fn)
	(
	const gnsdk_void_t*					userdata,
	gnsdk_musicidfile_query_handle_t	musicidfile_query_handle,
	gnsdk_musicidfile_fileinfo_handle_t	fileinfo,
	gnsdk_uint32_t						current_file,
	gnsdk_uint32_t						total_files,
	gnsdk_bool_t*						p_abort
	);

/** gnsdk_musicidfile_callback_musicid_complete_fn
  * Summary:
  *   Callback function declaration for <link MusicID-File> processing
  *   completion.
  * Parameters:
  *   userdata:                    [in] Value passed to the user_data parameter
  *                                of the function this callback was passed to
  *   musicidfile_query_handle:    [in] <link MusicID-File> query handle that
  *                                the callback operates on
  *   musicidfile_complete_error:  [in] Final error value of <link MusicID-File>
  *                                operation
  *
*/
typedef gnsdk_void_t
(GNSDK_CALLBACK_API *gnsdk_musicidfile_callback_musicid_complete_fn)
	(
	const gnsdk_void_t*					userdata,
	gnsdk_musicidfile_query_handle_t	musicidfile_query_handle,
	gnsdk_error_t						musicidfile_complete_error
	);

/** gnsdk_musicidfile_callbacks_s
  * Summary:
  *   Structure of callback functions for data retrieval and status updates as
  *   various <link MusicID-File> operations are performed.
*/
typedef struct _gnsdk_musicidfile_callbacks_s
{
	/** _gnsdk_musicidfile_callbacks_s::callback_status
	  * Summary:
	  *   Callback function for <link MusicID-File> progress status.
	*/
	gnsdk_musicidfile_callback_status_fn					callback_status;
	/** _gnsdk_musicidfile_callbacks_s::callback_get_fingerprint
	  * Summary:
	  *   Request for application to generate fingerprint for given FileInfo.
	*/
	gnsdk_musicidfile_callback_get_fingerprint_fn			callback_get_fingerprint;
	/** _gnsdk_musicidfile_callbacks_s::callback_get_metadata
	  * Summary:
	  *   Request for application to retrieve metadata for given FileInfo.
	*/
	gnsdk_musicidfile_callback_get_metadata_fn				callback_get_metadata;
	/** _gnsdk_musicidfile_callbacks_s::callback_result_available
	  * Summary:
	  *   \Result for one or more FileInfo has been determined by <link MusicID-File>.
	*/
	gnsdk_musicidfile_callback_result_available_fn			callback_result_available;
	/** _gnsdk_musicidfile_callbacks_s::callback_result_not_found
	  * Summary:
	  *   No results were found for FileInfo.
	*/
	gnsdk_musicidfile_callback_result_not_found_fn			callback_result_not_found;
	/** _gnsdk_musicidfile_callbacks_s::callback_musicid_complete
	  * Summary:
	  *   <link MusicID-File> processing complete for the given <link MusicID-File>
	  *   query handle.
	*/
	gnsdk_musicidfile_callback_musicid_complete_fn			callback_musicid_complete;

} gnsdk_musicidfile_callbacks_t;


/** gnsdk_musicidfile_fileinfo_callbacks_s
  * Summary:
  *   Structure of callback functions for data retrieval for a specific
  *   FileInfo.
*/
typedef struct _gnsdk_musicidfile_fileinfo_callbacks_s
{
	/** _gnsdk_musicidfile_fileinfo_callbacks_s::callback_get_fingerprint
	  * Summary:
	  *   Request for application to generate fingerprint for given Fileinfo.
	*/
	gnsdk_musicidfile_callback_get_fingerprint_fn	callback_get_fingerprint;
	/** _gnsdk_musicidfile_fileinfo_callbacks_s::callback_get_metadata
	  * Summary:
	  *   Request for application to retrieve metadata for given FileInfo.
	*/
	gnsdk_musicidfile_callback_get_metadata_fn		callback_get_metadata;

} gnsdk_musicidfile_fileinfo_callbacks_t;


/** gnsdk_musicidfile_fileinfo_userdata_delete_fn
  * Summary:
  *   Delete callback function declaration for
  *   gnsdk_musicidfile_query_fileinfo_userdata_set.
  * Parameters:
  *   musicidfile_query_handle:  [in] <link MusicID-File> query handle that the
  *                              FileInfo handle belongs to
  *   fileinfo_handle:           [in] <link MusicID-File> FileInfo handle that
  *                              the userdata belongs to
  *   userdata:                  [in] Value provided to userdata parameter of
  *                              gnsdk_musicidfile_fileinfo_userdata_set that
  *                              is to be deleted
  *
*/
typedef gnsdk_void_t
(GNSDK_CALLBACK_API *gnsdk_musicidfile_fileinfo_userdata_delete_fn)
	(
	gnsdk_musicidfile_query_handle_t	musicidfile_query_handle,
	gnsdk_musicidfile_fileinfo_handle_t	fileinfo_handle,
	gnsdk_void_t*						userdata
	);

/******************************************************************************
 * MusicID-File SDK Initialization APIs
 ******************************************************************************/

/** gnsdk_musicidfile_initialize
  * Summary:
  *   Initializes the Gracenote <link MusicID-File> library.
  * Parameters:
  *   sdkmgr_handle:  [in] Handle from successful gnsdk_manager_initialize
  *                   call
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_initialize(
	gnsdk_manager_handle_t	sdkmgr_handle
	);

/** gnsdk_musicidfile_shutdown
  * Summary:
  *   Shuts down and release resources for the <link MusicID-File> library.
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_shutdown(void);

/** gnsdk_musicidfile_get_version
  * Summary:
  *   Retrieves the <link MusicID-File> library's version string.
*/
gnsdk_cstr_t GNSDK_API
gnsdk_musicidfile_get_version(void);

/** gnsdk_musicidfile_get_build_date
  * Summary:
  *   Retrieves the <link MusicID-File> library's build date string.
*/
gnsdk_cstr_t GNSDK_API
gnsdk_musicidfile_get_build_date(void);

/******************************************************************************
 * MusicID-File SDK query handle APIs
 ******************************************************************************/

/** gnsdk_musicidfile_query_create
  * Summary:
  *   Creates a <link MusicID-File> query handle.
  * Parameters:
  *   user_handle:                 [in] User handle for the user making the
  *                                query request
  *   callbacks:                   [in_opt] Callback function for status and
  *                                progress
  *   callback_userdata:           [in_opt] Data that is passed back through
  *                                calls to the callback functions
  *   p_musicidfile_query_handle:  [out] Pointer to receive the <link MusicID-File>
  *                                query handle
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_query_create(
	gnsdk_user_handle_t					user_handle,
	gnsdk_musicidfile_callbacks_t*		callbacks,
	const gnsdk_void_t*					callback_userdata,
	gnsdk_musicidfile_query_handle_t*	p_musicidfile_query_handle
	);

/** gnsdk_musicidfile_query_release
  * Summary:
  *   Releases resources for given <link MusicID-File> query handle.
  * Parameters:
  *   musicidfile_query_handle:  [in] <link MusicID-File> query handle to
  *                              release
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_query_release(
	gnsdk_musicidfile_query_handle_t	musicidfile_query_handle
	);

/** gnsdk_musicidfile_query_do_trackid
  * Summary:
  *   Initiates TrackID processing.
  * Parameters:
  *   musicidfile_query_handle:  [in] <link MusicID-File> query handle to
  *                              perform TrackID with
  *   query_flags:               [in] One or more <link !!MACROS_midf_query_flags, MusicID-File Query Flag>
  *                              values
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_query_do_trackid(
	gnsdk_musicidfile_query_handle_t	musicidfile_query_handle,
	gnsdk_uint32_t						query_flags
	);

/** gnsdk_musicidfile_query_do_albumid
  * Summary:
  *   Initiates AlbumID processing.
  * Parameters:
  *   musicidfile_query_handle:  [in] <link MusicID-File> query handle to
  *                              perform AlbumID processing with
  *   query_flags:               [in] One or more <link !!MACROS_midf_query_flags, MusicID-File Query Flag>
  *                              values
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_query_do_albumid(
	gnsdk_musicidfile_query_handle_t	musicidfile_query_handle,
	gnsdk_uint32_t						query_flags
	);

/** gnsdk_musicidfile_query_do_libraryid
  * Summary:
  *   Initiates LibraryID processing.
  * Parameters:
  *   musicidfile_query_handle:  [in] <link MusicID-File> query handle to
  *                              perform LibraryID processing with
  *   query_flags:               [in] One or more <link !!MACROS_midf_query_flags, MusicID-File Query Flag>
  *                              values
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_query_do_libraryid(
	gnsdk_musicidfile_query_handle_t	musicidfile_query_handle,
	gnsdk_uint32_t						query_flags
	);

/** gnsdk_musicidfile_query_cancel
  * Summary:
  *   Sets the internal cancel flag for a <link MusicID-File> query handle
  *   that is currently being processed.
  * Parameters:
  *   musicidfile_query_handle:  [in] <link MusicID-File> query handle to cancel
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_query_cancel(
	gnsdk_musicidfile_query_handle_t	musicidfile_query_handle
	);

/** gnsdk_musicidfile_query_status
  * Summary:
  *   Retrieves the processing status of a given <link MusicID-File> query
  *   handle.
  * Parameters:
  *   musicidfile_query_handle:      [in] <link MusicID-File> query handle to
  *                                  retrieve status for
  *   p_musicidfile_handle_status:   [out] Pointer to receive handle <link gnsdk_musicidfile_handle_status_t, status value>
  *   p_musicidfile_complete_error:  [out] Pointer to receive last processing
  *                                  error value
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_query_status(
	gnsdk_musicidfile_query_handle_t	musicidfile_query_handle,
	gnsdk_musicidfile_handle_status_t*	p_musicidfile_handle_status,
	gnsdk_error_t*						p_musicidfile_complete_error
	);

/** gnsdk_musicidfile_query_wait_for_complete
  * Summary:
  *   Sets a wait time for a <link MusicID-File> operation to complete.
  * Parameters:
  *   musicidfile_query_handle:      [in] <link MusicID-File> query handle
  *                                  currently processing asynchronously
  *   timeout_value:                 [in] Length of time to wait in
  *                                  milliseconds, or the <link GNSDK_MUSICIDFILE_TIMEOUT_INFINITE, GNSDK_MUSICIDFILE_TIMEOUT_INFINITE flag>
  *   p_musicidfile_complete_error:  [out] Pointer to receive error value that
  *                                  is returned from <link MusicID-File>
  *                                  processing upon completion.
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_query_wait_for_complete(
	gnsdk_musicidfile_query_handle_t	musicidfile_query_handle,
	gnsdk_uint32_t						timeout_value,
	gnsdk_error_t*						p_musicidfile_complete_error
	);

/** GNSDK_MUSICIDFILE_TIMEOUT_INFINITE
  * Summary:
  *   Value for infinite wait in a call to
  *   gnsdk_musicidfile_query_wait_for_complete.
  *
  *   Otherwise enter the timeout duration in milliseconds.
*/
#define GNSDK_MUSICIDFILE_TIMEOUT_INFINITE						0xffffffff



/** gnsdk_musicidfile_query_option_set
  * Summary:
  *   Sets an option for a given <link MusicID-File> query handle.
  * Parameters:
  *   musicidfile_query_handle:  [in] <link MusicID-File> query handle to set
  *                              option for
  *   option_key:                [in] One of the <link !!MACROS_midf_option_keys, MusicID-File Option Keys>
  *                              for the option to set
  *   option_value:              [in] String value or one of <link !!MACROS_midf_option_values, MusicID-File Option Values>
  *                              that corresponds to option key
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_query_option_set(
	gnsdk_musicidfile_query_handle_t	musicidfile_query_handle,
	gnsdk_cstr_t						option_key,
	gnsdk_cstr_t						option_value
	);

/** gnsdk_musicidfile_query_option_get
  * Summary:
  *   Retrieves an option for a given <link MusicID-File> query handle.
  * Parameters:
  *   musicidfile_query_handle:  [in] <link MusicID-File> query handle to
  *                              retrieve option from
  *   option_key:                [in] One of the <link !!MACROS_midf_option_keys, MusicID-File Option Keys>
  *                              to retrieve an option value for
  *   p_option_value:            [out] Pointer to receive option value
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_query_option_get(
	gnsdk_musicidfile_query_handle_t	musicidfile_query_handle,
	gnsdk_cstr_t						option_key,
	gnsdk_cstr_t*						p_option_value
	);

/** gnsdk_musicidfile_query_get_response_gdo
  * Summary:
  *   Retrieves a GDO handle containing a response with all albums for all
  *   matching FileInfos.
  * Parameters:
  *   musicidfile_query_handle:  [in] MusicID\-File query handle to retrieve
  *                              response GDO for
  *   p_response_gdo:            [out] Pointer to receive the response GDO
  *                              handle
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_query_get_response_gdo(
	gnsdk_musicidfile_query_handle_t	musicidfile_query_handle,
	gnsdk_gdo_handle_t*					p_response_gdo
	);

/******************************************************************************
 * MusicID-File SDK filinfo handle APIs
 ******************************************************************************/

/** gnsdk_musicidfile_query_fileinfo_create
  * Summary:
  *   Creates a gnsdk_musicidfile_fileinfo_handle_t for a <link MusicID-File>
  *   query handle.
  * Parameters:
  *   musicidfile_query_handle:  [in] <link MusicID-File> query handle to
  *                              create FileInfo for
  *   ident_str:                 [in] Unique string identifier for the media
  *                              \file this FileInfo represents
  *   callbacks:                 [in_opt] Callback function for status and
  *                              progress for this FileInfo
  *   callback_userdata:         [in_opt] Data that is passed back through
  *                              calls to the callback functions for this
  *                              FileInfo
  *   p_fileinfo_handle:         [out] Pointer to receive created FileInfo
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_query_fileinfo_create(
	gnsdk_musicidfile_query_handle_t			musicidfile_query_handle,
	gnsdk_cstr_t								ident_str,
	gnsdk_musicidfile_fileinfo_callbacks_t*		callbacks,
	const gnsdk_void_t*							callback_userdata,
	gnsdk_musicidfile_fileinfo_handle_t* const	p_fileinfo_handle
	);

/** gnsdk_musicidfile_query_fileinfo_remove
  * Summary:
  *   Removes a FileInfo object from a <link MusicID-File> query handle.
  * Parameters:
  *   musicidfile_query_handle:  [in] <link MusicID-File> query handle to
  *                              remove FileInfo from
  *   fileinfo_handle:           [in] FileInfo handle to remove
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_query_fileinfo_remove(
	gnsdk_musicidfile_query_handle_t		musicidfile_query_handle,
	gnsdk_musicidfile_fileinfo_handle_t		fileinfo_handle
	);

/** gnsdk_musicidfile_query_fileinfo_create_from_xml
  * Summary:
  *   Creates and populate FileInfos from a given XML source.
  * Parameters:
  *   musicidfile_query_handle:  [in] <link MusicID-File> query handle to
  *                              remove FileInfo from
  *   fileinfo_xml:              [in] XML source containing FileInfo data
  *   p_count:                   [out] Pointer to receive number of FileInfo
  *                              elements created
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_query_fileinfo_create_from_xml(
	gnsdk_musicidfile_query_handle_t	musicidfile_query_handle,
	gnsdk_cstr_t						fileinfo_xml,
	gnsdk_uint32_t*						p_count
	);

/** gnsdk_musicidfile_query_fileinfo_count
  * Summary:
  *   Retrieves the number of the FileInfo objects created in a given <link MusicID-File>
  *   query handle.
  * Parameters:
  *   musicidfile_query_handle:  [in] <link MusicID-File> query handle to
  *                              retrieve FileInfo count from
  *   p_count:                   [out] Pointer to receive FileInfo count
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_query_fileinfo_count(
	gnsdk_musicidfile_query_handle_t	musicidfile_query_handle,
	gnsdk_uint32_t*						p_count
	);

/** gnsdk_musicidfile_query_fileinfo_get_by_index
  * Summary:
  *   Retrieves the index'th FileInfo object associated with a <link MusicID-File>
  *   handle.
  * Parameters:
  *   musicidfile_query_handle:  [in] <link MusicID-File> query handle to
  *                              retrieve FileInfo from
  *   index:                     [in] Index of FileInfo to retrieve
  *   p_fileinfo_handle:         [out] Pointer to receive FileInfo handle
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_query_fileinfo_get_by_index(
	gnsdk_musicidfile_query_handle_t			musicidfile_query_handle,
	gnsdk_uint32_t								index,
	gnsdk_musicidfile_fileinfo_handle_t* const	p_fileinfo_handle
	);

/** gnsdk_musicidfile_query_fileinfo_get_by_ident
  * Summary:
  *   Retrieves the FileInfo object that matches the given identifier string
  *   and is associated with the given <link MusicID-File> handle.
  * Parameters:
  *   musicidfile_query_handle:  [in] <link MusicID-File> query handle to
  *                              retrieve FileInfo from
  *   ident_str:                 [in] String identifier of FileInfo to retrieve
  *   p_fileinfo_handle:         [out] Pointer to receive FileInfo handle
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_query_fileinfo_get_by_ident(
	gnsdk_musicidfile_query_handle_t			musicidfile_query_handle,
	gnsdk_cstr_t								ident_str,
	gnsdk_musicidfile_fileinfo_handle_t* const	p_fileinfo_handle
	);

/** gnsdk_musicidfile_query_fileinfo_get_by_filename
  * Summary:
  *   Retrieves the FileInfo object that matches the given file name and is
  *   associated with the given <link MusicID-File> handle.
  * Parameters:
  *   musicidfile_query_handle:  [in] <link MusicID-File> query handle to
  *                              retrieve FileInfo from
  *   file_name:                 [in] File name or other string identifier of
  *                              FileInfo to retrieve
  *   p_fileinfo_handle:         [out] Pointer to receive FileInfo handle
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_query_fileinfo_get_by_filename(
	gnsdk_musicidfile_query_handle_t			musicidfile_query_handle,
	gnsdk_cstr_t								file_name,
	gnsdk_musicidfile_fileinfo_handle_t* const	p_fileinfo_handle
	);

/** gnsdk_musicidfile_query_fileinfo_get_by_folder
  * Summary:
  *   Retrieves the index'th FileInfo object matching the given folder name
  *   and associated with the given <link MusicID-File> handle.
  * Parameters:
  *   musicidfile_query_handle:  [in] <link MusicID-File> query handle to
  *                              retrieve FileInfo from
  *   folder_name:               [in] Folder name of FileInfo to retrieve
  *   index:                     [in] Index of FileInfo in folder name to
  *                              retrieve
  *   p_fileinfo_handle:         [out] Pointer to receive FileInfo handle
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_query_fileinfo_get_by_folder(
	gnsdk_musicidfile_query_handle_t			musicidfile_query_handle,
	gnsdk_cstr_t								folder_name,
	gnsdk_uint32_t								index,
	gnsdk_musicidfile_fileinfo_handle_t* const	p_fileinfo_handle
	);

/** gnsdk_musicidfile_fileinfo_metadata_set
  * Summary:
  *   Sets metadata information for a FileInfo object.
  * Parameters:
  *   fileinfo_handle:  [in] FileInfo object handle to set metadata for
  *   metadata_key:     [in] One of the <link !!MACROS_midf_fileinfo_values, FileInfo value>
  *                     keys
  *   metadata_value:   [in] String value for the specified metadata_key
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_fileinfo_metadata_set(
	gnsdk_musicidfile_fileinfo_handle_t fileinfo_handle,
	gnsdk_cstr_t						metadata_key,
	gnsdk_cstr_t						metadata_value
	);

/** gnsdk_musicidfile_fileinfo_metadata_get
  * Summary:
  *   Retrieves a metadata input value from a FileInfo object.
  * Parameters:
  *   fileinfo_handle:      [in] Fileinfo object handle to retrieve metadata
  *                         for
  *   metadata_key:         [in] One of the <link !!MACROS_midf_fileinfo_values, Fileinfo value>
  *                         keys
  *   p_data_value:         [out] Pointer to receive the data value defined for
  *                         the specified metadata_key
  *   p_data_value_source:  [out] Pointer to receive a <link !!MACROS_midf_fileinfo_value_sources, FileInfo value source>
  *                         value indicating the whether the data value's
  *                         source is either the application or a file
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_fileinfo_metadata_get(
	gnsdk_musicidfile_fileinfo_handle_t fileinfo_handle,
	gnsdk_cstr_t						metadata_key,
	gnsdk_cstr_t*						p_data_value,
	gnsdk_cstr_t*						p_data_value_source
	);

/** gnsdk_musicidfile_fileinfo_status
  * Summary:
  *   Retrieves the current status for a specific FileInfo handle.
  * Parameters:
  *   fileinfo_handle:    [in] FileInfo object handle to retrieve response code
  *                       for
  *   p_fileinfo_status:  [out] Pointer to receive status value
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_fileinfo_status(
	gnsdk_musicidfile_fileinfo_handle_t		fileinfo_handle,
	gnsdk_musicidfile_fileinfo_status_t*	p_fileinfo_status
	);

/** gnsdk_musicidfile_fileinfo_get_response_gdo
  * Summary:
  *   Retrieves a GDO handle containing the response for the given FileInfo
  *   handle.
  * Parameters:
  *   fileinfo_handle:  [in] FileInfo handle to retrieve GDO from
  *   p_response_gdo:   [out] Pointer to receive the GDO handle
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_fileinfo_get_response_gdo(
	gnsdk_musicidfile_fileinfo_handle_t	fileinfo_handle,
	gnsdk_gdo_handle_t*					p_response_gdo
	);

/** gnsdk_musicidfile_fileinfo_userdata_set
  * Summary:
  *   Sets application data for a specific FileInfo handle.
  * Parameters:
  *   fileinfo_handle:  [in] FileInfo object handle to set application user data
  *                     for
  *   userdata:         [in] Application\-defined user data
  *   delete_callback:  [in_opt] Application\-provided delete callback for user
  *                     data
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_fileinfo_userdata_set(
	gnsdk_musicidfile_fileinfo_handle_t				fileinfo_handle,
	gnsdk_void_t*									userdata,
	gnsdk_musicidfile_fileinfo_userdata_delete_fn	delete_callback
	);

/** gnsdk_musicidfile_fileinfo_userdata_get
  * Summary:
  *   Retrieves application user data from a specific FileInfo handle.
  * Parameters:
  *   fileinfo_handle:  [in] FileInfo object handle to retrieve application
  *                     user data from
  *   p_userdata:       [out] Pointer to receive application user data value
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_fileinfo_userdata_get(
	gnsdk_musicidfile_fileinfo_handle_t		fileinfo_handle,
	gnsdk_void_t**							p_userdata
	);


/******************************************************************************
 * MusicID-File SDK Fingerprint Generation APIs
 ******************************************************************************/

/** gnsdk_musicidfile_fileinfo_fingerprint_begin
  * Summary:
  *   Initializes fingerprint generation for a FileInfo handle.
  * Parameters:
  *   fileinfo_handle:    [in] FileInfo handle to generate the fingerprint for
  *   audio_sample_rate:  [in] Sample frequency of audio to be provided\: 11
  *                       kHz, 22 kHz, or 44 kHz
  *   audio_sample_size:  [in] Sample rate of audio to be provided (in 8\-bit,
  *                       16\-bit, or 32\-bit bytes per sample)
  *   audio_channels:     [in] Number of channels for audio to be provided (1
  *                       or 2)
  *                                                                           
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_fileinfo_fingerprint_begin(
	gnsdk_musicidfile_fileinfo_handle_t	fileinfo_handle,
	gnsdk_uint32_t						audio_sample_rate,
	gnsdk_uint32_t						audio_sample_size,
	gnsdk_uint32_t						audio_channels
	);

/** gnsdk_musicidfile_fileinfo_fingerprint_write
  * Summary:
  *   Provides uncompressed audio data to a FileInfo handle for fingerprint
  *   generation.
  * Parameters:
  *   fileinfo_handle:  [in] FileInfo handle to generate the fingerprint for
  *   audio_data:       [in] Pointer to audio data buffer that matches the
  *                     audio format described to
  *                     gnsdk_musicidfile_fileinfo_fingerprint_begin
  *   audio_data_size:  [in] Size of audio data buffer (in bytes)
  *   pb_complete:      [out] Pointer to receive whether the fingerprint
  *                     generation has received enough audio data
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_fileinfo_fingerprint_write(
	gnsdk_musicidfile_fileinfo_handle_t fileinfo_handle,
	const gnsdk_void_t*					audio_data,
	gnsdk_size_t						audio_data_size,
	gnsdk_bool_t*						pb_complete
	);

/** gnsdk_musicidfile_fileinfo_fingerprint_end
  * Summary:
  *   Finalizes fingerprint generation for a FileInfo handle.
  * Parameters:
  *   fileinfo_handle:  [in] FileInfo handle to generate the fingerprint for
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicidfile_fileinfo_fingerprint_end(
	gnsdk_musicidfile_fileinfo_handle_t fileinfo_handle
	);


#ifdef __cplusplus
}
#endif

#endif /* _GNSDK_MUSICID_FILE_H_ */
