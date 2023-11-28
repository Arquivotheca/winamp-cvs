#pragma once
#include "../nu/PtrMap.h"
#include "avi_reader.h"

namespace nsavi
{
	typedef PtrMap<uint32_t, char> InfoMap;
	class Info : private InfoMap 
	{
	public:
		Info();
		~Info();
		int Read(avi_reader *reader, uint32_t data_len);
		const char *GetMetadata(uint32_t id);

		typedef InfoMap::const_iterator const_iterator;
		using InfoMap::begin;
		using InfoMap::end;
	};
};