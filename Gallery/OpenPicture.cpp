#include "OpenPicture.h"

// PROCESS_INFORMATION is a struct that contains information about a certain process
// like the handles to the process and its thread and the process and thread id.
// we use it to track and terminate the picture's process
PROCESS_INFORMATION procInfo = { 0 };
bool pictureOpen = true;

/*
	Fucntion to manipulate the CTRL+C interrupt
	input: the signal number (handed from the signal() function)
	output: no return values
*/

void closePictureProcess(int signum)
{
	// if paint or infraview is open and theres a proccess open we close it
	if (pictureOpen && procInfo.hProcess != nullptr)
	{
		std::cout << "Ctrl+C was typed when an image was open, closing it" << std::endl;

		// terminate the proccess and close its handles
		TerminateProcess(procInfo.hProcess, 0);
		CloseHandle(procInfo.hProcess);
		CloseHandle(procInfo.hThread);

		pictureOpen = false;
	}
	// else we exit
	else
	{
		std::cout << "Ctrl+C was typed when no image was open, existing the program" << std::endl;
		exit(signum);
	}

	// the signal function 'loads' only once every call, so we call it every time we need to
	// get ready for the next interrupt
	signal(SIGINT, closePictureProcess);
}