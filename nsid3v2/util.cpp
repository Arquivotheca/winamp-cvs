#include "util.h"

uint32_t ID3v2::Util::Int28To32(uint32_t val)
{
	uint32_t ret=0;
	uint8_t *bytes = (uint8_t *)&ret;
	const uint8_t *value = (const uint8_t *)&val;

	ret = (value[0] << 21) + (value[1] << 14) + (value[2] << 7) + (value[3]);
	
	//	for (size_t i=0;i<sizeof(uint32_t);i++ )
	//		value[sizeof(uint32_t)-1-i]=(uint8_t)(val>>(i*8)) & 0xFF;

	return ret;
}

size_t ID3v2::Util::UnsynchroniseTo(void *_output, const void *_input, size_t bytes)
{
	uint8_t *output = (uint8_t *)_output;
	const uint8_t *input = (const uint8_t *)_input;
	size_t bytes_read = 0;
	while (bytes)
	{
		if (input[0] == 0xFF && input[1] == 0)
		{
			*output++ = 0xFF;
			input+=2;
			bytes_read+=2;
			bytes--;
		}
		else
		{
			*output++=*input++;
			bytes_read++;
			bytes--;
		}
	}
	return bytes_read;
}

size_t ID3v2::Util::UnsynchronisedSize(const void *data, size_t bytes)
{
	const uint8_t *input = (const uint8_t *)data;
	size_t bytes_read = 0;
	while (bytes)
	{
		if (input[0] == 0xFF && input[1] == 0)
		{
			input+=2;
			bytes_read+=2;
			bytes--;
		}
		else
		{
			input++;
			bytes_read++;
			bytes--;
		}
	}
	return bytes_read;
}

// returns output bytes used
size_t ID3v2::Util::SynchroniseTo(void *_output, const void *data, size_t bytes)
{
	uint8_t *output = (uint8_t *)_output;
	const uint8_t *input = (const uint8_t *)data;
	size_t bytes_needed = 0;
	while (bytes)
	{
		*output++=*input;
		bytes_needed++;
		if (*input++ == 0xFF)
		{
			if (bytes == 1)
			{
				// if this is the last byte, we need to make room for an extra 0
				*output = 0;
				return bytes_needed + 1;
			}
			else if ((*input & 0xE0) == 0xE0 || *input == 0)
			{
				*output++ = 0;
				bytes_needed++;
			}
		}
		bytes--;
	}
	return bytes_needed;
}

size_t ID3v2::Util::SynchronisedSize(const void *data, size_t bytes)
{
	const uint8_t *input = (const uint8_t *)data;
	size_t bytes_needed = 0;
	while (bytes)
	{
		bytes_needed++;
		if (*input++ == 0xFF)
		{
			if (bytes == 1)
			{
				// if this is the last byte, we need to make room for an extra 0
				return bytes_needed + 1;
			}
			else if ((*input & 0xE0) == 0xE0 || *input == 0)
			{
				bytes_needed++;
			}
		}
		bytes--;
	}
	return bytes_needed;
}
