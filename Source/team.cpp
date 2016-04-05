#include "team.h"

#include <boost\\algorithm\\string\\predicate.hpp>
#include <boost\\lexical_cast.hpp>

#include "player.h"
#include "interface.h"
#include "program.h"
#include "vote.h"

#include <iostream>

sTeam::sTeam (std::string iName, std::string iNamePlural, int xpos, int ypos, bool _noRules)
{
	noRules = _noRules;

	x=xpos;
	y=ypos;

	name = iName;
	namePlural = iNamePlural;
	if (boost::iequals(iNamePlural, ""))
		namePlural = name;
	warsWon = 0;
	warsLost = 0;
	king = NULL;
}

void AddTeam (std::string name, std::string namePlural, int xpos, int ypos, bool norules)
{
	sTeam* newTeam = new sTeam(name, namePlural, xpos, ypos, norules);
	program->teams.push_back(newTeam);
	form->AddConsoleMessage("Added new team " + newTeam->name + " (" + newTeam->namePlural + ")		(" + boost::lexical_cast<std::string>(newTeam->x) + ", " + boost::lexical_cast<std::string>(newTeam->y) + ")");
}

void CreateDefaultTeams (std::string configFile)
{
	char buffer[MAX_PATH];

	int r=0;
	while(r>=0)
	{
		std::string app = "TEAM"; app.append(boost::lexical_cast<std::string>(r));
		if (GetPrivateProfileString(app.c_str(), "Name", "", buffer, sizeof(buffer), configFile.c_str()))
		{
			GetPrivateProfileString(app.c_str(), "Name", "", buffer, sizeof(buffer), configFile.c_str());
			std::string Name = buffer;
			GetPrivateProfileString(app.c_str(), "NamePlural", Name.c_str(), buffer, sizeof(buffer), configFile.c_str());
			std::string NamePlural = buffer;
			int X = GetPrivateProfileInt(app.c_str(), "X", 0, configFile.c_str());
			int Y = GetPrivateProfileInt(app.c_str(), "Y", 0, configFile.c_str());
			bool norules = ((GetPrivateProfileInt(app.c_str(), "noRules", 0, configFile.c_str())) ? true : false);
			AddTeam	(Name, NamePlural, X, Y, norules);
			r++;
		}
		else
			r=-1;
	}
}

void EndWar (sTeam* loser, sTeam* winner, sPlayer* pKiller, sPlayer* pVictim)
{
	// Broadcast message
	std::string endMsg = std::string(winner->name + " " + pKiller->name + " has killed " + loser->name + " King " + pVictim->name + ", ending the war!");
	sendMsg("********************************");
	sendMsg(endMsg);
	sendMsg("********************************");

	playSound(winner->x, winner->y, SOUND_FANFARE);
	playSound(loser->x, loser->y, SOUND_LOSTWAR);

	/*CONSOLE NOTIFICATION HERE*/ form->AddConsoleMessage(endMsg);

	// Remove the king and end all wars
	RemoveKing(loser);

	// Log the loss/win
	loser->warsLost++;
	winner->warsWon++;
}

void NewKing (sTeam* team, sPlayer* king)
{
	// Broadcast message
	sendMsg(std::string("*** " + king->name + " has become the " + team->name + " King! ***"));
	playSound(king->team->x, king->team->y, SOUND_NEWKING);

	// Remove previous kings on that team
	for(unsigned i=0; i<king->team->players.size(); i++)
		king->team->players[i]->king=false;
	
	// Assign new king
	king->king=true;
	team->king=king;
	king->timesKing++;

	// Delete votes for that team
	for (unsigned i=0; i<program->voteking.size(); i++)
	{
		if (program->voteking[i]->target->team == king->team)
		{
			program->voteking.erase(program->voteking.begin()+i);
		}
	}

	// Lock To Team
	king->teamLock.Name = king->team->name;
	king->teamLock.Type = 2;

	// Set Seclev to king
	AssignSeclev(king->name, "king.cfg");
}

void RemoveKing (sTeam* team)
{
	// Log
	form->AddConsoleMessage("Removing king " + team->king->name + " from " + team->namePlural);

	// Unlock From Team
	team->king->teamLock.Name = "";
	team->king->teamLock.Type = 0;

	// Remove from seclev
	RemoveSeclev(team->king->name, "king.cfg");

	// Everyone ends their war against the victim
	for (unsigned i=0; i<program->teams.size(); i++)
	{
		for (unsigned e=0; e<program->teams[i]->enemies.size(); e++)
		{
			if (program->teams[i]->enemies[e] == team)
			{
				program->teams[i]->enemies.erase (program->teams[i]->enemies.begin()+e);
				break;
			}
		}
	}

	// Victim ends their war against everyone
	team->enemies.clear();

	// Remove the king on the losing team
	team->king->king=false;
	team->king=NULL;
}