#include "DatabaseAccess.h"

/*
	Helper method to exectue a given sql query statement to avoid code duplication and implement the encapsulation pricple
	input: the sql statement as a string
	output: true if statement succeeded and false otherwise (according to SQLITE_OK)
*/

bool DatabaseAccess::executeSqlQuery(const std::string& statement) const
{
	char* errorMsg = nullptr;

	int res = sqlite3_exec(this->m_db, statement.c_str(), nullptr, nullptr, &errorMsg);
	if (res != SQLITE_OK)
	{
		std::cout << "Error:" << errorMsg << std::endl;

		// if sqlite3 used errorMsg we need to free the memory using their functions
		if (errorMsg)
		{
			sqlite3_free(errorMsg);
		}
		return false;
	}

	std::cout << statement << std::endl;   // for debugging purposes
	return true;
}

/*
	Helper static method to convert the const unsigned char* that sqlite3_colums_text() returns
	input: the statement and the column to convert
	output: the converted string
*/

const std::string& DatabaseAccess::colText(sqlite3_stmt* stmt, int col)
{
	// extract it from the records
	const unsigned char* text = sqlite3_column_text(stmt, col);

	if (text)
	{
		// first of all, sqlite3_column_bytes returns the number of bytes in the text column
		// and this string constructor copies the first text + sqlite3_column_bytes(stmt, col)
		// from into the text "array"
		return std::string(text, text + sqlite3_column_bytes(stmt, col));
	}
	else
	{
		return std::string();
	}
}

/*
	Helper method too reduce code duplication in the method deleteAlbum()
	input: the album name to delete and its owner's id
	output: no return values
*/

void DatabaseAccess::deleteAlbumBind(sqlite3_stmt* stmt, const std::string& albumName, int userId) const
{
	sqlite3_bind_text(stmt, 1, albumName.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 2, userId);
}
