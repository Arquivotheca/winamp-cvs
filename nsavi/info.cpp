#include "info.h"
#include "read.h"

nsavi::Info::Info()
{

}

nsavi::Info::~Info()
{
	for (InfoMap::iterator itr = InfoMap::begin();itr!=InfoMap::end();itr++)
	{
		free(itr->second);
	}
}

int nsavi::Info::Read(nsavi::avi_reader *reader, uint32_t data_len)
{
	while (data_len)
	{
		riff_chunk chunk;
		uint32_t bytes_read=0;
		nsavi::read_riff_chunk(reader, &chunk, &bytes_read);
		data_len -= bytes_read;
		size_t malloc_size = chunk.size + 1;
		if (malloc_size == 0)
			return READ_INVALID_DATA;

		char *str = (char *)malloc(malloc_size);
		if (!str)
			return READ_OUT_OF_MEMORY;

		reader->Read(str, chunk.size, &bytes_read);
		str[chunk.size] = 0;
		data_len -= bytes_read;
		set(chunk.id, str);
		if (chunk.size & 1)
		{
			reader->Skip(1);
			data_len--;
		}
	}
	return 0;
}

const char *nsavi::Info::GetMetadata(uint32_t id)
{
	InfoMap::iterator itr = InfoMap::find(id);
	if (itr != InfoMap::end())
	{
		return itr->second;
	}
	return 0;
}