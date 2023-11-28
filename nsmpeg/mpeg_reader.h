#pragma once
#include <stdio.h>
#include <bfc/platform/types.h>
#include "../f263/BitReader.h"

namespace nsmpeg
{
	static const int packet_size = 256000; 
	enum
	{
		READ_OK = 0,
		READ_EOF = 1,
	};
	class mpeg_reader : private BitReader
{
public:
	mpeg_reader();
	int Open(const wchar_t *filename);
	void Close();

	int hasbits(size_t bits);
	using BitReader::getbits;
	using BitReader::showbits;
	using BitReader::flushbits;
	using BitReader::size;
	using BitReader::getbytes;
	using BitReader::data;
private:
	uint8_t packet[packet_size];
	size_t packet_len; // amount of valid data in the packet buffer
	FILE *f;
};
}