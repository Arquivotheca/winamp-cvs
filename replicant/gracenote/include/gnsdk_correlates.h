/** Gracenote SDK: GNSDK Manager Public Header File
  * Author:
  *   Copyright (c) 2012 Gracenote, Inc.
  *   
  *   This software may not be used in any way or distributed without
  *   permission. All rights reserved.
  *   
  *   Some code herein may be covered by USA and international patents.
*/

/** gnsdk_correlates.h: Implementation of GNSDK Correlate APIs
*/

#ifndef _GNSDK_CORRELATES_H_
#define _GNSDK_CORRELATES_H_

#ifdef __cplusplus
extern "C"{
#endif


gnsdk_error_t GNSDK_API
gnsdk_correlates_initialize(gnsdk_manager_handle_t sdkmgr_handle);

gnsdk_error_t GNSDK_API
gnsdk_correlates_shutdown(void);

gnsdk_cstr_t GNSDK_API
gnsdk_correlates_get_version(void);

gnsdk_cstr_t GNSDK_API
gnsdk_correlates_get_build_date(void);

/*
** Correlates Handle
*/

#ifndef _GNSDK_MANAGER_PRIVILEGED_H_

	GNSDK_DECLARE_HANDLE( gnsdk_list_correlates_handle_t );

	/* Correlate Types
	** Use one of the following for the correlates_type parameter 
	*/
	#define	GNSDK_CORRELATES_TYPE_GENRES					"gnsdk_correlates_genre"
	#define	GNSDK_CORRELATES_TYPE_ORIGINS					"gnsdk_correlates_origin"
	#define	GNSDK_CORRELATES_TYPE_ERAS						"gnsdk_correlates_era"
	#define	GNSDK_CORRELATES_TYPE_ARTISTTYPES				"gnsdk_correlates_arttype"
	#define	GNSDK_CORRELATES_TYPE_MOODS						"gnsdk_correlates_mood"

#endif /* _GNSDK_MANAGER_PRIVILEGED_H_ */

/* 
 * Correlates APIs
 */

gnsdk_error_t GNSDK_API
gnsdk_correlates_retrieve(
	gnsdk_cstr_t					correlates_type, 
	gnsdk_user_handle_t				user_handle,
	gnsdk_manager_query_callback_fn	callback,
	const gnsdk_void_t*				callback_user_data,
	gnsdk_list_correlates_handle_t* p_correlates_handle
	);


gnsdk_error_t GNSDK_API
gnsdk_correlates_data_revision(
	gnsdk_list_correlates_handle_t	correlates_handle,
	gnsdk_uint32_t*					p_revision
	);


gnsdk_error_t GNSDK_API
gnsdk_correlates_render_set(
	gnsdk_list_correlates_handle_t	correlates_handle,
	gnsdk_uint32_t					master_id,
	gnsdk_str_t*					p_xml
	);

/* Master ID GDO keys
** Use these keys to retrieve master_id values from Album and Track GDOs
*/
#define GNSDK_GDO_VALUE_CORRELATE_GENRE_ID				"_sdkmgr_val_list_genre_id"
#define GNSDK_GDO_VALUE_CORRELATE_ORIGIN_ID				"_sdkmgr_val_list_origin_id"
#define GNSDK_GDO_VALUE_CORRELATE_ERA_ID				"_sdkmgr_val_list_era_id"
#define GNSDK_GDO_VALUE_CORRELATE_ARTISTTYPE_ID			"_sdkmgr_val_list_artist_type_id"
#define GNSDK_GDO_VALUE_CORRELATE_MOOD_ID				"_sdkmgr_val_list_mood_id"


gnsdk_error_t GNSDK_API
gnsdk_correlates_release(
	gnsdk_list_correlates_handle_t	correlates_handle
	);



#ifdef __cplusplus
}
#endif

#endif /* _GNSDK_CORRELATES_H_ */

