#include "ChunkCache.h"
#include "nu/LockFreeLIFO.h"
static lifo_t chunk_cache = {0,};
static lifo_t cache_bases= {0,};

#define CHUNK_CACHE_SEED 64
static void RefillCache()
{
	NSIFF::Chunk *cache_seed = (NSIFF::Chunk *)malloc(CHUNK_CACHE_SEED*sizeof(NSIFF::Chunk));
	
	if (cache_seed)
	{
		int i=CHUNK_CACHE_SEED;
		while (--i)
		{
			lifo_push(&chunk_cache, (queue_node_t *)&cache_seed[i]);
		}
		lifo_push(&cache_bases, (queue_node_t *)cache_seed);
	}
	else
	{
		Sleep(0); // yield and hope that someone else pops something off soon		
	}	
}

NSIFF::Chunk *NSIFF_ChunkCache_Pop()
{
	NSIFF::Chunk *chunk = 0;

	do 
	{
		chunk = (NSIFF::Chunk *)lifo_pop(&chunk_cache);
		if (!chunk)
			RefillCache();
	} while (!chunk);
	memset(chunk, 0, sizeof(*chunk));
	return chunk;
}

void NSIFF_ChunkCache_Push(NSIFF::Chunk *chunk)
{
	lifo_push(&chunk_cache, chunk);
}