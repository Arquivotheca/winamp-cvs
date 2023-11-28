#pragma once
/*
Android (linux) implementation
*/
#include <stdio.h>
#include "Record.h"
#include <nu/PtrDeque2.h>
class IndexField;
class IndexRecord : public RecordBase
{
public:
	IndexRecord(int RecordPos, int insertionPoint, VFILE *FileHandle, Table *p);
	void BuildCollaboration();
	bool NeedFix();
	IndexField *GetIndexByName(const char *name);
	int GetColumnCount() { return Fields.size() - 1; }
	bool CheckIndexing(int v);
	int WriteFields(Table *ParentTable);
	int WriteIndex(Table *ParentTable);
	typedef bool (*FieldsWalker)(IndexRecord *record, Field *entry, void *context);
	void WalkFields(FieldsWalker callback, void *context);
};

