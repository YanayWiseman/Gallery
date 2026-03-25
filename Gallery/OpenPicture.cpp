#include "OpenPicture.h"

PictureProcessManager* PictureProcessManager::instance = nullptr;

/*
	Constructor method to Construct the picture process manager object
	input: no input values
	output: no return values
*/

PictureProcessManager::PictureProcessManager() :
	_pictureOpen(false), _procInfo({0})
{
	instance = this;
	std::signal(SIGINT, PictureProcessManager::staticHandler);
}

/*
	Destructor method to destruct the picture process manager object
	input: no input values
	output: no return values
*/

PictureProcessManager::~PictureProcessManager()
{
	// close  all handles
	if(this->_procInfo.hProcess)
	{
		CloseHandle(this->_procInfo.hProcess);
	}
	if (this->_procInfo.hThread)
	{
		CloseHandle(this->_procInfo.hThread);
	}
}

/*
	Static method to act as kind of a bridge form the instance of this class and the signal() function
	input: the signal number recieved from the signal() function
	output: no return values
*/

void PictureProcessManager::staticHandler(int signum)
{
	if (instance)
	{
		instance->closePictureProcess(signum);
	}
}

/*
	Method to manipulate the CTRL+C interrupt
	input: the signal number (handed from the signal() function)
	output: no return values
*/

void PictureProcessManager::closePictureProcess(int signum)
{
	// if paint or irfananview is open and theres a proccess open we close it
	if (this->_pictureOpen && this->_procInfo.hProcess != nullptr)
	{
		std::cout << "Ctrl+C was typed when an image was open, closing it" << std::endl;

		// terminate the proccess and close its handles
		TerminateProcess(this->_procInfo.hProcess, 0);
		CloseHandle(this->_procInfo.hProcess);
		CloseHandle(this->_procInfo.hThread);

		this->_procInfo = { 0 };
		this->_pictureOpen = false;

		// the signal function 'loads' only once every call, so we call it every time we need to
		// get ready for the next interrupt
		std::signal(SIGINT, PictureProcessManager::staticHandler);
	}
	// else we exit
	else
	{
		std::cout << "Ctrl+C was typed when no image was open, existing the program" << std::endl;
		exit(signum);
	}
}

/*
	Method to open a picture using an external application chosen by the user
	input: the path of the picture
	output: no return values
*/

void PictureProcessManager::openPicture(const std::string& path)
{
	std::cout << "Please choose one of the following applications to open the picture with: " << std::endl;
	std::cout << "1. Paint" << std::endl << "2. IrfanView" << std::endl;
	std::string input;
	std::getline(std::cin, input);

	if (input != "1" && input != "2")
	{
		std::cout << "Invalid Choice!, returning to menu..." << std::endl;
		return;
	}
	int answer = atoi(input.c_str());

	std::string cmdLine;

	// now we can open the picture
	switch (answer)
	{
		case PAINT:
		{
			cmdLine = "mspaint " + path;
			if (!openExternalApplication(cmdLine))
			{
				return;
			}
			break;
		}
		case INFRAVIEW:
		{
			cmdLine = "i_view64.exe " + path;
			if (!openExternalApplication(cmdLine))
			{
				return;
			}
			break;
		}
	}
	this->_pictureOpen = true;
	std::string temp = (answer == PAINT) ? "Paint!" : "IrfanView!";
	std::cout << "Picture Opened Using " << temp << std::endl;
}

/*
	Helper Method used to open a picture with an external application and keep our program encapsulated
	input: the path to the picture
	output: true if succeeded and false otherwise
*/

bool PictureProcessManager::openExternalApplication(const std::string& cmdLine)
{
	// in order to start a process we need to use the STARTUPINFO struct
	// (we already did it in archi)
	// the only field that matters is the size of the strucure
	STARTUPINFO startUpInfo = { sizeof(startUpInfo) };

	// now we need to create the proccess
	// everything that is either NULL, 0  or FALSE doesnt matter to us
	if(CreateProcessA(NULL,
		(LPSTR)cmdLine.c_str(), // this is what will be typed in the cmd as parameters to main to the paint program
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&startUpInfo, // A pointer to a STARTUPINFO structure. (from the MSDN)
		&(this->_procInfo)))  // A pointer to a PROCESS_INFORMATION structure that receives identification information about the new process. (FROM MSDN)
	{
		return true;
	}
	else
	{
		DWORD error = GetLastError();
		std::cout << "Error: " << error << std::endl;
		return false;
	}
}
