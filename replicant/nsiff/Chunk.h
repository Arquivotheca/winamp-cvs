#pragma once
#include "foundation/types.h"
#include "nu/nodelist.h"
#include "nsiff.h"

namespace NSIFF
{
	class Chunk : public queue_node_t, public nsiff_chunk_s
	{
	public:
		nodelist_s children;
		Chunk *parent; // to allow us to resume parsing more effectively
		// TODO cache line padding?
	};
}