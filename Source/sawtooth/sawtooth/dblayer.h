#ifndef db_layer_h
#define db_layer_h

#include "sqlite3.h"
#include "sawtoothexception.h"
#include <string>
#include <map>
namespace Sawtooth {
	class DBConnection {
	private:
		sqlite3 *db;
	public:
		DBConnection(std::string path) {
			if (path.length() == 0) {
				throw SawtoothException(Sawtooth_DBOpenError, "zero length path");
			}
			if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
				throw SawtoothException(Sawtooth_DBOpenError, "sqlite_open error");
			}
		}
		~DBConnection() {
			sqlite3_close(db);
		}

		sqlite3_stmt* prepare(std::string sql) {
			sqlite3_stmt* stmt;
			int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
			if (rc != SQLITE_OK) {
				throw SawtoothException(Sawtooth_DBQueryError, "sqlite_prepare_v2 error");
			}
			return stmt;
		}
	};

	class Cursor {
	private:
		sqlite3_stmt* _stmt;
		std::map<std::string, int> columnMap;
		int GetColIdx(std::string colname) {
			auto idx = columnMap.find(colname);
			if (idx == columnMap.end()) {
				throw SawtoothException(Sawtooth_DBQueryError, "specified column name not found");
			}
			return idx->second;
		}
	public:
		Cursor(sqlite3_stmt* stmt) {
			_stmt = stmt;
			int ctotal = sqlite3_column_count(stmt);
			for (int i = 0; i < ctotal; i++) {
				const char* colname = sqlite3_column_name(_stmt, i);
				std::string _colname = std::string(colname);
				if (columnMap.find(_colname) != columnMap.end()) {
					throw SawtoothException(Sawtooth_DBQueryError, "duplicate column detected in query");
				}
				columnMap[_colname] = i;
			}
		}
		~Cursor() {
			sqlite3_finalize(_stmt);
		}
		int MoveNext() {
			int rc = sqlite3_step(_stmt);
			if (rc == SQLITE_DONE)
				return false;
			if (rc != SQLITE_ROW) {
				throw SawtoothException(Sawtooth_DBQueryError, "db cursor move next error");
			}
			return true;
		}

		bool IsNull(std::string colname) {
			return sqlite3_column_type(_stmt, GetColIdx(colname)) == SQLITE_NULL;
		}

		std::string GetValueString(std::string colname) {
			return std::string(reinterpret_cast<const char*>(
				sqlite3_column_text(_stmt, GetColIdx(colname))));
		}
		int32_t GetValueInt32(std::string colname) {
			return sqlite3_column_int(_stmt, GetColIdx(colname));
		}
		int64_t GetValueInt64(std::string colname) {
			return sqlite3_column_int64(_stmt, GetColIdx(colname));
		}

		double GetValueDouble(std::string colname) {
			return sqlite3_column_double(_stmt, GetColIdx(colname));
		}
	};
}
#endif