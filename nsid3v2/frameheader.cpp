#include "frameheader.h"
#include "util.h"
#include "values.h"

/* === ID3v2 common === */
ID3v2::FrameHeader::FrameHeader(const ID3v2::Header &_header) : tagHeader(_header)
{
}

static bool CharOK(int8_t c)
{
	if (c >= '0' && c <= '9')
		return true;

	if (c >= 'A' && c <= 'Z')
		return true;

	return false;
}

/* === ID3v2.2 === */
ID3v2_2::FrameHeader::FrameHeader(const ID3v2::Header &_header, const int8_t *id, int flags) : ID3v2::FrameHeader(_header)
{
		memcpy(&frameHeaderData.id, id, 3);
		frameHeaderData.id[3]=0;
		memset(&frameHeaderData.size, 0, 3);
}

ID3v2_2::FrameHeader::FrameHeader(const ID3v2::Header &_header, const void *data) : ID3v2::FrameHeader(_header)
{
	if (tagHeader.Unsynchronised())
		ID3v2::Util::UnsynchroniseTo(&frameHeaderData, data, sizeof(FrameHeaderData));
	else
	{
		memcpy(&frameHeaderData.id, data, 3);
		frameHeaderData.id[3]=0;
		memcpy(&frameHeaderData.size, (int8_t *)data+3, 3);
	}
}

bool ID3v2_2::FrameHeader::IsValid() const
{
	if (CharOK(frameHeaderData.id[0])
		&& CharOK(frameHeaderData.id[1])
		&& CharOK(frameHeaderData.id[2]))
	return true;

	return false;
}

const int8_t *ID3v2_2::FrameHeader::GetIdentifier() const
{
	return frameHeaderData.id;
}

bool ID3v2_2::FrameHeader::Unsynchronised() const
{
	return tagHeader.Unsynchronised();
}

uint32_t ID3v2_2::FrameHeader::FrameSize() const
{
	return (frameHeaderData.size[0] << 16) | (frameHeaderData.size[1] << 8) | (frameHeaderData.size[2]);
}

/* === ID3v2.3+ common === */
ID3v2_3::FrameHeaderBase::FrameHeaderBase(const ID3v2::Header &_header, const int8_t *id, int flags) : ID3v2::FrameHeader(_header)
{
		memcpy(&frameHeaderData.id, id, 4);
		memset(&frameHeaderData.size, 0, 4);
		// TODO: flags
		frameHeaderData.flags[0]=0;
		frameHeaderData.flags[1]=0;
}

ID3v2_3::FrameHeaderBase::FrameHeaderBase(const ID3v2::Header &_header, const void *data) : ID3v2::FrameHeader(_header)
{
	if (tagHeader.Unsynchronised())
		ID3v2::Util::UnsynchroniseTo(&frameHeaderData, data, sizeof(FrameHeaderData));
	else
		memcpy(&frameHeaderData, data, sizeof(FrameHeaderData));
}

const int8_t *ID3v2_3::FrameHeaderBase::GetIdentifier() const
{
	return frameHeaderData.id;
}


bool ID3v2_3::FrameHeaderBase::IsValid() const
{
	if (CharOK(frameHeaderData.id[0])
		&& CharOK(frameHeaderData.id[1])
		&& CharOK(frameHeaderData.id[2])
		&& CharOK(frameHeaderData.id[3]))
	return true;

	return false;
}

/* === ID3v2.3 === */
ID3v2_3::FrameHeader::FrameHeader(const ID3v2::Header &_header, const int8_t *id, int flags) : ID3v2_3::FrameHeaderBase(_header, id, flags)
{
}

ID3v2_3::FrameHeader::FrameHeader(const ID3v2::Header &_header, const void *data) : ID3v2_3::FrameHeaderBase(_header, data)
{
	
}

uint32_t ID3v2_3::FrameHeader::FrameSize() const
{
	return ID3v2::Util::UInt32RawToUInt32(frameHeaderData.size);
}

bool ID3v2_3::FrameHeader::Unsynchronised() const
{
	return tagHeader.Unsynchronised();
}

bool ID3v2_3::FrameHeader::Grouped() const
{
	return !!(frameHeaderData.flags[1] & (1 << 5));
}

bool ID3v2_3::FrameHeader::Compressed() const
{
	return !!(frameHeaderData.flags[1] & (1 << 7));
}



/* === ID3v2.4 === */
ID3v2_4::FrameHeader::FrameHeader(const ID3v2::Header &_header, const int8_t *id, int flags) : ID3v2_3::FrameHeaderBase(_header, id, flags)
{
}

ID3v2_4::FrameHeader::FrameHeader(const ID3v2::Header &_header, const void *data) : ID3v2_3::FrameHeaderBase(_header, data)
{
}

uint32_t ID3v2_4::FrameHeader::FrameSize() const
{
	// many programs write non-syncsafe sizes (iTunes is the biggest culprit)
	// so we'll try to detect it.  unfortunately this isn't foolproof
	// ID3v2_4::Frame will have some additional checks
	int mask = frameHeaderData.size & 0x80808080;
	if (mask)
		return ID3v2::Util::UInt32RawToUInt32(frameHeaderData.size);
	else
		return ID3v2::Util::Int28To32(frameHeaderData.size);
}

bool ID3v2_4::FrameHeader::Unsynchronised() const
{
	return tagHeader.Unsynchronised() || !!(frameHeaderData.flags[1] & (1 << 1));
}

bool ID3v2_4::FrameHeader::DataLengthIndicated() const
{
	return !!(frameHeaderData.flags[1] & (1 << 0));
}

bool ID3v2_4::FrameHeader::Compressed() const
{
	return !!(frameHeaderData.flags[1] & (1 << 3));
}

bool ID3v2_4::FrameHeader::Grouped() const
{
	return !!(frameHeaderData.flags[1] & (1 << 6));
}
