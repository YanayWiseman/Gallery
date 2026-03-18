#include "DatabaseAccess.h"

/*
	Constructor for the DatabaseAccess class, to contsruct the object
	input: no input values
	output: no return values
*/

DatabaseAccess::DatabaseAccess()
{
	this->m_db = nullptr;
	this->m_dbFile = DATABASE_NAME;  // file name of the database
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

void DatabaseAccess::tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	// we got the album's name, the picture's name and the user's id
	// to insert into the tags table we need the picture's id and the user id (which we already got)

	// we need to make an sql query to get the picture's id using the picture's name and its album's name
	// and integrate it into an INSERT query
	std::string getPictureId = "(SELECT PICTURES.id FROM PICTURES INNER JOIN ALBUMS ON ALBUMS.id = PICTURES.album_id WHERE PICTURES.name = '" + pictureName + "' AND ALBUMS.name = '" + albumName + "')";
	std::string tagUserInPicture = "INSERT INTO TAGS (picture_id, user_id) VALUES (" + getPictureId + ", " + std::to_string(userId) + ");";
	
	bool executed = executeSqlQuery(tagUserInPicture);
	if (!executed)
	{
		std::cout << "Tagging user in picture failed!" << std::endl;
	}
}

/*
	Method to untag a user from a picture
	input: the album name and picture name to untag from and the userId to untag
	output: no return values
*/

void DatabaseAccess::untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	// here too, we nede the picture id by the album name and picture's name.
	std::string getPictureId = "(SELECT PICTURES.id FROM PICTURES INNER JOIN ALBUMS ON ALBUMS.id = PICTURES.album_id WHERE PICTURES.name = '" + pictureName + "' AND ALBUMS.name = '" + albumName + "')";
	
	// but now we want to remove it
	std::string unTagUser = "DELETE FROM TAGS WHERE TAGS.picture_id = " + getPictureId + " AND TAGS.user_id = " + std::to_string(userId) + ";";

	bool executed = executeSqlQuery(unTagUser);
	if (!executed)
	{
		std::cout << "Untagging user in picture failed!" << std::endl;
	}
}

/*
	Method to create a user given a user object
	input: the user object to add (contains name and id)
	output: no return values
*/

void DatabaseAccess::createUser(User& user)
{
	// we need to create an insert statement that creates a user into users with
	// the given user's name and id even though id might be autoincremented
	std::string addUser = "INSERT INTO USER (id, name) VALUES (" + user.getId() + ", '" + user.getName() + "';";
	
	bool executed = executeSqlQuery(addUser);
	if (!executed)
	{
		std::cout << "Creating user failed!" << std::endl;
	}
}

/*
	Method to delete a user given a user object
	input: the user object to remove
	output: no return values
*/

void DatabaseAccess::deleteUser(const User& user)
{

}

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