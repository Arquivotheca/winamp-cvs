#include "./groupedList.h"


GLItem::GLItem(INT itemId, LPCTSTR pszTitle, UINT itemFlags) :
	ref(1), parent(NULL), id(itemId), pszText(NULL), flags(itemFlags)
{
	SetTitle(pszTitle);
}

GLItem::~GLItem()
{
	if (NULL != pszText)
		free(pszText);
}

ULONG GLItem::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG GLItem::Release(void)
{
	if (0 == ref)
		return ref;

	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

GLRoot *GLItem::GetRoot()
{
	for (GLItem *item = this; NULL != item; item = item->parent)
	{
		if (GLGroup::FlagTypeGroup == item->GetType() && NULL == item->parent)
			return (GLRoot*)item;
	}
	return NULL;
}

void GLItem::SetFlags(UINT newFlags, UINT flagsMask)
{  	
	UINT oldFlags = flags;
	flags = (flags & ~flagsMask) | (newFlags & flagsMask); 

	if (oldFlags != flags)
	{
		GLRoot *root = GetRoot();
		if (NULL != root)
			 root->NotifyStyleChanged(this, oldFlags, flags);
	}
}

INT GLItem::GetLevel()
{	
	GLItem *item = this->parent;
	if (NULL == item)
		return 0;

	INT level;
	for (level = 0; NULL != item->parent; item = item->parent, level++);
	return level;
}

UINT GLItem::GetType()
{
	return (GLItem::FlagTypeMask & flags);
}
UINT GLItem::GetValue()
{
	return (GLItem::FlagValueMask & flags);
}
UINT GLItem::GetState()
{
	return (GLItem::FlagStateMask & flags);
}

GLItem *GLItem::Next()
{
	if (NULL == parent || (GLGroup::FlagTypeGroup != parent->GetType())) 
		return NULL;

	GLItem *next = ((GLGroup*)parent)->NextChild(this);
	
	if (NULL == next) 
		next = parent->Next();
		
	return next;
}

GLItem *GLItem::Previous()
{
	if (NULL == parent || (GLGroup::FlagTypeGroup != parent->GetType())) 
		return NULL;

	GLItem *previous = ((GLGroup*)parent)->PreviousChild(this);
	
	if (NULL == previous && NULL != parent->parent) // ignore root
		previous = parent;
		
	return previous;
}

HRESULT GLItem::SetTitle(LPCTSTR pszTitle)
{
	if (NULL != pszText)
	{
		free(pszText);
		pszText = NULL;
	}

	if (NULL != pszTitle)
	{
		INT cchText = lstrlen(pszTitle);
	
		pszText = (LPTSTR)malloc(sizeof(TCHAR) * (cchText + 1));
		if (NULL == pszText)
			return E_OUTOFMEMORY;
		
		CopyMemory(pszText, pszTitle, sizeof(TCHAR) * cchText);
		pszText[cchText] = TEXT('\0');
	}

	return S_OK;
}