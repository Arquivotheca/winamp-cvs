/** Gracenote SDK: VideoID public header file
  * Author:
  *   Copyright (c) 2012 Gracenote, Inc.
  *   
  *   This software may not be used in any way or distributed without
  *   permission. All rights reserved.
  *   
  *   Some code herein may be covered by US and international patents.
*/

#ifndef _GNSDK_VIDEO_H_
/** gnsdk_video.h: primary interface for the VideoID SDK
*/
#define _GNSDK_VIDEO_H_

#ifdef __cplusplus
extern "C"{
#endif

/*
 * gnsdk_video.h:	Interface for VideoID SDK.
 */

/******************************************************************************
 * Typdefs
 ******************************************************************************/

/** <title gnsdk_video_query_handle_t>
  * <toctitle gnsdk_video_query_handle_t>
  * 
  * gnsdk_video_query_handle_t
  * Summary:
  *   The VideoID query handle.          
*/
GNSDK_DECLARE_HANDLE( gnsdk_video_query_handle_t );

/** gnsdk_video_match_type_t
  * Summary:
  *   Indicates a Video match type value.
*/
typedef enum
{
	gnsdk_video_match_type_unknown = 0,/** gnsdk_video_match_type_t@1::gnsdk_video_match_type_unknown
	                                       * Summary:
	                                       *   Match type of unknown, the default state.                   
	                                     */
	

	gnsdk_video_match_type_none,			/** gnsdk_video_match_type_t@1::gnsdk_video_match_type_none
	                              			  * Summary:
	                              			  *   No matches; the submitted identifier (TOC or TUI) has no matching
	                              			  *   metadata records.                                                
	                              			*/
	gnsdk_video_match_type_single_exact,	/** gnsdk_video_match_type_t@1::gnsdk_video_match_type_single_exact
	                                      	  * Summary:
	                                      	  *   One exact match; the submitted TOC exactly matches only one metadata
	                                      	  *   record.                                                             
	                                      	*/
	gnsdk_video_match_type_multi_exact,	/** gnsdk_video_match_type_t@1::gnsdk_video_match_type_multi_exact
	                                     	  * Summary:
	                                     	  *   More than one exact match; the submitted TOC exactly matches more than
	                                     	  *   \one metadata record.                                                 
	                                     	*/
	gnsdk_video_match_type_fuzzy,			/** gnsdk_video_match_type_t@1::gnsdk_video_match_type_fuzzy
	                               			  * Summary:
	                               			  *   One or more fuzzy matches; the submitted TOC closely resembles one or
	                               			  *   more metadata records.                                               
	                               			*/
	gnsdk_video_match_type_partial,		/** gnsdk_video_match_type_t@1::gnsdk_video_match_type_partial
	                                 		  * Summary:
	                                 		  *   One or more partial matches on the disc's features. The exact submitted
	                                 		  *   TOC is not found in the Gracenote Service, but certain disc feature(s)
	                                 		  *   are found on another disc or discs; for example, a disc for a movie that
	                                 		  *   has a different set of extra features.                                  
	                                 		*/
	gnsdk_video_match_type_aggressive,	/** gnsdk_video_match_type_t@1::gnsdk_video_match_type_aggressive
	                                    	  * Summary:
	                                    	  *   One or more partial matches, based on heuristic matching. This
	                                    	  *   additional match type is for cases when nothing is found in the
	                                    	  *   Gracenote Service for the exact submitted TOC, so algorithms are run to
	                                    	  *   find similar likely products; for example, a disc that is a foreign
	                                    	  *   language release of a similar product in the Gracenote Service.        
	                                    	*/
	gnsdk_video_match_type_latest_revision/** gnsdk_video_match_type_t@1::gnsdk_video_match_type_latest_revision
	                                          * Summary:
	                                          *   Latest revision match                                               
	                                        */
	

} gnsdk_video_match_type_t;

/** gnsdk_video_status_t
  * Summary:
  *   The status value of the current query, passed to
  *   gnsdk_video_callback_fn.                        
*/
typedef enum
{
	gnsdk_video_status_unknown = 0, /** gnsdk_video_status_t@1::gnsdk_video_status_unknown
	                                    * Summary:
	                                    *   Indicates a VideoID status value of unknown, the default state.
	                                  */
	
	gnsdk_video_status_query_begin = 10,/** TODO:
	  *   need more info; summary just repeats name. should the value ( "10",
	  *   "100", etc.) be discussed?                                         
	*/
	
	gnsdk_video_status_connecting = 20,/** gnsdk_video_status_t@1::gnsdk_video_status_connecting
	                                 * Summary:
	                                 *   Indicates a VideoID new query status value.           
	                               */
	
	gnsdk_video_status_sending = 30,/** gnsdk_video_status_t@1::gnsdk_video_status_sending
	                               * Summary:
	                               *   Indicates a VideoID sending status value.           
	                             */
	
	gnsdk_video_status_receiving = 40,/** gnsdk_video_status_t@1::gnsdk_video_status_receiving
	                                 * Summary:
	                                 *   Indicates a VideoID receiving status value.           
	                               */
	
	gnsdk_video_status_query_complete = 100, /** TODO:
	  *   need more info; summary just repeats name. should the value ( "10",
	  *   "100", etc.) be discussed?                                         
	*/
	

	gnsdk_video_status_query_delete = 999 /** gnsdk_video_status_t@1::gnsdk_video_status_query_delete
	                                    * Summary:
	                                    *   Indicates a VideoID query delete status value.           
	                                  */
	

} gnsdk_video_status_t;

/** gnsdk_video_search_type_t
  * Summary:
  *   The type of text search that is used to find results.
*/
typedef enum
{
	gnsdk_video_search_type_unknown = 0,	/** gnsdk_video_search_type_t@1::gnsdk_video_search_type_unknown
	                                      	  * Summary:
	                                      	  *   Unknown search type; the default state.                       
	                                      	*/
	gnsdk_video_search_type_anchored,		/** gnsdk_video_search_type_t@1::gnsdk_video_search_type_anchored
	                                   		  * Summary:
	                                   		  *   Anchored text search, used for product title and suggestion searches.
	                                   		  *   Retrieves results that begin with the same characters as exactly
	                                   		  *   specified; for example, entering <i>dar</i>, <i>dark</i>, <i>dark k</i>,
	                                   		  *   \or <i>dark kni</i> retrieves the title <i>Dark Knight,</i> but entering<i>
	                                   		  *   knight</i> will not retrieve<i> Dark Knight</i>. Note that this search
	                                   		  *   type recognizes both partial and full words.                               
	                                   		*/
	gnsdk_video_search_type_default		/** gnsdk_video_search_type_t@1::gnsdk_video_search_type_default
	                                 		  * Summary:
	                                 		  *   Normal keyword filter search for contributor, product, and work title
	                                 		  *   searches; for example, a search using a keyword of <i>dark</i>, <i>knight</i>,
	                                 		  *   \or <i>dark knight </i>retrieves the title <i>Dark Knight</i>. Note that
	                                 		  *   this search type recognizes only full words, not partial words; this
	                                 		  *   means that entering only <i>dar</i> for <i>dark</i> is not recognized.        
	                                 		*/
	                           			
} gnsdk_video_search_type_t;



/** gnsdk_video_callback_fn
  * Summary:
  *   Receive status updates as Video data is retrieved.
  * Parameters:
  *   callback_userdata:   [in] Pointer to data passed in to the
  *                        gnsdk_video_query_create function through the
  *                        callback_userdata parameter. This pointer must be
  *                        cast from the gnsdk_void_t type to its original type
  *                        to be accessed properly.
  *   video_query_handle:  [in] Video query handle that the callback operates
  *                        on
  *   status:              [in] One of gnsdk_video_status_t values
  *   bytes_done:          [in] Current number of bytes transferred. Set to a
  *                        value greater than 0 to indicate progress, or 0 to
  *                        indicate no progress.
  *   bytes_total:         [in] Total number of bytes to be transferred. Set to
  *                        a value greater than 0 to indicate progress, or 0 to
  *                        indicate no progress.
  *   p_abort:             [out] Set dereferenced value to GNSDK_TRUE to abort
  *                        the operation that is calling the callback
  *                                                                            
*/
typedef gnsdk_void_t
(GNSDK_CALLBACK_API *gnsdk_video_callback_fn)(
	const gnsdk_void_t*			callback_userdata,
	gnsdk_video_query_handle_t	video_query_handle,
	gnsdk_video_status_t		status,
	gnsdk_size_t				bytes_done,
	gnsdk_size_t				bytes_total,
	gnsdk_bool_t*				p_abort
	);

/** gnsdk_video_initialize
  * Summary:
  *   Initializes the Gracenote VideoID and Video Explore library.
  * Parameters:
  *   sdkmgr_handle:  [in] Handle from a successful gnsdk_manager_initialize
  *                   call
  *                                                                            
*/
gnsdk_error_t GNSDK_API
gnsdk_video_initialize(
	gnsdk_manager_handle_t sdkmgr_handle
	);

/** gnsdk_video_shutdown
  * Summary:
  *   Shuts down and release resources for the VideoID and Video Explore
  *   library.                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_video_shutdown(void);

/** gnsdk_video_get_version
  * Summary:
  *   Retrieves the VideoID and Video Explore library's version string.
*/
gnsdk_cstr_t GNSDK_API
gnsdk_video_get_version(void);

/** gnsdk_video_get_build_date
  * Summary:
  *   Retrieves the VideoID and Video Explore library's build date string.
*/
gnsdk_cstr_t GNSDK_API
gnsdk_video_get_build_date(void);

/******************************************************************************
 * VideoID Query Handle - for the life of the query
 ******************************************************************************/

/** gnsdk_video_query_create
  * Summary:
  *   Creates a VideoID or Video Explore query handle.
  * Parameters:
  *   user_handle:             [in] User handle for the user requesting the
  *                            query
  *   callback_fn:             [in_opt] Callback function for status and
  *                            progress
  *   callback_userdata:       [in_opt] Data that is passed back through calls
  *                            to the callback functions
  *   p_video_query_handle:  [out] Pointer to receive the VideoID or Video
  *                            Explore query handle
  *                                                                            
*/
gnsdk_error_t GNSDK_API
gnsdk_video_query_create(
	gnsdk_user_handle_t			user_handle,
	gnsdk_video_callback_fn		callback_fn,
	const gnsdk_void_t*			callback_userdata,
	gnsdk_video_query_handle_t*	p_video_query_handle
	);

/** gnsdk_video_query_release
  * Summary:
  *   Invalidates and releases resources for a given VideoID or Video Explore
  *   query handle.
  * Parameters:
  *   video_query_handle:  [in] VideoID or Video Explore query handle to
  *                          release
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_video_query_release(
	gnsdk_video_query_handle_t	video_query_handle
	);

/** gnsdk_video_query_set_toc_string
  * Summary:
  *   Sets a video disc TOC to enable querying the applicable VideoID query
  *   handle.
  * Parameters:
  *   video_query_handle:  [in] VideoID query handle this video disc TOC
  *                        applies to
  *   toc_string:          [in] Externally produced video TOC string
  *   toc_string_flags:    [in] <link !!MACROS_vid_toc_flags, TOC flags> for
  *                        video TOC query modifiers
  *                                                                         
*/
gnsdk_error_t GNSDK_API
gnsdk_video_query_set_toc_string(
	gnsdk_video_query_handle_t	video_query_handle,
	gnsdk_cstr_t				toc_string,
	gnsdk_uint32_t				toc_string_flags
	);

/** GNSDK_VIDEO_TOC_FLAG_DEFAULT
  * Summary:
  *   Generally recommended flag to use when setting a video TOC.
*/
#define	GNSDK_VIDEO_TOC_FLAG_DEFAULT			0x00
/** GNSDK_VIDEO_TOC_FLAG_PAL
  * Summary:
  *   Flag to indicate a given simple video TOC is from a PAL disc.
*/
#define	GNSDK_VIDEO_TOC_FLAG_PAL				0x10
/** GNSDK_VIDEO_TOC_FLAG_ANGLES
  * Summary:
  *   Flag to indicate a given simple video TOC contains angle data.
*/
#define	GNSDK_VIDEO_TOC_FLAG_ANGLES				0x20

/** gnsdk_video_query_set_external_id
  * Summary:
  *   Sets a video's external ID string information used to query a VideoID or
  *   Video Explore query handle.
  * Parameters:
  *   video_query_handle:  [in] VideoID or Video Explore query handle this ID
  *                          applies to
  *   external_id:           [in] External ID string
  *   external_id_type:      [in_opt] External ID type (such as a <link GNSDK_VIDEO_EXTERNAL_ID_TYPE_UPC, UPC>)
  *   external_id_source:    [in_opt] External ID source (such as a <link GNSDK_VIDEO_EXTERNAL_ID_SOURCE_DEFAULT, custom external ID source>)
  *                                                                                                                                            
*/
gnsdk_error_t GNSDK_API
gnsdk_video_query_set_external_id(
	gnsdk_video_query_handle_t	video_query_handle,
	gnsdk_cstr_t				external_id,
	gnsdk_cstr_t				external_id_type,
	gnsdk_cstr_t				external_id_source
	);

/** GNSDK_VIDEO_EXTERNAL_ID_TYPE_UPC
  * Summary:
  *   Sets a Universal Product Code (UPC) value as an external ID for a
  *   Products query.                                                  
*/
#define	GNSDK_VIDEO_EXTERNAL_ID_TYPE_UPC			"gnsdk_video_xid_type_upc"

/** GNSDK_VIDEO_EXTERNAL_ID_TYPE_ISAN
  * Summary:
  *   Sets a International Standard Audiovisual Number (ISAN) code as an
  *   external ID for a Works query.                                    
*/
#define	GNSDK_VIDEO_EXTERNAL_ID_TYPE_ISAN			"gnsdk_video_xid_type_isan"

/** GNSDK_VIDEO_EXTERNAL_ID_SOURCE_DEFAULT
  * Summary:
  *   Sets the default external ID source.  
*/
#define	GNSDK_VIDEO_EXTERNAL_ID_SOURCE_DEFAULT		"gnsdk_video_xid_source_gn"



/** gnsdk_video_query_set_filter_by_list_element
  * Summary:
  *   Sets a filter for a Video Explore query handle, using a value from a
  *   list.
  * Parameters:
  *   video_query_handle:  [in] Video Explore query handle this filter applies
  *                        to
  *   filter_key:          [in] One of the video <link !!MACROS_vid_filter_keys, filter keys>
  *   list_element:        [in] A list element handle used to populate the
  *                        filter value. The list element must be from a list
  *                        that corresponds to the filter name.
  *                                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_video_query_set_filter_by_list_element(
	gnsdk_video_query_handle_t	video_query_handle,
	gnsdk_cstr_t				filter_key,
	gnsdk_list_element_handle_t	list_element
	);

/** gnsdk_video_query_set_filter
  * Summary:
  *   Sets a filter for a VideoID or Video Explore query handle.
  * Parameters:
  *   video_query_handle:  [in] VideoID or Video Explore query handle this
  *                        filter applies to
  *   filter_key:          [in] One of the available <link !!MACROS_vid_filter_keys, filter keys>
  *   filter_value:        [in] String value for corresponding data key,
  *                        generally one of the available <link !!MACROS_vid_filter_values_genre, Genre>
  *                        or <link !!MACROS_vid_filter_values_prod, Production Type>
  *                        values
  *                                                                                                     
*/
gnsdk_error_t GNSDK_API
gnsdk_video_query_set_filter(
	gnsdk_video_query_handle_t	video_query_handle,
	gnsdk_cstr_t				filter_key,
	gnsdk_cstr_t				filter_value
	);




/** GNSDK_VIDEO_FILTER_KEY_SEASON_NUM
  * Summary:
  *   Filter for Season numbers; not list-based.
*/
#define GNSDK_VIDEO_FILTER_KEY_SEASON_NUM					"gnsdk_video_key_season_num"
/** GNSDK_VIDEO_FILTER_KEY_SEASON_EPISODE_NUM
  * Summary:
  *   Filter for season episode numbers; not list-based.
*/
#define GNSDK_VIDEO_FILTER_KEY_SEASON_EPISODE_NUM			"gnsdk_video_key_season_episode_num"
/** GNSDK_VIDEO_FILTER_KEY_SERIES_EPISODE_NUM
  * Summary:
  *   Filter for series episode numbers; not list-based.
*/
#define GNSDK_VIDEO_FILTER_KEY_SERIES_EPISODE_NUM			"gnsdk_video_key_series_episode_num"

/** GNSDK_VIDEO_FILTER_KEY_GENRE_INCLUDE
  * Summary:
  *   List-based filter to include genres.
*/

#define GNSDK_VIDEO_FILTER_KEY_GENRE_INCLUDE				"gnsdk_video_key_genre_include"
/** GNSDK_VIDEO_FILTER_KEY_GENRE_EXCLUDE
  * Summary:
  *   List-based filter to exclude genres.
*/
#define GNSDK_VIDEO_FILTER_KEY_GENRE_EXCLUDE				"gnsdk_video_key_genre_exclude"
/** GNSDK_VIDEO_FILTER_KEY_PRODUCTION_TYPE_INCLUDE
  * Summary:
  *   List-based filter to include production types.
*/
#define GNSDK_VIDEO_FILTER_KEY_PRODUCTION_TYPE_INCLUDE		"gnsdk_video_key_production_type_include"
/** GNSDK_VIDEO_FILTER_KEY_PRODUCTION_TYPE_EXCLUDE
  * Summary:
  *   List-based filter to exclude production types.
*/
#define GNSDK_VIDEO_FILTER_KEY_PRODUCTION_TYPE_EXCLUDE		"gnsdk_video_key_production_type_exclude"
/** GNSDK_VIDEO_FILTER_KEY_SERIAL_TYPE_INCLUDE
  * Summary:
  *   List-based filter to include serial types.
*/
#define GNSDK_VIDEO_FILTER_KEY_SERIAL_TYPE_INCLUDE			"gnsdk_video_key_serial_type_include"
/** GNSDK_VIDEO_FILTER_KEY_SERIAL_TYPE_EXCLUDE
  * Summary:
  *   List-based filter to exclude serial types.
*/
#define GNSDK_VIDEO_FILTER_KEY_SERIAL_TYPE_EXCLUDE			"gnsdk_video_key_serial_type_exclude"
/** GNSDK_VIDEO_FILTER_KEY_ORIGIN_INCLUDE
  * Summary:
  *   List-based filter to include origin.
*/
#define GNSDK_VIDEO_FILTER_KEY_ORIGIN_INCLUDE				"gnsdk_video_key_origin_include"
/** GNSDK_VIDEO_FILTER_KEY_ORIGIN_EXCLUDE
  * Summary:
  *   List-based filter to exclude origin.
*/
#define GNSDK_VIDEO_FILTER_KEY_ORIGIN_EXCLUDE				"gnsdk_video_key_origin_exclude"


/*!ignore me! Production type filter values*/


/** GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_ANIMATION
  * Summary:
  *   Filter on all production types classified at and under the top-level category: <i>Animation</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_ANIMATION              GNSDK_LIST_PRODUCTION_TYPE_ANIMATION                                     
/** 
  * 
  * GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_DOCUMENTARY
  * Summary:
  *   Filter on all production types classified at and under the top-level category: <i>Documentary</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_DOCUMENTARY            GNSDK_LIST_PRODUCTION_TYPE_DOCUMENTARY                     
/** 
  * 
  * GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_EDUCATIONAL
  * Summary:
  *   Filter on all production types classified at and under the top-level category: <i>Educational</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_EDUCATIONAL            GNSDK_LIST_PRODUCTION_TYPE_EDUCATIONAL                     
/** 
  * 
  * GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_GAME_SHOW
  * Summary:
  *   Filter on all production types classified at and under the top-level category: <i>Game Show</i> 
*/
#define GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_GAME_SHOW              GNSDK_LIST_PRODUCTION_TYPE_GAME_SHOW                       
/** 
  * 
  * GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_INSTRUCTIONAL
  * Summary:
  *   Filter on all production types classified at and under the top-level category: <i>Instructional</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_INSTRUCTIONAL          GNSDK_LIST_PRODUCTION_TYPE_INSTRUCTIONAL                   
/** 
  * 
  * GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_KARAOKE
  * Summary:
  *   Filter on all production types classified at and under the top-level category: <i>Karaoke</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_KARAOKE	            GNSDK_LIST_PRODUCTION_TYPE_KARAOKE	                     
/** 
  * 
  * GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_LIVE_PERFORMANCE
  * Summary:
  *   Filter on all production types classified at and under the top-level category: <i>Live Performance</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_LIVE_PERFORMANCE       GNSDK_LIST_PRODUCTION_TYPE_LIVE_PERFORMANCE                
/** 
  * 
  * GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_MINI_SERIES
  * Summary:
  *   Filter on all production types classified at and under the top-level category: <i>Mini Series</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_MINI_SERIES		    GNSDK_LIST_PRODUCTION_TYPE_MINI_SERIES		             
/** GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_MOTION_PICTURE
  * Summary:
  *   Filter on all production types classified at and under the top-level
  *   category <i>Motion Picture</i>                                     
*/
#define GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_MOTION_PICTURE	        GNSDK_LIST_PRODUCTION_TYPE_MOTION_PICTURE	           
/** 
  * 
  * GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_MUSIC_VIDEO
  * Summary:
  *   Filter on all production types classified at and under the top-level category: <i>Music Video</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_MUSIC_VIDEO		    GNSDK_LIST_PRODUCTION_TYPE_MUSIC_VIDEO		             
/** 
  * 
  * GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_SERIAL
  * Summary:
  *   Filter on all production types classified at and under the top-level category: <i>Serial</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_SERIAL			        GNSDK_LIST_PRODUCTION_TYPE_SERIAL			           
/** 
  * 
  * GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_SHORT_FEATURE
  * Summary:
  *   Filter on all production types classified at and under the top-level category: <i>Short Feature</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_SHORT_FEATURE		    GNSDK_LIST_PRODUCTION_TYPE_SHORT_FEATURE		         
/** 
  * 
  * GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_SPORTING_EVENT
  * Summary:
  *   Filter on all production types classified at and under the top-level category: <i>Sporting Event</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_SPORTING_EVENT	        GNSDK_LIST_PRODUCTION_TYPE_SPORTING_EVENT	           
/** 
  * 
  * GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_STAGE_PRODUCTION
  * Summary:
  *   Filter on all production types classified at and under the top-level category: <i>Stage Production</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_STAGE_PRODUCTION	    GNSDK_LIST_PRODUCTION_TYPE_STAGE_PRODUCTION	           
/** 
  * 
  * GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_TV_SERIES
  * Summary:
  *   Filter on all production types classified at and under the top-level category: <i>Television Series</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_TV_SERIES			    GNSDK_LIST_PRODUCTION_TYPE_TV_SERIES			         
/** GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_VARIETY_SHOW
  * Summary:
  *   Filter on all production types classified at and under the top-level category: <i>Variety Show</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_PRODUCTION_TYPE_VARIETY_SHOW		    GNSDK_LIST_PRODUCTION_TYPE_VARIETY_SHOW		           

/*!ignore me! Genre filter values */



/** GNSDK_VIDEO_FILTER_VALUE_GENRE_ACTION_ADVENTURE
  * Summary:
  *   Filter on all genres classified at and under the top-level category: <i>Action Adventure</i>
  *                                                                             
*/
#define GNSDK_VIDEO_FILTER_VALUE_GENRE_ACTION_ADVENTURE	                GNSDK_LIST_VIDEO_GENRE_ACTION_ADVENTURE	
/** GNSDK_VIDEO_FILTER_VALUE_GENRE_ADULT
  * Summary:
  *    Filter on all genres classified at and under the top-level category: <i>Adult</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_GENRE_ADULT				            GNSDK_LIST_VIDEO_GENRE_ADULT				         
/** GNSDK_VIDEO_FILTER_VALUE_GENRE_ANIMATION
  * Summary:
  *    Filter on all genres classified at and under the top-level category: <i>Animation</i> 
*/
#define GNSDK_VIDEO_FILTER_VALUE_GENRE_ANIMATION			            GNSDK_LIST_VIDEO_GENRE_ANIMATION			         
/** GNSDK_VIDEO_FILTER_VALUE_GENRE_CHILDREN
  * Summary:
  *    Filter on all genres classified at and under the top-level category: <i>Children</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_GENRE_CHILDREN			                GNSDK_LIST_VIDEO_GENRE_CHILDREN			
/** GNSDK_VIDEO_FILTER_VALUE_GENRE_COMEDY
  * Summary:
  *    Filter on all genres classified at and under the top-level category: <i>Comedy</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_GENRE_COMEDY			                GNSDK_LIST_VIDEO_GENRE_COMEDY			
/** GNSDK_VIDEO_FILTER_VALUE_GENRE_DOCUMENTARY
  * Summary:
  *    Filter on all genres classified at and under the top-level category: <i>Documentary</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_GENRE_DOCUMENTARY		                GNSDK_LIST_VIDEO_GENRE_DOCUMENTARY		
/** GNSDK_VIDEO_FILTER_VALUE_GENRE_DRAMA
  * Summary:
  *    Filter on all genres classified at and under the top-level category: <i>Drama</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_GENRE_DRAMA				            GNSDK_LIST_VIDEO_GENRE_DRAMA				         
/** GNSDK_VIDEO_FILTER_VALUE_GENRE_HORROR
  * Summary:
  *    Filter on all genres classified at and under the top-level category: <i>Horror</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_GENRE_HORROR			                GNSDK_LIST_VIDEO_GENRE_HORROR			
/** GNSDK_VIDEO_FILTER_VALUE_GENRE_MUSICAL
  * Summary:
  *    Filter on all genres classified at and under the top-level category: <i>Musical </i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_GENRE_MUSICAL			                GNSDK_LIST_VIDEO_GENRE_MUSICAL			
/** GNSDK_VIDEO_FILTER_VALUE_GENRE_MYSTERY_AND_SUSPENSE
  * Summary:
  *    Filter on all genres classified at and under the top-level category: <i>Mystery and
  *   Suspense</i>                                                   
*/
#define GNSDK_VIDEO_FILTER_VALUE_GENRE_MYSTERY_AND_SUSPENSE	 	        GNSDK_LIST_VIDEO_GENRE_MYSTERY_AND_SUSPENSE	 	
/** GNSDK_VIDEO_FILTER_VALUE_GENRE_ART_AND_EXPERIMENTAL
  * Summary:
  *    Filter on all genres classified at and under the top-level category: <i>Art and
  *   Experimental</i>                                           
*/
#define GNSDK_VIDEO_FILTER_VALUE_GENRE_ART_AND_EXPERIMENTAL 		    GNSDK_LIST_VIDEO_GENRE_ART_AND_EXPERIMENTAL 		 
/** GNSDK_VIDEO_FILTER_VALUE_GENRE_OTHER
  * Summary:
  *    Filter on all genres classified at and under the top-level category: <i>Other</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_GENRE_OTHER						    GNSDK_LIST_VIDEO_GENRE_OTHER						 
/** GNSDK_VIDEO_FILTER_VALUE_GENRE_ROMANCE
  * Summary:
  *    Filter on all genres classified at and under the top-level category: <i>Romance</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_GENRE_ROMANCE					        GNSDK_LIST_VIDEO_GENRE_ROMANCE					
/** GNSDK_VIDEO_FILTER_VALUE_GENRE_SCIFI_FANTASY
  * Summary:
  *    Filter on all genres classified at and under the top-level category:<i> Science Fiction and
  *   Fantasy</i>                                                            
*/
#define GNSDK_VIDEO_FILTER_VALUE_GENRE_SCIFI_FANTASY				    GNSDK_LIST_VIDEO_GENRE_SCIFI_FANTASY				 
/** GNSDK_VIDEO_FILTER_VALUE_GENRE_SPECIAL_INTEREST_EDUCATION
  * Summary:
  *    Filter on all genres classified at and under the top-level category: <i>Special Interest
  *   and Education</i>                                                   
*/
#define GNSDK_VIDEO_FILTER_VALUE_GENRE_SPECIAL_INTEREST_EDUCATION	    GNSDK_LIST_VIDEO_GENRE_SPECIAL_INTEREST_EDUCATION	
/** GNSDK_VIDEO_FILTER_VALUE_GENRE_MUSIC_AND_PERFORMING_ARTS
  * Summary:
  *    Filter on all genres classified at and under the top-level category: <i>Music and
  *   Performing Arts</i>                                          
*/
#define GNSDK_VIDEO_FILTER_VALUE_GENRE_MUSIC_AND_PERFORMING_ARTS		GNSDK_LIST_VIDEO_GENRE_MUSIC_AND_PERFORMING_ARTS	 
/** GNSDK_VIDEO_FILTER_VALUE_GENRE_SPECIAL_INTEREST_EDUCATION
  * Summary:
  *    Filter on all genres classified at and under the top-level category: <i>Sports</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_GENRE_SPORTS						    GNSDK_LIST_VIDEO_GENRE_SPORTS						
/** GNSDK_VIDEO_FILTER_VALUE_GENRE_TELEVISION_AND_INTERNET
  * Summary:
  *    Filter on all genres classified at and under the top-level category: <i>Television and
  *   Internet</i>                                                     
*/
#define GNSDK_VIDEO_FILTER_VALUE_GENRE_TELEVISION_AND_INTERNET		    GNSDK_LIST_VIDEO_GENRE_TELEVISION_AND_INTERNET		
/** GNSDK_VIDEO_FILTER_VALUE_GENRE_MILITARY_AND_WAR
  * Summary:
  *    Filter on all genres classified at and under the top-level category: <i>Military and War</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_GENRE_MILITARY_AND_WAR				    GNSDK_LIST_VIDEO_GENRE_MILITARY_AND_WAR				
/** GNSDK_VIDEO_FILTER_VALUE_GENRE_WESTERN
  * Summary:
  *    Filter on all genres classified at and under the top-level category: <i>Western</i>
*/
#define GNSDK_VIDEO_FILTER_VALUE_GENRE_WESTERN							GNSDK_LIST_VIDEO_GENRE_WESTERN						 




/** gnsdk_video_query_set_text
  * Summary:
  *   Sets text information used to search a VideoID or Video Explore query
  *   handle.
  * Parameters:
  *   video_query_handle:  [in] VideoID or Video Explore query handle this text
  *                        applies to
  *   search_field:        [in] Search field this text applies to, from the
  *                        available video <link !!MACROS_vid_search_fields, search fields>
  *   search_text:         [in] Actual text to search
  *   search_type:         [in] <link gnsdk_video_search_type_t, Search type>
  *                        to perform with the given text
  *                                                                                        
*/
gnsdk_error_t GNSDK_API
gnsdk_video_query_set_text(
	gnsdk_video_query_handle_t		video_query_handle,
	gnsdk_cstr_t					search_field,
	gnsdk_cstr_t					search_text,
	gnsdk_video_search_type_t		search_type
	);

/** GNSDK_VIDEO_SEARCH_FIELD_CONTRIBUTOR_NAME
  * Summary:
  *   Specifies text for a contributor name search field that is used with <link gnsdk_video_query_set_text@gnsdk_video_query_handle_t@gnsdk_cstr_t@gnsdk_cstr_t@gnsdk_video_search_type_t, gnsdk_video_query_set_text>
*/
#define GNSDK_VIDEO_SEARCH_FIELD_CONTRIBUTOR_NAME		"gnsdk_video_search_field_contributor_name"
/** GNSDK_VIDEO_SEARCH_FIELD_CHARACTER_NAME
  * Summary:
  *   Specifies text for a character name search field that is used with <link gnsdk_video_query_set_text@gnsdk_video_query_handle_t@gnsdk_cstr_t@gnsdk_cstr_t@gnsdk_video_search_type_t, gnsdk_video_query_set_text>
*/
#define GNSDK_VIDEO_SEARCH_FIELD_CHARACTER_NAME			"gnsdk_video_search_field_character_name"
/** GNSDK_VIDEO_SEARCH_FIELD_WORK_FRANCHISE
  * Summary:
  *   Specifies text for a work franchise search field that is used with <link gnsdk_video_query_set_text@gnsdk_video_query_handle_t@gnsdk_cstr_t@gnsdk_cstr_t@gnsdk_video_search_type_t, gnsdk_video_query_set_text>
*/
#define GNSDK_VIDEO_SEARCH_FIELD_WORK_FRANCHISE			"gnsdk_video_search_field_work_franchise"
/** GNSDK_VIDEO_SEARCH_FIELD_WORK_SERIES
  * Summary:
  *   Specifies text for a work series search field that is used with <link gnsdk_video_query_set_text@gnsdk_video_query_handle_t@gnsdk_cstr_t@gnsdk_cstr_t@gnsdk_video_search_type_t, gnsdk_video_query_set_text>
*/
#define GNSDK_VIDEO_SEARCH_FIELD_WORK_SERIES			"gnsdk_video_search_field_work_series"
/** GNSDK_VIDEO_SEARCH_FIELD_WORK_TITLE
  * Summary:
  *   Specifies text for a work title search field that is used with <link gnsdk_video_query_set_text@gnsdk_video_query_handle_t@gnsdk_cstr_t@gnsdk_cstr_t@gnsdk_video_search_type_t, gnsdk_video_query_set_text>
*/
#define GNSDK_VIDEO_SEARCH_FIELD_WORK_TITLE				"gnsdk_video_search_field_work_title"
/** GNSDK_VIDEO_SEARCH_FIELD_PRODUCT_TITLE
  * Summary:
  *   Specifies text for a product title search field that is used with <link gnsdk_video_query_set_text@gnsdk_video_query_handle_t@gnsdk_cstr_t@gnsdk_cstr_t@gnsdk_video_search_type_t, gnsdk_video_query_set_text>
*/
#define GNSDK_VIDEO_SEARCH_FIELD_PRODUCT_TITLE			"gnsdk_video_search_field_product_title"
/** GNSDK_VIDEO_SEARCH_FIELD_SERIES_TITLE
  * Summary:
  *   Specifies text for a series title search field that is used with <link gnsdk_video_query_set_text@gnsdk_video_query_handle_t@gnsdk_cstr_t@gnsdk_cstr_t@gnsdk_video_search_type_t, gnsdk_video_query_set_text>
*/
#define GNSDK_VIDEO_SEARCH_FIELD_SERIES_TITLE			"gnsdk_video_search_field_series_title"
/** GNSDK_VIDEO_SEARCH_FIELD_ALL
  * Summary:
  *   Specifies text for a comprehensive search field that is used with <link gnsdk_video_query_set_text@gnsdk_video_query_handle_t@gnsdk_cstr_t@gnsdk_cstr_t@gnsdk_video_search_type_t, gnsdk_video_query_set_text>.
*/
#define GNSDK_VIDEO_SEARCH_FIELD_ALL					"gnsdk_video_search_field_all"


/** gnsdk_video_query_set_gdo
  * Summary:
  *   Sets a GDO as input to a VideoID or Video Explore query handle.
  * Parameters:
  *   video_query_handle:  [in] VideoID or Video Explore query handle this
  *                          text applies to
  *   query_gdo:             [in] GDO that identifies a video object (such as a
  *                          DVD)
  *                                                                            
*/
gnsdk_error_t GNSDK_API
gnsdk_video_query_set_gdo(
	gnsdk_video_query_handle_t	video_query_handle,
	gnsdk_gdo_handle_t			query_gdo
	);

/** gnsdk_video_query_option_set
  * Summary:
  *   Sets an option for a given VideoID or Video Explore query handle.
  * Parameters:
  *   query_handle:  [in] VideoID or Video Explore query handle to set option
  *                  for
  *   option_key:    [in] An option from the available video <link !!MACROS_vid_options, option keys>
  *   option_value:  [in] A value that corresponds to the defined option key;
  *                  may be an alphabetical or numeric value, or one of the
  *                  available video <link !!MACROS_vid_options, option values>
  *                                                                                                  
*/
gnsdk_error_t GNSDK_API
gnsdk_video_query_option_set(
	gnsdk_video_query_handle_t	query_handle,
	gnsdk_cstr_t				option_key,
	gnsdk_cstr_t				option_value
	);

/** gnsdk_video_query_option_get
  * Summary:
  *   Retrieves an option for a given VideoID or Video Explore query handle
  * Parameters:
  *   query_handle:    [in] VideoID or Video Explore query handle to retrieve
  *                    option from
  *   option_key:      [in] One of the <link !!MACROS_vid_options, Video option keys>
  *                    to retrieve an option value for
  *   p_option_value:  [out] Pointer to receive an option value
  *                                                                                  
*/
gnsdk_error_t GNSDK_API
gnsdk_video_query_option_get(
	gnsdk_video_query_handle_t	query_handle,
	gnsdk_cstr_t				option_key,
	gnsdk_cstr_t*				p_option_value
	);

/** GNSDK_VIDEO_OPTION_ENABLE_LINK_DATA
  * Summary:
  *   Indicates whether a response includes any Link data (third-party
  *   metadata).                                                      
*/
#define GNSDK_VIDEO_OPTION_ENABLE_LINK_DATA				"gnsdk_video_option_enable_link"

/** GNSDK_VIDEO_OPTION_PREFERRED_LANG
  * Summary:
  *   Specifies the preferred language of the returned results. This option
  *   applies only to TOC Lookups.                                         
*/
#define GNSDK_VIDEO_OPTION_PREFERRED_LANG				"gnsdk_video_preferred_lang"
/** GNSDK_VIDEO_RESULT_RANGE_START
  * Summary:
  *   Specifies the initial value for a result range that is returned by a
  *   response. The initial value can be an ordinal.                      
*/
#define GNSDK_VIDEO_OPTION_RESULT_RANGE_START			"gnsdk_video_result_range_start"
/** GNSDK_VIDEO_RESULT_RANGE_SIZE
  * Summary:
  *   Specifies a maximum number of results that a response can return.
*/
#define GNSDK_VIDEO_OPTION_RESULT_RANGE_SIZE			"gnsdk_video_result_range_size"


/** GNSDK_VIDEO_OPTION_QUERY_NOCACHE
  * Summary:
  *   Indicates whether a response is not automatically stored in the cache.
*/
#define GNSDK_VIDEO_OPTION_QUERY_NOCACHE				"gnsdk_video_option_query_nocache"

/** GNSDK_VIDEO_OPTION_QUERY_ENABLE_COMMERCE_TYPE
  * Summary:
  *   Specifies that a TOC lookup includes the disc's commerce type.
*/
#define GNSDK_VIDEO_OPTION_QUERY_ENABLE_COMMERCE_TYPE		"gnsdk_video_option_query_enable_commerce_type"


/** GNSDK_VIDEO_OPTION_VALUE_TRUE
  * Summary:
  *   Represents a boolean TRUE. 
*/
#define GNSDK_VIDEO_OPTION_VALUE_TRUE					"true"
/** GNSDK_VIDEO_OPTION_VALUE_FALSE
  * Summary:
  *   Represents a boolean FALSE.   
*/
#define GNSDK_VIDEO_OPTION_VALUE_FALSE					"false"


/** gnsdk_video_query_find_products
  * Summary:
  *   Performs a VideoID or Video Explore query for Products.
  * Parameters:
  *   video_query_handle:  [in] VideoID or Video Explore query handle to
  *                          perform query with
  *   p_match_type:          [out] Pointer to receive the match type for the
  *                          video query
  *   p_response_gdo:        [out] Pointer to receive the GDO handle with the
  *                          Products' response
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_video_query_find_products(
	gnsdk_video_query_handle_t	video_query_handle,
	gnsdk_video_match_type_t*	p_match_type,
	gnsdk_gdo_handle_t*			p_response_gdo
	);



#define gnsdk_video_query_find_videos		gnsdk_video_query_find_products

/** gnsdk_video_query_find_works
  * Summary:
  *   Performs a Video Explore query for Works.
  * Parameters:
  *   video_query_handle:  [in] Video Explore query handle to perform query
  *                          with
  *   p_match_type:          [out] Pointer to receive the match type for the
  *                          video query
  *   p_response_gdo:        [out] Pointer to receive the GDO handle with the
  *                          Works' response
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_video_query_find_works(
	gnsdk_video_query_handle_t	video_query_handle,
	gnsdk_video_match_type_t*	p_match_type,
	gnsdk_gdo_handle_t*			p_response_gdo
	);

/** gnsdk_video_query_find_seasons
  * Summary:
  *   Performs a Video Explore query for Seasons.
  * Parameters:
  *   video_query_handle:  [in] Video Explore query handle to perform query
  *                          with
  *   p_match_type:          [out] Pointer to receive the match type for the
  *                          video query
  *   p_response_gdo:        [out] Pointer to receive the GDO handle with the
  *                          Seasons' response
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_video_query_find_seasons(
	gnsdk_video_query_handle_t	video_query_handle,
	gnsdk_video_match_type_t*	p_match_type,
	gnsdk_gdo_handle_t*			p_response_gdo
	);

/** gnsdk_video_query_find_series
  * Summary:
  *   Performs a Video Explore query for Series.
  * Parameters:
  *   video_query_handle:  [in] Video Explore query handle to perform query
  *                          with
  *   p_match_type:          [out] Pointer to receive the match type for the
  *                          video query
  *   p_response_gdo:        [out] Pointer to receive the GDO handle with the
  *                          Series' response
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_video_query_find_series(
	gnsdk_video_query_handle_t	video_query_handle,
	gnsdk_video_match_type_t*	p_match_type,
	gnsdk_gdo_handle_t*			p_response_gdo
	);

/** gnsdk_video_query_find_contributors
  * Summary:
  *   Performs a Video Explore query for Contributors.
  * Parameters:
  *   video_query_handle:  [in] Video Explore query handle to perform query
  *                          with
  *   p_match_type:          [out] Pointer to receive the match type for the
  *                          video query
  *   p_response_gdo:        [out] Pointer to receive the GDO handle with the
  *                          Contributors' response
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_video_query_find_contributors(
	gnsdk_video_query_handle_t	video_query_handle,
	gnsdk_video_match_type_t*	p_match_type,
	gnsdk_gdo_handle_t*			p_response_gdo
	);


/** gnsdk_video_query_find_objects
  * Summary:
  *   Performs a Video Explore query for any type of video object.
  * Parameters:
  *   video_query_handle:  [in] Video Explore query handle to perform query
  *                          with
  *   p_match_type:          [out] Pointer to receive the match type for the
  *                          video query
  *   p_response_gdo:        [out] Pointer to receive the GDO handle with the
  *                          Objects' response
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_video_query_find_objects(
	gnsdk_video_query_handle_t	video_query_handle,
	gnsdk_video_match_type_t*	p_match_type,
	gnsdk_gdo_handle_t*			p_response_gdo
	);

/** gnsdk_video_query_find_suggestions
  * Summary:
  *   Performs a VideoID or Video Explore query for search suggestion text.
  * Parameters:
  *   video_query_handle:  [in] VideoID or Video Explore query handle to
  *                          perform query with
  *   p_response_gdo:        [out] Pointer to receive the GDO handle with the
  *                          video suggestions' response
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_video_query_find_suggestions(
	gnsdk_video_query_handle_t	video_query_handle,
	gnsdk_gdo_handle_t*			p_response_gdo
	);

/** gnsdk_video_query_resolve
  * Summary:
  *   Selects an individual video result from a set of multiple returned
  *   matches.
  * Parameters:
  *   video_query_handle:  [in] VideoID or Video Explore query handle that
  *                          was previously queried
  *   response_gdo:          [in] Response GDO from previous VideoID or Video
  *                          Explore query
  *   match_ordinal:         [in] 1\-based ordinal of result to return
  *   p_response_gdo:        [out] Pointer to receive result GDO
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_video_query_resolve(
	gnsdk_video_query_handle_t	video_query_handle,
	gnsdk_gdo_handle_t			response_gdo,
	gnsdk_uint32_t				match_ordinal,
	gnsdk_gdo_handle_t*			p_response_gdo
	);



#ifdef __cplusplus
}
#endif

#endif /* _GNSDK_VIDEO_H_ */


