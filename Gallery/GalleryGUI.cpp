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


LRESULT CALLBACK GalleryGUI::mainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

}
LRESULT CALLBACK GalleryGUI::inputDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

}