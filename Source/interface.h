#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <list>
#include <string>
#include <fstream>

#define TEXT_HEIGHT				18

#define MAX_CHAT_MESSAGES		10
#define MAX_CONSOLE_MESSAGES	160

#define IDC_CONSOLE_WINDOW		101
#define IDC_PLAYER_INFO			104
#define IDC_INPUT_WINDOW		103

struct SWnd
{
	HWND MainWindow;
	HWND Chat;
	HWND Console;
	HWND Players;
	HWND Input;
	HWND PlayerLabel;
	HWND PlayersOffline;
};

class Interface
{
private:	std::list<std::string> ChatBuffer;
			std::list<std::string> ConsoleBuffer;

public:		SWnd wnd;

public:		Interface (HINSTANCE hInst=NULL);
			int Loop ();
			
			void UpdatePlayerList ();

			void AddChatMessage(std::string message);
			void AddConsoleMessage(std::string message);

			std::ofstream conlog;
};
extern Interface* form;

// Thread for autorefresh
void UpdateForms (void);

// The main protocol for the window
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Create the console editbox
HWND CreateConsoleWindow (HWND parent, RECT area, int idc_id, DWORD extraParams=NULL);

// Create the main window
HWND CreateMainWindow (RECT area, HINSTANCE hInst=NULL);

// Quickly show and refresh window
void RefreshWindow (HWND hwnd);

// Sets default font for window
void SetDefaultFont (HWND hwnd);

// Gets text from a hwnd
std::string GetHwndText (HWND hwnd);