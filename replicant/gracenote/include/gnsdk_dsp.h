/** Gracenote SDK: DSP public header file
  * Author:
  *   Copyright (c) 2012 Gracenote, Inc.
  *   
  *   This software may not be used in any way or distributed without
  *   permission. All rights reserved.
  *   
  *   Some code herein may be covered by US and international patents.
*/

#ifndef _GNSDK_DSP_H_
/** gnsdk_dsp.h: primary interface for the DSP SDK
*/
#define _GNSDK_DSP_H_

#ifdef __cplusplus
extern "C"{
#endif

/*
 * gnsdk_dsp.h:	Fingerprinting and other DSP implementations
 *  
 */

/******************************************************************************
 * GNSDK DSP Initialization APIs
 ******************************************************************************/

/** gnsdk_dsp_initialize
  * Summary:
  *   Initializes the DSP for GNSDK
  * Parameters:
  *   sdkmgr_handle:  [in] Handle from a successful gnsdk_manager_initialize
  *                   call
  *                                                                            
*/
gnsdk_error_t GNSDK_API
gnsdk_dsp_initialize(
	gnsdk_manager_handle_t	sdkmgr_handle
	);

/** gnsdk_dsp_shutdown
  * Summary:
  *   Shut downs and releases resources for the DSP library.
*/
gnsdk_error_t GNSDK_API
gnsdk_dsp_shutdown(void);

/** gnsdk_dsp_get_version
  * Summary:
  *   Retrieves DSP's version string for GNSDK.
*/
gnsdk_cstr_t GNSDK_API
gnsdk_dsp_get_version(void);

/** gnsdk_dsp_get_build_date
  * Summary:
  *   Retrieves the DSP's build date string for GNSDK.
*/
gnsdk_cstr_t GNSDK_API
gnsdk_dsp_get_build_date(void);

/******************************************************************************
** Direct DSP Feature Creation APIs
*/
typedef gnsdk_handle_t		gnsdk_dsp_feature_handle_t;

typedef gnsdk_uint32_t		gnsdk_dsp_feature_qualities_t;

#define GNSDK_DSP_FEATURE_QUALITY_DEFAULT		0x0
#define GNSDK_DSP_FEATURE_QUALITY_SHORT			0x1
#define GNSDK_DSP_FEATURE_QUALITY_SILENT		0x2

gnsdk_error_t GNSDK_API
gnsdk_dsp_feature_audio_begin(
	gnsdk_user_handle_t				user_handle,
	gnsdk_cstr_t					feature_type,
	gnsdk_uint32_t					audio_sample_rate,
	gnsdk_uint32_t					audio_sample_size,
	gnsdk_uint32_t					audio_channels,
	gnsdk_dsp_feature_handle_t*		p_feature_handle
	);

gnsdk_error_t GNSDK_API
gnsdk_dsp_feature_audio_write(
	gnsdk_dsp_feature_handle_t		feature_handle,
	const gnsdk_void_t*				audio_data,
	gnsdk_size_t					audio_data_bytes,
	gnsdk_bool_t*					pb_complete
	);

gnsdk_error_t GNSDK_API
gnsdk_dsp_feature_end_of_write(
	gnsdk_dsp_feature_handle_t		feature_handle
	);

gnsdk_error_t GNSDK_API
gnsdk_dsp_feature_retrieve_data(
	gnsdk_dsp_feature_handle_t		feature_handle,
	gnsdk_dsp_feature_qualities_t*	p_feature_qualities,
	gnsdk_cstr_t*					p_feature_data
	);

gnsdk_error_t GNSDK_API
gnsdk_dsp_feature_release(
	gnsdk_dsp_feature_handle_t		feature_handle
	);


#ifdef __cplusplus
}
#endif

#endif /* _GNSDK_DSP_H_ */

