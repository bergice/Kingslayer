#include "receiver.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <boost\\algorithm\\string\\predicate.hpp>
#include <boost\\algorithm\\string\\case_conv.hpp>
#include "boost\\lexical_cast.hpp"
#include "boost\\regex.hpp"
#include "misc.h"

#include "sender.h"

#include "player.h"
#include "team.h"
#include "vote.h"
#include "freeze.h"

#include "program.h"
#include "interface.h"
#include "zombies.h"

bool readingPlayers=false;

void ProcessMessage (std::string message, bool sendOutput)
{
	unsigned int pos;
	unsigned int pos2;
	boost::smatch match;

	if (readingPlayers)
	{
		boost::regex pattern("\\[..:..:..\\]........\\[(.+)\\].\\(id.(\\d+)\\)"); // Fetch ip too?
		boost::smatch match;
		if (boost::regex_search(message, match, pattern))
		{
			std::string name = match[1];
			std::string result = ClanNameToName(name);
			std::string id = match[2];
			sPlayer* plr = GetPlayerByName(result, true);
			if (plr)
			{
				plr->id=atoi(id.c_str());

				if (program->switchPlayers)
					swapid(plr->id);
			}

			return;
		}

		boost::regex pattern2("\\[..:..:..\\].");
		if (boost::regex_match(message, match, pattern2))
		{
			if (program->switchPlayers)
				program->switchPlayers = false;

			readingPlayers=false;
		}
	}


	// Admin freeze command
	{
		boost::regex pattern("\\[..:..:..\\].\\[RCON.(.+)\\].\\\\freezeid.(.+)");
		if (boost::regex_search(message, match, pattern))
		{
			std::string admin = ClanNameToName(match[1]);
			int id = boost::lexical_cast<int>(match[2]);
			if (program->GetAdmin(admin))
			{
				sPlayer* plr = GetPlayerByID(id);
				if (plr)
				{
					Freeze(plr, 60);
				}
			}
			return;
		}
	}

	// Admin unfreeze command
	{
		boost::regex pattern("\\[..:..:..\\].\\[RCON.(.+)\\].\\\\unfreezeid.(.+)");
		if (boost::regex_search(message, match, pattern))
		{
			std::string admin = ClanNameToName(match[1]);
			int id = boost::lexical_cast<int>(match[2]);
			if (program->GetAdmin(admin))
			{
				sPlayer* plr = GetPlayerByID(id);
				if (plr)
				{
					Unfreeze(plr);
				}
			}
			return;
		}
	}


	// Command by player
	pos = message.find("> /");
	pos2 = message.find("> !");
	if (pos == std::string::npos && pos2 != std::string::npos)
		pos=pos2;
	if (pos != std::string::npos)
	{
		std::string cMsg = message.substr(pos+3, message.length()-pos-3);
		std::string cName = message.substr(message.find("<")+1, pos-message.find("<")-1);
		std::string name = ClanNameToName(cName);

		PlayerMessage(name, cMsg);

		return;
	}
	{
		if (!readingPlayers)
		{
			boost::regex pattern("\\[..:..:..\\]./players");
			if (boost::regex_search(message, match, pattern))
			{
				readingPlayers=true;
				return;
			}
		}
	}
	{
		boost::regex pattern("\\[..:..:..\\].(.+).hammered.(.+).to.death");
		if (boost::regex_search(message, match, pattern))
		{
			std::string killer = ClanNameToName(match[1]);
			std::string victim = ClanNameToName(match[2]);
			sPlayer* plr = GetPlayerByName(killer, false);
			if (plr)
			{
				Slay(killer, victim, SLAY_HAMMER);
			}
			return;
		}
	}
	{
		boost::regex pattern("\\[..:..:..\\].(.+).assisted.in.(.+).dying.under.falling.rocks");
		if (boost::regex_search(message, match, pattern))
		{
			std::string killer = ClanNameToName(match[1]);
			std::string victim = ClanNameToName(match[2]);
			sPlayer* plr = GetPlayerByName(killer, false);
			if (plr)
			{
				Slay(killer, victim, SLAY_ROCKS);
			}
			return;
		}
	}
	{
		boost::regex pattern("\\[..:..:..\\].(.+).pushed.(.+).to.h...death");
		if (boost::regex_search(message, match, pattern))
		{
			std::string killer = ClanNameToName(match[1]);
			std::string victim = ClanNameToName(match[2]);
			sPlayer* plr = GetPlayerByName(killer, false);
			if (plr)
			{
				Slay(killer, victim, SLAY_PUSH);
			}
			return;
		}
	}
	{
		boost::regex pattern("\\[..:..:..\\].(.+).gibbed.(.+).into.pieces");
		if (boost::regex_search(message, match, pattern))
		{
			std::string killer = ClanNameToName(match[1]);
			std::string victim = ClanNameToName(match[2]);
			sPlayer* plr = GetPlayerByName(killer, false);
			if (plr)
			{
				Slay(killer, victim, SLAY_BOMB);
			}
			return;
		}
	}
	{
		boost::regex pattern("\\[..:..:..\\].(.+).shot.(.+).with.h...arrow");
		if (boost::regex_search(message, match, pattern))
		{
			std::string killer = ClanNameToName(match[1]);
			std::string victim = ClanNameToName(match[2]);
			sPlayer* plr = GetPlayerByName(killer, false);
			if (plr)
			{
				Slay(killer, victim, SLAY_ARROW);
			}
			return;
		}
	}
	{
		boost::regex pattern("\\[..:..:..\\].(.+).slew.(.+).with.h...sword");
		if (boost::regex_search(message, match, pattern))
		{
			std::string killer = ClanNameToName(match[1]);
			std::string victim = ClanNameToName(match[2]);
			sPlayer* plr = GetPlayerByName(killer, false);
			if (plr)
			{
				Slay(killer, victim, SLAY_SWORD);
			}
			return;
		}
	}
	{
		boost::regex pattern("\\[..:..:..\\]....COLLAPSE.by.(.+).\\(size.(.+).blocks\\)");
		if (boost::regex_search(message, match, pattern))
		{
			std::string cName = match[1];
			std::string name = ClanNameToName(cName);
			std::string cCollapseSize = match[2];
			sPlayer* plr = GetPlayerByName(name, false);
			if (plr)
			{
				if (plr->id)
				{
					int collapsedBlocks = atoi(cCollapseSize.c_str());
					plr->blocksCollapsed+=collapsedBlocks;

					if (collapsedBlocks>16)
					{
						plr->warnings++;
						if (collapsedBlocks>100)
							plr->warnings++;
						if (collapsedBlocks>200)
							plr->warnings++;

						if (plr->warnings<3)
						{
							Freeze(plr, 5+(5*plr->warnings), "RK exceeded warnings issued");
						}
						else
						{
							if (plr->warnings<5)
							{
								Kick(plr, "RK exceeded warnings issued");
							}
							else
							{
								Ban(plr, 5, "RK exceeded warnings issued");
							}
						}
					}
				}
			}
			sendMsg(std::string("Collapse by " + name + " (" + cCollapseSize + " blocks)"));
			return;
		}
	}
	// Player has joined a team
	pos = message.find("has joined");
	pos2 = message.find("has joined the game");
	// Player has joined the game
	if (pos2 != std::string::npos)
	{
		std::string nameWCLANTAG = message.substr(11, pos-12);
		std::string name = ClanNameToName(nameWCLANTAG);
		sPlayer* player = GetPlayerByName(name);
	}
	// Player has joined a team
	if (pos != std::string::npos && pos2 == std::string::npos)
	{
		std::string nameWCLANTAG = message.substr(11, pos-12);
		std::string name = ClanNameToName(nameWCLANTAG);

		sPlayer* player = GetPlayerByName(name, false);
		if (player)
		{
			for (unsigned teamIndex=1; teamIndex<program->teams.size(); teamIndex++) // Skip 0 for spectator team
			{	
				// Player switch to team matches current team index
				boost::regex pattern("\\[..:..:..\\].(.+).has.joined." + program->teams[teamIndex]->name);

				if (boost::regex_search(message, match, pattern))
				{
					// Add the player to said team
					addPlayerToTeam(player, program->teams[teamIndex]);

					if (player->id)
					{
						// Check team lock
						switch(player->teamLock.Type)
						{
							// We want to switch to the team with the current lock name once
							case 1:
							{
								form->AddConsoleMessage("Lock type is 1");

								if (strcmp(player->teamLock.Name.c_str(), program->teams[teamIndex]->name.c_str()))
								{	
									form->AddConsoleMessage("Player " + player->name + " has joined team " + program->teams[teamIndex]->name + " but the target team is " + player->teamLock.Name + "! Switching again");

									swapid(player->id);
								}
								else
								{
									form->AddConsoleMessage("Player " + player->name + " has been put back on his team " + player->teamLock.Name);

									form->AddConsoleMessage("Setting lock type to 0");

									player->teamLock.Type = 0;
									player->teamLock.Name = "";
								}
							}
							break;
							// We want to keep the player locked in the lock name
							case 2:
							{
								form ->AddConsoleMessage("Player " + player->name + " is locked to team " + player->teamLock.Name);
								if ( strcmp( player->teamLock.Name.c_str(), program->teams[teamIndex]->name.c_str() ) != 0 )
								{
									form ->AddConsoleMessage("Player " + player->name + " joined team " + program->teams[teamIndex]->name + " but is locked to " + player->teamLock.Name + ". Swapping!");

									swapid(player->id);
								}
							}
							break;
							// We want to switch to the previous team the player was on
							case 3:
							{
								form->AddConsoleMessage("Lock type is 3");

								// Loop
								int index = teamIndex-1;
								if (index==0)
								{
									index += program->teams.size()-1;
								}

								form->AddConsoleMessage("Setting lock type to 1");
								form->AddConsoleMessage("Setting lock target to " + program->teams[index]->name);

								player->teamLock.Type = 1;
								player->teamLock.Name = program->teams[index]->name;

								form->AddConsoleMessage("Attempting to put player " + player->name + " back to team " + player->teamLock.Name + " with index " + boost::lexical_cast<std::string>(index));
								
								swapid(player->id);
							}
							break;
						}
					}
				}
			}
		}
		return;
	}
	// Player joined spectator
	pos = message.find("is now spectating");
	if (pos != std::string::npos)
	{
		std::string cname = message.substr(11, pos-12);
		std::string name = ClanNameToName(cname);
		addPlayerToTeam(GetPlayerByName(name), program->teams[0]);
		return;
	}
	// Player left the game
	{
		boost::regex pattern_disconnected("\\[..:..:..\\].Player.(.+).left.the.game");
		if (boost::regex_search(message, match, pattern_disconnected))
		{
			std::string cname = match[1];
			std::string name = ClanNameToName(cname);
			playerLeave(name);
			return;
		}
	}
	// Player joined the game
	{
		boost::regex pattern_connected("\\[..:..:..\\].\\*.(.+).connected");
		if (boost::regex_search(message, match, pattern_connected))
		{
			std::string cname = match[1];
			std::string name = ClanNameToName(cname);
			addPlayerToTeam(GetPlayerByName(name), program->teams[0]);
			DoWelcomeMessage(name);
			// Auto-kick function goes here <<<
			return;
		}
	}
}

void PlayerMessage (std::string playername, std::string message)
{
	if (boost::iequals(message, std::string("help")) || boost::iequals(message, std::string("commands")) || boost::iequals(message, std::string("list")))
	{
		sendMsg("Commands: /king <NAME>, /war <TEAM>, /wars, /me, /achievements, /stats, /top10, /tutorial, /whoisking, /listkings");
		return;
	}
	if (boost::iequals(message, std::string("stats")))
	{
		sPlayer* plr = GetPlayerByName(playername, false);
		if (plr)
		{
			///achievements, /stats, /top10
		}
		else
			form->AddConsoleMessage("Error finding player in /stats command!");
	}
	if (boost::iequals(message, std::string("me")))
	{
		std::string s="";
		sPlayer* plr = GetPlayerByName(playername, false);
		if (plr)
		{
			s.append(plr->name);
			s.append(" Kills: ");
			s.append(boost::lexical_cast<std::string>(plr->kills));
			s.append(" | Deaths: ");
			s.append(boost::lexical_cast<std::string>(plr->deaths));
			s.append(" | Teamkills: ");
			s.append(boost::lexical_cast<std::string>(plr->teamkills));
		}
		else
			form->AddConsoleMessage("Error finding player in /me command!");
		sendMsg(s);
		return;
	}
	if (boost::iequals(message, std::string("accept")))
	{
		sPlayer* plr = GetPlayerByName(playername, false);
		if (plr)
		{
			if ( strcmp(plr->team->name.c_str(), "Tutorial") == 0 )
			{
				//plr->teamLock.Name="";
				//plr->teamLock.Type=0;

				RemoveSeclev(plr->name, "tutorial.cfg");
				swapid(plr->id);
			}
		}
		else
			form->AddConsoleMessage("Error finding player in /accept command!");
	}
	if (boost::iequals(message, std::string("tutorial")))
	{
		sPlayer* plr = GetPlayerByName(playername, false);
		if (plr)
		{
			if (!plr->king)
			{
				if ( strcmp(plr->team->name.c_str(), "Tutorial") != 0 )
				{
					//plr->teamLock.Name="Tutorial";
					//plr->teamLock.Type=2;

					AssignSeclev(plr->name, "tutorial.cfg");
				}
			}
		}
		else
			form->AddConsoleMessage("Error finding player in /tutorial command!");
	}
	if (boost::iequals(message, std::string("listall")) || boost::iequals(message, std::string("listplayers")))
	{
		std::string s = "Registered players: ";
		for(unsigned i=0; i<program->players.size(); i++)
		{
			s.append(program->players[i]->name);
			if (i<program->players.size()-1)
								s.append(", ");
		}
		sendMsg(s);
		return;
	}
	if (boost::iequals(message, std::string("listspectators")))
	{
		std::string s = "Registered players: ";
		for(unsigned i=0; i<program->teams[0]->players.size(); i++)
		{
			s.append(program->teams[0]->players[i]->name);
			if (i<program->teams[0]->players.size()-1)
								s.append(", ");
		}
		sendMsg(s);
		return;
	}
	if (boost::iequals(message, std::string("listhumans")))
	{
		std::string s = "Registered players: ";
		for(unsigned i=0; i<program->teams[1]->players.size(); i++)
		{
			s.append(program->teams[1]->players[i]->name);
			if (i<program->teams[1]->players.size()-1)
								s.append(", ");
		}
		sendMsg(s);
		return;
	}
	if (boost::iequals(message, std::string("listdwarves")))
	{
		std::string s = "Registered players: ";
		for(unsigned i=0; i<program->teams[2]->players.size(); i++)
		{
			s.append(program->teams[2]->players[i]->name);
			if (i<program->teams[2]->players.size()-1)
								s.append(", ");
		}
		sendMsg(s);
		return;
	}
	if (boost::iequals(message, std::string("listelves")))
	{
		std::string s = "Registered players: ";
		for(unsigned i=0; i<program->teams[3]->players.size(); i++)
		{
			s.append(program->teams[3]->players[i]->name);
			if (i<program->teams[3]->players.size()-1)
								s.append(", ");
		}
		sendMsg(s);
		return;
	}
	if (boost::iequals(message, std::string("listgoblins")))
	{
		std::string s = "Registered players: ";
		for(unsigned i=0; i<program->teams[4]->players.size(); i++)
		{
			s.append(program->teams[4]->players[i]->name);
			if (i<program->teams[4]->players.size()-1)
								s.append(", ");
		}
		sendMsg(s);
		return;
	}
	if (boost::iequals(message, std::string("listtrolls")))
	{
		std::string s = "Registered players: ";
		for(unsigned i=0; i<program->teams[5]->players.size(); i++)
		{
			s.append(program->teams[5]->players[i]->name);
			if (i<program->teams[5]->players.size()-1)
								s.append(", ");
		}
		sendMsg(s);
		return;
	}
	// **************
	// ADMIN COMMANDS
	// **************
	if (program->GetAdmin(playername))
	{
		if (boost::iequals(message, std::string("zombies")))
		{
			ZombieApocalypseStart ();
			return;
		}
		if (boost::iequals(message, std::string("save")))
		{
			program->SaveBackup();
			program->SavePlayers();
			sendMsg("Saved restore file.");
			return;
		}
		if (boost::iequals(message, std::string("playsound")))
		{
			//playSound(300, 64, SOUND_FANFARE);
			return;
		}
		if (boost::iequals(message, std::string("load")))
		{
			program->Load("restore.dat");
			program->Load("players.dat");
			sendMsg("Loaded restore file.");
			return;
		}
		if (boost::iequals(message, std::string("getpid")))
		{
			sendRcon("/players");
			sendMsg("Read player ID's");
			return;
		}
		if (boost::iequals(message, std::string("switch all")))
		{
			SwitchAllPlayers ();
			return;
		}
		{
			boost::regex pattern("^switch\\s+(.+)$");
			boost::smatch match;
			if (boost::regex_search(message, match, pattern))
			{
				sPlayer* player = GetPlayerByName(ClanNameToName(match[1]), false, true);
			
				if (player)
				{
					if (player->id)
					{
						player->teamLock.Type = 3;
						player->teamLock.Name = "";
						swapid(player->id);
					}
				}
				return;
			}
		}
		{
			// Kick player
			boost::regex pattern("kick\\s+(\\w+)\\s*(.*)");
			boost::smatch match;
			if (boost::regex_search(message, match, pattern))
			{
				sPlayer* player = GetPlayerByName(ClanNameToName(match[1]), false, true);
				if (player)
				{
					std::string reason = match[2];

					Kick(player, (char*)reason.c_str());
				}
				return;
			}
		}
		{
			// Ban player
			boost::regex pattern("ban\\s+(\\w+)\\s+(\\d+)\\s*(.*)");
			boost::smatch match;
			if (boost::regex_search(message, match, pattern))
			{
				sPlayer* player = GetPlayerByName(ClanNameToName(match[1]), false, true);
				if (player)
				{
					int minutes = boost::lexical_cast<int>(match[2]);
					std::string reason = match[3];

					Ban(player, minutes, (char*)reason.c_str());
				}
				return;
			}
		}
		{
			// Mute player
			boost::regex pattern("mute\\s+(\\w+)\\s+(\\d+)");
			boost::smatch match;
			if (boost::regex_search(message, match, pattern))
			{
				sPlayer* player = GetPlayerByName(ClanNameToName(match[1]), false, true);
				if (player)
				{
					int minutes = boost::lexical_cast<int>(match[2]);
					Mute(player, minutes);
				}
				return;
			}
		}
		{
			// Unmute player
			boost::regex pattern("unmute\\s+(\\w+)");
			boost::smatch match;
			if (boost::regex_search(message, match, pattern))
			{
				sPlayer* player = GetPlayerByName(ClanNameToName(match[1]), false, true);
				if (player)
				{
					Unmute(player);
				}
				return;
			}
		}
	}
	if (boost::iequals(message, std::string("whoisking")))
	{
		sPlayer* plr = GetPlayerByName(playername, false);
		if (plr)
		{
			if (plr->team)
			{
				if (plr->team != program->teams[0])
				{
					sPlayer* king = plr->team->king;
					if (king)
					{
						sendMsg(std::string("Current " + plr->team->name + " King: " + king->name));
					}
					else
					{
						sendMsg(std::string("There is currently no King for " + plr->team->namePlural));
					}
				}
			}
		}
		return;
	}
	if (boost::iequals(message, std::string("listkings")))
	{
		std::string msg;
		for (unsigned i=1; i<program->teams.size(); i++)
		{
			msg.append(program->teams[i]->name);
			msg.append(" King: ");
			if (program->teams[i]->king)
				msg.append(program->teams[i]->king->name);
			else
				msg.append("-NONE-");
			if (i!=program->teams.size()-1)
				msg.append(" | ");
		}
		sendMsg(msg);
		return;
	}
	boost::regex pattern("^king\\s+(.+)$");
	boost::smatch match;
	if (boost::regex_search(message, match, pattern))
	{
		sPlayer* plr = GetPlayerByName(playername, false);
		if (plr) // If player exists
		{
			if (plr->team) // If plr is in a team
			{
				if (!plr->team->noRules) // If not in a special team
				{
					sPlayer* kingVote;
					std::string match1=match[1];
					if (strcmp("me", match1.c_str())==0)
						kingVote = plr;
					else
						kingVote = GetPlayerByName(ClanNameToName(match[1]), false, true);
					if (kingVote) // If nominee exists
					{
						if (plr->team == kingVote->team) // Voting for same team
						{
							if (true)//!boost::iequals(plr->name, kingVote->name)) PLAYER !!!IS!!! ALLOWED TO VOTE FOR HIMSELF
							{
								sVote* vote;
								bool voteExists=false;
								for (unsigned i=0; i<program->voteking.size(); i++)
								{
									if (program->voteking[i]->target)
									{
										if (boost::iequals(program->voteking[i]->target->name, kingVote->name))
										{
											voteExists=true;
											vote=program->voteking[i];
										}
									}
								}
								if (!voteExists)
								{
									vote = new sVote();
									vote->kind=0;
									vote->team=kingVote->team;
									program->voteking.push_back(vote);
									vote->target=kingVote;
								}
								// Get already voted for the same person and ignore if true
								bool alreadyVoted=false;
								for (unsigned i=0; i<vote->voters.size(); i++)
								{
									if (boost::iequals(vote->voters[i]->name, plr->name))
										alreadyVoted=true;
								}
								if (!alreadyVoted)
								{
									vote->voters.push_back(plr);
									vote->verifyVoters();
									int currentVotes = vote->voters.size();
									int votesNeeded = (int)ceil(vote->team->players.size()/2.6f);
									//if (votesNeeded<2)                                     // Currently only 1 person needs to vote to become king
									//	votesNeeded=2;
									char b1[8]; sprintf(b1, "%i", currentVotes);
									char b2[8]; sprintf(b2, "%i", votesNeeded);
									if (currentVotes>=votesNeeded)
									{
										// VOTE COMPLETED
										NewKing(vote->team, vote->target);
									}
									else
									{
										// VOTE IN PROGRESS
										// CHECK ALREADY VOTED, CHANGE THE NOMINEE AND REMOVE OTHER VOTES
										sendMsg(std::string(plr->name + " has voted " + kingVote->name + " for king (" + b1 + "/" + b2 + " votes needed)"));
									}
								}
								else
								{
									sendMsg(std::string(plr->name + " has already voted " + kingVote->name + " for king"));
								}
							}
						}
					}
				}
			}
		}
		return;
	}
	boost::regex pattern3("war\\s+(\\w+)\\s*");
	boost::smatch match5;
	if (boost::regex_search(message, match5, pattern3))
	{
		sPlayer* plr = GetPlayerByName(playername, false);
		if (plr)
		{
			if (plr->king==true)
			{
				std::string s = match5[1];
				std::string lcTeam = boost::algorithm::to_lower_copy(s);
				boost::regex pattern4("(human|humans|blue)|(dwarf|dwarves|dwarfs|red)|(elf|elves|elfs|green)|(goblin|goblins|purple)|(troll|trolls|orange|yellow)|(vip|demigod)");
				boost::smatch match2;
				if (boost::regex_search(lcTeam, match2, pattern4))
				{
					int team=0;
					if (std::string("")!=match2[1])
						team=1;
					if (std::string("")!=match2[2])
						team=2;
					if (std::string("")!=match2[3])
						team=3;
					if (std::string("")!=match2[4])
						team=4;
					if (std::string("")!=match2[5])
						team=5;
					if (std::string("")!=match2[6])
						team=6;
					if (!boost::iequals(plr->team->name, program->teams[team]->name))
					{
						// Check if already at war
						bool alreadyAtWar=false;
						for (unsigned i=0; i<program->teams[team]->enemies.size(); i++)
						{
							if (program->teams[team]->enemies[i]==plr->team)
								alreadyAtWar=true;
						}
						if (!alreadyAtWar)
						{
							// Team being waged war on needs a king
							if (program->teams[team]->king)
							{
								program->teams[team]->enemies.push_back(plr->team);
								plr->team->enemies.push_back(program->teams[team]);
								sendMsg("********************************");
								sendMsg(std::string(plr->team->namePlural + " have declared war on " + program->teams[team]->namePlural));
								sendMsg("********************************");

								playSound(program->teams[team]->x, program->teams[team]->y, SOUND_WAGEWAR);
							}
						}
					}
				}
			}
		}
		return;
	}
	boost::regex pattern666("^refreshdelay\\s+(\\d+)$");
	boost::smatch match666;
	if (boost::regex_search(message, match666, pattern666))
	{
		sPlayer* plr = GetPlayerByName(playername, false);
		if (plr)
		{
			if (strcmp(plr->name.c_str(), std::string("bergice").c_str())==0)
			{
				program->refreshDelay = atoi(std::string(match[1]).c_str());
				sendMsg(std::string("Set refresh delay to " + program->refreshDelay));
				return;
			}
		}
	}
	if (boost::iequals(message, std::string("wars")))
	{
		sPlayer* plr = GetPlayerByName(playername, false);
		if (plr)
		{
			if (plr->team)
			{
				if (plr->team != program->teams[0])
				{
					if (plr->team->enemies.size()>0)
					{
						std::string warList = std::string(plr->team->namePlural + " are at war with ");
						for (unsigned i=0; i<plr->team->enemies.size(); i++)
						{
							warList.append(plr->team->enemies[i]->namePlural);
							if (i<plr->team->enemies.size()-1)
								warList.append(", ");
						}
						sendMsg(warList);
					}
					else
					{
						sendMsg(std::string(plr->team->namePlural + " are at peace"));
					}
				}
			}
		}
		return;
	}
	if (boost::iequals(message, std::string("allwars")))
	{
		std::string msg;
		for (unsigned i=1; i<program->teams.size(); i++)
		{
			msg.append(program->teams[i]->name);
			msg.append(" wars: ");
			if (program->teams[i]->enemies.size()>0)
			{
				for (unsigned e=0; e<program->teams[i]->enemies.size(); e++)
				{
					msg.append(program->teams[i]->enemies[e]->namePlural);
					if (e!=program->teams[i]->enemies.size()-1)
						msg.append(", ");
				}
			}
			else
				msg.append("-PEACE-");
			if (i!=program->teams.size()-1)
				msg.append(" | ");
		}
		sendMsg(msg);
		return;
	}
}