/** Gracenote SDK: Playlist public header file
  * Author:
  *   Copyright (c) 2012 Gracenote, Inc.
  *   
  *   This software may not be used in any way or distributed without
  *   permission. All rights reserved.
  *   
  *   Some code herein may be covered by US and international patents.
*/

#ifndef _GNSDK_PLAYLIST_H_
/** gnsdk_playlist.h: Primary interface for the Playlist SDK.
*/
#define _GNSDK_PLAYLIST_H_

#ifdef __cplusplus
extern "C"{
#endif


/** <title gnsdk_playlist_collection_handle_t>
  * <toctitle gnsdk_playlist_collection_handle_t>
  * <flag PL>
  * 
  * gnsdk_playlist_collection_handle_t
  * Summary:
  *   Handle to a Playlist collection summary.
  *   
  *   The application must release this handle using
  *   gnsdk_playlist_collection_release.            
*/
GNSDK_DECLARE_HANDLE( gnsdk_playlist_collection_handle_t );
/** <title gnsdk_playlist_results_handle_t>
  * <toctitle gnsdk_playlist_results_handle_t>
  * <flag PL>
  * 
  * gnsdk_playlist_results_handle_t
  * Summary:
  *   Handle to Playlist results.
  *   
  *   The application must release this handle using
  *   gnsdk_playlist_results_release.               
*/
GNSDK_DECLARE_HANDLE( gnsdk_playlist_results_handle_t );


/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_status_t
  * Summary:
  *   Indicates the Playlist callback function status values.
*/
typedef enum
{
	gnsdk_playlist_status_unknown	= 0, /** <flag PL>
	                             	       * <unfinished>
	                             	       * 
	                             	       * gnsdk_playlist_status_t@1::gnsdk_playlist_status_unknown
	                             	       * Summary:
	                             	       *   Indicates a Playlist status value of unknown, the default state.
	                             	     */

	gnsdk_playlist_status_new_ident	= 10,
	gnsdk_playlist_status_old_ident	= 11
	

} gnsdk_playlist_status_t;

/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_update_callback_fn
  * Summary:
  *   Receive Playlist processing status updates.
  * Parameters:
  *   callback_userdata:  [in_opt] Data that is passed back through calls to
  *                       the callback function
  *   h_collection:       [in] Handle to a collection
  *   ident:              [in] Media identifier
  *   status:             [in] One of gnsdk_playlist_status_t values
  *   pb_abort:           [out] Set dereferenced value to GNSDK_TRUE to abort
  *                       the operation that calls the callback
  *                                                                          
*/
typedef gnsdk_void_t		(GNSDK_CALLBACK_API *gnsdk_playlist_update_callback_fn)(
	const gnsdk_void_t*					callback_userdata,
	gnsdk_playlist_collection_handle_t	h_collection, 
	gnsdk_cstr_t						ident,
	gnsdk_playlist_status_t				status,
	gnsdk_bool_t*						pb_abort
	);
#if 0
{
	/* gnsdk_playlist_status_t':
	**    new_media:			application should add media to collection
	**	  existing_media:		application can update or ignore this media
	**	  non_existing_media:	application should remove this media from collection
	*/
}
#endif



/** <flag PL>
  * 
  * gnsdk_playlist_initialize
  * Summary:
  *   Initializes the GNSDK Playlist library.
  * Parameters:
  *   sdkmgr_handle:  [in] Handle from a successful gnsdk_manager_initialize
  *                   call
  *                                                                         
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_initialize(gnsdk_manager_handle_t sdkmgr_handle);
/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_shutdown
  * Summary:
  *   Shuts down and releases resources for the Playlist library.
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_shutdown(void);

/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_get_version
  * Summary:
  *   Retrieves the Playlist library's version string.
*/
gnsdk_cstr_t GNSDK_API
gnsdk_playlist_get_version(void);
/** <flag PL>
  * 
  * gnsdk_playlist_get_build_date
  * Summary:
  *   Retrieves the Playlist library's build date string.
*/
gnsdk_cstr_t GNSDK_API
gnsdk_playlist_get_build_date(void);


/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_attributes_count
  * Summary:
  *   Retrieves the total number of registered attributes for use in the
  *   Playlist system.
  * Parameters:
  *   p_count:  [out] Pointer to receive total number of registered Playlist
  *             attributes
  *                                                                         
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_attributes_count(
	gnsdk_uint32_t* p_count
	);

/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_attributes_enum
  * Summary:
  *   Retrieves  a <i>n</i>'th registered Playlist attribute.
  * Parameters:
  *   index:             [in] Index of Playlist attribute to retrieve
  *   p_attribute_name:  [out] Pointer to receive attribute name string
  *                                                                    
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_attributes_enum(
	gnsdk_uint32_t	index,
	gnsdk_cstr_t*	p_attribute_name
	);

/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_collection_create
  * Summary:
  *   Creates a new, empty Playlist Collection Summary.
  * Parameters:
  *   collection_name:  [in] String for a collection summary name as a valid
  *                     UTF\-8 text string
  *   ph_collection:    [out] Pointer to a collection handle
  *                                                                         
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_collection_create(
	gnsdk_cstr_t						collection_name,
	gnsdk_playlist_collection_handle_t*	ph_collection
	);

/** <unfinished>
  * 
  * gnsdk_playlist_collection_get_name
  * Summary:
  *   Retrieves a Playlist Collection Summary name.
  * Parameters:
  *   h_collection:       [in] Handle to a collection summary
  *   p_collection_name:  [out] Pointer to a collection summary name
  *                                                                 
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_collection_get_name(
	gnsdk_playlist_collection_handle_t	h_collection,
	gnsdk_cstr_t*						p_collection_name
	);

/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_collection_update_name
  * Summary:
  *   Changes a Playlist Collection Summary name.
  * Parameters:
  *   h_collection:     [in] Handle to a collection 
  *   collection_name:  [in] Updated collection name
  *                                                 
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_collection_update_name(
	gnsdk_playlist_collection_handle_t	h_collection,
	gnsdk_cstr_t						collection_name
	);

/** <unfinished>
  * 
  * gnsdk_playlist_collection_release
  * Summary:
  *   Invalidates and releases resources for a given Playlist Collection
  *   \Summary.
  * Parameters:
  *   h_collection:  [in] Handle to a collection 
  *                                                                     
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_collection_release(
	gnsdk_playlist_collection_handle_t	h_collection
	);




/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_collection_ident_add
  * Summary:
  *   Adds an identifier plus related metadata to a Collection Summary.
  * Parameters:
  *   h_collection:  [in] Handle to a collection summary
  *   ident:         [in] Valid UTF\-8 string of media identifier data provided
  *                  by the application; must be unique within the collection
  *                  \summary
  *   media_gdo:     [in] Handle to a media GDO, such as an Album or Track GDO
  *                                                                            
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_collection_ident_add(
	gnsdk_playlist_collection_handle_t	h_collection,
	gnsdk_cstr_t						ident,
	gnsdk_gdo_handle_t					media_gdo
	);

/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_collection_ident_exists
  * Summary:
  *   Retrieves a boolean value indicating if an identifier exists for a
  *   Playlist Collection Summary.
  * Parameters:
  *   h_collection:      [in] Handle to a collection summary
  *   ident:             [in] String of identifier data
  *   pb_ident_present:  [out] Pointer to receive boolean value indicating if
  *                      the identifier is associated with the collection
  *                      \summary
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_collection_ident_exists(
	gnsdk_playlist_collection_handle_t	h_collection,
	gnsdk_cstr_t						ident,
	gnsdk_bool_t*						pb_ident_present
	);

/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_collection_ident_count
  * Summary:
  *   Retrieves the total number of identifiers associated with a Playlist
  *   Collection Summary.
  * Parameters:
  *   h_collection:  [in] Handle to a collection summary
  *   p_count:       [out] Pointer to receive the total number of identifiers
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_collection_ident_count(
	gnsdk_playlist_collection_handle_t	h_collection,
	gnsdk_uint32_t*						p_count
	);

/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_collection_ident_enum
  * Summary:
  *   Retrieves a <i>n</i>'th identifier from a Playlist Collection Summary.
  * Parameters:
  *   h_collection:  [in] Handle to a collection summary
  *   index:         [in] Index of identifier to retrieve
  *   p_ident:       [out] Pointer to receive a identifier
  *                                                                         
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_collection_ident_enum(
	gnsdk_playlist_collection_handle_t	h_collection,
	gnsdk_uint32_t						index,
	gnsdk_cstr_t*						p_ident
	);

/** <flag PL>
  * <unfinished>
  * 
  * \ \ 
  * Summary:
  *   Retrieves attribute data for the specified identifier.
  * Parameters:
  *   h_collection:  [in] Handle to a Playlist collection summary
  *   ident:         [in] String of identifier data
  *   ph_gdo_ident:  [out] Pointer to receive a GDO identifier handle
  *                                                                  
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_collection_ident_get_gdo(
	gnsdk_playlist_collection_handle_t	h_collection,
	gnsdk_cstr_t						ident,
	gnsdk_gdo_handle_t*					ph_gdo_ident
	);


/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_collection_ident_remove
  * Summary:
  *   Removes an identifier from a Playlist Collection Summary.
  * Parameters:
  *   h_collection:  [in] Handle to a Playlist collection summary
  *   ident:         [in] String of identifier data
  *                                                              
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_collection_ident_remove(
	gnsdk_playlist_collection_handle_t	h_collection,
	gnsdk_cstr_t						ident
	);
	

/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_collection_sync_ident_add
  * Summary:
  *   Indicate an identifier is still physically present when setting up
  *   synchronization of a Collection Summary.
  * Parameters:
  *   h_collection:  [in] Handle to a Playlist collection summary
  *   ident:         [in] Identifier string
  *                                                                     
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_collection_sync_ident_add(
	gnsdk_playlist_collection_handle_t	h_collection,
	gnsdk_cstr_t						ident
	);

/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_collection_sync_process
  * Summary:
  *   Synchronizes a Playlist Collection Summary with Gracenote Service data.
  * Parameters:
  *   h_collection:       [in] Handle to a collection summary
  *   callback_fn:        [in] Callback function for information on identifiers
  *                       to add or remove (not optional)
  *   callback_userdata:  [in_opt] Data that is passed back through calls to
  *                       the callback function
  *                                                                            
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_collection_sync_process(
	gnsdk_playlist_collection_handle_t	h_collection,
	gnsdk_playlist_update_callback_fn	callback_fn,
	const gnsdk_void_t*					callback_userdata
	);


/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_collection_serialize_size
  * Summary:
  *   \Returns an estimated size of buffer required to hold a serialized
  *   representation of the collection. The size is guaranteed to be large
  *   enough, but may be slightly larger than required.
  * Parameters:
  *   h_collection:  [in] Handle to a collection
  *   p_size:        [out] Pointer to receive collection buffer size
  *                                                                       
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_collection_serialize_size(
	gnsdk_playlist_collection_handle_t	h_collection,
	gnsdk_size_t*						p_size
	);

/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_collection_serialize
  * Summary:
  *   Outputs the Playlist Collection Summary to a buffer.
  * Parameters:
  *   h_collection:      [in] Handle to a collection
  *   p_collection_buf:  [in] Pointer to buffer to received serialized data
  *   p_buf_size:        [in/out] Size of buffer pointed to by p_collection_buf
  *                      , and number of bytes written to p_collection_buf on
  *                      successful serialization
  *                                                                            
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_collection_serialize(
	gnsdk_playlist_collection_handle_t	h_collection,
	gnsdk_byte_t*						p_collection_buf,
	gnsdk_size_t*						p_buf_size
	);

/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_collection_deserialize
  * Summary:
  *   Reconstitutes a Playlist Collection Summary from serialized collection
  *   \summary data.
  * Parameters:
  *   p_collection_buf:  [in] Pointer to the serialized collection summary data
  *   buf_size:          [in] Size of serialized data buffer pointed to by
  *                      p_collection_buf
  *   ph_collection:     [out] Pointer to receive a collection summary handle
  *                                                                            
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_collection_deserialize(
	gnsdk_byte_t*						p_collection_buf,
	gnsdk_size_t						buf_size,
	gnsdk_playlist_collection_handle_t*	ph_collection
	);

/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_collection_attributes_count
  * Summary:
  *   Retrieves the total number of attributes utilized by a collection
  *   \summary.
  * Parameters:
  *   h_collection:  [in] Handle to a collection summary
  *   p_count:       [out] Pointer to receive total number of attributes for a
  *                  collection summary
  *                                                                           
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_collection_attributes_count(
	gnsdk_playlist_collection_handle_t	h_collection,
	gnsdk_uint32_t*						p_count
	);

/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_collection_attributes_enum
  * Summary:
  *   Retrieves a <i>n</i>'th registered attribute specific to collection
  *   \summary.
  * Parameters:
  *   h_collection:      [in] Handle to a collection summary
  *   index:             [in] Index of collection summary attribute to retrieve
  *   p_attribute_name:  [out] Pointer to receive attribute name string
  *                                                                            
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_collection_attributes_enum(
	gnsdk_playlist_collection_handle_t	h_collection,
	gnsdk_uint32_t						index,
	gnsdk_cstr_t*						p_attribute_name
	);

/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_storage_store_collection
  * Summary:
  *   Saves a Playlist Collection Summary to local storage.
  * Parameters:
  *   h_collection:  [in] Handle to a collection summary
  *                                                        
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_storage_store_collection(
	gnsdk_playlist_collection_handle_t	h_collection
	);

/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_storage_load_collection
  * Summary:
  *   Loads a Playlist Collection Summary from local storage into memory for
  *   use.
  * Parameters:
  *   collection_name:  [in] Collection summary to load into memory
  *   ph_collection:    [out] Pointer to receive a collection summary handle
  *                                                                         
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_storage_load_collection(
	gnsdk_cstr_t						collection_name,
	gnsdk_playlist_collection_handle_t*	ph_collection
	);

/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_storage_count_collections
  * Summary:
  *   Retrieves the total number of Playlist Collection Summaries available in
  *   storage.
  * Parameters:
  *   p_collection_count:  [out] Pointer to receive count of stored collection
  *                        summaries
  *                                                                           
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_storage_count_collections(
	gnsdk_uint32_t*						p_collection_count
	);

/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_storage_enum_collections
  * Summary:
  *   Retrieves a <i>n</i>'th Playlist Collection Summary from storage.
  * Parameters:
  *   index:                [in] Index to specific collection summary
  *   collection_name_buf:  [in] Pointer to buffer to receive collection name
  *   buf_size:             [in] Size of collection to retrieve from buffer
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_storage_enum_collections(
	gnsdk_uint32_t						index,
	gnsdk_char_t*						collection_name_buf,
	gnsdk_size_t						buf_size
	);

/** <flag P: PL>
  * <unfinished>
  * 
  * gnsdk_playlist_storage_remove_collection
  * Summary:
  *   Permanently removes a Playlist collection summary from local storage.
  * Parameters:
  *   collection_name:  [in] Collection summary to delete from storage
  *                                                                        
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_storage_remove_collection(
	gnsdk_cstr_t						collection_name
	);

/** <flag PL>
  * <keywords PDL statement validation, playlist generation/PDL statement validation>
  * <unfinished>
  * 
  * gnsdk_playlist_statement_validate
  * Summary:
  *   Validates a Playlist Definition Language (PDL) statement.
  * Parameters:
  *   pdl_statement:     [in] PDL statement string
  *   h_collection:      [in] Handle to a Playlist collection summary
  *   pb_seed_required:  [out] Pointer to receive a boolean value indicating if
  *                      a Playlist Seed is required
  *                                                                                  
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_statement_validate(
	gnsdk_cstr_t						pdl_statement,
	gnsdk_playlist_collection_handle_t	h_collection,
	gnsdk_bool_t*						pb_seed_required
	);

/** <flag PL>
  * <keywords playlist generation/general function>
  * <unfinished>
  * 
  * gnsdk_playlist_generate_playlist
  * Summary:
  *   Generates a Playlist.
  * Parameters:
  *   pdl_statement:  [in] PDL statement string
  *   h_collection:   [in] Handle to a Playlist collection summary
  *   h_gdo_seed:     [in] Handle to a GDO Seed (if necessary)
  *   ph_results:     [out] Pointer to receive generated Playlist results
  *                   handle
  *   user_handle:    [in] Handle to a user for the user requesting the
  *                   playlist
  *                                                                      
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_generate_playlist(
	gnsdk_user_handle_t					user_handle,
	gnsdk_cstr_t						pdl_statement,
	gnsdk_playlist_collection_handle_t	h_collection,
	gnsdk_gdo_handle_t					h_gdo_seed,
	gnsdk_playlist_results_handle_t*	ph_results
	);

/** <keywords More Like This, playlist generation/More Like This function>
  * <unfinished>
  * 
  * gnsdk_playlist_generate_morelikethis
  * Summary:
  *   Generates a Playlist using Gracenote's More Like This algorithm and GDO
  *   Seed input.
  * Parameters:
  *   user_handle:   [in] Handle to a user for the user requesting the More Like
  *                  This playlist
  *   h_collection:  [in] Handle to a Playlist collection summary
  *   h_gdo_seed:    [in] Handle to a GDO Seed (required)
  *   ph_results:    [out] Pointer to receive generated Playlist results handle
  *                                                                             
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_generate_morelikethis(
	gnsdk_user_handle_t					user_handle,
	gnsdk_playlist_collection_handle_t	h_collection,
	gnsdk_gdo_handle_t					h_gdo_seed,
	gnsdk_playlist_results_handle_t*	ph_results
	);

/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_results_count
  * Summary:
  *   Retrieves the total number of results of a generated Playlist.
  * Parameters:
  *   h_results:  [in] Handle to Playlist results
  *   p_count:    [out] Pointer to receive the total number of generated
  *               results
  *                                                                     
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_results_count(
	gnsdk_playlist_results_handle_t		h_results,
	gnsdk_uint32_t*						p_count
	);

/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_results_enum
  * Summary:
  *   Retrieves the n'th identifier of a result in a generated Playlist
  * Parameters:
  *   h_results:  [in] Handle to Playlist results
  *   index:      [in] Index of result to retrieve
  *   p_ident:    [out] Pointer to receive resulting identifier
  *                                                                    
*/
gnsdk_error_t GNSDK_API
gnsdk_playlist_results_enum(
	gnsdk_playlist_results_handle_t		h_results,
	gnsdk_uint32_t						index,
	gnsdk_cstr_t*						p_ident
	);

/** <flag PL>
  * <unfinished>
  * 
  * gnsdk_playlist_results_release
  * Summary:
  *   Invalidates and releases resources for a given Playlist's results.
  * Parameters:
  *   h_results:  [in] Handle to Playlist results
  *                                                                     
*/

gnsdk_error_t GNSDK_API
gnsdk_playlist_results_release(
	gnsdk_playlist_results_handle_t		h_results
	);



#ifdef __cplusplus
}
#endif

#endif /* _GNSDK_PLAYLIST_H_ */

