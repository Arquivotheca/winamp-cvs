#pragma once
#include "vector.h"
#include "foundation/types.h"
#include <string.h>

template <class val_t>
class ValueSetComp
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
class ValueSetComp<GUID>
{
public:
	int operator()(const GUID &a, const GUID &b)
	{
		return memcmp(&a, &b, sizeof(GUID));
	}
};

template <class INDEX, class COMP=ValueSetComp<INDEX> >
class ValueSet : private Vector<INDEX> 
{
public:
#ifdef __GNUC__
	typedef Vector<INDEX>  __super;
#endif
	typedef INDEX value_type;
	typedef typename __super::iterator iterator;
	typedef typename __super::const_iterator const_iterator;

	INDEX &getItem(const INDEX &a)
	{
		if (size() == 0)
		{
            __super::push_back(a);
			return Vector<INDEX>::at(0);
		}

		ptrdiff_t bot = 0, top = (ptrdiff_t)size() - 1, mid;

		for (ptrdiff_t c = 0; c < (ptrdiff_t)size() + 1; c++)
		{
			if (bot > top) 
			{
				__super::insert(bot, a);
				return Vector<INDEX>::at(bot);
			}
			mid = (bot + top) / 2;
			INDEX compareTo = __super::at(mid);
			COMP comp;
			int compare = comp(a, compareTo);
			if (compare == 0)
			{
				return __super::at(mid);
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
        __super::push_back(a);
		return __super::at(size()-1);
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
			INDEX compareTo = __super::at(mid);
			COMP comp;
			int compare = comp(a, compareTo);
			if (compare == 0)
			{
				return &__super::at(mid);
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

	void insert(const INDEX &a)
	{
		getItem(a);
	}

	void erase(const INDEX &a)
	{
		iterator found = find(a);
		if (found != end())
			Vector<INDEX>::erase(found);
	}

	//using Vector<INDEX>::at;
	using Vector<INDEX>::begin;
	using Vector<INDEX>::end;
	using Vector<INDEX>::clear;
	using Vector<INDEX>::size;
	//using Vector<INDEX>::erase;
	using Vector<INDEX>::empty;
	using Vector<INDEX>::reserve;
};
