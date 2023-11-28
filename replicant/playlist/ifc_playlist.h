#pragma once

#include "foundation/dispatch.h"
#include "foundation/types.h"

class ifc_playlist : public Wasabi2::Dispatchable
{
protected:
	ifc_playlist() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_playlist() {}
	
public:
#if 0
	void Clear();
	//void AppendWithInfo(const wchar_t *filename, const char *title, int lengthInMS);
	//void Append(const wchar_t *filename);

	size_t GetNumItems();
	size_t GetItem(size_t item, wchar_t *filename, size_t filenameCch);
	size_t GetItemTitle(size_t item, wchar_t *title, size_t titleCch);
	int GetItemLengthMilliseconds(size_t item); // TODO: maybe microsecond for better resolution?
	size_t GetItemExtendedInfo(size_t item, const wchar_t *metadata, wchar_t *info, size_t infoCch);
	int Reverse(); // optional, return 1 to indicate that you did the reversal (otherwise, caller must perform manually)
	int Swap(size_t item1, size_t item2);
	int Randomize(int (*generator)()); // optional, return 1 to indicate that you did the randomization (otherwise, caller must perform manually)
	void Remove(size_t item);
	int SortByTitle(); // optional, return 1 to indicate that you did the sort (otherwise, caller must perform manually)
	int SortByFilename();// optional, return 1 to indicate that you did the sort (otherwise, caller must perform manually)
#endif
private:
	enum
	{
		DISPATCHABLE_VERSION=0,
	};
};
