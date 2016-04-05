#pragma once

#include "telnet_client.h"
#include "boost\\thread.hpp"

#include <vector>
#include <string>

#include "misc.h"

#include "sender.h"
#include "receiver.h"

struct sTeam;
struct sPlayer;
struct sVote;
struct sFreeze;

enum
{
	MSG_REGULAR,
	MSG_WARNING,
	MSG_ERROR
};
void Message (std::string msg, short type = MSG_REGULAR);

enum
{
	D_NULL,
	D_GENERAL_INFO,
	D_PLAYER_INFO,
	D_VOTE_INFO,
	D_FREEZE_INFO
};

class Program
{
private:	int autosaveDelay;
			void AutosaveThread (void);

public:		telnet_client* c;
			boost::thread* t;

			int PrintAll;
			int AutosaveDelay;

			boost::thread_group tgroup;

			bool switchPlayers;


			std::string strbuffer;

			char buffer[MAX_PATH];
			std::string configPath;


public:		Program ();
			~Program ();

			void SetupTeamsAndLoadPlayerStats ();

			void StartThreads ();
			
			bool debug;
			int refreshDelay;

			std::vector <sTeam*> teams;
			int GetTeamIndexByTeamName (std::string name);
			std::vector <sPlayer*> players;
			std::vector <sPlayer*> players_offline;
			std::vector <sVote*> voteking;
			std::vector <sVote*> votekick;
			std::vector <sFreeze> frozen;

			std::vector<std::string> admins;

			void LoadAdmins (std::string configPath);
			bool GetAdmin (std::string name);

			void Load (const char* filename);
			void SaveBackup (void);
			void SavePlayers (void);
			void LoadPlayers (void);

			void Connection (void);

			void ReadConfig (void);

			void PushMessage(std::string read);
};
void ConnectionThread(Program* p);
extern Program* program;