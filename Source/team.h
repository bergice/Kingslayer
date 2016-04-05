#pragma once

#include <string>
#include <vector>


struct sPlayer;
struct sTeam
{
	bool noRules; // This is the variable used to define whether the team is a regular team or a special team

	int x;
	int y;

	std::string name;
	std::string namePlural;

	unsigned warsWon;
	unsigned warsLost;

	std::vector<sTeam*> enemies;

	sPlayer* king;
	std::vector<sPlayer*> players;

	sTeam::sTeam (std::string iName, std::string iNamePlural, int xpos, int ypos, bool _noRules);
};

void CreateDefaultTeams (std::string configFile);
void EndWar (sTeam* loser, sTeam* winner, sPlayer* pKiller, sPlayer* pVictim);
void NewKing (sTeam* team, sPlayer* king);
void RemoveKing (sTeam* team);