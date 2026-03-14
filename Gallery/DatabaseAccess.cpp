#include "DatabaseAccess.h"

/*
	Constructor for the DatabaseAccess class, to contsruct the object
	input: no input values
	output: no return values
*/

DatabaseAccess::DatabaseAccess()
{
	this->m_db = nullptr;
	this->m_dbFile = "YanayGallery.sqlite";  // file name of the database
}

/*
	Destructor for the DatabaseAccess class, to destruct the object
	input: no input values
	output: no return values
*/

DatabaseAccess::~DatabaseAccess()
{
	close();
}

/*
	Method to open the database
	input: no input values
	output: bool, true if opened and false otherwise
*/

bool DatabaseAccess::open()
{
	// open the data base
	int res = sqlite3_open(this->m_dbFile.c_str(), &this->m_db);

	// check if opened
	if (res != SQLITE_OK)
	{
		this->m_db = nullptr;
		return false;
	}
	return true;
}

void DatabaseAccess::close()
{

}
void DatabaseAccess::clear()
{

}