#include "tag.h"
#include "frameheader.h"
#include "frames.h"

/* === ID3v2 common === */
static bool IdentifierMatch(const int8_t *id1, const int8_t *id2)
{
	return !memcmp(id1, id2, 4);
}


ID3v2::Tag::Tag(const ID3v2::Header &_header) :  header(_header)
{
}

void ID3v2::Tag::RemoveFrame(ID3v2::Frame *frame)
{
	frames.erase(frame);
}

ID3v2::Frame *ID3v2::Tag::FindFirstFrame(const int8_t *id) const
{
	for (FrameList::const_iterator itr=frames.begin();itr != frames.end();itr++)
	{
		if (IdentifierMatch(itr->GetIdentifier(), id))
			return *itr;
	}
	return 0;
}

ID3v2::Frame *ID3v2::Tag::FindNextFrame(const Frame *frame) const
{
	for (FrameList::const_iterator itr=frame;itr != frames.end();itr++)
	{
		if (IdentifierMatch(itr->GetIdentifier(), frame->GetIdentifier()))
			return *itr;
	}
	return 0;
}

void ID3v2::Tag::RemoveFrames(const int8_t *id)
{
	// TODO: not exactly the fastest way
	Frame *frame;
	while (frame = FindFirstFrame(id))
		frames.erase(frame);
}
void ID3v2::Tag::AddFrame(ID3v2::Frame *frame)
{
	frames.push_back(frame);
}

/* === ID3v2.2 === */

ID3v2_2::Tag::Tag(const ID3v2::Header &_header) : ID3v2::Tag(_header), extendedHeader(header)
{
}

ID3v2_2::Tag::~Tag()
{
	frames.deleteAll();
}

static inline void Advance(const void *&data, size_t &len, size_t amount)
{
	data = (const uint8_t *)data + amount;
	len -= amount;
}

int ID3v2_2::Tag::Parse(const void *data, size_t len)
{
	/* Is there an extended header? */
	if (header.HasExtendedHeader())
	{
		size_t read=0;
		if (extendedHeader.Parse(data, len, &read) != 0)
		{
			return 1;
		}
		Advance(data, len, read);
	}

	/* Read each frame */
	while (len >= FrameHeader::SIZE)
	{
		/* if next byte is zero, we've hit the padding area, GTFO */
		if (*(uint8_t *)data == 0x0)
			break;

		/* Read frame header first */
		FrameHeader frame_header(header, data);
		Advance(data, len, FrameHeader::SIZE);

		if (!frame_header.IsValid())
			return 1;

		/* read frame data */
		Frame *new_frame = new Frame(frame_header);
		size_t read=0;
		if (new_frame->Parse(data, len, &read) == 0)
		{
			Advance(data, len, read);
			frames.push_back(new_frame);
		}
		else
		{
			delete new_frame;
			return 1;
		}
	}
	return 0;
}

static bool IdentifierMatch3(const int8_t *id1, const int8_t *id2)
{
	return !memcmp(id1, id2, 3);
}

ID3v2_2::Frame *ID3v2_2::Tag::FindFirstFrame(int frame_id) const
{
	return (ID3v2_2::Frame *)ID3v2::Tag::FindFirstFrame(frame_ids[frame_id].v2);
};


void ID3v2_2::Tag::RemoveFrames(int frame_id)
{
		// TODO: not exactly the fastest way
	Frame *frame;
	while (frame = FindFirstFrame(frame_id))
		frames.erase(frame);
}
/* === ID3v2.3 === */

ID3v2_3::Tag::Tag(const ID3v2::Header &_header) : ID3v2::Tag(_header), extendedHeader(header)
{
}

ID3v2_3::Tag::~Tag()
{
	frames.deleteAll();
}

int ID3v2_3::Tag::Parse(const void *data, size_t len)
{
	/* Is there an extended header? */
	if (header.HasExtendedHeader())
	{
		size_t read=0;
		if (extendedHeader.Parse(data, len, &read) != 0)
		{
			return 1;
		}
		Advance(data, len, read);
	}

	/* Read each frame */
	while (len >= FrameHeader::SIZE)
	{
		/* if next byte is zero, we've hit the padding area, GTFO */
		if (*(uint8_t *)data == 0x0)
			break;

		/* Read frame header first */
		FrameHeader frame_header(header, data);
		Advance(data, len, FrameHeader::SIZE);

		if (!frame_header.IsValid())
			return 1;

		/* read frame data */
		Frame *new_frame = new Frame(frame_header);
		size_t read=0;
		if (new_frame->Parse(data, len, &read) == 0)
		{
			Advance(data, len, read);
			frames.push_back(new_frame);
		}
		else
		{
			delete new_frame;
			return 1;
		}
	}
	return 0;
}


ID3v2_3::Frame *ID3v2_3::Tag::FindFirstFrame(int frame_id) const
{
	return (ID3v2_3::Frame *)ID3v2::Tag::FindFirstFrame(frame_ids[frame_id].v3);
};

void ID3v2_3::Tag::RemoveFrames(int frame_id)
{
		// TODO: not exactly the fastest way
	Frame *frame;
	while (frame = FindFirstFrame(frame_id))
		frames.erase(frame);
}

/* === ID3v2.4 === */
ID3v2_4::Tag::Tag(const ID3v2::Header &_header) : ID3v2::Tag(_header), extendedHeader(header)
{
}

ID3v2_4::Tag::~Tag()
{
	frames.deleteAll();
}

int ID3v2_4::Tag::Parse(const void *data, size_t len)
{
	/* Is there an extended header? */
	if (header.HasExtendedHeader())
	{
		size_t read=0;
		if (extendedHeader.Parse(data, len, &read) != 0)
		{
			return 1;
		}
		Advance(data, len, read);
	}

	/* Read each frame */
	while (len >= FrameHeader::SIZE)
	{
		/* if next byte is zero, we've hit the padding area, GTFO */
		if (*(uint8_t *)data == 0x0)
			break;

		/* Read frame header first */
		FrameHeader frame_header(header, data);
		Advance(data, len, FrameHeader::SIZE);

		if (!frame_header.IsValid())
			return 1;

		/* read frame data */
		Frame *new_frame = new Frame(frame_header);
		size_t read=0;
		if (new_frame->Parse(data, len, &read) == 0)
		{
			Advance(data, len, read);
			frames.push_back(new_frame);
		}
		else
		{
			delete new_frame;
			return 1;
		}
	}
	return 0;
}

ID3v2_4::Frame *ID3v2_4::Tag::FindFirstFrame(int frame_id) const
{
	return (ID3v2_4::Frame *)ID3v2::Tag::FindFirstFrame(frame_ids[frame_id].v4);
};

void ID3v2_4::Tag::RemoveFrames(int frame_id)
{
		// TODO: not exactly the fastest way
	Frame *frame;
	while (frame = FindFirstFrame(frame_id))
		frames.erase(frame);
}

ID3v2_4::Frame *ID3v2_4::Tag::NewFrame(int frame_id, int flags) const
{
	return new ID3v2_4::Frame(header, frame_ids[frame_id].v4, flags); 
}