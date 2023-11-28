/** Gracenote SDK: Submit GDOs
  * Author:
  *   Copyright (c) 2012 Gracenote, Inc.
  *   
  *   This software may not be used in any way or distributed without
  *   permission. All rights reserved.
  *   
  *   Some code herein may be covered by US and international patents.
*/

#ifndef _GNSDK_SUBMIT_GDO_H_
/** gnsdk_submit_gdo.h: primary interface for editable GDOs.
*/
#define _GNSDK_SUBMIT_GDO_H_

#ifdef __cplusplus
extern "C"{
#endif

/** gnsdk_submit_edit_gdo_create_album_from_toc
  * Summary:
  *   Creates an editable Album GDO to submit data for a specific GDO context.
  *   The album comes pre-populated with the correct number of track children,
  *   but no metadata.
  * Parameters:
  *   toc:         [in] CD TOC string
  *   p_edit_gdo:  [out] Pointer to receive an editable GDO handle
  *                                                                           
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_edit_gdo_create_album_from_toc(
	gnsdk_cstr_t				toc,
	gnsdk_gdo_handle_t*			p_edit_gdo
	);

/** gnsdk_submit_edit_gdo_create_empty
  * Summary:
  *   Creates an editable GDO to submit data for a specific GDO context.
  * Parameters:
  *   context:     [in] GDO context from which the editable GDO is created
  *   p_edit_gdo:  [out] Pointer to receive an editable GDO handle
  *                                                                       
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_edit_gdo_create_empty(
	gnsdk_cstr_t				context,
	gnsdk_gdo_handle_t*			p_edit_gdo
	);

/** gnsdk_submit_edit_gdo_create_from_gdo
  * Summary:
  *   Creates an editable GDO with data derived from a source GDO.
  * Parameters:
  *   source_gdo:  [in] Source GDO for data used to create a new editable GDO
  *   p_edit_gdo:  [out] Pointer to receive an editable GDO handle
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_edit_gdo_create_from_gdo(
	gnsdk_gdo_handle_t			source_gdo,
	gnsdk_gdo_handle_t*			p_edit_gdo
	);

/** gnsdk_submit_edit_gdo_create_from_xml
  * Summary:
  *   Creates an editable GDO with data parsed from pre-populated,
  *   GDO-formatted XML.
  * Parameters:
  *   xml:         [in] Source XML for data used to create a new editable GDO
  *   p_edit_gdo:  [out] Pointer to receive an editable GDO handle
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_edit_gdo_create_from_xml(
	gnsdk_cstr_t				xml,
	gnsdk_gdo_handle_t*			p_edit_gdo
	);

/** gnsdk_submit_edit_gdo_value_set
  * Summary:
  *   Changes a GDO value for a supported key of an editable GDO. If the value
  *   does not exist, it will be added. If the value does exist, it will be
  *   changed. If NULL or an empty string is passed in, the value will be
  *   deleted.
  * Parameters:
  *   edit_gdo_handle:  [in] Handle to an editable GDO
  *   key:              [in] An available <link !!MACROS_mgr_GDO_valuekeys, GDO Value Key>
  *   ordinal:          [in] Updates the n'th instance of the key (1\-based)
  *   value:            [in] Value corresponding to the specified GDO value key
  *                                                                                       
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_edit_gdo_value_set(
	gnsdk_gdo_handle_t			edit_gdo_handle,
	gnsdk_cstr_t				key,
	gnsdk_uint32_t				ordinal,
	gnsdk_cstr_t				value
	);

/** gnsdk_submit_edit_gdo_value_has_changed
  * Summary:
  *   \Returns a boolean value indicating whether the specified GDO value has
  *   changed. Changes include adding, deleting, and changing data.
  * Parameters:
  *   edit_gdo_handle:  [in] Handle to an editable GDO
  *   key:              [in] An available <link !!MACROS_mgr_GDO_valuekeys, GDO Value Key>
  *   ordinal:          [in] Updates the n'th instance of the key (1\-based)
  *   p_has_changed:    [out] GNSDK_TRUE if changed, GNSDK_FALSE otherwise
  *                                                                                       
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_edit_gdo_value_has_changed(
	gnsdk_gdo_handle_t			edit_gdo_handle,
	gnsdk_cstr_t				key,
	gnsdk_uint32_t				ordinal,
	gnsdk_bool_t*				p_has_changed
	);

/** gnsdk_submit_edit_gdo_child_add_empty
  * Summary:
  *   Adds an empty child GDO to a editable parent GDO.
  * Parameters:
  *   edit_gdo_handle:  [in] Handle to an editable GDO
  *   child_key:        [in] An available <link !!MACROS_mgr_GDO_Childkeys, GDO child key>
  *                     that corresponds with the parent editable GDO handle's
  *                     context; for example, a Track or Credit child key for a
  *                     parent Album context GDO
  *   p_edit_gdo:       [out] Pointer to receive an editable child GDO handle
  *                                                                                       
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_edit_gdo_child_add_empty(
	gnsdk_gdo_handle_t			edit_gdo_handle,
	gnsdk_cstr_t				child_key,
	gnsdk_gdo_handle_t*			p_edit_gdo
	);
/** gnsdk_submit_edit_gdo_child_add_from_gdo
  * Summary:
  *   Adds a derived editable child GDO to an editable parent GDO.
  * Parameters:
  *   edit_gdo_handle:   [in] Handle to an editable GDO
  *   child_key:         [in] An available <link !!MACROS_mgr_GDO_Childkeys, GDO child key>
  *                      that corresponds with the parent editable GDO handle's
  *                      context; for example, a Track or Credit child key for
  *                      a parent Album context GDO
  *   child_gdo_handle:  [in] Handle to a derived editable child GDO Handle to
  *                      a derived editable child GDO
  *   p_edit_gdo:        [out] Pointer to receive an editable child GDO handle
  *                                                                                        
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_edit_gdo_child_add_from_gdo(
	gnsdk_gdo_handle_t			edit_gdo_handle,
	gnsdk_cstr_t				child_key,
	gnsdk_gdo_handle_t			child_gdo_handle,
	gnsdk_gdo_handle_t*			p_edit_gdo
	);
/** gnsdk_submit_edit_gdo_child_add_from_xml
  * Summary:
  *   Adds a child GDO derived from source XML to an editable parent GDO.
  * Parameters:
  *   edit_gdo_handle:  [in] Handle to an editable GDO derived from XML source
  *                     data
  *   child_key:        [in] An available <link !!MACROS_mgr_GDO_Childkeys, GDO child key>
  *                     that corresponds with the parent editable GDO handle's
  *                     context; for example, a Track or Credit child key for a
  *                     parent Album context GDO
  *   child_xml:        [in] Handle to a GDO child derived from source XML
  *   p_edit_gdo:       [out] Pointer to receive an editable child GDO handle
  *                                                                                       
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_edit_gdo_child_add_from_xml(
	gnsdk_gdo_handle_t			edit_gdo_handle,
	gnsdk_cstr_t				child_key,
	gnsdk_cstr_t				child_xml,
	gnsdk_gdo_handle_t*			p_edit_gdo
	);
/** gnsdk_submit_edit_gdo_child_remove
  * Summary:
  *   Removes an editable child GDO from a parent GDO. 
  * Parameters:
  *   edit_gdo_handle:       [in] Handle to the parent editable GDO
  *   edit_gdo_child_handle: [in] Handle to the child GDO
  *                                                                                                           
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_edit_gdo_child_remove(
	gnsdk_gdo_handle_t			edit_gdo_handle,
	gnsdk_gdo_handle_t			edit_gdo_child_handle
	);

/** gnsdk_submit_edit_gdo_list_value_set_by_submit_id
  * Summary:
  *   Sets a list-based value by the list item Submit ID for an editable GDO.
  *   If the value does not exist, it is added. If the value does exist, it is
  *   changed. If the list_item_submit_id is 0, the value is deleted.
  * Parameters:
  *   edit_gdo_handle:      [in] Handle to an editable GDO
  *   list_type:            [in] An available <link !!MACROS_mgr_list_types, GNSDK Manager list type>
  *   ordinal:              [in] Set the n'th item of the list type (1\-based)
  *   list_item_submit_id:  [in] Submit ID for a list item of the specified
  *                         list type
  *                                                                                                  
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_edit_gdo_list_value_set_by_submit_id(
	gnsdk_gdo_handle_t			edit_gdo_handle,
	gnsdk_cstr_t				list_type,
	gnsdk_uint32_t				ordinal,
	gnsdk_uint32_t				list_item_submit_id
	);

/** gnsdk_submit_edit_gdo_list_value_get_submit_id
  * Summary:
  *   Gets the list item Submit ID of a list-based value from an editable GDO.
  * Parameters:
  *   edit_gdo_handle:        [in] Handle to an editable GDO
  *   list_type:              [in] An available <link !!MACROS_mgr_list_types, GNSDK Manager list type>
  *   ordinal:                [in] Set the n'th item of the list type (1\-based)
  *   p_list_item_submit_id:  Submit ID for a list item of the specified list
  *                           type
  *                                                                                                    
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_edit_gdo_list_value_get_submit_id(
	gnsdk_gdo_handle_t			edit_gdo_handle,
	gnsdk_cstr_t				list_type,
	gnsdk_uint32_t				ordinal,
	gnsdk_uint32_t*				p_list_item_submit_id
	);


/** gnsdk_submit_gdo_validate_callback_fn
  * Summary:
  *   Receive Submit validation status updates.
  * Parameters:
  *   user_data:        [in] Pointer to data passed in to
  *                     gnsdk_submit_edit_gdo_validate_set_callback through the
  *                     callback_userdata parameter. This pointer must be cast
  *                     from the gnsdk_void_t type to its original type to be
  *                     accessed properly.
  *   edit_gdo_handle:  [in] Handle to an editable GDO
  *   p_error_info:     [out] Pointer to a gnsdk_extended_error_info_t
  *                     structure
  *   invalid_key:      [in] Key that has a validation error
  *   invalid_ord:      [in] Ordinal for the invalid key
  *   p_abort:          [out] Pointer to receive boolean value indicating if
  *                     the callback must be aborted
  *                                                                            
*/
typedef gnsdk_void_t
(GNSDK_CALLBACK_API *gnsdk_submit_gdo_validate_callback_fn)(
	const gnsdk_void_t*						user_data,
	gnsdk_gdo_handle_t						edit_gdo_handle,
	gnsdk_extended_error_info_t*			p_error_info,
	gnsdk_cstr_t							invalid_key,
	gnsdk_uint32_t							invalid_ord,
	gnsdk_bool_t*							p_abort
	);

/** gnsdk_submit_edit_gdo_validate_set_callback
  * Summary:
  *   Retrieves a boolean value indicating if validation status is applied to
  *   the GDO's children.
  * Parameters:
  *   edit_gdo_handle:    [in] Handle to an editable GDO
  *   callback_fn:        [in_opt] Callback function for status and progress
  *   callback_userdata:  [in_opt] Data that is passed back through calls to
  *                       the callback functions
  *   use_for_children:   [out] Returns boolean value; if true, indicates the
  *                       callback is applied to all the GDO's children
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_edit_gdo_validate_set_callback(
	gnsdk_gdo_handle_t						edit_gdo_handle,
	gnsdk_submit_gdo_validate_callback_fn	callback_fn,
	gnsdk_void_t*							callback_userdata,
	gnsdk_bool_t							use_for_children /* if true, this callback will be used for all child gdos */
	);

/** <unfinished>
  * 
  * gnsdk_submit_edit_gdo_validate
  * Summary:
  *   \Returns a boolean value indicating whether an edited GDO is valid.
  * Parameters:
  *   edit_gdo_handle:  [in] Handle to an editable GDO
  *   p_has_errors:     [out] GNSDK_TRUE if GDO has errors, GNSDK_FALSE
  *                     otherwise.
  *                                                                      
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_edit_gdo_validate(
	gnsdk_gdo_handle_t						edit_gdo_handle,
	gnsdk_bool_t*							p_has_errors
	);


/** GNSDK_GDO_VALUE_TOC_ALBUM
  * Summary:
  *   Sets an album TOC value from a GDO.
*/
#define GNSDK_GDO_VALUE_TOC_ALBUM				"gnsdk_val_toc_album"


#ifdef __cplusplus
}
#endif

#endif /* _GNSDK_SUBMIT_GDO_H_ */

