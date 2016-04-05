#include "zombies.h"

#include "interface.h"
#include "sender.h"
#include "program.h"
#include "player.h"
#include "team.h"


bool Zombies;

int ZOMBIE_EVENT_MINIMUM_PLAYERS;
int ZOMBIE_EVENT_MINIMUM_DELAY;
float ZOMBIE_EVENT_PERCENT_INFECTION;
bool AutostartEnabled;

void ZombieSetup (std::string configPath)
{
	Zombies=false;
	ZOMBIE_EVENT_MINIMUM_PLAYERS	= GetPrivateProfileInt("ZOMBIES", "MinimumPlayers", 2, configPath.c_str());
	ZOMBIE_EVENT_MINIMUM_DELAY		= GetPrivateProfileInt("ZOMBIES", "MinimumDelay", 1000, configPath.c_str());
	ZOMBIE_EVENT_PERCENT_INFECTION	= ( GetPrivateProfileInt("ZOMBIES", "MinimumInfection", 50, configPath.c_str()) ) / 100.0f;
	AutostartEnabled = ((GetPrivateProfileInt("ZOMBIES", "AutostartEnabled", 0, configPath.c_str())) ? true : false);
	RemoveAllSeclev ("zombies.cfg");
}

bool ZombieApocalypseOn (void)
{
	return Zombies;
}

void ZombieApocalypseStart ()
{
	if (!ZombieApocalypseOn())
	{
		// Broadcast message
		sendMsg ("**********************************");
		sendMsg ("The zombie apocalypse has started!");
		sendMsg ("**********************************");
		form->AddConsoleMessage ("The zombie apocalypse has started!");
		for (unsigned i=1; i<program->teams.size(); i++)
		{
			playSound (program->teams[i]->x, program->teams[i]->y, SOUND_MONSTER);
		}
		// Turn zombie boolean on
		Zombies = true;

		// Switch random players as first blood
		std::string playerList="";
		int minInfected = program->players.size() * ZOMBIE_EVENT_PERCENT_INFECTION;
		if (minInfected==0)
			minInfected=1;
		int numInfected = 0;
		int tries = 0;
		while (numInfected < minInfected)
		{
			if (tries>1000)
			{
				form->AddConsoleMessage ("Error: Tried 1000 times switching players to zombies and we still haven't switched enough players. Breaking.");
				break;
			}
			for (unsigned i=0; i<program->players.size(); i++)
			{
				sPlayer* plr = program->players[i];
				if (plr)
				{
					if (!plr->king)
					{
						//if ((int)(rand() % program->players.size()) == 0)
						{
							// Player is picked out, switch to zombies.
							//plr->teamLock.Name = "Zombies";
							//plr->teamLock.Type = 2;
							playerList.append(plr->name);
							playerList.append("; ");
							form->AddConsoleMessage ("Added " + plr->name + " to zombie team");
							numInfected++;
							if (numInfected >= minInfected)
								break;
						}
					}
				}
				else
					form->AddConsoleMessage ("Error: Player pointer not found when trying to switch first blood players to zombie team");
			}
			tries++;
		}
		playerList.erase(playerList.length()-2, 2);
		AssignSeclev (playerList, "zombies.cfg");
	}
}

void ZombieEventThread (void)
{
	Sleep(5000);
	Zombies=false;
	while(true)
	{
		Sleep(ZOMBIE_EVENT_MINIMUM_DELAY);
		if (AutostartEnabled)
		{
			if (program->players.size() >= ZOMBIE_EVENT_MINIMUM_PLAYERS)
				ZombieApocalypseStart ();
		}
		if (Zombies)
		{
			bool stillPlayersAlive=false;
			for(unsigned i=1; i<=5; i++)
			{
				if (program->teams[i]->players.size()>0)
				{
					stillPlayersAlive=true;
					break;
				}
			}
			if (!stillPlayersAlive)
			{
				sendMsg("****************************************");
				sendMsg("The zombies have taken over the world!");
				//sendMsg("Last survivor: ");
				//sendMsg("The apocalypse lasted for - minutes.");
				sendMsg("****************************************");
				sendRcon("/nextmap");
				RemoveAllSeclev("zombies.cfg");
				Zombies=false;
			}
		}
	}
}

void ZombieLockPlayer (sPlayer* plr)
{
	if (plr->king)
		RemoveKing(plr->team);
	AssignSeclev(plr->name, "zombies.cfg");
	plr->teamLock.Name="";
	plr->teamLock.Type=0;
}