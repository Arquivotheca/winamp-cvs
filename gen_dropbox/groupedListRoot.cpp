#include "./groupedList.h"

GLRoot::GLRoot() : GLGroup(0, -1, NULL, 0), callback(NULL) 
{
}

GLRoot::~GLRoot()
{
}

GLRoot *GLRoot::Create() 
{ 
	return new GLRoot(); 
}

void GLRoot::RegisterCallback(GLCallback *callback) 
{ 
	this->callback = callback; 
}

BOOL GLRoot::Draw(GLStyle *style, HDC hdc, const RECT *prcItem, UINT drawFlags) 
{ 
	return FALSE;
}

INT GLRoot::GetHeight(GLStyle *style)
{
	return 0;
}

BOOL GLRoot::AdjustRect(GLStyle *style, RECT *prcItem)
{
	if (NULL == prcItem)
		return FALSE;
	prcItem->right = prcItem->left;
	prcItem->bottom = prcItem->top;
	return TRUE;
}

	
GLItem *GLRoot::FindItem(INT flatIndex)
{
	GLItem *item;
	size_t base = 0;
	return (LookupItem(flatIndex, &base, &item)) ? item : NULL;
}

INT GLRoot::FindIndex(const GLItem *item)
{
	size_t index = 0;
	return (NULL != item && FALSE != LookupIndex(item, &index)) ? (INT)index : LB_ERR;
}

void GLRoot::MouseEnter(GLView *view)
{
}
void GLRoot::MouseLeave(GLView *view)
{
}

void GLRoot::MouseMove(GLView *view, const RECT *prcItem, POINT pt)
{
}

BOOL GLRoot::LButtonDown(GLView *view, const RECT *prcItem, POINT pt)
{
	return TRUE;
}
void GLRoot::LButtonUp(GLView *view, const RECT *prcItem, POINT pt)
{
}
void GLRoot::LButtonClick(GLView *view)
{
}
void GLRoot::KeyDown(GLView *view, UINT vKey)
{
}

void GLRoot::KeyUp(GLView *view, UINT vKey)
{
}

void GLRoot::KeyPressed(GLView *view, UINT vKey)
{
}

void GLRoot::StyleChanged(GLView *view, UINT updateFlags)
{
}

void GLRoot::NotifyStyleChanged(GLItem *item, UINT styleOld, UINT styleNew)
{
	if (NULL != callback && NULL != item)
		callback->ItemStyleChanged(item, styleOld, styleNew);
}