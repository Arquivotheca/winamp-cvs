#include "nsid3v2.h"
#include "nsid3v2/header.h"
#include "nsid3v2/tag.h"
#include "nu/utf.h"
#include <string.h>

#if 0 // save for reference
int NSID3v2_Tag_URL_GetUTF8(const nsid3v2_tag_t t, int frame_enum, char *buf, size_t buf_cch, int text_flags)
{
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	const ID3v2::Frame *frame = tag->FindFirstFrame(frame_enum);
	if (frame)
	{
		const void *data;
		size_t data_len;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0)
		{
			// TODO: UINT codepage = (text_flags & NSID3V2_TEXT_SYSTEM)?28591:CP_ACP;
			size_t utf8_len = ISO_8859_1_to_utf8((const char *)data, data_len, 0, 0);

			if (utf8_len)
			{
				utf8_len = ISO_8859_1_to_utf8((const char *)data, data_len, buf, utf8_len);
				buf[utf8_len]=0;
			}
			else
			{
				buf[0]=0;
			}
			return NErr_Success;
		}
	}
	return NErr_Error;
}

int NSID3v2_Frame_Text_SetUTF8(nsid3v2_frame_t f, const char *value)
{
	ID3v2::Frame *frame = (ID3v2::Frame *)f;
	size_t len = utf8_to_utf16LE(value, strlen(value), 0, 0);
	size_t bytes = len * 2 + 1; // leave 1 byte for encoding
	if (bytes < len) // woops, integer overflow
		return NErr_Error;

	size_t datalen;
	void *data;
	int ret = frame->NewData(bytes, &data, &datalen);
	if (ret == NErr_Success)
	{
		uint8_t *data8 = (uint8_t *)data;
		data8[0]=1; // set encoding to UTF-16
		utf8_to_utf16LE(value, strlen(value), (uint16_t *)(data8+1), len);
	}
	return ret;
}

int NSID3v2_Frame_UserText_SetUTF8(nsid3v2_frame_t f, const char *description, const char *value)
{
	ID3v2::Frame *frame = (ID3v2::Frame *)f;
	size_t value_len = utf8_to_utf16LE(value, strlen(value), 0, 0);
	size_t description_len = utf8_to_utf16LE(description, strlen(description), 0, 0);
	size_t bytes = (value_len + description_len + 1) * 2 + 1; // leave 1 byte for encoding

	size_t datalen;
	void *data;
	int ret = frame->NewData(bytes, &data, &datalen);
	if (ret == NErr_Success)
	{
		uint8_t *data8 = (uint8_t *)data;
		data8[0]=1; // set encoding to UTF-16
		utf8_to_utf16LE(description, strlen(description), (uint16_t *)(data8+1), description_len);
		utf8_to_utf16LE(value, strlen(value), (uint16_t *)(data8+1+1+description_len*2), value_len);
	}
	return ret;
}
#endif