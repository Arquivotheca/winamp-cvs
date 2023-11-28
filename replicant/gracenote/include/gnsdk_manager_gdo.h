/** Gracenote SDK: GNSDK Manager GDO APIs and Key Definitions
  * Author:
  *   Copyright (c) 2012 Gracenote, Inc.
  *   
  *   This software may not be used in any way or distributed without
  *   permission. All rights reserved.
  *   
  *   Some code herein may be covered by USA and international patents.
*/

/** gnsdk_manager_gdo.h: GDO interface for the GNSDK Manager.
*/

#ifndef _GNSDK_MANAGER_GDO_H_
#define _GNSDK_MANAGER_GDO_H_

#ifdef __cplusplus
extern "C"{
#endif


/******************************************************************************
 * SDK Manager 'Gracenote Data Objects (GDOs)' APIs
 ******************************************************************************/


/** gnsdk_manager_gdo_get_context
  * Summary:
  *   Retrieves the context of a given GDO.
  * Parameters:
  *   gdo_handle:  [in] Handle to GDO to retrieve context for
  *   p_context:   [out] Pointer to receive context value for given GDO
  *                                                                    
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_gdo_get_context(
	gnsdk_gdo_handle_t	gdo_handle,
	gnsdk_cstr_t*		p_context
	);

/** gnsdk_manager_gdo_value_count
  * Summary:
  *   Retrieves the number of occurrences of the given value key that are
  *   available in a given GDO.
  * Parameters:
  *   gdo_handle:  [in] Handle to GDO
  *   value_key:   [in] <link !!MACROS_mgr_GDO_valuekeys, GDO value key> to
  *                count
  *   p_count:     [out] Pointer to integer that receives the count
  *                                                                        
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_gdo_value_count(
	gnsdk_gdo_handle_t	gdo_handle,
	gnsdk_cstr_t		value_key,
	gnsdk_uint32_t*		p_count
	);

/** gnsdk_manager_gdo_value_get
  * Summary:
  *   Retrieves a value from a given GDO for a given value key.
  * Parameters:
  *   gdo_handle:  [in] Handle to GDO
  *   value_key:   [in] <link !!MACROS_mgr_GDO_valuekeys, GDO value key> to
  *                retrieve
  *   ordinal:     [in] Retrieve n'th instance of the value (1\-based)
  *   p_value:     [out] Pointer to string to receive the GDO value
  *                                                                        
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_gdo_value_get(
	gnsdk_gdo_handle_t	gdo_handle,
	gnsdk_cstr_t		value_key,
	gnsdk_uint32_t		ordinal,
	gnsdk_cstr_t*		p_value
	);

/** gnsdk_manager_gdo_child_count
  * Summary:
  *   Retrieves the number of child contexts for a given child key that are
  *   available in a given GDO.
  * Parameters:
  *   gdo_handle:  [in] Handle to GDO
  *   child_key:   [in] <link !!MACROS_mgr_GDO_Childkeys, GDO child key> to
  *                count
  *   p_count:     [out] Pointer to integer that receives the count
  *                                                                        
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_gdo_child_count(
	gnsdk_gdo_handle_t	gdo_handle,
	gnsdk_cstr_t		child_key,
	gnsdk_uint32_t*		p_count
	);

/** gnsdk_manager_gdo_child_get
  * Summary:
  *   Retrieves a child GDO from a given GDO for a given context key.
  * Parameters:
  *   gdo_handle:    [in] Handle to GDO
  *   child_key:     [in] <link !!MACROS_mgr_GDO_Childkeys, GDO child key> to
  *                  retrieve
  *   ordinal:       [in] Retrieve n'th instance of the child (1 based)
  *   p_gdo_handle:  [out] Pointer to GDO handle to receive the GDO child
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_gdo_child_get(
	gnsdk_gdo_handle_t	gdo_handle,
	gnsdk_cstr_t		child_key,
	gnsdk_uint32_t		ordinal,
	gnsdk_gdo_handle_t* p_gdo_handle
	);

/** gnsdk_manager_gdo_serialize
  * Summary:
  *   Serializes a GDO into encrypted text for storage or transmission.
  * Parameters:
  *   gdo_handle:        [in] Handle to a GDO
  *   p_serialized_gdo:  [out] Pointer to string to receive serialized GDO
  *                      data
  *                                                                       
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_gdo_serialize(
	gnsdk_gdo_handle_t	gdo_handle,
	gnsdk_str_t*		p_serialized_gdo
	);

/** gnsdk_manager_gdo_deserialize
  * Summary:
  *   Reconstitutes a GDO handle from serialized GDO data.
  * Parameters:
  *   serialized_gdo:  [in] String of serialized GDO handle data
  *   p_gdo_handle:    [out] Pointer to receive a GDO handle
  *                                                             
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_gdo_deserialize(
	gnsdk_cstr_t		serialized_gdo,
	gnsdk_gdo_handle_t*	p_gdo_handle
	);

/** gnsdk_manager_gdo_render_to_xml
  * Summary:
  *   Renders contents of a GDO as an XML string.
  * Parameters:
  *   gdo_handle:    [in] Handle to a GDO to render
  *   render_flags:  [in] One or more <link !!MACROS_mgr_GDO_render_flags, GDO render flags>
  *                  to enable optional content in XML
  *   p_xml_render:  [out] Pointer to the string that receives the rendered XML
  *                                                                                         
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_gdo_render_to_xml(
	gnsdk_gdo_handle_t	gdo_handle,
	gnsdk_uint32_t		render_flags,
	gnsdk_str_t*		p_xml_render
	);

/** GNSDK_GDO_RENDER_XML_MINIMAL
  * Summary:
  *   Renders minimal metadata into the XML for a GDO.
*/
#define GNSDK_GDO_RENDER_XML_MINIMAL				0x0000
/** GNSDK_GDO_RENDER_XML_STANDARD
  * Summary:
  *   Renders standard metadata into the XML for a GDO.
*/
#define GNSDK_GDO_RENDER_XML_STANDARD				0x0001
/** GNSDK_GDO_RENDER_XML_CREDITS
  * Summary:
  *   Renders any credit metadata into the XML for a GDO.
*/
#define GNSDK_GDO_RENDER_XML_CREDITS				0x0002
/** GNSDK_GDO_RENDER_XML_SERIAL_GDOS
  * Summary:
  *   Renders any serialized GDO values into the XML for a GDO.
*/
#define GNSDK_GDO_RENDER_XML_SERIAL_GDOS			0x0010
/** GNSDK_GDO_RENDER_XML_PRODUCT_IDS
  * Summary:
  *   Renders any Gracenote Product ID values into the XML for a GDO.
*/
#define GNSDK_GDO_RENDER_XML_PRODUCT_IDS			0x0020
/** GNSDK_GDO_RENDER_XML_DISCOVER_SEEDS
  * Summary:
  *   Renders any Gracenote Discover Seed values into the XML for a GDO.
*/
#define GNSDK_GDO_RENDER_XML_DISCOVER_SEEDS			0x0040
/** GNSDK_GDO_RENDER_XML_RAW_TUIS
  * Summary:
  *   Renders any Gracenote TUI values into XML for a GDO.
*/
#define GNSDK_GDO_RENDER_XML_RAW_TUIS				0x0080
/** GNSDK_GDO_RENDER_XML_SUBMIT
  * Summary:
  *   Renders any Gracenote data supported for Submit editable GDOs into the
  *   XML for a GDO. This rendered data includes both the supported editable
  *   and non-editable data.                                                
*/
#define GNSDK_GDO_RENDER_XML_SUBMIT					0x0200
/** GNSDK_GDO_RENDER_XML_GENRE
  * Summary:
  *   Renders any Gracenote genre values into XML for a GDO.
*/
#define GNSDK_GDO_RENDER_XML_GENRE					0x1000
/** GNSDK_GDO_RENDER_XML_GENRE_META
  * Summary:
  *   Renders any Gracenote meta-genre (coarser) values into XML for a GDO.
*/
#define GNSDK_GDO_RENDER_XML_GENRE_META				0x2000
/** GNSDK_GDO_RENDER_XML_GENRE_MICRO
  * Summary:
  *   Renders any Gracenote micro-genre (finer) values into XML for a GDO.
*/
#define GNSDK_GDO_RENDER_XML_GENRE_MICRO			0x4000

/** GNSDK_GDO_RENDER_XML_DEFAULT
  * Summary:
  *   Renders the default metadata into the XML for a GDO (identifiers not
  *   included).                                                          
*/
#define GNSDK_GDO_RENDER_XML_DEFAULT				(GNSDK_GDO_RENDER_XML_STANDARD|GNSDK_GDO_RENDER_XML_GENRE)
/** GNSDK_GDO_RENDER_XML_FULL
  * Summary:
  *   Renders majority of metadata into the XML for a GDO (identifiers not
  *   included)                                                           
*/
#define GNSDK_GDO_RENDER_XML_FULL					(GNSDK_GDO_RENDER_XML_DEFAULT|GNSDK_GDO_RENDER_XML_CREDITS)


/** gnsdk_manager_gdo_set_locale
  * Summary:
  *   Applies lists to use for retrieving and rendering locale-related values.
  * Parameters:
  *   gdo_handle:     [in] Handle to a GDO
  *   locale_handle:  [in] Handle to a locale                                 
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_gdo_set_locale(
	gnsdk_gdo_handle_t		gdo_handle,
	gnsdk_locale_handle_t	locale_handle
	);

/** gnsdk_manager_gdo_set_transcription_lang
  * Summary:
  *   Specifies the spoken language to be used for any requested MediaVOCS
  *   transcription data.
  * Parameters:
  *   gdo_handle:       [in] Handle to a GDO
  *   spoken_language:  [in] An available <link !!MACROS_mgr_spoken_lang, spoken language>
  *                     value (GNSDK_SPOKEN_LANG_*)
  *                                                                                       
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_gdo_set_transcription_lang(
	gnsdk_gdo_handle_t	gdo_handle,
	gnsdk_cstr_t		spoken_language
	);

/** gnsdk_manager_gdo_addref
  * Summary:
  *   Increments reference count for a GDO handle.
  * Parameters:
  *   gdo_handle:  [in] Handle to a GDO
  *                                               
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_gdo_addref(
	gnsdk_gdo_handle_t	gdo_handle
	);

/** gnsdk_manager_gdo_release
  * Summary:
  *   Releases a GDO handle reference.
  * Parameters:
  *   gdo_handle:  [in] Handle to a GDO
  *                                    
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_gdo_release(
	gnsdk_gdo_handle_t	gdo_handle
	);

/** gnsdk_manager_gdo_create_from_id
  * Summary:
  *   Creates a GDO handle from identifier information.
  * Parameters:
  *   id_value:      [in] A value determined by the id_source parameter
  *   id_value_tag:  [in_opt] A value determined by the id_source parameter
  *   id_source:     [in] An available <link !!MACROS_mgr_ID_source, ID Source>
  *                  value
  *   p_gdo_handle:  [out] Pointer to receive a GDO handle
  *                                                                            
*/
gnsdk_error_t GNSDK_API
gnsdk_manager_gdo_create_from_id(
	gnsdk_cstr_t		id_value,
	gnsdk_cstr_t		id_value_tag,
	gnsdk_cstr_t		id_source,
	gnsdk_gdo_handle_t*	p_gdo_handle
	);

/** GNSDK_ID_SOURCE_ALBUM
  * Summary:
  *   Album source ID    
*/
#define GNSDK_ID_SOURCE_ALBUM						"gnsdk_id_source_album"
/** GNSDK_ID_SOURCE_TRACK
  * Summary:
  *   Track source ID    
*/
#define GNSDK_ID_SOURCE_TRACK						"gnsdk_id_source_track"
/** GNSDK_ID_SOURCE_CONTRIBUTOR
  * Summary:
  *   Contributor source ID    
*/
#define GNSDK_ID_SOURCE_CONTRIBUTOR					"gnsdk_id_source_contributor"
/** GNSDK_ID_SOURCE_VIDEO_WORK
  * Summary:
  *   Video work source ID    
*/
#define GNSDK_ID_SOURCE_VIDEO_WORK					"gnsdk_id_source_videowork"
/** GNSDK_ID_SOURCE_VIDEO_PRODUCT
  * Summary:
  *   Video product source ID    
*/
#define GNSDK_ID_SOURCE_VIDEO_PRODUCT				"gnsdk_id_source_videoprod"
/** GNSDK_ID_SOURCE_VIDEO_DISC
  * Summary:
  *   Video disc source ID    
*/
#define GNSDK_ID_SOURCE_VIDEO_DISC					"gnsdk_id_source_videodisc"
/** GNSDK_ID_SOURCE_VIDEO_SEASON
  * Summary:
  *   Season source ID          
*/
#define GNSDK_ID_SOURCE_VIDEO_SEASON				"gnsdk_id_source_videoseason"
/** GNSDK_ID_SOURCE_VIDEO_SERIES
  * Summary:
  *   Series source ID          
*/
#define GNSDK_ID_SOURCE_VIDEO_SERIES				"gnsdk_id_source_videoseries"
/** GNSDK_ID_SOURCE_LYRIC
  * Summary:
  *   Lyric source ID    
*/
#define GNSDK_ID_SOURCE_LYRIC						"gnsdk_id_source_lyric"
/** GNSDK_ID_SOURCE_CDDBID
  * Summary:
  *   CDDB identification source ID
*/
#define GNSDK_ID_SOURCE_CDDBID						"gnsdk_id_source_cddbid"


/*****************************************************************************
** GDO Keys common to multiple responses
*/

/** GNSDK_GDO_CONTEXT_EXTENDED_DATA
  * Summary:
  *   GDO is of extended data context.
*/
#define GNSDK_GDO_CONTEXT_EXTENDED_DATA				"gnsdk_ctx_extdata"
/** GNSDK_GDO_CONTEXT_EXTERNAL_ID
  * Summary:
  *   GDO is of external ID context.
*/
#define GNSDK_GDO_CONTEXT_EXTERNAL_ID				"gnsdk_ctx_xid"

/** GNSDK_GDO_CHILD_EXTENDED_DATA
  * Summary:
  *   Retrieves an extended data context (retrievable from most GDOs).
*/
#define GNSDK_GDO_CHILD_EXTENDED_DATA				GNSDK_GDO_CONTEXT_EXTENDED_DATA"!"
/** GNSDK_GDO_CHILD_EXTERNAL_ID
  * Summary:
  *   Retrieves an extended ID context (retrievable from most GDOs). This
  *   child GDO supports Link data.                                      
*/
#define GNSDK_GDO_CHILD_EXTERNAL_ID					GNSDK_GDO_CONTEXT_EXTERNAL_ID"!"

/** \ \ 
  * Summary:
  *   Retrieves a serialized GDO value for the current context.
*/
#define GNSDK_GDO_VALUE_EXTENDED_DATA				"gnsdk_val_extdata"
/** GNSDK_GDO_VALUE_DISCOVER_SEED
  * Summary:
  *   Retrieves a Discover Seed value from GDO.
*/
#define GNSDK_GDO_VALUE_DISCOVER_SEED				"gnsdk_val_discoverseed"
/** \ \ 
  * Summary:
  *   Retrieves a product ID for the current product from a GDO.
*/
#define GNSDK_GDO_VALUE_PRODUCTID					"gnsdk_val_productid"
/** \ \ 
  * Summary:
  *   Retrieves a TAGID value for a product from a GDO (Tag ID is a synonym
  *   for Product ID).                                                     
*/
#define GNSDK_GDO_VALUE_TAGID						GNSDK_GDO_VALUE_PRODUCTID

/** \ \ 
  * Summary:
  *   Retrieves a TUI value from a GDO.
*/
#define GNSDK_GDO_VALUE_TUI							"gnsdk_val_tui"
/** \ \ 
  * Summary:
  *   Retrieves a second TUI from a Works GDO, if that GDO can be accessed by
  *   two IDs. This TUI is used for matching partial Products objects to full
  *   Works objects.                                                         
*/
#define GNSDK_GDO_VALUE_TUI_MATCH_PRODUCT			"gnsdk_val_tui_match_product"
/** \ \ 
  * Summary:
  *   Retrieves a TUI Tag value from a GDO.
*/
#define GNSDK_GDO_VALUE_TUI_TAG						"gnsdk_val_tui_tag"

/** \ \ 
  * Summary:
  *   Retrieves the ordinal of the first result in the returned range.
*/
#define GNSDK_GDO_VALUE_RESPONSE_RANGE_START		"gnsdk_val_rangestart"
/** \ \ 
  * Summary:
  *   Retrieves the ordinal of the last result in the returned range.
*/
#define GNSDK_GDO_VALUE_RESPONSE_RANGE_END			"gnsdk_val_rangeend"
/** \ \ 
  * Summary:
  *   Retrieves the estimated total number of results possible.
*/
#define GNSDK_GDO_VALUE_RESPONSE_RANGE_TOTAL		"gnsdk_val_rangecount"


/** \ \ 
  * Summary:
  *   Retrieves an external ID value from a GDO.
*/
#define GNSDK_GDO_VALUE_EXTERNALID_VALUE			"gnsdk_val_xid_val"
/** \ \ 
  * Summary:
  *   Retrieves an external ID source value from a GDO.
*/
#define GNSDK_GDO_VALUE_EXTERNALID_SOURCE			"gnsdk_val_xid_source"
/** \ \ 
  * Summary:
  *   Retrieves an external ID type value from a GDO.
*/
#define GNSDK_GDO_VALUE_EXTERNALID_TYPE				"gnsdk_val_xid_type"


/** \ \ 
  * Summary:
  *   Retrieves an official name value from a GDO; used for credits and
  *   contributors.                                                    
*/
#define GNSDK_GDO_VALUE_NAME_DISPLAY				"gnsdk_val_name"
/** \ \ 
  * Summary:
  *   Retrieves the official original display language value for a returned
  *   GNSDK_GDO_VALUE_NAME_DISPLAY object.                                 
*/
#define GNSDK_GDO_VALUE_NAME_DISPLAY_LANGUAGE		"gnsdk_val_name_language"
/** \ \ 
  * Summary:
  *   Retrieves a family (or last) name value of a person from a GDO.
*/
#define GNSDK_GDO_VALUE_NAME_FAMILY					"gnsdk_val_name_family"
/** \ \ 
  * Summary:
  *   Retrieves a sortable name value of a person from a GDO.
*/
#define GNSDK_GDO_VALUE_NAME_SORTABLE				"gnsdk_val_name_sortable"
/** \ \ 
  * Summary:
  *   Retrieves the given (or first) name value of a person from a GDO.
*/
#define GNSDK_GDO_VALUE_NAME_GIVEN					"gnsdk_val_name_given"
/** \ \ 
  * Summary:
  *   Retrieve a prefix name value of a person from a GDO.
*/
#define GNSDK_GDO_VALUE_NAME_PREFIX					"gnsdk_val_name_prefix"

/** \ \ 
  * Summary:
  *   Retrieves an official regional variant of the name based on the GDO's
  *   locale language.                                                     
*/
#define GNSDK_GDO_VALUE_NAME_REGIONAL				"gnsdk_val_name_reg"
/** \ \ 
  * Summary:
  *   Retrieves the official regional variant language value for a returned
  *   GNSDK_GDO_VALUE_NAME_REGIONAL object.                                
*/
#define GNSDK_GDO_VALUE_NAME_REGIONAL_LANGUAGE		"gnsdk_val_name_reglang"

/** GNSDK_GDO_VALUE_BIOGRAPHY
  * Summary:
  *   Retrieves an available biography value from a GDO.
*/
#define GNSDK_GDO_VALUE_BIOGRAPHY					"gnsdk_val_biography"
/** GNSDK_GDO_VALUE_BIOGRAPHY_LANGUAGE
  * Summary:
  *   Retrieves the available language value for a returned
  *   GNSDK_GDO_VALUE_BIOGRAPHY object.                    
*/
#define GNSDK_GDO_VALUE_BIOGRAPHY_LANGUAGE			"gnsdk_val_biography_language"
/** GNSDK_GDO_VALUE_BIRTH_DATE
  * Summary:
  *   Retrieves a birth date value from a GDO.
*/
#define GNSDK_GDO_VALUE_BIRTH_DATE					"gnsdk_val_birth_date"
/** GNSDK_GDO_VALUE_BIRTH_PLACE
  * Summary:
  *   Retrieves a birth place value from a GDO.
*/
#define GNSDK_GDO_VALUE_BIRTH_PLACE					"gnsdk_val_birth_place"
/** GNSDK_GDO_VALUE_DEATH_DATE
  * Summary:
  *   Retrieves a death date value from a GDO.
*/
#define GNSDK_GDO_VALUE_DEATH_DATE					"gnsdk_val_death_date"
/** GNSDK_GDO_VALUE_DEATH_PLACE
  * Summary:
  *   Retrieves a death place value from a GDO.
*/
#define GNSDK_GDO_VALUE_DEATH_PLACE					"gnsdk_val_death_place"

/*****************************************************************************
** GDO Keys specific to ALBUM responses
*/

/*
 * GDO Contexts
 * One of these contexts will be the string returned by
 * gnsdk_manager_gdo_get_context() for any returned GDO
 */

/** GNSDK_GDO_CONTEXT_RESPONSE_ALBUM
  * Summary:
  *   GDO is of album response context.
*/
#define GNSDK_GDO_CONTEXT_RESPONSE_ALBUM			"gnsdk_ctx_response_album"
/** GNSDK_GDO_CONTEXT_RESPONSE_TRACK
  * Summary:
  *   GDO is of track response context.
*/
#define GNSDK_GDO_CONTEXT_RESPONSE_TRACK			"gnsdk_ctx_response_track"
/** GNSDK_GDO_CONTEXT_ALBUM
  * Summary:
  *   GDO is of album context.
*/
#define GNSDK_GDO_CONTEXT_ALBUM						"gnsdk_ctx_album"
/** GNSDK_GDO_CONTEXT_TRACK
  * Summary:
  *   GDO is of track context.
*/
#define GNSDK_GDO_CONTEXT_TRACK						"gnsdk_ctx_track"
/** GNSDK_GDO_CONTEXT_CREDIT
  * Summary:
  *   GDO is of a Credit context.
*/
#define GNSDK_GDO_CONTEXT_CREDIT					"gnsdk_ctx_credit"
/** GNSDK_GDO_CONTEXT_AUDIO_WORK
  * Summary:
  *   GDO is of an audio work context.
*/
#define GNSDK_GDO_CONTEXT_AUDIO_WORK				"gnsdk_ctx_audio_work"

/*
 * GDO Context Keys
 * For retrieving specific contexts from a GDO.
 * Actual context returned will be one of the defined contexts above.
 */

/** GNSDK_GDO_CHILD_ALBUM
  * Summary:
  *   Retrieves a child album context.
*/
#define GNSDK_GDO_CHILD_ALBUM						GNSDK_GDO_CONTEXT_ALBUM"!"
/** GNSDK_GDO_CHILD_TRACK
  * Summary:
  *   Retrieves a child track context.
*/
#define GNSDK_GDO_CHILD_TRACK						GNSDK_GDO_CONTEXT_TRACK"!"
/** GNSDK_GDO_CHILD_TRACK_BY_NUMBER
  * Summary:
  *   Retrieves a child track context matching the given track number.
*/
#define GNSDK_GDO_CHILD_TRACK_BY_NUMBER				GNSDK_GDO_CONTEXT_TRACK"!number"
/** GNSDK_GDO_CHILD_TRACK_MATCHED
  * Summary:
  *   Retrieves a child track context that was matched.
*/
#define GNSDK_GDO_CHILD_TRACK_MATCHED				GNSDK_GDO_CONTEXT_TRACK"!matching"

/** GNSDK_GDO_CHILD_AUDIO_WORK
  * Summary:
  *   Retrieves a child audio work context.
*/
#define GNSDK_GDO_CHILD_AUDIO_WORK					GNSDK_GDO_CONTEXT_AUDIO_WORK"!"
/** GNSDK_GDO_CHILD_CREDIT
  * Summary:
  *   Retrieves a child credit context.
*/
#define GNSDK_GDO_CHILD_CREDIT						GNSDK_GDO_CONTEXT_CREDIT"!"

/** GNSDK_GDO_CHILD_CREDIT_SERIES
  * Summary:
  *   Retrieve a child credit of a Series context.
*/
#define GNSDK_GDO_CHILD_CREDIT_SERIES				GNSDK_GDO_CONTEXT_CREDIT"!series"
/** GNSDK_GDO_CHILD_CREDIT_SEASON
  * Summary:
  *   Retrieve a child credit of a Season context.
*/
#define GNSDK_GDO_CHILD_CREDIT_SEASON				GNSDK_GDO_CONTEXT_CREDIT"!season"
/** GNSDK_GDO_CHILD_CREDIT_WORK_NONSERIES
  * Summary:
  *   Retrieve a child credit of a Works non-Series context.
*/
#define GNSDK_GDO_CHILD_CREDIT_WORK_NONSERIES		GNSDK_GDO_CONTEXT_CREDIT"!worknonseries"

/** \Author credit
*/
#define GNSDK_GDO_CHILD_CREDIT_AUTHOR				GNSDK_GDO_CONTEXT_CREDIT"!author"
/** \ \ 
  * Summary:
  *   Composer credit
*/
#define GNSDK_GDO_CHILD_CREDIT_COMPOSER				GNSDK_GDO_CONTEXT_CREDIT"!3"
/** Copyright credit
*/
#define GNSDK_GDO_CHILD_CREDIT_COPYRIGHT			GNSDK_GDO_CONTEXT_CREDIT"!4"
/** Librettist credit
*/
#define GNSDK_GDO_CHILD_CREDIT_LIBRETTIST			GNSDK_GDO_CONTEXT_CREDIT"!5"
/** Lyricist credit
*/
#define GNSDK_GDO_CHILD_CREDIT_LYRICIST				GNSDK_GDO_CONTEXT_CREDIT"!6"
/** Publisher credit
*/
#define GNSDK_GDO_CHILD_CREDIT_PUBLISHER			GNSDK_GDO_CONTEXT_CREDIT"!7"
/** Score credit
*/
#define GNSDK_GDO_CHILD_CREDIT_SCORE				GNSDK_GDO_CONTEXT_CREDIT"!8"
/** Songwriter credit
*/
#define GNSDK_GDO_CHILD_CREDIT_SONGWRITER			GNSDK_GDO_CONTEXT_CREDIT"!9"
/** Arranger credit
*/
#define GNSDK_GDO_CHILD_CREDIT_ARRANGER				GNSDK_GDO_CONTEXT_CREDIT"!11"
/** Conductor credit
*/
#define GNSDK_GDO_CHILD_CREDIT_CONDUCTOR			GNSDK_GDO_CONTEXT_CREDIT"!12"
/** Director credit
*/
#define GNSDK_GDO_CHILD_CREDIT_DIRECTOR				GNSDK_GDO_CONTEXT_CREDIT"!director"
/** Orchestrator credit
*/
#define GNSDK_GDO_CHILD_CREDIT_ORCHESTRATOR			GNSDK_GDO_CONTEXT_CREDIT"!14"
/** Percussion drums credit
*/
#define GNSDK_GDO_CHILD_CREDIT_PERCUSSION_DRUMS		GNSDK_GDO_CONTEXT_CREDIT"!16"
/** Drum programming credit
  *                        
*/
#define GNSDK_GDO_CHILD_CREDIT_DRUM_PROGRAMMING		GNSDK_GDO_CONTEXT_CREDIT"!17"
/** Gong chimes credit
*/
#define GNSDK_GDO_CHILD_CREDIT_GONG_CHIMES			GNSDK_GDO_CONTEXT_CREDIT"!18"
/** Marimba credit
*/
#define GNSDK_GDO_CHILD_CREDIT_MARIMBA				GNSDK_GDO_CONTEXT_CREDIT"!19"
/** Metallophone credit
*/
#define GNSDK_GDO_CHILD_CREDIT_METALLOPHONE			GNSDK_GDO_CONTEXT_CREDIT"!20"
/** Various percussion instruments credit
*/
#define GNSDK_GDO_CHILD_CREDIT_PERCUSSION_VARIOUS	GNSDK_GDO_CONTEXT_CREDIT"!21"
/** Steel drum credit
*/
#define GNSDK_GDO_CHILD_CREDIT_STEEL_DRUM			GNSDK_GDO_CONTEXT_CREDIT"!22"
/** Tabla credit
*/
#define GNSDK_GDO_CHILD_CREDIT_TABLA				GNSDK_GDO_CONTEXT_CREDIT"!23"
/** Timpani credit
*/
#define GNSDK_GDO_CHILD_CREDIT_TIMPANI				GNSDK_GDO_CONTEXT_CREDIT"!24"
/** Vibraphone credit
*/
#define GNSDK_GDO_CHILD_CREDIT_VIBRAPHONE			GNSDK_GDO_CONTEXT_CREDIT"!25"
/** Washboard credit
*/
#define GNSDK_GDO_CHILD_CREDIT_WASHBOARD			GNSDK_GDO_CONTEXT_CREDIT"!26"
/** Xylophone credit
*/
#define GNSDK_GDO_CHILD_CREDIT_XYLOPHONE			GNSDK_GDO_CONTEXT_CREDIT"!27"
/** Acoustic bass credit
*/
#define GNSDK_GDO_CHILD_CREDIT_ACOUSTIC_BASS		GNSDK_GDO_CONTEXT_CREDIT"!29"
/** Acoustic guitar credit
*/
#define GNSDK_GDO_CHILD_CREDIT_ACOUSTIC_GUITAR		GNSDK_GDO_CONTEXT_CREDIT"!30"
/** Banjo credit
*/
#define GNSDK_GDO_CHILD_CREDIT_BANJO				GNSDK_GDO_CONTEXT_CREDIT"!31"
/** Bass guitar credit
*/
#define GNSDK_GDO_CHILD_CREDIT_BASS_GUITAR			GNSDK_GDO_CONTEXT_CREDIT"!32"
/** Cittern credit
*/
#define GNSDK_GDO_CHILD_CREDIT_CITTERN				GNSDK_GDO_CONTEXT_CREDIT"!33"
/** Chapman stick credit
*/
#define GNSDK_GDO_CHILD_CREDIT_CHAPMAN_STICK		GNSDK_GDO_CONTEXT_CREDIT"!34"
/** Electric guitar credit
*/
#define GNSDK_GDO_CHILD_CREDIT_ELECTRIC_GUITAR		GNSDK_GDO_CONTEXT_CREDIT"!35"
/** Koto credit
*/
#define GNSDK_GDO_CHILD_CREDIT_KOTO					GNSDK_GDO_CONTEXT_CREDIT"!36"
/** Lute credit
*/
#define GNSDK_GDO_CHILD_CREDIT_LUTE					GNSDK_GDO_CONTEXT_CREDIT"!37"
/** Lyre credit
*/
#define GNSDK_GDO_CHILD_CREDIT_LYRE					GNSDK_GDO_CONTEXT_CREDIT"!38"
/** Mandolin credit
*/
#define GNSDK_GDO_CHILD_CREDIT_MANDOLIN				GNSDK_GDO_CONTEXT_CREDIT"!39"
/** Pedal steel guitar credit
*/
#define GNSDK_GDO_CHILD_CREDIT_PEDAL_STEEL_GUITAR	GNSDK_GDO_CONTEXT_CREDIT"!40"
/** Samisen credit
*/
#define GNSDK_GDO_CHILD_CREDIT_SAMISEN				GNSDK_GDO_CONTEXT_CREDIT"!41"
/** Sitar credit
*/
#define GNSDK_GDO_CHILD_CREDIT_SITAR				GNSDK_GDO_CONTEXT_CREDIT"!42"
/** Tambura credit
*/
#define GNSDK_GDO_CHILD_CREDIT_TAMBURA				GNSDK_GDO_CONTEXT_CREDIT"!43"
/** Ukulele credit
*/
#define GNSDK_GDO_CHILD_CREDIT_UKULELE				GNSDK_GDO_CONTEXT_CREDIT"!44"
/** Vina credit
*/
#define GNSDK_GDO_CHILD_CREDIT_VINA					GNSDK_GDO_CONTEXT_CREDIT"!45"
/* Keyboard Credits */
#define GNSDK_GDO_CHILD_CREDIT_CLAVICHORD			GNSDK_GDO_CONTEXT_CREDIT"!47"
/** Harpsichord credit
*/
#define GNSDK_GDO_CHILD_CREDIT_HARPSICHORD			GNSDK_GDO_CONTEXT_CREDIT"!48"
/** Various keyboards credit
*/
#define GNSDK_GDO_CHILD_CREDIT_KEYBOARDS_VARIOUS	GNSDK_GDO_CONTEXT_CREDIT"!49"
/** Harmonium credit
*/
#define GNSDK_GDO_CHILD_CREDIT_HARMONIUM			GNSDK_GDO_CONTEXT_CREDIT"!50"
/** Organ credit
*/
#define GNSDK_GDO_CHILD_CREDIT_ORGAN				GNSDK_GDO_CONTEXT_CREDIT"!51"
/** Piano credit
*/
#define GNSDK_GDO_CHILD_CREDIT_PIANO				GNSDK_GDO_CONTEXT_CREDIT"!52"
/** Synthesizer credit
*/
#define GNSDK_GDO_CHILD_CREDIT_SYNTHESIZER			GNSDK_GDO_CONTEXT_CREDIT"!53"
/** Virginal credit
*/
#define GNSDK_GDO_CHILD_CREDIT_VIRGINAL				GNSDK_GDO_CONTEXT_CREDIT"!54"
/** Accordian credit
*/
#define GNSDK_GDO_CHILD_CREDIT_ACCORDIAN				GNSDK_GDO_CONTEXT_CREDIT"!56"
/** Bagpipe credit
*/
#define GNSDK_GDO_CHILD_CREDIT_BAGPIPE					GNSDK_GDO_CONTEXT_CREDIT"!57"
/** Concertina credit
*/
#define GNSDK_GDO_CHILD_CREDIT_CONCERTINA				GNSDK_GDO_CONTEXT_CREDIT"!58"
/** Dulcimer credit
*/
#define GNSDK_GDO_CHILD_CREDIT_DULCIMER					GNSDK_GDO_CONTEXT_CREDIT"!59"
/** Harmonica credit
*/
#define GNSDK_GDO_CHILD_CREDIT_HARMONICA				GNSDK_GDO_CONTEXT_CREDIT"!60"
/** Historical instrument credit
*/
#define GNSDK_GDO_CHILD_CREDIT_HISTORICAL_INSTRUMENT	GNSDK_GDO_CONTEXT_CREDIT"!61"
/** Jews harp credit
*/
#define GNSDK_GDO_CHILD_CREDIT_JEWS_HARP				GNSDK_GDO_CONTEXT_CREDIT"!62"
/** Kazoo credit
*/
#define GNSDK_GDO_CHILD_CREDIT_KAZOO					GNSDK_GDO_CONTEXT_CREDIT"!63"
/** Penny whistle credit
*/
#define GNSDK_GDO_CHILD_CREDIT_PENNY_WHISTLE			GNSDK_GDO_CONTEXT_CREDIT"!64"
/** Regional instrument credit
*/
#define GNSDK_GDO_CHILD_CREDIT_REGIONAL_INSTRUMENT		GNSDK_GDO_CONTEXT_CREDIT"!65"
/** DJ scratch credit
*/
#define GNSDK_GDO_CHILD_CREDIT_DJ_SCRATCH				GNSDK_GDO_CONTEXT_CREDIT"!66"
/** Unlisted instrument credit
*/
#define GNSDK_GDO_CHILD_CREDIT_UNLISTED_INSTRUMENT		GNSDK_GDO_CONTEXT_CREDIT"!67"
/** Assistant engineer credit
*/
#define GNSDK_GDO_CHILD_CREDIT_ASSISTANT_ENGINEER	GNSDK_GDO_CONTEXT_CREDIT"!69"
/** Assistant producer credit
*/
#define GNSDK_GDO_CHILD_CREDIT_ASSISTANT_PRODUCER	GNSDK_GDO_CONTEXT_CREDIT"!70"
/** Coproducer credit
*/
#define GNSDK_GDO_CHILD_CREDIT_COPRODUCER			GNSDK_GDO_CONTEXT_CREDIT"!71"
/** Engineer credit
*/
#define GNSDK_GDO_CHILD_CREDIT_ENGINEER				GNSDK_GDO_CONTEXT_CREDIT"!72"
/** Executive producer credit
*/
#define GNSDK_GDO_CHILD_CREDIT_EXECUTIVE_PRODUCER	GNSDK_GDO_CONTEXT_CREDIT"!executiveproduder"
/** Mastering credit
*/
#define GNSDK_GDO_CHILD_CREDIT_MASTERING			GNSDK_GDO_CONTEXT_CREDIT"!74"
/** Mastering location credit
*/
#define GNSDK_GDO_CHILD_CREDIT_MASTERING_LOCATION	GNSDK_GDO_CONTEXT_CREDIT"!75"
/** Mixing credit
*/
#define GNSDK_GDO_CHILD_CREDIT_MIXING				GNSDK_GDO_CONTEXT_CREDIT"!76"
/** Mixing location credit
*/
#define GNSDK_GDO_CHILD_CREDIT_MIXING_LOCATION		GNSDK_GDO_CONTEXT_CREDIT"!77"
/** Producer credit
*/
#define GNSDK_GDO_CHILD_CREDIT_PRODUCER				GNSDK_GDO_CONTEXT_CREDIT"!producer"
/** Programming credit
*/
#define GNSDK_GDO_CHILD_CREDIT_PROGRAMMING			GNSDK_GDO_CONTEXT_CREDIT"!79"
/** Recording location credit
*/
#define GNSDK_GDO_CHILD_CREDIT_RECORDING_LOCATION	GNSDK_GDO_CONTEXT_CREDIT"!80"
/** Remixing credit
*/
#define GNSDK_GDO_CHILD_CREDIT_REMIXING				GNSDK_GDO_CONTEXT_CREDIT"!147"
/** Alto saxophone credit
*/
#define GNSDK_GDO_CHILD_CREDIT_ALTO_SAXOPHONE			GNSDK_GDO_CONTEXT_CREDIT"!82"
/** Baritone saxophone credit
*/
#define GNSDK_GDO_CHILD_CREDIT_BARITONE_SAXOPHONE		GNSDK_GDO_CONTEXT_CREDIT"!83"
/** Bass saxophone credit
*/
#define GNSDK_GDO_CHILD_CREDIT_BASS_SAXOPHONE			GNSDK_GDO_CONTEXT_CREDIT"!84"
/** Contrabass saxophone credit
*/
#define GNSDK_GDO_CHILD_CREDIT_CONTRABASS_SAXOPHONE		GNSDK_GDO_CONTEXT_CREDIT"!85"
/** Saxophone credit
*/
#define GNSDK_GDO_CHILD_CREDIT_SAXOPHONE				GNSDK_GDO_CONTEXT_CREDIT"!86"
/** Sopranino saxophone credit
*/
#define GNSDK_GDO_CHILD_CREDIT_SOPRANINO_SAXOPHONE		GNSDK_GDO_CONTEXT_CREDIT"!87"
/** Soprano saxophone credit
*/
#define GNSDK_GDO_CHILD_CREDIT_SOPRANO_SAXOPHONE		GNSDK_GDO_CONTEXT_CREDIT"!88"
/** Subcontrabass saxophone credit
*/
#define GNSDK_GDO_CHILD_CREDIT_SUBCONTRABASS_SAXOPHONE	GNSDK_GDO_CONTEXT_CREDIT"!89"
/** Tenor saxophone credit
*/
#define GNSDK_GDO_CHILD_CREDIT_TENOR_SAXOPHONE			GNSDK_GDO_CONTEXT_CREDIT"!90"
/** Cello credit
*/
#define GNSDK_GDO_CHILD_CREDIT_CELLO				GNSDK_GDO_CONTEXT_CREDIT"!92"
/** Double bass credit
*/
#define GNSDK_GDO_CHILD_CREDIT_DOUBLE_BASS			GNSDK_GDO_CONTEXT_CREDIT"!93"
/** Fiddle credit
*/
#define GNSDK_GDO_CHILD_CREDIT_FIDDLE				GNSDK_GDO_CONTEXT_CREDIT"!94"
/** Harp credit
*/
#define GNSDK_GDO_CHILD_CREDIT_HARP					GNSDK_GDO_CONTEXT_CREDIT"!95"
/** Viol credit
*/
#define GNSDK_GDO_CHILD_CREDIT_VIOL					GNSDK_GDO_CONTEXT_CREDIT"!96"
/** Viola credit
*/
#define GNSDK_GDO_CHILD_CREDIT_VIOLA				GNSDK_GDO_CONTEXT_CREDIT"!97"
/** Violin credit
*/
#define GNSDK_GDO_CHILD_CREDIT_VIOLIN				GNSDK_GDO_CONTEXT_CREDIT"!98"
/** Zither credit
*/
#define GNSDK_GDO_CHILD_CREDIT_ZITHER				GNSDK_GDO_CONTEXT_CREDIT"!99"
/** Actor credit
*/
#define GNSDK_GDO_CHILD_CREDIT_ACTOR				GNSDK_GDO_CONTEXT_CREDIT"!actor"
/** Alto voice credit
*/
#define GNSDK_GDO_CHILD_CREDIT_ALTO					GNSDK_GDO_CONTEXT_CREDIT"!102"
/** Baritone voice credit
*/
#define GNSDK_GDO_CHILD_CREDIT_BARITONE				GNSDK_GDO_CONTEXT_CREDIT"!103"
/** Basso voice credit
*/
#define GNSDK_GDO_CHILD_CREDIT_BASSO				GNSDK_GDO_CONTEXT_CREDIT"!104"
/** Contralto voice credit
*/
#define GNSDK_GDO_CHILD_CREDIT_CONTRALTO			GNSDK_GDO_CONTEXT_CREDIT"!105"
/** Counter tenor voice credit
*/
#define GNSDK_GDO_CHILD_CREDIT_COUNTER_TENOR		GNSDK_GDO_CONTEXT_CREDIT"!106"
/** Mezzo soprano voice credit
*/
#define GNSDK_GDO_CHILD_CREDIT_MEZZO_SOPRANO		GNSDK_GDO_CONTEXT_CREDIT"!107"
/** Narrator voice credit
*/
#define GNSDK_GDO_CHILD_CREDIT_NARRATOR				GNSDK_GDO_CONTEXT_CREDIT"!narrator"
/** Rap voice credit
*/
#define GNSDK_GDO_CHILD_CREDIT_RAP					GNSDK_GDO_CONTEXT_CREDIT"!109"
/** Reader voice credit
*/
#define GNSDK_GDO_CHILD_CREDIT_READER				GNSDK_GDO_CONTEXT_CREDIT"!110"
/** Soprano voice credit
*/
#define GNSDK_GDO_CHILD_CREDIT_SOPRANO				GNSDK_GDO_CONTEXT_CREDIT"!111"
/** Tenor voice credit
*/
#define GNSDK_GDO_CHILD_CREDIT_TENOR				GNSDK_GDO_CONTEXT_CREDIT"!112"
/** Vocals credit
*/
#define GNSDK_GDO_CHILD_CREDIT_VOCALS				GNSDK_GDO_CONTEXT_CREDIT"!113"
/** Backing vocals credit
*/
#define GNSDK_GDO_CHILD_CREDIT_VOCALS_BACKING		GNSDK_GDO_CONTEXT_CREDIT"!114"
/** Lead vocals credit
*/
#define GNSDK_GDO_CHILD_CREDIT_VOCALS_LEAD			GNSDK_GDO_CONTEXT_CREDIT"!115"
/** Other voice credit
*/
#define GNSDK_GDO_CHILD_CREDIT_VOICE_OTHER			GNSDK_GDO_CONTEXT_CREDIT"!116"
/** Spoken voice credit
*/
#define GNSDK_GDO_CHILD_CREDIT_VOICE_SPOKEN			GNSDK_GDO_CONTEXT_CREDIT"!117"
/** Bugle credit
*/
#define GNSDK_GDO_CHILD_CREDIT_BUGLE				GNSDK_GDO_CONTEXT_CREDIT"!119"
/** Cornet credit
*/
#define GNSDK_GDO_CHILD_CREDIT_CORNET				GNSDK_GDO_CONTEXT_CREDIT"!120"
/** English horn credit
*/
#define GNSDK_GDO_CHILD_CREDIT_ENGLISH_HORN			GNSDK_GDO_CONTEXT_CREDIT"!121"
/** Euphonium credit
*/
#define GNSDK_GDO_CHILD_CREDIT_EUPHONIUM			GNSDK_GDO_CONTEXT_CREDIT"!122"
/** French horn credit
*/
#define GNSDK_GDO_CHILD_CREDIT_FRENCH_HORN			GNSDK_GDO_CONTEXT_CREDIT"!123"
/** Horn credit
*/
#define GNSDK_GDO_CHILD_CREDIT_HORN					GNSDK_GDO_CONTEXT_CREDIT"!124"
/** Sousaphone credit
*/
#define GNSDK_GDO_CHILD_CREDIT_SOUSAPHONE			GNSDK_GDO_CONTEXT_CREDIT"!125"
/** Trombone credit
*/
#define GNSDK_GDO_CHILD_CREDIT_TROMBONE				GNSDK_GDO_CONTEXT_CREDIT"!126"
/** Trombone valve credit
*/
#define GNSDK_GDO_CHILD_CREDIT_TROMBONE_VALVE		GNSDK_GDO_CONTEXT_CREDIT"!127"
/** Trumpet credit
*/
#define GNSDK_GDO_CHILD_CREDIT_TRUMPET				GNSDK_GDO_CONTEXT_CREDIT"!128"
/** Trumpet slide credit
*/
#define GNSDK_GDO_CHILD_CREDIT_TRUMPET_SLIDE		GNSDK_GDO_CONTEXT_CREDIT"!129"
/** Tuba credit
*/
#define GNSDK_GDO_CHILD_CREDIT_TUBA					GNSDK_GDO_CONTEXT_CREDIT"!130"
/** Bass clarinet credit
*/
#define GNSDK_GDO_CHILD_CREDIT_BASS_CLARINET		GNSDK_GDO_CONTEXT_CREDIT"!132"
/** Bassoon credit
*/
#define GNSDK_GDO_CHILD_CREDIT_BASSOON				GNSDK_GDO_CONTEXT_CREDIT"!133"
/** Clarinet credit
*/
#define GNSDK_GDO_CHILD_CREDIT_CLARINET				GNSDK_GDO_CONTEXT_CREDIT"!134"
/** Contrabassoon credit
*/
#define GNSDK_GDO_CHILD_CREDIT_CONTRABASSOON		GNSDK_GDO_CONTEXT_CREDIT"!135"
/** Cor anglais credit
*/
#define GNSDK_GDO_CHILD_CREDIT_COR_ANGLAIS			GNSDK_GDO_CONTEXT_CREDIT"!136"
/** Didgeridoo credit
*/
#define GNSDK_GDO_CHILD_CREDIT_DIDGERIDOO			GNSDK_GDO_CONTEXT_CREDIT"!137"
/** Fife credit
*/
#define GNSDK_GDO_CHILD_CREDIT_FIFE					GNSDK_GDO_CONTEXT_CREDIT"!138"
/** Flageolet credit
*/
#define GNSDK_GDO_CHILD_CREDIT_FLAGEOLET			GNSDK_GDO_CONTEXT_CREDIT"!139"
/** Flute credit
*/
#define GNSDK_GDO_CHILD_CREDIT_FLUTE				GNSDK_GDO_CONTEXT_CREDIT"!140"
/** Oboe credit
*/
#define GNSDK_GDO_CHILD_CREDIT_OBOE					GNSDK_GDO_CONTEXT_CREDIT"!141"
/** Oboe d'amore credit
*/
#define GNSDK_GDO_CHILD_CREDIT_OBOE_DAMORE			GNSDK_GDO_CONTEXT_CREDIT"!142"
/** Panpipes credit
*/
#define GNSDK_GDO_CHILD_CREDIT_PANPIPES				GNSDK_GDO_CONTEXT_CREDIT"!143"
/** Piccolo credit
*/
#define GNSDK_GDO_CHILD_CREDIT_PICCOLO				GNSDK_GDO_CONTEXT_CREDIT"!144"
/** Recorder credit
*/
#define GNSDK_GDO_CHILD_CREDIT_RECORDER				GNSDK_GDO_CONTEXT_CREDIT"!145"
/** Shawm credit
*/
#define GNSDK_GDO_CHILD_CREDIT_SHAWM				GNSDK_GDO_CONTEXT_CREDIT"!146"
/** Band credit
*/
#define GNSDK_GDO_CHILD_CREDIT_BAND					GNSDK_GDO_CONTEXT_CREDIT"!149"
/** Ensemble chamber credit
*/
#define GNSDK_GDO_CHILD_CREDIT_CHAMBER_ENSEMBLE		GNSDK_GDO_CONTEXT_CREDIT"!150"
/** Chorus credit
*/
#define GNSDK_GDO_CHILD_CREDIT_CHORUS				GNSDK_GDO_CONTEXT_CREDIT"!151"
/** Orchestra credit
*/
#define GNSDK_GDO_CHILD_CREDIT_ORCHESTRA			GNSDK_GDO_CONTEXT_CREDIT"!152"
/** Vocal ensemble credit
*/
#define GNSDK_GDO_CHILD_CREDIT_VOCAL_ENSEMBLE		GNSDK_GDO_CONTEXT_CREDIT"!153"
/** Peformer credit
*/
#define GNSDK_GDO_CHILD_CREDIT_PERFORMER			GNSDK_GDO_CONTEXT_CREDIT"!154"

/*
 * GDO Value Keys
 * For retrieving values from GDO. Values may or may not exist, or have
 * different values depending on the context of the GDO.
 */

/** \ \ 
  * Summary:
  *   Retrieves a language value from a GDO as the ISO code (for example,
  *   "eng").                                                            
*/
#define GNSDK_GDO_VALUE_PACKAGE_LANGUAGE				"gnsdk_val_pkglang"
/** \ \ 
  * Summary:
  *   Retrieves the package language as text (for example, "English").
*/
#define GNSDK_GDO_VALUE_PACKAGE_LANGUAGE_DISPLAY		"gnsdk_val_pkglang_display"
/** \ \ 
  * Summary:
  *   Retrieves an album, track, or lyric display title value from a GDO.
*/
#define GNSDK_GDO_VALUE_TITLE_DISPLAY				"gnsdk_val_title_display"
/** \ \ 
  * Summary:
  *   Retrieves the available language value for a returned
  *   GNSDK_GDO_VALUE_TITLE_DISPLAY object.                
*/
#define GNSDK_GDO_VALUE_TITLE_DISPLAY_LANGUAGE		"gnsdk_val_title_display_language"

/** \ \ 
  * Summary:
  *   Retrieves a sortable title value from a GDO.
*/
#define GNSDK_GDO_VALUE_TITLE_SORTABLE				"gnsdk_val_title_sortable"
/** \ \ 
  * Summary:
  *   Retrieves the available language value for a returned
  *   GNSDK_GDO_VALUE_TITLE_SORTABLE object.               
*/
#define GNSDK_GDO_VALUE_TITLE_SORTABLE_LANGUAGE		"gnsdk_val_title_sortable_language"
/** GNSDK_GDO_VALUE_TITLE_TLS
  * Summary:
  *   Retrieves a Gracenote Classical Music Initiative Three Line Solution
  *   (TLS) title value from a GDO.                                       
*/
#define GNSDK_GDO_VALUE_TITLE_TLS					"gnsdk_val_title_tls"
/** GNSDK_GDO_VALUE_LYRICIST
  * Summary:
  *   Retrieves a lyricist value from a GDO.
*/
#define GNSDK_GDO_VALUE_LYRICIST					"gnsdk_val_lyricist"
/** GNSDK_GDO_VALUE_PUBLISHER
  * Summary:
  *   Retrieves a publisher value from a GDO.
*/
#define GNSDK_GDO_VALUE_PUBLISHER					"gnsdk_val_publisher"
/** GNSDK_GDO_VALUE_DATE
  * Summary:
  *   Retrieves a date value from a GDO.
*/
#define GNSDK_GDO_VALUE_DATE						"gnsdk_val_date"
/** Summary:
  *   Retrieves a release date value from a GDO.
*/
#define GNSDK_GDO_VALUE_DATE_RELEASE				"gnsdk_val_daterel"
/** GNSDK_GDO_VALUE_ALBUM_LABEL
  * Summary:
  *   Retrieves an album label value from a GDO.
*/
#define GNSDK_GDO_VALUE_ALBUM_LABEL					"gnsdk_val_albumlabel"
/** GNSDK_GDO_VALUE_ALBUM_TRACK_COUNT
  * Summary:
  *   Retrieves the total number of tracks on an album.
*/
#define GNSDK_GDO_VALUE_ALBUM_TRACK_COUNT			"gnsdk_val_albumtrackcnt"
/** GNSDK_GDO_VALUE_ALBUM_TOTAL_IN_SET
  * Summary:
  *   Retrieves the total discs in set value from a GDO.
*/
#define GNSDK_GDO_VALUE_ALBUM_TOTAL_IN_SET			"gnsdk_val_albumtotalset"
/** GNSDK_GDO_VALUE_ALBUM_DISC_IN_SET
  * Summary:
  *   Retrieves a disc number value from a GDO.
*/
#define GNSDK_GDO_VALUE_ALBUM_DISC_IN_SET			"gnsdk_val_albumdiscset"
/** GNSDK_GDO_VALUE_ALBUM_COMPILATION
  * Summary:
  *   Retrieves an album compilation value from a GDO.
*/
#define GNSDK_GDO_VALUE_ALBUM_COMPILATION			"gnsdk_val_albumcompflag"
/** GNSDK_GDO_VALUE_TRACK_NUMBER
  * Summary:
  *   Retrieves a track number value from a GDO.
*/
#define GNSDK_GDO_VALUE_TRACK_NUMBER				"gnsdk_val_tracknumber"
/** GNSDK_GDO_VALUE_TRACK_MATCHED_NUM
  * Summary:
  *   Retrieves a numeric value indicating the track number of the matching 
  *   track (if applicable) for the source query
*/
#define GNSDK_GDO_VALUE_TRACK_MATCHED_NUM			"gnsdk_val_track_matched"
/** GNSDK_GDO_VALUE_TRACK_MATCHED_POSITION
  * Summary:
  *   Retrieves a numeric value indicating the position (in milliseconds) in
  *   the Track where the GNFPX fingerprint match occurs.                   
*/
#define GNSDK_GDO_VALUE_TRACK_MATCHED_POSITION		"gnsdk_val_track_matched_pos"

/** GNSDK_GDO_VALUE_TEXT_MATCH_SCORE
  * Summary:
  *   Retrieve a match score value for the result.
*/
#define GNSDK_GDO_VALUE_TEXT_MATCH_SCORE			"gnsdk_val_txtmatchscore"

/** \ \ 
  * Summary:
  *   Retrieves a value indicating if a GDO context response result is full
  *   (not partial). Returns 1 if full, 0 if partial.                      
*/
#define GNSDK_GDO_VALUE_FULL_RESULT					"gnsdk_val_full_result"

/** \ \ 
  * Summary:
  *   Retrieves a genre value from a GDO; applicable for music or video data.
*/
#define GNSDK_GDO_VALUE_GENRE						"gnsdk_val_list_genre"
/** \ \ 
  * Summary:
  *   Retrieves a genre meta group value from a GDO; applicable for music or
  *   video data.                                                           
*/
#define GNSDK_GDO_VALUE_GENRE_META					"gnsdk_val_list_genremeta"
/** \ \ 
  * Summary:
  *   Retrieves a genre micro group value from a GDO; applicable for music or
  *   video data.                                                            
*/
#define GNSDK_GDO_VALUE_GENRE_MICRO					"gnsdk_val_list_genremicro"
/** GNSDK_GDO_VALUE_ARTIST_DISPLAY
  * Summary:
  *   Retrieves the official artist name value, in the original language, from
  *   a GDO.                                                                  
*/
#define GNSDK_GDO_VALUE_ARTIST_DISPLAY				"gnsdk_val_artist"
/** GNSDK_GDO_VALUE_ARTIST_DISPLAY_LANGUAGE
  * Summary:
  *   Retrieves the official display language value for a returned
  *   GNSDK_GDO_VALUE_ARTIST_DISPLAY object.                      
*/
#define GNSDK_GDO_VALUE_ARTIST_DISPLAY_LANGUAGE		"gnsdk_val_artist_lang"
/** \ \ 
  * Summary:
  *   Retrieves the sortable variation for an artist's name from a GDO. For
  *   \example, <i>Beatles</i> for <i>The Beatles</i>.                     
*/
#define GNSDK_GDO_VALUE_ARTIST_SORTABLE				"gnsdk_val_artist_sortable"
/** GNSDK_GDO_VALUE_ARTIST_PREFIX
  * Summary:
  *   Retrieves the prefix (such as <i>Mr.</i>, <i>Ms.</i>, <i>Miss, </i>or <i>The</i>)
  *   for an artist's name from a GDO.                                                 
*/
#define GNSDK_GDO_VALUE_ARTIST_PREFIX				"gnsdk_val_artist_prefix"
/** GNSDK_GDO_VALUE_ARTIST_FAMILY
  * Summary:
  *   Retrieves the family (or last) name of an artist from a GDO.
*/
#define GNSDK_GDO_VALUE_ARTIST_FAMILY				"gnsdk_val_artist_family"
/** GNSDK_GDO_VALUE_ARTIST_GIVEN
  * Summary:
  *   Retrieves the given (or first) name of an artist from a GDO.
*/
#define GNSDK_GDO_VALUE_ARTIST_GIVEN				"gnsdk_val_artist_given"

/** GNSDK_GDO_VALUE_ARTIST_REGIONAL
  * Summary:
  *   Retrieves the official regional variant of the artist's name, based on
  *   the GDO's locale language.                                            
*/
#define GNSDK_GDO_VALUE_ARTIST_REGIONAL				"gnsdk_val_artist_reg"
/** GNSDK_GDO_VALUE_ARTIST_REGIONAL_LANGUAGE
  * Summary:
  *   Retrieves the official regional variant language value for a returned
  *   GNSDK_GDO_VALUE_ARTIST_REGIONAL object.                              
*/
#define GNSDK_GDO_VALUE_ARTIST_REGIONAL_LANGUAGE	"gnsdk_val_artist_reglang"

/** GNSDK_GDO_VALUE_ARTIST_ORIGIN
  * Summary:
  *   Retrieves an artist origin value from a GDO.
*/
#define GNSDK_GDO_VALUE_ARTIST_ORIGIN				"gnsdk_val_list_artorg"
/** GNSDK_GDO_VALUE_ARTIST_ERA
  * Summary:
  *   Retrieves an artist era value from a GDO.
*/
#define GNSDK_GDO_VALUE_ARTIST_ERA					"gnsdk_val_list_artera"
/** GNSDK_GDO_VALUE_ARTIST_TYPE
  * Summary:
  *   Retrieves an artist type value from a GDO.
*/
#define GNSDK_GDO_VALUE_ARTIST_TYPE					"gnsdk_val_list_arttype"

/** GNSDK_GDO_VALUE_AVAIL_GNCOVERART
  * Summary:
  *   Retrieves an available content value from a GDO. This value is
  *   applicable to music and video data.                           
*/
#define GNSDK_GDO_VALUE_AVAIL_GNCOVERART			"gnsdk_val_link_gncoverart"
/** GNSDK_GDO_VALUE_AVAIL_GNLYRICS
  * Summary:
  *   Retrieves an available Gracenote lyric value from a GDO. This value is
  *   specific to music data.                                               
*/
#define GNSDK_GDO_VALUE_AVAIL_GNLYRICS				"gnsdk_val_link_gnlyrics"
/** GNSDK_GDO_VALUE_AVAIL_GNBIOGRAPHY
  * Summary:
  *   Retrieves an available Gracenote biography value from a GDO. This value
  *   is specific to music data.                                             
*/
#define GNSDK_GDO_VALUE_AVAIL_GNBIOGRAPHY			"gnsdk_val_link_gnbiography"

/** \ \ 
  * Summary:
  *   Retrieves a role category credit value from a GDO.
*/

#define GNSDK_GDO_VALUE_ROLE_CATEGORY				"gnsdk_val_role_cat"
/** \ \ 
  * Summary:
  *   Retrieves a role value from a GDO.
*/
#define GNSDK_GDO_VALUE_ROLE						"gnsdk_val_role"
/** \ \ 
  * Summary:
  *   Retrieves a role billing value from a GDO.
*/
#define GNSDK_GDO_VALUE_ROLE_BILLING				"gnsdk_val_role_billing"
/** \ \ 
  * Summary:
  *   Retrieves an origin value from a GDO.
*/
#define GNSDK_GDO_VALUE_ORIGIN						"gnsdk_val_list_origin"
/** GNSDK_GDO_VALUE_ERA
  * Summary:
  *   Retrieves a classical musical era value from a GDO.
*/
#define GNSDK_GDO_VALUE_ERA							"gnsdk_val_desc_era"
/** GNSDK_GDO_VALUE_COMPOSITION_FORM
  * Summary:
  *   Retrieves a classical music composition form value (for example, <i>Symphony</i>)
  *   from a GDO.                                                                      
*/
#define GNSDK_GDO_VALUE_COMPOSITION_FORM			"gnsdk_val_desc_compform"
/** GNSDK_GDO_VALUE_COMPOSITION_STYLE
  * Summary:
  *   Retrieve a classical music composition style value from a GDO.
*/
#define GNSDK_GDO_VALUE_COMPOSITION_STYLE			"gnsdk_val_desc_compstyle"
/** GNSDK_GDO_VALUE_INSTRUMENTATION
  * Summary:
  *   Retrieves an instrumentation value from a GDO.
*/
#define GNSDK_GDO_VALUE_INSTRUMENTATION				"gnsdk_val_desc_instrument"
/** GNSDK_GDO_VALUE_CLASSICAL_DATA
  * Summary:
  *   Retrieves a boolean value (Y/N) that indicates whether enhanced
  *   classical music data exists in the GDO.                        
*/
#define GNSDK_GDO_VALUE_CLASSICAL_DATA				"gnsdk_val_classical_data"


/*****************************************************************************
** GDO Keys specific to LYRIC responses
*/

/** GNSDK_GDO_CONTEXT_RESPONSE_LYRIC
  * Summary:
  *   GDO is of lyric response context.
*/
#define GNSDK_GDO_CONTEXT_RESPONSE_LYRIC			"gnsdk_ctx_response_lyric"
/** GNSDK_GDO_CONTEXT_LYRIC
  * Summary:
  *   GDO is of lyric context.
*/
#define GNSDK_GDO_CONTEXT_LYRIC						"gnsdk_ctx_lyric"
/** GNSDK_GDO_CONTEXT_LYRIC_SAMPLE
  * Summary:
  *   GDO is of lyric sample context.
*/
#define GNSDK_GDO_CONTEXT_LYRIC_SAMPLE				"gnsdk_ctx_lyricsample"

/*
 * GDO Context Keys
 * For retrieving specific contexts from a GDO.
 * Actual context returned will be one of the defined contexts above.
 */

/** GNSDK_GDO_CHILD_LYRIC
  * Summary:
  *   Retrieves a child lyric context.
*/
#define GNSDK_GDO_CHILD_LYRIC						GNSDK_GDO_CONTEXT_LYRIC"!"
/** GNSDK_GDO_CHILD_LYRIC_SAMPLE
  * Summary:
  *   Retrieves a lyric sample context (from lyric response GDO).
*/
#define GNSDK_GDO_CHILD_LYRIC_SAMPLE				GNSDK_GDO_CONTEXT_LYRIC_SAMPLE"!"
/** GNSDK_GDO_VALUE_LYRIC_LINE
  * Summary:
  *   Retrieves a line of a lyric from a lyric sample GDO.
*/
#define GNSDK_GDO_VALUE_LYRIC_LINE					"gnsdk_val_lyricline"


/*****************************************************************************
** GDO Keys specific to VIDEO responses
*/

/** GNSDK_GDO_CONTEXT_RESPONSE_VIDEO_PRODUCT
  * Summary:
  *   GDO is of video response context.     
*/
#define GNSDK_GDO_CONTEXT_RESPONSE_VIDEO_PRODUCT			"gnsdk_ctx_response_video_product"


/** GNSDK_GDO_CONTEXT_RESPONSE_VIDEO
  * Summary:
  *   GDO is of video product response context.
*/
#define GNSDK_GDO_CONTEXT_RESPONSE_VIDEO					GNSDK_GDO_CONTEXT_RESPONSE_VIDEO_PRODUCT

/** GNSDK_GDO_CONTEXT_VIDEO_PRODUCT
  * Summary:
  *   GDO is of video product context.
*/
#define GNSDK_GDO_CONTEXT_VIDEO_PRODUCT						"gnsdk_ctx_video_product"
/** GNSDK_GDO_CONTEXT_VIDEO_DISC
  * Summary:
  *   GDO is of video disc context.
*/
#define GNSDK_GDO_CONTEXT_VIDEO_DISC						"gnsdk_ctx_video_disc"
/** GNSDK_GDO_CONTEXT_VIDEO_SIDE
  * Summary:
  *   GDO is of video disc side context.
*/
#define GNSDK_GDO_CONTEXT_VIDEO_SIDE						"gnsdk_ctx_video_side"
/** GNSDK_GDO_CONTEXT_VIDEO_LAYER
  * Summary:
  *   GDO is of video disc layer context.
*/
#define GNSDK_GDO_CONTEXT_VIDEO_LAYER						"gnsdk_ctx_video_layer"
/** GNSDK_GDO_CONTEXT_VIDEO_FEATURE
  * Summary:
  *   GDO is of video feature context.
*/
#define GNSDK_GDO_CONTEXT_VIDEO_FEATURE						"gnsdk_ctx_video_feature"
/** GNSDK_GDO_CONTEXT_VIDEO_CHAPTER
  * Summary:
  *   GDO is of video feature chapter context.
*/
#define GNSDK_GDO_CONTEXT_VIDEO_CHAPTER						"gnsdk_ctx_video_chapter"

/** GNSDK_GDO_CHILD_VIDEO_PRODUCT
  * Summary:
  *   Retrieves a child video product context.
*/
#define GNSDK_GDO_CHILD_VIDEO_PRODUCT						GNSDK_GDO_CONTEXT_VIDEO_PRODUCT"!"
/** GNSDK_GDO_CHILD_VIDEO_DISC
  * Summary:
  *   Retrieves a child video disc context.
*/
#define GNSDK_GDO_CHILD_VIDEO_DISC							GNSDK_GDO_CONTEXT_VIDEO_DISC"!"
/** GNSDK_GDO_CHILD_VIDEO_SIDE
  * Summary:
  *   Retrieves a child video Side context.
*/
#define GNSDK_GDO_CHILD_VIDEO_SIDE							GNSDK_GDO_CONTEXT_VIDEO_SIDE"!"
/** GNSDK_GDO_CHILD_VIDEO_LAYER
  * Summary:
  *   Retrieves a child video layer context.
*/
#define GNSDK_GDO_CHILD_VIDEO_LAYER							GNSDK_GDO_CONTEXT_VIDEO_LAYER"!"
/** GNSDK_GDO_CHILD_VIDEO_FEATURE
  * Summary:
  *   Retrieves a child video feature context.
*/
#define GNSDK_GDO_CHILD_VIDEO_FEATURE						GNSDK_GDO_CONTEXT_VIDEO_FEATURE"!"
/** GNSDK_GDO_CHILD_VIDEO_CHAPTER
  * Summary:
  *   Retrieves a child video chapter context.
*/
#define GNSDK_GDO_CHILD_VIDEO_CHAPTER						GNSDK_GDO_CONTEXT_VIDEO_CHAPTER"!"

/** GNSDK_GDO_CHILD_VIDEO_FEATURE_MAIN
  * Summary:
  *   Retrieves a child video feature context with a feature type of <i>main.</i>
*/
#define GNSDK_GDO_CHILD_VIDEO_FEATURE_MAIN					GNSDK_GDO_CONTEXT_VIDEO_FEATURE"!main"
/** GNSDK_GDO_CHILD_VIDEO_FEATURE_EPISODE
  * Summary:
  *   Retrieves a child video feature context with a feature type of <i>episode.</i>
*/
#define GNSDK_GDO_CHILD_VIDEO_FEATURE_EPISODE				GNSDK_GDO_CONTEXT_VIDEO_FEATURE"!episode"
/** GNSDK_GDO_CHILD_VIDEO_FEATURE_EXTRA
  * Summary:
  *   Retrieves a child video feature context with a feature type of <i>extra.</i>
*/
#define GNSDK_GDO_CHILD_VIDEO_FEATURE_EXTRA					GNSDK_GDO_CONTEXT_VIDEO_FEATURE"!extra"
/** GNSDK_GDO_CHILD_VIDEO_FEATURE_RESERVED
  * Summary:
  *   Retrieves a child video feature context with a feature type of <i>reserved.</i>
*/
#define GNSDK_GDO_CHILD_VIDEO_FEATURE_RESERVED				GNSDK_GDO_CONTEXT_VIDEO_FEATURE"!reserved"

/** GNSDK_GDO_CHILD_VIDEO_MATCHED_DISC
  * Summary:
  *   Retrieves a child video disc context that matches the input criteria.
*/
#define GNSDK_GDO_CHILD_VIDEO_MATCHED_DISC					GNSDK_GDO_CONTEXT_VIDEO_DISC"!matched"
/** GNSDK_GDO_CHILD_VIDEO_MATCHED_SIDE
  * Summary:
  *   Retrieves a child video side context that matches the input criteria.
*/
#define GNSDK_GDO_CHILD_VIDEO_MATCHED_SIDE					GNSDK_GDO_CONTEXT_VIDEO_SIDE"!matched"
/** GNSDK_GDO_CHILD_VIDEO_MATCHED_LAYER
  * Summary:
  *   Retrieves a child video layer context that matches the input criteria.
*/
#define GNSDK_GDO_CHILD_VIDEO_MATCHED_LAYER					GNSDK_GDO_CONTEXT_VIDEO_LAYER"!matched"

/** Cast credit
*/
#define GNSDK_GDO_CHILD_CREDIT_CAST							GNSDK_GDO_CONTEXT_CREDIT"!15941"
/** Dancer credit
*/
#define GNSDK_GDO_CHILD_CREDIT_DANCER						GNSDK_GDO_CONTEXT_CREDIT"!15943"
/** Guest credit
*/
#define GNSDK_GDO_CHILD_CREDIT_GUEST						GNSDK_GDO_CONTEXT_CREDIT"!15944"
/** Host credit
*/
#define GNSDK_GDO_CHILD_CREDIT_HOST							GNSDK_GDO_CONTEXT_CREDIT"!15945"
/** Puppeteer credit
*/
#define GNSDK_GDO_CHILD_CREDIT_PUPPETEER					GNSDK_GDO_CONTEXT_CREDIT"!15947"
/** Voice of credit
*/
#define GNSDK_GDO_CHILD_CREDIT_VOICE_OF						GNSDK_GDO_CONTEXT_CREDIT"!15948"
/** Crew credit
*/
#define GNSDK_GDO_CHILD_CREDIT_CREW							GNSDK_GDO_CONTEXT_CREDIT"!15949"
/** Camera person credit
*/
#define GNSDK_GDO_CHILD_CREDIT_CAMERA_PERSON				GNSDK_GDO_CONTEXT_CREDIT"!15950"
/** Gopher credit
*/
#define GNSDK_GDO_CHILD_CREDIT_GOPHER						GNSDK_GDO_CONTEXT_CREDIT"!15951"
/** Stunt person credit
*/
#define GNSDK_GDO_CHILD_CREDIT_STUNT_PERSON					GNSDK_GDO_CONTEXT_CREDIT"!15952"
/** Music credit
*/
#define GNSDK_GDO_CHILD_CREDIT_MUSIC						GNSDK_GDO_CONTEXT_CREDIT"!15953"
/** Music director credit
*/
#define GNSDK_GDO_CHILD_CREDIT_MUSIC_DIRECTOR				GNSDK_GDO_CONTEXT_CREDIT"!15954"
/** Original score credit
*/
#define GNSDK_GDO_CHILD_CREDIT_ORIGINAL_SCORE				GNSDK_GDO_CONTEXT_CREDIT"!15955"
/** Production credit
*/
#define GNSDK_GDO_CHILD_CREDIT_PRODUCTION					GNSDK_GDO_CONTEXT_CREDIT"!15956"
/** Associate producer credit
*/
#define GNSDK_GDO_CHILD_CREDIT_ASSOCIATE_PRODUCER			GNSDK_GDO_CONTEXT_CREDIT"!15959"
/** Screenwriter credit
*/
#define GNSDK_GDO_CHILD_CREDIT_SCREENWRITER					GNSDK_GDO_CONTEXT_CREDIT"!15961"
/** Art credit
*/
#define GNSDK_GDO_CHILD_CREDIT_ART							GNSDK_GDO_CONTEXT_CREDIT"!15963"
/** Art direction credit
*/
#define GNSDK_GDO_CHILD_CREDIT_ART_DIRECTION				GNSDK_GDO_CONTEXT_CREDIT"!15964"
/** Casting credit
*/
#define GNSDK_GDO_CHILD_CREDIT_CASTING						GNSDK_GDO_CONTEXT_CREDIT"!15965"
/** Cinematography credit
*/
#define GNSDK_GDO_CHILD_CREDIT_CINEMATOGRAPHY				GNSDK_GDO_CONTEXT_CREDIT"!15966"
/** Costume design credit
*/
#define GNSDK_GDO_CHILD_CREDIT_COSTUME_DESIGN				GNSDK_GDO_CONTEXT_CREDIT"!15967"
/** Editor credit
*/
#define GNSDK_GDO_CHILD_CREDIT_EDITOR						GNSDK_GDO_CONTEXT_CREDIT"!15968"
/** Makeup credit
*/
#define GNSDK_GDO_CHILD_CREDIT_MAKEUP						GNSDK_GDO_CONTEXT_CREDIT"!15969"
/** Production design credit
*/
#define GNSDK_GDO_CHILD_CREDIT_PRODUCTION_DESIGN			GNSDK_GDO_CONTEXT_CREDIT"!15970"
/** Production management credit
*/
#define GNSDK_GDO_CHILD_CREDIT_PRODUCTION_MANAGEMENT		GNSDK_GDO_CONTEXT_CREDIT"!15971"
/** Special effects credit
*/
#define GNSDK_GDO_CHILD_CREDIT_SPECIAL_EFFECTS				GNSDK_GDO_CONTEXT_CREDIT"!15972"

/** GNSDK_GDO_VALUE_MATCHED
  * Summary:
  *   Retrieves the matched boolean value for the current context; this
  *   indicates whether this context is the one that matched the input
  *   criteria. Available from many video contexts.                    
*/
#define GNSDK_GDO_VALUE_MATCHED							"gnsdk_val_matched"
/** \ \ 
  * Summary:
  *   Retrieves an ordinal of the current context.
*/
#define GNSDK_GDO_VALUE_ORDINAL							"gnsdk_val_ordinal"
/** GNSDK_GDO_VALUE_DURATION
  * Summary:
  *   Retrieves a duration value in seconds from the current context, such as <c>"3600"</c>
  *   for a 60-minute program. Available for video Chapters, Features,
  *   Products, Seasons, Series, and Works.                                                
*/
#define GNSDK_GDO_VALUE_DURATION						"gnsdk_val_duration"
/** GNSDK_GDO_VALUE_DURATION_UNITS
  * Summary:
  *   Retrieves a duration units value (seconds, <c>"SEC"</c>) from the
  *   current context. Available for video Chapters, Features, Products,
  *   Seasons, Series, and Works.                                       
*/
#define GNSDK_GDO_VALUE_DURATION_UNITS					"gnsdk_val_duration_units"

/** GNSDK_GDO_VALUE_PLOT_SYNOPSIS
  * Summary:
  *   Retrieves a plot synopsis for the current context. Available from many
  *   video contexts.                                                       
*/
#define GNSDK_GDO_VALUE_PLOT_SYNOPSIS					"gnsdk_val_plot_synopsis"
/** GNSDK_GDO_VALUE_PLOT_SYNOPSIS_LANGUAGE
  * Summary:
  *   Retrieves the available language value for a returned
  *   GNSDK_GDO_VALUE_PLOT_SYNOPSIS object.                
*/
#define GNSDK_GDO_VALUE_PLOT_SYNOPSIS_LANGUAGE			"gnsdk_val_plot_synopsis_language"

/** GNSDK_GDO_VALUE_PLOT_SUMMARY
  * Summary:
  *   Retrieves a plot summary value from a GDO.
*/
#define GNSDK_GDO_VALUE_PLOT_SUMMARY					"gnsdk_val_plot_summary"
/** GNSDK_GDO_VALUE_PLOT_TAGLINE
  * Summary:
  *   Retrieves a plot tagline value from a GDO.
*/
#define GNSDK_GDO_VALUE_PLOT_TAGLINE					"gnsdk_val_plot_tagline"
/** GNSDK_GDO_VALUE_FRANCHISE_TITLE
  * Summary:
  *   Retrieves a Franchise title value from a GDO.
*/
#define GNSDK_GDO_VALUE_FRANCHISE_TITLE					"gnsdk_val_franchise_title"
/** GNSDK_GDO_VALUE_FRANCHISE_TITLE_LANGUAGE
  * Summary:
  *   Retrieves the available language value for a returned
  *   GNSDK_GDO_VALUE_FRANCHISE_TITLE object.              
*/
#define GNSDK_GDO_VALUE_FRANCHISE_TITLE_LANGUAGE		"gnsdk_val_franchise_title_language"
/** GNSDK_GDO_VALUE_FRANCHISE_NUM
  * Summary:
  *   Retrieves a Franchise number value from a GDO.
*/
#define GNSDK_GDO_VALUE_FRANCHISE_NUM					"gnsdk_val_franchise_num"
/** GNSDK_GDO_VALUE_FRANCHISE_COUNT
  * Summary:
  *   Retrieves a Franchise count value from a GDO.
*/
#define GNSDK_GDO_VALUE_FRANCHISE_COUNT					"gnsdk_val_franchise_count"


/** GNSDK_GDO_VALUE_SERIES_TITLE
  * Summary:
  *   Retrieves a series title value from a GDO.
*/
#define GNSDK_GDO_VALUE_SERIES_TITLE					"gnsdk_val_series_title"
/** GNSDK_GDO_VALUE_SERIES_TITLE_LANGUAGE
  * Summary:
  *   Retrieves the available language value for a returned
  *   GNSDK_GDO_VALUE_SERIES_TITLE object.                 
*/
#define GNSDK_GDO_VALUE_SERIES_TITLE_LANGUAGE			"gnsdk_val_series_title_language"
/** GNSDK_GDO_VALUE_SERIES_EPISODE
  * Summary:
  *   Retrieves a series episode value from a GDO.
*/
#define GNSDK_GDO_VALUE_SERIES_EPISODE					"gnsdk_val_series_episode"
/** GNSDK_GDO_VALUE_SERIES_EPISODE_COUNT
  * Summary:
  *   Retrieves a series episode count value from a GDO.
*/
#define GNSDK_GDO_VALUE_SERIES_EPISODE_COUNT			"gnsdk_val_series_episode_count"
/** GNSDK_GDO_VALUE_SEASON_EPISODE
  * Summary:
  *   Retrieves a Season episode value from a GDO.
*/
#define GNSDK_GDO_VALUE_SEASON_EPISODE					"gnsdk_val_season_episode"
/** GNSDK_GDO_VALUE_SEASON_EPISODE_COUNT
  * Summary:
  *   Retrieves an episode count value from a GDO.
*/
#define GNSDK_GDO_VALUE_SEASON_EPISODE_COUNT			"gnsdk_val_season_episode_count"
/** GNSDK_GDO_VALUE_SEASON_NUMBER
  * Summary:
  *   Retrieves a Season number value from a GDO.
*/
#define GNSDK_GDO_VALUE_SEASON_NUMBER					"gnsdk_val_season_number"
/** GNSDK_GDO_VALUE_SEASON_COUNT
  * Summary:
  *   Retrieves a Season count value from a GDO.
*/
#define GNSDK_GDO_VALUE_SEASON_COUNT					"gnsdk_val_season_count"

/** GNSDK_GDO_VALUE_RATING
  * Summary:
  *   Retrieves a rating value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING							"gnsdk_val_list_rating"
/** GNSDK_GDO_VALUE_RATING_DESC
  * Summary:
  *   Retrieves a rating description value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_DESC						"gnsdk_val_list_rating_desc"
/** GNSDK_GDO_VALUE_RATING_TYPE
  * Summary:
  *   Retrieves a rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_TYPE						"gnsdk_val_list_rating_type"

/** GNSDK_GDO_VALUE_RATING_TYPE_ID
  * Summary:
  *   Retrieves a rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_TYPE_ID					"gnsdk_val_list_rating_type_id"

/** GNSDK_GDO_VALUE_RATING_REASON
  * Summary:
  *   Retrieves a rating reason value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_REASON					"gnsdk_val_list_rating_reason"

/** GNSDK_GDO_VALUE_RATING_MPAA
  * Summary:
  *   Retrieves a MPAA rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_MPAA						"gnsdk_val_list_rating_typed_mpaa"
/** GNSDK_GDO_VALUE_RATING_MPAA_TV
  * Summary:
  *   Retrieves a MPAA TV rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_MPAA_TV					"gnsdk_val_list_rating_typed_mpaa_tv"	
/** GNSDK_GDO_VALUE_RATING_FAB
  * Summary:
  *   Retrieves a FAB rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_FAB						"gnsdk_val_list_rating_typed_fab"
/** GNSDK_GDO_VALUE_RATING_CHVRS
  * Summary:
  *   Retrieves a CHVRS rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_CHVRS					"gnsdk_val_list_rating_typed_chvrs"
/** GNSDK_GDO_VALUE_RATING_CANADIAN_TV
  * Summary:
  *   Retrieves a Canadian TV rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_CANADIAN_TV				"gnsdk_val_list_rating_typed_canadian_tv"
/** GNSDK_GDO_VALUE_RATING_BBFC
  * Summary:
  *   Retrieves a BBFC rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_BBFC						"gnsdk_val_list_rating_typed_bbfc"
/** GNSDK_GDO_VALUE_RATING_CBFC
  * Summary:
  *   Retrieves a CBFC rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_CBFC						"gnsdk_val_list_rating_typed_cbfc"
/** GNSDK_GDO_VALUE_RATING_OFLC
  * Summary:
  *   Retrieves a OFLC TV rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_OFLC						"gnsdk_val_list_rating_typed_oflc"
/** GNSDK_GDO_VALUE_RATING_HONG_KONG
  * Summary:
  *   Retrieves a Hong Kong rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_HONG_KONG				"gnsdk_val_list_rating_typed_hong_kong"
/** GNSDK_GDO_VALUE_RATING_FINNISH
  * Summary:
  *   Retrieves a Finnish rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_FINNISH					"gnsdk_val_list_rating_typed_finnish"
/** GNSDK_GDO_VALUE_RATING_KMRB
  * Summary:
  *   Retrieves a KMRB rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_KMRB						"gnsdk_val_list_rating_typed_kmrb"
/** GNSDK_GDO_VALUE_RATING_DVD_PARENTAL
  * Summary:
  *   Retrieves a DVD Parental rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_DVD_PARENTAL				"gnsdk_val_list_rating_typed_dvd_parental"
/** GNSDK_GDO_VALUE_RATING_EIRIN
  * Summary:
  *   Retrieves a EIRIN rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_EIRIN					"gnsdk_val_list_rating_typed_eirin"
/** GNSDK_GDO_VALUE_RATING_INCAA
  * Summary:
  *   Retrieves a INCAA rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_INCAA					"gnsdk_val_list_rating_typed_incaa"
/** GNSDK_GDO_VALUE_RATING_DJTCQ
  * Summary:
  *   Retrieves a DJTCQ rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_DJTCQ					"gnsdk_val_list_rating_typed_djtcq"
/** GNSDK_GDO_VALUE_RATING_QUEBEC
  * Summary:
  *   Retrieves a Quebecois rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_QUEBEC					"gnsdk_val_list_rating_typed_quebec"
/** GNSDK_GDO_VALUE_RATING_FRANCE
  * Summary:
  *   Retrieves a French rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_FRANCE					"gnsdk_val_list_rating_typed_france"
/** GNSDK_GDO_VALUE_RATING_FSK
  * Summary:
  *   Retrieves a FSK rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_FSK						"gnsdk_val_list_rating_typed_fsk"
/** GNSDK_GDO_VALUE_RATING_ITALY
  * Summary:
  *   Retrieves a Italian rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_ITALY					"gnsdk_val_list_rating_typed_italy"
/** GNSDK_GDO_VALUE_RATING_SPAIN
  * Summary:
  *   Retrieves a Spanish rating type value from a GDO.
*/
#define GNSDK_GDO_VALUE_RATING_SPAIN					"gnsdk_val_list_rating_typed_spain"

/** GNSDK_GDO_VALUE_VIDEO_PRODUCTION_TYPE
  * Summary:
  *   Retrieves a video production type value from a GDO.
*/
#define GNSDK_GDO_VALUE_VIDEO_PRODUCTION_TYPE			"gnsdk_val_list_video_prodtype"


/** GNSDK_GDO_VALUE_VIDEO_PRODUCTION_TYPE_ID
  * Summary:
  *   Retrieves a video production type value from a GDO.
*/
#define GNSDK_GDO_VALUE_VIDEO_PRODUCTION_TYPE_ID		"gnsdk_val_list_video_prodtype_id"


/** GNSDK_GDO_VALUE_VIDEO_FEATURE_TYPE
  * Summary:
  *   Retrieves a video feature type value from a GDO.
*/
#define GNSDK_GDO_VALUE_VIDEO_FEATURE_TYPE				"gnsdk_val_list_video_feattype"
/** GNSDK_GDO_VALUE_SERIAL_TYPE
  * Summary:
  *   Retrieves a serial type value from a GDO.
*/
#define GNSDK_GDO_VALUE_SERIAL_TYPE						"gnsdk_val_list_vidserial_type"

/** GNSDK_GDO_VALUE_WORK_TYPE
  * Summary:
  *   Retrieves a work type value from a GDO.
*/
#define GNSDK_GDO_VALUE_WORK_TYPE						"gnsdk_val_list_work_type"
/** GNSDK_GDO_VALUE_STORY_TYPE
  * Summary:
  *   Retrieves a story type value from a video Work GDO.
*/
#define GNSDK_GDO_VALUE_STORY_TYPE						"gnsdk_val_list_story_type"

/** GNSDK_GDO_VALUE_AUDIENCE
  * Summary:
  *   Retrieves an audience value from a video Work GDO.
*/
#define GNSDK_GDO_VALUE_AUDIENCE						"gnsdk_val_list_audience"
/** \ \ 
  * Summary:
  *   Retrieves a mood group value from a GDO.
*/
#define GNSDK_GDO_VALUE_MOOD							"gnsdk_val_list_mood"
/** GNSDK_GDO_VALUE_REPUTATION
  * Summary:
  *   Retrieves a reputation value from a video Work GDO.
*/
#define GNSDK_GDO_VALUE_REPUTATION						"gnsdk_val_list_reputation"
/** GNSDK_GDO_VALUE_SCENARIO
  * Summary:
  *   Retrieves a scenario value from a video Work GDO.
*/
#define GNSDK_GDO_VALUE_SCENARIO						"gnsdk_val_list_scenario"
/** GNSDK_GDO_VALUE_SETTING_ENVIRONMENT
  * Summary:
  *   Retrieves a setting environment value from a video Work GDO.
*/
#define GNSDK_GDO_VALUE_SETTING_ENVIRONMENT				"gnsdk_val_list_settingenv"

/** GNSDK_GDO_VALUE_SETTING_TIME_PERIOD
  * Summary:
  *   Retrieves a historical time period value, such as <i>Elizabethan Era,
  *   1558-1603</i>.                                                       
*/
#define GNSDK_GDO_VALUE_SETTING_TIME_PERIOD				"gnsdk_val_list_settingtimeperiod"

/** GNSDK_GDO_VALUE_SOURCE
  * Summary:
  *   Retrieves a source value from a video Work GDO.
*/
#define GNSDK_GDO_VALUE_SOURCE							"gnsdk_val_list_source"
/** GNSDK_GDO_VALUE_STYLE
  * Summary:
  *   Retrieves a style value from a video Work GDO.
*/
#define GNSDK_GDO_VALUE_STYLE							"gnsdk_val_list_style"
/** GNSDK_GDO_VALUE_TOPIC
  * Summary:
  *   Retrieves a topic value from a video Work GDO.
*/
#define GNSDK_GDO_VALUE_TOPIC							"gnsdk_val_list_topic"



/** GNSDK_GDO_VALUE_MEDIA_SPACE
  * Summary:
  *   Retrieves a media space value from a GDO.
*/
#define GNSDK_GDO_VALUE_MEDIA_SPACE						"gnsdk_val_list_media_space"

/** GNSDK_GDO_VALUE_DATE_ORIGINAL_RELEASE
  * Summary:
  *   Retrieves a release date from the current context. Available for video
  *   Products, Features, and Works.                                        
*/
#define GNSDK_GDO_VALUE_DATE_ORIGINAL_RELEASE			"gnsdk_val_date_original_release"
/** GNSDK_GDO_VALUE_NOTES
  * Summary:
  *   Retrieves the notes for the current context. Available from most video
  *   contexts.                                                             
*/
#define GNSDK_GDO_VALUE_NOTES							"gnsdk_val_notes"
/** GNSDK_GDO_VALUE_ASPECT_RATIO
  * Summary:
  *   Retrieves the aspect ratio for the current context. Available from many
  *   video contexts.                                                        
*/
#define GNSDK_GDO_VALUE_ASPECT_RATIO					"gnsdk_val_aspect"
/** GNSDK_GDO_VALUE_ASPECT_RATIO_TYPE
  * Summary:
  *   Retrieves the aspect ratio for the current context. Available from many
  *   video contexts.                                                        
*/
#define GNSDK_GDO_VALUE_ASPECT_RATIO_TYPE				"gnsdk_val_aspect_type"

/** GNSDK_GDO_VALUE_TV_SYSTEM
  * Summary:
  *   Retrieves a TV system value (like <i>NTSC</i>) from the current context.
  *   Available from many video contexts.                                     
*/
#define GNSDK_GDO_VALUE_TV_SYSTEM						"gnsdk_val_tv_system"
/** GNSDK_GDO_VALUE_REGION_CODE
  * Summary:
  *   Retrieves a region code (for example, <i>FE</i>) for the current
  *   context. Available from many video contexts.                    
*/
#define GNSDK_GDO_VALUE_REGION_CODE						"gnsdk_val_region_code"
/** GNSDK_GDO_VALUE_VIDEO_REGION
  * Summary:
  *   Retrieves a video product region value from the current context.
  *   Available from many video contexts. Example: <i>1</i>           
*/
#define GNSDK_GDO_VALUE_VIDEO_REGION					"gnsdk_val_list_vidregion"
/** GNSDK_GDO_VALUE_VIDEO_REGION_DESC
  * Summary:
  *   Retrieves a video product region description for the current context.
  *   Available from many video contexts. An example description is: <i>USA,
  *   Canada, US Territories, Bermuda, and Cayman Islands</i>               
*/
#define GNSDK_GDO_VALUE_VIDEO_REGION_DESC				"gnsdk_val_list_vidregion_desc"
/** GNSDK_GDO_VALUE_MEDIA_TYPE
  * Summary:
  *   Retrieves a media type value from a GDO.
*/
#define GNSDK_GDO_VALUE_MEDIA_TYPE						"gnsdk_val_list_mediatype"

/** <unfinished>
  * 
  * GNSDK_GDO_VALUE_COMMERCE_TYPE
  * Summary:
  *   Retrieves a numerical value indicating if commerce type data exists for
  *   a video disc.                                                          
*/
#define GNSDK_GDO_VALUE_COMMERCE_TYPE					"gnsdk_val_commerce_type"

/** GNSDK_GDO_VALUE_TITLE_MAIN
  * Summary:
  *   Retrieves a video product or disc main title value from a GDO.
*/
#define GNSDK_GDO_VALUE_TITLE_MAIN							"gnsdk_val_title_main"
/** GNSDK_GDO_VALUE_TITLE_EDITION
  * Summary:
  *   Retrieves an edition title for the current context. Available from many
  *   video contexts.                                                        
*/
#define GNSDK_GDO_VALUE_TITLE_EDITION						"gnsdk_val_title_edition"
/** \ \ 
  * Summary:
  *   Retrieves the prefix of a title from the current context.
*/
#define GNSDK_GDO_VALUE_TITLE_PREFIX						"gnsdk_val_title_prefix"
/** \ \ 
  * Summary:
  *   Retrieves the available language value for a returned
  *   GNSDK_GDO_VALUE_TITLE_PREFIX object.                 
*/
#define GNSDK_GDO_VALUE_TITLE_PREFIX_LANGUAGE				"gnsdk_val_title_prefix_language"

/** GNSDK_GDO_VALUE_RANK
  * Summary:
  *   Retrieves a rank (ordinal) for the current credit context.
*/
#define GNSDK_GDO_VALUE_RANK								"gnsdk_val_rank"
/** GNSDK_GDO_VALUE_CHARACTER_NAME
  * Summary:
  *   Retrieves a character name value from a GDO.
*/
#define GNSDK_GDO_VALUE_CHARACTER_NAME						"gnsdk_val_character_name"


/** GNSDK_GDO_VALUE_ROLE_ID
  * Summary:
  *   \Returns a Role ID for a credit.
*/
#define GNSDK_GDO_VALUE_ROLE_ID						"gnsdk_val_list_role_id"


/** GNSDK_GDO_CONTEXT_RESPONSE_VIDEO_WORK
  * Summary:
  *   GDO value keys specific to VIDEO_WORK responses.
*/
#define GNSDK_GDO_CONTEXT_RESPONSE_VIDEO_WORK				"gnsdk_ctx_response_video_work"
/** GNSDK_GDO_CONTEXT_VIDEO_WORK
  * Summary:
  *   GDO is of a video Work context.
  */
#define GNSDK_GDO_CONTEXT_VIDEO_WORK						"gnsdk_ctx_video_work"
/** GNSDK_GDO_CHILD_VIDEO_WORK
  * Summary:
  *   Retrieves a child Video Work context.
*/
#define GNSDK_GDO_CHILD_VIDEO_WORK							GNSDK_GDO_CONTEXT_VIDEO_WORK"!"



/** GNSDK_GDO_CHILD_VIDEO_WORK_NONSEASON
  * Summary:
  *   Retrieves a child Video Work context that is not associated with a
  *   Season.                                                           
*/
#define GNSDK_GDO_CHILD_VIDEO_WORK_NONSEASON				GNSDK_GDO_CONTEXT_VIDEO_WORK"!nonseason"

/** GNSDK_GDO_CONTEXT_RESPONSE_VIDEO_SEASON
  * Summary:
  *   GDO is of a video Seasons response context.
*/
#define GNSDK_GDO_CONTEXT_RESPONSE_VIDEO_SEASON				"gnsdk_ctx_response_video_season"
/** GNSDK_GDO_CONTEXT_VIDEO_SEASON
  * Summary:
  *   GDO is of a video Season context.
*/
#define GNSDK_GDO_CONTEXT_VIDEO_SEASON						"gnsdk_ctx_video_season"
/** GNSDK_GDO_CHILD_VIDEO_SEASON
  * Summary:
  *   Retrieve a child Season context.
*/
#define GNSDK_GDO_CHILD_VIDEO_SEASON						GNSDK_GDO_CONTEXT_VIDEO_SEASON"!"
/** GNSDK_GDO_CONTEXT_RESPONSE_VIDEO_SERIES
  * Summary:
  *   GDO is of a video Series response context.
*/
#define GNSDK_GDO_CONTEXT_RESPONSE_VIDEO_SERIES				"gnsdk_ctx_response_video_series"
/** GNSDK_GDO_CONTEXT_VIDEO_SERIES
  * Summary:
  *   GDO is of a video Series context.
*/
#define GNSDK_GDO_CONTEXT_VIDEO_SERIES						"gnsdk_ctx_video_series"
/** GNSDK_GDO_CHILD_VIDEO_SERIES
  * Summary:
  *   Retrieve a child Series context.
*/
#define GNSDK_GDO_CHILD_VIDEO_SERIES						GNSDK_GDO_CONTEXT_VIDEO_SERIES"!"

/** GNSDK_GDO_CONTEXT_RESPONSE_CONTRIBUTOR
  * Summary:
  *   GDO value keys specific to CONTRIBUTOR responses.
*/

#define GNSDK_GDO_CONTEXT_RESPONSE_CONTRIBUTOR				"gnsdk_ctx_response_contributor"

/** GNSDK_GDO_CONTEXT_CONTRIBUTOR
  * Summary:
  *   GDO is of a Contributor context.
*/
#define GNSDK_GDO_CONTEXT_CONTRIBUTOR						"gnsdk_ctx_contributor"

/** GNSDK_GDO_CHILD_CONTRIBUTOR
  * Summary:
  *   Retrieves a child contributor context.
*/
#define GNSDK_GDO_CHILD_CONTRIBUTOR							GNSDK_GDO_CONTEXT_CONTRIBUTOR"!"
/** GNSDK_GDO_CONTEXT_RESPONSE_VIDEO_OBJECT
  * Summary:
  *   GDO contains keys specific to video Object responses.
*/
#define GNSDK_GDO_CONTEXT_RESPONSE_VIDEO_OBJECT				"gnsdk_ctx_response_video_object"


/*****************************************************************************
** GDO Keys specific to Suggestion responses
*/

/** GNSDK_GDO_CONTEXT_RESPONSE_SUGGESTIONS
  * Summary:
  *   GDO is of search suggestion response context.
*/
#define GNSDK_GDO_CONTEXT_RESPONSE_SUGGESTIONS				"gnsdk_ctx_response_suggestions"

/** GNSDK_GDO_VALUE_SUGGESTION_TITLE
  * Summary:
  *   Retrieves the title for the current search suggestion context.
*/
#define GNSDK_GDO_VALUE_SUGGESTION_TITLE					"gnsdk_val_suggestion_title"
/** GNSDK_GDO_VALUE_SUGGESTION_TYPE
  * Summary:
  *   Retrieves the type for the current search suggestion context; this value
  *   is only available for a video product title search.                     
*/

#define GNSDK_GDO_VALUE_SUGGESTION_TYPE						"gnsdk_val_suggestion_type"


/** GNSDK_GDO_VALUE_SUGGESTION_TEXT
  * Summary:
  *   Retrieves the text for the current search suggestion context.
*/
#define GNSDK_GDO_VALUE_SUGGESTION_TEXT						"gnsdk_val_suggestion_text"

/** GNSDK_GDO_CONTEXT_RESPONSE_VIDEOCLIP
  * Summary:
  *   GDO Keys for video clip response objects.
*/
#define GNSDK_GDO_CONTEXT_RESPONSE_VIDEOCLIP				"gnsdk_ctx_response_videoclip"
/** GNSDK_GDO_CONTEXT_VIDEOCLIP
  * Summary:
  *   GDO is of video clip context.
*/
#define GNSDK_GDO_CONTEXT_VIDEOCLIP							"gnsdk_ctx_videoclip"

/** GNSDK_GDO_CHILD_VIDEOCLIP
  * Summary:
  *   Retrieves a child Video Clip context.
*/
#define GNSDK_GDO_CHILD_VIDEOCLIP							GNSDK_GDO_CONTEXT_VIDEOCLIP"!"

/** GNSDK_GDO_CONTEXT_MEDIAVOCS_CONTAINER
  * Summary:
  *   Container context to hold MediaVOCS data.
*/
#define GNSDK_GDO_CONTEXT_MEDIAVOCS_CONTAINER				"gnsdk_ctx_mediavocs_container"
/** GNSDK_GDO_CONTEXT_MEDIAVOCS_DATA
  * Summary:
  *   The MediaVOCS data context for accessing transcriptions and the display
  *   string (orthography).                                                  
*/
#define GNSDK_GDO_CONTEXT_MEDIAVOCS_DATA					"gnsdk_ctx_mediavocs_data"

/** GNSDK_GDO_CHILD_TITLE_MEDIAVOCS
  * Summary:
  *   Retrieves a child MediaVOCs title container context.
*/
#define GNSDK_GDO_CHILD_TITLE_MEDIAVOCS						GNSDK_GDO_CONTEXT_MEDIAVOCS_CONTAINER"!title"
/** GNSDK_GDO_CHILD_ARTIST_MEDIAVOCS
  * Summary:
  *   Retrieves a child MediaVOCS artist container context.
*/
#define GNSDK_GDO_CHILD_ARTIST_MEDIAVOCS					GNSDK_GDO_CONTEXT_MEDIAVOCS_CONTAINER"!artist"
/** GNSDK_GDO_CHILD_OFFICIAL_MEDIAVOCS
  * Summary:
  *   Retrieves the MediaVOCS data for the official orthography.
*/
#define GNSDK_GDO_CHILD_OFFICIAL_MEDIAVOCS					GNSDK_GDO_CONTEXT_MEDIAVOCS_DATA"!official"
/** GNSDK_GDO_CHILD_ALTERNATE_MEDIAVOCS
  * Summary:
  *   Retrieves the MediaVOCS data for an alternate orthography.
*/
#define GNSDK_GDO_CHILD_ALTERNATE_MEDIAVOCS					GNSDK_GDO_CONTEXT_MEDIAVOCS_DATA"!alternate"
/** GNSDK_GDO_CHILD_REGIONAL_MEDIAVOCS
  * Summary:
  *   Retrieve the MediaVOCS data for a regional (meaning, locale-dependent)
  *   \orthography.                                                         
*/
#define GNSDK_GDO_CHILD_REGIONAL_MEDIAVOCS					GNSDK_GDO_CONTEXT_MEDIAVOCS_DATA"!regional"

/** GNSDK_GDO_VALUE_DISPLAY
  * Summary:
  *   Retrieves the display value from a MediaVOCS data context GDO.
*/
#define GNSDK_GDO_VALUE_DISPLAY								"gnsdk_val_display"

/** GNSDK_GDO_VALUE_DISPLAY_LANGUAGE
  * Summary:
  *   Retrieves the display language from a MediaVOCS data context GDO.
*/
#define GNSDK_GDO_VALUE_DISPLAY_LANGUAGE					"gnsdk_val_displaylang"

/** GNSDK_GDO_VALUE_TRANSCRIPTION
  * Summary:
  *   Retrieves a phonetic transcription for a parent MediaVOCS or Alternate
  *   context GDO.                                                          
*/
#define GNSDK_GDO_VALUE_TRANSCRIPTION										"gnsdk_val_transcription"
/** GNSDK_GDO_VALUE_TRANSCRIPTION_IS_ORIGIN_LANGUAGE
  * Summary:
  *   Retrieves a language of origin value (either <i>True</i> or <i>False</i>)
  *   from a GDO.                                                              
*/
#define GNSDK_GDO_VALUE_TRANSCRIPTION_IS_ORIGIN_LANGUAGE					"gnsdk_val_is_origin_lang"
/** GNSDK_GDO_VALUE_TRANSCRIPTION_IS_MISPRONUNCIATION
  * Summary:
  *   Retrieves a mispronunciation value (either <i>True</i> or <i>False</i>)
  *   from a GDO.                                                            
*/
#define GNSDK_GDO_VALUE_TRANSCRIPTION_IS_MISPRONUNCIATION					"gnsdk_val_is_mispronunciation"
/** GNSDK_GDO_VALUE_TRANSCRIPTION_ALPHABET
  * Summary:
  *   Retrieves the phonetic alphabet of the transcription.
*/
#define GNSDK_GDO_VALUE_TRANSCRIPTION_ALPHABET								"gnsdk_val_alphabet"
/** GNSDK_GDO_VALUE_TRANSCRIPTION_LANGUAGE
  * Summary:
  *   Retrieves a transcription language value from a GDO.
*/
#define GNSDK_GDO_VALUE_TRANSCRIPTION_LANGUAGE								"gnsdk_val_transcription_lang"

/*
 * Spoken Languages for use in gnsdk_manager_gdo_set_transcription_lang().
 */

/** GNSDK_SPOKEN_LANG_UK_ENGLISH
  * Summary:
  *   Language value for English as spoken in the United Kingdom.
*/
#define			GNSDK_SPOKEN_LANG_UK_ENGLISH			"GBR_eng"
/** GNSDK_SPOKEN_LANG_USA_ENGLISH
  * Summary:
  *   Language value for English as spoken in the United States of America.
*/
#define			GNSDK_SPOKEN_LANG_USA_ENGLISH			"USA_eng"
/** GNSDK_SPOKEN_LANG_FRANCE_FRENCH
  * Summary:
  *   Language value for French as spoken in France.
*/
#define			GNSDK_SPOKEN_LANG_FRANCE_FRENCH			"FRA_fre"
/** GNSDK_SPOKEN_LANG_CANADIAN_FRENCH
  * Summary:
  *   Language value for French as spoken in Canada.
*/
#define			GNSDK_SPOKEN_LANG_CANADIAN_FRENCH		"CAN_fre"
/** GNSDK_SPOKEN_LANG_SPAIN_SPANISH
  * Summary:
  *   Language value for Castilian Spanish as spoken in Spain.
*/
#define			GNSDK_SPOKEN_LANG_SPAIN_SPANISH			"ESP_spa"
/** GNSDK_SPOKEN_LANG_MEXICO_SPANISH
  * Summary:
  *   Language value for Spanish as spoken in Mexico.
*/
#define			GNSDK_SPOKEN_LANG_MEXICO_SPANISH		"MEX_spa"
/** GNSDK_SPOKEN_LANG_GERMANY_GERMAN
  * Summary:
  *   Language value for German as spoken in Germany.
*/
#define			GNSDK_SPOKEN_LANG_GERMANY_GERMAN		"DEU_ger"
/** GNSDK_SPOKEN_LANG_ITALY_ITALIAN
  * Summary:
  *   Language value for Italian as spoken in Italy.
*/
#define			GNSDK_SPOKEN_LANG_ITALY_ITALIAN			"ITA_ita"
/** GNSDK_SPOKEN_LANG_JAPAN_JAPANESE
  * Summary:
  *   Language value for Japanese as spoken in Japan.
*/
#define			GNSDK_SPOKEN_LANG_JAPAN_JAPANESE		"JPN_jpn"
/** GNSDK_SPOKEN_LANG_CHINA_MANDARIN
  * Summary:
  *   Language value for Mandarin as spoken in China.
*/
#define			GNSDK_SPOKEN_LANG_CHINA_MANDARIN		"CHN_qad"
/** GNSDK_SPOKEN_LANG_RUSSIA_RUSSIAN
  * Summary:
  *   Language value for Russian as spoken in Russia.
*/
#define			GNSDK_SPOKEN_LANG_RUSSIA_RUSSIAN		"RUS_rus"
/** GNSDK_SPOKEN_LANG_PORTUGAL_PORTUGUESE
  * Summary:
  *   Language value for Portuguese as spoken in Portugal.
*/
#define			GNSDK_SPOKEN_LANG_PORTUGAL_PORTUGUESE	"PRT_por"
/** GNSDK_SPOKEN_LANG_BRAZIL_PORTUGUESE
  * Summary:
  *   Language value for Portuguese as spoken in Brazil.
*/
#define			GNSDK_SPOKEN_LANG_BRAZIL_PORTUGUESE		"BRA_por"
/** GNSDK_SPOKEN_LANG_NETHERLANDS_DUTCH
  * Summary:
  *   Language value for Dutch as spoken in the Netherlands.
*/
#define			GNSDK_SPOKEN_LANG_NETHERLANDS_DUTCH		"NLD_dut"
/** GNSDK_SPOKEN_LANG_TURKEY_TURKISH
  * Summary:
  *   Language value for Turkish as spoken in Turkey.
*/
#define			GNSDK_SPOKEN_LANG_TURKEY_TURKISH		"TUR_tur"
/** GNSDK_SPOKEN_LANG_AUSTRALIA_ENGLISH
  * Summary:
  *   Language value for English as spoken in Australia.
*/
#define			GNSDK_SPOKEN_LANG_AUSTRALIA_ENGLISH		"AUS_eng"
/** GNSDK_SPOKEN_LANG_KOREA_KOREAN
  * Summary:
  *   Language value for Korean as spoken in Korea.
*/
#define			GNSDK_SPOKEN_LANG_KOREA_KOREAN			"KOR_kor"


/** GNSDK_GDO_VALUE_TEMPO_META
  * Summary:
  *   Retrieves a tempo meta group value from a GDO. Use this key with sonic
  *   attribute data.                                                       
*/
#define GNSDK_GDO_VALUE_TEMPO_META				"gnsdk_val_tempometa"
/** GNSDK_GDO_VALUE_TEMPO_SUB
  * Summary:
  *   Retrieves a sub tempo value from a GDO. Use this key with sonic
  *   attribute data.                                                
*/
#define GNSDK_GDO_VALUE_TEMPO_SUB				"gnsdk_val_temposub"
/** GNSDK_GDO_VALUE_TEMPO_MICRO
  * Summary:
  *   Retrieves a tempo micro group value from a GDO. Use this key with sonic
  *   attribute data.                                                        
*/
#define GNSDK_GDO_VALUE_TEMPO_MICRO				"gnsdk_val_tempomicro"

/** GNSDK_GDO_VALUE_MOOD_SUB 
  * Summary:
  *   Retrieves a sub mood value from a GDO.
*/
#define GNSDK_GDO_VALUE_MOOD_SUB				"gnsdk_val_moodsub"
/** GNSDK_GDO_VALUE_MOOD_META 
  * Summary:
  *   Retrieves a mood meta group value from a GDO.
*/
#define GNSDK_GDO_VALUE_MOOD_META				"gnsdk_val_moodcat"


/** \ \ 
  * Summary:
  *   Retrieves a boolean string value representing a boolean TRUE.
*/
#define	GNSDK_GDO_VALUE_TRUE					"TRUE"
/** \ \ 
  * Summary:
  *   Retrieves a boolean string value representing a boolean FALSE.
*/
#define GNSDK_GDO_VALUE_FALSE					"FALSE"

/** \ \ 
  * Summary:
  *   \ \ 
  * Ignore Text:
  *   \ \ 
  *      Sony-specific value; hidden from other dvlprs
*/
#define GNSDK_GDO_VALUE_COMMERCE_TYPE_RETAIL	"1"
/** \ \ 
  * Summary:
  *   Ignore Text:
  *      Sony-specific value; hidden from other dvlprs
*/
#define GNSDK_GDO_VALUE_COMMERCE_TYPE_RENTAL	"2"


/*****************************************************************************
** GDO Keys specific to MATCH_INFO responses
*/
#define GNSDK_GDO_VALUE_MATCH_INFO_TYPE				"gnsdk_val_match_info_type"
#define GNSDK_GDO_CONTEXT_RESPONSE_MATCH_INFO		"gnsdk_ctx_response_matchinfo"
#define GNSDK_GDO_CONTEXT_MATCH_INFO				"gnsdk_ctx_matchinfo"

#define GNSDK_GDO_CHILD_MATCH_INFO					GNSDK_GDO_CONTEXT_MATCH_INFO"!"

#define GNSDK_GDO_VALUE_MATCH_INFO_TYPE_EXACT		"gnsdk_val_match_info_typed_exact"
#define GNSDK_GDO_VALUE_MATCH_INFO_TYPE_NO_MATCH	"gnsdk_val_match_info_typed_no_match"
#define GNSDK_GDO_VALUE_MATCH_INFO_TYPE_FP_NONEXIST	"gnsdk_val_match_info_typed__fingerprint_nonexist"
#define GNSDK_GDO_VALUE_MATCH_INFO_TYPE_UNDEFINED	"gnsdk_val_match_info_typed_undefined"

#ifdef __cplusplus
}
#endif

#endif /* _GNSDK_MANAGER_GDO_H_ */

