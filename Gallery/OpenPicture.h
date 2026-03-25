#pragma once
#include <Windows.h>
#include <csignal>
#include <iostream>
#include <string>

#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <iomanip>
#include <sstream>
#include <thread>
#include <atomic>

#define PAINT 1
#define INFRAVIEW 2

class PictureProcessManager
{
private:
	// fields
	
	// PROCESS_INFORMATION is a struct that contains information about a certain process
	// like the handles to the process and its thread and the process and thread id.
	// we use it to track and terminate the picture's process
	PROCESS_INFORMATION _procInfo;
	std::atomic<bool> _pictureOpen;  // i dont want to use mutexes

	// static pointer to keep the adress of the active instance
	static PictureProcessManager* instance;

	// private method to open the picture using an external application
	// we use helper to keep our code encaplsulated and private
	bool openExternalApplication(const std::string& path);

	std::string getLastModificationTime(const std::string& path) const;

	// thread function
	void monitorProcess(const std::string& path);
public:
	// methods

	// c'tor and d'tor
	PictureProcessManager();
	~PictureProcessManager();

	static void staticHandler(int signum);
	void closePictureProcess(int signum);
	void openPicture(const std::string& path);
};


