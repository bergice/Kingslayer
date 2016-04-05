#include "interface.h"

#include "main.h"
#include "program.h"
#include "player.h"
#include "team.h"
#include "boost\\lexical_cast.hpp"

#include "Commctrl.h"
#pragma comment(lib, "Comctl32.lib")

Interface* form = NULL;

Interface::Interface (HINSTANCE hInst)
{
	form=this;
	
	conlog.open("console.log");
	conlog << "Starting log...\n";

	int cx = GetSystemMetrics(SM_CXSCREEN)/2;
	int cy = GetSystemMetrics(SM_CYSCREEN)/2;
	RECT area;
	area.left	= cx-(WINDOW_WIDTH/2);
	area.right	= cx+(WINDOW_WIDTH/2);
	area.top	= cy-(WINDOW_HEIGHT/2);
	area.bottom	= cy+(WINDOW_HEIGHT/2);
	wnd.MainWindow = CreateMainWindow(area, hInst);
}

int Interface::Loop ()
{
	MSG Msg;
    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}

void Interface::AddChatMessage(std::string message)
{
	std::string messageNL = message;

	ChatBuffer.push_back(messageNL);

	if (ChatBuffer.size()>MAX_CHAT_MESSAGES)
		ChatBuffer.pop_front();

	std::string output_msg;
	std::list<std::string>::iterator it;
	for (it=ChatBuffer.begin(); it!=ChatBuffer.end(); ++it)
	{
		output_msg.append(*it);
		output_msg.append("\r\n");
	}

	SendMessage(form->wnd.Chat,WM_SETTEXT,NULL,(LPARAM)output_msg.c_str());
}
void Interface::AddConsoleMessage(std::string message)
{
	std::string messageNL = message;

	ConsoleBuffer.push_back(messageNL);

	if (ConsoleBuffer.size()>MAX_CHAT_MESSAGES)
		ConsoleBuffer.pop_front();

	std::string output_msg;
	std::list<std::string>::iterator it;
	for (it=ConsoleBuffer.begin(); it!=ConsoleBuffer.end(); ++it)
	{
		output_msg.append(*it);
		output_msg.append("\r\n");
	}

	conlog << getTime() << " " << message << "\n";

	SendMessage(form->wnd.Console,WM_SETTEXT,NULL,(LPARAM)output_msg.c_str());
}

HWND CreateConsoleWindow (HWND parent, int x, int y, int width, int height, int idc_id, DWORD extraParams)
{
	HWND hwnd=CreateWindowEx(WS_EX_CLIENTEDGE,
		"EDIT",
		"",
		WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|extraParams,
		x, y, width, height,
		parent,
		(HMENU)idc_id,
		GetModuleHandle(NULL),
		NULL);
	SetDefaultFont(hwnd);

	return hwnd;
}
HWND CreateListboxWindow (HWND parent, int x, int y, int width, int height, int idc_id, DWORD extraParams)
{
	INITCOMMONCONTROLSEX icex;           // Structure for control initialization.
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    RECT rcClient;                       // The parent window's client area.

    GetClientRect (parent, &rcClient); 

    // Create the list-view window in report view with label editing enabled.
    HWND hWndListView = CreateWindow(WC_LISTVIEW, 
                                     "",
                                     WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_BORDER| extraParams,
                                     x, y, width, height,
                                     parent,
                                     (HMENU)idc_id,
                                     GetModuleHandle(NULL),
                                     NULL); 

    return (hWndListView);
}

HWND CreateLabel (HWND parent, int x, int y, int width, int height, int idc_id, DWORD extraParams, char* text)
{
	HWND hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,
								"STATIC",
								text,
								WS_CHILD|WS_VISIBLE,
								x, y, width, height,
								parent,
								(HMENU)idc_id,
								GetModuleHandle(NULL),
						NULL);
	SetDefaultFont(hwnd);
	return hwnd;
}

inline BOOL ListBoxSetTabStops(const HWND hdlg, const int id, const int cTabs, const LPINT nTabs)
{
	return SendDlgItemMessage(hdlg, id, LB_SETTABSTOPS, (WPARAM) cTabs, (LPARAM) nTabs);
}

inline int ListBoxAddString(const HWND hdlg, const int id, LPCSTR s)
{
	return SendDlgItemMessage(hdlg, id, LB_ADDSTRING, 0, (LPARAM) s);
}
HWND CreatePlayerInfo (HWND parent, int x, int y, int width, int height)
{
	HWND listbox = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "List View", WS_CHILD | WS_VISIBLE | LBS_STANDARD | LBS_SORT | LBS_USETABSTOPS,
				x, y, width, height, parent, (HMENU)IDC_PLAYER_INFO, (HINSTANCE) GetWindowLong (parent, GWL_HINSTANCE), NULL);
	SetDefaultFont(listbox);

	int	nTabs[] = { 20, 50, 80 };
   	ListBoxSetTabStops(parent, IDC_PLAYER_INFO, 3, nTabs);

	return listbox;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
		case WM_CREATE:{
			// Chat Window
			form->wnd.Chat = CreateConsoleWindow (hwnd,			50, 100, 500, MAX_CHAT_MESSAGES*14,		IDC_CONSOLE_WINDOW, ES_READONLY|ES_MULTILINE);

			// Console Window
			form->wnd.Console = CreateConsoleWindow (hwnd,		50, 100+MAX_CHAT_MESSAGES*16, 500, MAX_CHAT_MESSAGES*14, IDC_CONSOLE_WINDOW, ES_READONLY|ES_MULTILINE);

			// Input Window
			form->wnd.Input = CreateConsoleWindow (hwnd, 50, 100+MAX_CHAT_MESSAGES*14, 500, 18, IDC_INPUT_WINDOW, ES_WANTRETURN|ES_MULTILINE|ES_AUTOVSCROLL);

			// Player List Window
			form->wnd.Players = CreatePlayerInfo(hwnd, 550, 100, 230, 18*TEXT_HEIGHT);

			// Player Label
			form->wnd.PlayerLabel = CreateLabel(hwnd, 550, 100-TEXT_HEIGHT, 80, TEXT_HEIGHT, NULL, NULL, "Players: ");
			// Players Offline Label
			form->wnd.PlayersOffline = CreateLabel(hwnd, 550, 100-(TEXT_HEIGHT*2), 80+40, TEXT_HEIGHT, NULL, NULL, "Players Logged: ");
			// Player ID Label
			HWND idlabel = CreateLabel(hwnd, 550+80, 100-TEXT_HEIGHT, 40, TEXT_HEIGHT, NULL, NULL, "ID");
			// Player Team Label
			HWND teamLabel = CreateLabel(hwnd, 550+80+40, 100-TEXT_HEIGHT, 70, TEXT_HEIGHT, NULL, NULL, "Team");
			// Player Warning label
			HWND warningLabel = CreateLabel(hwnd, 550+80+40+70, 100-TEXT_HEIGHT, 40, TEXT_HEIGHT, NULL, NULL, "Warn");
		}break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_CONSOLE_WINDOW:{
				}break;
				case IDC_INPUT_WINDOW:{
					if (GetAsyncKeyState(VK_RETURN)&0x8000)
					{
						std::string text = GetHwndText(form->wnd.Input);
						SendMessage(form->wnd.Input, WM_SETTEXT, 0, (LPARAM)"");
						sendRcon(text);
					}
				}break;
				case IDC_PLAYER_INFO:{
				}break;
			}
		break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
			form->conlog << "Ending log...\n";
			form->conlog.close();
			delete program;
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

HWND CreateMainWindow (RECT area, HINSTANCE hInst)
{
	WNDCLASSEX wc;
    HWND hwnd;

    //Step 1: Registering the Window Class
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = 0;//hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "Window Class";
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        exit(EXIT_FAILURE);
    }

    // Step 2: Creating the Window
    hwnd = CreateWindowEx(	WS_EX_CLIENTEDGE,
							"Window Class",
							"Kingslayer - Beta",
							WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
							area.left, area.top, 
							area.right-area.left, area.bottom-area.top,
							NULL, NULL, hInst/*hInstance*/, NULL);

    if(!hwnd)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        exit(EXIT_FAILURE);
    }

    RefreshWindow(hwnd);

	return hwnd;
}

void RefreshWindow (HWND hwnd)
{
	ShowWindow(hwnd, true/*nCmdShow*/);
    UpdateWindow(hwnd);
}

void SetDefaultFont (HWND hwnd)
{
	HGDIOBJ hfDefault=GetStockObject(DEFAULT_GUI_FONT);
	SendMessage(hwnd,WM_SETFONT,(WPARAM)hfDefault,MAKELPARAM(FALSE,0));
}

std::string GetHwndText (HWND hwnd)
{
    int length = SendMessage(hwnd,WM_GETTEXTLENGTH,0,0);
    if(length == -1)
        return "";
    char* buffer = new char[length+1];
    SendMessage(hwnd,WM_GETTEXT,length+1,(LPARAM)buffer);
    std::string str(buffer);
    delete[] buffer;

    return str;
}

std::string convertInt(int number)
{
	std:: stringstream ss;//create a stringstream
	ss << number;//add number to the stream
	return ss.str();//return a string with the contents of the stream
}

void Interface::UpdatePlayerList ()
{
	// Remove previous players
	SendDlgItemMessage (form->wnd.MainWindow, IDC_PLAYER_INFO, LB_RESETCONTENT, 0, 0);

	// Add info for each player
	std::string info;
	for (unsigned i=0 ; i<program->players.size(); i++)
	{
		info = "";
		info.append(program->players[i]->name);			info.append("\t");
		info.append(convertInt(program->players[i]->id));			info.append("\t");
		if (program->players[i]->team)
			info.append(program->players[i]->team->name);
		else
			info.append("-");
		info.append("\t");
		info.append(convertInt(program->players[i]->warnings)); info.append("\t");

   		ListBoxAddString(form->wnd.MainWindow, IDC_PLAYER_INFO, info.c_str());
	}

	// Update player count
	std::string label = "Players: ";
	label.append(boost::lexical_cast<std::string>(program->players.size()));
	SendMessage(form->wnd.PlayerLabel, WM_SETTEXT, 0, (LPARAM)label.c_str()); // for Win32 windows

	// Update players logged offline
	std::string labelLogged = "Players Logged: ";
	labelLogged.append(boost::lexical_cast<std::string>(program->players_offline.size()));
	SendMessage(form->wnd.PlayersOffline, WM_SETTEXT, 0, (LPARAM)labelLogged.c_str()); // for Win32 windows
}


// Thread for autorefresh
void UpdateForms (void)
{
	while(true)
	{
		form->UpdatePlayerList();
		Sleep(1000*5);
	}
}