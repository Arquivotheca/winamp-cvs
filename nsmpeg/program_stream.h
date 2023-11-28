#pragma once
#include <bfc/platform/types.h>
#include "mpeg_reader.h"

namespace nsmpeg
{
	class ProgramStream
	{
	public:
		ProgramStream();
		nsmpeg::mpeg_reader *reader;
	};
}