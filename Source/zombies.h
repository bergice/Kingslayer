#pragma once

#include <string>

void ZombieSetup (std::string configPath);

void ZombieEventThread (void);

bool ZombieApocalypseOn (void);
void ZombieApocalypseStart (void);

struct sPlayer;
void ZombieLockPlayer (sPlayer* plr);