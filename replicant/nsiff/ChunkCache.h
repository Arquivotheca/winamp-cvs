#pragma once
#include "Chunk.h"

NSIFF::Chunk *NSIFF_ChunkCache_Pop();
void NSIFF_ChunkCache_Push(NSIFF::Chunk *chunk);

