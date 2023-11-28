#pragma once
#include "vector.h"
#include "Pair.h"
#include "foundation/types.h"
#include <string.h>

template <class val_t>
class PtrMapComp
{
public:
	int operator ()(const val_t &a, const val_t &b)
	{
		if (a < b)
			return -1;
		else if (a > b)
			return 1;
		else
			return 0;
	}
};


template <>
class PtrMapComp<GUID>
{
public:
	int operator()(const GUID &a, const GUID &b)
	{
		return memcmp(&a, &b, sizeof(GUID));
	}
};

#define PTRMAP_PARENT Vector<nu::Pair<INDEX, DATA *>, 32, 2>

template <class INDEX, class DATA, class COMP=PtrMapComp<INDEX> >
class PtrMap : private PTRMAP_PARENT
{
public:
#ifdef __GNUC__
	typedef PTRMAP_PARENT __super;
#endif
	typedef nu::Pair<INDEX, DATA *> MapPair;
	typedef nu::Pair<INDEX, DATA *> value_type;
	typedef typename __super::iterator iterator;
	typedef typename __super::const_iterator const_iterator;

	DATA *&operator [](INDEX index)
	{
		MapPair &mp = getItem(index);
		return mp.second;
	}
	
	void set(INDEX index, DATA *data) // an alternative to operator [] for classes that use private inheritance
	{
		getItem(index).second = data;
	}

	MapPair &getItem(const INDEX &a)
	{
		if (size() == 0)
		{
            __super::push_back(MapPair(a, 0));
			return at(0);
		}

		ptrdiff_t bot = 0, top = (ptrdiff_t)size() - 1, mid;

		for (ptrdiff_t c = 0; c < (ptrdiff_t)size() + 1; c++)
		{
			if (bot > top) 
			{
				__super::insert(bot, MapPair(a, 0));
				return at(bot);
			}
			mid = (bot + top) / 2;
			INDEX compareTo = at(mid).first;
			COMP comp;
			int compare = comp(a, compareTo);
			if (compare == 0)
			{
				return at(mid);
			}
			else if (compare < 0)
			{
				top = mid - 1;
			}
			else
			{
				bot = mid + 1;
			}
		}
        __super::push_back(MapPair(a, 0));
		return at(size()-1);
	}

	iterator find(const INDEX &a) const
	{
		if (size() == 0)
		{
			return end();
		}

		ptrdiff_t bot = 0, top = (ptrdiff_t)size() - 1, mid;

		for (ptrdiff_t c = 0; c < (ptrdiff_t)size() + 1; c++)
		{
			if (bot > top) 
			{
				return end();
			}
			mid = (bot + top) / 2;
			INDEX compareTo = at(mid).first;
			COMP comp;
						int compare = comp(a, compareTo);
			if (compare == 0)
			{
				return &at(mid);
			}
			else if (compare < 0)
			{
				top = mid - 1;
			}
			else
			{
				bot = mid + 1;
			}
		}
		return end();
	}

	void insert(MapPair &newPair)
	{
		getItem(newPair.first).second = newPair.second;
	}

	void deleteAll()
	{
		for (iterator itr=begin();itr!=end();itr++)
		{
			delete itr->second;
		}
		clear();
	}

	using PTRMAP_PARENT::at;
	using PTRMAP_PARENT::begin;
	using PTRMAP_PARENT::end;
	using PTRMAP_PARENT::clear;
	using PTRMAP_PARENT::size;
	using PTRMAP_PARENT::erase;
	using PTRMAP_PARENT::empty;
};
