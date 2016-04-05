#include "program.h"

#include "boost\\filesystem.hpp"
#include "boost\\lexical_cast.hpp"
using namespace boost::filesystem;
using namespace boost::filesystem3;
#include <iostream>

#include "team.h"
#include "player.h"
#include "vote.h"
#include "freeze.h"

#include "file.h"

#include "interface.h"
#include "sender.h"

#include "zombies.h"


Program* program=NULL;

Program::Program()
{
	configPath="";
	DWORD read = GetCurrentDirectory(sizeof(buffer),buffer);
	if (read)
		configPath.append(buffer, read);
	configPath.append("\\config.ini");

	switchPlayers=false;
}

void Program::SetupTeamsAndLoadPlayerStats ()
{
	CreateDefaultTeams(configPath);

	//LoadPlayers();
}

void Program::StartThreads ()
{
	boost::thread_group tgroup;

	tgroup.create_thread( boost::bind(&SwapThread) ); // To swap players properly
	tgroup.create_thread( boost::bind(&FreezeThread) ); // For auto-unfreezing
	tgroup.create_thread( boost::bind(&RulesThread) ); // To broadcast automatic messages

	tgroup.create_thread( boost::bind(&Program::AutosaveThread, this) ); // For saving player and game data

	tgroup.create_thread( boost::bind(&ZombieEventThread) ); // Timer to start zombie events

	tgroup.create_thread( boost::bind(&UpdateForms) ); // To update the windows interface data
}

Program::~Program()
{
	delete c;
}

int Program::GetTeamIndexByTeamName (std::string name)
{
	for (unsigned i=0; i<teams.size(); i++)
	{
		if (strcmp(name.c_str(), teams[i]->name.c_str())==0)
			return i;
	}
	return -1;
}

void Program::Load (const char* fn)
{
	std::string fname = ExePath();
	//fname = "C:\\Users\\bergice\\Desktop\\";
	fname.append(fn);

	HANDLE file = OpenToRead(fname.c_str());
	short byte;
	bool eof=false;
	if (file)
	{
		players.clear();
		for(unsigned i=0; i<teams.size(); i++)
			teams[i]->players.clear();
		do
		{
			byte = ReadByte(file, &eof);
			switch(byte)
			{
				case D_GENERAL_INFO:
				{
					debug = ReadByte(file) ? true: false;
					refreshDelay = ReadWord(file);
				}
				break;
				case D_PLAYER_INFO:
				{
					sPlayer* plr = new sPlayer("NewPlayer");
					plr->Load(file);
					players.push_back(plr);

					if (debug)
						std::cout << "Read player. Fetched name: " << plr->name << " with PID: " << plr->id << std::endl;
				}
				break;
			}
		}
		while (byte && !eof);

		CloseHandle(file);
	}
}

void Program::SaveBackup ()
{
	std::string fname = ExePath();
	//fname = "C:\\Users\\bergice\\Desktop";
	fname.append("\\restore.dat");

	if (exists(path(fname)))
		remove(fname.c_str());
	HANDLE file = OpenToWrite(fname.c_str());
	if (file)
	{
		WriteByte(file, D_GENERAL_INFO);
			WriteByte(file, (int)debug);
			WriteByte(file, refreshDelay);
		CloseHandle(file);
	}
}

void Program::SavePlayers ()
{
	std::string fname = ExePath();
	fname.append("\\players.dat");

	if (exists(path(fname)))
		remove(fname.c_str());
	HANDLE file = OpenToWrite(fname.c_str());
	if (file)
	{
		for (unsigned i=0; i<players.size(); i++)
		{	
			WriteByte(file, D_PLAYER_INFO);
			players[i]->Save(file);
		}

		for (unsigned i=0; i<players_offline.size(); i++)
		{	
			WriteByte(file, D_PLAYER_INFO);
			players_offline[i]->Save(file);
		}

		CloseHandle(file);
	}
}

void Program::LoadPlayers ()
{
	std::string fname = ExePath();
	fname.append("\\players.dat");

	HANDLE file = OpenToRead(fname.c_str());
	if (file)
	{
		bool eof = false;
		int type = ReadByte(file, &eof);
		while (!eof)
		{
			sPlayer* plr = new sPlayer("");
			plr->Load(file);
			program->players_offline.push_back(plr);
			type = ReadByte(file, &eof);
		}
		switchPlayers = true;
		CloseHandle(file);
	}
}

void Program::Connection ()
{		
	try
	{
		boost::asio::io_service io_service;
		// resolve the host name and port number to an iterator that can be used to connect to the server
		tcp::resolver resolver(io_service);

		// Get port and address to connect to
		std::string configPath="";
		char buffer[MAX_PATH];
		DWORD read = GetCurrentDirectory(sizeof(buffer),buffer);
		if (read)
			configPath.append(buffer, read);
		configPath.append("\\config.ini");

		std::string address="127.0.0.1";
		std::string port="50301";

		if (GetPrivateProfileString("DEFAULT", "Address", "127.0.0.1", buffer, sizeof(buffer), configPath.c_str()))
			address = buffer;
		if (GetPrivateProfileString("DEFAULT", "Port", "50301", buffer, sizeof(buffer), configPath.c_str()))
			port = buffer;

		// Connect
		tcp::resolver::query query(address, port);
		tcp::resolver::iterator iterator = resolver.resolve(query);
		// define an instance of the main class of this program
		c = new telnet_client(io_service, iterator);
		// run the IO service as a separate thread, so the main thread can block on standard input
		t = new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service));

		while (c->active()) // check the internal state of the connection to make sure it's still running
		{
			char ch;
			cin.get(ch); // blocking wait for standard input
			if (ch == 3) // ctrl-C to end program
				break;
			c->write(ch);
			//c->writeString();

			//if (c->tempmsg)
			//{
			//	c->write_startString();//c->writeString();
			//}
		}

		c->close(); // close the telnet client connection
		t->join(); // wait for the IO service thread to close
	}
	catch (exception& e)
	{
		cerr << "Exception: " << e.what() << "\n";
	}
}

void ConnectionThread(Program* p)
{
	p->Connection();
}


void UpdateFormsInit ()
{
	Sleep(1000);
	form->UpdatePlayerList();
}
void UpdateFormsInitThread ()
{
	boost::thread_group tgroup;
	tgroup.create_thread(boost::bind(&UpdateFormsInit));
}

void Program::LoadAdmins (std::string configPath)
{
	admins.clear();
	if (GetPrivateProfileString("DEFAULT", "Admins", "", buffer, sizeof(buffer), configPath.c_str()))
	{
		std::string str = buffer;
		size_t pos = str.find_first_of("; ");
		while (pos!=std::string::npos)
		{
			admins.push_back (str.substr(0, pos));
			str.erase (0, pos+2);

			pos = str.find_first_of("; ");
		}
	}
}

bool Program::GetAdmin (std::string name)
{
	for (unsigned i=0; i<admins.size(); i++)
	{
		if ( strcmp(admins[i].c_str(), name.c_str()) == 0 )
			return true;
	}
	return false;
}

void Program::ReadConfig (void)
{
	SetFTPSettings(configPath);

	if (GetPrivateProfileString("DEFAULT", "Password", "", buffer, sizeof(buffer), configPath.c_str()))
		sendRcon(buffer);
	PrintAll = GetPrivateProfileInt("DEFAULT", "PrintAll", 0, configPath.c_str());
	AutosaveDelay = GetPrivateProfileInt("DEFAULT", "AutosaveDelay", 60000, configPath.c_str());
	EnablePunishing		( ((GetPrivateProfileInt("DEFAULT", "PunishPlayers", 1, configPath.c_str()))		? true : false) );
	EnableBroadcasting  ( ((GetPrivateProfileInt("DEFAULT", "EnableBroadcasting", 1, configPath.c_str()))	? true : false) );
	int r=1;
	while(r)
	{
		std::string key = "Settings"; key.append(boost::lexical_cast<std::string>(r));
		if (GetPrivateProfileString("DEFAULT", key.c_str(), "", buffer, sizeof(buffer), configPath.c_str()))
		{
			sendRcon(buffer);
			r++;
		}
		else
			r=0;
	}

	// Add admins
	LoadAdmins (configPath);

	ZombieSetup(configPath);
	RemoveAllSeclev("king.cfg");
	UpdateFormsInitThread();
}

void Program::PushMessage(std::string read)
{
	strbuffer.append(read);
	int find;
	do
	{
		find = strbuffer.find_first_of("\n");
		if (find!=std::string::npos)
		{
			std::string extracted = strbuffer.substr(0, find);
			strbuffer.erase(0, find+1);
			if (!PrintAll)
			{
				if (extracted.length()>=11)
				{
					if (extracted[11]=='<')
						form->AddChatMessage(extracted);
				}
			}
			else
				form->AddChatMessage(extracted);
			ProcessMessage(extracted);
		}
	}while(find!=std::string::npos);
}

void Program::AutosaveThread (void)
{
	Sleep(6000);
	while(true)
	{
		//SaveBackup();
		SavePlayers();
		Sleep(AutosaveDelay);
	}
}