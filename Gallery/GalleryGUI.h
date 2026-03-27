#pragma once
#include "IGalleryApp.h"
#include "IDataAccess.h"
#include "Album.h"
#include "User.h"
#include "Picture.h"
#include <Windows.h>
#include <string>

//CONTROL IDs                                                              
// every button, text box, and panel needs a unique id so that
// the window procedure can identify who sent the WM_COMMAND message

// album buttons   
#define ID_BTN_CREATE_ALBUM 1001
#define ID_BTN_OPEN_ALBUM 1002
#define ID_BTN_CLOSE_ALBUM 1003
#define ID_BTN_DELETE_ALBUM 1004
#define ID_BTN_LIST_ALBUMS 1005
#define ID_BTN_ALBUMS_OF_USER 1006

// picture buttons (only enabled when an album is open)   
#define ID_BTN_ADD_PICTURE 1007
#define ID_BTN_REMOVE_PICTURE 1008
#define ID_BTN_LIST_PICTURES 1009
#define ID_BTN_TAG_USER 1010
#define ID_BTN_UNTAG_USER 1011
#define ID_BTN_LIST_TAGS 1012

// user buttons   
#define ID_BTN_ADD_USER 1013
#define ID_BTN_REMOVE_USER 1014
#define ID_BTN_LIST_USERS 1015
#define ID_BTN_USER_STATS 1016

// query buttons   
#define ID_BTN_TOP_TAGGED_USER 1017
#define ID_BTN_TOP_TAGGED_PIC 1018
#define ID_BTN_TAGGED_PICS 1019

#define ID_BTN_CLEAR_OUTPUT 1020   // clears the right panel output area

// output text area   
#define ID_OUTPUT_EDIT 2001

// window layout constants
#define WINDOW_H 790
#define WINDOW_W 900
#define PANEL_W 230

// button layout constants
#define BTN_X 10 // left margin for buttons
#define BTN_W 210  // button width
#define BTN_H 28  // button height
#define BTN_GAP 4  // vertical gap between buttons
#define BTN_STEP (BTN_H + BTN_GAP) 


class GalleryGUI : public IGalleryApp
{
private:
	// fields
	IDataAccess& m_dataAccess;

	// the windows
	HWND m_mainWindow;
	HWND m_textBoxDialog;

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
	void createControls();
	void layoutButtons();
	void setAlbumOpenState(bool isOpen);

	// callback functions for the windows
	// just like in sqlite, the callback function's declaration must look like the library wants
	// this is the syntax for winapi
	static LRESULT CALLBACK mainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK inputDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// output helpers
	void appendOutput(const std::string& text);   // append a line to the right panel
	void clearOutput();

	// input dialog
	std::string showInputDialog(const std::string& prompt);

	// equivalent to AlbumManager::refreshOpenAlbum()
	bool refreshOpenAlbum();

	// a string cast helper
	std::string str(int n);

	// command handlers
	// albums                            
	void onCreateAlbum();
	void onOpenAlbum();
	void onCloseAlbum();
	void onDeleteAlbum();
	void onListAlbums();
	void onListAlbumsOfUser();

	// pictures
	void onAddPicture();
	void onRemovePicture();
	void onListPictures();
	void onTagUser();
	void onUntagUser();
	void onListTags();

	// users
	void onAddUser();
	void onRemoveUser();
	void onListUsers();
	void onUserStats();

	// queries
	void onTopTaggedUser();
	void onTopTaggedPicture();
	void onTaggedPicturesOfUser();

public:
	// c'tor
	explicit GalleryGUI(IDataAccess& dataAccess);

	// methods
	void run() override;

};
