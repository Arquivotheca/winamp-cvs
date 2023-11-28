#include "./main.h"
#include "./itemViewManager.h"
#include "./simpleView.h"
#include "./detailsView.h"


static DropboxViewManager viewManager;
extern DropboxViewManager *pluginViewManager = &viewManager;

static DropboxViewMeta* szRegisteredViews[] = 
{
	simpleViewMeta, 
	detailsViewMeta,
};


DropboxViewMeta *DropboxViewManager::FindById(INT viewId)
{
	for (size_t i = 0; i < ARRAYSIZE(szRegisteredViews); i++)
	{
		if (szRegisteredViews[i]->GetId() == viewId)
			return szRegisteredViews[i];
	}
	return NULL;
}

DropboxViewMeta *DropboxViewManager::FindByName(LPCTSTR viewName)
{
	INT cchName = (NULL != viewName) ? lstrlen(viewName) : 0;
	if (0 == cchName) return NULL;

	for (size_t i = 0; i < ARRAYSIZE(szRegisteredViews); i++)
	{
		if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, viewName, cchName, szRegisteredViews[i]->GetName(), -1))
			return szRegisteredViews[i];
	}
	return NULL;
}

DropboxViewMeta *DropboxViewManager::First()
{
	return (0 != ARRAYSIZE(szRegisteredViews)) ? szRegisteredViews[0] : NULL;
}

BOOL DropboxViewManager::EnumerateViews(EnumProc proc, ULONG_PTR param)
{
	if (NULL == proc)
		return FALSE;

	for (size_t i = 0; i < ARRAYSIZE(szRegisteredViews); i++)
	{
		if (!proc(szRegisteredViews[i], param))
			return FALSE;
	}
	return TRUE;
}
