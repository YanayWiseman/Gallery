#pragma once
#include <Windows.h>
#include <csignal>
#include <iostream>
#include <string>

class PictureProcessManager
{
private:
	// fields
	
	// PROCESS_INFORMATION is a struct that contains information about a certain process
	// like the handles to the process and its thread and the process and thread id.
	// we use it to track and terminate the picture's process
	PROCESS_INFORMATION procInfo;
	bool pictureOpen;

	// static pointer to keep the adress of the active instance
	static PictureProcessManager* instance;

public:
	// methods

	// c'tor and d'tor
	PictureProcessManager();
	~PictureProcessManager();

	static void staticHandler(int signum);
	void closePictureProcess(int signum);

	void openPicture(const std::string& path);
};


