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

/*
	Method to close the database
	input: no input values
	output: no return values
*/

void DatabaseAccess::close()
{
	// if the database is open
	if (this->m_db != nullptr)
	{
		sqlite3_close(this->m_db);
		this->m_db = nullptr;
	}
}

/*
	Method to clear all of the data from the database object (free allocated memory)
	input: no input values
	output: no return values
*/

void DatabaseAccess::clear()
{
	this->m_users.clear();
	this->m_albums.clear();
	this->m_pictures.clear();
}

/*
	Method to tag a user in a picture
	input: the album name and picture name to tag to and the userId  to tag
	output: no return values
*/

void tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{

}

/*
	Method to untag a user from a picture
	input: the album name and picture name to untag from and the userId to untag
	output: no return values
*/

void untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{

}

/*
	Method to create a user given a user object
	input: the user object to add (contains name and id)
	output: no return values
*/

void createUser(User& user)
{

}

/*
	Method to delete a user given a user object
	input: the user object to remove
	output: no return values
*/

void deleteUser(const User& user)
{

}