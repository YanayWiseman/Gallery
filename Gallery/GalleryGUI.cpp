#include "GalleryGUI.h"
#include "MyException.h"
#include <sstream>
#include <string>
#include <list>

/*
	Constructor method to construct the GallertyGUI object
	input: the data access (database)
	output: no return values
*/

GalleryGUI::GalleryGUI(IDataAccess& dataAccess)
	: m_dataAccess(dataAccess), m_mainWindow(nullptr), m_textBox(nullptr)
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
}

/*
	Method to register the windows class, it creates the windows class,gives them their values
	bassically give the windows class all of the information they need (including callback functions)
	input: no input values
	output: true if succeeded and false otherwise
*/

bool GalleryGUI::registerWindowClasses()
{

}

