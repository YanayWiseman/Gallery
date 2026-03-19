#pragma once

#include <list>
#include <vector>
#include <unordered_map>
#include "Album.h"
#include "User.h"
#include "IDataAccess.h"
#include "sqlite3.h"

#define DATABASE_NAME "YanayGallery.sqlite"

class DatabaseAccess : public IDataAccess
{
private:
	// Fields
	sqlite3* m_db;
	std::string m_dbFile;

	// m_users is almost always accesed by user id so using a map should make this more efficient
	std::unordered_map<int, User> m_users;    // key is user id

	// m_albums is almost always accessed by album name so using a map should make this more efficient
	std::unordered_map<std::string, Album> m_albums;   // key is album name
	
	// m_pictures is never looked up individualy, a list is the right call for this case
	// a map would make the code awkward
	std::list<Picture> m_pictures;

	bool executeSqlQuery(const std::string& statement) const;
	//static const Album& albumFromRow(sqlite3_stmt* stmt);
	static const std::string& colText(sqlite3_stmt* stmt, int col);

public:
	// Methods
	
	// Constructor and Destructor
	DatabaseAccess();
	virtual ~DatabaseAccess();

	// databse related
	bool open() override;
	void close() override;
	void clear() override;

	// album related
	const std::list<Album> getAlbums() override;
	const std::list<Album> getAlbumsOfUser(const User& user) override;
	void createAlbum(const Album& album) override;
	void deleteAlbum(const std::string& albumName, int userId) override;
	bool doesAlbumExists(const std::string& albumName, int userId) override;
	Album openAlbum(const std::string& albumName) override;
	void closeAlbum(Album& pAlbum) override;
	void printAlbums() override;

	// picture related
	void addPictureToAlbumByName(const std::string& albumName, const Picture& picture) override;
	void removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName) override;
	void tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId) override;
	void untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId) override;

	// user related
	void printUsers() override;
	void createUser(User& user) override;
	void deleteUser(const User& user) override;
	bool doesUserExists(int userId) override;
	User getUser(int userId) override;

	// user statistics
	int countAlbumsOwnedOfUser(const User& user) override;
	int countAlbumsTaggedOfUser(const User& user) override;
	int countTagsOfUser(const User& user) override;
	float averageTagsPerAlbumOfUser(const User& user) override;

	// queries
	User getTopTaggedUser() override;
	Picture getTopTaggedPicture() override;
	std::list<Picture> getTaggedPicturesOfUser(const User& user) override;
};
