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

static Nullsoft::Utility::LockGuard alloc_forbid;


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
	HANDLE block_heap;
	void (*shutdown)(void *blockdata);
	void *shutdata;
};

MM_ALLOC::MM_ALLOC()
{
	name[0]=0;
	shutdown=0;
	shutdata=0;
	parent=0;
	block_heap = HeapCreate(0, 0, 0);
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

	block_heap = HeapCreate(0, 0, 0);
}

MM_ALLOC::~MM_ALLOC()
{
	HeapDestroy(block_heap);
}

static MM_ALLOC global_alloc;


//static CHAR *msg_fail = "%s > Failure allocating block of size: %d";
static CHAR *msg_set  = "Out of memory!  Please check the logfile for more information.";

static void myfree(MM_ALLOC *type, void *block)
{
	HeapFree(type->block_heap, 0, block);
}

static void *mymalloc(MM_ALLOC *type, size_t size)
{
	return HeapAlloc(type->block_heap, 0, size);
}

static void *myrealloc(MM_ALLOC *type, void *old_block, size_t size)
{
	return HeapReAlloc(type->block_heap, 0, old_block, size);
}

MM_ALLOC *_mmalloc_create(const char *name, MM_ALLOC *parent)
{
	MM_ALLOC *type = new MM_ALLOC(name);

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

		delete type;
	}
}


void *MikMod_malloc(MM_ALLOC *type, size_t size)
{
	if (!type)
	{
		type = &global_alloc;
	}

	void *d = mymalloc(type, size);

	if (!d)
	{
		_mmlog("MikMod_malloc> failure allocating block of size: %d", size);
		_mmerr_set(MMERR_OUT_OF_MEMORY, msg_set);
	}

	return d;
}

void MikMod_freex(MM_ALLOC *type, void *_d)
{
	void **d = (void **)_d;

	if (!type)
		type = &global_alloc;

	if (d && *d)
	{
		myfree(type, *d);		
		*d=0;
	}
}

void *MikMod_calloc(MM_ALLOC *type, size_t nitems, size_t size)
{
	if (SizeTMult(nitems, size, &size) != S_OK)
	{
		_mmlog("MikMod_calloc> Integer Overflow: %u items, size %u", nitems, size);
		_mmerr_set(MMERR_OUT_OF_MEMORY, msg_set);
		return 0;
	}

	if (!type)
		type = &global_alloc;

	void *block = MikMod_malloc(type, size);
	if (!block)
		return 0;

	memset(block, 0, size);

	return block;
}

void *_mm_realloc(MM_ALLOC *type, void *old_blk, size_t size)
{
	if (!type)
		type = &global_alloc;

	if (!old_blk)
		return MikMod_malloc(type, size);


	void *new_block = myrealloc(type, old_blk, size);

	return new_block;
}
