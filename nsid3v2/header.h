#pragma once

#include <bfc/platform/types.h>

namespace ID3v2
{
	
#pragma pack(push, 1)
	struct HeaderData
	{
		uint8_t marker[3];
		uint8_t version;
		uint8_t revision;
		uint8_t flags;
		uint32_t size;
	};
#pragma pack(pop)

	class Header
	{
	public:
		Header();
		Header(const void *data);
		void Parse(const void *data);
		/* Does this seem like a valid ID3v2 header? */
		bool Valid() const;
		static bool Valid(const HeaderData *data);
		/* how much space the tag occupies on disk */
		uint32_t TagSize() const;
		bool HasExtendedHeader() const;
		uint8_t GetVersion() const;
		uint8_t GetRevision() const;
		bool Unsynchronised() const;
		bool HasFooter() const;
		enum
		{
			SIZE=10,
		};
	private:
		HeaderData headerData;
	};
}
