#pragma once
#include "IGalleryApp.h"
#include "IDataAccess.h"
#include "Album.h"
#include "User.h"
#include "Picture.h"
#include <Windows.h>
#include <string>                                                      

class GalleryGUI : public IGalleryApp
{
private:
	// fields
	IDataAccess& m_dataAccess;

	// the windows
	HWND m_mainWindow;
	HWND m_textBox;

	// picture buttons
	HWND m_pictureBtns[6];

	// like album manager:
	bool m_isAlbumOpen;
	std::string m_currentAlbumName;
	Album m_openAlbum;
	int m_nextUserId;
	int m_nextPictureId;

	// methods

	// win api's window setup
	bool registerWindowClasses();


public:
	// c'tor
	explicit GalleryGUI(IDataAccess& dataAccess);

	// methods
	void run() override;

};
