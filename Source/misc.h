#pragma once

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#include <string>

typedef unsigned short  WORD;

#define FOREGROUND_WHITE	FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE
#define BACKGROUND_BLACK	0

void color (WORD c = FOREGROUND_WHITE|BACKGROUND_BLACK);

size_t LevenshteinDistance(const std::string &s1, const std::string &s2);

std::string ClanNameToName (std::string cname);

std::string getTime ();