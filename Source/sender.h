#pragma once

#include <string>

void sendMsg (std::string message);
void EnableBroadcasting (bool state);

void sendRcon (std::string message);

void SwapThread ();
void swapid (int id);

#define SOUND_FANFARE		"Entities/Rooms/RP/Room_Fanfare.cfg"
#define SOUND_LOSTWAR		"Entities/Rooms/RP/Room_Lostwar.cfg"
#define SOUND_MONSTER		"Entities/Rooms/RP/Room_Monster.cfg"
#define SOUND_NEWKING		"Entities/Rooms/RP/Room_Newking.cfg"
#define SOUND_WAGEWAR		"Entities/Rooms/RP/Room_Wagewar.cfg"

void playSound (int x, int y, std::string sound);
void addBot    ();


void DoWelcomeMessage (std::string playername);

void RulesThread (void);

// New commands
struct sPlayer;
void Ban (sPlayer* plr, int minutes, char* reason="");
void Kick (sPlayer* plr, char* reason);
void Freeze (sPlayer* plr, int seconds, char* reason="");
void Unfreeze (sPlayer* plr);
void Mute (sPlayer* plr, int minutes);
void Unmute (sPlayer* plr);

void EnablePunishing (bool state);

// FTP and Security Commands
void _assignseclev (std::string filename, std::string username, std::string list = "users = ");
void _removeseclev (std::string filename, std::string username, std::string list = "users = ");
void _removeallseclev (std::string filename, std::string list = "users = ");

void SetFTPSettings (std::string configPath);
bool FTPUpload (std::string remotename, std::string localname);
bool FTPDownload (std::string remotename, std::string localname);

bool AssignSeclev (std::string username, std::string seclevFileName, std::string list = "users = ");
bool RemoveSeclev (std::string username, std::string seclevFileName, std::string list = "users = ");

bool RemoveAllSeclev (std::string seclevFileName, std::string list = "users = ");