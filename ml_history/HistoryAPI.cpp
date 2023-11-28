#include "HistoryAPI.h"
#include "ml_history.h"

static void saveQueryToList(nde_scanner_t s, historyRecordList *obj)
{
	emptyRecentRecordList(obj);

	NDE_Scanner_First(s);

	int r;
	do
	{
		nde_field_t f = NDE_Scanner_GetFieldByID(s, HISTORYVIEW_COL_FILENAME);
		if (f)
		{
			allocRecentRecordList(obj, obj->Size + 1);
			if (!obj->Alloc) break;

			wchar_t *strval =  NDE_StringField_GetString(f);
			ndestring_retain(strval);
			obj->Items[obj->Size].filename = strval;
			recentScannerRefToObjCacheNFN(s, obj);
		}

		r = NDE_Scanner_Next(s);
	}
	while (r && !NDE_Scanner_EOF(s));

	if (obj->Size && obj->Size < obj->Alloc - 1024)
	{
		obj->Alloc = obj->Size;
		obj->Items = (historyRecord*)realloc(obj->Items, sizeof(historyRecord) * obj->Alloc);
	}
}

historyRecordList *HistoryAPI::Query(const wchar_t *query)
{
	if (!openDb())
		return 0;

	// run query
	EnterCriticalSection(&g_db_cs);
	nde_scanner_t s=NDE_Table_CreateScanner(g_table);
	NDE_Scanner_Query(s, query);

	historyRecordList obj;

	obj.Alloc = 0;
	obj.Items = NULL;
	obj.Size = 0;
	saveQueryToList(s, &obj);
	NDE_Table_DestroyScanner(g_table, s);
	LeaveCriticalSection(&g_db_cs);
	if (obj.Size)
	{
		historyRecordList *result = (historyRecordList *)malloc(sizeof(historyRecordList));
		memcpy(result, &obj, sizeof(historyRecordList));
		return result;
	}
	else
	{
		freeRecentRecordList(&obj);
		return 0;
	}
}

void HistoryAPI::FreeHistoryList(historyRecordList *historyList)
{
	freeRecentRecordList(historyList);
}

#define CBCLASS HistoryAPI
START_DISPATCH;
CB(API_HISTORY_QUERY, Query)
VCB(API_HISTORY_FREEHISTORYLIST, FreeHistoryList)
END_DISPATCH;
#undef CBCLASS
