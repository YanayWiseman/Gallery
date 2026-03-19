#include "DatabaseAccess.h"

// PART 1

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

// PART 2

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
	std::string addUser = "INSERT INTO USER (id, name) VALUES (" + std::string(user.getId()) + ", '" + std::string(user.getName()) + "');";
	
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
	// deleting a user is more then just deleting the user from USERS
	// we need to take care of the foreign keys in the rest of the tables
	// that the user has records in
	// we need to start from TAGS then go to pictures -> albums and finally users
	// "counter clockwise" or in the opposite direction of the primary keys

	std::string userId = std::to_string(user.getId());

	// all or nothing
	executeSqlQuery("BEGIN TRANSACTION;");

	// delete from tags
	std::string deleteFromTags = "DELETE FROM TAGS WHERE user_id = " + std::string(user.getId()) + ";";

	// delete from pictures, this is a complicated query because we need to create another query in order
	// to get the albums of the user
	// also, since the SELECT query might return a list, we use the IN clause for an iterative deletion
	std::string deleteFromPictures = "DELETE FROM PICTURES WHERE album_id IN (SELECT id FROM ALBUMS WHERE user_id = " + std::string(user.getId()) + ");";

	// delete from albums
	std::string deleteFromAlbums = "DELETE FROM ALBUMS WHERE user_id = " + std::string(user.getId()) + ";";

	// finally delete from users
	std::string deleteFromUsers = "DELETE FROM USERS WHERE id = " +std::string(user.getId()) + ";";

	// execute deletion in order using a vector
	std::vector<std::string> queries = { deleteFromTags, deleteFromPictures, deleteFromAlbums, deleteFromUsers };
	bool executed = true;

	// loop through the vector
	for (const auto& query : queries)
	{
		// if even one of them fail, retrieve
		if (!executeSqlQuery(currQuery))
		{
			executed = false;
			break;
		}
	}

	// if everything worked perfectly
	if (executed)
	{
		// save and close transaction
		executeSqlQuery("COMMIT;");
	}
	else
	{
		// go back
		executeSqlQuery("ROLLBACK;");
		std::cout << "Delete User failed!" << std::endl;
	}
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

// PART 3

// albums related

/*
	Method to get the list of all of the albums
	input: no input values
	output: the list of albums
*/

const std::list<Album> DatabaseAccess::getAlbums()
{
	// clear the map if anything is left in there
	// we want to fill it from the beggining and not have any left overs
	this->m_albums.clear();

	// we are going to be using prepared statements, this is what will contain the result of out query
	// and we will be using sqlite3_step() in oreder to take out a one record at once
	sqlite3_stmt* stmt = nullptr;

	// prepare the statement
	int res = sqlite3_prepare_v2(this->m_db, "SELECT ALBUMS.id, ALBUMS.name, ALBUMS.creation_date, ALBUMS.user_id, USERS.name FROM ALBUMS INNER JOIN USERS ON ALBUMS.user_id = USERS.id;", -1, &stmt, nullptr);
	
	// check for failure
	if (res != SQLITE_OK)
	{
		std::cout << "Error: " << sqlite3_errmsg(this->m_db);
		return {};  // return an empty list
	}

	// iterate through the records returned
	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		// quick explanation about how i am extracting the records here
		// the columnsa re sorted by 0 based index in the oreder i ahve selected in
		// the SELECT quert above, the second paramerter of the sqlite3_column_sometype() is the index

		// here we are getting the album id
		int id = sqlite3_column_int(stmt, 0);
	
		// sqlite3_column_text() returns a const unsigned char* so we need
		// to convert in oreder to create an album instance
		std::string albumName = colText(stmt, 1);
		std::string creationDate = colText(stmt, 2);
		int ownerId = sqlite3_column_int(stmt, 3);
		std::string ownerName = colText(stmt, 4);  // from USERS (name)

		Album album(ownerId, albumName);
		album.setCreationDate(creationDate);

		// insert into the data
		this->m_albums[albumName] = album;
		std::cout << std::setw(5) << "* " << album;
	}

	// finish the statement
	sqlite3_finalize(stmt);

	// we are returning a list, not a map so lets convert
	std::list<Album> albumsResult;
	for (const auth& pair : this->m_albums)
	{
		albumsResult.push_back(pair.second);
	}

	return albumsResult;
}

///*
//	Helper static method to extract one record from a given sqlite3_stmt
//	input: the sqlite statement
//	output: the album
//*/
//
//static const Album& DatabaseAccess::albumFromRow(sqlite3_stmt* stmt)
//{
//	// teh columns are oriented in the order we have selected in the query
//
//	int id = sqlite3_column_int(stmt, 0);
//	
//}

/*
	Helper static method to convert the const unsigned char* that sqlite3_colums_text() returns
	input: the statement and the column to convert
	output: the converted string
*/

static const std::string& DatabaseAccess::colText(sqlite3_stmt* stmt, int col)
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

*/

const std::list<Album> DatabaseAccess::getAlbumsOfUser(const User& user)
{

}

/*

*/

void DatabaseAccess::createAlbum(const Album& album)
{

}

/*

*/

void DatabaseAccess::deleteAlbum(const std::string& albumName, int userId)
{

}

/*

*/

bool DatabaseAccess::doesAlbumExists(const std::string& albumName, int userId)
{

}

/*

*/

Album DatabaseAccess::openAlbum(const std::string& albumName)
{

}

/*

*/

void DatabaseAccess::closeAlbum(Album& pAlbum)
{

}

/*

*/

void DatabaseAccess::printAlbums()
{

}

// picture related

/*

*/

void DatabaseAccess::addPictureToAlbumByName(const std::string& albumName, const Picture& picture)
{

}

/*

*/

void DatabaseAccess::removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName)
{

}

// users related

/*

*/

void DatabaseAccess::printUsers()
{

}
User DatabaseAccess::getUser(int userId)
{

}

/*

*/

bool DatabaseAccess::doesUserExists(int userId)
{

}

// statistics

/*

*/

int DatabaseAccess::countAlbumsOwnedOfUser(const User& user)
{

}

/*

*/

int DatabaseAccess::countAlbumsTaggedOfUser(const User& user)
{

}

/*

*/

int DatabaseAccess::countTagsOfUser(const User& user)
{

}

/*

*/

float DatabaseAccess::averageTagsPerAlbumOfUser(const User& user)
{

}

// queries

/*

*/

User DatabaseAccess::getTopTaggedUser()
{

}

/*

*/

Picture DatabaseAccess::getTopTaggedPicture()
{

}

/*

*/

std::list<Picture> DatabaseAccess::getTaggedPicturesOfUser(const User& user)
{

}