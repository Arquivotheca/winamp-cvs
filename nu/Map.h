#ifndef NULLSOFT_UTILITY_MAP_H
#define NULLSOFT_UTILITY_MAP_H

#include "Vector.h"
#include "Pair.h"
namespace nu
{

/**
  This is a simple map from one data type to another. It isn't nearly as
  super-powered as the STL map or anything. It will, however, let you specify
  the sorting type as PtrList-compatible sort class.
  It cannot hold all classes, just simple ones, with valid copy constructors.
  They also must have < and == operators.
*/
#define MAP_IMPL Vector<Pair<INDEX, DATA> >
template <class INDEX, class DATA>
class Map : private Vector<Pair<INDEX, DATA> >
{
public:
		typedef Pair<INDEX, DATA> MapPair;
		typedef Pair<INDEX, DATA> value_type;

		typedef typename MAP_IMPL::iterator iterator;
		typedef typename MAP_IMPL::const_iterator const_iterator;

public:
	DATA &operator [](INDEX index)
	{
		MapPair &mp = getItem(index);
		return mp.second;
	}

public:
	iterator find(const INDEX &a) const
	{
		for (size_t i = 0; i != size(); i++)
		{
			if (at(i).first == a)
			{
				return &at(i);
			}
		}

		return end();
	}
	
	size_t getPosition(const INDEX &a)
	{
		for (size_t i = 0; i != size(); i++)
		{
			if (at(i).first == a)
			{
				return i;
			}
		}
		return size();
	}

	MapPair &getItem(const INDEX &a)
	{
		for (size_t i = 0; i != size(); i++)
		{
			if (at(i).first == a)
			{
				return at(i);
			}
		}
		push_back(MapPair(a, DATA()));
		return Vector<Pair<INDEX, DATA> >::back();
	}

	bool reverseLookup(const DATA &a, INDEX *index)
	{
		for (size_t i = 0; i != size(); i++)
		{
			if (at(i).second== a)
			{
				*index = at(i).first;
				return true;
			}
		}
		return false;
	}


	void insert(MapPair &newPair)
	{
		push_back(newPair);
	}

	void erase(const INDEX &a)
	{
		for (size_t i = 0; i != size(); i++)
		{
			if (at(i).first == a)
			{
				 Vector<Pair<INDEX, DATA> >::eraseAt(i);
				 return;
			}
		}
	}

	using Vector<MapPair>::at;
	using Vector<MapPair>::begin;
	using Vector<MapPair>::end;
	using Vector<MapPair>::clear;
	using Vector<MapPair>::size;
	using Vector<MapPair>::Reset;
	using Vector<MapPair>::erase;
	using Vector<MapPair>::empty;
};

}

#endif
