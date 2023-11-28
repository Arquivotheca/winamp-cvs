#ifndef NULLSOFT_NU_PTRLISTH
#define NULLSOFT_NU_PTRLISTH

#include <stddef.h>
#include <stdlib.h>
namespace nu
{
class PtrListBase
{
public:
	PtrListBase();
	PtrListBase(const PtrListBase &copy);
	virtual ~PtrListBase();
	inline void clear()
	{
		numPtrs = 0;
	}
	inline size_t size() const
	{
		return numPtrs;
	}
	inline size_t capacity()
	{
		return allocSize;
	}
	inline bool empty() const
	{
		return numPtrs == 0;
	}

	int reserve(size_t reserveSize);
	void eraseindex(size_t index);
	void erase(void *callback);
	void eraseRange(size_t first, size_t last);
	void eraseAll(void *callback);
	int push_back(void *callback);
	void pop_back();
	void pop_front();
	void push_front(void *callback);
	bool contains(void *callback);
	void insertBefore(size_t index, void *callback);
	void insertBefore(size_t index, void *first, size_t count);
	void own(PtrListBase *victim);
	void freeAll();
	bool findItem(void *callback, size_t *index);
	bool findItem_reverse(void *callback, size_t *index); // does the lookup in reverse, use when you have good reason to believe it will be near the end
	
protected:
	void **callbacks;
	size_t numPtrs;
	size_t allocSize;
};

template <class PtrType>
class PtrList : public nu::PtrListBase
{
public:
	typedef PtrType **iterator;
	inline PtrType *&operator[](size_t index) const
	{
		return (PtrType *&)callbacks[index];
	}
	inline PtrType *&at(size_t index) const
	{
		return (PtrType *&)callbacks[index];
	}
	inline PtrType *back()
	{
		if (numPtrs)
			return (PtrType *&)callbacks[numPtrs-1];
		else
			return 0;
	}
	inline PtrType *front()
	{
		if (numPtrs)
			return (PtrType *&)callbacks[0];
		else
			return 0;
	}
	iterator begin() const
	{
		return (PtrType **)&callbacks[0];
	}

	iterator end() const
	{
		return (PtrType **)&callbacks[numPtrs];
	}

	int push_back(PtrType *callback)
	{
		return PtrListBase::push_back((void *)(callback));
	}

	void deleteAll()
	{
		for (size_t i = 0; i != numPtrs;i++)
			delete(PtrType *)(callbacks[i]);

		clear();
	}

	void insert(iterator where, PtrType *item)
	{
		size_t index = (void **)where - callbacks;
		insertBefore(index, item);
	}

};

template <class HandleType>
class HandleList : public nu::PtrListBase
{
public:
	typedef HandleType *iterator;
	inline HandleType &operator[](size_t index)
	{
		return (HandleType &)callbacks[index];
	}
	inline HandleType &at(size_t index)
	{
		return (HandleType &)callbacks[index];
	}
	inline HandleType back()
	{
		if (numPtrs)
			return (HandleType &)callbacks[numPtrs-1];
		else
			return 0;
	}
	inline HandleType front()
	{
		if (numPtrs)
			return (HandleType &)callbacks[0];
		else
			return 0;
	}
	iterator begin()
	{
		return (HandleType *)&callbacks[0];
	}

	iterator end()
	{
		return (HandleType *)&callbacks[numPtrs];
	}

	int push_back(HandleType callback)
	{
		return PtrListBase::push_back((void *)(callback));
	}

	void insert(iterator where, HandleType item)
	{
		size_t index = (void **)where - callbacks;
		insertBefore(index, item);
	}
};

}
#endif
