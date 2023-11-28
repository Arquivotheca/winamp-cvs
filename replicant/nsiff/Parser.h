#pragma once
#include "nx/nxuri.h"
#include "nsiff.h"
#include "Types.h"
#include <stdio.h>


namespace NSIFF
{
	static const size_t max_depth=128;
	class Chunk;
	class Parser
	{
	public:
		static ns_error_t CreateParser(Parser **out_parser, nx_file_t file, nsiff_callbacks_t callbacks, void *context);
		ns_error_t Parse();
		virtual ~Parser();
		ns_error_t ReadCurrentChunk(void *data, size_t read_size, size_t *bytes_read);
		ns_error_t Read(void *data, size_t read_size, size_t *bytes_read);
		ns_error_t Seek(uint64_t position);
		ns_error_t Tell(uint64_t *position);
	protected:
		Parser();
		
		virtual ns_error_t ReadChunk(NSIFF::Chunk *chunk)=0; // it can be assumed that chunk is memset to 0 before being passed in

		static bool IsList(uint8_t chunk[4]);
		ns_error_t ParseList(NSIFF::Chunk *parent, uint64_t size, bool skip);
		enum State
		{
			STATE_IDENTIFIED,
			STATE_PARSING,
		};

		nx_file_t file;
		nsiff_callbacks_t callbacks;
		void *context;
		uint64_t position;
		State parse_state;
		NSIFF::FileType file_type;
		NSIFF::FileMode file_mode;
		NSIFF::Chunk *root_chunk;
		NSIFF::Chunk *current_chunk;
		uint8_t path[max_depth*4 + 1];
		size_t path_position;
		size_t current_depth;
	private:
		ns_error_t PushPath(uint8_t chunk[4]);
		void PopPath();
	};

	class Parser32LE : public Parser
	{
	protected:
		ns_error_t ReadChunk(NSIFF::Chunk *chunk);
	};

	class Parser32BE : public Parser
	{
	protected:
		ns_error_t ReadChunk(NSIFF::Chunk *chunk);
	};
}
