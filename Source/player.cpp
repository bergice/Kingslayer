#include "player.h"

#include <iostream>
#include <boost\\algorithm\\string\\predicate.hpp>
#include "boost\\lexical_cast.hpp"
#include "file.h"

#include "misc.h"

#include "team.h"
#include "vote.h"
#include "freeze.h"

#include "sender.h"

#include "program.h"

#include "interface.h"
#include "zombies.h"

void addPlayerToTeam (sPlayer* player, sTeam* team) // NULL = spectate
{
	if (player)
	{
		// If already in a team, remove player from that team.
		if (player->team>0)
		{
			for(unsigned i=0; i<player->team->players.size(); i++)
			{	
				if (strcmp (player->team->players[i]->name.c_str(), player->name.c_str()) == 0)
				{
					// Erase all nominations the player has started and erase all votes with only the player as nominator.
					for (unsigned k=0; k<program->voteking.size(); k++)
					{
						for (unsigned v=0; v<program->voteking[k]->voters.size(); v++)
						{
							if (boost::iequals(player->name, program->voteking[k]->voters[v]->name))
								program->voteking[k]->voters.erase (program->voteking[k]->voters.begin()+v);
						}
						if (program->voteking[k]->voters.size()<=0)
							program->voteking.erase (program->voteking.begin()+k);
					}
					// End wars and set king to none if king
					if (player->king)
					{
						//RemoveKing(player->team);
					}
					player->team->players.erase(player->team->players.begin()+i);

					break;
				}
			}
		}
		if (team)
			/*CONSOLE NOTIFICATION HERE*/ form->AddConsoleMessage("Putting player " + player->name + " on team " + team->name);
		else
			/*CONSOLE NOTIFICATION HERE*/ form->AddConsoleMessage("Putting player " + player->name + " on team *NULL*");
		player->team = team;
		if (team)
			team->players.push_back(player);
	}
}

sPlayer::sPlayer (std::string iName)
{
	name = iName;

	id = NULL;
	king = false;
	team = NULL;
	teamLock.Name = "";
	teamLock.Type = NULL;

	timesKing = 0;
	blocksCollapsed = 0;
	teamkills = 0;
	kills = 0;
	deaths = 0;
	playtime = 0;
	playtime_human = 0;
	playtime_dwarf = 0;
	playtime_elf = 0;
	playtime_goblin = 0;
	playtime_troll = 0;

	warnings = 0;

	/*CONSOLE NOTIFICATION HERE*/ form->AddConsoleMessage("Constructed new player with name " +  name);
}
void sPlayer::Save (HANDLE handle)
{
	WriteString	(handle, name.c_str());
	//WriteByte	(handle, id);
	//WriteByte	(handle, (int)king);
	//if (team)
	//	WriteString (handle, team->name.c_str());
	//else
	//	WriteString (handle, program->teams[0]->name.c_str());
	WriteByte   (handle, warnings);
	WriteByte   (handle, blocksCollapsed);
	WriteByte   (handle, teamkills);
	WriteByte   (handle, kills);
	WriteByte   (handle, deaths);
	WriteByte   (handle, timesKing);

	/*CONSOLE NOTIFICATION HERE*/ form->AddConsoleMessage("Saved player " +  name);
}
void sPlayer::Load (HANDLE handle)
{
	name = ReadString (handle);
	id = ReadByte (handle);
	//king = ReadByte (handle) ? true: false;
	//std::string teamname = ReadString (handle);
	//for (unsigned t=0; t<program->teams.size(); t++)
	//{
	//	if (boost::iequals(teamname, program->teams[t]->name))
	//		addPlayerToTeam (this, program->teams[t]);
	//}
	warnings = ReadByte (handle);
	blocksCollapsed = ReadByte (handle);
	teamkills = ReadByte (handle);
	kills = ReadByte (handle);
	deaths = ReadByte (handle);
	timesKing = ReadByte (handle);

	/*CONSOLE NOTIFICATION HERE*/ form->AddConsoleMessage("Loaded player " +  name);
}

int getClosestPlayerMatch (std::string s)
{
	int index=-1;;
	int closest = 9999;
	for (unsigned p=0; p<program->players.size(); p++)
	{
		int lev = LevenshteinDistance(s, program->players[p]->name);

		if (lev<closest && lev < (int)s.length()/1.25f)
		{	
			closest=lev;
			index=p;
		}
	}
	return index;
}

sPlayer* playerJoin (std::string name)
{
	sPlayer* plr=NULL;

	// Check if player is on offline list
	int index = -1;
	for(unsigned i=0; i<program->players_offline.size(); i++)
	{
		if (strcmp(program->players_offline[i]->name.c_str(), name.c_str())==0)
		{
			index = i;
			break;
		}
	}
	bool loadedPlayer=false;
	if (index>=0)
	{
		plr = program->players_offline[index];

		if (plr)
		{
			addPlayerToTeam(program->players_offline[index], NULL);

			program->players_offline.erase(program->players_offline.begin()+index);

			loadedPlayer=true;

			form->UpdatePlayerList();
		}
	}
	
	// Player isn't in offline list, create new player
	if (!loadedPlayer)
	{
		plr = new sPlayer(name);

		if (!ZombieApocalypseOn())
		{
			form->AddConsoleMessage (name + " is new, adding to tutorial seclev and locking.");
			AssignSeclev(name, "tutorial.cfg");
			//plr->teamLock.Name="Tutorial";
			//plr->teamLock.Type=2;
		}
		/*CONSOLE NOTIFICATION HERE*/ form->AddConsoleMessage(name + " joined the server for the first time");
	}
	else
	{
		if (ZombieApocalypseOn())
		{
			form->AddConsoleMessage (name + " is being locked to zombie team on join.");
			ZombieLockPlayer (plr);
		}
		/*CONSOLE NOTIFICATION HERE*/ form->AddConsoleMessage("Recurring player " + name + " joined the game");
	}

	addPlayerToTeam (plr, program->teams[0]);
	program->players.push_back(plr);

	//form->UpdatePlayerList();

	return plr;
}

void playerLeave (std::string name)
{
	int index = -1;
	for(unsigned i=0; i<program->players.size(); i++)
	{
		if (strcmp(program->players[i]->name.c_str(), name.c_str())==0)
		{
			index = i;
			break;
		}
	}
	
	// Inform that the player is leaving and add to the offline list
	if (index>=0)
	{
		sPlayer* plr = program->players[index];

		if (plr)
		{
			addPlayerToTeam(program->players[index], NULL);

			program->players.erase(program->players.begin()+index);
			program->players_offline.push_back(plr);
		}
	}
	/*CONSOLE NOTIFICATION HERE*/ form->AddConsoleMessage("Player " + name + " left the game, added to offline list");
	form->UpdatePlayerList();
}

sPlayer* GetPlayerByName(std::string name, bool createNew, bool useLevenshtein)
{
	if (useLevenshtein)
	{
		int cm = getClosestPlayerMatch(name);
		if (cm!=-1)
		{
			return program->players[cm];
		}
	}
	else
	{
		for (unsigned i=0; i<program->players.size(); i++)
		{
			if (boost::iequals(name, program->players[i]->name))
			{
				return program->players[i];
			}
		}
	}
	// No player registered with that name? Create it:
	if (createNew)
		return playerJoin(name);
	else
		return NULL;
}

void Slay (std::string killer, std::string victim, int slay_type)
{
	std::string killername = ClanNameToName(killer);
	std::string victimname = ClanNameToName(victim);

	sPlayer* pKiller=GetPlayerByName(killername, false);
	sPlayer* pVictim=GetPlayerByName(victimname, false);

	if (!pKiller||!pVictim)
		return;

	if ( strcmp("Zombie", pKiller->team->name.c_str()) == 0 || strcmp("Zombie", pVictim->team->name.c_str()) == 0)
	{
		if (strcmp("Zombie", pKiller->team->name.c_str()) == 0)
		{
			ZombieLockPlayer (pVictim);
			form->AddConsoleMessage ("Added " + pVictim->name + " to zombie team");
		}
		if (strcmp("Zombie", pVictim->team->name.c_str()) == 0)
		{
			// Player slayed a zombie!
		}
	}
	else
		// Use regular Slay rules
	{
		bool teamkill = true;
		if (pKiller->team == pVictim->team)
			teamkill=false;
		for (unsigned i=0; i<pKiller->team->enemies.size(); i++)
		{
			if (pKiller->team->enemies[i] == pVictim->team)
			{	
				teamkill=false;
				break;
			}
		}
		bool kingslayer = pVictim->king;
		bool atWar = false;
		for (unsigned i=0; i<pKiller->team->enemies.size(); i++)
		{
			if (pKiller->team->enemies[i] == pVictim->team)
			{
				atWar=true;
				break;
			}
		}

		form->AddConsoleMessage("Killer: " + pKiller->name + " | Victim: " + pVictim->name + " | Killed king=" + boost::lexical_cast<std::string>(kingslayer) + " | Was teamkill=" + boost::lexical_cast<std::string>(teamkill) + " | At war=" + boost::lexical_cast<std::string>(atWar));

		// Player killed the enemy King
		if (!teamkill && kingslayer && atWar)
		{
			// End the war and log victory/loss to the teams
			EndWar(pVictim->team, pKiller->team, pKiller, pVictim);
		}

		if (teamkill)
		{
			pKiller->teamkills++;
			pKiller->kills-=1;

			pKiller->warnings++;

			if (pKiller->warnings<3)
			{
				Freeze(pKiller, 5+(5*pKiller->warnings), "RK exceeded warnings issued");
				/*CONSOLE NOTIFICATION HERE*/ form->AddConsoleMessage("FREEZE punish for randomkilling player " + pKiller->name);
			}
			else
			{
				if (pKiller->warnings<5)
				{
					Kick(pKiller, "RK exceeded warnings issued");
					/*CONSOLE NOTIFICATION HERE*/ form->AddConsoleMessage("KICK punish for randomkilling player " + pKiller->name);
				}
				else
				{
					std::string rkReason = "RK exceeded warnings issued ("+boost::lexical_cast<std::string>(pKiller->warnings)+")";
					Ban(pKiller, 5, (char*)rkReason.c_str());
					/*CONSOLE NOTIFICATION HERE*/ form->AddConsoleMessage("5 MINUTE BAN punish for randomkilling player " + pKiller->name);
				}
			}
		}
		
		if (!teamkill && atWar)
		{
			pKiller->kills++;
			pVictim->deaths++;
		}
	}
}

void SwitchAllPlayers ()
{
	for (unsigned i=0; i<program->players.size(); i++)
	{
		sPlayer* player = program->players[i];

		if (player)
		{
			if (player->id)
			{
				player->teamLock.Type = 3;
				player->teamLock.Name = "";
				swapid(player->id);
			}
		}
	}
	sendMsg("Switched all players");
}


sPlayer* GetPlayerByID (int id)
{
	for (unsigned i=0; i<program->players.size(); i++)
	{
		if (program->players[i]->id==id)
			return program->players[i];
	}
	return NULL;
}