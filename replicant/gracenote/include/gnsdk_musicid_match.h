/** Gracenote SDK MusicID Match public header file
  * Author:
  *   Copyright (c) 2012 Gracenote, Inc.
  *
  *   This software may not be used in any way or distributed without
  *   permission. All rights reserved.
  *
  *   Some code herein may be covered by US and international patents.
*/

#ifndef _GNSDK_MUSICID_MATCH_H_
/** 
 * 
 * Gracenote MusicID Match provides audio recognition features to compare a 
 * user's music collection against a set of known tracks in another collection, 
 * such as a music library used for cloud-based music services.
 * MusicID Match implements a two-phased approach:
 * <ol>
 * <li>Generate a set of track identifiers from a base music library using Cantametrix fingerprinting.
 * <li>Generate full-length fingerprints (microFAPI and nanoFAPI) for a users music collection, 
 *		and compare the corresponding track identifiers to the base music library. 
 * </ol>
 * Possible return values are:
 * <ul>
 *  <li> Exact: Compared files are an exact match.
 *  <li> NoMatch: Compared files do not match.
 *  <li> NonExist: Gracenote Service does not have a corresponding external ID for the comparison.
 *  <li> Undefined: Not a comparison query. Lookup queries will always return a value of Undefined.
 * </ul>
 */
#define _GNSDK_MUSICID_MATCH_H_

#ifdef __cplusplus
extern "C"{
#endif

/******************************************************************************
 * Typdefs
 ******************************************************************************/

/** gnsdk_musicid_match_query_handle_t
  * 
  *   Handle to MusicID Match query. Created by gnsdk_musicid_match_query_create API, the
  *   application must create this handle to perform any MusicID Match queries it
  *   wishes to execute.
*/
GNSDK_DECLARE_HANDLE( gnsdk_musicid_match_query_handle_t );

/** gnsdk_musicid_match_status_t
  * 
  *   MusicID Match callback function status values
*/
typedef enum
{
	/** gnsdk_musicid_match_status_unknown
	 * 
	 *   MusicID Match query status is unknown.
	 */
	gnsdk_musicid_match_status_unknown = 0,
	
	/** gnsdk_musicid_match_query_begin
	 * 
	 *   MusicID Match query is starting.
	 */
	gnsdk_musicid_match_status_query_begin = 10,	
	
	/** gnsdk_musicid_match_status_connecting
	 * 
	 *   MusicID Match query is starting a network connection
	 */
	gnsdk_musicid_match_status_connecting = 20,	

	/** gnsdk_musicid_match_status_sending
	 * 
	 *   MusicID Match is sending query data.
	 */
	gnsdk_musicid_match_status_sending = 30,

	/** gnsdk_musicid_match_status_receiving
	 * 
	 *   MusicID Match is receiving query data.
	 */	
	gnsdk_musicid_match_status_receiving = 40,
	
	/** 
	 * 
	 *   MusicID Match query handle is completing the query process.
	 */
	gnsdk_musicid_match_status_query_complete = 100, 
	
	/** gnsdk_musicid_match_status_query_delete
	 * 
	 *   MusicID Match query handle is about to be deleted.      
	 */	
	gnsdk_musicid_match_status_query_delete = 999 } gnsdk_musicid_match_status_t;

	
/** gnsdk_musicid_match_callback_fn
  * 
  *   MusicID Match callback function for status updates as content is retrieved.
  * 
  * @param user_data	[in] Pointer to data passed in to gnsdk_musicid_match_query_create 
  *                        function through the callback_userdata parameter. 
  *                        You must cast this pointer from the gnsdk_void_t type to its original 
  *                        type to be accessed properly. 
  * @param musicid_match_query_handle	[in] The query handle that the callback operates on.
  * @param status	[in] One of gnsdk_musicid_match_query_status_t values. 
  * @param bytes_done	[in] Current number of bytes transferred. Set to a value greater than 0 to 
  *                         indicate progress, or 0 to indicate no progress. 
  * @param bytes_total	[in] Total number of bytes to be transferred. 
  *                           Set to a value greater than 0 to indicate progress, or 0 to indicate no progress. 
  * @param p_abort	[out] Set dereferenced value to GNSDK_TRUE to abort the operation that calls the callback.
  *
*/
typedef gnsdk_void_t
(GNSDK_CALLBACK_API *gnsdk_musicid_match_callback_fn)(
	gnsdk_void_t*						user_data,
	gnsdk_musicid_match_query_handle_t	musicid_match_query_handle,
	gnsdk_musicid_match_status_t		status,
	gnsdk_size_t						bytes_done,
	gnsdk_size_t						bytes_total,
	gnsdk_bool_t*						p_abort
	);

/** gnsdk_musicid_match_initialize
  * 
  *   Initialize the Gracenote MusicID Match SDK.
  * 
  *  @param	sdkmgr	Handle from successful gnsdk_sdkmanager_initialize call
  *
*/

gnsdk_error_t GNSDK_API
gnsdk_musicid_match_initialize(
	gnsdk_manager_handle_t sdkmgr
	);

/** gnsdk_musicid_match_shutdown
  * 
  *   Shut down the MusicID Match SDK.
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_match_shutdown(void);

/** gnsdk_musicid_match_get_version
  * 
  *   Retrieve the MusicID Match SDK's version string.
*/
gnsdk_cstr_t GNSDK_API
gnsdk_musicid_match_get_version(void);

/** gnsdk_musicid_match_get_build_date
  * 
  *   Retrieve MusicID Match SDK's build date string.
*/
gnsdk_cstr_t GNSDK_API
gnsdk_musicid_match_get_build_date(void);


/******************************************************************************
 * MusicID Match Query Handle - for the life of the query
 ******************************************************************************/

/** gnsdk_musicid_match_query_create
  * 
  *   Create a MusicID Match query handle.
  * 
  *   @param client_handle	Client handle for the client requesting the query
  *   @param user_handle	User handle for the user requesting the query
  *   @param callback_fn	(Optional) Callback function for status and
  *                         progress
  *   @param callback_userdata	(Optional) Data that is passed back through calls
  *                         to the callback function
  *  @param  p_musicid_match_query_handle	Pointer to receive the MusicID Match query handle
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_match_query_create(
	gnsdk_user_handle_t			user_handle,
	gnsdk_musicid_match_callback_fn		callback_fn,
	gnsdk_void_t*				callback_userdata,
	gnsdk_musicid_match_query_handle_t*	p_musicid_match_query_handle
	);

/** gnsdk_musicid_match_query_release
  * 
  *   Invalidate and release resources for a given MusicID Match query handle.
  * 
  *  @param  musicid_match_query_handle	  MusicID Match query handle to release
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_match_query_release(
	gnsdk_musicid_match_query_handle_t	musicid_match_query_handle
	);
	
/******************************************************************************
 * 
 ******************************************************************************/

/** gnsdk_musicid_match_query_set_id_datasource
  * 
  *   Set the id data source for which the entire query operates in . This determines the id context of the 
  *   query response.
  * 
  *  @param  musicid_match_query_handle		MusicID Match query handle this lookup request applies to
  *	 @param  id_source						the id source for which the results will be returned. 
*/

gnsdk_error_t GNSDK_API
gnsdk_musicid_match_query_set_id_datasource(
	gnsdk_musicid_match_query_handle_t	musicid_match_query_handle,
	gnsdk_cstr_t						id_source
	);



/** gnsdk_musicid_match_query_set_lookup_fp
  * 
  *   Set fingerprint data for a lookup request for a MusicID Match query.
  * 
  *  @param  musicid_match_query_handle		MusicID Match query handle this lookup request applies to
  *  @param  ident								[in] identity string that must be unique for a query.
  *	 @param  fp_data							[in] String representation of fingerprint data, 
  *													must be a cantametrix fingerprint.
*/

gnsdk_error_t GNSDK_API
gnsdk_musicid_match_query_set_lookup_fp(
	gnsdk_musicid_match_query_handle_t	musicid_match_query_handle,
	gnsdk_cstr_t						ident,
	gnsdk_cstr_t						fp_data
	);


/** gnsdk_musicid_match_query_set_compare_fp
  * 
  *		Set fingerprint data for a compare request for a MusicID Match query.
  * 
  *  @param  musicid_match_query_handle		MusicID Match query handle this compare request applies to
  *  @param  ident								[in] identity string that must be unique for a query.
  *  @param  fp_data							[in] String representation of fingerprint data, 
  *													must be a Phillips macro fingerprint. 
  * 
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_match_query_set_compare_fp(
	gnsdk_musicid_match_query_handle_t	musicid_match_query_handle,
	gnsdk_cstr_t						ident,
	gnsdk_cstr_t						fp_data
	);


/** gnsdk_musicid_match_query_set_compare_data
  * 
  *		Set id data for a compare request for a MusicID Match query.  
  * 
  *  @param  musicid_match_query_handle		MusicID Match query handle this compare request applies to
  *  @param  ident								[in] identity string that corresponds to the ident and fingerprint data set  
  *													with gnsdk_musicid_match_query_set_compare_fp
  *	 @param id_data								[in] the id data which has to be compared.
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_match_query_set_compare_data(
	gnsdk_musicid_match_query_handle_t	musicid_match_query_handle,
	gnsdk_cstr_t						ident,
	gnsdk_cstr_t						id_data
	);



/** gnsdk_musicid_match_query_option_set
  * 
  *   Set an option for a given MusicID Match query handle.
  * 
  *  @param  musicid_match_query_handle	  MusicID Match query handle this option applies to
  *  @param  option_key					  An option from the available MusicID Match option list.
  *  @param  option_value				  Value to set for given option
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_match_query_option_set(
	gnsdk_musicid_match_query_handle_t	musicid_match_query_handle,
	gnsdk_cstr_t						option_key,
	gnsdk_cstr_t						option_value
	);


/** gnsdk_musicid_match_query_option_get
  * 
  *   Retrieve an option for a given MusicID Match query handle.
  * 
  *  @param  musicid_match_query_handle	   MusicID Match query handle to retrieve option from
  *  @param  option_key					   An option from the available MusicID Match option list.
  *  @param  p_option_value				   Pointer to value set for given option
  *
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_match_query_option_get(
	gnsdk_musicid_match_query_handle_t	musicid_match_query_handle,
	gnsdk_cstr_t						option_key,
	gnsdk_cstr_t*						p_option_value
	);



/** gnsdk_musicid_match_query_find_matches
  * 
  *   Performs a MusicID Match query for the added lookup and compare requests.
  * 
  *   @param musicid_match_query_handle		[in] MusicID Match query handle to perform the query with
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_match_query_find_matches(
	gnsdk_musicid_match_query_handle_t	musicid_match_query_handle
	);


/** gnsdk_musicid_match_query_get_response
  * 
  *   Gets the result gdo for a given ident in a MusicID Match query.
  * 
  *   @param musicid_match_query_handle		[in] MusicID Match query handle to perform query with
  *   @param ident								[in] ident used for adding requests to the query	                                               
  *   @param p_response_gdo					[out] response gdo
*/
gnsdk_error_t GNSDK_API
gnsdk_musicid_match_query_get_response(
	gnsdk_musicid_match_query_handle_t	musicid_match_query_handle,
	gnsdk_cstr_t						ident,
	gnsdk_gdo_handle_t*				p_response_gdo
	);



#ifdef __cplusplus
}
#endif

#endif /*_GNSDK_MUSICID_MATCH_H_*/
