#include "CloudDB.h"
#include "foundation/error.h"

int Cloud_DBConnection::PrepareStatement(sqlite3_stmt *&statement, const char *sql, size_t sql_cch)
{
	if (statement)
	{
		sqlite3_reset(statement);
		return NErr_Success;
	}
	
	return sqlite3_prepare_v2(database_connection, sql, sql_cch, &statement, 0);		
}


int Cloud_DBConnection::Step(sqlite3_stmt *&statement, const char *sql, size_t sql_cch)
{
	int sqlite_ret=PrepareStatement(statement, sql, sql_cch);
	if (sqlite_ret != SQLITE_OK)
		return sqlite_ret;

	sqlite_ret =  sqlite3_step(statement);	
	if (sqlite_ret == SQLITE_BUSY)
	{
		sqlite_ret=sqlite_ret;
	}
	return sqlite_ret;
}
