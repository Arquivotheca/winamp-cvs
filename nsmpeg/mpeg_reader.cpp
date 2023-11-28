#include "mpeg_reader.h"

nsmpeg::mpeg_reader::mpeg_reader()
{
	f = 0;
	packet_len=0;
	BitReader::data=packet;
	BitReader::numBits = 0;
}

int nsmpeg::mpeg_reader::Open(const wchar_t *filename)
{
	f=  _wfopen(filename, L"rb");
	return READ_OK;
}

void nsmpeg::mpeg_reader::Close()
{
	fclose(f);
}

int nsmpeg::mpeg_reader::hasbits(size_t bits)
{
	if (BitReader::numBits < bits)
	{
		int difference = (bits + 7 - BitReader::numBits) / 8;
		difference = (difference + 8191) & ~8191; // read a sensible amount of data at a time

		// if we don't have enough room to read into the packet, do a memmove
		if (packet_size - packet_len < difference)
		{
			int bytes_left = (BitReader::numBits + 7) / 8; // number of bytes left in the bitreader
			int byte_position = packet_len - bytes_left; // position (in bytes) into the packet that the bitreader has

			memmove(packet, data, bytes_left);
			packet_len = bytes_left;
			BitReader::data = packet;
		}

		size_t bytes_read = fread(packet+packet_len, 1, difference, f);
		BitReader::numBits += bytes_read*8;
		packet_len += bytes_read;
		if (bytes_read * 8 < bits)
			return nsmpeg::READ_EOF;

	}
	return nsmpeg::READ_OK;
}

