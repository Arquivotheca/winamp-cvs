#include "nsid3v2.h"
#include "header.h"
#include "tag.h"
#include "../nu/AutoWide.h"
#include <strsafe.h>

/* TODO: better checking of data_len in ParseText and ParseUserText
including checking that it's divisible by 2 for UTF-16 */
int NSID3v2_Header_Valid(const void *header_data)
{
	if (ID3v2::Header::Valid((const ID3v2::HeaderData *)header_data))
		return NErr_Success;
	else
		return NErr_Error;
}

int NSID3v2_Header_Create(nsid3v2_header_t *header, const void *header_data, size_t header_len)
{
	if (header_len < 10 || !ID3v2::Header::Valid((const ID3v2::HeaderData *)header_data))
		return NErr_Error;

	*header = (nsid3v2_header_t)new ID3v2::Header(header_data);
	return NErr_Success;
}

int NSID3v2_Header_TagSize(const nsid3v2_header_t h, uint32_t *tag_size)
{
	const ID3v2::Header *header = (const ID3v2::Header *)h;
	if (!header)
		return NErr_NullPointer;

	*tag_size = header->TagSize();
	return NErr_Success;
}

int NSID3v2_Header_HasFooter(const nsid3v2_header_t h)
{
	const ID3v2::Header *header = (const ID3v2::Header *)h;
	if (!header)
		return NErr_NullPointer;

	if (header->HasFooter())
		return NErr_Success;
	else
		return NErr_False;
}

int NSID3v2_Header_Destroy(nsid3v2_header_t h)
{
	const ID3v2::Header *header = (const ID3v2::Header *)h;
	if (!header)
		return NErr_NullPointer;

	delete header;
	return NErr_Success;
}

/*
================== Tag ================== 
=                                       =
========================================= 
*/

int NSID3v2_Tag_Create(nsid3v2_tag_t *t, nsid3v2_header_t h, const void *bytes, size_t bytes_len)
{
	const ID3v2::Header *header = (const ID3v2::Header *)h;
	if (!header)
		return NErr_NullPointer;

	switch(header->GetVersion())
	{
	case 2:
		{
			ID3v2_2::Tag *tag = new ID3v2_2::Tag(*header);
			tag->Parse(bytes, bytes_len);
			*t = (nsid3v2_tag_t)tag;
			return NErr_Success;
		}
	case 3:
		{
			ID3v2_3::Tag *tag = new ID3v2_3::Tag(*header);
			tag->Parse(bytes, bytes_len);
			*t = (nsid3v2_tag_t)tag;
			return NErr_Success;
		}
	case 4:
		{
			ID3v2_4::Tag *tag = new ID3v2_4::Tag(*header);
			tag->Parse(bytes, bytes_len);
			*t = (nsid3v2_tag_t)tag;
			return NErr_Success;
		}
	default:
		return NErr_NotImplemented;
	}
}

int NSID3v2_Tag_URL_GetUTF16(const nsid3v2_tag_t t, int frame_enum, wchar_t *buf, size_t buf_cch, int text_flags)
{
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	const ID3v2::Frame *frame = tag->FindFirstFrame(frame_enum);
	if (frame)
	{
		const void *data;
		size_t data_len;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0)
		{
			UINT codepage = (text_flags & NSID3V2_TEXT_SYSTEM)?28591:CP_ACP;
			int utf16_len = MultiByteToWideChar(codepage, 0, (const char *)data, data_len, 0, 0);

			if (utf16_len)
			{
				utf16_len = MultiByteToWideChar(codepage, 0, (const char *)data, data_len, buf, utf16_len-1);
				buf[utf16_len]=0;
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
int NSID3v_Tag_GetFrame(const nsid3v2_tag_t t, int frame_enum, nsid3v2_frame_t *f)
{
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	const ID3v2::Frame *frame = tag->FindFirstFrame(frame_enum);
	if (frame)
	{
		*f = (nsid3v2_frame_t)frame;
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int NSID3v_Tag_GetNextFrame(const nsid3v2_tag_t t, const nsid3v2_frame_t start_frame, nsid3v2_frame_t *f)
{
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	const ID3v2::Frame *frame = tag->FindNextFrame((const ID3v2::Frame *)start_frame);
	if (frame)
	{
		*f = (nsid3v2_frame_t)frame;
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int NSID3v2_Tag_RemoveFrame(nsid3v2_tag_t t, nsid3v2_frame_t f)
{
	ID3v2::Tag *tag = (ID3v2::Tag *)t;
	ID3v2::Frame *frame = (ID3v2::Frame *)f;
	tag->RemoveFrame(frame);
	return NErr_Success;
}

int NSID3v2_Frame_Text_SetUTF16(nsid3v2_frame_t f, const wchar_t *value)
{
	ID3v2::Frame *frame = (ID3v2::Frame *)f;
	size_t len = wcslen(value);
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
		memcpy(data8+1, value, len*2);
	}
	return ret;
}


int NSID3v2_Tag_CreateFrame(nsid3v2_tag_t t, int frame_enum, int flags, nsid3v2_frame_t *f)
{
	ID3v2::Tag *tag = (ID3v2::Tag *)t;
	*f = (nsid3v2_frame_t)tag->NewFrame(frame_enum, flags);
	return NErr_Success;
}

int NSID3v2_Tag_AddFrame(nsid3v2_tag_t t, nsid3v2_frame_t f)
{
	ID3v2::Tag *tag = (ID3v2::Tag *)t;
	ID3v2::Frame *frame = (ID3v2::Frame *)f;
	tag->AddFrame(frame);
	return NErr_Success;
}

int NSID3v2_Frame_UserText_SetUTF16(nsid3v2_frame_t f, const wchar_t *description, const wchar_t *value)
{
	ID3v2::Frame *frame = (ID3v2::Frame *)f;
	size_t value_len = wcslen(value);
	size_t description_len = wcslen(value);
	size_t bytes = (value_len + description_len + 1) * 2 + 1; // leave 1 byte for encoding

	size_t datalen;
	void *data;
	int ret = frame->NewData(bytes, &data, &datalen);
	if (ret == NErr_Success)
	{
		uint8_t *data8 = (uint8_t *)data;
		data8[0]=1; // set encoding to UTF-16
		wcscpy((wchar_t *)(data8+1), description); // guaranteed to be room
		memcpy(data8+1+1+description_len*2, value, value_len*2);
	}
	return ret;
}

int NSID3v2_Frame_UserText_SetLatin(nsid3v2_frame_t f, const char *description, const char *value)
{
	ID3v2::Frame *frame = (ID3v2::Frame *)f;
	size_t value_len = strlen(value);
	size_t description_len = strlen(value);
	size_t bytes = (value_len + description_len + 1) + 1; // leave 1 byte for encoding

	size_t datalen;
	void *data;
	int ret = frame->NewData(bytes, &data, &datalen);
	if (ret == NErr_Success)
	{
		uint8_t *data8 = (uint8_t *)data;
		data8[0]=0; // set encoding to ISO-8859-1
		strcpy((char *)(data8+1), description); // guaranteed to be room
		memcpy(data8+1+1+description_len, value, value_len);
	}
	return ret;
}