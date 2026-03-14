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

DatabaseAccess::~DatabaseAccess()
{

}

bool DatabaseAccess::open()
{

}
void DatabaseAccess::close()
{

}
void DatabaseAccess::clear()
{

}