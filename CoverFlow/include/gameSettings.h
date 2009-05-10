#ifndef _GAMESETTINGS_H_
#define _GAMESETTINGS_H_

#include "coverflow.h"

typedef struct{

	int ocarina;
	int hooktype;
	int language;
	int video;
	int vipatch;
	char lastPlayed[128];
	
} s_gameSettings;

void initGameSettings(s_gameSettings* gs);
bool getGameSettings(char* gameCode, s_gameSettings* gs);
void succa(s_gameSettings* gs);
void setGameSettings(char* gameCode, s_gameSettings* gs);
void createEmptyGameSettingsFile();

#endif
