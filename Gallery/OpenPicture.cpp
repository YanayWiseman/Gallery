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
	if (pictureOpen && procInfo.hProcess != nullptr)
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