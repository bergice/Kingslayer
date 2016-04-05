#pragma once

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#include <string>

#define SLAY_ROCKS		1
#define SLAY_PUSH		2
#define SLAY_BOMB		3
#define SLAY_HAMMER		4
#define SLAY_ARROW		5
#define SLAY_SWORD		6

enum RankEnum
{
	Peasant,
	Recruit,
	Guard,
	Captain,
	Duke
};
struct Ranking
{
	RankEnum Rank;
	int Points;
};
struct Achievements
{
	// Ranking achievements
	bool Peasant; // Joined the server for the first time
	bool Recruit; // Reached rank Recruit
	bool Guard; // Reached rank Guard
	bool Captain; // Reached rank Captain
	bool Duke; // Reached rank Duke

	// Kill achievements
	bool Rampage; // Killed 20 enemies
	bool Kingslayer; // Killed 10 kings

	// Misc achievements
	bool ZombieSlayer; // Was the last player standing in a zombie apocalypse
	bool General; // Won 5 wars as king
	bool Insomnia; // Has played 12 hours on the server
};

struct TeamLock
{
	std::string Name; // The name of the team to lock on to
	short Type; // 0 = Disable team locking
				// 1 = Switch to team once then set to 0
				// 2 = Always keep player locked in team
				// 3 = Set TeamLockName to the first team read -1 then set TeamLockType to 1
};

struct sTeam;
struct sPlayer
{
	std::string name;
	int id; // LOCAL
	bool king; // LOCAL
	sTeam* team; // LOCAL
	TeamLock teamLock; // LOCAL 

	unsigned warnings;

	unsigned int blocksCollapsed;
	unsigned int teamkills;
	int kills;
	unsigned int deaths;
	unsigned short timesKing;
	unsigned playtime; // Total time played
		unsigned playtime_human;
		unsigned playtime_dwarf;
		unsigned playtime_elf;
		unsigned playtime_goblin;
		unsigned playtime_troll;

	time_t played;
	unsigned previousPlaytime; // In seconds

	sPlayer::sPlayer (std::string iName);
	void Save (HANDLE handle);
	void Load (HANDLE handle);
};

void addPlayerToTeam (sPlayer* player, sTeam* team); // NULL = spectate

int getClosestPlayerMatch (std::string s);

sPlayer* playerJoin (std::string name);

void playerLeave (std::string name);

sPlayer* GetPlayerByName(std::string name, bool createNew=true, bool useLevenshtein=false);

void Slay (std::string killer, std::string victim, int slay_type);


void SwitchAllPlayers ();

sPlayer* GetPlayerByID (int id);