/** Gracenote SDK: Submit public header file
  * Author:
  *   Copyright (c) 2012 Gracenote, Inc.
  *   
  *   This software may not be used in any way or distributed without
  *   permission. All rights reserved.
  *   
  *   Some code herein may be covered by US and international patents.
*/

/** gnsdk_submit.h: Primary interface for the Submit SDK.
*/

#ifndef _GNSDK_SUBMIT_H_
#define _GNSDK_SUBMIT_H_

#ifdef __cplusplus
extern "C"{
#endif

/******************************************************************************
 * Typdefs
 ******************************************************************************/

/** <title gnsdk_submit_parcel_handle_t>
  * <toctitle gnsdk_submit_parcel_handle_t>
  * 
  * gnsdk_submit_parcel_handle_t
  * Summary:
  *   Handle to Submit parcel. Created by gnsdk_submit_parcel_create, the
  *   application must create this handle to execute a submit.           
*/
GNSDK_DECLARE_HANDLE( gnsdk_submit_parcel_handle_t );

/** gnsdk_submit_status_t
  * Summary:
  *   The Submit callback function status values; indicates the status value
  *   \of the submission or the feature lookup, passed to
  *   gnsdk_submit_callback_fn.                                             
*/
typedef enum
{
	gnsdk_submit_status_unknown = 0,/** gnsdk_submit_status_t@1::gnsdk_submit_status_unknown
	                                  * Summary:
	                                  *   Submission status is unknown; this is the initial callback state.
	                                */

	gnsdk_submit_status_submit_begin = 10,/** gnsdk_submit_status_t@1::gnsdk_submit_status_submit_begin
	                                 * Summary:
	                                 *   Submit is beginning a new network transaction.
	                               */

	gnsdk_submit_status_connecting = 20,
	gnsdk_submit_status_sending = 30,/** gnsdk_submit_status_t@1::gnsdk_submit_status_sending
	                              * Summary:
	                              *   Submit is sending parcel data.
	                            */

	gnsdk_submit_status_receiving = 40,/** gnsdk_submit_status_t@1::gnsdk_submit_status_receiving
	                                * Summary:
	                                *   Submit is receiving parcel data.
	                              */

	gnsdk_submit_status_submit_complete = 100,/** gnsdk_submit_status_t@1::gnsdk_submit_status_submit_complete
	                                    * Summary:
	                                    *   Submit has completed the network transaction.
	                                  */

} gnsdk_submit_status_t;

/** gnsdk_submit_callback_fn
  * Summary:
  *   Retrieve Submit status updates as content.
  * Parameters:
  *   user_data:      [in] Pointer to data passed in to the
  *                   gnsdk_submit_parcel_create function through the
  *                   callback_userdata parameter. This pointer must be cast
  *                   from the gnsdk_void_t type to its original type to be
  *                   accessed properly.
  *   parcel_handle:  [in] Submit parcel handle that the callback operates on
  *   status:         [in] One of gnsdk_submit_status_t values
  *   bytes_done:     [in] Current number of bytes transferred. Set to a value
  *                   greater than 0 to indicate progress, or 0 to indicate no
  *                   progress.
  *   bytes_total:    [in] Total number of bytes to be transferred. Set to a
  *                   value greater than 0 to indicate progress, or 0 to
  *                   indicate no progress.
  *   p_abort:        [out] Set dereferenced value to GNSDK_TRUE to abort the
  *                   operation that is calling the callback
  *                                                                           
*/
typedef gnsdk_void_t
(GNSDK_CALLBACK_API *gnsdk_submit_callback_fn)(
	const gnsdk_void_t*				user_data,
	gnsdk_submit_parcel_handle_t	parcel_handle,
	gnsdk_submit_status_t			status,
	gnsdk_size_t					bytes_done,
	gnsdk_size_t					bytes_total,
	gnsdk_bool_t*					p_abort
	);

/** gnsdk_submit_state_t
  * Summary:
  *   Indicates the submit state of the parcel data.
*/
typedef enum
{
	gnsdk_submit_state_unknown	= 0,/** gnsdk_submit_state_t@1::gnsdk_submit_state_unknown
	                          	      * Summary:
	                          	      *   Parcel data submittal state is unknown; this is the initial state.
	                          	    */

	gnsdk_submit_state_nothing_to_do,/** gnsdk_submit_state_t@1::gnsdk_submit_state_nothing_to_do
	                                   * Summary:
	                                   *   No useful data to submit and process from the track.
	                                 */

	gnsdk_submit_state_ready_for_audio,/** gnsdk_submit_state_t@1::gnsdk_submit_state_ready_for_audio
	                                     * Summary:
	                                     *   Parcel data is ready for audio data.
	                                   */

	gnsdk_submit_state_processing_error,/** gnsdk_submit_state_t@1::gnsdk_submit_state_processing_error
	                                     * Summary:
	                                     *   An error was encountered during processing of the data.
	                                   */

	gnsdk_submit_state_ready_to_upload,/** gnsdk_submit_state_t@1::gnsdk_submit_state_ready_to_upload
	                                     * Summary:
	                                     *   Parcel data is ready for uploading.
	                                   */

	gnsdk_submit_state_upload_succeeded,/** gnsdk_submit_state_t@1::gnsdk_submit_state_upload_succeeded
	                                      * Summary:
	                                      *   Parcel data submittal succeeded.
	                                    */

	gnsdk_submit_state_upload_partially_succeeded,/** gnsdk_submit_state_t@1::gnsdk_submit_state_upload_partially_succeeded
	                                                * Summary:
	                                                *   Parcel data submittal partially succeeded.
	                                              */

	gnsdk_submit_state_upload_failed/** gnsdk_submit_state_t@1::gnsdk_submit_state_upload_failed
	                                  * Summary:
	                                  *   Parcel data submittal failed.
	                                */

} gnsdk_submit_state_t;

/** gnsdk_submit_audio_format_t@1
  * Summary:
  *   Audio format types
*/

typedef enum
{
	gnsdk_submit_audio_format_unknown = 0,/** gnsdk_submit_audio_format_t@1::gnsdk_submit_audio_format_unknown
	                                        * Summary:
	                                        *   Unknown audio format; this is the initial state, prior to obtaining
	                                        *   format information.
	                                      */

	gnsdk_submit_audio_format_pcm16,/** gnsdk_submit_audio_format_t@1::gnsdk_submit_audio_format_pcm16
	                                  * Summary:
	                                  *   Audio format of PCM 16
	                                */

} gnsdk_submit_audio_format_t;

/** gnsdk_submit_initialize
  * Summary:
  *   Initializes the GNSDK Submit library.
  * Parameters:
  *   sdkmgr_handle:  [in] Handle from a successful gnsdk_manager_initialize
  *                   call
  *                                                                         
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_initialize(
	gnsdk_manager_handle_t sdkmgr_handle
	);

/** gnsdk_submit_shutdown
  * Summary:
  *   Shuts down and releases resources for the Submit library.
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_shutdown(void);

/** gnsdk_submit_get_version
  * Summary:
  *   Retrieves version string of Submit library.
*/
gnsdk_cstr_t GNSDK_API
gnsdk_submit_get_version(void);

/** gnsdk_submit_get_build_date
  * Summary:
  *   Retrieves build date string of Submit library.
*/
gnsdk_cstr_t GNSDK_API
gnsdk_submit_get_build_date(void);

/******************************************************************************
 * Submit Parcel Handle - for the life of the Submit parcel
 ******************************************************************************/

/** gnsdk_submit_parcel_create
  * Summary:
  *   Creates a handle to a parcel. A single parcel can contain multiple types
  *   \of data.
  * Parameters:
  *   user_handle:        [in] User handle for the user making the submit
  *                       request
  *   callback_fn:        [in_opt] Callback function for status and progress
  *   callback_userdata:  [in_opt] Data that is passed back to calls to the
  *                       callback function
  *   p_parcel_handle:    [out] Pointer to receive the parcel handle
  *                                                                           
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_parcel_create(
	gnsdk_user_handle_t				user_handle,
	gnsdk_submit_callback_fn		callback_fn,
	gnsdk_void_t*					callback_userdata,
	gnsdk_submit_parcel_handle_t*	p_parcel_handle
	);

/** gnsdk_submit_parcel_release
  * Summary:
  *   Invalidates and releases resources for a given Submit handle.
  * Parameters:
  *   parcel_handle:  [in] Handle to a parcel to release
  *                                                                
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_parcel_release(
	gnsdk_submit_parcel_handle_t	parcel_handle
	);

/** gnsdk_submit_parcel_data_add_gdo
  * Summary:
  *   Adds a GDO containing metadata to a parcel for submission to the
  *   Gracenote Service.
  * Parameters:
  *   parcel_handle:  [in] Handle to a parcel
  *   gdo:            [in] Handle to a GDO of a supported context
  *   data_ident:     [in] String to uniquely identify the data contained in the
  *                   parcel
  *                                                                             
*/

gnsdk_error_t GNSDK_API
gnsdk_submit_parcel_data_add_gdo(
	gnsdk_submit_parcel_handle_t	parcel_handle,
	gnsdk_gdo_handle_t				gdo,
	gnsdk_cstr_t					data_ident /* a string to uniquely identify this data inside the parcel */
);

/******************************************************************************
 * Submit Parcel audio Feature APIs - for the generation and submission of
 * features generated from audio streams.
 ******************************************************************************/

/** gnsdk_submit_parcel_data_init_features
  * Summary:
  *   Prepares a parcel for gathering and generating features.
  * Parameters:
  *   parcel_handle:      [in] Handle to a parcel
  *   gdo:                [in] Handle to a GDO; note that these are generally not
  *                       editable GDOs
  *   flags:              [in] One of the available <link !!MACROS_Submit_parcel_flags, Submit Parcel Flags>
  *   p_something_to_do:  [out] Pointer to receive a boolean value indicating whether
  *                       there is any processing work required
  *                                                                                                    
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_parcel_data_init_features(
	gnsdk_submit_parcel_handle_t		parcel_handle,
	gnsdk_gdo_handle_t					gdo,
	gnsdk_uint32_t						flags,
	gnsdk_bool_t*						p_something_to_do
	);

/** GNSDK_SUBMIT_PARCEL_FEATURE_FLAG_DEFAULT
  * Summary:
  *   Default features initialize flag.     
*/

#define	GNSDK_SUBMIT_PARCEL_FEATURE_FLAG_DEFAULT			0x00

/** gnsdk_submit_parcel_feature_init_audio
  * Summary:
  *   Initializes the generation of features for a specific audio stream.
  * Parameters:
  *   parcel_handle:   [in] Handle to a parcel
  *   data_id:         [in] Unique identifier for the audio stream
  *   track_num:       [in] Ordinal for the audio stream (for albums, this is the
  *                    track number)
  *   audio_rate:      [in] Sample rate of audio to be provided ( for example\:
  *                    44100)
  *   audio_format:    [in] The audio format (see <link gnsdk_submit_audio_format_t, gnsdk_submit_audio_format_t Enumeration>)
  *   audio_channels:  [in] Number of channels for audio to be provided (for
  *                    \example\: 2)
  *                                                                                                                           
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_parcel_feature_init_audio(
	gnsdk_submit_parcel_handle_t		parcel_handle,
	gnsdk_cstr_t						data_id,		/* a way to uniquely identify the audio stream */
	gnsdk_uint32_t						track_num,		/* the track number for this stream */
	gnsdk_uint32_t						audio_rate,
	gnsdk_submit_audio_format_t			audio_format,
	gnsdk_uint32_t						audio_channels
	);

/** gnsdk_submit_parcel_feature_option_set
  * Summary:
  *   Sets an option for a specific audio stream in a parcel.
  * Parameters:
  *   parcel_handle:  [in] Handle to a parcel
  *   data_id:        [in] Unique identifier for an audio stream
  *   option_key:     [in] One of the available <link !!MACROS_Submit_feature_option_keys, Submit feature option keys>
  *   option_value:   [in] One of the available <link !!MACROS_Submit_feature_option_values, Submit feature option values>
  *                   that corresponds to the selected option key
  *                                                                                                                       
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_parcel_feature_option_set(
	gnsdk_submit_parcel_handle_t		parcel_handle,
	gnsdk_cstr_t						data_id,		/* a way to uniquely identify the audio stream */
	gnsdk_cstr_t						option_key,
	gnsdk_cstr_t						option_value
	);

/** gnsdk_submit_parcel_feature_option_get
  * Summary:
  *   Retrieves an option for a specific audio stream in a parcel.
  * Parameters:
  *   parcel_handle:   [in] Handle to a parcel
  *   data_id:         [in] Unique identifier for an audio stream
  *   option_key:      [in] One of the available <link !!MACROS_Submit_feature_option_keys, Submit feature option keys>
  *   p_option_value:  [out] The value for the specified key
  *                                                                                                                    
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_parcel_feature_option_get(
	gnsdk_submit_parcel_handle_t		parcel_handle,
	gnsdk_cstr_t						data_id,		/* a way to uniquely identify the audio stream */
	gnsdk_cstr_t						option_key,
	gnsdk_cstr_t*						p_option_value
	);

/** gnsdk_submit_parcel_feature_write_audio_data
  * Summary:
  *   Processes audio stream data contained in a parcel.
  * Parameters:
  *   parcel_handle:     [in] Handle to a parcel
  *   data_id:           [in] Unique identifier for an audio stream
  *   audio_data:        [in] Pointer to audio data buffer that matches the
  *                      audio format described to
  *                      gnsdk_submit_parcel_feature_init_audio
  *   audio_data_bytes:  [in] Number of audio data bytes in this sample
  *   p_complete:        [out] Pointer to receive boolean value as to whether
  *                      the write process has received enough data
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_parcel_feature_write_audio_data(
	gnsdk_submit_parcel_handle_t		parcel_handle,
	gnsdk_cstr_t						data_id,		/* a way to uniquely identify the audio stream */
	const gnsdk_void_t*					audio_data,
	gnsdk_size_t						audio_data_bytes,
	gnsdk_bool_t*						p_complete
	);

/** gnsdk_submit_parcel_feature_finalize
  * Summary:
  *   Finalizes the processing of the audio for the given stream.
  * Parameters:
  *   parcel_handle:  [in] Handle to a parcel
  *   data_id:        [in] Unique identifier for an audio stream
  *   abort:          [in] Boolean value to indicate whether the finalization
  *                   process must stop operating, due to processing errors
  *                                                                          
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_parcel_feature_finalize(
	gnsdk_submit_parcel_handle_t		parcel_handle,
	gnsdk_cstr_t						data_id,		/* a way to uniquely identify the audio stream */
	gnsdk_bool_t						abort 			/* to differentiate between success and stopping do to an error */
	);

/** GNSDK_SUBMIT_UPLOAD_FLAG_DEFAULT
  * Summary:
  *   Flag to indicate that the parcel data is being uploaded to the Gracenote
  *   Service.
*/

#define	GNSDK_SUBMIT_PARCEL_UPLOAD_FLAG_DEFAULT			0x00
/** GNSDK_SUBMIT_PARCEL_UPLOAD_FLAG_TEST_MODE
  * Summary:
  *   Flag for development test purposes only, used to validate the submission
  *   logic without inadvertently adding invalid data to the Gracenote
  *   Service.
  *   
  *   After Gracenote validates the application's submission logic, clear this
  *   flag to enable active submittals.                                       
*/
#define	GNSDK_SUBMIT_PARCEL_UPLOAD_FLAG_TEST_MODE		0x01

/** GNSDK_SUBMIT_FEATURE_OPTION_SOURCE_NAME
  * Summary:
  *   Indicates the feature source name.   
*/
#define GNSDK_SUBMIT_FEATURE_OPTION_SOURCE_NAME				"gnsdk_submit_feature_source_name"
/** GNSDK_SUBMIT_FEATURE_OPTION_SOURCE_ID
  * Summary:
  *   Indicates the feature source ID.   
*/
#define GNSDK_SUBMIT_FEATURE_OPTION_SOURCE_ID				"gnsdk_submit_feature_source_id"
/** GNSDK_SUBMIT_FEATURE_OPTION_SOURCE_DESC
  * Summary:
  *   Indicates the feature source description.
*/
#define GNSDK_SUBMIT_FEATURE_OPTION_SOURCE_DESC				"gnsdk_submit_feature_source_desc"

/** GNSDK_SUBMIT_FEATURE_OPTION_SOURCE_BITRATE
* Summary:
*   Indicates the bitrate source.           
*/
#define GNSDK_SUBMIT_FEATURE_OPTION_SOURCE_BITRATE			"gnsdk_submit_feature_source_bps"		
/** GNSDK_SUBMIT_FEATURE_OPTION_SOURCE_BITRATE_TYPE
  * Summary:
  *   Indicates the bitrate type source.           
*/
#define GNSDK_SUBMIT_FEATURE_OPTION_SOURCE_BITRATE_TYPE		"gnsdk_submit_feature_source_bps_type"

/** GNSDK_SUBMIT_FEATURE_OPTION_VALUE_SOURCE_NAME_CDDA
  * Summary:
  *   Represents a feature source name of CDDA.       
*/
#define GNSDK_SUBMIT_FEATURE_OPTION_VALUE_SOURCE_NAME_CDDA				"CD Track"
/** GNSDK_SUBMIT_FEATURE_OPTION_VALUE_SOURCE_ID_CDDA
  * Summary:
  *   Represents a feature source ID of CDDA.       
*/
#define GNSDK_SUBMIT_FEATURE_OPTION_VALUE_SOURCE_ID_CDDA				"CDDA"
/** GNSDK_SUBMIT_FEATURE_OPTION_VALUE_SOURCE_DESC_CDDA
  * Summary:
  *   Represents a feature source description of CDDA.
*/
#define GNSDK_SUBMIT_FEATURE_OPTION_VALUE_SOURCE_DESC_CDDA				"CD Audio"
/** GNSDK_SUBMIT_FEATURE_OPTION_VALUE_SOURCE_BITRATE_1411200
  * Summary:
  *   Represents a feature source bitrate of 1,411,200 bit/s.
*/
#define GNSDK_SUBMIT_FEATURE_OPTION_VALUE_SOURCE_BITRATE_1411200		"1411200"
/** GNSDK_SUBMIT_FEATURE_OPTION_VALUE_SOURCE_BITRATE_TYPE_CONSTANT
  * Summary:
  *   Represents a feature source constant bitrate type.          
*/
#define GNSDK_SUBMIT_FEATURE_OPTION_VALUE_SOURCE_BITRATE_TYPE_CONSTANT	"CONSTANT"

/** gnsdk_submit_parcel_upload
  * Summary:
  *   Uploads a parcel to the Gracenote Service.
  * Parameters:
  *   parcel_handle:  [in] Handle to a parcel to upload
  *   flags:          [in] An available <link !!MACROS_Submit_parcel_flags, Submit parcel flag>
  *   p_state:        [out] Pointer to receive a submit state
  *                                                                                            
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_parcel_upload(
	gnsdk_submit_parcel_handle_t	parcel_handle,
	gnsdk_uint32_t					flags,
	gnsdk_submit_state_t*			p_state
	);

/** gnsdk_submit_parcel_data_get_state
  * Summary:
  *   Retrieves the state of an individual data item in a parcel being
  *   uploaded.
  * Parameters:
  *   parcel_handle:  [in] Handle to a parcel
  *   id:             [in] Identifier for either a data_ident or data_id string
  *   p_state:        [out] Pointer to a parcel state
  *   p_error:        [out] Pointer to receive an error
  *   p_info:         [out] Pointer to receive an information string; note this
  *                   string is tied to the parcel and is not guaranteed to
  *                   exist after the parcel is freed
  *                                                                            
*/
gnsdk_error_t GNSDK_API
gnsdk_submit_parcel_data_get_state(
	gnsdk_submit_parcel_handle_t	parcel_handle,
	gnsdk_cstr_t					id,			/* data_ident or stream_id */
	gnsdk_submit_state_t*			p_state,
	gnsdk_error_t*					p_error,	/* optional */
	gnsdk_cstr_t*					p_info		/* optional - note this string is tied to the parcel and is not guaranteed to exist once the parcel has been freed */
	);



#ifdef __cplusplus
}
#endif

#endif /* _GNSDK_SUBMIT_H_ */

