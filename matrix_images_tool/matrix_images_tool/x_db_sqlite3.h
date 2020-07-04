#ifndef _X_DB_SQLITE3_H_
#define _X_DB_SQLITE3_H_

#include <string>
#include <vector>

extern "C"
{
#include <sqlite3.h>
}

#include "x_common.h"

namespace xM
{
    namespace db
    {
		typedef std::string xSqlite3ResultCell;
		typedef std::vector<xSqlite3ResultCell> xSqlite3ResultRow;
		typedef std::vector<xSqlite3ResultRow> xSqlite3Result;

#define XM_DB_SQLITYE3_RESULT_COL_NAME_INDEX 0
#define XM_DB_SQLITYE3_RESULT_ROW_FRIST_INDEX 1

		class xSqlite3
		{
		private:
			sqlite3* ctx_;

		public:
			xSqlite3();
			~xSqlite3();

		public:
			bool Open(const char* _connet);
			void Close();
			bool ExecuteNonQuery(const char* _sql);

			bool Query(const char* _sql, xSqlite3Result& _result);
		};
    }
}

#endif // !_X_DB_SQLITE3_H_