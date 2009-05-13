#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "coverflow.h"

typedef struct {
    float coverZoom;
    float coverAngle;
    float coverSpacing;
    int drawWindow;
    int coverText;
    int theme;
    int fontColor; // stores the hex val of the theme

	int volume;

    int sound;
    int music;
    int parentalLock;
    int rumble;
    int quickstart;
    int enablepitch; //0=OFF, 1=ON skip the load disc dialog

    int ocarina;
    int hooktype;
    int language;
    int video;
    int vipatch;
	char localLanguage[5];
} s_settings;


void SETTINGS_Init(void);

int SETTINGS_Load();

int SETTINGS_Save();

#endif