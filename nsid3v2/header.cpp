#include "header.h"
#include "values.h"
#include "util.h"
#include <assert.h>

ID3v2::Header::Header()
{
	memset(&headerData, 0, sizeof(HeaderData));
}

ID3v2::Header::Header(const void *data)
{
	Parse(data);
}

void ID3v2::Header::Parse(const void *data)
{
	memcpy(&headerData, data, sizeof(HeaderData));
}

bool ID3v2::Header::Valid(const HeaderData *data)
{
if (data->marker[0] != 'I'
		|| data->marker[1] != 'D'
		|| data->marker[2] != '3')
		return false;

	if (!Values::KnownVersion(data->version, data->revision))
		return false;

	if (data->flags & ~Values::ValidHeaderMask(data->version, data->revision))
		return false;

	if (data->size & 0x80808080)
		return false;

	return true;
}

bool ID3v2::Header::Valid() const
{
	return Valid(&headerData);
}

uint32_t ID3v2::Header::TagSize() const
{
	assert(Valid());
	uint32_t size = Util::Int28To32(headerData.size);
	if (headerData.flags & 0x10) // check for footer flag
		size+=10;
	return size;
}

bool ID3v2::Header::HasExtendedHeader() const
{
	return !!(headerData.flags & ID3v2::Values::ExtendedHeaderFlag(headerData.version, headerData.revision));
}

uint8_t ID3v2::Header::GetVersion() const
{
	return headerData.version;
}

uint8_t ID3v2::Header::GetRevision() const
{
	return headerData.revision;
}

bool ID3v2::Header::Unsynchronised() const
{
	return !!(headerData.flags & (1 << 7));
}

bool ID3v2::Header::HasFooter() const
{
	return !!(headerData.flags & 0x10);
}