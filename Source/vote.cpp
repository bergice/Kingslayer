#include "vote.h"

#include "player.h"


void sVote::verifyVoters ()
	{
		for (unsigned i=0; i<voters.size(); i++)
		{
			if (!voters[i])
				voters.erase(voters.begin()+i);
			else
			{
				if (voters[i]->team!=team)
					voters.erase(voters.begin()+i);
			}
		}
	}