#pragma once
#include <sql.h>
#include <sqlext.h>


enum
{
	WVARCHAR_MAX = 4000,
	BINARY_MAX = 8000,
};

class DBConnection
{
public:
	bool Connect(SQLHENV henv, const WCHAR* connectionString);
	void Clear();

	bool Execute(const WCHAR* query);
	bool Fetch();
	int32 GetRowCount();
	void Unbind();

public:
	bool BindParam(SQLUSMALLINT paramIndex, bool* value, SQLLEN* index);
	bool BindParam(SQLUSMALLINT paramIndex, float* value, SQLLEN* index);
	bool BindParam(SQLUSMALLINT paramIndex, double* value, SQLLEN* index);
	bool BindParam(SQLUSMALLINT paramIndex, int8* value, SQLLEN* index);
	bool BindParam(SQLUSMALLINT paramIndex, int16* value, SQLLEN* index);
	bool BindParam(SQLUSMALLINT paramIndex, int32* value, SQLLEN* index);
	bool BindParam(SQLUSMALLINT paramIndex, int64* value, SQLLEN* index);
	bool BindParam(SQLUSMALLINT paramIndex, TIMESTAMP_STRUCT* value, SQLLEN* index);
	bool BindParam(SQLUSMALLINT paramIndex, const WCHAR* str, SQLLEN* index);
	bool BindParam(SQLUSMALLINT paramIndex, const BYTE* bin, int32 size, SQLLEN* index);

	bool BindCol(SQLUSMALLINT columnIndex, bool* value, SQLLEN* index);
	bool BindCol(SQLUSMALLINT columnIndex, float* value, SQLLEN* index);
	bool BindCol(SQLUSMALLINT columnIndex, double* value, SQLLEN* index);
	bool BindCol(SQLUSMALLINT columnIndex, int8* value, SQLLEN* index);
	bool BindCol(SQLUSMALLINT columnIndex, int16* value, SQLLEN* index);
	bool BindCol(SQLUSMALLINT columnIndex, int32* value, SQLLEN* index);
	bool BindCol(SQLUSMALLINT columnIndex, int64* value, SQLLEN* index);
	bool BindCol(SQLUSMALLINT columnIndex, TIMESTAMP_STRUCT* value, SQLLEN* index);
	bool BindCol(SQLUSMALLINT columnIndex, WCHAR* str, int32 size, SQLLEN* index);
	bool BindCol(SQLUSMALLINT columnIndex, BYTE* bin, int32 size, SQLLEN* index);

private:
	bool BindParam(SQLUSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, SQLPOINTER ptr, SQLLEN* index);
	bool BindCol(SQLUSMALLINT columnIndex, SQLSMALLINT cType, SQLULEN len, SQLPOINTER value, SQLLEN* index);
	void HandleError(SQLRETURN ret);

private:
	SQLHDBC _connection = SQL_NULL_HANDLE;
	SQLHSTMT _statement = SQL_NULL_HANDLE;
};