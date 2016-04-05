#include "freeze.h"

#include "program.h"

#include "boost\\lexical_cast.hpp"

#include "player.h"

void FreezeThread (void)
{
	while(true)
	{
		for (unsigned i=0; i<program->frozen.size(); i++)
		{
			if (program->frozen[i].player)
			{
				time_t now; time(&now);
				double seconds = difftime(now, program->frozen[i].time);
				if (seconds>=0)
				{
					std::string msg = "/unfreezeid ";
					msg.append(boost::lexical_cast<std::string>(program->frozen[i].player->id));
					sendRcon(msg);
					program->frozen.erase(program->frozen.begin()+i);
				}
			}
		}
		Sleep(2000);
	}
}