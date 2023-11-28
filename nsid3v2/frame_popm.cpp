#include "nsid3v2.h"
#include "header.h"
#include "tag.h"
#include "../nu/AutoWide.h"
#include <strsafe.h>

struct ParsedPopularimeter
{
	const char *email;
	size_t cch;
	uint8_t rating;
	uint64_t playcount;
};

static int ParsePopularimeter(const void *data, size_t data_len, ParsedPopularimeter &parsed)
{
	if (data_len < 6)
		return NErr_Error;

	const uint8_t *data8 = (const uint8_t *)data;
	parsed.email = (const char *)&data8[0];

		while (data_len && parsed.email[parsed.cch])
		{
			data_len--;
			parsed.cch++;
		}

		parsed.rating = data8[parsed.cch+1];
		data_len--;
		parsed.playcount=0;
		const uint8_t *p = &data8[parsed.cch+2];
		while (data_len)
		{
			parsed.playcount <<= 8;
			parsed.playcount |= *p++;
		}
		return NErr_Success;
}


int NSID3v2_Tag_Popularimeter_GetRatingPlaycount(const nsid3v2_tag_t t, const char *email, uint8_t *rating, uint64_t *playcount)
{
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	const ID3v2::Frame *frame = tag->FindFirstFrame(NSID3V2_FRAME_POPULARIMETER);
	while (frame)
	{
		const void *data;
		size_t data_len;
		ParsedPopularimeter parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParsePopularimeter(data, data_len, parsed) == NErr_Success)
		{
			if (!_stricmp(email, parsed.email))
			{
				*rating = parsed.rating;
				*playcount = parsed.playcount;
				return NErr_Success;
			}

		}
		frame = tag->FindNextFrame(frame);
	}

	return NErr_Error;
}

