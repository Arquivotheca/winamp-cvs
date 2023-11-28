#ifndef _MAP_H
#define _MAP_H

#include <bfc/std_math.h>
#include <bfc/pair.h>
#include <bfc/ptrlist.h>

/**
  This is a simple map from one data type to another. It isn't nearly as
  super-powered as the STL map or anything. It will, however, let you specify
  the sorting type as PtrList-compatible sort class.
  It cannot hold all classes, just simple ones, with valid copy constructors.
  They also must have < and == operators.
*/
template <class INDEX, class DATA, class COMPARE=NotSorted>
class Map {
protected:
  typedef Pair<INDEX, DATA> MapPair;
  class SortPair {
  public:
    static int compareItem(MapPair *p1, MapPair* p2) {
      if (p1->a < p2->a) return -1;
      if (p1->a == p2->a) return CMP3(p1, p2);
      return 1;
    }
    static int compareAttrib(const wchar_t *attrib, MapPair *item) {
      return CMP3<INDEX>(*(INDEX*)attrib, item->a);
    }
  };

public:
  Map() {
//    list.setAutoSort(0);	// for now, no sorting
  }
  ~Map() {
    deleteAll();
  }

/**
  Adds an item to the map. No dup checking on the index will be done.
*/
  void addItem(const INDEX &a, const DATA &b) {
    list.addItem(new MapPair(a, b));
  }

  void purge() {
    list.purge();
  }

/**
  Fetches a data record for an index.
  @param a The index value.
  @param b The pointer for where data value should be written. Can be NULL.
  @ret TRUE if a record was found, FALSE otherwise.
  @see getItemRef()
  @see reverseGetItem()
*/
  int getItem(const INDEX &a, DATA *b=NULL) 
	{
    MapPair *mp = list.findItem((const wchar_t *)&a);
//    MapPair *mp = NULL;
    for (int i = 0; i < list.getNumItems(); i++) {
      if (list[i]->a == a) {
        mp = list[i];
        if (b) *b = mp->b;
        return 1;
      }
    }
    return 0;
  }

/**
  Fetches the first index for a given data record. Note that this uses a slower
  linear search method (since the map is sorted by index, not data).
  @see getItem()
*/
  int reverseGetItem(const DATA &b, INDEX *a=NULL) {
    MapPair *mp = NULL;
    for (int i = 0; i < list.getNumItems(); i++) {
      if (list[i]->b == b) {
        mp = list[i];
        if (a) *a = mp->a;
        return 1;
      }
    }
    return 0;
  }

/**
  Returns a pointer to the data field for an index.
  @see getItem()
*/
	const DATA *getItemRef(const INDEX &a) {
    MapPair *mp = NULL;
    for (int i = 0; i < list.getNumItems(); i++) {
      if (list[i]->a == a) {
        mp = list[i];
        return &mp->b;
      }
    }
    return NULL;
  }

	DATA *getItemMutableRef(const INDEX &a) {
    MapPair *mp = NULL;
    for (int i = 0; i < list.getNumItems(); i++) {
      if (list[i]->a == a) {
        mp = list[i];
        return &mp->b;
      }
    }
    return NULL;
  }

/**
  Removes the record for the given index.
*/
  int delItem(const INDEX &a) {
    for (int i = 0; i < list.getNumItems(); i++) {
      MapPair *pair = list.q(i);
      if (pair->a == a) {
        list.removeByPos(i);
        delete pair;
        return 1;
      }
    }
    return 0;
  }

/**
  Removes the record for the first record for the given data.
*/
  int reverseDelItem(const DATA &b) {
    for (int i = 0; i < list.getNumItems(); i++) {
      MapPair *pair = list.q(i);
      if (pair->b == b) {
        list.removeByPos(i);
        delete pair;
        return 1;
      }
    }
    return 0;
  }

  void setItem(const INDEX &a, const DATA &b) {
    // cheesy huh
    delItem(a);
    addItem(a, b);
  }

/**
  Returns how many records are in the map.
*/
  int getNumItems() const {
    return list.getNumItems();
  }

  DATA enumItemByPos(int n, DATA default_val) {
    MapPair *mp = list.enumItem(n);
    if (mp == NULL) return default_val;
    return mp->b;
  }

  INDEX enumIndexByPos(int n, INDEX default_val) {
    MapPair *mp = list.enumItem(n);
    if (mp == NULL) return default_val;
    return mp->a;
  }

/**
  Returns how many records are in the map. Same as getNumItems(), but better named.
*/
  int getNumPairs() const {
    return list.getNumItems();
  }

  void deleteAll() {
    list.deleteAll();
  }

protected:	// not private, so SortPair can access us
//  PtrList<MapPair> list;
  PtrListQuickSorted<MapPair, SortPair> list;
};

#endif
