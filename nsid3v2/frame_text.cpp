#include "nsid3v2.h"
#include "header.h"
#include "tag.h"
#include <strsafe.h>


struct ParsedText
{
	uint8_t encoding; // 0 - iso-8859-1, 1 - UTF16LE, 2 - UTF16BE, 3 - UTF8
	union 
	{
		const char *as8;
		const wchar_t *as16;
	} string_data;
	size_t cch;
};

static int ParseText(const void *data, size_t data_len, ParsedText &parsed)
{
	const uint8_t *data8 = (const uint8_t *)data;
	switch(data8[0])
	{
	case 0: // ISO-8859-1
		parsed.encoding = 0;
		parsed.string_data.as8 = (const char *)&data8[1];
		parsed.cch = data_len-1;
		return NErr_Success;
	case 1: // UTF-16
		if ((data_len & 1) == 0) 
			return NErr_Error;
		parsed.string_data.as16 = (const wchar_t *)&data8[1];
		parsed.cch = (data_len-1)/2;
		if (parsed.string_data.as16[0] == 0xFFFE)
		{
			parsed.encoding=2;
			parsed.string_data.as16++;
			parsed.cch--;
		}
		else if (parsed.string_data.as16[0] == 0xFEFF)
		{
			parsed.encoding=1;
			parsed.string_data.as16++;
			parsed.cch--;
		}
		else
		{
			parsed.encoding=1;
		}
		return NErr_Success;
	case 2: // UTF-16BE
		parsed.encoding=2;
		parsed.string_data.as16 = (const wchar_t *)&data8[1];
		parsed.cch = (data_len-1)/2;
		return NErr_Success; 
	case 3: // UTF-8
		parsed.encoding = 0;
		parsed.string_data.as8 = (const char *)&data8[1];
		parsed.cch = data_len-1;
		if (parsed.cch >= 3 && parsed.string_data.as8[0] == 0xEF && parsed.string_data.as8[1] == 0xBB && parsed.string_data.as8[2] == 0xBF)
		{
			parsed.string_data.as8+=3;
			parsed.cch-=3;
		}
		return NErr_Success;
	}
	return NErr_NotImplemented;
}

int NSID3v2_Tag_Text_GetUTF16(const nsid3v2_tag_t t, const int8_t *frame_id, wchar_t *buf, size_t buf_cch, int text_flags)
{
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	const ID3v2::Frame *frame = tag->FindFirstFrame(frame_id);
	if (frame)
	{
		const void *data;
		size_t data_len;
		ParsedText parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParseText(data, data_len, parsed) == NErr_Success)
		{
			switch(parsed.encoding)
			{
			case 0: // ISO-8859-1
				{
					UINT codepage = (text_flags & NSID3V2_TEXT_SYSTEM)?28591:CP_ACP;
					int utf16_len = MultiByteToWideChar(codepage, 0, parsed.string_data.as8, parsed.cch, 0, 0);

					if (utf16_len)
					{
						utf16_len = MultiByteToWideChar(codepage, 0, parsed.string_data.as8, parsed.cch, buf, utf16_len-1);
						buf[utf16_len]=0;
					}
					else
					{
						buf[0]=0;
					}
				}
				break;
			case 1: // UTF-16
				StringCchCopyNW(buf, buf_cch, parsed.string_data.as16, parsed.cch);
				break;
			case 2: // UTF-16BE
				{
					size_t toCopy = buf_cch-1;
					if (parsed.cch < toCopy)
						toCopy = parsed.cch;
					for (size_t i=0;i<toCopy;i++)
					{
						buf[i] = ((parsed.string_data.as16[i] >> 8) & 0xFF) | (((parsed.string_data.as16[i]) & 0xFF) << 8);
					}
					buf[toCopy]=0;
				}
				break;
			case 3: // UTF-8
				{
					int utf16_len = MultiByteToWideChar(CP_UTF8, 0, parsed.string_data.as8, parsed.cch, 0, 0);

					if (utf16_len)
					{
						utf16_len = MultiByteToWideChar(CP_UTF8, 0, parsed.string_data.as8, parsed.cch, buf, utf16_len-1);
						buf[utf16_len]=0;
					}
					else
					{
						buf[0]=0;
					}
				}
				break;
			}
			return NErr_Success;
		}
	}

	return NErr_Error;
}


