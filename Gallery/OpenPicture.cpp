#include "OpenPicture.h"

PictureProcessManager* PictureProcessManager::instance = nullptr;

/*
	Constructor method to Construct the picture process manager object
	input: no input values
	output: no return values
*/

PictureProcessManager::PictureProcessManager() :
	pictureOpen(false), procInfo({0})
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
	if(procInfo.hProcess)
	{
		CloseHandle(procInfo.hProcess);
	}
	if (procInfo.hThread)
	{
		CloseHandle(procInfo.hThread);
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
	// if paint or infraview is open and theres a proccess open we close it
	if (true || pictureOpen && procInfo.hProcess != nullptr)
	{
		std::cout << "Ctrl+C was typed when an image was open, closing it" << std::endl;

		// terminate the proccess and close its handles
		TerminateProcess(procInfo.hProcess, 0);
		CloseHandle(procInfo.hProcess);
		CloseHandle(procInfo.hThread);

		procInfo = { 0 };
		pictureOpen = false;

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
	std::cout << "1. Paint" << std::endl << "2. InfraView" << std::endl;
	std::string input;
	std::getline(std::cin, input);

	if (input != "1" && input != "2")
	{
		std::cout << "Invalid Choice!, returning to menu..." << std::endl;
		return;
	}
	int answer = atoi(input.c_str());

	// now we can open the picture
	switch (answer)
	{
		case PAINT:
		{
			if (!openPaint(path))
			{
				return;
			}
			break;
		}
		case INFRAVIEW:
		{
			if (!openInfraView(path))
			{
				return;
			}
			break;
		}
	}

	std::string temp = (answer == PAINT) ? "Paint!" : "InfraView!";
	std::cout << "Picture Opened Using " << temp << std::endl;
}

/*
	Helper Method used to open a picture with Paint and keep our program encapsulated
	input: the path to the picture
	output: true if succeeded and false otherwise
*/

bool PictureProcessManager::openPaint(const std::string& path)
{
	return true;
}

/*
	Helper Method used to open a picture with InfraView and keep our program encapsulated
	input: the path to the picture
	output: true if succeeded and false otherwise
*/

bool PictureProcessManager::openInfraView(const std::string& path)
{
	return true;
}
