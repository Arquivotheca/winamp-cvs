#pragma once
#include <bfc/platform/types.h>
#include "header.h"

namespace ID3v2
{
	class FrameHeader
	{
	protected:
		FrameHeader(const ID3v2::Header &_header);
		const ID3v2::Header &tagHeader;
	};
}

namespace ID3v2_2
{
#pragma pack(push, 1)
	struct FrameHeaderData
	{
		int8_t id[4]; // ID3v2.2 uses 3 bytes but we add a NULL for the last to make it easier
		uint8_t size[3]; // 24 bit size field
	};
#pragma pack(pop)

	class FrameHeader : public ID3v2::FrameHeader
	{
	public:
		FrameHeader(const ID3v2::Header &_header, const int8_t *id, int flags);
		FrameHeader(const ID3v2::Header &_header, const void *data);
		bool IsValid() const;
		bool Unsynchronised() const;
		uint32_t FrameSize() const;
		const int8_t *GetIdentifier() const;
		enum
		{
			SIZE=6,
		};
	private:
		FrameHeaderData frameHeaderData;
	};
}

namespace ID3v2_3
{
#pragma pack(push, 1)
	struct FrameHeaderData
	{
		int8_t id[4];
		uint32_t size;
		uint8_t flags[2];
	};
#pragma pack(pop)

	class FrameHeaderBase : public ID3v2::FrameHeader
	{
	public:
		bool IsValid() const;
		const int8_t *GetIdentifier() const;
		enum
		{
			SIZE=10,
		};

	protected:
		FrameHeaderBase(const ID3v2::Header &_header, const int8_t *id, int flags);
		FrameHeaderBase(const ID3v2::Header &_header, const void *data);

		FrameHeaderData frameHeaderData;
	};

	class FrameHeader : public ID3v2_3::FrameHeaderBase
	{
	public:
		FrameHeader(const ID3v2::Header &_header, const int8_t *id, int flags);
		FrameHeader(const ID3v2::Header &_header, const void *data);
		
		uint32_t FrameSize() const;
		bool Encrypted() const;
		bool Compressed() const;
		bool Grouped() const;
		bool ReadOnly() const;
		bool Unsynchronised() const;
		
		
	private:

	};
}

namespace ID3v2_4
{
	class FrameHeader : public ID3v2_3::FrameHeaderBase
	{
	public:
		FrameHeader(const ID3v2::Header &_header, const int8_t *id, int flags);
		FrameHeader(const ID3v2::Header &_header, const void *data);
		/* size to read from disk */
		uint32_t FrameSize() const;
		bool Encrypted() const;
		bool Compressed() const;
		bool Grouped() const;
		bool ReadOnly() const;
		bool Unsynchronised() const;
		bool DataLengthIndicated() const;
	};
}