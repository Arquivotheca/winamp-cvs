#include <windows.h>
#include "undo.h"
#include "render.h"

int C_UndoStack::list_pos;
C_UndoItem *C_UndoStack::list[256];

C_UndoItem::C_UndoItem() : data(NULL), length(0), isdirty(true)
{
}

C_UndoItem::C_UndoItem(const C_UndoItem& T) : data(NULL), length(0), isdirty(true)
{
	*this = T;
}

C_UndoItem::C_UndoItem(void *_data, int _length, bool _isdirty) : data(NULL), length(length), isdirty(_isdirty)
{
	data = GlobalAlloc(GPTR, length);
	memcpy(data, _data, length);
}

C_UndoItem::~C_UndoItem()
{
	if (data) 
	{
		GlobalFree(data);
	}
}

C_UndoItem & C_UndoItem::operator = (const C_UndoItem& T)
{
	length = T.length;
	isdirty = T.isdirty;
	if (data) 
	{
		GlobalFree(data);
	}
	data = GlobalAlloc(GPTR, length);
	memcpy(data, T.data, length);
	return *this;
}

bool C_UndoItem::operator == (const C_UndoItem& T) const
{
	bool retval = false;
	if (length == T.length)
	{
		retval = (memcmp(data, T.data, length) == 0);
	}
	return retval;
}

void C_UndoItem::set(void *_data, int _length, bool _isdirty)
{
	length = _length;
	isdirty = _isdirty;
	if (data) 
	{
		GlobalFree(data);
	}
	data = GlobalAlloc(GPTR, length);
	memcpy(data, _data, length);
}

void C_UndoStack::saveundo(int is2)
{
	// Save to the undo buffer (sets the dirty bit for this item)
	C_UndoItem *item = new C_UndoItem;
	C_UndoItem *old = list[list_pos];

	if (is2) 
	{
		g_render_effects2->__SavePresetToUndo(*item);
	}
	else 
	{
		g_render_effects->__SavePresetToUndo(*item);
	}

	// Only add it to the stack if it has changed.
	if (!old || !(*old == *item))
	{
	    if (list_pos == sizeof(list)/sizeof(list[0])-1)
		{
			delete list[0];
			memcpy(list,list+1,sizeof(list)/sizeof(list[0])-1);
			list_pos--;
		}
		list[++list_pos]=item;
	}
	else delete item;
}

void C_UndoStack::cleardirty()
{
	// If we're clearing the dirty bit, we only clear it on the current item.
	if (list_pos >= 0 && list_pos < sizeof(list)/sizeof(list[0]) && list[list_pos])
	{
	    list[list_pos]->isdirty = 0;
	}
}

bool C_UndoStack::isdirty()
{
	if (list_pos >= 0 && list_pos < sizeof(list)/sizeof(list[0]) && list[list_pos])
		return list[list_pos]->isdirty;
	return false;
}

int C_UndoStack::can_undo()
{
	return (list_pos>0 && list[list_pos-1]);
}

int C_UndoStack::can_redo()
{
	return list_pos < sizeof(list)/sizeof(list[0])-1 && list[list_pos+1];
}

void C_UndoStack::undo()
{
	if (list_pos>0 && list[list_pos-1])
	{
		g_render_transition->LoadPreset(NULL,0,list[--list_pos]);
	}
}

void C_UndoStack::redo()
{
	if (list_pos < sizeof(list)/sizeof(list[0])-1 && list[list_pos+1])
	{
	    g_render_transition->LoadPreset(NULL,0,list[++list_pos]);
	}
}

void C_UndoStack::clear()
{
	list_pos=0;
	for (int x = 0; x < sizeof(list)/sizeof(list[0]); x ++)
	{
		delete list[x];
		list[x]=0;
	}
}