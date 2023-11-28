#pragma once
#include "foundation/types.h"

typedef enum {
	FDK_AAC_DEC_OK                             = 0x0000,  /*!< No error occured. Output buffer is valid and error free. */
	FDK_AAC_DEC_OUT_OF_MEMORY                  = 0x0002,  /*!< Heap returned NULL pointer. Output buffer is invalid. */
	FDK_AAC_DEC_UNKNOWN                        = 0x0005,  /*!< Error condition is of unknown reason, or from a another module. Output buffer is invalid. */

	/* Synchronization errors. Output buffer is invalid. */
	fdk_aac_dec_sync_error_start               = 0x1000,
	FDK_AAC_DEC_TRANSPORT_SYNC_ERROR           = 0x1001,  /*!< The transport decoder had syncronisation problems. Do not exit decoding. Just feed new bitstream data. */
	FDK_AAC_DEC_NOT_ENOUGH_BITS                = 0x1002,  /*!< The input buffer ran out of bits. */
	fdk_aac_dec_sync_error_end                 = 0x1FFF,

	/* Initialization errors. Output buffer is invalid. */
	fdk_aac_dec_init_error_start               = 0x2000,
	FDK_AAC_DEC_INVALID_HANDLE                 = 0x2001,  /*!< The handle passed to the function call was invalid (NULL). */
	FDK_AAC_DEC_UNSUPPORTED_AOT                = 0x2002,  /*!< The AOT found in the configuration is not supported. */
	FDK_AAC_DEC_UNSUPPORTED_FORMAT             = 0x2003,  /*!< The bitstream format is not supported.  */
	FDK_AAC_DEC_UNSUPPORTED_ER_FORMAT          = 0x2004,  /*!< The error resilience tool format is not supported. */
	FDK_AAC_DEC_UNSUPPORTED_EPCONFIG           = 0x2005,  /*!< The error protection format is not supported. */
	FDK_AAC_DEC_UNSUPPORTED_MULTILAYER         = 0x2006,  /*!< More than one layer for AAC scalable is not supported. */
	FDK_AAC_DEC_UNSUPPORTED_CHANNELCONFIG      = 0x2007,  /*!< The channel configuration (either number or arrangement) is not supported. */
	FDK_AAC_DEC_UNSUPPORTED_SAMPLINGRATE       = 0x2008,  /*!< The sample rate specified in the configuration is not supported. */
	FDK_AAC_DEC_INVALID_SBR_CONFIG             = 0x2009,  /*!< The SBR configuration is not supported. */
	FDK_AAC_DEC_SET_PARAM_FAIL                 = 0x200A,  /*!< The parameter could not be set. Either the value was out of range or the parameter does not exist. */
	FDK_AAC_DEC_NEED_TO_RESTART                = 0x200B,  /*!< The decoder needs to be restarted, since the requiered configuration change cannot be performed. */
	fdk_aac_dec_init_error_end                 = 0x2FFF,

	/* Decode errors. Output buffer is valid but concealed. */
	fdk_aac_dec_decode_error_start             = 0x4000,
	FDK_AAC_DEC_TRANSPORT_ERROR                = 0x4001,  /*!< The transport decoder encountered an unexpected error. */
	FDK_AAC_DEC_PARSE_ERROR                    = 0x4002,  /*!< Error while parsing the bitstream. Most probably it is corrupted, or the system crashed. */
	FDK_AAC_DEC_UNSUPPORTED_EXTENSION_PAYLOAD  = 0x4003,  /*!< Error while parsing the extension payload of the bitstream. The extension payload type found is not supported. */
	FDK_AAC_DEC_DECODE_FRAME_ERROR             = 0x4004,  /*!< The parsed bitstream value is out of range. Most probably the bitstream is corrupt, or the system crashed. */
	FDK_AAC_DEC_CRC_ERROR                      = 0x4005,  /*!< The embedded CRC did not match. */
	FDK_AAC_DEC_INVALID_CODE_BOOK              = 0x4006,  /*!< An invalid codebook was signalled. Most probably the bitstream is corrupt, or the system crashed. */
	FDK_AAC_DEC_UNSUPPORTED_PREDICTION         = 0x4007,  /*!< Predictor found, but not supported in the AAC Low Complexity profile. Most probably the bitstream is corrupt, or has a wrong format. */
	FDK_AAC_DEC_UNSUPPORTED_CCE                = 0x4008,  /*!< A CCE element was found which is not supported. Most probably the bitstream is corrupt, or has a wrong format. */
	FDK_AAC_DEC_UNSUPPORTED_LFE                = 0x4009,  /*!< A LFE element was found which is not supported. Most probably the bitstream is corrupt, or has a wrong format. */
	FDK_AAC_DEC_UNSUPPORTED_GAIN_CONTROL_DATA  = 0x400A,  /*!< Gain control data found but not supported. Most probably the bitstream is corrupt, or has a wrong format. */
	FDK_AAC_DEC_UNSUPPORTED_SBA                = 0x400B,  /*!< SBA found, but currently not supported in the BSAC profile. */
	FDK_AAC_DEC_TNS_READ_ERROR                 = 0x400C,  /*!< Error while reading TNS data. Most probably the bitstream is corrupt or the system crashed. */
	FDK_AAC_DEC_RVLC_ERROR                     = 0x400D,  /*!< Error while decoding error resillient data. */
	fdk_aac_dec_decode_error_end               = 0x4FFF,

	/* Ancillary data errors. Output buffer is valid. */
	fdk_aac_dec_anc_data_error_start           = 0x8000,
	FDK_AAC_DEC_ANC_DATA_ERROR                 = 0x8001,  /*!< Non severe error concerning the ancillary data handling. */
	FDK_AAC_DEC_TOO_SMALL_ANC_BUFFER           = 0x8002,  /*!< The registered ancillary data buffer is too small to receive the parsed data. */
	FDK_AAC_DEC_TOO_MANY_ANC_ELEMENTS          = 0x8003,  /*!< More than the allowed number of ancillary data elements should be written to buffer. */
	fdk_aac_dec_anc_data_error_end             = 0x8FFF

} FDK_AAC_DECODER_ERROR;

/** Macro to identify initialization errors. */
#define FDK_IS_INIT_ERROR(err)   ( (((err)>=fdk_aac_dec_init_error_start)   && ((err)<=fdk_aac_dec_init_error_end))   ? 1 : 0)
/** Macro to identify decode errors. */
#define FDK_IS_DECODE_ERROR(err) ( (((err)>=fdk_aac_dec_decode_error_start) && ((err)<=fdk_aac_dec_decode_error_end)) ? 1 : 0)
/** Macro to identify if the audio output buffer contains valid samples after calling aacDecoder_DecodeFrame(). */
#define FDK_IS_OUTPUT_VALID(err) ( ((err) == FDK_AAC_DEC_OK) || FDK_IS_DECODE_ERROR(err) )

typedef enum
{
	FDK_AAC_PCM_OUTPUT_INTERLEAVED              = 0x0000,  /*!< PCM output mode (1: interleaved (default); 0: not interleaved). */
	FDK_AAC_PCM_OUTPUT_CHANNELS                 = 0x0001,  /*!< Number of PCM output channels (if different from encoded audio channels, downmixing or
																												 upmixing is applied). \n
																												 -1: Disable up-/downmixing. The decoder output contains the same number of channels as the
																												 encoded bitstream. \n
																												 1: The decoder performs a mono matrix mix-down if the encoded audio channels are greater
																												 than one. Thus it ouputs always exact one channel. \n
																												 2: The decoder performs a stereo matrix mix-down if the encoded audio channels are greater
																												 than two. If the encoded audio channels are smaller than two the decoder duplicates the
																												 output. Thus it ouputs always exact two channels. \n 
																												 */

																												 FDK_AAC_PCM_DUAL_CHANNEL_OUTPUT_MODE        = 0x0002,  /*!< Defines how the decoder processes two channel signals:
																																																								0: Leave both signals as they are (default).
																																																								1: Create a dual mono output signal from channel 1.
																																																								2: Create a dual mono output signal from channel 2.
																																																								3: Create a dual mono output signal by mixing both channels (L' = R' = 0.5*Ch1 + 0.5*Ch2). */
																																																								FDK_AAC_PCM_OUTPUT_CHANNEL_MAPPING          = 0x0003,  /*!< Output buffer channel ordering. 0: MPEG PCE style order, 1: WAV file channel order (default). */

																																																								FDK_AAC_CONCEAL_METHOD                      = 0x0100,  /*!< Error concealment: Processing method. \n
																																																																																			 0: Spectral muting. \n
																																																																																			 1: Noise substitution (see ::CONCEAL_NOISE). \n
																																																																																			 2: Energy interpolation (adds additional signal delay of one frame, see ::CONCEAL_INTER). \n */

																																																																																			 FDK_AAC_DRC_BOOST_FACTOR                    = 0x0200,  /*!< Dynamic Range Control: Scaling factor for boosting gain values.
																																																																																																															Defines how the boosting DRC factors (conveyed in the bitstream) will be applied to the
																																																																																																															decoded signal. The valid values range from 0 (don't apply boost factors) to 127 (fully
																																																																																																															apply all boosting factors). */
																																																																																																															FDK_AAC_DRC_ATTENUATION_FACTOR              = 0x0201,  /*!< Dynamic Range Control: Scaling factor for attenuating gain values. Same as
																																																																																																																																										 AAC_DRC_BOOST_FACTOR but for attenuating DRC factors. */
																																																																																																																																										 FDK_AAC_DRC_REFERENCE_LEVEL                 = 0x0202,  /*!< Dynamic Range Control: Target reference level. Defines the level below full-scale
																																																																																																																																																																						(quantized in steps of 0.25dB) to which the output audio signal will be normalized to by
																																																																																																																																																																						the DRC module. The valid values range from 0 (full-scale) to 127 (31.75 dB below
																																																																																																																																																																						full-scale). The value smaller than 0 switches off normalization. */
																																																																																																																																																																						FDK_AAC_DRC_HEAVY_COMPRESSION               = 0x0203,  /*!< Dynamic Range Control: En-/Disable DVB specific heavy compression (aka RF mode).
																																																																																																																																																																																																	 If set to 1, the decoder will apply the compression values from the DVB specific ancillary
																																																																																																																																																																																																	 data field. At the same time the MPEG-4 Dynamic Range Control tool will be disabled. By
																																																																																																																																																																																																	 default heavy compression is disabled. */

																																																																																																																																																																																																	 FDK_AAC_QMF_LOWPOWER                        = 0x0300,  /*!< Quadrature Mirror Filter (QMF) Bank processing mode. \n
																																																																																																																																																																																																																													-1: Use internal default. Implies MPEG Surround partially complex accordingly. \n
																																																																																																																																																																																																																													0: Use complex QMF data mode. \n
																																																																																																																																																																																																																													1: Use real (low power) QMF data mode. \n */

																																																																																																																																																																																																																													FDK_AAC_MPEGS_ENABLE                        = 0x0500,  /*!< MPEG Surround: Allow/Disable decoding of MPS content. Available only for decoders with MPEG
																																																																																																																																																																																																																																																								 Surround support. */

																																																																																																																																																																																																																																																								 FDK_AAC_TPDEC_CLEAR_BUFFER                  = 0x0603   /*!< Clear internal bit stream buffer of transport layers. The decoder will start decoding
																																																																																																																																																																																																																																																																																				at new data passed after this event and any previous data is discarded. */

} FDK_AACDEC_PARAM;

/** Speaker description tags */
typedef enum {
	FDK_ACT_NONE,
	FDK_ACT_FRONT,
	FDK_ACT_SIDE,
	FDK_ACT_BACK,
	FDK_ACT_LFE,
	FDK_ACT_FRONT_TOP,
	FDK_ACT_SIDE_TOP,
	FDK_ACT_BACK_TOP,
	FDK_ACT_TOP /* Ts */
} FDK_AUDIO_CHANNEL_TYPE;

typedef enum
{
	FDK_AOT_NONE             = -1,
	FDK_AOT_NULL_OBJECT      = 0,
	FDK_AOT_AAC_MAIN         = 1, /**< Main profile                              */
	FDK_AOT_AAC_LC           = 2, /**< Low Complexity object                     */
	FDK_AOT_AAC_SSR          = 3,
	FDK_AOT_AAC_LTP          = 4,
	FDK_AOT_SBR              = 5,
	FDK_AOT_AAC_SCAL         = 6,
	FDK_AOT_TWIN_VQ          = 7,
	FDK_AOT_CELP             = 8,
	FDK_AOT_HVXC             = 9,
	FDK_AOT_RSVD_10          = 10, /**< (reserved)                                */
	FDK_AOT_RSVD_11          = 11, /**< (reserved)                                */
	FDK_AOT_TTSI             = 12, /**< TTSI Object                               */
	FDK_AOT_MAIN_SYNTH       = 13, /**< Main Synthetic object                     */
	FDK_AOT_WAV_TAB_SYNTH    = 14, /**< Wavetable Synthesis object                */
	FDK_AOT_GEN_MIDI         = 15, /**< General MIDI object                       */
	FDK_AOT_ALG_SYNTH_AUD_FX = 16, /**< Algorithmic Synthesis and Audio FX object */
	FDK_AOT_ER_AAC_LC        = 17, /**< Error Resilient(ER) AAC Low Complexity    */
	FDK_AOT_RSVD_18          = 18, /**< (reserved)                                */
	FDK_AOT_ER_AAC_LTP       = 19, /**< Error Resilient(ER) AAC LTP object        */
	FDK_AOT_ER_AAC_SCAL      = 20, /**< Error Resilient(ER) AAC Scalable object   */
	FDK_AOT_ER_TWIN_VQ       = 21, /**< Error Resilient(ER) TwinVQ object         */
	FDK_AOT_ER_BSAC          = 22, /**< Error Resilient(ER) BSAC object           */
	FDK_AOT_ER_AAC_LD        = 23, /**< Error Resilient(ER) AAC LowDelay object   */
	FDK_AOT_ER_CELP          = 24, /**< Error Resilient(ER) CELP object           */
	FDK_AOT_ER_HVXC          = 25, /**< Error Resilient(ER) HVXC object           */
	FDK_AOT_ER_HILN          = 26, /**< Error Resilient(ER) HILN object           */
	FDK_AOT_ER_PARA          = 27, /**< Error Resilient(ER) Parametric object     */
	FDK_AOT_RSVD_28          = 28, /**< might become SSC                          */
	FDK_AOT_PS               = 29, /**< PS, Parametric Stereo (includes SBR)      */
	FDK_AOT_MPEGS            = 30, /**< MPEG Surround                             */

	FDK_AOT_ESCAPE           = 31, /**< Signal AOT uses more than 5 bits          */

	FDK_AOT_MP3ONMP4_L1      = 32, /**< MPEG-Layer1 in mp4                        */
	FDK_AOT_MP3ONMP4_L2      = 33, /**< MPEG-Layer2 in mp4                        */
	FDK_AOT_MP3ONMP4_L3      = 34, /**< MPEG-Layer3 in mp4                        */
	FDK_AOT_RSVD_35          = 35, /**< might become DST                          */
	FDK_AOT_RSVD_36          = 36, /**< might become ALS                          */
	FDK_AOT_AAC_SLS          = 37, /**< AAC + SLS                                 */
	FDK_AOT_SLS              = 38, /**< SLS                                       */
	FDK_AOT_ER_AAC_ELD       = 39, /**< AAC Enhanced Low Delay                    */

	FDK_AOT_USAC             = 42, /**< USAC                                      */
	FDK_AOT_SAOC             = 43, /**< SAOC                                      */
	FDK_AOT_LD_MPEGS         = 44, /**< Low Delay MPEG Surround                   */

	FDK_AOT_RSVD50           = 50,  /**< Interim AOT for Rsvd50                   */

	/* Pseudo AOTs */
	FDK_AOT_MP2_AAC_MAIN     = 128, /**< Virtual AOT MP2 Main profile                           */
	FDK_AOT_MP2_AAC_LC       = 129, /**< Virtual AOT MP2 Low Complexity profile                 */
	FDK_AOT_MP2_AAC_SSR      = 130, /**< Virtual AOT MP2 Scalable Sampling Rate profile         */

	FDK_AOT_MP2_SBR          = 132, /**< Virtual AOT MP2 Low Complexity Profile with SBR        */

	FDK_AOT_DAB              = 134, /**< Virtual AOT for DAB (Layer2 with scalefactor CRC)      */
	FDK_AOT_DABPLUS_AAC_LC   = 135, /**< Virtual AOT for DAB plus AAC-LC                        */
	FDK_AOT_DABPLUS_SBR      = 136, /**< Virtual AOT for DAB plus HE-AAC                        */
	FDK_AOT_DABPLUS_PS       = 137, /**< Virtual AOT for DAB plus HE-AAC v2                     */

	FDK_AOT_PLAIN_MP1        = 140, /**< Virtual AOT for plain mp1                              */
	FDK_AOT_PLAIN_MP2        = 141, /**< Virtual AOT for plain mp2                              */
	FDK_AOT_PLAIN_MP3        = 142, /**< Virtual AOT for plain mp3                              */

	FDK_AOT_DRM_AAC          = 143, /**< Virtual AOT for DRM (ER-AAC-SCAL without SBR)          */
	FDK_AOT_DRM_SBR          = 144, /**< Virtual AOT for DRM (ER-AAC-SCAL with SBR)             */
	FDK_AOT_DRM_MPEG_PS      = 145, /**< Virtual AOT for DRM (ER-AAC-SCAL with SBR and MPEG-PS) */
	FDK_AOT_DRM_SURROUND     = 146, /**< Virtual AOT for DRM Surround (ER-AAC-SCAL (+SBR) +MPS) */

	FDK_AOT_MP2_PS           = 156, /**< Virtual AOT MP2 Low Complexity Profile with SBR and PS */

	FDK_AOT_MPEGS_RESIDUALS  = 256  /**< Virtual AOT for MPEG Surround residuals                */

} FDK_AUDIO_OBJECT_TYPE;

typedef struct
{
	/* These three members are the only really relevant ones for the user.                                                           */
	int32_t               sampleRate;          /*!< The samplerate in Hz of the fully decoded PCM audio signal (after SBR processing).   */
	int32_t               frameSize;           /*!< The frame size of the decoded PCM audio signal. \n
																						 1024 or 960 for AAC-LC \n
																						 2048 or 1920 for HE-AAC (v2) \n
																						 512 or 480 for AAC-LD and AAC-ELD                                                    */
	int32_t               numChannels;         /*!< The number of output audio channels in the decoded and interleaved PCM audio signal. */
	FDK_AUDIO_CHANNEL_TYPE *pChannelType;       /*!< Audio channel type of each output audio channel.           */
	uint8_t             *pChannelIndices;     /*!< Audio channel index for each output audio channel.
																						See ISO/IEC 13818-7:2005(E), 8.5.3.2 Explicit channel mapping using a program_config_element() */
	/* Decoder internal members. */
	int32_t               aacSampleRate;       /*!< sampling rate in Hz without SBR (from configuration info).                           */
	int32_t               profile;             /*!< MPEG-2 profile (from file header) (-1: not applicable (e. g. MPEG-4)).               */
	FDK_AUDIO_OBJECT_TYPE aot;                 /*!< Audio Object Type (from ASC): is set to the appropriate value for MPEG-2 bitstreams (e. g. 2 for AAC-LC). */
	int32_t               channelConfig;       /*!< Channel configuration (0: PCE defined, 1: mono, 2: stereo, ...                       */
	int32_t               bitRate;             /*!< Instantaneous bit rate.                   */
	int32_t               aacSamplesPerFrame;  /*!< Samples per frame for the AAC core (from ASC). \n
																						 1024 or 960 for AAC-LC \n
																						 512 or 480 for AAC-LD and AAC-ELD         */

	FDK_AUDIO_OBJECT_TYPE extAot;              /*!< Extension Audio Object Type (from ASC)   */
	int32_t               extSamplingRate;     /*!< Extension sampling rate in Hz (from ASC) */

	uint32_t              flags;               /*!< Copy if internal flags. Only to be written by the decoder, and only to be read externally. */

	int8_t             epConfig;            /*!< epConfig level (from ASC): only level 0 supported, -1 means no ER (e. g. AOT=2, MPEG-2 AAC, etc.)  */

	/* Statistics */
	int32_t               numLostAccessUnits;  /*!< This integer will reflect the estimated amount of lost access units in case aacDecoder_DecodeFrame()
																						 returns AAC_DEC_TRANSPORT_SYNC_ERROR. It will be < 0 if the estimation failed. */

	uint32_t              numTotalBytes;       /*!< This is the number of total bytes that have passed through the decoder. */
	uint32_t              numBadBytes;         /*!< This is the number of total bytes that were considered with errors from numTotalBytes. */
	uint32_t              numTotalAccessUnits; /*!< This is the number of total access units that have passed through the decoder. */
	uint32_t              numBadAccessUnits;   /*!< This is the number of total access units that were considered with errors from numTotalBytes. */

} FDK_CStreamInfo;

typedef enum
{
	FDK_TT_UNKNOWN           = -1, /**< Unknown format.            */
	FDK_TT_MP4_RAW           = 0,  /**< "as is" access units (packet based since there is obviously no sync layer) */
	FDK_TT_MP4_ADIF          = 1,  /**< ADIF bitstream format.     */
	FDK_TT_MP4_ADTS          = 2,  /**< ADTS bitstream format.     */

	FDK_TT_MP4_LATM_MCP1     = 6,  /**< Audio Mux Elements with muxConfigPresent = 1 */
	FDK_TT_MP4_LATM_MCP0     = 7,  /**< Audio Mux Elements with muxConfigPresent = 0, out of band StreamMuxConfig */

	FDK_TT_MP4_LOAS          = 10, /**< Audio Sync Stream.         */

	FDK_TT_DRM               = 12, /**< Digital Radio Mondial (DRM30/DRM+) bitstream format. */

	FDK_TT_MP1_L1            = 16, /**< MPEG 1 Audio Layer 1 audio bitstream. */
	FDK_TT_MP1_L2            = 17, /**< MPEG 1 Audio Layer 2 audio bitstream. */
	FDK_TT_MP1_L3            = 18, /**< MPEG 1 Audio Layer 3 audio bitstream. */

	FDK_TT_RSVD50            = 50 /**< */

} FDK_TRANSPORT_TYPE;

typedef struct FDK_AAC_DECODER_INSTANCE *HANDLE_FDKAACDECODER;

typedef FDK_AAC_DECODER_ERROR (AACDECODER_ANCDATAINIT)(HANDLE_FDKAACDECODER self, uint8_t *buffer, int size);
typedef FDK_AAC_DECODER_ERROR (*AACDECODER_ANCDATAGET) ( HANDLE_FDKAACDECODER self, int index, uint8_t **ptr, int *size);
typedef FDK_AAC_DECODER_ERROR (*AACDECODER_SETPARAM)(const HANDLE_FDKAACDECODER self, const FDK_AACDEC_PARAM param, const int32_t value);
typedef FDK_AAC_DECODER_ERROR (*AACDECODER_GETFREEBYTES)(const HANDLE_FDKAACDECODER self, uint32_t *pFreeBytes);
typedef HANDLE_FDKAACDECODER (*AACDECODER_OPEN)(FDK_TRANSPORT_TYPE transportFmt, uint32_t nrOfLayers);
typedef FDK_AAC_DECODER_ERROR (*AACDECODER_CONFIGRAW)(HANDLE_FDKAACDECODER self, uint8_t *conf[], const uint32_t length[]);
typedef FDK_AAC_DECODER_ERROR (*AACDECODER_FILL)(HANDLE_FDKAACDECODER  self, uint8_t *pBuffer[], const uint32_t bufferSize[], uint32_t *bytesValid);

#define FDK_AACDEC_CONCEAL  1 /*!< Flag for aacDecoder_DecodeFrame(): do not consider new input data. Do concealment. */
#define FDK_AACDEC_FLUSH    2 /*!< Flag for aacDecoder_DecodeFrame(): Do not consider new input data. Flush filterbanks (output delayed audio). */
#define FDK_AACDEC_INTR     4 /*!< Flag for aacDecoder_DecodeFrame(): Signal an input bit stream data discontinuity. Resync any internals as necessary. */
#define FDK_AACDEC_CLRHIST  8 /*!< Flag for aacDecoder_DecodeFrame(): Clear all signal delay lines and history buffers.
Caution: This can cause discontinuities in the output signal. */

typedef FDK_AAC_DECODER_ERROR (*AACDECODER_DECODEFRAME)(HANDLE_FDKAACDECODER  self, int16_t *pTimeData, const int32_t timeDataSize, const uint32_t flags);

typedef void (*AACDECODER_CLOSE)(HANDLE_FDKAACDECODER self);

typedef FDK_CStreamInfo *(*AACDECODER_GETSTREAMINFO)(HANDLE_FDKAACDECODER self);

//typedef INT aacDecoder_GetLibInfo( LIB_INFO *info );
