/** Gracenote SDK: Link public header file
  * Author:
  *   Copyright (c) 2012 Gracenote, Inc.
  *   
  *   This software may not be used in any way or distributed without
  *   permission. All rights reserved.
  *   
  *   Some code herein may be covered by US and international patents.
*/

#ifndef _GNSDK_LINK_H_
#define _GNSDK_LINK_H_

#ifdef __cplusplus
extern "C"{
#endif

/******************************************************************************
 * Typdefs
 ******************************************************************************/

/** <title gnsdk_link_query_handle_t>
  * <toctitle gnsdk_link_query_handle_t>
  * 
  * gnsdk_link_query_handle_t
  * Summary:
  *   Handle to a Link query. Created gnsdk_link_query_create the application
  *   must create this handle to perform any Link queries it needs to execute.
*/
GNSDK_DECLARE_HANDLE( gnsdk_link_query_handle_t );


/** gnsdk_link_content_type_t
  * Summary:
  *   Indicates available content types that can be retrieved for Albums,
  *   Lyrics, Tracks, or Video Products, Contributors, Works, Seasons, or
  *   Series (or some combination of these object types). Not all content
  *   types are available for all objects.                               
*/
typedef enum
{
	/** \ \ 
	* Summary:
	*   Indicates an invalid content type.
	*/
	gnsdk_link_content_unknown = 0,
	/** \ \ 
	  * Summary:
	  *   Retrieves cover artwork; this is Album-level and Video Product-level
	  *   data.
	  *   
	  *   Use with GNSDK_GDO_CONTEXT_ALBUM and GNSDK_GDO_CONTEXT_VIDEO_PRODUCT.
	  *   
	  *   Do not use with GNSDK_GDO_CONTEXT_VIDEO_WORK,
	  *   GNSDK_GDO_CONTEXT_VIDEO_SERIES, GNSDK_GDO_CONTEXT_VIDEO_SEASON, or
	  *   GNSDK_GDO_CONTEXT_CONTRIBUTOR.                                       
	*/
	gnsdk_link_content_cover_art,
	/** \ \ 
	  * Summary:
	  *   Retrieves artwork for the object's primary genre; this is Album-level
	  *   and Track-level data.                                                
	*/
	gnsdk_link_content_genre_art,
	/** \ \ 
	  * Summary:
	  *   Retrieves review content for the object; this is Album-level data.
	*/
	gnsdk_link_content_review,
	/** \ \ 
	  * Summary:
	  *   Retrieves biography content for artist; this is Album-level data.
	*/
	gnsdk_link_content_biography,
	/** \ \ 
	  * Summary:
	  *   Retrieves news content for the artist; this is Album-level data.
	*/
	gnsdk_link_content_artist_news,
	/** \ \ 
	  * Summary:
	  *   Retrieves Lyric content in XML form; this is Lyric-level and Track-level
	  *   data.                                                                   
	*/
	gnsdk_link_content_lyric_xml,
	/** \ \ 
	  * Summary:
	  *   Retrieves Lyric content in plain text form; this is Lyric-level and
	  *   Track-level data.                                                  
	*/
	gnsdk_link_content_lyric_text,
	/** \ \ 
	  * Summary:
	  *   Retrieves custom non-Gracenote identifier for an object; this is
	  *   Album-level, Track-level, and Video Product-level data.         
	*/
	gnsdk_link_content_external_id,
	/** \ \ 
	  * Summary:
	  *   Retrieves a custom non-Gracenote identifier for a Track; this is
	  *   Track-level data.                                               
	*/	
	gnsdk_link_content_track_external_id,
	/** \ \ 
	  * Summary:
	  *   Retrieves cover artwork identifier; this is Album-level and Video
	  *   Product-level data.
	  *   
	  *   Use with GNSDK_GDO_CONTEXT_ALBUM and GNSDK_GDO_CONTEXT_VIDEO_PRODUCT.
	  *   
	  *   Do not use with GNSDK_GDO_CONTEXT_VIDEO_WORK,
	  *   GNSDK_GDO_CONTEXT_VIDEO_SERIES, GNSDK_GDO_CONTEXT_VIDEO_SEASON, or
	  *   GNSDK_GDO_CONTEXT_CONTRIBUTOR.                                       
	*/
	gnsdk_link_content_cover_art_id,
	/** Summary:
	  *   Retrieves artwork for the object's primary genre; this is Album-level
	  *   and Track-level data.                                                
	*/
	gnsdk_link_content_genre_art_id,
	/** \ \ 
	  * Summary:
	  *   Retrieves review identifier for the object; this is Album-level data.
	*/
	gnsdk_link_content_review_id,
	/** \ \ 
	  * Summary:
	  *   Retrieves biography identifier for artist; this is Album-level data.
	*/
	gnsdk_link_content_biography_id,
	/** \ \ 
	  * Summary:
	  *   Retrieves news identifier for artist; this is Album-level data.
	*/
	gnsdk_link_content_artist_news_id,
	/** \ \ 
	  * Summary:
	  *   Retrieves a Lyric identifier; this is Lyric-level and Track-level data.
	*/
	gnsdk_link_content_lyric_id,
	/** \ \ 
	  * Summary:
	  *   Retrieves DSP content; this is Track-level data.
	*/
	gnsdk_link_content_dsp_data,
	/** \ \ 
	  * Summary:
	  *   Retrieves DSP content identifier for media; this is Track-level data.
	*/
	gnsdk_link_content_dsp_data_id,
	/** \ \ 
	  * Summary:
	  *   Retrieves listener comments; this is Album-level data.
	*/
	gnsdk_link_content_comments_listener,
	/** \ \ 
	  * Summary:
	  *   Retrieves listener comments identifier; this is Album-level data.
	*/
	gnsdk_link_content_comments_listener_id,
	/** \ \ 
	  * Summary:
	  *   Retrieves new release comments; this is Album-level data.
	*/
	gnsdk_link_content_comments_release,
	/** \ \ 
	  * Summary:
	  *   Retrieves new release comments identifier; this is Album-level data.
	*/
	gnsdk_link_content_comments_release_id,
	/** \ \ 
	  * Summary:
	  *   Retrieves news for the object; this is Album-level data.
	*/
	gnsdk_link_content_news,
	/** \ \ 
	  * Summary:
	  *   Retrieves news identifier for the object; this is Album-level data.
	*/
	gnsdk_link_content_news_id,
	/** \ \ 
	  * Summary:
	  *   Retrieves a content image for specific Video contexts.
	  *   
	  *   Use with GNSDK_GDO_CONTEXT_VIDEO_WORK, GNSDK_GDO_CONTEXT_VIDEO_SERIES,
	  *   GNSDK_GDO_CONTEXT_VIDEO_SEASON, and GNSDK_GDO_CONTEXT_CONTRIBUTOR.
	  *   
	  *   Do not use with GNSDK_GDO_CONTEXT_VIDEO_PRODUCT.
	  *   
	  *   \Note: At the time of this guide's publication, support for Video
	  *   Explore Seasons and Series image retrieval through Seasons, Series, and
	  *   Works queries is limited. Contact your Gracenote Professional Services
	  *   representative for updates on the latest supported images.             
	*/
	gnsdk_link_content_image,	
	/** \ \ 
	  * Summary:
	  *   Retrieves a content image identifier for specific Video contexts.
	  *   
	  *   Use with GNSDK_GDO_CONTEXT_VIDEO_WORK, GNSDK_GDO_CONTEXT_VIDEO_SERIES,
	  *   GNSDK_GDO_CONTEXT_VIDEO_SEASON, and GNSDK_GDO_CONTEXT_CONTRIBUTOR.
	  *   
	  *   Do not use with GNSDK_GDO_CONTEXT_VIDEO_PRODUCT.
	  *   
	  *   \Note: At the time of this guide's publication, support for Video
	  *   Explore Seasons and Series image retrieval through Seasons, Series, and
	  *   Works queries is limited. Contact your Gracenote Professional Services
	  *   representative for updates on the latest supported images.             
	*/
	gnsdk_link_content_image_id,	

	/** \ \ 
	  * Summary:
	  *   Retrieves an artist image for Album contexts.
	  *   
	  *   Use with GNSDK_GDO_CONTEXT_ALBUM.
	  *   
	  *   \Note: At the time of this guide's publication, the available Album
	  *   artist images are limited. Contact your Gracenote Professional Services
	  *   representative for updates on the latest supported images.             
	*/
	gnsdk_link_content_image_artist,	
	/** \ \ 
	  * Summary:
	  *   Retrieves an artist image identifier for Album contexts.
	  *   
	  *   Use with GNSDK_GDO_CONTEXT_ALBUM.
	  *   
	  *   \Note: At the time of this guide's publication, the available Album
	  *   artist images are limited. Contact your Gracenote Professional Services
	  *   representative for updates on the latest supported images.             
	*/
	gnsdk_link_content_image_artist_id	

} gnsdk_link_content_type_t;


/** gnsdk_link_data_type_t
  * Summary:
  *   Indicates possible data formats for the retrieved content.
*/
typedef enum
{
	/** \ \ 
	* Summary:
	*   Indicates an invalid data type.
	*/
	gnsdk_link_data_unknown		= 0,
	/** \ \ 
	* Summary:
	*   Indicates the content buffer contains plain text data (null terminated).
	*/
	gnsdk_link_data_text_plain	= 1,
	/** \ \ 
	* Summary:
	*   Indicates the content buffer contains XML data (null terminated).
	*/
	gnsdk_link_data_text_xml	= 2,
	/** \ \ 
	* Summary:
	*   Indicates the content buffer contains a numerical value
	*   (gnsdk_uint32_t). Unused.                                     
	*/
	gnsdk_link_data_number		= 10,
	/** \ \ 
	* Summary:
	*   Indicates the content buffer contains jpeg image data.
	*/
	gnsdk_link_data_image_jpeg	= 20,
	/** \ \ 
	* Summary:
	*   Indicates the content buffer contains externally defined binary data.
	*/
	gnsdk_link_data_binary		= 100

} gnsdk_link_data_type_t;


/** gnsdk_link_query_status_t
  * Summary:
  *   Indicates the Link callback function status values.
*/
typedef enum
{
	/** \ \ 
	* Summary:
	*   Indicates an invalid status. This value should never be received.
	*/
	gnsdk_link_query_status_unknown = 0,
	/** \ \ 
	* Summary:
	*   Indicates a new Link query handle has been created.
	*/
	gnsdk_link_query_status_begin,
	/** \ \ 
	* Summary:
	*   Connecting to the Gracenote Service.   
	*/
	gnsdk_link_query_status_connecting,
	/** \ \ 
	* Summary:
	*   Encompasses redirects, sending headers and requests.
	*/
	gnsdk_link_query_status_sending,
	/** \ \ 
	* Summary:
	*   Encompasses receiving headers and responses.
	*/
	gnsdk_link_query_status_receiving,
	/** \ \ 
	* Summary:
	*   Indicates a Link query handle has been deleted.
	*/
	gnsdk_link_query_status_query_complete

} gnsdk_link_query_status_t;

/** gnsdk_link_callback_fn
  * Summary:
  *   Link callback function for status updates as content is retrieved.
  * Parameters:
  *   user_data:          [in] Pointer to data passed in to
  *                       gnsdk_link_query_create function through the
  *                       callback_userdata parameter. This pointer must be
  *                       cast from the gnsdk_void_t type to its original type
  *                       to be accessed properly.
  *   link_query_handle:  [in] Link query handle that the callback operates on
  *   status:             [in] One of gnsdk_link_query_status_t values
  *   bytes_done:         [in] Current number of bytes transferred. Set to a
  *                       value greater than 0 to indicate progress, or 0 to
  *                       indicate no progress.
  *   bytes_total:        [in] Total number of bytes to be transferred. Set to
  *                       a value greater than 0 to indicate progress, or 0 to
  *                       indicate no progress.
  *   p_abort:            [out] Set dereferenced value to GNSDK_TRUE to abort
  *                       the operation that calls the callback
  *                                                                           
*/
typedef gnsdk_void_t
(GNSDK_CALLBACK_API *gnsdk_link_callback_fn)(
	const gnsdk_void_t*			user_data,
	gnsdk_link_query_handle_t	link_query_handle,
	gnsdk_link_query_status_t	status,
	gnsdk_size_t				bytes_done,
	gnsdk_size_t				bytes_total,
	gnsdk_bool_t*				p_abort
	);

/** gnsdk_link_initialize
  * Summary:
  *   Initializes the GNSDK Link library.
  * Parameters:
  *   sdkmgr:  [in] Handle from successful gnsdk_manager_initialize call
  *                                                                        
*/
gnsdk_error_t GNSDK_API
gnsdk_link_initialize(
	gnsdk_manager_handle_t sdkmgr
	);

/** gnsdk_link_shutdown
  * Summary:
  *   Shuts down the Link library.
*/
gnsdk_error_t GNSDK_API
gnsdk_link_shutdown(void);

/** gnsdk_link_get_version
  * Summary:
  *   Retrieves the Link SDK's version string.
*/
gnsdk_cstr_t GNSDK_API
gnsdk_link_get_version(void);

/** gnsdk_link_get_build_date
  * Summary:
  *   Retrieves Link SDK's build date string.
*/
gnsdk_cstr_t GNSDK_API
gnsdk_link_get_build_date(void);


/******************************************************************************
 * Link Query Handle - for the life of the query
 ******************************************************************************/

/** gnsdk_link_query_create
  * Summary:
  *   Creates a Link query handle.
  * Parameters:
  *   user_handle:          [in] User handle for the user requesting the query
  *   callback_fn:          [in_opt] Callback function for status and progress
  *   callback_userdata:    [in_opt] Data that is passed back through calls to
  *                         the callback function
  *   p_link_query_handle:  [out] Pointer to receive the Link query handle
  *                                                                           
*/
gnsdk_error_t GNSDK_API
gnsdk_link_query_create(
	gnsdk_user_handle_t			user_handle,
	gnsdk_link_callback_fn		callback_fn,
	const gnsdk_void_t*			callback_userdata,
	gnsdk_link_query_handle_t*	p_link_query_handle
	);

/** gnsdk_link_query_release
  * Summary:
  *   Invalidates and releases resources for a given Link query handle.
  * Parameters:
  *   link_query_handle:  [in] Link query handle to release
  *                                                                    
*/
gnsdk_error_t GNSDK_API
gnsdk_link_query_release(
	gnsdk_link_query_handle_t	link_query_handle
	);

/******************************************************************************
 * Link Query Input - each Link query needs one input - GDO.
 ******************************************************************************/

/** gnsdk_link_query_set_gdo
  * Summary:
  *   Sets a result GDO as input for a Link query.
  * Parameters:
  *   link_query_handle:  [in] Link query handle this GDO applies to
  *   input_gdo:          [in] Handle to GDO
  *                                                                 
*/
gnsdk_error_t GNSDK_API
gnsdk_link_query_set_gdo(
	gnsdk_link_query_handle_t	link_query_handle,
	gnsdk_gdo_handle_t			input_gdo
	);


/** gnsdk_link_query_option_set
  * Summary:
  *   Sets an option for a given Link query handle.
  * Parameters:
  *   link_query_handle:  [in] Link query handle this option applies to
  *   option_key:         [in] An option from the available <link !!MACROS_link_query_option_keys, Query Option Keys>
  *   option_value:       [in] Value to set for given option
  *                                                                                                                  
*/
gnsdk_error_t GNSDK_API
gnsdk_link_query_option_set(
	gnsdk_link_query_handle_t	link_query_handle,
	gnsdk_cstr_t				option_key,
	gnsdk_cstr_t				option_value
	);

/** gnsdk_link_query_option_get
  * Summary:
  *   Retrieves an option for a given Link query handle.
  * Parameters:
  *   link_query_handle:  [in] Link query handle to retrieve option from
  *   option_key:         [in] An option from the available <link !!MACROS_link_query_option_values, Query Option Values>
  *   p_option_value:     [out] Pointer to value set for given option
  *                                                                                                                      
*/
gnsdk_error_t GNSDK_API
gnsdk_link_query_option_get(
	gnsdk_link_query_handle_t	link_query_handle,
	gnsdk_cstr_t				option_key,
	gnsdk_cstr_t*				p_option_value
	);

/** gnsdk_link_query_options_clear
  * Summary:
  *   Clears all options currently set for a given Link query handle.
  * Parameters:
  *   link_query_handle:  [in] Link query handle to clear options for
  *                                                                  
*/
gnsdk_error_t GNSDK_API
gnsdk_link_query_options_clear(
	gnsdk_link_query_handle_t	link_query_handle
	);

/** LINK_OPTION_KEY_IMAGE_SIZE
  * Summary:
  *   Indicates the image size being requested; use with a
  *   LINK_OPTION_VALUE_IMAGE_SIZE_* value.               
*/
#define LINK_OPTION_KEY_IMAGE_SIZE				"gnsdk_link_imagesize"
/** LINK_OPTION_KEY_TRACK_ORD
  * Summary:
  *   Explicitly identifies the track of interest by its ordinal number. This
  *   \option takes precedence over any provided track indicator in the GDO. 
*/
#define LINK_OPTION_KEY_TRACK_ORD				"gnsdk_link_trackord"
/** LINK_OPTION_KEY_DATASOURCE
* Summary:
*   Indicates the source provider of the content (for example, "Acme").
*/
#define LINK_OPTION_KEY_DATASOURCE				"gnsdk_link_datasource"
/** LINK_OPTION_KEY_DATATYPE
* Summary:
*   Indicates the type of the provider content (for example, "cover").
*/
#define LINK_OPTION_KEY_DATATYPE				"gnsdk_link_datatype"


/** LINK_OPTION_VALUE_IMAGE_SIZE_THUMBNAIL
  * Summary:
  *   Retrieves a thumbnail-sized image. Use with the
  *   LINK_OPTION_KEY_IMAGE_SIZE option key.         
*/
#define LINK_OPTION_VALUE_IMAGE_SIZE_THUMBNAIL	"thumbnail"	
/** LINK_OPTION_VALUE_IMAGE_SIZE_SMALL
  * Summary:
  *   Retrieves a small-sized image. Use with the LINK_OPTION_KEY_IMAGE_SIZE
  *   \option key.                                                          
*/
#define LINK_OPTION_VALUE_IMAGE_SIZE_SMALL		"small"
/** LINK_OPTION_VALUE_IMAGE_SIZE_MEDIUM
  * Summary:
  *   Retrieves a medium-sized image. Use with the LINK_OPTION_KEY_IMAGE_SIZE
  *   \option key.                                                           
*/
#define LINK_OPTION_VALUE_IMAGE_SIZE_MEDIUM		"medium"
/** LINK_OPTION_VALUE_IMAGE_SIZE_LARGE
  * Summary:
  *   Retrieves a large-sized image. Use with the LINK_OPTION_KEY_IMAGE_SIZE
  *   \option key.                                                          
*/
#define LINK_OPTION_VALUE_IMAGE_SIZE_LARGE		"large"
/** LINK_OPTION_VALUE_IMAGE_SIZE_XLARGE
  * Summary:
  *   Retrieves an extra large-sized image. Use with the
  *   LINK_OPTION_KEY_IMAGE_SIZE option key.            
*/
#define LINK_OPTION_VALUE_IMAGE_SIZE_XLARGE		"xlarge"

/******************************************************************************
 * Link Data Retrieval
 ******************************************************************************/

/** gnsdk_link_query_content_count
  * Summary:
  *   Retrieves count for the specified content that is described by the Link
  *   query handle options and content type flag.
  * Parameters:
  *   link_query_handle:  [in] Link query handle to retrieve content for
  *   content_type:       [in] <link gnsdk_link_content_type_t, Type of content>
  *                       to count
  *   p_count:            [out] Pointer to receive content count
  *                                                                             
*/
gnsdk_error_t GNSDK_API
gnsdk_link_query_content_count(
	gnsdk_link_query_handle_t	link_query_handle,
	gnsdk_link_content_type_t	content_type,
	gnsdk_uint32_t*				p_count
	);

/** gnsdk_link_query_content_info
  * Summary:
  *   Retrieves information about the specified content that is described by
  *   the Link query handle options and data type flag.
  * Parameters:
  *   link_query_handle:  [in] Link query handle to retrieve content for
  *   content_type:       [in] An available <link gnsdk_link_content_type_t, content type>
  *                       to retrieve information for
  *   ordinal:            [in] N'th item of content_type to retrieve
  *                       information for
  *   p_datasource_val:   [out] Pointer to receive content data source
  *   p_datasource_type:  [out] Pointer to receive content data type
  *                                                                                       
*/
gnsdk_error_t GNSDK_API
gnsdk_link_query_content_info(
	gnsdk_link_query_handle_t	link_query_handle,
	gnsdk_link_content_type_t	content_type,
	gnsdk_uint32_t				ordinal,
	gnsdk_cstr_t*				p_datasource_val,
	gnsdk_cstr_t*				p_datasource_type
	);

/** gnsdk_link_query_content_retrieve
  * Summary:
  *   Retrieves content data that is described by the Link query handle
  *   \options and data type flag.
  * Parameters:
  *   link_query_handle:   [in] Link query handle to retrieve content for
  *   content_type:        [in] Type of content to request. When retrieving
  *                        album or video Products artwork, request <link gnsdk_link_content_type_t, cover art>.
  *                        When retrieving Contributors, Works, Seasons, or
  *                        Series content, request an <link gnsdk_link_content_type_t, image>.
  *   ordinal:             [in] N'th item of content_type to retrieve
  *   p_buffer_data_type:  [out] Pointer to receive the <link gnsdk_link_data_type_t, content data type>
  *   p_buffer:            [out] Pointer to receive the buffer that contains
  *                        the requested content
  *   p_buffer_size:       [out] Pointer to receive the memory size pointed to
  *                        by p_buffer
  *                                                                                                             
*/
gnsdk_error_t GNSDK_API
gnsdk_link_query_content_retrieve(
	gnsdk_link_query_handle_t	link_query_handle,
	gnsdk_link_content_type_t	content_type,
	gnsdk_uint32_t				ordinal,
	gnsdk_link_data_type_t*		p_buffer_data_type,
	gnsdk_byte_t**				p_buffer,
	gnsdk_size_t*				p_buffer_size
	);


/** gnsdk_link_query_content_free
  * Summary:
  *   Releases resources used for content data from
  *   gnsdk_list_query_content_retrieve.
  * Parameters:
  *   buffer:  [in] Content buffer returned from
  *            gnsdk_list_query_content_retrieve
  *                                                
*/
gnsdk_error_t GNSDK_API
gnsdk_link_query_content_free(
	gnsdk_byte_t*				buffer
	);



#ifdef __cplusplus
}
#endif

#endif /* _GNSDK_LINK_H_ */
