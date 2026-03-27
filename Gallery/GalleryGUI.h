#pragma once
#include "IGalleryApp.h"
#include "IDataAccess.h"
#include "Album.h"
#include "User.h"
#include "Picture.h"
#include <Windows.h>
#include <string>

// window layout constants
#define WINDOW_H 790
#define WINDOW_W 900

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

	// callback functions for the windows
	// just like in sqlite, the callback function's declaration must look like the library wants
	// this is the syntax for winapi
	static LRESULT CALLBACK mainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK inputDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	// c'tor
	explicit GalleryGUI(IDataAccess& dataAccess);

	// methods
	void run() override;

};
