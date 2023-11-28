#ifndef _UNDO_H_
#define _UNDO_H_

class C_UndoStack;

class C_UndoItem
{
	friend C_UndoStack;
	public:
	    C_UndoItem();
		~C_UndoItem();
		C_UndoItem(const C_UndoItem& T);
		C_UndoItem(void *data, int length, bool isdirty);

		C_UndoItem & operator = (const C_UndoItem& T);
		bool operator == (const C_UndoItem& T) const;

		void set(void *data, int length, bool isdirty);
		void *get() const { return data; }
		int size() const { return length; }

	private:
		void *data;
		int length;
		bool isdirty;
};

class C_UndoStack
{
	public:
		static void saveundo(int is2=0);
		static void cleardirty();
		static bool isdirty();

		static void undo();
		static void redo();

		static int can_undo();
		static int can_redo();

		static void clear();

	private:
	    // sorry to do this mig, but that doubly linked lists made me scared. I think it
	    // wasn't actually the source of my bug (I later fixed it), but doubly linked lists
		// are just plain hard to get right. :)
		static int list_pos;
		static C_UndoItem *list[256]; // only keep 256 elements in list at a time
};

#endif//_UNDO_H_