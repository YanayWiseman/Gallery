#include "GalleryGUI.h"
#include "MyException.h"
#include <sstream>
#include <string>
#include <list>

// each window class needs a string name registered to it
static const char* MAIN_WND_CLASS = "GalleryMainWindow";
static const char* INPUT_DLG_CLASS = "GalleryInputDialog";

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

void GalleryGUI::setAlbumOpenState(bool isOpen)
{

}
/*
	Method to append a line to the right panel (the text box in the middle)
	input; the text to append
	output: no return values
*/

void GalleryGUI::appendOutput(const std::string& text)
{
	//TODO
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