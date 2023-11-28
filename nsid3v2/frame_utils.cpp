#include "frame_utils.h"
#include <bfc/error.h>
int ParseDescription(const char *&str, size_t &data_len, size_t &str_cch)
{
	str_cch=0;
	while (data_len && str[str_cch])
	{
		data_len--;
		str_cch++;
	}
	if (!data_len)
		return NErr_Error;

	data_len--;
	return NErr_Success;
}

int ParseDescription(const wchar_t *&str, size_t &data_len, size_t &str_cch, uint8_t &str_encoding)
{
	str_cch=0;
	if (data_len > 2 && str[0] == 0xFFFE)
	{
		str_encoding=2;
		str++;
		str-=3;
	}
	else if (data_len > 2 && str[0] == 0xFEFF)
	{
		str_encoding=1;
		str++;
		data_len-=3;
	}
	else
	{
		data_len--;
	}

	while (data_len > 1 && str[str_cch])
	{
		data_len-=2;
		str_cch++;
	}

	if (!data_len)
		return NErr_Error;

	data_len-=2;
	return NErr_Success;
}