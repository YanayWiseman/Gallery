#include "GalleryGUI.h"
#include "MyException.h"
#include <sstream>
#include <string>
#include <list>

// each window class needs a string name registered to it
static const char* MAIN_WND_CLASS = "GalleryMainWindow";
static const char* INPUT_DLG_CLASS = "GalleryInputDialog";

// The callback function for the input dialog is a static function which means it cannot access
// GalleryGUI's member variables, thats why we use a pair of globals to pass the
// result back. only one input dialog can be open at a time, so this is safe.
static std::string inputResult;
static bool inputSubmitted;  // set to true when OK or Cancel is pressed

/*
	Constructor method to construct the GallertyGUI object
	input: the data access (database)
	output: no return values
*/

GalleryGUI::GalleryGUI(IDataAccess& dataAccess)
	: m_dataAccess(dataAccess), m_mainWindow(nullptr), m_textBoxDialog(nullptr)
	, m_isAlbumOpen(false), m_nextUserId(200), m_nextPictureId(100)
{
	int i = 0;

	// init all of the button
	for (i = 0; i < 6; i++)
	{
		this->m_pictureBtns[i] = nullptr;
	}
}

/*
	This method is the main function that will get requests and answer them 
	input: no input values
	output: no return values
*/

void GalleryGUI::run()
{
	// first we need to register both windows classes with windows.
	// a window class is a template that describes the window's style, background color, 
	// all of that graphical stuff but most importantly the callback function to call when an event happens
	if (!registerWindowClasses())
	{
		// MessageBoxA() displays a dialog box that contains a system icon,
		// a set of buttons, and a brief application specific message,
		// such as status or error information (learned from msdn)
		// the parameters are what you see in the box
		// MB_OK makes the box have an ok button and MB_ICONERROR adds a stop sign icon
		MessageBoxA(nullptr, "Failed registering the windows classes", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	// now we can actually create the main window

	this->m_mainWindow = CreateWindowExA(
		0,  // 0 means no extended styles (good ol fashioned window style)
		MAIN_WND_CLASS,  // this is the window class's name
		"Gallery",  // the title
		WS_OVERLAPPEDWINDOW,  // the window is overlapped just like every other window we know of
		CW_USEDEFAULT,  // x value, we can use default
		CW_USEDEFAULT,  // y value
		WINDOW_H,  // heigth
		WINDOW_W,  // width
		nullptr, nullptr,  // optional parameters
		GetModuleHandleA(nullptr),  // handle to the instance of the module to be associated with the window (from the msdn)
		this  // this is the most important parameter, this allows the callback function to connect to the window
		);

	// check if worked
	if (!this->m_mainWindow)
	{
		MessageBoxA(nullptr, "Failed creating the main window", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	// show and paint the window
	// it takes the handle to the window and the second parameter is the usual flag it takes
	ShowWindow(this->m_mainWindow, SW_SHOWNORMAL);

	// no need to explain what this is doing
	UpdateWindow(this->m_mainWindow);

	// now to the GUI's main loop
	//get the message, translate it, and dispatch it (explanation in the loop)
	MSG msg;  // MSG is the struct that GetMessage fills up
	// GetMessage takes the message struct to fill, the rest are optional parameters
	while (GetMessageA(&msg, nullptr, 0, 0))
	{
		// translates virtual-key messages into character message
		TranslateMessage(&msg);

		// DispatchMessage sends the message to the appropriate callback function!!!
		DispatchMessageA(&msg);
	}
}

/*
	Method to register the windows class, it creates the windows class,gives them their values
	bassically give the windows class all of the information they need (including callback functions)
	input: no input values
	output: true if succeeded and false otherwise
*/

bool GalleryGUI::registerWindowClasses()
{
	// we want to create the main window class
	WNDCLASSEXA mainW = {};  // first create it to avoid errors

	// now fill
	mainW.cbSize = sizeof(mainW);  // size of the struct
	// CS_HREDRAW | CS_VREDRAW means it will redraw the window 
	// if it is moved and dragged around the screen or resized
	mainW.style = CS_HREDRAW | CS_VREDRAW;
	mainW.lpfnWndProc = GalleryGUI::mainWndProc;  // a pointer to the window's procedure (callback function)
	mainW.hInstance = GetModuleHandleA(nullptr);  // like in CreateWindowExA()
	mainW.hCursor = LoadCursorA(nullptr, IDC_ARROW);  // handle to the class's curser (I have took LoadCursorA()'s parameters from msdn)
	mainW.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);  // 1 must be added to the chosen color (gray), also we need to cast to HBRUSH (said in msdn)
	mainW.lpszClassName = MAIN_WND_CLASS;  // the class's name

	// now register!
	if (!RegisterClassExA(&mainW))
	{
		// the function returns a false value if it fails
		return false;
	}
	
	// now we do the same for the textBox dialog's window
	WNDCLASSEXA textBox = {};
	textBox.cbSize = sizeof(textBox);
	textBox.lpfnWndProc = GalleryGUI::inputDlgProc;
	textBox.hInstance = GetModuleHandle(nullptr);
	textBox.hCursor = LoadCursor(nullptr, IDC_ARROW);
	textBox.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	textBox.lpszClassName = INPUT_DLG_CLASS;

	if (!RegisterClassExA(&textBox))
	{
		return false;
	}

	// it worked
	return true;
}

/*
	Method to create all of the controls, called from WM_CREATE
	input: no input values
	output: no return values
*/

void GalleryGUI::createControls()
{
	HINSTANCE instanceHandle = GetModuleHandleA(nullptr);  // used in CreateWindowExA()

	// now we create the text box dialog window inside of the main window
	this->m_textBoxDialog = CreateWindowExA(
		WS_EX_CLIENTEDGE,  // this is extended style, WS_EX_CLIENTEDGE makes the window have a border with a sunken edge
		"EDIT",  // class name
		"",  // window name
		// the style here is very important
		// ws_child says its a child window (in the main window), ws_visible means teh window is initialy visible,
		// ws_vscroll and hscroll adds a vertical and horizontal scroll bars,
		// ees_multiline is for a control that has lots of line  of text in it,
		// es_readonly means we cant edit whats printed in it
		// and es_autovscroll means it scrolls vertically automalically when the text is flooding
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL
		| ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
		PANEL_W + 10,  // x value
		10,  // y value
		WINDOW_W - PANEL_W - 25, // width
		WINDOW_H - 70,  //height
		this->m_mainWindow,  // parant window
		(HMENU)ID_OUTPUT_EDIT, // the id of the control
		instanceHandle,
		nullptr		
	);

	// set a monospace font so the output aligns nicely
	HFONT fontHandle = CreateFontA(
		14, 0, 0, 0, // height, width
		FW_NORMAL,  //  weight of the dont, normal is 400
		FALSE,  // no font
		FALSE,  //no underline
		FALSE,  //no strikeout font
		ANSI_CHARSET,  // the character set, this font supports the Windows ANSI character set (from mdsn)
		OUT_DEFAULT_PRECIS, //the output percision is default
		CLIP_DEFAULT_PRECIS,  // teh clipping percision is default
		DEFAULT_QUALITY, // the output's quality is default
		FIXED_PITCH | FF_MODERN,  // pitch and family of the font, the pitch is FIXED and loooks modern
		"Consolas"
	);

	// send the message to the window, wn_setfont is the type of message
	SendMessageA(this->m_textBoxDialog, WM_SETFONT, (WPARAM)fontHandle, TRUE);

	// left panel: all the command buttons                                
	layoutButtons();
}

/*
	Method to create all of the left panel controls and groupe them by category
	input; no input values
	output: no return values
*/

void GalleryGUI::layoutButtons()
{
	HINSTANCE instanceHandle = GetModuleHandleA(nullptr);
	int y = 8;   // current y position, we incremente it as we add controls

	// the left panel has 3 parts, the label for the group of commands
	// the buttons themselves, and the seperators so everytthing is nicely divided
	// we create helper functions for them using lambda
	// for the label:
	auto addLabel = [&](const char* text)
		{
			// create the label window
			// i have listed the parameters and documented using this function before
			// so i will not repeat it here
			CreateWindowExA(
				0, "STATIC", text, WS_CHILD | WS_VISIBLE | SS_LEFT,  // ss_left aligns the text to the left
				BTN_X, y, BTN_W, 18, this->m_mainWindow, nullptr, instanceHandle, nullptr
			); 
			y += 20;  // increment the y level for the next control
		};

	// for the button creation:
	// it gets the button's text and id, and returns a handle to it
	auto addButton = [&](const char* text, int id) -> HWND
		{
			HWND buttonHandle = CreateWindowExA(
				0, "BUTTON", text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,  // bs_pushbutton means its a pushable button
				BTN_X, y, BTN_W, BTN_H, this->m_mainWindow, (HMENU)(UINT_PTR)id,
				instanceHandle, nullptr
			);
			y += BTN_STEP;
			return buttonHandle;
		};
	
	// the horizontal dividers
	auto addSeperator = [&]()
		{
			y += 4;
			CreateWindowExA(
				0, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, // ss_etchedhorz used to draw a horizontal line, typically for separating sections in a user interface
				BTN_X, y, BTN_W, 2, this->m_mainWindow, nullptr,
				instanceHandle, nullptr
			);
			y += 8;
		};

	// now we can add all of the buttons, seperators and labels in order
	// albums                                                             
	addLabel("   Albums   ");
	addButton("Create album", ID_BTN_CREATE_ALBUM);
	addButton("Open album", ID_BTN_OPEN_ALBUM);
	addButton("Close album", ID_BTN_CLOSE_ALBUM);
	addButton("Delete album", ID_BTN_DELETE_ALBUM);
	addButton("List all albums", ID_BTN_LIST_ALBUMS);
	addButton("Albums of user", ID_BTN_ALBUMS_OF_USER);

	addSeperator();

	// pictures (require open album)                                      
	addLabel("   Pictures (open album first)   ");
	// store handles so we can enable/disable this group
	this->m_pictureBtns[0] = addButton("Add picture", ID_BTN_ADD_PICTURE);
	this->m_pictureBtns[1] = addButton("Remove picture", ID_BTN_REMOVE_PICTURE);
	this->m_pictureBtns[2] = addButton("List pictures", ID_BTN_LIST_PICTURES);
	this->m_pictureBtns[3] = addButton("Tag user", ID_BTN_TAG_USER);
	this->m_pictureBtns[4] = addButton("Untag user", ID_BTN_UNTAG_USER);
	this->m_pictureBtns[5] = addButton("List tags", ID_BTN_LIST_TAGS);

	addSeperator();

	// users                                                              
	addLabel("   Users   ");
	addButton("Add user", ID_BTN_ADD_USER);
	addButton("Remove user", ID_BTN_REMOVE_USER);
	addButton("List users", ID_BTN_LIST_USERS);
	addButton("User statistics", ID_BTN_USER_STATS);

	addSeperator();

	// queries                                                            
	addLabel("   Queries   ");
	addButton("Top tagged user", ID_BTN_TOP_TAGGED_USER);
	addButton("Top tagged picture", ID_BTN_TOP_TAGGED_PIC);
	addButton("Pictures tagged by user", ID_BTN_TAGGED_PICS);

	addSeperator();

	// utility                                                            
	addButton("Clear output", ID_BTN_CLEAR_OUTPUT);

	// start with picture buttons disabled because no album is open yet
	setAlbumOpenState(false);
}

/*
	Method to enable or disable the picture's buttons
	input: the mode to set to
	output: no return values
*/

void GalleryGUI::setAlbumOpenState(bool mode)
{
	this->m_isAlbumOpen = mode;

	// the function EnableWindow(handle, TRUE or FALSE) greys out the button and ignores clicks
	int i = 0;
	for (i = 0; i < 6; i++)
	{
		EnableWindow(this->m_pictureBtns[i], mode ? TRUE : FALSE);
	}
}

/*
	Method to append a line to the right panel (the text box in the middle)
	input; the text to append
	output: no return values
*/

void GalleryGUI::appendOutput(const std::string& text)
{
	// retrieve the length of the specified window's title bar text 
	int len = GetWindowTextLengthA(this->m_textBoxDialog);

	// we are calling the callback function here
	// send the specified message to the window
	// EM_SETSEL means select a range of characters in aa control or to place the text cursor at a specific position
	SendMessageA(this->m_textBoxDialog, EM_SETSEL, (WPARAM)len, (LPARAM)len);

	// EM_REPLACESEL replaces the current selection (which is the end)
	// with new text, this is the standard win api way to append
	SendMessageA(this->m_textBoxDialog, EM_REPLACESEL, FALSE, (LPARAM)text.c_str());

	// scroll the edit box to the bottom so the latest output is visible
	SendMessageA(this->m_textBoxDialog, WM_VSCROLL, SB_BOTTOM, 0);
}

/*
	Method to clear the output from the textBox Dialog
	input: no inpput values
	output: no return values
*/

void GalleryGUI::clearOutput()
{
	// just set the window's text to a blank string
	SetWindowTextA(this->m_mainWindow, "");
}

/*
	Method to show a pop up window with a dialog, used when input needs to be entered
	like user id, album name etc
	input: the prompt to showcase in the dialog
	output: the input entered by the user
*/

std::string GalleryGUI::showInputDialog(const std::string& prompt)
{
	inputResult = "";
	inputSubmitted = false;

	// disable the main window so the user must interact with the dialog first
	EnableWindow(this->m_mainWindow, FALSE);

	// now we can create th einput dialog window
	// we pass the prompt string as the lParam of WM_CREATE so the dialog's
	// callback fucntion can read it and set the static text label
	HWND dialogWindow = CreateWindowExA(
		WS_EX_DLGMODALFRAME,  // visual frame that looks like a dialog
		INPUT_DLG_CLASS,  // the string name registered to the window's class
		"Enter value",  // shown on the dialog
		WS_POPUP | WS_CAPTION | WS_SYSMENU,   // popup with title bar and x button
		0, 0,  // position
		320, 130,   // height and width
		this->m_mainWindow,   // parant window
		nullptr, GetModuleHandleA(nullptr),   // instance handle
		(LPVOID)prompt.c_str()   // prompt
	);

	// now we want to center the dialog relative to the main window
	RECT mainRectangle;
	GetWindowRect(this->m_mainWindow, &mainRectangle);   // this gets the proporsions of the main window
	int dialogX = mainRectangle.left + (mainRectangle.right - mainRectangle.left - 320) / 2;
	int dialogY = mainRectangle.top + (mainRectangle.bottom - mainRectangle.top - 130) / 2;

	//  now we set it
	// the last parameter is the window sizing and positioning flags
	// swp_nosize means it stays the size we set it, ignoring the cy, cx parameters (we set them to 0 anyways)
	// swp_nozorder retains the current Z order (ignores the hWndInsertAfter parameter)
	SetWindowPos(dialogWindow, nullptr, dialogX, dialogY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	
	// show it
	ShowWindow(dialogWindow, SW_SHOW);
	UpdateWindow(dialogWindow);

	// now the dialog window is displayed, we want to recieve output
	// this loop waits until inputSubmitted becomes true
	MSG msg;  // the message we recieve from the input
	while (!inputSubmitted)
	{
		// PeekMessage with PM_REMOVE checks for messages without blocking
		// we use this instead of GetMessage so we can check inputSubmitted inbetween messages
		if (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				// repost so the outer message loop also sees it
				PostQuitMessage((int)msg.wParam);
				break;
			}
		}
		// handle the message
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	// destroy the dialog window if it still exists
	if (IsWindow(dialogWindow))
	{
		DestroyWindow(dialogWindow);
	}

	// return focus to the main window
	EnableWindow(this->m_mainWindow, TRUE);
	SetForegroundWindow(this->m_mainWindow);  // brings the thread that created the specified window into the foreground and activates the window

	return inputResult;
}

/*
	Method to fetch the album from the database, it must be called before any
	operation that reads from m_openAlbum
	it is equivalent to ALbumManager::refreshOpenAlbum()
	input: no input values
	output: true if refreshed false  otherwise
*/

bool GalleryGUI::refreshOpenAlbum()
{
	if (!this->m_isAlbumOpen)
	{
		appendOutput("Error: no album is open currently!");
		return false;
	}

	this->m_openAlbum = this->m_dataAccess.openAlbum(this->m_currentAlbumName);
	return true;
}

/*
	Method to help cast an int to a string with a more friendly function
	input: the integer to cast
	output: the int as a string
*/

std::string GalleryGUI::str(int n)
{
	return std::to_string(n);
}

// The following methods behave like AlbumManager

// ALBUMS
/*
	Method to create an album on the CreateAlbum button click
	input: no input values
	output: no return values
*/

void GalleryGUI::onCreateAlbum()
{
	// now we can use all of the GUI helpers we have created very easily!
	// and ofcourse the DatabaseAccess methods from v1.0.2
	std::string userIdStr = showInputDialog("Enter owner user id:");
	if (userIdStr.empty()) 
	{
		return;
	}

	int userId = 0;
	try
	{
		userId = std::stoi(userIdStr);
	}
	catch (...)
	{
		appendOutput("Error: invalid user id.\r\n");
		return;
	}

	if (!this->m_dataAccess.doesUserExists(userId))
	{
		appendOutput("Error: no user with id @" + userIdStr + ".\r\n");
		return;
	}

	std::string name = showInputDialog("Enter album name:");
	if (name.empty()) 
	{
		return;
	}

	if (this->m_dataAccess.doesAlbumExists(name, userId))
	{
		appendOutput("Error: album [" + name + "] already exists for this user.\r\n");
		return;
	}

	Album newAlbum(userId, name);
	this->m_dataAccess.createAlbum(newAlbum);
	appendOutput("Album [" + name + "] created by user@" + userIdStr + ".\r\n");
}

/*
	Method to open an album on the OpenAlbum button click
	input: no input values
	output: no return values
*/

void GalleryGUI::onOpenAlbum()
{
	// if an album is already open, close it first (same behaviour as AlbumManager)
	if (this->m_isAlbumOpen)
	{
		this->m_dataAccess.closeAlbum(this->m_openAlbum);
		setAlbumOpenState(false);
		this->m_currentAlbumName = "";
	}

	std::string userIdStr = showInputDialog("Enter owner user id:");
	if (userIdStr.empty()) 
	{
		return;
	}

	int userId = 0;
	try 
	{
		userId = std::stoi(userIdStr);
	}
	catch (...)
	{
		appendOutput("Error: invalid user id.\r\n");
		return;
	}

	if (!this->m_dataAccess.doesUserExists(userId))
	{
		appendOutput("Error: no user with id @" + userIdStr + ".\r\n");
		return;
	}

	std::string name = showInputDialog("Enter album name:");
	if (name.empty())
	{
		return;
	}

	if (!this->m_dataAccess.doesAlbumExists(name, userId))
	{
		appendOutput("Error: no album [" + name + "] for user@" + userIdStr + ".\r\n");
		return;
	}

	this->m_openAlbum = this->m_dataAccess.openAlbum(name);
	this->m_currentAlbumName = name;
	setAlbumOpenState(true);
	appendOutput("Album [" + name + "] opened successfully.\r\n");
}

/*
	Method to close an album on the closeAlbum button click
	input: no input values
	output: no return values
*/

void GalleryGUI::onCloseAlbum()
{
	if (!this->m_isAlbumOpen)
	{
		appendOutput("Error: no album is currently open.\r\n");
		return;
	}
	this->m_dataAccess.closeAlbum(this->m_openAlbum);
	appendOutput("Album [" + this->m_currentAlbumName + "] closed.\r\n");
	this->m_currentAlbumName = "";
	setAlbumOpenState(false);
}

/*
	Method to delete an album on the deleteAlbum button click
	input: no input values
	output: no return values
*/

void GalleryGUI::onDeleteAlbum()
{
	std::string userIdStr = showInputDialog("Enter owner user id:");
	if (userIdStr.empty()) 
	{
		return;
	}

	int userId = 0;
	try
	{
		userId = std::stoi(userIdStr);
	}
	catch (...)
	{
		appendOutput("Error: invalid user id.\r\n");
		return;
	}

	if (!this->m_dataAccess.doesUserExists(userId))
	{
		appendOutput("Error: no user with id @" + userIdStr + ".\r\n");
		return;
	}

	std::string name = showInputDialog("Enter album name:");
	if (name.empty()) 
	{
		return;
	}

	if (!this->m_dataAccess.doesAlbumExists(name, userId))
	{
		appendOutput("Error: no album [" + name + "] for user@" + userIdStr + ".\r\n");
		return;
	}

	// if the album being deleted is currently open, close it first
	if (this->m_isAlbumOpen && this->m_currentAlbumName == name)
	{
		this->m_dataAccess.closeAlbum(this->m_openAlbum);
		this->m_currentAlbumName = "";
		setAlbumOpenState(false);
	}

	this->m_dataAccess.deleteAlbum(name, userId);
	appendOutput("Album [" + name + "] deleted.\r\n");
}

/*
	Method to list the albums on the listAlbums button click
	input: no input values
	output: no return values
*/

void GalleryGUI::onListAlbums()
{
	// getAlbums returns the list so we format it ourselves (nicer than printAlbums)
	std::list<Album> albums = this->m_dataAccess.getAlbums();

	appendOutput("    All albums                          \r\n");
	if (albums.empty())
	{
		appendOutput("  (no albums)\r\n");
	}
	else
	{
		for (const auto& album : albums)
		{
			appendOutput("  [" + album.getName() + "]"
				+ "  owner@" + str(album.getOwnerId())
				+ "  created: " + album.getCreationDate() + "\r\n");
		}
	}
	appendOutput("                                        \r\n");
}

/*
	Method to list album of user on the listAlbumsOfUser button click
	input: no input values
	output: no return values
*/

void GalleryGUI::onListAlbumsOfUser()
{
	std::string userIdStr = showInputDialog("Enter user id:");
	if (userIdStr.empty()) 
	{
		return;
	}

	int userId = 0;
	try
	{
		userId = std::stoi(userIdStr);
	}
	catch (...)
	{
		appendOutput("Error: invalid user id.\r\n");
		return;
	}

	if (!this->m_dataAccess.doesUserExists(userId))
	{
		appendOutput("Error: no user with id @" + userIdStr + ".\r\n");
		return;
	}

	User user = this->m_dataAccess.getUser(userId);
	std::list<Album> albums = this->m_dataAccess.getAlbumsOfUser(user);

	appendOutput("    Albums of user@" + userIdStr + "               \r\n");
	for (const auto& album : albums)
	{
		appendOutput("  [" + album.getName() + "]  created: " + album.getCreationDate() + "\r\n");
	}
	appendOutput("                                        \r\n");
}

// PICTURES                                                                 

/*
	Method to add a picture to an album on the addPicture button click
	input: no input values
	output: no return values
*/

void GalleryGUI::onAddPicture()
{
	if (!refreshOpenAlbum()) 
	{
		return;
	}

	std::string picName = showInputDialog("Enter picture name:");
	if (picName.empty())
	{
		return;
	}

	if (this->m_openAlbum.doesPictureExists(picName))
	{
		appendOutput("Error: picture <" + picName + "> already exists in this album.\r\n");
		return;
	}

	std::string picPath = showInputDialog("Enter picture path (e.g. pics\\photo.jpg):");
	if (picPath.empty())
	{
		return;
	}

	// increment counter, DatabaseAccess::addPictureToAlbumByName will
	// autocorrect the id if it collides with existing database rows
	this->m_nextPictureId++;
	Picture picture(this->m_nextPictureId, picName);
	picture.setPath(picPath);

	this->m_dataAccess.addPictureToAlbumByName(this->m_currentAlbumName, picture);
	appendOutput("Picture [" + str(picture.getId()) + "] <" + picName
		+ "> added to album [" + this->m_currentAlbumName + "].\r\n");
}

/*
	Method to remove a picture to an album on the removePicture button click
	input: no input values
	output: no return values
*/

void GalleryGUI::onRemovePicture()
{
	if (!refreshOpenAlbum())
	{
		return;
	}

	std::string picName = showInputDialog("Enter picture name:");
	if (picName.empty())
	{
		return;
	}

	if (!this->m_openAlbum.doesPictureExists(picName))
	{
		appendOutput("Error: no picture <" + picName + "> in this album.\r\n");
		return;
	}

	this->m_dataAccess.removePictureFromAlbumByName(this->m_currentAlbumName, picName);
	appendOutput("Picture <" + picName + "> removed from [" + this->m_currentAlbumName + "].\r\n");
}

/*
	Method to list the pictures of an album on the listPictures button click
	input: no input values
	output: no return values
*/

void GalleryGUI::onListPictures()
{
	if (!refreshOpenAlbum())
	{
		return;
	}

	std::list<Picture> pics = this->m_openAlbum.getPictures();
	appendOutput("    Pictures in [" + this->m_currentAlbumName + "]       \r\n");

	if (pics.empty())
	{
		appendOutput("  (no pictures)\r\n");
	}
	else
	{
		for (const auto& pic : pics)
		{
			appendOutput("  [" + str(pic.getId()) + "] " + pic.getName()
				+ "  path: " + pic.getPath()
				+ "  tags: " + str(pic.getTagsCount())
				+ "  date: " + pic.getCreationDate() + "\r\n");
		}
	}
	appendOutput("                                        \r\n");
}

/*
	Method to tag a user to an picture on the taguser button click
	input: no input values
	output: no return values
*/

void GalleryGUI::onTagUser()
{
	if (!refreshOpenAlbum())
	{
		return;
	}

	std::string picName = showInputDialog("Enter picture name:");
	if (picName.empty())
	{
		return;
	}

	if (!this->m_openAlbum.doesPictureExists(picName))
	{
		appendOutput("Error: no picture <" + picName + "> in this album.\r\n");
		return;
	}

	std::string userIdStr = showInputDialog("Enter user id to tag:");
	if (userIdStr.empty())
	{
		return;
	}

	int userId = 0;
	try
	{
		userId = std::stoi(userIdStr);
	}
	catch (...) 
	{
		appendOutput("Error: invalid user id.\r\n");
		return;
	}

	if (!m_dataAccess.doesUserExists(userId))
	{
		appendOutput("Error: no user with id @" + userIdStr + ".\r\n");
		return;
	}

	this->m_dataAccess.tagUserInPicture(this->m_currentAlbumName, picName, userId);
	appendOutput("User @" + userIdStr + " tagged in <" + picName + ">.\r\n");
}

/*
	Method to untag a user from a picture on the UntagUser button click
	input: no input values
	output: no return values
*/

void GalleryGUI::onUntagUser()
{
	if (!refreshOpenAlbum())
	{
		return;
	}

	std::string picName = showInputDialog("Enter picture name:");
	if (picName.empty())
	{
		return;
	}

	if (!this->m_openAlbum.doesPictureExists(picName))
	{
		appendOutput("Error: no picture <" + picName + "> in this album.\r\n");
		return;
	}

	std::string userIdStr = showInputDialog("Enter user id to untag:");
	if (userIdStr.empty()) return;

	int userId = 0;
	try
	{
		userId = std::stoi(userIdStr);
	}
	catch (...)
	{
		appendOutput("Error: invalid user id.\r\n");
		return;
	}

	if (!this->m_dataAccess.doesUserExists(userId))
	{
		appendOutput("Error: no user with id @" + userIdStr + ".\r\n");
		return;
	}

	// check the user is actually tagged before trying to untag
	Picture pic = this->m_openAlbum.getPicture(picName);
	if (!pic.isUserTagged(userId))
	{
		appendOutput("Error: user @" + userIdStr + " is not tagged in <" + picName + ">.\r\n");
		return;
	}

	this->m_dataAccess.untagUserInPicture(this->m_currentAlbumName, picName, userId);
	appendOutput("User @" + userIdStr + " untagged from <" + picName + ">.\r\n");
}

/*
	Method to list the tags on the listTags button click
	input: no input values
	output: no return values
*/

void GalleryGUI::onListTags()
{
	if (!refreshOpenAlbum())
	{
		return;
	}

	std::string picName = showInputDialog("Enter picture name:");
	if (picName.empty())
	{
		return;
	}

	if (!this->m_openAlbum.doesPictureExists(picName))
	{
		appendOutput("Error: no picture <" + picName + "> in this album.\r\n");
		return;
	}

	Picture pic = this->m_openAlbum.getPicture(picName);
	const std::set<int>& tags = pic.getUserTags();

	appendOutput("    Tags in <" + picName + ">               \r\n");
	if (tags.empty())
	{
		appendOutput("  (no tags)\r\n");
	}
	else
	{
		for (int userId : tags)
		{
			User user = this->m_dataAccess.getUser(userId);
			appendOutput("  @" + str(userId) + " - " + user.getName() + "\r\n");
		}
	}
	appendOutput("                                        \r\n");
}

// USERS                                                                    

/*
	Method to add a user on the AddUser button click
	input: no input values
	output: no return values
*/

void GalleryGUI::onAddUser()
{
	std::string name = showInputDialog("Enter user name:");
	if (name.empty())
	{
		return;
	}

	this->m_nextUserId++;
	User user(this->m_nextUserId, name);
	this->m_dataAccess.createUser(user);   // databaseAccess auto corrects id if needed
	appendOutput("User @" + str(user.getId()) + " [" + name + "] created.\r\n");
}

/*
	Method to remove a user on the removeUser button click
	input: no input values
	output: no return values
*/

void GalleryGUI::onRemoveUser()
{
	std::string userIdStr = showInputDialog("Enter user id:");
	if (userIdStr.empty())
	{
		return;
	}

	int userId = 0;
	try
	{
		userId = std::stoi(userIdStr);
	}
	catch(...)
	{
		appendOutput("Error: invalid user id.\r\n");
		return;
	}

	if (!this->m_dataAccess.doesUserExists(userId))
	{
		appendOutput("Error: no user with id @" + userIdStr + ".\r\n");
		return;
	}

	// if the open album belongs to this user, close it first
	if (this->m_isAlbumOpen && this->m_openAlbum.getOwnerId() == userId)
	{
		this->m_dataAccess.closeAlbum(this->m_openAlbum);
		this->m_currentAlbumName = "";
		setAlbumOpenState(false);
	}

	User user = this->m_dataAccess.getUser(userId);
	this->m_dataAccess.deleteUser(user);
	appendOutput("User @" + userIdStr + " deleted.\r\n");
}

/*
	Method to list the users on the listUsers button click
	input: no input values
	output: no return values
*/

void GalleryGUI::onListUsers()
{
	appendOutput("    Users                               \r\n");
	appendOutput("  (users also printed to console window)\r\n");

	this->m_dataAccess.printUsers();

	appendOutput("  Check the console window for the full list.\r\n");
	appendOutput("                                        \r\n");
}

/*
	Method to showcase the user's statistics on the UserStats button click
	input: no input values
	output: no return values
*/

void GalleryGUI::onUserStats()
{
	std::string userIdStr = showInputDialog("Enter user id:");
	if (userIdStr.empty())
	{
		return;
	}

	int userId = 0;
	try
	{
		userId = std::stoi(userIdStr);
	}
	catch (...)
	{
		appendOutput("Error: invalid user id.\r\n");
		return;
	}

	if (!this->m_dataAccess.doesUserExists(userId))
	{
		appendOutput("Error: no user with id @" + userIdStr + ".\r\n");
		return;
	}

	User user = this->m_dataAccess.getUser(userId);

	appendOutput("    Stats for @" + userIdStr + " [" + user.getName() + "]   \r\n");
	appendOutput("  Albums owned:    " + str(this->m_dataAccess.countAlbumsOwnedOfUser(user)) + "\r\n");
	appendOutput("  Albums tagged:   " + str(this->m_dataAccess.countAlbumsTaggedOfUser(user)) + "\r\n");
	appendOutput("  Total tags:      " + str(this->m_dataAccess.countTagsOfUser(user)) + "\r\n");

	float avg = this->m_dataAccess.averageTagsPerAlbumOfUser(user);
	std::ostringstream stringStream;
	stringStream.precision(2);
	stringStream << std::fixed << avg;
	appendOutput("  Avg tags/album:  " + stringStream.str() + "\r\n");
	appendOutput("                                        \r\n");
}

// QUERIES                                                                  

/*
	Method to get the top tagged user on the TopTaggedUser button click
	input: no input values
	output: no return values
*/

void GalleryGUI::onTopTaggedUser()
{
	User user = this->m_dataAccess.getTopTaggedUser();

	appendOutput("    Top tagged user                      \r\n");
	if (user.getId() == -1)
	{
		appendOutput("  (no tags in the database yet)\r\n");
	}
	else
	{
		appendOutput("  @" + str(user.getId()) + " - " + user.getName()
			+ "  (" + str(this->m_dataAccess.countTagsOfUser(user)) + " tags)\r\n");
	}
	appendOutput("                                        \r\n");
}

/*
	Method to get the top tagged picture on the TopTaggedPicture button click
	input: no input values
	output: no return values
*/

void GalleryGUI::onTopTaggedPicture()
{
	Picture pic = this->m_dataAccess.getTopTaggedPicture();

	appendOutput("    Top tagged picture                   \r\n");
	if (pic.getId() == -1)
	{
		appendOutput("  (no tags in the database yet)\r\n");
	}
	else
	{
		appendOutput("  [" + str(pic.getId()) + "] " + pic.getName()
			+ "  path: " + pic.getPath()
			+ "  tags: " + str(pic.getTagsCount()) + "\r\n");
	}
	appendOutput("                                        \r\n");
}

/*
	Method to get the tagged pictures of a user on the taggedPicturesOfUser button click
	input: no input values
	output: no return values
*/

void GalleryGUI::onTaggedPicturesOfUser()
{
	std::string userIdStr = showInputDialog("Enter user id:");
	if (userIdStr.empty())
	{
		return;
	}

	int userId = 0;
	try
	{
		userId = std::stoi(userIdStr);
	}
	catch (...)
	{
		appendOutput("Error: invalid user id.\r\n");
		return;
	}

	if (!this->m_dataAccess.doesUserExists(userId))
	{
		appendOutput("Error: no user with id @" + userIdStr + ".\r\n");
		return;
	}

	User user = this->m_dataAccess.getUser(userId);
	std::list<Picture> pics = this->m_dataAccess.getTaggedPicturesOfUser(user);

	appendOutput("    Pictures tagged by @" + userIdStr + "         \r\n");
	if (pics.empty())
	{
		appendOutput("  (none)\r\n");
	}
	else
	{
		for (const auto& pic : pics)
		{
			appendOutput("  [" + str(pic.getId()) + "] " + pic.getName()
				+ "  " + pic.getPath() + "\r\n");
		}
	}
	appendOutput("                                        \r\n");
}

/*
	Callback function, this is the most important method of the classs, this method is called by
	winapi for every single event that occurs in the main window, including when it gets created and deleted
	input: the parameters are similar for every callback function because its predetermined
	hwnd is the handle to the window, uMsg is the message code, wParam is the first message's parameter
	, lparam is the second message's parameter, both wParam and lParam are depended on uMsg
	output: the response that the program returns to the window (int)
*/

LRESULT CALLBACK GalleryGUI::mainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// retrieve the GalleryGUI* stored in the window's user data (getWindowLongA() does that)
	// on WM_CREATE this is not set yet, so we check for nullptr
	GalleryGUI* self = reinterpret_cast<GalleryGUI*>(GetWindowLongA(hwnd, GWLP_USERDATA));  // GWLP_USERDATA returns the user data

	// now we sort by the uMsg, there are 4 cases
	// creation call, command call, resize, destroy
	switch (uMsg)
	{
		case WM_CREATE:
		{
			// WM_CREATE happens when creating the window using CreateWindowExA()
			// we create the buttons in createControls() (our method)
			// but first, when WM_CREATE happens, it carries a CREATESTRUCT* in lparam
			// we need to store it in GWLP_USERDATA for future calls in WM_COMMAND
			// this is why why retrieve it before the switch case
			// the types here are scatchy so we use reinterpret cast
			CREATESTRUCTA* createStruct = reinterpret_cast<CREATESTRUCTA*>(lParam);

			// lpCreateParams is additional data used to create the window
			GalleryGUI* gui = reinterpret_cast<GalleryGUI*>(createStruct->lpCreateParams);
			// this fucntion canges the attribute of the window
			SetWindowLongA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(gui));

			// now we create all of the controls
			gui->m_mainWindow = hwnd;
			gui->createControls();

			// welcome message in the output area
			gui->appendOutput("Welcome to Gallery!\r\n");
			gui->appendOutput("Select a command from the left panel.\r\n");
			gui->appendOutput("                                        \r\n");

			return 0;
		}
		case WM_COMMAND:
		{
			return 0;
		}
		case WM_SIZE:
		{
			return 0;
		}
		case WM_DESTROY:
		{
			return 0;
		}
	}
	
	// if the message is not one of the four (unlikely)
	// theres a defualt callback function by winapi, we let them handle my problems
	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

/*
	Callback function, for the text box dialog window, the input and output is the same as in mainWndProc()
	please read mainWndProc's docstring
*/

LRESULT CALLBACK GalleryGUI::inputDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

}