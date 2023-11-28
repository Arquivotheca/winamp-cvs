#include <stdio.h>
#include "../f263/bitreader.h"
#include <bfc/platform/types.h>
#include "../mpeg2dec/mpegvid_api.h"
#include "mpeg_reader.h"
#include "types.h"
#include "mpegsync.h"
#include "program_stream.h"
#include <assert.h>

mpegvideo_decoder_t decoder;
mpeg_demuxer_t video_demuxer;

int ParsePackHeader(nsmpeg::mpeg_reader &reader, nsmpeg::PackHeader &header)
{
	if (reader.hasbits(32) != nsmpeg::READ_OK)
		return 1;

	// 1) you lock the target
	if (reader.getbits(32) == 0x1BA)
	{
		if (reader.hasbits(64) != nsmpeg::READ_OK)
			return 1;

		memset(&header, 0, sizeof(nsmpeg::PackHeader));

		/* byte 4 */
		header.version = reader.getbits(2);
		if (header.version == 0)
		{ // MPEG1
			if (reader.getbits(2) != 0x2)
			{
				return 1;
			}

			header.system_clock = ((uint64_t)reader.getbits(3)) << 30;
			if (reader.getbits(1) != 1) // sanity check marker bit 
			{
				return 1;
			}

			header.system_clock |= reader.getbits(15) << 15;
			if (reader.getbits(1) != 1) // sanity check marker bit 
			{
				return 1;
			}

			header.system_clock |= reader.getbits(15);
			if (reader.getbits(2) != 3) // sanity check marker bit 
			{
				return 1;
			}

			header.bitrate = reader.getbits(22);
			if (reader.getbits(1) != 1) // sanity check marker bit 
			{
				return 1;
			}
		}
		else
		{ // MPEG-2
			if (reader.hasbits(78) != nsmpeg::READ_OK)
				return 1;

			header.system_clock = ((uint64_t)reader.getbits(3)) << 30;
			if (reader.getbits(1) != 1) // sanity check marker bit 
			{
				return 1;
			}

			header.system_clock |= reader.getbits(15) << 15;
			if (reader.getbits(1) != 1) // sanity check marker bit 
			{
				return 1;
			}

			header.system_clock |= reader.getbits(15);
			if (reader.getbits(1) != 1) // sanity check marker bit 
			{
				return 1;
			}

			header.system_clock_remainder = reader.getbits(9);
			if (reader.getbits(1) != 1) // sanity check marker bit 
			{
				return 1;
			}

			header.bitrate = reader.getbits(22);
			if (reader.getbits(2) != 3) // sanity check marker bit 
			{
				return 1;
			}

			reader.flushbits(5); // reserved
			uint8_t stuffing_bytes = reader.getbits(3); // turkey time?

			if (reader.hasbits(stuffing_bytes*8) != nsmpeg::READ_OK)
				return 1;

			reader.flushbits(stuffing_bytes * 8); 
		}
		return 0;
	}
	else
		return 1;

}

int ParseSystemHeader(nsmpeg::mpeg_reader &reader)
{
	if (reader.hasbits(32) != nsmpeg::READ_OK)
		return 1;

	if (reader.getbits(32) == 0x1BB)
	{
		if (reader.hasbits(16) != nsmpeg::READ_OK)
			return 1;

		uint16_t header_size = reader.getbits(16);
		if (reader.getbits(1) != 1) // sanity check marker bit 
		{
			return 1;
		}

		nsmpeg::MPEG1::SystemHeader header;
		header.rate_bound = reader.getbits(22);
		if (reader.getbits(1) != 1) // sanity check marker bit 
		{
			return 1;
		}

		header.audio_bound = reader.getbits(6);
		header.fixed_flag = reader.getbits(1);
		header.csps_flag = reader.getbits(1);
		header.system_audio_lock_flag = reader.getbits(1);
		header.system_video_lock_flag = reader.getbits(1);

		if (reader.getbits(1) != 1) // sanity check marker bit 
		{
			return 1;
		}

		header.video_bound = reader.getbits(5);
		header.packet_rate_restriction_flag = reader.getbits(1);
		reader.flushbits(7); // reserved
		header.extension_count = (header_size-6)/3;
		for (int i=0;i<header.extension_count;i++)
		{
			if (reader.hasbits(24) != nsmpeg::READ_OK)
				return 1;

			if (reader.showbits(1) == 1)
			{
				//reader.flushbits(1);
				header.extension[i].stream_id = reader.getbits(8);

				if (reader.getbits(2) != 3) // sanity check marker bit 
				{
					return 1;
				}

				header.extension[i].p_std_buffer_bound_scale = reader.getbits(1);
				header.extension[i].p_std_buffer_size_bound = reader.getbits(13);
			}
		}
		return 0;
	}
	else
		return 1;
}

int ParsePadding(nsmpeg::mpeg_reader &reader)
{
	if (reader.hasbits(48) != nsmpeg::READ_OK)
		return 1;

	reader.flushbits(24);

	uint8_t stream_id = reader.getbits(8);

	uint16_t packet_length = reader.getbits(16);

	if (reader.hasbits(packet_length * 8) != nsmpeg::READ_OK)
		return 1;

	reader.flushbits(packet_length * 8);
	return 0;
}


int ParsePES(nsmpeg::mpeg_reader &reader)
{
	if (reader.hasbits(48) != nsmpeg::READ_OK)
		return 1;

	reader.flushbits(24);

	uint8_t stream_id = reader.getbits(8);

	uint16_t packet_length = reader.getbits(16);
	uint32_t packet_bits = (uint32_t)packet_length*8;

	if (reader.hasbits(packet_bits) != nsmpeg::READ_OK)
		return 1;
	nsmpeg::MPEG1::PESHeader header;
	memset(&header, 0, sizeof(nsmpeg::MPEG1::PESHeader));

	while (reader.showbits(1) == 1)
	{
		reader.flushbits(8);
		packet_bits-=8;
	}

	if (reader.showbits(2) == 1)
	{
		reader.flushbits(2);
		header.std_buffer_scale = reader.getbits(1);
		header.std_buffer_size = reader.getbits(13);
		packet_bits-=16;
	}

	if (reader.showbits(4) == 2)
	{
		if (reader.getbits(4) != 0x2)
			return 1;

		header.has_pts = 1;
		header.pts = reader.getbits(3) << 30;
		if (reader.getbits(1) != 1) 
			return 1;

		header.pts |= reader.getbits(15) << 15;
		if (reader.getbits(1) != 1)
			return 1;

		header.pts |= reader.getbits(15);
		if (reader.getbits(1) != 1) 
			return 1;

		packet_bits -= 40;

	}
	else if (reader.showbits(4) == 3)
	{
		if (reader.getbits(4) != 0x3)
			return 1;

		header.has_pts = 1;
		header.pts = reader.getbits(3) << 30;
		if (reader.getbits(1) != 1)
			return 1;

		header.pts |= reader.getbits(15) << 15;
		if (reader.getbits(1) != 1)
			return 1;

		header.pts |= reader.getbits(15);
		if (reader.getbits(1) != 1) 
			return 1;

		if (reader.getbits(4) != 0x1) 
			return 1;

		header.has_dts = 1;
		header.dts = reader.getbits(3) << 30;
		if (reader.getbits(1) != 1)
			return 1;

		header.dts |= reader.getbits(15) << 15;
		if (reader.getbits(1) != 1) 
			return 1;

		header.dts |= reader.getbits(15);
		if (reader.getbits(1) != 1) 
			return 1;

		packet_bits -= 80;
	}
	else
	{
		reader.flushbits(8);
		packet_bits -= 8;
	}

	uint8_t packet[65536];
	packet_length = packet_bits/8;
	reader.getbytes(packet, packet_length);
	const void *data = packet;
	size_t datalen = packet_length;
	while (datalen)
	{
		const void *unit;
		size_t unit_len;
		MPEG_AddData(video_demuxer, &data, &datalen);
		if (MPEG_GetUnit(video_demuxer, &unit, &unit_len) == MPEG_UnitAvailable)
		{
			MPEGVideo_DecodeFrame(decoder, (void *)unit, unit_len);
		}
	}


	return 0;
}


int ParsePES_MPEG2(nsmpeg::mpeg_reader &reader)
{
	static int xxx;
	xxx++;
	if (reader.hasbits(48) != nsmpeg::READ_OK)
		return 1;

	reader.flushbits(24);

	uint8_t stream_id = reader.getbits(8);

	uint16_t packet_length = reader.getbits(16);
	uint32_t packet_bits = packet_length*8;

	if (reader.hasbits(packet_bits) != nsmpeg::READ_OK)
		return 1;

	if (reader.getbits(2) != 0x2) return 1;

	uint8_t scrambling_code = reader.getbits(2);
	uint8_t priority = reader.getbits(1);
	uint8_t data_alignment_indicator = reader.getbits(1);
	uint8_t copyright = reader.getbits(1);
	uint8_t original_or_copy = reader.getbits(1);
	uint8_t pts_dts_flags = reader.getbits(2);
	uint8_t escr_flag = reader.getbits(1);
	uint8_t es_rate_flag = reader.getbits(1);
	uint8_t dsm_trick_mode_flag = reader.getbits(1);
	uint8_t additional_copy_info_flag = reader.getbits(1);
	uint8_t crc_flag = reader.getbits(1);
	uint8_t extension_flag = reader.getbits(1);
	uint8_t header_data_length= reader.getbits(8);
	packet_bits -= 24;
	if (pts_dts_flags == 0x2)
	{
		if (reader.getbits(4) != 0x2) return 1;

		uint64_t pts = reader.getbits(3) << 30;
		if (reader.getbits(1) != 1) return 1;

		pts |= reader.getbits(15) << 15;
		if (reader.getbits(1) != 1) return 1;

		pts |= reader.getbits(15);
		if (reader.getbits(1) != 1) return 1;

		packet_bits -= 40;
	}
	else if (pts_dts_flags == 0x3)
	{
		if (reader.getbits(4) != 0x3) return 1;

		uint64_t pts = reader.getbits(3) << 30;
		if (reader.getbits(1) != 1) return 1;

		pts |= reader.getbits(15) << 15;
		if (reader.getbits(1) != 1) return 1;

		pts |= reader.getbits(15);
		if (reader.getbits(1) != 1) return 1;

		if (reader.getbits(4) != 0x1) return 1;

		uint64_t dts = reader.getbits(3) << 30;
		if (reader.getbits(1) != 1) return 1;

		dts |= reader.getbits(15) << 15;
		if (reader.getbits(1) != 1) return 1;

		dts |= reader.getbits(15);
		if (reader.getbits(1) != 1) return 1;

		packet_bits -= 80;
	}

	if (escr_flag == 1)
	{
		reader.flushbits(2);
		uint64_t escr_base = reader.getbits(3) << 30;
		if (reader.getbits(1) != 1) return 1;

		escr_base |= reader.getbits(15) << 15;
		if (reader.getbits(1) != 1) return 1;

		escr_base |= reader.getbits(15);
		if (reader.getbits(1) != 1) return 1;

		uint16_t escr_extension = reader.getbits(9);
		if (reader.getbits(1) != 1) return 1;
		packet_bits -= 48;
	}

	if (es_rate_flag == 1)
	{
		if (reader.getbits(1) != 1) return 1;
		uint32_t es_rate = reader.getbits(22);
		if (reader.getbits(1) != 1) return 1;
		packet_bits -= 24;
	}

	if (dsm_trick_mode_flag == 1)
	{
		uint8_t trick_mode_control = reader.getbits(3);
		/* trick modes
		0 - fast foward
		1 - slow motion
		2 - freeze frame
		3 - fast reverse
		4 - slow reverse
		5-7 - reserved
		*/

		if (trick_mode_control == 0 /*fast foward*/)
		{
			uint8_t field_id = reader.getbits(2);
			uint8_t intra_slice_refresh = reader.getbits(1);
			uint8_t frequency_truncation = reader.getbits(2);
		}
		else if (trick_mode_control == 1 /*slow motion*/)
		{
			uint8_t rep_cntrl = reader.getbits(5);
		}
		else if (trick_mode_control == 2 /*freeze frame*/)
		{
			uint8_t field_id = reader.getbits(2);
			reader.flushbits(3);
		}
		else if (trick_mode_control == 3 /*fast reverse*/)
		{
			uint8_t field_id = reader.getbits(2);
			uint8_t intra_slice_refresh = reader.getbits(1);
			uint8_t frequency_truncation = reader.getbits(2);
		}
		else if (trick_mode_control == 4 /*slow reverse*/)
		{
			uint8_t rep_cntrl = reader.getbits(5);
		}
		else
		{
			reader.flushbits(5);
		}
		packet_bits -= 8;
	}

	if (additional_copy_info_flag == 1)
	{
		if (reader.getbits(1) != 1) return 1;

		uint8_t additional_copy_info = reader.getbits(7);
		packet_bits -= 8;
	}

	if (crc_flag == 1)
	{
		uint16_t previous_packet_crc = reader.getbits(16);
		packet_bits -= 16;
	}

	if (extension_flag == 1)
	{
		uint8_t private_data_flag = reader.getbits(1);
		uint8_t pack_header_field_flag = reader.getbits(1);
		uint8_t program_packet_sequence_counter_flag = reader.getbits(1);
		uint8_t p_std_buffer_flag = reader.getbits(1);
		reader.flushbits(3);
		uint8_t extension_flag_2 = reader.getbits(1);
		packet_bits -= 8;
		if (private_data_flag == 1)
		{
			reader.flushbits(128);
			packet_bits -= 128;
		}

		if (pack_header_field_flag == 1)
		{
			uint8_t pack_field_length = reader.getbits(8);
			nsmpeg::PackHeader header;
			ParsePackHeader(reader, header);
			packet_bits -= 8 + 8*pack_field_length;
		}

		if (program_packet_sequence_counter_flag == 1)
		{
			if (reader.getbits(1) != 1) return 1;
			uint8_t program_packet_sequence_counter = reader.getbits(7);
			if (reader.getbits(1) != 1) return 1;
			uint8_t mpeg1_mpeg2_identifier = reader.getbits(1);
			uint8_t original_stuffing_length = reader.getbits(6);
			packet_bits -= 16;
		}

		if (p_std_buffer_flag == 1)
		{
			if (reader.getbits(2) != 1) return 1;
			uint8_t p_std_buffer_scale = reader.getbits(1);
			uint16_t p_std_buffer_size = reader.getbits(13);
			packet_bits -= 16;
		}

		if (extension_flag_2 == 1)
		{
			if (reader.getbits(2) != 1) return 1;
			uint8_t extension_field_length = reader.getbits(7);
			reader.flushbits(8*extension_field_length);
			packet_bits -= 8 + 8*extension_field_length;
		}
	}

	// stuffing bytes.  is this from original_stuffing_length?
	while (reader.showbits(8) == 0xFF)
	{
		reader.flushbits(8);
		packet_bits -= 8;
	}

	uint8_t packet[65536];
	packet_length = packet_bits/8;
	reader.getbytes(packet, packet_length);

	const void *data = packet;
	size_t datalen = packet_length;
	while (datalen)
	{
		const void *unit;
		size_t unit_len;
		MPEG_AddData(video_demuxer, &data, &datalen);
		if (MPEG_GetUnit(video_demuxer, &unit, &unit_len) == MPEG_UnitAvailable)
		{
			MPEGVideo_DecodeFrame(decoder, (void *)unit, unit_len);
		}
	}
	return 0;
}


nsmpeg::ProgramStream::ProgramStream()
{
	reader = 0;
}

int ps_main()
{
	nsmpeg::ProgramStream program_stream;
	nsmpeg::PackHeader pack_header;

	video_demuxer = MPEG_Create(10000000);
	decoder = MPEGVideo_CreateDecoder();
	nsmpeg::mpeg_reader reader;
	reader.Open(L"c:/users/benski/videos/Crystal Castles - Crimewave.mpg");
	program_stream.reader = &reader;

	while(reader.hasbits(32) == nsmpeg::READ_OK)
	{
		uint32_t start_code = reader.showbits(32);
		if ((start_code & 0xFFFFFF00) != 0x00000100)
		{
			reader.flushbits(8);
			continue;
		}
		if (start_code == 0x1BA)
		{
			ParsePackHeader(reader, pack_header);
		}
		else if (start_code == 0x1BB)
		{
			ParseSystemHeader(reader);
		}
		else if (start_code == 0x1BE)
		{
			ParsePadding(reader);
		}
		else if (start_code == 0x1e0)//((start_code & 0xF0) == 0xE0)
		{
			if (pack_header.version == 0)
			{
				ParsePES(reader);
			}
			else
			{
				if (ParsePES_MPEG2(reader) != 0)
				{
					int x;
					x=0;
				}
			}
		}
		else if (start_code == 0x1C0)
		{
			ParsePadding(reader);
		}
		else if (start_code == 0x1B9)
		{
			reader.flushbits(32); // TODO
		}
		else if (start_code == 0x1BD)
		{ // private stream 1
			ParsePadding(reader);
		}
		else if (start_code == 0x1BF)
		{ // private stream 2
			ParsePadding(reader);
		}
		else
		{
			reader.flushbits(8);
		}
	}

	reader.Close();
	return 0;
}

int main()
{
		decoder = MPEGVideo_CreateDecoder();
	nsmpeg::mpeg_reader reader;
	reader.Open(L"c:/users/benski/videos/Royksopp - Happy Up Here 1080p h264 AC-3 5.1.m2ts");

	for(;;)
	{
		BitReader packet_reader;
		reader.hasbits(192*8);
		packet_reader.data = reader.data;
		packet_reader.numBits = 192*8;

		uint32_t timestamp = packet_reader.getbits(32);
		uint8_t sync_byte = packet_reader.getbits(8);
		assert(sync_byte == 0x47);
		uint8_t error_indicator = packet_reader.getbits(1);
		uint8_t payload_unit_start = packet_reader.getbits(1);
		uint8_t transport_priority = packet_reader.getbits(1);
		uint16_t pid = packet_reader.getbits(13);
		uint8_t scrambling = packet_reader.getbits(2);
		uint8_t adaption_field_flag = packet_reader.getbits(1);
		uint8_t payload_data_flag = packet_reader.getbits(1);
		uint8_t continuity_counter = packet_reader.getbits(4);
		if (adaption_field_flag)
		{
			uint8_t field_length = packet_reader.getbits(8);
			if (field_length)
			{
				uint8_t discontinuity_indicator = packet_reader.getbits(1);
				uint8_t random_access_indicator = packet_reader.getbits(1);
				uint8_t elementary_stream_priority_indicator = packet_reader.getbits(1);
				uint8_t pcr_flag = packet_reader.getbits(1);
				uint8_t opcr_flag = packet_reader.getbits(1);
				uint8_t splicing_point_flag = packet_reader.getbits(1);
				uint8_t transport_private_data_flag = packet_reader.getbits(1);
				uint8_t extension_flag = packet_reader.getbits(1);
				field_length -= 1;
				if (pcr_flag)
				{
					packet_reader.flushbits(48); // TODO
					field_length -= 6;
				}

				if (opcr_flag)
				{
					packet_reader.flushbits(48); // TODO
					field_length -= 6;
				}

				if (splicing_point_flag)
				{
					uint8_t splice_countdown = packet_reader.getbits(8);
					field_length -= 8;
				}
				if (transport_private_data_flag)
				{
					uint8_t private_data_length = packet_reader.getbits(8);
					field_length -= (1 + private_data_length);
					packet_reader.flushbits(private_data_length*8);
				}
				if (extension_flag)
				{
					uint8_t extension_field_length = packet_reader.getbits(8);
					field_length -= (1 + extension_field_length);
					uint8_t ltw_flag = packet_reader.getbits(1);
					uint8_t piecewise_rate_flag = packet_reader.getbits(1);
					uint8_t seamless_splice_flag = packet_reader.getbits(1);
					packet_reader.flushbits(5);
					extension_field_length -= 1;
					if (ltw_flag)
					{
						uint8_t ltw_valid_flag = packet_reader.getbits(1);
						uint16_t ltw_offset = packet_reader.getbits(15);
						extension_field_length -= 2;
					}

					if (piecewise_rate_flag)
					{
						packet_reader.flushbits(2);
						uint32_t piecewise_rate = packet_reader.getbits(22);
						extension_field_length -= 3;
					}

					if (seamless_splice_flag)
					{
						uint8_t splice_type = packet_reader.getbits(4);
						packet_reader.flushbits(36); // TODO
						extension_field_length -= 5;
					}

					packet_reader.flushbits(extension_field_length*8);
				}
				packet_reader.flushbits(field_length*8);
			}


		}

		if (payload_data_flag)
		{
			uint8_t payload_length = packet_reader.numBits / 8;
			uint8_t packet[192];
			packet_reader.getbytes(packet, payload_length);
			int x;
			x=0;
		}
		reader.flushbits(192*8);
	}
	reader.Close();
	return 0;
}