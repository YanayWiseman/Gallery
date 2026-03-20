#include "DatabaseAccess.h"

// PART 1

/*
	Constructor for the DatabaseAccess class, to contsruct the object
	input: no input values
	output: no return values
*/

DatabaseAccess::DatabaseAccess()
	: m_db(nullptr), m_dbFile(DATABASE_NAME)
{
	//  in oreder to execute queries on the database we need to open it
	open();

	// we need to create the tables
	// users
	executeSqlQuery("CREATE TABLE IF NOT EXISTS USERS ("
		"  id INTEGER PRIMARY KEY,"
		"  name TEXT NOT NULL"
		");");
	// albums
	executeSqlQuery("CREATE TABLE IF NOT EXISTS ALBUMS ("
		" id INTEGER PRIMARY KEY AUTOINCREMENT,"
		" name TEXT NOT NULL,"
		" creation_date TEXT NOT NULL,"
		" user_id INTEGER NOT NULL,"
		" FOREIGN KEY(user_id) REFERENCES USERS(id)"
		");");
	// pictures
	executeSqlQuery("CREATE TABLE IF NOT EXISTS PICTURES ("
		" id INTEGER PRIMARY KEY AUTOINCREMENT,"
		" name TEXTNOT NULL,"
		" location TEXT NOT NULL,"
		" creation_date TEXT NOT NULL,"
		" album_id INTEGER NOT NULL,"
		" FOREIGN KEY(album_id) REFERENCES ALBUMS(id)"
		");");
	// tags
	executeSqlQuery("CREATE TABLE IF NOT EXISTS TAGS ("
		" id INTEGER PRIMARY KEY AUTOINCREMENT,"
		" picture_id INTEGER NOT NULL,"
		" user_id INTEGER NOT NULL,"
		" FOREIGN KEY(picture_id) REFERENCES PICTURES(id),"
		" FOREIGN KEY(user_id) REFERENCES USERS(id)"
		");");
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

// HELPERS

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

/*
	Helper method for the statistics part, to avoid code duplication in the functions that have the same pattern
	input: the query and thhe bind value
	output: the count (result of statistics query)
*/

int DatabaseAccess::runCountQuery(const char* query, int bindValue) const
{
	sqlite3_stmt* stmt = prepare(query);
	if (!stmt)
	{
		return 0;
	}
	int count = 0;

	sqlite3_bind_int(stmt, 1, bindValue);
	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		count = sqlite3_column_int(stmt, 0);
	}
	sqlite3_finalize(stmt);

	return count;
}


/*
	Helper method to prepare a given SQL statement
	input: the statement in char form
	output: the sqlite3_stmt*
*/

sqlite3_stmt* DatabaseAccess::prepare(const char* statement) const
{
	sqlite3_stmt* stmt = nullptr;

	// prepare the statement
	int res = sqlite3_prepare_v3(this->m_db, statement, -1, &stmt, nullptr);

	// check for failure
	if (res != SQLITE_OK)
	{
		std::cout << "Error: " << sqlite3_errmsg(this->m_db);
		return nullptr;
	}
	return stmt;
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
	// prepare the statement
	sqlite3_stmt* stmt = prepare("SELECT ALBUMS.id, ALBUMS.name, ALBUMS.creation_date, ALBUMS.user_id, USERS.name FROM ALBUMS INNER JOIN USERS ON ALBUMS.user_id = USERS.id;");
	if (!stmt)
	{
		return {};
	}

	// iterate through the records returned
	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		// quick explanation about how i am extracting the records here
		// the columns a re sorted by 0 based index in the oreder i ahve selected in
		// the SELECT quert above, the second paramerter of the sqlite3_column_sometype() is the index

		// here we are getting the album id
		int id = sqlite3_column_int(stmt, 0);
	
		// sqlite3_column_text() returns a const unsigned char* so we need
		// to convert in oreder to create an album instance
		std::string albumName = DatabaseAccess::colText(stmt, 1);
		std::string creationDate = DatabaseAccess::colText(stmt, 2);
		int ownerId = sqlite3_column_int(stmt, 3);
		std::string ownerName = DatabaseAccess::colText(stmt, 4);  // from USERS (name)

		Album album(ownerId, albumName);
		album.setCreationDate(creationDate);

		// insert into the data
		this->m_albums[albumName] = album;
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

/*
	Method to get the albums of a certain user
	input: the user (object)
	output: the list of albums he owns
*/

const std::list<Album> DatabaseAccess::getAlbumsOfUser(const User& user)
{
	std::cout << "Albums list of user@" << user.getId() << ":" << std::endl;
	std::cout << "-----------------------" << std::endl;

	// the proccess is similar to getAlbums()
	this->m_albums.clear();

	// now we have a WHERE clause because we need to filter the answers
	// we need to use ? in order to input the user id
	sqlite3_stmt* stmt = prepare("SELECT name, creation_date FROM ALBUMS WHERE user_id = ?;");

	if (!stmt)
	{
		return {};
	}

	// now we need to bind the user_id into the statement
	// since sqlite3_bind is 1-based counted (unlike sqlite3_column), we have only one question mark
	// so we bind to 1 the user id
	sqlite3_bind_int(stmt, 1, user.getId());

	std::list<Album> albumListByUser;

	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		std::string albumName = DatabaseAccess::colText(stmt, 0);
		std::string creationDate = DatabaseAccess::colText(stmt, 1);

		Album album(user.getId(), albumName);
		album.setCreationDate(creationDate);

		// add to list
		albumListByUser.push_back(album);
	}

	// finish the statement
	sqlite3_finalize(stmt);
	return albumListByUser;
}

/*
	Method to create an album in the db using the given album object
*/

void DatabaseAccess::createAlbum(const Album& album)
{
	sqlite3_stmt* stmt = prepare("INSERT INTO ALBUMS (name, creation_date, user_id) VALUES (?, ?, ?);");

	if (!stmt)  // is nullptr
	{
		return;
	}

	// now we got 3 question marks we need to assign
	// the fifth parameter is a destructor function pointer that SQLite
	// will call to dispose of the data string after it has finished using it
	// if it is the special value SQLITE_TRANSIENT it will make its own its own private copy of 
	// the string data immediately, and the application is free to reuse or free its original memory as soon as sqlite3_bind_text() returns.
	sqlite3_bind_text(stmt, 1, album.getName().c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, album.getCreationDate().c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 3, album.getOwnerId());

	if (sqlite3_step() != SQLITE_DONE)
	{
		std::cout << "Failed creating an album." << std::endl;
	}
	else
	{
		this->m_albums[album.getName()] = album;
	}
	sqlite3_finalize(stmt);
}

/*
	Method to delete an album from the database
	input: the album name to delete and its owner's id
	output: no return values
*/

void DatabaseAccess::deleteAlbum(const std::string& albumName, int userId)
{
	// this method too, requires more then one sql query
	// which means we need a transaction in order to make sure its all or nothing
	executeSqlQuery("BEGIN TRANSACTION;");
	
	// it will be simpler if we won't use prepare() here
	// because we are using one sqlite3_stmt for multiple queries
	bool ok = true;
	sqlite3_stmt* stmt = nullptr;

	// this is a complicated query, we need to get the picture id
	const char* deleteTags = "DELETE FROM TAGS WHERE picture_id"
		"IN (SELECT PICTURES.id FROM PICTURES"
		"INNER JOIN ALBUMS ON ALBUMS.id = PICTURES.album_id"
		"WHERE ALBUMS.name = ? AND ALBUMS.user_id = ? ); ";

	if (sqlite3_prepare_v2(this->m_db, deleteTags, -1, &stmt, nullptr) == SQLITE_OK)
	{
		// bind
		deleteAlbumBind(stmt, albumName, userId);

		// check if succeeded (was the last step because it sohuld only be one step)
		ok = (sqlite3_step(stmt) == SQLITE_DONE);
		sqlite3_finalize(stmt);
	}
	else
	{
		ok = false;
	}

	// move onto the next delete
	if (ok)
	{
		const char* deletePictures = "DELETE FROM ALBUMS WHERE "
			"ALBUM.name = ? AND user_id = ?;";	

		if (sqlite3_prepare_v2(this->m_db, deletePictures, -1, &stmt, nullptr) == SQLITE_OK)
		{
			// bind
			deleteAlbumBind(stmt, albumName, userId);

			ok = (sqlite3_step(stmt) == SQLITE_DONE);
			sqlite3_finalize(stmt);
		}
		else
		{
			ok = false;
		}
	}
	// final query
	if (ok)
	{
		const char* deletePictures = "DELETE FROM PICTURES WHERE "
			"ALBUM.id = (SELECT FROM ALBUMS WHERE name = ? AND user_id = ?);";

		if (sqlite3_prepare_v2(this->m_db, deletePictures, -1, &stmt, nullptr) == SQLITE_OK)
		{
			// bind
			deleteAlbumBind(stmt, albumName, userId);

			ok = (sqlite3_step(stmt) == SQLITE_DONE);
			sqlite3_finalize(stmt);
		}
		else
		{
			ok = false;
		}
	}

	// end the transaction by commiting or rolling back
	if (ok)
	{
		executeSqlQuery("COMMIT;");
		this->m_albums.erase(albumName);
	}
	else
	{
		executeSqlQuery("ROLLBACK;");
		std::cout << "Delete album failed!" << std::endl;
	}
}

/*
	Method to check if an album exists
	input: the album name and the owner's id
	output: true if exists and false otherwise
*/

bool DatabaseAccess::doesAlbumExists(const std::string& albumName, int userId)
{
	// first check if its saved
	auto it = this->m_albums.find(albumName);
	if (it != this->m_albums.end())
	{
		// make sure its the correct user
		return it->second.getOwnerId() == userId;
	}

	sqlite3_stmt* stmt = prepare("SELECT COUNT(*) FROM ALBUMS WHERE name  = ? AND user_id = ?;");
	if (!stmt)
	{
		return false;
	}
	bool exists = false;
	// bind the statement
	sqlite3_bind_text(stmt, 1, albumName.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 2, userId);

	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		// check if the count is bigger then 0
		exists = (sqlite3_column_int(stmt, 0) > 0);
	}

	sqlite3_finalize(stmt);

	return exists;
}

/*
	Method to open an album given its name
	input: the album's name
	output: the album itself
*/

Album DatabaseAccess::openAlbum(const std::string& albumName)
{
	this->m_pictures.clear();

	// try to find albumName if exists
	Album album;
	int albumId = -1;

	auto it = this->m_albums.find(albumName);
	if (it != this->m_albums.end())
	{
		album = it->second;

		// get the id
		sqlite3_stmt* idStmt = prepare("SELECT id FROM ALBUMS WHERE name = ?");
		if (!idStmt)
		{
			return Album(0, "");
		}
		sqlite3_bind_text(idStmt, 1, albumName.c_str(), -1, SQLITE_TRANSIENT);
		
		if (sqlite3_step(idStmt) == SQLITE_ROW)
		{
			albumId = sqlite3_column_int(idStmt, 0);
		}
		sqlite3_finalize(idStmt);
	}
	else
	{
		// select it
		// even though these are all of ALBUMS's fields, detailing them makes it easier
		// to take out the output of the query, knowing which column is in what index
		sqlite3_stmt* stmt = prepare("SELECT id, name, creation_date,"
			" user_id FROM ALBUMS WHERE name = ?;");
		if (!stmt)
		{
			return Album(0, "");
		}
		// bind the question mark
		sqlite3_bind_text(stmt, 1, albumName.c_str(), -1, SQLITE_TRANSIENT);
		if(sqlite3_step(stmt) == SQLITE_ROW)
		{
			// extract all of the data
			albumId = sqlite3_column_int(stmt, 0);
			album.setName(DatabaseAccess::colText(stmt, 1));
			album.setCreationDate(DatabaseAccess::colText(stmt, 2));
			album.setOwner(sqlite3_column_int(stmt, 3));

			this->m_albums[albumName] = album;
		}
		sqlite3_finalize(stmt);
	}

	// not found in databse
	if (albumId == -1)
	{
		return album;
	}

	// add the pictures to the album
	sqlite3_stmt* picturesStmt = prepare("SELECT id, name, location, creation_date "
	"FROM PICTURES WHERE album_id = ?;");
	if (!picturesStmt)
	{
		return Album(0, "");
	}

	sqlite3_bind_int(picturesStmt, 1, albumId);
	while (sqlite3_step() == SQLITE_ROW)
	{
		// construct the picture
		Picture picture(sqlite3_column_int(picturesStmt, 0),  // id
			DatabaseAccess::colText(picturesStmt, 1),  // name
			DatabaseAccess::colText(picturesStmt, 2), //location
			DatabaseAccess::colText(picturesStmt, 3));  //creation date

		// add to the local db and to the album itself
		this->m_pictures.push_back(picture);
		album.addPicture(picture);
	}
	sqlite3_finalize(picturesStmt);

	this->m_currentAlbum = album;  // save which album is open
	this->m_isAlbumOpen = true;
	return album;
}

/*
	Method to close an open album if open
	input: the album to close
	output: no return values
*/

void DatabaseAccess::closeAlbum(Album& pAlbum)
{ 
	// album manager does most of the work for us
	this->m_pictures.clear();
}

/*
	Method to print all of the albums
	input: no input values
	output: no return values
*/

void DatabaseAccess::printAlbums()
{
	std::list<Album> albumsList = getAlbums();

	std::cout << "Albums list:" << std::endl << "-----------" << std::endl;

	// print all of the albums
	for (auto& album : albumsList)
	{
		std::cout << "   * [" << album.getName() << " - created by user@" << album.getOwnerId <<
			album.getCreationDate() << std::endl;
	}
}

// picture related

/*
	Method to add a picture the an album by its name
	input: the album's name and the picture to add
	output: no return values
*/

void DatabaseAccess::addPictureToAlbumByName(const std::string& albumName, const Picture& picture)
{
	// we need an insert statement
	sqlite3_stmt* stmt= prepare("INSERT INTO PICTURES (name, location, creation_date, album_id)"
		"VALUES  (?, ?, ?, (SELECT id FROM ALBUMS WHERE name = ?));";
	if (!stmt)
	{
		std::cout << "Failed" << std::endl;
		return;
	}

	// bind all of the values caarefully
	sqlite3_bind_text(stmt, 0, picture.getName.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 1, picture.getPath().c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, picture.getCreationDate().c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, albumName.c_str(), -1, SQLITE_TRANSIENT);

	// check if it returned  something else then DONE (success)
	if (sqlite3_step() != SQLITE_DONE)
	{
		std::cout << "picture adding failed: " << sqlite3_errmsg(this->m_db) << std::endl;
	}
	sqlite3_finalize(stmt);

}

/*
	Mehtod to remove a pricture from an album by the picture's name
	input: the album's name and the picture's name
	output: no return vallues
*/

void DatabaseAccess::removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName)
{
	// again, this is a multible query situation so we need a transaction
	executeSqlQuery("BEGIN TRANSACTION;");
	
	// it will be simpler if we won't use prepare() here
	// because we are using one sqlite3_stmt for multiple queries
	sqlite3_stmt* stmt = nullptr;
	bool ok = false;

	// we need to make a query that allows us to remove the tags only by album name and picture name
	const char* deleteTags = "DELETE FROM TAGS WHERE picture_id = "
		"(SELECT PICTURES.id FROM PICTURES INNER JOIN ALBUMS ON ALBUMS.id = PICTURES.album_id "
		"WHERE PICTURES.name = ? AND ALBUMS.name = ?);";
	if (sqlite3_prepare_v2(this->m_db, deleteTags, -1, &stmt, nullptr) == SQLITE_OK)
	{
		// lets bind
		sqlite3_bind_text(stmt, 0, pictureName.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 1, albumName.c_str(), -1, SQLITE_TRANSIENT);

		ok = (sqlite3_step(stmt) == SQLITE_DONE);
		sqlite3_finalize(stmt);
	}
	else
	{
		ok = false;
	}

	// move on to the next deletion
	if (ok)
	{
		const char* deletePictures = "DELETE FROM PICTURES WHERE name = ? AND album_id = (SELECT id FROM ALBUMS WHERE name = ?);";
		if (sqlite3_prepare_v2(this->m_db, deletePictures, -1, &stmt, nullptr) == SQLITE_DONE)
		{
			sqlite3_bind_text(stmt, 0, pictureName.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 1, albumName.c_str(), -1, SQLITE_TRANSIENT);

			ok = (sqlite3_step(stmt) == SQLITE_DONE);
			sqlite3_finalize(stmt);
		}
		else
		{
			ok = false;
		}
	}
	if (ok)
	{
		// commit
		executeSqlQuery("COMMIT;");
	}
	else
	{
		// rollback
		executeSqlQuery("ROLLBACK;");
		std::cout << "removing picture failed." << std::endl;
	}
}

// users related

/*
	Method to print all of the users
	input: no input values
	output: no return values
*/

void DatabaseAccess::printUsers()
{
	this->m_users.clear();

	// select all of the users
	sqlite3_stmt* stmt = prepare("SELECT id, name FROM USERS;");
	if (!stmt)
	{
		return;
	}

	// iterate through the user records
	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		// extract the id and name from the record
		int id = sqlite3_column_int(stmt, 0);
		User user(id, DatabaseAccess::colText(stmt, 1));

		this->m_users[id] = user;
	}
	sqlite3_finalize(stmt);

	// print them
	std::cout << "Users list : " << std::endl << "-----------" << std::endl;
	for (const auto& pair : this->m_users)
	{
		std::cout << "   + @" << pair.first << " - " << pair.second.getName() << std::endl;
	}
}

/*
	Method to get a usetr by his id
	input: the user's id
	output: the user (object)
*/

User DatabaseAccess::getUser(int userId)
{
	// first check m_users
	auto it = this->m_users.find(userId);
	if (it != this->m_users.end())
	{
		return it->second;
	}

	sqlite3_stmt* stmt = prepare("SELECT name FROM USERS WHERE id = ?;");
	// there is no need to check if the stmt is null since it will always return a value
	// i can know that for sure because the method is only called when albumManager knows for
	// sure that the userId exists

	sqlite3_bind_int(stmt, 1, userId);

	User user(-1, "");
	if (sqlite3_step() != SQLITE_ROW)
	{
		// extract the data
		user = User(userId, colText(stmt, 0));
		this->m_users[userId] = user;
	}
	sqlite3_finalize(stmt);

	return user;

}

/*
	Method to check if a user exists by its id
	input: the user's id
	output: true if exists and false otherwise
*/

bool DatabaseAccess::doesUserExists(int userId)
{
	// first check m_users
	auto it = this->m_users.find(userId);
	if (it != this->m_users.end())
	{
		return true
	}
	sqlite3_stmt* stmt = prepare("SELECT COUNT(*) FROM USERS WHERE id = ?;");
	bool exists = false;


	if (!stmt)
	{
		return false;
	}
	sqlite3_bind_int(stmt, 1, userId);

	if (sqlite3_step() == SQLITE_ROW)
	{
		// if the count is bigger then 0 it exists
		exists = (sqlite3_column_int(stmt, 0) > 0);
	}
	sqlite3_finalize(stmt);

	return exists;	
}

// statistics

/*
	Method to count the amount of albums owner by a given user
	input: the user to count his albumms
	output: the count
*/

int DatabaseAccess::countAlbumsOwnedOfUser(const User& user)
{
	return runCountQuery("SELECT COUNT(*) FROM ALBUMS WHERE user_id = ?;", user.getId());
}

/*
	Method to count the amount of tags on albums of a user
	input: the user
	output: the count
*/

int DatabaseAccess::countAlbumsTaggedOfUser(const User& user)
{
	// explanataion of the query:
	//DISTINCT(PICTURES.album_id) returns the albums ids
	// we need to join them to tags using their id's
	// then we get the album id's that the user is tagged in them, but surrounding it with a COUNT
	// makes it count the records returniing the count of albums tagged of user
	return runCountQuery("SELECT COUNT(DISTINCT(PICTURES.album_id)) "
		"FROM TAGS INNER JOIN PICTURES ON TAGS.picture_id = PICTURES.id WHERE TAGS.user_id = ?;", user.getId());
}

/*
	Method to count the amount of tags a user have all accross all of fthe albums
	input: the user
	outpuut: the count
*/

int DatabaseAccess::countTagsOfUser(const User& user)
{
	return runCountQuery("SELECT COUNT(*) FROM TAGS WHERE user_id = ?;", user.getId());
}

/*
	Method the calculate the average tags per album of user
	input: the user
	output: the avg
*/

float DatabaseAccess::averageTagsPerAlbumOfUser(const User& user)
{
	int albums = countAlbumsOwnedOfUser(user);
	if (albuns == 0)
	{
		return 0.0;
	}

	return (float)countTagsOfUser(user) / (float)albums;
}

// queries

/*
	Method to get the user with the msot tags
	input: no input values
	output: the user (object)
*/

User DatabaseAccess::getTopTaggedUser()
{
	sqlite3_stmt* stmt = prepare("SELECT id, name FROM USERS ORDER BY (SELECT COUNT(*)"
		" FROM TAGS WHERE TAGS.user_id = USERS.id) DESC LIMIT 1;");
	User user(-1, "");
	if (!stmt)
	{
		return user;
	}

	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		// extract the values
		int id = sqlite3_column_int(stmt, 0);
		user.setId(id);
		user.setName(DatabaseAccess::colText(stmt, 1));

		this->m_users[id] = user;
	}

	sqlite3_finalize(stmt);
	return user;
}

/*
	Method to get the picture with the most tags
	input: no input values
	output: the picture with the most tags
*/

Picture DatabaseAccess::getTopTaggedPicture()
{
	
}

/*
	Method to get the tagged pictures of a user
	input: the user
	output: the list of pictures
*/

std::list<Picture> DatabaseAccess::getTaggedPicturesOfUser(const User& user)
{
	this->m_pictures.clear();

	sqlite3_stmt* stmt = prepare("SELECT PICTURES.id, PICTURES.name, PICTURES.location, PICTURES.creation_date "
	"FROM PICTURES INNER JOIN TAGS ON PICTURES.id = TAGS.picture_id WHERE TAGS.user_id = ?;");
	if (!stmt)
	{
		return {};
	}

	// bind and step
	sqlite3_bind_int(stmt, 1, user.getId());
	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		// craft the pictures
		Picture picture(sqlite3_column_int(stmt, 0),  // id
			DatabaseAccess::colText(stmt, 1),  // name
			DatabaseAccess::colText(stmt, 2),  //location
			DatabaseAccess::colText(stmt, 3));  //creation date
		this->m_pictures.push_back(picture);
	}
	sqlite3_finalize(stmt);

	return this->m_pictures;
}