#include <iostream>
#include <string>
#include <sstream>
#include <Windows.h>
#include <csignal>
#include "MemoryAccess.h"
#include "AlbumManager.h"
#include "DatabaseAccess.h"
#include "OpenPicture.h"
#include "GalleryGUI.h"


int getCommandNumberFromUser()
{
	std::string message("\nPlease enter any command(use number): ");
	std::string numericStr("0123456789");

	std::cout << message << std::endl;
	std::string input;
	std::getline(std::cin, input);
	
	while (std::cin.fail() || std::cin.eof() || input.find_first_not_of(numericStr) != std::string::npos)
	{

		std::cout << "Please enter a number only!" << std::endl;

		if (input.find_first_not_of(numericStr) == std::string::npos)
		{
			std::cin.clear();
		}

		std::cout << std::endl << message << std::endl;
		std::getline(std::cin, input);
	}
	
	return std::atoi(input.c_str());
}



int main(void)
{
	// the signal function is a function that detects interrupts in the program
	// the SIGINT flag detects a signak that is sent to the program in order to end it
	// closePictureProcess is a function that I have coded, that is called automatically when
	// the interrupt is triggered, it gets the signal number (used in the exit function)
	// in the function i can do what ever i want when CTRL+C is pressed, in this case, close the opened picture's proccess
	std::signal(SIGINT, PictureProcessManager::staticHandler);

	// initialization data access
	DatabaseAccess dataAccess;
	dataAccess.open();

	std::cout << "Welcome to Gallery!" << std::endl;
	std::cout << "\n=========================" << std::endl;

	// let the user choose weather he wants CLI or GUI
	std::cout << "Select launch mode:" << std::endl;
	std::cout << "  1. CLI (command line)" << std::endl;
	std::cout << "  2. GUI (graphic window)" << std::endl;
	std::cout << "Choice: ";

	std::string modeInput;
	std::getline(std::cin, modeInput);

	int mode = 0;
	try
	{
		mode = std::stoi(modeInput);
	}
	catch (...)
	{
		mode = 1;
	}

	std::unique_ptr<IGalleryApp> app;

	if (mode == 2)
	{
		std::cout << "Opening GUI..." << std::endl;
		app = std::make_unique<GalleryGUI>(dataAccess);
	}
	else
	{
		std::cout << "Starting CLI..." << std::endl;
		app = std::make_unique<AlbumManager>(dataAccess);
	}

	// for CLI, AlbumManager::run()
	// for GUI, GalleryGUI::run()
	app->run();

	dataAccess.clear();
}


