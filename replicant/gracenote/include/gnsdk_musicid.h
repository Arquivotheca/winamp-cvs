/** Gracenote SDK: MusicID public header file
  * Author:
  *   Copyright (c) 2012 Gracenote, Inc.
  *   
  *   This software may not be used in any way or distributed without
  *   permission. All rights reserved.
  *   
  *   Some code herein may be covered by US and international patents.
*/

#ifndef _GNSDK_MUSICID_H_
/** gnsdk_musicid.h: Primary interface for the MusicID SDK.
*/
#define _GNSDK_MUSICID_H_

#ifdef __cplusplus
extern "C"{
#endif


/******************************************************************************
 * Typdefs
 ******************************************************************************/

/** <title gnsdk_musicid_query_handle_t>
  * <toctitle gnsdk_musicid_query_handle_t>
  * 
  * gnsdk_musicid_query_handle_t
  * Summary:
  *   Handle for a single MusicID query that is created by the
  *   gnsdk_musicid_query_create API.
  *   
  *   The application must create this handle for each MusicID query it needs
  *   to execute.
  *   
  *                                                                          
*/
GNSDK_DECLARE_HANDLE( gnsdk_musicid_query_handle_t );

/** gnsdk_musicid_match_type_t@1
  * Summary:
  *   Indicates a MusicID match type value.
*/
typedef enum
{
	gnsdk_musicid_match_type_unknown	= 0,/** gnsdk_musicid_match_type_unknown
	  * Summary:
	  *   Match type unknown.           
	*/
	
	gnsdk_musicid_match_type_none,				/** gnsdk_musicid_match_type_none
	                              				  * Summary:
	                              				  *   No matches; no response GDO.
	                              				*/
	gnsdk_musicid_match_type_single_exact,		/** gnsdk_musicid_match_type_single_exact
	                                      		  * Summary:
	                                      		  *   One exact match; single full album in the response GDO.
	                                      		*/
	gnsdk_musicid_match_type_multi_exact,		/** gnsdk_musicid_match_type_multi_exact
	                                     		  * Summary:
	                                     		  *   More than one exact match; partial albums in the response GDO.
	                                     		*/
	gnsdk_musicid_match_type_fuzzy,				/** gnsdk_musicid_match_type_fuzzy
	                               				  * Summary:
	                               				  *   One or more fuzzy matches; partial album(s) in the response GDO.
	                               				*/
	gnsdk_musicid_match_type_latest_revision 	/** gnsdk_musicid_match_type_latest_revision
	                                         	  * Summary:
	                                         	  *   \Input GDO is of latest revision; no response GDO.
	                                         	*/

} gnsdk_musicid_match_type_t;

/** gnsdk_musicid_query_status_t
  * Summary:
  *   Indicates a MusicID callback function status value.
*/
typedef enum
{
	gnsdk_musicid_status_unknown = 0,/** gnsdk_musicid_status_unknown
	                                   * Summary:
	                                   *   MusicID query status is unknown.
	                                 */
	
	gnsdk_musicid_status_query_begin = 10,
	

	gnsdk_musicid_status_connecting = 20,/** gnsdk_musicid_status_connecting
	                                 * Summary:
	                                 *   MusicID query is starting a network connection
	                               */
	
	gnsdk_musicid_status_sending = 30,/** gnsdk_musicid_status_sending
	                               * Summary:
	                               *   MusicID is sending query data.
	                             */
	
	gnsdk_musicid_status_receiving = 40,/** gnsdk_musicid_status_receiving
	                                 * Summary:
	                                 *   MusicID is receiving query data.
	                               */
	
	gnsdk_musicid_status_query_complete = 100, /** \ \ 
	                                             * Summary:
	                                             *   MusicID query handle is completing the query process.
	                                           */
	

	gnsdk_musicid_status_query_delete = 999 /** gnsdk_musicid_status_query_delete
	                                    * Summary:
	                                    *   MusicID query handle is about to be deleted.      
	                                  */
} gnsdk_musicid_query_status_t;


/** gnsdk_musicid_query_callback_fn
  * Summary:
  *   Receive status updates as MusicID queries are performed.
  * Parameters:
  *   user_data:             [in] Pointer to data passed in to the
  *                          gnsdk_musicid_query_create function through the
  *                          callback_userdata parameter. This pointer must be
  *                          cast from the gnsdk_void_t type to its original
  *                          type to be accessed properly.
  *   musicid_query_handle:  [in] MusicID query handle that the callback
  *                          operates on
  *   status:                [in] One of gnsdk_musicid_query_status_t values
  *   bytes_done:            [in] Current number of bytes transferred. Set to a
  *                          value greater than 0 to indicate progress, or 0 to
  *                          indicate no progress.
  *   bytes_total:           [in] Total number of bytes to be transferred. Set
  *                          to a value greater than 0 to indicate progress, or
  *                          0 to indicate no progress.
  *   p_abort:               [out] Set dereferenced value to GNSDK_TRUE to
  *                          abort the operation that is calling the callback
  *                                                                            
*/
typedef gnsdk_void_t
(GNSDK_CALLBACK_API *gnsdk_musicid_query_callback_fn)(
	const gnsdk_void_t*				user_data,
	gnsdk_musicid_query_handle_t	musicid_query_handle,
	gnsdk_musicid_query_status_t	status,
	gnsdk_size_t					bytes_done,
	gnsdk_size_t					bytes_total,
	gnsdk_bool_t*					p_abort
	);


/******************************************************************************
 * Initialization APIs
 ******************************************************************************/

/** gnsdk_musicid_initialize
  * Summary:
  *   Initializes the MusicID library.
  * Parameters:
  *   sdkmgr_handle:  [in] Handle from a successful gnsdk_manager_initialize
  *                   call
  *                                                                            
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_initialize(
	gnsdk_manager_handle_t	sdkmgr_handle
	);

/** gnsdk_musicid_shutdown
  * Summary:
  *   Shuts down and release resources for the MusicID library.
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_shutdown(void);

/** gnsdk_musicid_get_version
  * Summary:
  *   Retrieves the MusicID SDK's version string.
*/
gnsdk_cstr_t GNSDK_API
gnsdk_musicid_get_version(void);

/** gnsdk_musicid_get_build_date
  * Summary:
  *   Retrieves the MusicID SDK's build date string.
*/
gnsdk_cstr_t GNSDK_API
gnsdk_musicid_get_build_date(void);


/******************************************************************************
 * MusicID Query Instance Handle - for the life of the query
 ******************************************************************************/

/** gnsdk_musicid_query_create
  * Summary:
  *   Creates a MusicID query handle.
  * Parameters:
  *   user_handle:             [in] User handle for the user requesting the
  *                            query
  *   callback_fn:             [in_opt] Callback function for status and
  *                            progress
  *   callback_userdata:       [in_opt] Data that is passed back through calls
  *                            to the callback function
  *   p_musicid_query_handle:  [out] Pointer to receive a MusicID query handle
  *                                                                            
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_query_create(
	gnsdk_user_handle_t				user_handle,
	gnsdk_musicid_query_callback_fn	callback_fn,
	const gnsdk_void_t*				callback_userdata,
	gnsdk_musicid_query_handle_t*	p_musicid_query_handle
	);

/** gnsdk_musicid_query_release
  * Summary:
  *   Invalidates and releases resources for a given MusicID query handle.
  * Parameters:
  *   musicid_query_handle:  [in] MusicID query handle to release
  *                                                                       
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_query_release(
	gnsdk_musicid_query_handle_t	musicid_query_handle
	);


/******************************************************************************
 * MID Query Inputs APIs
 ******************************************************************************/

/** gnsdk_musicid_query_set_toc_string
  * Summary:
  *   Sets a CD TOC to allow querying of the applicable MusicID query handle.
  * Parameters:
  *   musicid_query_handle:  [in] MusicID query handle this CD TOC applies to
  *   toc_string:            [in] Externally produced CD TOC string
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_query_set_toc_string(
	gnsdk_musicid_query_handle_t	musicid_query_handle,
	gnsdk_cstr_t					toc_string
	);

/** gnsdk_musicid_query_add_toc_offset
  * Summary:
  *   Sets CD TOC offset values to allow querying of the applicable MusicID
  *   query handle.
  * Parameters:
  *   musicid_query_handle:  [in] MusicID query handle this TOC offset applies
  *                          to
  *   toc_offset:            [in] Value of CD TOC offset
  *                                                                           
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_query_add_toc_offset(
	gnsdk_musicid_query_handle_t	musicid_query_handle,
	gnsdk_uint32_t					toc_offset
	);

/** gnsdk_musicid_query_set_text
  * Summary:
  *   Sets text for a search field used to search a MusicID query handle.
  * Parameters:
  *   musicid_query_handle:  [in] MusicID query handle the text applies to
  *   search_field:          [in] <link !!MACROS_mid_search_fields, Search fields>
  *                          the search text applies to
  *   search_text:           [in] Actual text set to perform a search
  *                                                                               
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_query_set_text(
	gnsdk_musicid_query_handle_t	musicid_query_handle,
	gnsdk_cstr_t					search_field,
	gnsdk_cstr_t					search_text
	);

/** GNSDK_MUSICID_FIELD_ARTIST
  * Summary:
  *   Specifies text for an artist name search field that is used with
  *   gnsdk_musicid_query_set_text.                                   
*/
#define GNSDK_MUSICID_FIELD_ARTIST				"gnsdk_musicid_field_artist"
/** GNSDK_MUSICID_FIELD_ALBUM
  * Summary:
  *   Specifies text for an album title search field that is used with
  *   gnsdk_musicid_query_set_text.                                   
*/
#define GNSDK_MUSICID_FIELD_ALBUM				"gnsdk_musicid_field_album"
/** GNSDK_MUSICID_FIELD_TITLE
  * Summary:
  *   Specifies text for a track title search field that is used with
  *   gnsdk_musicid_query_set_text.                                  
*/
#define GNSDK_MUSICID_FIELD_TITLE				"gnsdk_musicid_field_title"
/** GNSDK_MUSICID_LYRIC_FRAGMENT
  * Summary:
  *   Specifies text for an lyric fragment search field that is used with
  *   gnsdk_musicid_query_set_text.                                      
*/
#define GNSDK_MUSICID_FIELD_LYRIC_FRAGMENT		"gnsdk_musicid_field_fragment"


/** gnsdk_musicid_query_set_fp_data
  * Summary:
  *   Sets externally-generated fingerprint data used to query a MusicID query
  *   handle.
  * Parameters:
  *   musicid_query_handle:  [in] MusicID query handle this fingerprint data
  *                          applies to
  *   fp_data:               [in] String representation of fingerprint data
  *   fp_data_type:          [in] One of the available <link !!MACROS_mid_fp_types, fingerprint data types>
  *                          given by fp_data_type, either Gracenote
  *                          Fingerprint Extraction (GNFPX) or Cantametrix
  *                          (CMX).
  *                                                                                                        
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_query_set_fp_data(
	gnsdk_musicid_query_handle_t	musicid_query_handle,
	gnsdk_cstr_t					fp_data,
	gnsdk_cstr_t					fp_data_type
	);

/** gnsdk_musicid_query_get_fp_data
  * Summary:
  *   Retrieves externally-generated and internally-generated Gracenote
  *   Fingerprint Extraction (GNFPX) or Cantametrix (CMX) fingerprint data.
  * Parameters:
  *   musicid_query_handle:  [in] MusicID query handle to retrieve the
  *                          fingerprint for
  *   p_fp_data:             [out] Pointer to receive the fingerprint data
  *                                                                        
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_query_get_fp_data(
	gnsdk_musicid_query_handle_t	musicid_query_handle,
	gnsdk_cstr_t					*p_fp_data
	);

/** GNSDK_MUSICID_FP_DATA_TYPE_GNFPX
  * Summary:
  *   Specifies a Gracenote Fingerprint Extraction (GNFPX) data type for
  *   generating fingerprints used with <link musicid_overview, MusicID-Stream>.
  *   
  *   \Note: You must use only this data type when generating fingerprints for
  *   use with GNSDK_GDO_VALUE_TRACK_MATCHED_POSITION.                          
*/
#define GNSDK_MUSICID_FP_DATA_TYPE_GNFPX			"gnsdk_musicid_fp_gnfpx"
#define GNSDK_MUSICID_FP_DATA_TYPE_STREAM3			"gnsdk_musicid_fp_3stream"
#define GNSDK_MUSICID_FP_DATA_TYPE_STREAM6			"gnsdk_musicid_fp_6stream"
/** GNSDK_MUSICID_FP_DATA_TYPE_CMX
  * Summary:
  *   Specifies a Cantametrix (CMX) fingerprint data type for generating
  *   fingerprints used with MusicID-File.
  *   
  *                                                                     
*/
#define GNSDK_MUSICID_FP_DATA_TYPE_CMX				"gnsdk_musicid_fp_cmx"
#define GNSDK_MUSICID_FP_DATA_TYPE_FILE				"gnsdk_musicid_fp_file"

/** gnsdk_musicid_query_set_gdo
  * Summary:
  *   Sets a GDO to be used as input for a followup query.
  * Parameters:
  *   musicid_query_handle:  [in] MusicID query handle this GDO applies to
  *   query_gdo:             [in] Handle of GDO to query
  *                                                                       
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_query_set_gdo(
	gnsdk_musicid_query_handle_t	musicid_query_handle,
	gnsdk_gdo_handle_t				query_gdo
	);


/** gnsdk_musicid_query_fingerprint_begin
  * Summary:
  *   Initializes native fingerprint generation for a query handle.
  * Parameters:
  *   musicid_query_handle:  [in] MusicID query handle to generate the
  *                          fingerprint for
  *   fp_data_type:          [in] One of the available <link !!MACROS_mid_fp_types, fingerprint data types>
  *                          given by fp_data_type, either Gracenote
  *                          Fingerprint Extraction (GNFPX) or Cantametrix
  *                          (CMX).
  *   audio_sample_rate:     [in] Sample rate of audio to be provided in Hz.
  *                          (for example, 44100)
  *   audio_sample_size:     [in] Size of a single sample of audio to be
  *                          provided\: 8 for 8\-bit audio (0\-255 integers),
  *                          16 for 16\-bit audio (\-32767–32768 integers), and
  *                          32 for 32\-bit audio (\-1.0–1.0 floating point)
  *   audio_channels:        [in] Number of channels for audio to be provided
  *                          (1 or 2)
  *                                                                                                        
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_query_fingerprint_begin(
	gnsdk_musicid_query_handle_t	musicid_query_handle,
	gnsdk_cstr_t					fp_data_type,
	gnsdk_uint32_t					audio_sample_rate,
	gnsdk_uint32_t					audio_sample_size,
	gnsdk_uint32_t					audio_channels
	);

/** gnsdk_musicid_query_fingerprint_write
  * Summary:
  *   Provides uncompressed audio data to a query handle for native
  *   fingerprint generation and returns boolean value indicating when enough
  *   data has been received.
  * Parameters:
  *   musicid_query_handle:  [in] MusicID query handle to generate the
  *                          fingerprint for
  *   audio_data:            [in] Pointer to audio data buffer that matches
  *                          audio format described to
  *                          gnsdk_musicid_query_fingerprint_begin
  *   audio_data_size:       [in] Size of audio data buffer in bytes
  *   pb_complete:           [out] Pointer to receive boolean value indicating
  *                          whether the fingerprint generation process
  *                          gathered enough audio data
  *                                                                           
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_query_fingerprint_write(
	gnsdk_musicid_query_handle_t	musicid_query_handle,
	const gnsdk_void_t*				audio_data,
	gnsdk_size_t					audio_data_size,
	gnsdk_bool_t*					pb_complete
	);

/** gnsdk_musicid_query_fingerprint_end
  * Summary:
  *   Finalizes native fingerprint generation for a MusicID query handle.
  * Parameters:
  *   musicid_query_handle:  [in] MusicID query handle to generate the
  *                          fingerprint for
  *                                                                      
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_query_fingerprint_end(
	gnsdk_musicid_query_handle_t	musicid_query_handle
	);


/** gnsdk_musicid_query_option_set
  * Summary:
  *   Sets an option for a given MusicID query handle.
  * Parameters:
  *   musicid_query_handle:  [in] MusicID query handle to set an option for
  *   option_key:            [in] An option from the available <link !!MACROS_mid_option_keys, MusicID Option Keys>
  *   option_value:          [in] A string value or one of the available <link !!MACROS_mid_option_values, MusicID Option Values>
  *                          that corresponds to the defined option key
  *                                                                                                                              
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_query_option_set(
	gnsdk_musicid_query_handle_t	musicid_query_handle,
	gnsdk_cstr_t					option_key,
	gnsdk_cstr_t					option_value
	);

/** gnsdk_musicid_query_option_get
  * Summary:
  *   Retrieves an option for a given MusicID query handle.
  * Parameters:
  *   musicid_query_handle:  [in] MusicID query handle to retrieve an option
  *                          from
  *   option_key:            [in] One of the <link !!MACROS_mid_option_keys, MusicID Option Keys>
  *                          to retrieve an option value for
  *   p_option_value:        [out] Pointer to receive option value
  *                                                                                              
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_query_option_get(
	gnsdk_musicid_query_handle_t	musicid_query_handle,
	gnsdk_cstr_t					option_key,
	gnsdk_cstr_t*					p_option_value
	);

/** GNSDK_MUSICID_OPTION_ENABLE_CLASSICAL_DATA
  * Summary:
  *   Indicates whether a response should include any associated classical
  *   music data.                                                         
*/

#define GNSDK_MUSICID_OPTION_ENABLE_CLASSICAL_DATA	"gnsdk_musicid_option_enable_classical"
/** GNSDK_MUSICID_OPTION_ENABLE_MEDIAVOCS_DATA
  * Summary:
  *   Indicates whether a response should include any associated MediaVOCS
  *   data.                                                               
*/
#define GNSDK_MUSICID_OPTION_ENABLE_MEDIAVOCS_DATA	"gnsdk_musicid_option_enable_mvocs"
/** GNSDK_MUSICID_OPTION_ENABLE_DSP_DATA
  * Summary:
  *   Indicates whether a response should include any associated sonic
  *   attribute (DSP) data.                                           
*/
#define GNSDK_MUSICID_OPTION_ENABLE_DSP_DATA		"gnsdk_musicid_option_enable_dsp"
/** GNSDK_MUSICID_OPTION_ENABLE_PLAYLIST
  * Summary:
  *   Indicates whether a response should include associated attribute data
  *   for GNSDK Playlist.                                                  
*/
#define GNSDK_MUSICID_OPTION_ENABLE_PLAYLIST		"gnsdk_musicid_option_enable_playlist"
/** GNSDK_MUSICID_OPTION_ENABLE_LINK_DATA
  * Summary:
  *   Indicates whether a response should include any Link data (third-party
  *   metadata).                                                            
*/
#define GNSDK_MUSICID_OPTION_ENABLE_LINK_DATA		"gnsdk_musicid_option_enable_link"

/** GNSDK_MUSICID_OPTION_PREFERRED_LANG
  * Summary:
  *   Indicates the preferred language of the returned results.
*/
#define GNSDK_MUSICID_OPTION_PREFERRED_LANG			"gnsdk_musicid_preferred_lang"
/** GNSDK_MUSICID_OPTION_RESULT_SINGLE
  * Summary:
  *   Indicates whether a response must return only the single best result.
*/
#define GNSDK_MUSICID_OPTION_RESULT_SINGLE			"gnsdk_musicid_single_result"
/** GNSDK_MUSICID_OPTION_RANGE_START
  * Summary:
  *   Indicates whether a response must return a range of results that begin
  *   at a specified value.                                                 
*/
#define GNSDK_MUSICID_OPTION_RESULT_RANGE_START		"gnsdk_musicid_result_range_start"
/** GNSDK_MUSICID_OPTION_RANGE_SIZE
  * Summary:
  *   Indicates whether a response can return up to a specified maximum number
  *   \of results.                                                            
*/
#define GNSDK_MUSICID_OPTION_RESULT_RANGE_SIZE		"gnsdk_musicid_result_range_size"
/** GNSDK_MUSICID_OPTION_REVISION_CHECK
  * Summary:
  *   Indicates whether a response must return results only when an updated
  *   revision exists.                                                     
*/
#define GNSDK_MUSICID_OPTION_REVISION_CHECK			"gnsdk_musicid_option_revision_check"
/** GNSDK_MUSICID_OPTION_QUERY_NOCACHE
  * Summary:
  *   Indicates whether a response is retrieved from data existing in the
  *   lookup cache.                                                      
*/
#define GNSDK_MUSICID_OPTION_QUERY_NOCACHE			"gnsdk_musicid_option_query_nocache"
/** GNSDK_MUSICID_OPTION_VALUE_TRUE
  * Summary:
  *   Specifies that an option must be enabled.
*/
#define GNSDK_MUSICID_OPTION_VALUE_TRUE				"true"
/** GNSDK_MUSICID_OPTION_VALUE_FALSE
  * Summary:
  *   Specifies that an option must not be enabled.
*/
#define GNSDK_MUSICID_OPTION_VALUE_FALSE			"false"

/******************************************************************************
 * MID Query APIs
 ******************************************************************************/


/** gnsdk_musicid_query_find_albums
  * Summary:
  *   Performs a MusicID query for album results.
  * Parameters:
  *   musicid_query_handle:  [in] MusicID query handle to perform query with
  *   p_match_type:          [out] Pointer to receive the match type value
  *   p_response_gdo:        [out] Pointer to receive the GDO handle with the
  *                          album results
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_query_find_albums(
	gnsdk_musicid_query_handle_t	musicid_query_handle,
	gnsdk_musicid_match_type_t*		p_match_type,
	gnsdk_gdo_handle_t*				p_response_gdo
	);

/** gnsdk_musicid_query_find_tracks
  * Summary:
  *   Performs a MusicID query for track results.
  * Parameters:
  *   musicid_query_handle:  [in] MusicID query handle to perform query with
  *   p_match_type:          [out] Pointer to receive the match type value
  *   p_response_gdo:        [out] Pointer to receive the GDO handle with the
  *                          track results
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_query_find_tracks(
	gnsdk_musicid_query_handle_t	musicid_query_handle,
	gnsdk_musicid_match_type_t*		p_match_type,
	gnsdk_gdo_handle_t*				p_response_gdo
	);

/** gnsdk_musicid_query_find_lyrics
  * Summary:
  *   Performs a MusicID query for lyric results.
  * Parameters:
  *   musicid_query_handle:  [in] MusicID query handle to perform query with
  *   p_match_type:          [out] Pointer to receive the match type value
  *   p_response_gdo:        [out] Pointer to receive the GDO handle with the
  *                          lyric results
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_query_find_lyrics(
	gnsdk_musicid_query_handle_t	musicid_query_handle,
	gnsdk_musicid_match_type_t*		p_match_type,
	gnsdk_gdo_handle_t*				p_response_gdo
	);

/** gnsdk_musicid_query_resolve
  * Summary:
  *   Selects an individual result GDO from a set of multiple matching
  *   response GDOs.
  * Parameters:
  *   musicid_query_handle:  [in] MusicID query handle that was previously
  *                          queried
  *   match_ordinal:         [in] 1\-based ordinal of result to return
  *   response_gdo:          [in] Response GDO from previous MusicID query
  *   p_result_gdo:          [out] Pointer to receive result GDO
  *                                                                       
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_query_resolve(
	gnsdk_musicid_query_handle_t	musicid_query_handle,
	gnsdk_gdo_handle_t				response_gdo,
	gnsdk_uint32_t					match_ordinal,
	gnsdk_gdo_handle_t*				p_result_gdo
	);



#ifdef __cplusplus
}
#endif

#endif /* _GNSDK_MUSICID_H_ */

