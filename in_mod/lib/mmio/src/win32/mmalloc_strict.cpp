/*
Ben Allison
benski@winamp.com
(c) 2012 Nullsoft, Inc.
*/


#include <string.h>
#include <intsafe.h>
#include "../../../../nu/PtrDeque2.h"
#include "../../../../nu/AutoLock.h"
#include "mmio.h"
#include <malloc.h>
#include <new>
//#define MM_STRICT_HEAP

static Nullsoft::Utility::LockGuard alloc_forbid;

struct MM_BLOCKINFO : public nu::PtrDequeNode
{
	MM_ALLOC *parent;
	size_t size;
	char block[1];
};


// =====================================================================================
struct MM_ALLOC : public nu::PtrDequeNode
	// =====================================================================================
	// An allocation handle for use with the mmio allocation functions.
{
	MM_ALLOC();
	MM_ALLOC(const char *_name);
	~MM_ALLOC();
	char name[64];             // every allocation type has a name.  wowie zowie!
	MM_ALLOC *parent;
	nu::PtrDeque2<MM_ALLOC> children;
#ifdef MM_STRICT_HEAP
	nu::PtrDeque2<MM_BLOCKINFO> blocks;
#else
	HANDLE block_heap;
#endif
	void (*shutdown)(void *blockdata);
	void *shutdata;
};

MM_ALLOC::MM_ALLOC()
{
	name[0]=0;
	shutdown=0;
	shutdata=0;
	parent=0;
#ifndef MM_STRICT_HEAP
	block_heap = 0;
#endif
}

MM_ALLOC::MM_ALLOC(const char *_name)
{
	if (_name)
	{
		strncpy(name, _name, 63);
		name[63]=0;
	}
	else
	{
		name[0]=0;
	}
	shutdown=0;
	shutdata=0;
	parent=0;
#ifndef MM_STRICT_HEAP
	block_heap = HeapCreate(0, 0, 0);
#endif
}

MM_ALLOC::~MM_ALLOC()
{
	HeapDestroy(block_heap);
}

static MM_ALLOC global_alloc;


//static CHAR *msg_fail = "%s > Failure allocating block of size: %d";
static CHAR *msg_set  = "Out of memory!  Please check the logfile for more information.";

static size_t BlockMallocSize(size_t bytes)
{
	/* TODO: overflow check? */
	const MM_BLOCKINFO *dummy=0;
	size_t header = (size_t)&dummy->block[0] - (size_t)dummy;
	return header + bytes + 4096;
}

static MM_BLOCKINFO *GetBlockPointer(void *pointer)
{
	const MM_BLOCKINFO *dummy=0;
	size_t header = (size_t)&dummy->block[0] - (size_t)dummy;
	return (MM_BLOCKINFO *)((size_t)pointer - header);
}

static void myfree(MM_ALLOC *type, MM_BLOCKINFO *block)
{
#ifdef MM_STRICT_HEAP
	char *mem = (char *)((size_t)block & ~4095);
	VirtualFree(mem, 0, MEM_RELEASE);
#else
	free(block);
#endif
}

#ifdef MM_STRICT_HEAP
static void *StrictAlloc(size_t bytes)
{
	size_t allocated_size = (bytes + 8191) & ~4095;
	size_t offset = 4096 - (bytes & 4095);
	size_t pages = allocated_size / 4096;
	char *protect_start ;
	void *mem = VirtualAlloc(0, allocated_size, MEM_COMMIT, PAGE_READWRITE);

	if (!mem)
		return 0;


	protect_start = (char *)mem + (pages-1)*4096;
	VirtualProtect(protect_start, 4096, PAGE_NOACCESS, 0);

	return (void *)((char *)mem + offset);

}
#endif
static MM_BLOCKINFO *mymalloc(MM_ALLOC *type, size_t size)
{
#ifdef MM_STRICT_HEAP
	size_t bytes = BlockMallocSize(size);

	MM_BLOCKINFO *block = (MM_BLOCKINFO *)StrictAlloc(bytes);
	if (block)
	{
		block->parent=type;
		block->size = size;
	}
	return block;
#else
	size_t bytes = BlockMallocSize(size);
	MM_BLOCKINFO *block = (MM_BLOCKINFO *)malloc(bytes);
	if (!block)
		return 0;

	block->parent=type;
	block->size = size;
	return block;
#endif
}


static MM_BLOCKINFO *myrealloc(MM_ALLOC *type, MM_BLOCKINFO *old_block, size_t size)
{
#ifdef MM_STRICT_HEAP
	MM_BLOCKINFO *new_block =mymalloc(type, size);
	memcpy(new_block->block, old_block->block, old_block->size);
	myfree(type, old_block);
	return new_block;
#else
	size_t bytes = BlockMallocSize(size);
	MM_BLOCKINFO *block = (MM_BLOCKINFO *)realloc(old_block, bytes);
	if (!block)
		return 0;

	block->parent = type;
	block->size = size;
	return block;
#endif
}



MM_ALLOC *_mmalloc_create(const char *name, MM_ALLOC *parent)
{
#ifdef MM_STRICT_HEAP
	void *mem = StrictAlloc(sizeof(MM_ALLOC));
	MM_ALLOC *type = new (mem) MM_ALLOC(name);	
#else
	MM_ALLOC *type = new MM_ALLOC(name);
#endif

	if (!type)
		return 0;

	if (parent)
		type->parent = parent;
	else
		type->parent = &global_alloc;

	if (parent)
	{
		Nullsoft::Utility::AutoLock auto_lock(alloc_forbid);
		parent->children.push_back(type);
	}
	else
	{
		Nullsoft::Utility::AutoLock auto_lock(alloc_forbid);
		global_alloc.children.push_back(type);
	}
	return type;
}

void _mmalloc_setshutdown(MM_ALLOC *type, void (*shutdown)(void *block), void *block)
{
	if (type)
	{
		Nullsoft::Utility::AutoLock auto_lock(alloc_forbid);
		type->shutdown = shutdown;
		type->shutdata = block;
	}
}

// Shuts down the given alloc handle/type.  Frees all memory associated with the handle
// and optionally checks for buffer under/overruns (if they have been flagged).
void _mmalloc_close(MM_ALLOC *type)
{
	assert(type != &global_alloc);
	if (type)
	{
		Nullsoft::Utility::AutoLock auto_lock(alloc_forbid);

		if (type->shutdown) 
			type->shutdown(type->shutdata);

		if (type->parent)
			type->parent->children.erase(type);
		type->next = 0;
		type->prev = 0;

		// close child allocators
		while (!type->children.empty())
		{
			MM_ALLOC *child = type->children.back();
			_mmalloc_close(child);

		}
		#ifdef MM_STRICT_HEAP
		// free all blocks
		while (!type->blocks.empty())
		{
			MM_BLOCKINFO *block = type->blocks.back();
			type->blocks.pop_back();
			myfree(type, block);
		}


		char *mem = (char *)((size_t)type & ~4095);
		VirtualFree(mem, 0, MEM_RELEASE);
#else
		delete type;
#endif
	}
}


void *MikMod_malloc(MM_ALLOC *type, size_t size)
{
	Nullsoft::Utility::AutoLock auto_lock(alloc_forbid);

	if (!type)
	{
		type = &global_alloc;
	}

	MM_BLOCKINFO *d = mymalloc(type, size);

	if (!d)
	{
		_mmlog("MikMod_malloc> failure allocating block of size: %d", size);
		_mmerr_set(MMERR_OUT_OF_MEMORY, msg_set);
	}
	else
	{
		#ifdef MM_STRICT_HEAP
		type->blocks.push_back(d);
#endif
		return d->block;
	}

	return d;
}

void MikMod_freex(MM_ALLOC *type, void *_d)
{
	void **d = (void **)_d;
#ifdef MM_STRICT_HEAP
	
	if (d && *d)
	{
		void *block_data = *d;
		MM_BLOCKINFO *block = GetBlockPointer(block_data);

		if (!type)
			type = block->parent;

		assert(type == block->parent);
		alloc_forbid.Lock();
		type->blocks.erase(block);
		alloc_forbid.Unlock();

		myfree(type, block);
		*d = 0;
	}
#else
	if (d && *d)
	{
		myfree(type, _d);		
		*d=0;
	}
#endif
}

void *MikMod_calloc(MM_ALLOC *type, size_t nitems, size_t size)
{
	if (SizeTMult(nitems, size, &size) != S_OK)
	{
		_mmlog("MikMod_calloc> Integer Overflow: %u items, size %u", nitems, size);
		_mmerr_set(MMERR_OUT_OF_MEMORY, msg_set);
		return 0;
	}

	void *block = MikMod_malloc(type, size);
	if (!block)
		return 0;

	memset(block, 0, size);

	return block;
}

void *_mm_realloc(MM_ALLOC *type, void *old_blk, size_t size)
{
	if (!old_blk)
		return MikMod_malloc(type, size);

	Nullsoft::Utility::AutoLock auto_lock(alloc_forbid);


	MM_BLOCKINFO *block=0;

	block = GetBlockPointer(old_blk);

	assert(type == block->parent);
	if (!type)
		type = block->parent;


	type->blocks.erase(block);
	MM_BLOCKINFO *new_block = myrealloc(type, block, size);
	if (new_block)
		type->blocks.push_back(new_block);

	return new_block->block;
}
