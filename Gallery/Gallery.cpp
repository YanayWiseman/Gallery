#include <iostream>
#include <string>
#include <sstream>
#include <Windows.h>
#include <csignal>
#include "MemoryAccess.h"
#include "AlbumManager.h"
#include "DatabaseAccess.h"
#include "OpenPicture.h"

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

/*
	function to print the system's information suck as main developer and current time and hour
	input: no input values
	output: no return values
*/

void printSystemInfo()
{
	std::cout << "Chief Developer: Yanay Wiseman\nCurrent date:" << std::endl;

	time_t now = time(nullptr);
	std::stringstream oss;
	oss << std::put_time(localtime(&now), "%d/%m/%Y %H:%M:%S");
	std::cout << oss.str() << std::endl;

	std::cout << "=========================\n" << std::endl;
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

	// initialize album manager
	AlbumManager albumManager(dataAccess);

	std::string albumName;
	std::cout << "Welcome to Gallery!" << std::endl;
	std::cout << "\n=========================" << std::endl;

	printSystemInfo();

	std::cout << "Type " << HELP << " to a list of all supported commands" << std::endl;
	
	do
	{
		int commandNumber = getCommandNumberFromUser();

		try
		{
			albumManager.executeCommand(static_cast<CommandType>(commandNumber));
		} 
		catch (std::exception& e)
		{	
			std::cout << e.what() << std::endl;
		}
	} 
	while (true);

	dataAccess.clear();
}


