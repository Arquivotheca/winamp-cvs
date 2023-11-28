#include "Parser.h"
#include "nx/nxfile.h"
#include "ChunkCache.h"
#include "Chunk.h"
#include "nu/ByteReader.h"
#include <assert.h>
#include <new.h>



NSIFF::Parser::Parser()
{
	current_depth = 0;
	path_position=0;
	path[0]=0;

	file = 0;
	callbacks = 0;
	context = 0;
	position = 0;
	parse_state = STATE_IDENTIFIED;
	file_type = NSIFF::FileType_Unknown;
	file_mode = NSIFF::FileMode_Unknown;
	root_chunk = 0;
	current_chunk = 0;
}

static void ReleaseChunkTree(NSIFF::Chunk *chunk)
{
	// benski> REVIEW: if we wanted to eliminate recursion here (for stack-space/IFF-depth reasons), we could serialize everything to a single nodelist_t and push from there
	// because the chunk cache is multithreaded, we need to get any pointers out of it BEFORE giving it back to the cache,
	NSIFF::Chunk *iterator = chunk;
	while (iterator)
	{
		if (iterator->children.head)
			ReleaseChunkTree((NSIFF::Chunk *)iterator->children.head);

		NSIFF::Chunk *next = (NSIFF::Chunk *)iterator->Next;
		NSIFF_ChunkCache_Push(iterator);
		iterator = next;
	}	
}

NSIFF::Parser::~Parser()
{
	NXFileRelease(file);

	if (callbacks && callbacks->on_release)
		callbacks->on_release(context, (nsiff_t)this);

	callbacks=0;
	context=0;
	ReleaseChunkTree(root_chunk);
}

ns_error_t NSIFF::Parser::CreateParser(NSIFF::Parser **out_parser, nx_file_t file, nsiff_callbacks_t callbacks, void *context)
{
	NSIFF::Parser *parser=0;
	NSIFF::FileType file_type;
	NSIFF::FileMode file_mode;

	NSIFF::Chunk *identify = NSIFF_ChunkCache_Pop();
	if (!identify)
		return NErr_OutOfMemory;

	identify->position = 0;
	size_t bytes_read;

	if (NXFileRead(file, identify->chunk, 4, &bytes_read) != NErr_Success	|| bytes_read != 4)
	{
		NSIFF_ChunkCache_Push(identify);
		return NErr_Malformed;
	}

	if (!memcmp(identify->chunk, "RIFF", 4))
	{
		file_type = NSIFF::FileType_RIFF;
		file_mode = NSIFF::FileMode_32LE;
	}
	else if (!memcmp(identify->chunk, "FORM", 4))
	{
		file_type = NSIFF::FileType_FORM;
		file_mode = NSIFF::FileMode_32BE;
	}
#if 0 // TODO: find test file
	else if (!memcmp(identify->chunk, "RIFX", 4))
	{
		file_type = NSIFF::FileType_RIFX;
		file_mode = NSIFF::FileMode_32BE;		
	}
#endif
	// TODO: else if (!memcmp(identify->chunk, "riff", 4)) // Wave64
	else
	{
		NSIFF_ChunkCache_Push(identify);
		return NErr_NotImplemented;
	}

	bytereader_value_t byte_reader;
	if (NSIFF::FileMode_Is32(file_mode))
	{
		uint8_t buffer[4];
		if (NXFileRead(file, buffer, 4, &bytes_read) != NErr_Success || bytes_read != 4)
		{
			NSIFF_ChunkCache_Push(identify);
			return NErr_ReadTruncated;
		}

		bytereader_init(&byte_reader, buffer, 4);
		uint32_t size;
		if (NSIFF::FileMode_IsLittleEndian(file_mode))
			size = bytereader_read_u32_le(&byte_reader);
		else
			size = bytereader_read_u32_be(&byte_reader);

		if (size < 4)
		{
			NSIFF_ChunkCache_Push(identify);
			return NErr_Malformed;
		}

		identify->size = size;

		if (NXFileRead(file, identify->type, 4, &bytes_read) != NErr_Success || bytes_read != 4)
		{
			NSIFF_ChunkCache_Push(identify);
			return NErr_ReadTruncated;
		}

		if (NSIFF::FileMode_IsLittleEndian(file_mode))
			parser = new (std::nothrow) NSIFF::Parser32LE;
		else
			parser = new (std::nothrow) NSIFF::Parser32BE;
		if (!parser)
		{
			NSIFF_ChunkCache_Push(identify);
			return NErr_OutOfMemory;
		}

		parser->position = 12;
	}
	else
	{
		return NErr_NotImplemented;
	}

	identify->position = parser->position;
	parser->file = NXFileRetain(file);
	parser->callbacks = callbacks;
	parser->context = context;	
	parser->file_mode = file_mode;
	parser->file_type = file_type;
	parser->root_chunk = identify;

	if (callbacks && callbacks->on_retain)
		callbacks->on_retain(context, (nsiff_t)parser);

	*out_parser = parser;
	return NErr_Success;
}

ns_error_t NSIFF::Parser::ParseList(NSIFF::Chunk *parent, uint64_t size, bool skip)
{
	while (size)
	{
		if (size < 8)
			return NErr_Malformed; // TODO: we might be able to gracefully handle this

		NSIFF::Chunk *chunk = NSIFF_ChunkCache_Pop();
		if (!chunk)
			return NErr_OutOfMemory;

		chunk->position = position;
		ns_error_t ret = ReadChunk(chunk);
		if (ret == NErr_ReadTruncated)
		{
			NSIFF_ChunkCache_Push(chunk);
			return NErr_Success;
		}
		else if (ret != NErr_Success)
		{
			NSIFF_ChunkCache_Push(chunk);
			return ret;
		}

		chunk->parent = parent;
		chunk->data_position = position;
		bool is_list = chunk->type[0] != 0;
		const uint8_t *type = is_list?chunk->type:0;
		uint64_t chunk_data_size = is_list?(chunk->size - 4):chunk->size;
		if (chunk_data_size > size)
		{
			NSIFF_ChunkCache_Push(chunk);
			return NErr_Malformed; // TODO: we might be able to gracefully handle this
		}

		nsiff_return_t callback_return;
		if (is_list)
		{
			current_chunk = chunk;
			callback_return = callbacks->on_list_start(context, (nsiff_t)this, path, chunk);

			if ((callback_return & ~nsiff_flag_mask) != nsiff_ignore)
			{
				ret = PushPath(chunk->type);
				if (ret != NErr_Success)
					return ret; // don't push the chunk onto the stack, since we've already added it to the parent's nodelist
				ns_error_t ret = ParseList(chunk, chunk_data_size, skip);
				PopPath();
				if (ret != NErr_Success)
					return ret; // don't push the chunk onto the stack, since we've already added it to the parent's nodelist
			}

			callbacks->on_list_end(context, (nsiff_t)this, path, chunk);
		}
		else
		{
					current_chunk = chunk;
				nsiff_return_t callback_return = callbacks->on_chunk(context, (nsiff_t)this, path, chunk);
				
				// TODO: handle flags, for now we will bail out
				if (callback_return & nsiff_flag_mask)
				{
					NSIFF_ChunkCache_Push(chunk);
					return NErr_NotImplemented;
				}

				switch(callback_return & ~nsiff_flag_mask)
				{
					case nsiff_interrupt:
					NSIFF_ChunkCache_Push(chunk);
					return NErr_Interrupted; // TODO: somehow we need to get back here. that could be difficult
				case nsiff_abort:
					NSIFF_ChunkCache_Push(chunk);
					return NErr_Aborted;
				//default:
//					NSIFF_ChunkCache_Push(chunk);
	//				return NErr_BadReturnValue;
				}

		}

		nodelist_push_back(&parent->children, chunk);

		// TODO: if any CRT implementations handle nop seeks poorly, we can check for _ftelli64() == position beforehand
		NXFileSeek(file, chunk->position + chunk->size + 8);
		NXFileTell(file, &position);
		size -= (position - chunk->position);

		// adjust to required 2 byte alignment
		if (position & 1)
		{
			if (!size)
				return NErr_Malformed; // TODO: we might be able to gracefully handle this
			size--;
			position++;
			NXFileRead(file, 0, 1, 0);
		}


	}
	return NErr_Success;
}

ns_error_t NSIFF::Parser::Parse()
{
	bool skip_all=false;
	// see if we need to provide the root chunk
	if (parse_state == STATE_IDENTIFIED)
	{
		parse_state = STATE_PARSING;
		if (callbacks && callbacks->on_chunk)
		{
			nsiff_return_t callback_return = callbacks->on_list_start(context, (nsiff_t)this, 0 /* TODO: Path */, root_chunk);
			switch(callback_return & ~nsiff_flag_mask)
			{
			case nsiff_continue:
				break;
			case nsiff_skip:
				skip_all=true;
				break;
			case nsiff_ignore:
				return NErr_EndOfFile;
			case nsiff_interrupt:
				return NErr_Interrupted;
			case nsiff_abort:
				return NErr_Aborted;
			default:
				return NErr_BadReturnValue;
			}
		}
	}
	else if (parse_state == STATE_PARSING)
	{
		// we were resumed.  this means we need to figure out where the hell we left off
	}

	int ret = ParseList(root_chunk, root_chunk->size - 4, skip_all);
	if (ret != NErr_Success)
		return ret;
	callbacks->on_list_end(context, (nsiff_t)this, 0 /* TODO: Path */, root_chunk);
	return NErr_Success;
}

bool NSIFF::Parser::IsList(uint8_t chunk[4])
{
	if (!memcmp(chunk, "LIST", 4))
		return true;

	return false;
}

ns_error_t NSIFF::Parser::PushPath(uint8_t chunk[4])
{
	if (current_depth == NSIFF::max_depth)
		return NErr_MaximumDepth;

	memcpy(&path[current_depth*4], chunk, 4);
	current_depth++;
	path[current_depth*4]=0;
	return 0;
}

void NSIFF::Parser::PopPath()
{
	assert(current_depth);
	current_depth--;
	path[current_depth*4]=0;

}

ns_error_t NSIFF::Parser::ReadCurrentChunk(void *data, size_t read_size, size_t *bytes_read)
{
	if ((uint64_t)read_size > current_chunk->size)
		read_size = (size_t)current_chunk->size;
	return Read(data, read_size, bytes_read);
}

ns_error_t NSIFF::Parser::Seek(uint64_t position)
{
	return NXFileSeek(file, position);	
}

ns_error_t NSIFF::Parser::Tell(uint64_t *position)
{
	return NXFileTell(file, position);
}

ns_error_t NSIFF::Parser::Read(void *data, size_t read_size, size_t *_bytes_read)
{
	if (NXFileEndOfFile(file) == NErr_True)
		return NErr_EndOfFile;
	size_t bytes_read;
	int ret = NXFileRead(file, data, read_size, &bytes_read);
	if (ret != NErr_Success)
		return ret;

	if (_bytes_read)
		*_bytes_read = bytes_read;	
	return NErr_Success;
}
