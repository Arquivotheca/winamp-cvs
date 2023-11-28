#include "main.h"
#include "MoreItems.h"

static const wchar_t g_noentry[] = L"No Entry";

moreitems::moreitems()
: Next(0), strFile(0), strTitle(0), strCurtain(0),
cbFile(0), cbTitle(0), cbCurtain(0), starttime(0), endtime(0)
{
}

moreitems::~moreitems()
{
	// recursive, find the tail and remove it, work back to head
	delete Next;  Next = NULL;
	delete[] strFile; strFile=NULL;
	delete[] strTitle;
	delete[] strCurtain;
}

const wchar_t *moreitems::GetHiddenFilename(int index)
{
	if (this->index == index)
		return strFile;
	if (Next == NULL)
		return g_noentry;
	return Next->GetHiddenFilename(index);
}

int moreitems::SetRange(int index, unsigned long start, unsigned long end)
{
	if (this->index == index)
  {
     this->starttime = start;
     this->endtime = end;
     return 1;
  }
	if (Next == NULL)
		return 0;
	return Next->SetRange(index,start,end);
}

unsigned long moreitems::GetStart(int index)
{
	if (this->index == index)
  {
     return this->starttime;
  }
	if (Next == NULL)
		return 0;
	return Next->GetStart(index);
}

unsigned long moreitems::GetEnd(int index)
{
	if (this->index == index)
  {
    return this->endtime;
  }
	if (Next == NULL)
		return 0;
	return Next->GetEnd(index);
}

int moreitems::AddHiddenItem(const wchar_t *filename, const wchar_t *title, int length, int index, char *curtain)
{
	// Linked list head
	moreitems *additem = this;
	if (additem && index == 1)
	{
		// List empty
		// Use placeholder
	} 
	else
	{
		// Found items, walk to the end
		while (additem->Next) additem = additem->Next;
		additem->Next = new moreitems;
		additem = additem->Next;
	}
	additem->cbFile = lstrlenW(filename) + 1;
	additem->strFile = new wchar_t[additem->cbFile];
	StringCchCopyW(additem->strFile , additem->cbFile, filename);
	additem->cbTitle = (int)lstrlenW(title) + 1;
	additem->strTitle = new wchar_t[additem->cbFile];
	StringCchCopyW(additem->strTitle, additem->cbTitle, title);
	if ( curtain && *curtain ) 
	{
		additem->cbCurtain = (int)strlen(curtain) + 1;
		additem->strCurtain = new char[additem->cbCurtain];
		StringCchCopy(additem->strCurtain, additem->cbCurtain, curtain);
	}
	else 
	{
		additem->cbCurtain = 0;
		additem->strCurtain = NULL;
	}

	additem->length = length;
	additem->index = index;

	return 1;
}

const char *moreitems::GetHiddenCurtain(int index)
{
	moreitems *where = this;
	while ( where )
	{
		if ( where->index == index && where->cbCurtain ) return where->strCurtain;
		where = where->Next;
	}
	return NULL;
}
