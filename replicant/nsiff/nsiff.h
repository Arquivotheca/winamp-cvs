#pragma once
#include "foundation/types.h"
#include "foundation/error.h"
#include "nx/nxuri.h"
#include "nx/nxapi.h"
#include "nx/nxfile.h"

#ifdef _MSC_VER
#ifdef NSIFF_EXPORTS
#define NSIFF_API __declspec(dllexport)
#else
#define NSIFF_API __declspec(dllimport)
#endif
#elif defined(__ANDROID__) || defined(__APPLE__)
#define NSIFF_API __attribute__ ((visibility("default")))
#else
#error port me
#endif


#ifdef __cplusplus
extern "C" {
#endif

	/* Thread safety:
	an IFF object is not thread safe.
	It can, however, by used by multiple threads as long as the application performs external locking (e.g. critical section)
	If it is used solely by a single thread, you do not need to lock.  
	nsiff does not spawn any threads and all callbacks occur on the thread that called nsiff_parse, etc
	You can clone an IFF object via nsiff_clone and provide an independent reader object.  
	This clone can be used by another thread independently of the original IFF object without locking.
	*/
	// opaque handle type for an IFF parser/editor/creator
	typedef struct nsiff_s {} *nsiff_t;

	enum nsiff_return_t
	{
		// return values from your on_chunk callback function
		nsiff_continue=0, // continue parsing.  if the chunk was not read (via nsiff_read_current_chunk), it will be skipped
		nsiff_skip=1, // skips the chunk, even if it contains nested chunks.  the parser will still internally read any nested chunks, but no callbacks will be generated
		nsiff_ignore=2, // skips the entire chunk.  the parser will not try to read nest chunks, even for internal use
		nsiff_interrupt=3, // interrupts parsing.  you can continue later if you'd like.
		nsiff_abort=4, // aborts parsing and leaves the parser in an error state.  

		// OR any of these with the return value for special handling
		nsiff_flag_mask = (0xFFFF << 16), // a mask to apply to only look at the flags (or only look at the return value)
		nsiff_flag_padding = (1 << 16), // tells the parser that this chunk was meant as padding for the previous chunk (e.g. JUNK).
		nsiff_flag_list = (1 << 17), // tells the parser that this chunk is actually a list.  Your callback will get called again with "type" filled in
	};

	// TODO: ensure proper structure alignment between compilers
	typedef struct nsiff_chunk_s
	{
		uint64_t position; // position of the chunk header in the file (NOT the data position)
		uint8_t chunk[4];
		uint64_t size; // LIST-style chunks include the extra 4 bytes, beware!
		uint8_t type[4]; // if type[0] == 0, then it's not a LIST
		uint64_t data_position; // this is here to remove knowledge from users of chunk sizes (e.g. Wave64 chunk sizes differ from RIFF/IFF)
	} *nsiff_chunk_t;

	typedef void (*NSIFF_ON_RETAIN)(void *context, nsiff_t iff_object); 
	typedef void (*NSIFF_ON_RELEASE)(void *context, nsiff_t iff_object); 
	typedef nsiff_return_t (*NSIFF_ON_CHUNK)(void *context, nsiff_t iff_object, const uint8_t *path, const nsiff_chunk_t chunk);
	typedef void (*NSIFF_ON_PARSE_COMPLETE)(void *context, nsiff_t iff_object);
	typedef void (*NSIFF_ON_ERROR)(void *context, nsiff_t iff_object, ns_error_t error_code);
	typedef struct nsiff_callbacks_s
	{
		/* on_retain callback is called when an associate is established between an nsiff_t object and your context pointer.
		this function will be called before any other callback function, and it will be called during nsiff_create_*
		useful if you need to do reference counting internally.
		prototye: void on_retain(void *context, nsiff_t iff_object);
		iff_object: the IFF object.  Note that when you get this call, nsiff_create_* won't have returned, so this will be the first time you have seen the IFF object's handle
		context: a pointer passed in by your application.  typically a cast pointer to some object
		*/
		NSIFF_ON_RETAIN on_retain;

		/* on_retain callback is called when an association is de-established between an nsiff_t object and your context pointer.
		no other callback function will be called after this by the same nsiff_t object
		useful if you need to do reference counting internally.
		prototye: void on_release(void *context, nsiff_t iff_object);
		iff_object: the IFF object.  
		context: a pointer passed in by your application.  typically a cast pointer to some object
		*/
		NSIFF_ON_RELEASE on_release;

		/* on_retain callback is called when an associate is established between an nsiff_t object and your callback and context pointer.
		useful if you need to do reference counting internally
		prototye: void on_retain(void *context, nsiff_t iff_object);
		iff_object: the IFF object.  
		context: a pointer passed in by your application.  typically a cast pointer to some object
		*/

		/* on_chunk callback is called for every chunk encountered in the IFF file.  The passed fields are:
		prototype: nsiff_return_t on_chunk(void *context, nsiff_t iff_object, const uint8_t *path, const nsiff_chunk_t chunk);
		iff_object: the IFF object.  
		context: a pointer passed in by your application.  typically a cast pointer to some object
		path: a zero terminated path to the chunk, without the leading RIFF/RF64/etc.  use nsiff_match_path to compare
		chunk: pointer to chunk struct, see nsiff_chunk_s 
		*/
		NSIFF_ON_CHUNK on_list_start;
		NSIFF_ON_CHUNK on_chunk;
		NSIFF_ON_CHUNK on_list_end;

		/* on_parse_complete is called when the file has been parsed to the end */
		NSIFF_ON_PARSE_COMPLETE on_parse_complete;

		/* on_error is called when an error is encountered mid-stream */
		NSIFF_ON_ERROR on_error;

	} *nsiff_callbacks_t;

	NSIFF_API ns_error_t nsiff_create_parser_from_file(nsiff_t *out_iff_object, nx_file_t file, nsiff_callbacks_t callbacks, void *context);

	/* nsiff_clone creates a new IFF object with the same state as an existing, valid IFF object.
	The parse state remains intact.
	However, a new reader is opened (and a new callback structure and context pointer can be optionally assigned)
	If the same context pointer is shared between two IFF objects, you must ensure thread safety internally in your callback functions.
	This function is used mostly for reading two independent streams out of the same file, e.g. AVI audio/video playback of an HTTP stream might have two sockets which read independently
	nsiff_parse does not need to be called on the new object.
	Note that this function is not thread-safe.  
	It must be locked with respect to other users of the "original" IFF object (or called on the same thread if the object is being used by only one thread without locking)
	However, the newly created copied IFF object can be used independently on another thread after creation
	*/
	// TODO: int nsiff_clone(nsiff_t *out_iff_object, nsiff_t original, nsiff_callbacks_t callbacks, void *context);

	/* Flushes any pending I/O, finalizes data structures and closes the underlying reader/writer object.
	However, you can still call some nsiff_* functions to query parse state (e.g. determing chunk locations and sizes)
	It is not recommended that you call this function if you are going to immediately destroy the object, as it might do unnecessary work
	An error might be returned in some cases, e.g. if pending I/O on an editor/writer object has failed.  
	*/
	NSIFF_API ns_error_t nsiff_close(nsiff_t iff_object);

	/* destroys an IFF object.   
	We have not implemented reference counting, because nsiff is not thread-safe */
	NSIFF_API void nsiff_destroy(nsiff_t iff_object);

	
	/* begins or resumes parsing of the IFF file.  Callbacks are called until the file is completely parsed or your on_chunk callback returns nsiff_interrupt/nsiff_abort
	*/
	NSIFF_API ns_error_t nsiff_parse(nsiff_t iff_object);

	/* reads some or all of the current chunk.
	This can only be called in the middle of an on_chunk callback.  Calling this at any other time will lead to undefined behavior */
	NSIFF_API ns_error_t nsiff_read_current_chunk(nsiff_t object, void *data, size_t read_size, size_t *bytes_read);

	NSIFF_API ns_error_t nsiff_seek(nsiff_t iff_object, uint64_t position);
	NSIFF_API ns_error_t nsiff_tell(nsiff_t iff_object, uint64_t *position);

	NSIFF_API ns_error_t nsiff_read(nsiff_t iff_object, void *data, size_t bytes_requested, size_t *bytes_read);
#ifdef __cplusplus
}
#endif