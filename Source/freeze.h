#pragma once

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"


struct sPlayer;

struct sFreeze
{
	sPlayer* player;
	time_t time;
};

void FreezeThread (void);