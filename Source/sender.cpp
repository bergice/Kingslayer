#include "sender.h"

#include <iostream>
#include "misc.h"

#include "program.h"

#include "boost\\lexical_cast.hpp"
#include "boost\\thread.hpp"

#include "freeze.h"
#include "player.h"
#include "team.h"

#include "main.h"
#include "interface.h"

#include <vector>

#include "FTPClient.h"

#include "Shlwapi.h"
#pragma comment(lib, "Shlwapi.lib")

std::vector <int> swapQueue;
DWORD lastSwap = 0;

struct FTPStruct
{
	std::string Address;
	int Port;
	std::string Username;
	std::string Password;
	std::string BaseFolder;
	std::string DownloadFolder;
} ftp;

bool enableBroadcast = true;
void sendMsg (std::string message)
{
	if (enableBroadcast)
	{
		std::string output = "/msg \"";
		output.append(message);
		output.append("\"");
		sendRcon(output);
	}
}
void EnableBroadcasting (bool state)
{
	enableBroadcast=state;
}




void sendRcon (std::string message)
{
	program->c->writeString(message.c_str());
}


void SwapThread ()
{
	lastSwap = 0;
	while (true)
	{
		while(!swapQueue.empty())
		{
			if (GetTickCount()>lastSwap+1)
			{
				std::string str = "/swapid ";
				char buffer[10]; sprintf(buffer, "%i", swapQueue[0]);
				str.append(buffer);
				swapQueue.erase(swapQueue.begin());
				lastSwap=GetTickCount();
				sendRcon(str);
			}
		}
		Sleep(10);
	}
}

void swapidThread (int id)
{
	//swapQueue.push_back(id);
	//while (GetTickCount()<lastSwap+100 || swapQueue[0]!=id)
	//{}
	//std::string str = "/swapid ";
	//char buffer[10]; sprintf(buffer, "%i", id);
	//str.append(buffer);
	//swapQueue.erase(swapQueue.begin());
	//lastSwap=GetTickCount();
	//sendRcon(str);
}

void swapid (int id)
{
	swapQueue.push_back(id);
	//boost::thread_group tgroup;
	//tgroup.create_thread(boost::bind(swapidThread, id));
}

void playSound (int x, int y, std::string sound)
{
	std::string output = "addBlob(`room`, `";
	output+=sound;
	output+="`, ";
	output+=boost::lexical_cast<std::string>(x);
	output+=", ";
	output+=boost::lexical_cast<std::string>(y);
	output+=", 2);";
	sendRcon(output);
}
void addBot    ()
{

}



void WelcomeThread (std::string playername)
{
	Sleep(3000);
	sendMsg("Welcome, " + playername + " to " + SERVER_NAME);
	sendRcon("/players");
	form->UpdatePlayerList();
}
void DoWelcomeMessage (std::string playername)
{
	boost::thread_group tgroup;
	tgroup.create_thread(boost::bind(&WelcomeThread, playername));
}


// New commands
bool punishPlayers = true;
void Ban (sPlayer* plr, int minutes, char* reason)
{
	if (punishPlayers)
	{
		std::string msg = "/banid ";
		msg.append( boost::lexical_cast<std::string>(plr->id) );
		msg += " ";
		msg.append( boost::lexical_cast<std::string>(minutes) );
		sendRcon(msg);
		sendMsg(plr->name + " banned for " + boost::lexical_cast<std::string>(minutes) + " minutes. Reason: '" + reason + "'");
	}
	else
		std::cout << "INTERCEPTED BAN" << std::endl;
}

void Kick (sPlayer* plr, char* reason)
{
	if (punishPlayers)
	{
		std::string msg = "/kickid ";
		msg.append( boost::lexical_cast<std::string>(plr->id) );
		sendRcon(msg);
		sendMsg(plr->name + " kicked for reason '" + reason + "'");
	}
	else
		std::cout << "INTERCEPTED KICK" << std::endl;
}

void Freeze (sPlayer* plr, int seconds, char* reason)
{
	if (punishPlayers)
	{
		sFreeze f;
		f.player = plr;
		time_t t; time(&t);
		tm tim = *localtime(&t);
		tim.tm_sec+=seconds;
		t=mktime(&tim);
		f.time = t;
		program->frozen.push_back(f);

		std::string msg = "/freezeid ";
		msg.append( boost::lexical_cast<std::string>(plr->id) );
		sendRcon(msg);

		std::cout << "Frozen player " << plr->name << " with reason: " << reason << std::endl;
	}
	else
		std::cout << "INTERCEPTED FREEZE" << std::endl;
}

void Unfreeze (sPlayer* plr)
{
	std::string msg = "/unfreezeid ";
	msg.append( boost::lexical_cast<std::string>(plr->id) );
	sendRcon(msg);
}


void Mute (sPlayer* plr, int minutes)
{
	std::string msg = "/mute ";
	msg.append( plr->name );
	msg.append(" ");
	msg.append( boost::lexical_cast<std::string>(minutes) );
	sendRcon(msg);
}

void Unmute (sPlayer* plr)
{
	std::string msg = "/unmute ";
	msg.append( plr->name );
	sendRcon(msg);
}


void EnablePunishing (bool state)
{
	punishPlayers=state;
}


void RulesThread (void)
{
	int MSG_DELAY = 10000;
	unsigned short index=0;
	std::vector<std::string> rules;
	rules.push_back("Write '/help' for a list of commands. *"); // 0
	rules.push_back("Write '/king <PLAYER>' to vote for a King. *"); // 1
	rules.push_back("Write '/war <TEAM>' as a King to declare war on a race. *"); // 2
	rules.push_back("To win a war, kill the enemy King! *"); // 3
	rules.push_back("Write '/wars' to see who you are at war with. *"); // 4
	rules.push_back("Visit us at: 'kagrp.net' to view achievements, ranks and keep updated! *"); // 5
	rules.push_back("<<<WARS>>>"); // 6
	rules.push_back("<<<KINGS>>>"); // 7
	Sleep(MSG_DELAY);
	while(true)
	{
		// War messages
		if (index==6)
		{
			std::string msg;
			for (unsigned i=1; i<program->teams.size(); i++)
			{
				// The team is currently at war with someone
				if ( !program->teams[i]->enemies.empty() )
				{
					msg="";
					msg.append(program->teams[i]->namePlural);
					msg.append(" are at WAR with: ");
					for (unsigned e=0; e<program->teams[i]->enemies.size(); e++)
					{
						msg.append(program->teams[i]->enemies[e]->namePlural);
						if (e!=program->teams[i]->enemies.size()-1)
							msg.append(" and ");
					}
					sendMsg(msg);
				}
			}
		}
		// King messages
		if (index==7)
		{
			std::string msg;
			for (unsigned i=1; i<program->teams.size(); i++)
			{
				if (program->teams[i]->king)
				{
					msg="";
					msg.append(program->teams[i]->name);
					msg.append(" KING is currently ");
					msg.append(program->teams[i]->king->name);
					sendMsg(msg);
				}
			}
		}
		std::string msg = rules[index];
		if (index<=5)
			sendMsg(msg);
		index++;
		if (index>rules.size()-1)
			index=0;
		Sleep(MSG_DELAY);
	}
}


void _assignseclev (std::string filename, std::string username, std::string list)
{
	std::string fName = ftp.DownloadFolder; fName+=filename.c_str();
	std::ifstream file (fName.c_str(), std::ifstream::binary);
	if (file)
	{
		file.seekg (0, file.end);
		int length = file.tellg();
		file.seekg (0, file.beg);

		char* buffer = new char[length];
		file.read(buffer, length);
		file.close();

		std::string str;
		for(unsigned i=0; i<(unsigned)length; i++)
			str.push_back(buffer[i]);
		int pos;

		std::string usernameComplete = username;

		 // Remove Duplicates
		pos = str.find(usernameComplete);
		while(pos != std::string::npos)
		{
			str.erase(pos, usernameComplete.length());
			pos = str.find(usernameComplete);
		}

		 // Add the user now
		pos = str.find(list);
		if (pos != std::string::npos)
			str.insert(pos+list.length(), usernameComplete);

		//std::cout << str.c_str() << std::endl;

		std::ofstream ofile(fName.c_str(), ios_base::out |std::ofstream::binary);
		ofile.write(str.c_str(), str.length());
		ofile.close();

		delete[] buffer;
	}
}

void _removeseclev (std::string filename, std::string username, std::string list)
{
	std::string fName = ftp.DownloadFolder; fName+=filename.c_str();
	std::ifstream file (fName.c_str(), std::ifstream::binary);
	if (file)
	{
		file.seekg (0, file.end);
		int length = file.tellg();
		file.seekg (0, file.beg);

		char* buffer = new char[length];
		file.read(buffer, length);
		file.close();

		std::string erasePart = username;

		std::string str;
		for(unsigned i=0; i<(unsigned)length; i++)
			str.push_back(buffer[i]);
		int pos = str.find(erasePart);
		while(pos != std::string::npos)
		{
			str.erase(pos, erasePart.length());
			pos = str.find(erasePart);
		}

		std::ofstream ofile(fName.c_str(), ios_base::out |std::ofstream::binary);
		ofile.write(str.c_str(), str.length());
		ofile.close();

		delete[] buffer;
	}
}

void _removeallseclev (std::string filename, std::string list)
{
	std::string fName = ftp.DownloadFolder; fName+=filename.c_str();
	std::ifstream file (fName.c_str(), std::ifstream::binary);
	if (file)
	{
		// Copy the file to buffer
		file.seekg (0, file.end);
		int length = file.tellg();
		file.seekg (0, file.beg);

		char* buffer = new char[length];
		file.read(buffer, length);
		file.close();

		std::string str;
		for(unsigned i=0; i<(unsigned)length; i++)
			str.push_back(buffer[i]);

		// Find start point
		int startPoint = str.find(list);
		if (startPoint != std::string::npos)
		{
			startPoint += list.length();
			std::string afterStart = str.substr (startPoint, str.length()-startPoint);
			std::size_t endPoint = afterStart.find_first_of ("\r\n");
			std::string extract = afterStart.substr(0, endPoint);
			if (endPoint != std::string::npos)
			{
				str.erase (startPoint, endPoint);
			}
			else
				form->AddConsoleMessage ("Could not find end point in _removeallseclev");
		}
		else
			form->AddConsoleMessage ("Could not find start point in _removeallseclev");

		// Write the new changed file
		std::ofstream ofile(fName.c_str(), ios_base::out |std::ofstream::binary);
		ofile.write(str.c_str(), str.length());
		ofile.close();

		delete[] buffer;
	}
}

void SetFTPSettings (std::string configPath)
{
	char buffer[MAX_PATH];

	if (GetPrivateProfileString("FTP", "Address", "127.0.0.1", buffer, sizeof(buffer), configPath.c_str()))
		ftp.Address = buffer;

	ftp.Port = GetPrivateProfileInt("FTP", "Port", 21, configPath.c_str());

	if (GetPrivateProfileString("FTP", "Username", "", buffer, sizeof(buffer), configPath.c_str()))
		ftp.Username = buffer;
	if (GetPrivateProfileString("FTP", "Password", "", buffer, sizeof(buffer), configPath.c_str()))
		ftp.Password = buffer;

	if (GetPrivateProfileString("FTP", "BaseFolder", "", buffer, sizeof(buffer), configPath.c_str()))
		ftp.BaseFolder = buffer;

	ftp.DownloadFolder="";
	DWORD read = GetCurrentDirectory(sizeof(buffer),buffer);
	if (read)
		ftp.DownloadFolder.append(buffer, read);
	ftp.DownloadFolder.append("\\Temp\\");
}

bool FTPUpload (std::string remotename, std::string localname)
{
	nsFTP::CFTPClient ftpClient;
	nsFTP::CLogonInfo logonInfo(ftp.Address, ftp.Port, ftp.Username, ftp.Password);
	if (!ftpClient.Login(logonInfo))
	{
		MessageBox(NULL, "Could not log onto FTP server!", "FTP Error", MB_OK|MB_ICONWARNING);
		return false;
	}
	ftpClient.Delete(ftp.BaseFolder+remotename);
	if (!ftpClient.UploadFile(ftp.DownloadFolder+localname, ftp.BaseFolder+remotename))
	{
		MessageBox(NULL, "Could not upload file to FTP!", "FTP Error", MB_OK|MB_ICONWARNING);
		return false;
	}
	ftpClient.Logout();
	return true;
}

bool FTPDownload (std::string remotename, std::string localname)
{
	//if (PathFileExists(localname.c_str()))
	//	DeleteFile(localname.c_str());

	nsFTP::CFTPClient ftpClient;
	nsFTP::CLogonInfo logonInfo(ftp.Address, ftp.Port, ftp.Username, ftp.Password);
	if (!ftpClient.Login(logonInfo))
	{
		MessageBox(NULL, "Could not log onto FTP server!", "FTP Error", MB_OK|MB_ICONWARNING);
		return false;
	}
	ftpClient.DownloadFile(ftp.BaseFolder+remotename, ftp.DownloadFolder+localname);
	//if (!ftpClient.DownloadFile(ftp.BaseFolder+remotename, ftp.DownloadFolder+localname))
	//{
	//	std::string fname = "Could not fetch file from FTP!\nREMOTE: '";
	//	fname += ftp.BaseFolder+remotename+"'\nLOCAL: '";
	//	fname += ftp.DownloadFolder+localname;
	//	MessageBox(NULL, fname.c_str(), "FTP Error", MB_OK|MB_ICONWARNING);
	//	return false;
	//}
	ftpClient.Logout();
	return true;
}


bool AssignSeclev (std::string username, std::string seclevFileName, std::string list)
{
	std::string remoteFileName = "Security/";
	remoteFileName += seclevFileName;

	if (!FTPDownload(remoteFileName, seclevFileName))
		return false;
	_assignseclev(seclevFileName, username+"; ", list);
	if (!FTPUpload(remoteFileName, seclevFileName))
		return false;
	sendRcon("/reloadseclevs");
	return true;
}

bool RemoveSeclev (std::string username, std::string seclevFileName, std::string list)
{
	std::string remoteFileName = "Security/";
	remoteFileName += seclevFileName;

	if (!FTPDownload(remoteFileName, seclevFileName))
		return false;
	_removeseclev(seclevFileName, username+"; ", list);
	if (!FTPUpload(remoteFileName, seclevFileName))
		return false;
	sendRcon("/reloadseclevs");
	return true;
}


bool RemoveAllSeclev (std::string seclevFileName, std::string list)
{
	std::string remoteFileName = "Security/";
	remoteFileName += seclevFileName;

	if (!FTPDownload(remoteFileName, seclevFileName))
		return false;
	_removeallseclev(seclevFileName, list);
	if (!FTPUpload(remoteFileName, seclevFileName))
		return false;
	sendRcon("/reloadseclevs");
	return true;
}