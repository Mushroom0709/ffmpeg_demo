#include "x_db_sqlite3.h"

namespace xM
{
	namespace db
	{
		xSqlite3::xSqlite3()
		{
			ctx_ = NULL;
		}
		xSqlite3::~xSqlite3()
		{
			Close();
		}

		bool xSqlite3::Open(const char* _connet)
		{
			if (ctx_ != NULL)
				return false;

			if (SQLITE_OK != sqlite3_open(_connet, &ctx_))
			{
				xErrorPrintln("[xSqlite3] [Open fail:%s]", sqlite3_errmsg(ctx_));
				return false;
			}

			return true;
		}
		void xSqlite3::Close()
		{
			if (ctx_ != NULL)
			{
				sqlite3_close(ctx_);
				ctx_ = NULL;
			}
		}
		bool xSqlite3::ExecuteNonQuery(const char* _sql)
		{
			if (ctx_ == NULL)
				return false;

			char* err_msg = NULL;
			if (SQLITE_OK != sqlite3_exec(ctx_, _sql, NULL, NULL, &err_msg))
			{
				xErrorPrintln("[xSqlite3] [ExecuteNonQuery fail:%s] [SQL:%s]", err_msg, _sql);
				return false;
			}

			return true;
		}

		bool xSqlite3::Query(const char* _sql, xSqlite3Result& _result)
		{
			if (ctx_ == NULL)
				return false;

			char* err_msg = NULL;
			char** res = NULL;
			int rows = 0;
			int cols = 0;

			if (SQLITE_OK != sqlite3_get_table(ctx_, _sql, &res, &rows, &cols, &err_msg))
			{
				xErrorPrintln("[xSqlite3] [ExecuteNonQuery fail:%s] [SQL:%s]", err_msg, _sql);
				sqlite3_free_table(res);
				return false;
			}

			_result.clear();

			//组建列标头
			{
				xSqlite3ResultRow row;
				for (size_t i = 0; i < cols; i++)
				{
					row.push_back(res[i]);
				}
				_result.push_back(row);
			}

			//组建行
			for (size_t i = 0; i < rows; i++)
			{
				xSqlite3ResultRow row;
				for (size_t j = 0; j < cols; j++)
				{
					row.push_back(res[cols * (i + 1) + j]);
				}
				_result.push_back(row);
			}

			return true;
		}
	}
}