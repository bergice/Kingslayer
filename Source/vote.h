#pragma once

#include <vector>


struct sPlayer;
struct sTeam;

struct sVote
{
	unsigned short kind;
	sPlayer* target;
	std::vector <sPlayer*> voters;
	sTeam* team;

	void verifyVoters ();
};