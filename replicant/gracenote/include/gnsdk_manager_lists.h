/** Gracenote SDK: GNSDK Manager Lists APIs and Key Definitions
  * Author:
  *   Copyright (c) 2012 Gracenote, Inc.
  *   
  *   This software may not be used in any way or distributed without
  *   permission. All rights reserved.
  *   
  *   Some code herein may be covered by USA and international patents.
*/

/** gnsdk_manager_lists.h: Lists interface for the GNSDK Manager.
*/

#ifndef _GNSDK_MANAGER_LISTS_H_
#define _GNSDK_MANAGER_LISTS_H_

#ifdef __cplusplus
extern "C"{
#endif


/******************************************************************************
 * Typdefs
 ******************************************************************************/

/** <title gnsdk_list_handle_t>
  * <toctitle gnsdk_list_handle_t>
  * 
  * gnsdk_list_handle_t
  * Summary:
  *   Handle to a List object. List objects are used to maintain certain
  *   Gracenote data fields too complex or too dynamic to be included in the
  *   GDO. Examples of List objects are the Genre List, Language List and
  *   Region List.
  *   
  *   This handle must be released by gnsdk_manager_list_release.           
*/
GNSDK_DECLARE_HANDLE( gnsdk_list_handle_t );

/** <title gnsdk_list_element_handle_t>
  * <toctitle gnsdk_list_element_handle_t>
  * 
  * gnsdk_list_element_handle_t
  * Summary:
  *   Handle to a List Element object. A List Element object stores a single
  *   value that is contained in a List object.
  *   
  *   This handle does not have to be released.                             
*/
GNSDK_DECLARE_HANDLE( gnsdk_list_element_handle_t );

/** <title gnsdk_list_correlate_set_handle_t>
  * <toctitle gnsdk_list_correlate_set_handle_t>
  * <flag list>
  * 
  * gnsdk_list_correlate_set_handle_t
  * Summary:
  *   Handle to a list correlate set.           
*/
GNSDK_DECLARE_HANDLE( gnsdk_list_correlate_set_handle_t );


/******************************************************************************
 * SDK Manager Lists APIs
 ******************************************************************************/

/** gnsdk_manager_list_retrieve
  * Summary:
  *   Retrieves a list handle for the specified list. A list not currently in
  *   memory is downloaded from the Gracenote Service. As this function blocks
  *   the current thread until the download is complete, set a status callback
  *   function to receive progress messages.
  * Parameters:
  *   list_type:           [in] Type of list to retrieve. Specify from an
  *                        available <link !!MACROS_mgr_list_types, List Types>
  *                        value.
  *   list_language:       [in_opt] Language of list to retrieve. Specify from
  *                        an available <link !!MACROS_mgr_locale_lang, Locale Languages>
  *                        value.
  *   list_region:         [in_opt] Region for list to retrieve. Specify from
  *                        an available <link !!MACROS_mgr_locale_regions, Locale Regions>
  *                        value.
  *   list_descriptor:     [in_opt] Descriptor of list to retrieve. Specify
  *                        from an available <link !!MACROS_mgr_locale_descr, Locale Descriptors>
  *                        value.
  *   user_handle:         [in] User handle for the user making the list
  *                        retrieve request
  *   callback:            [in_opt] Callback function for status and progress
  *   callback_user_data:  [in_opt] Data that is passed back through calls to
  *                        the callback function
  *   p_list_handle:       [out] Pointer to receive the list object handle
  *                                                                                              
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_retrieve(
	gnsdk_cstr_t					list_type,
	gnsdk_cstr_t					list_language,
	gnsdk_cstr_t					list_region,
	gnsdk_cstr_t					list_descriptor,
	gnsdk_user_handle_t				user_handle,
	gnsdk_manager_query_callback_fn	callback,
	const gnsdk_void_t*				callback_user_data,
	gnsdk_list_handle_t*			p_list_handle
	);

/** gnsdk_manager_list_update
  * Summary:
  *   Tests an existing list for available updates and downloads a new list,
  *   if available. As this function blocks the current thread until the
  *   download is complete, set a status callback function to receive progress
  *   messages.
  * Parameters:
  *   list_handle:         [in] List object handle to a pre\-loaded list
  *   user_handle:         [in] Client handle for the user making the list
  *                        retrieve request
  *   callback:            [in_opt] Callback function for status and progress
  *   callback_user_data:  [in_opt] Data that is passed back to calls to the
  *                        callback function
  *   p_updated:           [out] Pointer to receive flag indicating if the list
  *                        was updated; is set to GNSDK_TRUE if an update
  *                        occurred.
  *                                                                            
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_update(
	gnsdk_list_handle_t				list_handle,
	gnsdk_user_handle_t				user_handle,
	gnsdk_manager_query_callback_fn	callback,
	const gnsdk_void_t*				callback_user_data,
	gnsdk_bool_t*					p_updated
	);

/** gnsdk_manager_list_release
  * Summary:
  *   Unloads and frees all resources associated with a list.
  * Parameters:
  *   list_handle:  [in] Handle to list object
  *                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_release(
	gnsdk_list_handle_t				list_handle
	);

/** gnsdk_manager_list_serialize
  * Summary:
  *   Serializes a list handle into encrypted text so the application can
  *   store it for future use.
  * Parameters:
  *   list_handle:        [in] List handle to serialize
  *   p_serialized_list:  [out] Pointer to receive serialized list handle
  *                       string
  *                                                                      
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_serialize(
	gnsdk_list_handle_t				list_handle,
	gnsdk_str_t*					p_serialized_list
	);

/** gnsdk_manager_list_deserialize
  * Summary:
  *   Reconstitutes a list handle from given serialized list data.
  * Parameters:
  *   serialized_list:  [in] String of serialized list handle data
  *   p_list_handle:    [out] Pointer to receive list handle
  *                                                               
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_deserialize(
	gnsdk_cstr_t					serialized_list,
	gnsdk_list_handle_t*			p_list_handle
	);


/** gnsdk_manager_list_render_to_xml
  * Summary:
  *   Renders list data to XML.
  * Parameters:
  *   list_handle:   [in] Handle to a list
  *   levels:        [in] List level values to render
  *   render_flags:  [in] List render flags
  *   p_xml_render:  [out] Pointer to render list to XML
  *                                                     
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_render_to_xml(
	gnsdk_list_handle_t				list_handle,
	gnsdk_uint32_t					levels,
	gnsdk_uint32_t					render_flags,
	gnsdk_str_t*					p_xml_render
	);

/** GNSDK_LIST_RENDER_XML_MINIMAL
  * Summary:
  *   Renders minimal list data into the XML for a list.
*/
#define GNSDK_LIST_RENDER_XML_MINIMAL				0x0000
/** GNSDK_LIST_RENDER_XML_SUBMIT
  * Summary:
  *   Renders minimal and Submit-specific data into the XML for a list.
*/
#define GNSDK_LIST_RENDER_XML_SUBMIT				0x0001

/** gnsdk_manager_list_get_type
  * Summary:
  *   Retrieves a list type for a given list handle object.
  * Parameters:
  *   list_handle:  [in] Handle to list object
  *   p_type:       [out] Pointer to receive list type value
  *                                                         
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_get_type(
	gnsdk_list_handle_t				list_handle,
	gnsdk_cstr_t*					p_type
	);

/** gnsdk_manager_list_get_language
  * Summary:
  *   Retrieves a list language for a given list handle object.
  * Parameters:
  *   list_handle:  [in] Handle to list object
  *   p_lang:       [out] Pointer to receive list language value
  *                                                             
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_get_language(
	gnsdk_list_handle_t				list_handle,
	gnsdk_cstr_t*					p_lang
	);

/** gnsdk_manager_list_get_region
  * Summary:
  *   Retrieves a list region for a given list handle object.
  * Parameters:
  *   list_handle:  [in] Handle to list object
  *   p_region:     [out] Pointer to receive list region value
  *                                                           
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_get_region(
	gnsdk_list_handle_t				list_handle,
	gnsdk_cstr_t*					p_region
	);

/** gnsdk_manager_list_get_descriptor
  * Summary:
  *   Retrieves a list descriptor for a given list handle object.
  * Parameters:
  *   list_handle:   [in] Handle to a list object
  *   p_descriptor:  [out] Pointer to receive list descriptor value
  *                                                                
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_get_descriptor(
	gnsdk_list_handle_t				list_handle,
	gnsdk_cstr_t*					p_descriptor
	);

/** gnsdk_manager_list_get_level_count
  * Summary:
  *   Retrieves a maximum number of levels in a hierarchy for a given list.
  * Parameters:
  *   list_handle:  [in] Handle to list object
  *   p_count:      [out] Pointer to receive level count
  *                                                                        
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_get_level_count(
	gnsdk_list_handle_t				list_handle,
	gnsdk_uint32_t*					p_count
	);

/** gnsdk_manager_list_get_display_string_by_id
  * Summary:
  *   Retrieves the display string for the list element specified by the list
  *   element ID.
  * Parameters:
  *   list_handle:  [in] Handle to list object
  *   item_id:      [in] List element item ID
  *   level:        [in] Level of display string to retrieve
  *   p_string:     [out] Pointer to receive display string
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_get_display_string_by_id(
	gnsdk_list_handle_t				list_handle,
	gnsdk_uint32_t					item_id,
	gnsdk_uint32_t					level,
	gnsdk_cstr_t*					p_string
	);

/** gnsdk_manager_list_get_display_string_by_gdo
  * Summary:
  *   Retrieves the display string of the list element that corresponds to the
  *   given GDO.
  * Parameters:
  *   list_handle:  [in] Handle to list object
  *   gdo_handle:   [in] Handle to GDO
  *   ordinal:      [in] Ordinal of display string to retrieve (if multiple
  *                 values exist) (1\-based)
  *   level:        [in] Level of display string to retrieve
  *   p_string:     [out] Pointer to receive display string
  *                                                                           
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_get_display_string_by_gdo(
	gnsdk_list_handle_t				list_handle,
	gnsdk_gdo_handle_t				gdo_handle,
	gnsdk_uint32_t					ordinal,
	gnsdk_uint32_t					level,
	gnsdk_cstr_t*					p_string
	);

/** gnsdk_manager_list_get_element_count
  * Summary:
  *   Retrieves a number of list elements at a specified level for a given
  *   list.
  * Parameters:
  *   list_handle:  [in] Handle to list object
  *   level:        [in] Level of list to count
  *   p_count:      [out] Pointer to receive element count
  *                                                                       
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_get_element_count(
	gnsdk_list_handle_t				list_handle,
	gnsdk_uint32_t					level,
	gnsdk_uint32_t*					p_count
	);

/** gnsdk_manager_list_get_element
  * Summary:
  *   Retrieves a list element from a list using defined level and index
  *   values.
  * Parameters:
  *   list_handle:       [in] Handle to list object
  *   level:             [in] Level in list of element to retrieve
  *   index:             [in] Index in level of element to retrieve
  *   p_element_handle:  [out] Pointer that receives the list element handle
  *                                                                         
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_get_element(
	gnsdk_list_handle_t					list_handle,
	gnsdk_uint32_t						level,
	gnsdk_uint32_t						index,
	gnsdk_list_element_handle_t* const	p_element_handle
	);

/** gnsdk_manager_list_get_element_by_id
  * Summary:
  *   Retrieves a list element from a list using a specific list element ID.
  * Parameters:
  *   list_handle:       [in] Handle to list object
  *   item_id:           [in] List element item ID
  *   p_element_handle:  [out] Pointer that receives the list element handle
  *                                                                         
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_get_element_by_id(
	gnsdk_list_handle_t					list_handle,
	gnsdk_uint32_t						item_id,
	gnsdk_list_element_handle_t* const	p_element_handle
	);

/** gnsdk_manager_list_get_element_by_gdo
  * Summary:
  *   Retrieves a list element that corresponds to a given GDO.
  * Parameters:
  *   list_handle:       [in] Handle to list object
  *   gdo_handle:        [in] Handle to GDO
  *   ordinal:           [in] Ordinal of matching list element to retrieve (if
  *                      multiple values exist) (1\-based)
  *   level:             [in] Level of list element to retrieve
  *   p_element_handle:  [out] Pointer that receives the list element handle
  *                                                                           
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_get_element_by_gdo(
	gnsdk_list_handle_t					list_handle,
	gnsdk_gdo_handle_t					gdo_handle,
	gnsdk_uint32_t						ordinal,
	gnsdk_uint32_t						level,
	gnsdk_list_element_handle_t* const	p_element_handle
	);

/** gnsdk_manager_list_element_get_display_string
  * Summary:
  *   Retrieves a display string for a given list element.
  * Parameters:
  *   element_handle:  [in] Handle to list element object
  *   p_string:        [out] Pointer to receive display string
  *                                                           
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_element_get_display_string(
	gnsdk_list_element_handle_t		element_handle,
	gnsdk_cstr_t*					p_string
	);

/** gnsdk_manager_list_element_get_id
  * Summary:
  *   Retrieves a specified list element ID for a given list element.
  * Parameters:
  *   element_handle:  [in] Handle to list element object
  *   p_item_id:       [out] Pointer to receive list element ID
  *                                                                  
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_element_get_id(
	gnsdk_list_element_handle_t		element_handle,
	gnsdk_uint32_t*					p_item_id
	);

/** gnsdk_manager_list_element_get_id_for_submit
  * Summary:
  *   Retrieves a list element ID for use in submitting parcels.
  * Parameters:
  *   element_handle:    [in] Handle to list element object
  *   p_item_submit_id:  [out] Pointer to receive list element ID for Submit
  *                                                                         
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_element_get_id_for_submit(
	gnsdk_list_element_handle_t		element_handle,
	gnsdk_uint32_t*					p_item_submit_id
	);

/** gnsdk_manager_list_element_get_value
  * Summary:
  *   Retrieves a non-display string value for a given list element.
  * Parameters:
  *   element_handle:  [in] Handle to list element object
  *   list_value_key:  [in] <link !!MACROS_mgr_list_keys, List key> indicating
  *                    which value to retrieve
  *   p_value:         [out] Pointer to receive list element value
  *                                                                           
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_element_get_value(
	gnsdk_list_element_handle_t		element_handle,
	gnsdk_cstr_t					list_value_key,
	gnsdk_cstr_t*					p_value
	);

/*
 * List Value Keys
 */

/** GNSDK_LIST_KEY_DESC
  * Summary:
  *   The list element's description.
*/
#define			GNSDK_LIST_KEY_DESC				"gnsdk_list_key_desc"
/** GNSDK_LIST_KEY_RATINGTYPE_ID
  * Summary:
  *   The list element's Rating Type ID (available in content ratings list).
*/
#define			GNSDK_LIST_KEY_RATINGTYPE_ID	"gnsdk_list_key_ratingtype_id"

/** gnsdk_manager_list_element_get_parent
  * Summary:
  *   Retrieves the parent element of the given list element.
  * Parameters:
  *   element_handle:    [in] Handle to list element object
  *   p_element_handle:  [out] Pointer to receive parent list element handle
  *                                                                         
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_element_get_parent(
	gnsdk_list_element_handle_t			element_handle,
	gnsdk_list_element_handle_t* const	p_element_handle
	);

/** gnsdk_manager_list_element_get_level
  * Summary:
  *   Retrieves the hierarchy level for a given list element.
  * Parameters:
  *   element_handle:  [in] Handle to list element object
  *   p_level:         [out] Pointer to receive level value
  *                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_element_get_level(
	gnsdk_list_element_handle_t		element_handle,
	gnsdk_uint32_t*					p_level
	);

/** gnsdk_manager_list_element_get_child_count
  * Summary:
  *   Retrieves a specified number of child list elements for a given list
  *   element.
  * Parameters:
  *   element_handle:  [in] Handle to list element object
  *   p_count:         [out] Pointer to receive child element count
  *                                                                       
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_element_get_child_count(
	gnsdk_list_element_handle_t		element_handle,
	gnsdk_uint32_t*					p_count
	);

/** gnsdk_manager_list_element_get_child
  * Summary:
  *   Retrieves the specified child list element from the given list element.
  * Parameters:
  *   element_handle:          [in] Handle to list element object
  *   index:                   [in] Index of child of list element to retrieve
  *                            (0\-based)
  *   p_child_element_handle:  [out] Pointer to receive child list element
  *                            handle
  *                                                                           
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_list_element_get_child(
	gnsdk_list_element_handle_t			element_handle,
	gnsdk_uint32_t						index,
	gnsdk_list_element_handle_t* const	p_child_element_handle
	);

/*
 * List Types
 */

/** GNSDK_LIST_TYPE_LANGUAGES
  * Summary:
  *   The list of supported languages that are potentially available for use.
*/
#define			GNSDK_LIST_TYPE_LANGUAGES			"gnsdk_list_type_languages"
/** GNSDK_LIST_TYPE_GENRES
  * Summary:
  *   The list of supported music genres.
*/
#define			GNSDK_LIST_TYPE_GENRES				"gnsdk_list_type_genres"
/** GNSDK_LIST_TYPE_ORIGINS
  * Summary:
  *   The list of supported geographic origins for artists.
*/
#define			GNSDK_LIST_TYPE_ORIGINS				"gnsdk_list_type_origins"
/** GNSDK_LIST_TYPE_ERAS
  * Summary:
  *   The list of supported music era categories.
*/
#define			GNSDK_LIST_TYPE_ERAS				"gnsdk_list_type_eras"
/** GNSDK_LIST_TYPE_ARTISTTYPES
  * Summary:
  *   The list of supported artist type categories.
*/
#define			GNSDK_LIST_TYPE_ARTISTTYPES			"gnsdk_list_type_artist_types"
/** GNSDK_LIST_TYPE_ROLES
  * Summary:
  *   The list of supported roles and credits for album data.
*/
#define			GNSDK_LIST_TYPE_ROLES				"gnsdk_list_type_roles"
/** GNSDK_LIST_TYPE_GENRES_VIDEO
  * Summary:
  *   The list of supported video genres.
*/
#define			GNSDK_LIST_TYPE_GENRES_VIDEO		"gnsdk_list_type_genres_video"
/** GNSDK_LIST_TYPE_RATINGS
  * Summary:
  *   The list of supported content ratings for video data.
*/
#define			GNSDK_LIST_TYPE_RATINGS				"gnsdk_list_type_ratings"
/** GNSDK_LIST_TYPE_RATINGTYPES
  * Summary:
  *   The list of supported content rating types for video data.
*/
#define			GNSDK_LIST_TYPE_RATINGTYPES			"gnsdk_list_type_ratingtypes"
/** GNSDK_LIST_TYPE_CONTRIBUTORS
  * Summary:
  *   The list of supported roles and credits for video data.
*/
#define			GNSDK_LIST_TYPE_CONTRIBUTORS		"gnsdk_list_type_contributors"
/** GNSDK_LIST_TYPE_FEATURETYPES
  * Summary:
  *   The list of supported feature types for video data.
*/
#define			GNSDK_LIST_TYPE_FEATURETYPES		"gnsdk_list_type_featuretypes"
/** GNSDK_LIST_TYPE_VIDEOREGIONS
  * Summary:
  *   The list of supported video regions.
*/
#define			GNSDK_LIST_TYPE_VIDEOREGIONS		"gnsdk_list_type_videoregions"
/** GNSDK_LIST_TYPE_VIDEOTYPES
  * Summary:
  *   The list of supported video types, such as <i>Documentary</i>, <i>Sporting
  *   Event</i>, or <i>Motion Picture</i>.
*/
#define			GNSDK_LIST_TYPE_VIDEOTYPES			"gnsdk_list_type_videotypes"
/** GNSDK_LIST_TYPE_MEDIATYPES
  * Summary:
  *   The list of supported media types for music and video, such as <i>Audio
  *   CD</i>, <i>Blu-ray</i>, <i>DVD</i>, or <i>HD DVD.</i>                  
*/
#define			GNSDK_LIST_TYPE_MEDIATYPES			"gnsdk_list_type_mediatypes"

/** GNSDK_LIST_TYPE_VIDEOSERIALTYPES
  * Summary:
  *   The list of supported video serial types, such as <i>Series</i> or <i>Episode</i>.
*/
#define			GNSDK_LIST_TYPE_VIDEOSERIALTYPES	"gnsdk_list_type_videoserialtypes"
/** GNSDK_LIST_TYPE_WORKTYPES
  * Summary:
  *   The list of supported work types for video data, such as <i>Musical</i>
  *   \or <i>Image</i>.
*/

#define			GNSDK_LIST_TYPE_WORKTYPES			"gnsdk_list_type_worktypes"
/** GNSDK_LIST_TYPE_MEDIASPACES
  * Summary:
  *   The list of supported media spaces for video data, such as <i>Music</i>,
  *   <i>Film</i>, or <i>Stage</i>.
*/
#define			GNSDK_LIST_TYPE_MEDIASPACES			"gnsdk_list_type_mediaspaces"

/** GNSDK_LIST_TYPE_MOODS
  * Summary:
  *   The list of supported moods for music data; has two levels of
  *   granularity.                                                 
*/
#define			GNSDK_LIST_TYPE_MOODS				"gnsdk_list_type_moods"
/** GNSDK_LIST_TYPE_TEMPOS
  * Summary:
  *   The list of supported tempos for music data; has three levels of
  *   granularity.                                                    
*/
#define			GNSDK_LIST_TYPE_TEMPOS				"gnsdk_list_type_tempos"

/** GNSDK_LIST_TYPE_COMPOSITION_FORM
  * Summary:
  *   The list of supported composition forms for classical music.
*/
#define			GNSDK_LIST_TYPE_COMPOSITION_FORM	"gnsdk_list_type_compform"
/** GNSDK_LIST_TYPE_COMPOSITION_STYLE
  * Summary:
  *   The list of supported composition styles for classical music.
*/
#define			GNSDK_LIST_TYPE_COMPOSITION_STYLE	"gnsdk_list_type_compstyle"
/** GNSDK_LIST_TYPE_INSTRUMENTATION
  * Summary:
  *   The list of supported instrumentation for classical music.
*/
#define			GNSDK_LIST_TYPE_INSTRUMENTATION		"gnsdk_list_type_instrumentation"
/** GNSDK_LIST_TYPE_VIDEOSTORYTYPE
  * Summary:
  *   The list of supported overall story types for video data, such as<i>
  *   Love Story</i>.                                                     
*/
#define 		GNSDK_LIST_TYPE_VIDEOSTORYTYPE 		"gnsdk_list_type_videostorytype"
/** GNSDK_LIST_TYPE_VIDEOAUDIENCE
  * Summary:
  *   The list of supported audience types for video data.
*/
#define 		GNSDK_LIST_TYPE_VIDEOAUDIENCE 		"gnsdk_list_type_videoaudience"
/** GNSDK_LIST_TYPE_VIDEOMOOD
  * Summary:
  *   The list of supported moods for video data, such as <i>Offbeat</i>.
*/
#define 		GNSDK_LIST_TYPE_VIDEOMOOD 			"gnsdk_list_type_videomood"
/** GNSDK_LIST_TYPE_VIDEOREPUTATION
  * Summary:
  *   The list of supported film reputation types for video data, such as <i>Classic</i>.
*/
#define 		GNSDK_LIST_TYPE_VIDEOREPUTATION 	"gnsdk_list_type_videoreputation"
/** GNSDK_LIST_TYPE_VIDEOSCENARIO
  * Summary:
  *   The list of supported scenarios for video data.
*/
#define 		GNSDK_LIST_TYPE_VIDEOSCENARIO 		"gnsdk_list_type_videoscenario"
/** GNSDK_LIST_TYPE_VIDEOSETTINGENV
  * Summary:
  *   The list of supported setting environments for video data.
*/
#define 		GNSDK_LIST_TYPE_VIDEOSETTINGENV 	"gnsdk_list_type_videosettingenv"

/** GNSDK_LIST_TYPE_VIDEOSETTINGPERIOD
  * Summary:
  *   The list of supported historical time settings for video data, such as<i>
  *   Elizabethan Era, 1558-1603, </i>or <i>Jazz Age, 1919-1929</i>            
*/
#define 		GNSDK_LIST_TYPE_VIDEOSETTINGPERIOD 	"gnsdk_list_type_videosettingperiod"

/** GNSDK_LIST_TYPE_VIDEOSOURCE
  * Summary:
  *   The list of supported story concept sources for video data, such as <i>Fairy
  *   Tales &amp; Nursery Rhymes</i>.                                             
*/
#define 		GNSDK_LIST_TYPE_VIDEOSOURCE 		"gnsdk_list_type_videosource"
/** GNSDK_LIST_TYPE_VIDEOSTYLE
  * Summary:
  *   The list of supported film style types for video data, such as <i>Film
  *   Noir</i>.                                                             
*/
#define 		GNSDK_LIST_TYPE_VIDEOSTYLE 			"gnsdk_list_type_videostyle"
/** GNSDK_LIST_TYPE_VIDEOTOPIC
  * Summary:
  *   The list of supported film topics for video data, such as <i>Racing</i>
  *   \or<i>Teen</i> <i>Angst</i>.                                           
*/
#define 		GNSDK_LIST_TYPE_VIDEOTOPIC 			"gnsdk_list_type_videotopic"



#ifdef __cplusplus
}
#endif

#endif /* _GNSDK_MANAGER_LISTS_H_ */

