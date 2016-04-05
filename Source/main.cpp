#include "main.h"
#define WIN32_LEAN_AND_MEAN
#include "program.h"
#include <windows.h>
#include "interface.h"

//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
int main(int argc, char* argv[])
{
	// Create the user interface
	form = new Interface();

	// Create the program class and connect to the server in a new thread
	program = new Program();
	program->SetupTeamsAndLoadPlayerStats ();
	program->StartThreads ();
	boost::thread_group tgroup;
	tgroup.create_thread(boost::bind(&ConnectionThread,program));
	Sleep(150);

	// Fetch startup settings from ini configuration
	program->ReadConfig();

	// Start the program loop
	form->Loop();

	// Loop stopped, exit program
	return 0;
}