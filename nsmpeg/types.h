#pragma once
#include <bfc/platform/types.h>

namespace nsmpeg
{
	namespace MPEG1
	{
		struct PESHeader
		{
			uint8_t std_buffer_scale;
			uint16_t std_buffer_size;
			int has_pts;
			uint64_t pts;
			int has_dts;
			uint64_t dts;
		};

		struct SystemHeader
		{
			uint32_t rate_bound;
			uint8_t audio_bound;
			uint8_t fixed_flag;
			uint8_t csps_flag;
			uint8_t system_audio_lock_flag;
			uint8_t system_video_lock_flag;
			uint8_t video_bound;
			uint8_t packet_rate_restriction_flag;
			// TODO: better name than 'extension'
			size_t extension_count;
			struct
			{
				uint8_t stream_id;
				uint8_t p_std_buffer_bound_scale;
				uint16_t p_std_buffer_size_bound;
			} extension[256];
		};
	}

		struct PackHeader
		{
			uint8_t version; // 0 for MPEG1, 1 for MPEG2
			uint64_t system_clock;
			uint32_t bitrate;
			uint16_t system_clock_remainder;
		};


	
}