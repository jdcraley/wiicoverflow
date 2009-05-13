#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <ogcsys.h>

#include "cfg.h"

char *cfg_path = "SD:/apps/usbloader/";
//char *setting_path = "SD:/apps/usbloader/settings.cfg";
char current_path[100];

/* configurable fields */

/* default */
int ENTRIES_PER_PAGE = 12;
int MAX_CHARACTERS   = 37;
int CONSOLE_XCOORD   = 260;
int CONSOLE_YCOORD   = 30;
int CONSOLE_WIDTH    = 340;
int CONSOLE_HEIGHT   = 290;
int CONSOLE_FG_COLOR = 15;
int CONSOLE_BG_COLOR = 0;
int COVER_XCOORD     = 28;
int COVER_YCOORD     = 105;

struct CFG CFG;
/*
u8 ocarinaChoice;
u8 videoChoice;
u8 languageChoice;
u8 viChoice;
*/
u8 themeBlack;

#define TITLE_MAX 65

struct ID_Title
{
	u8 id[5];
	char title[TITLE_MAX];
	u8 block;
};

// renamed titles
int num_title = 0; //number of titles
struct ID_Title *cfg_title = NULL;

#define MAX_SAVED_GAMES 1000
int num_saved_games = 0;
struct Game_CFG cfg_game[MAX_SAVED_GAMES];


/* For Mapping */

static char *cfg_name, *cfg_val;

struct TextMap
{
	char *name;
	int id;
};

struct TextMap map_video[] =
{
	{ "system", CFG_VIDEO_SYS },
	{ "game",   CFG_VIDEO_GAME },
	{ "auto",   CFG_VIDEO_DEFAULT },
	{ "patch",  CFG_VIDEO_PATCH },
	{ "pal50", CFG_VIDEO_PAL50 },
	{ "pal60", CFG_VIDEO_PAL60 },
	{ "ntsc", CFG_VIDEO_NTSC },
	{ NULL, -1 }
};

struct TextMap map_language[] =
{
	{ "console",   CFG_LANG_CONSOLE },
	{ "japanese",  CFG_LANG_JAPANESE },
	{ "english",   CFG_LANG_ENGLISH },
	{ "german",    CFG_LANG_GERMAN },
	{ "french",    CFG_LANG_FRENCH },
	{ "spanish",   CFG_LANG_SPANISH },
	{ "italian",   CFG_LANG_ITALIAN },
	{ "dutch",     CFG_LANG_DUTCH },
	{ "s.chinese", CFG_LANG_S_CHINESE },
	{ "t.chinese", CFG_LANG_T_CHINESE },
	{ "korean",    CFG_LANG_KOREAN },
	{ NULL, -1 }
};

struct TextMap map_layout[] =
{
	{ "original",  CFG_LAYOUT_ORIG },
	{ "original1", CFG_LAYOUT_ORIG },
	{ "original2", CFG_LAYOUT_ORIG_12 },
	{ "small",     CFG_LAYOUT_SMALL },
	{ "medium",    CFG_LAYOUT_MEDIUM },
	{ "large",     CFG_LAYOUT_LARGE },
	{ "large2",    CFG_LAYOUT_LARGE_2 },
	{ "ultimate1", CFG_LAYOUT_ULTIMATE1 },
	{ "ultimate2", CFG_LAYOUT_ULTIMATE2 },
	{ "ultimatew", CFG_LAYOUT_ULTIMATE_W },
	{ NULL, -1 }
};

int map_get_id(struct TextMap *map, char *name)
{
	int i;
	for (i=0; map[i].name != NULL; i++)	{
		if (strcmp(name, map[i].name) == 0) return map[i].id;
	}
	return -1;
}

char* map_get_name(struct TextMap *map, short id)
{
	int i;
	for (i=0; map[i].name != NULL; i++)	{
		if (id == map[i].id) return map[i].name;
	}
	return NULL;
}

bool map_auto(char *name, char *name2, char *val, struct TextMap *map, short *var)
{
	if (strcmp(name, name2) != 0) return false;
 	int id = map_get_id(map, val);
	if (id == -1) {
		//printf("MAP FAIL: %s=%s : %d\n", name, val, id); sleep(1);
		return false;
	}
	*var = id;
	//printf("MAP AUTO: %s=%s : %d\n", name, val, id); sleep(1);
	return true;
}

bool cfg_map_auto(char *name, struct TextMap *map, short *var)
{
	return map_auto(name, cfg_name, cfg_val, map, var);
}

bool cfg_map(char *name, char *val, short *var, short id)
{
	if (strcmp(name, cfg_name)==0 && strcmpi(val, cfg_val)==0)
	{
		*var = id;
		return true;
	}
	return false;
}

bool cfg_bool(char *name, short *var)
{
	return (cfg_map(name, "0", var, 0) || cfg_map(name, "1", var, 1));
}

void cfg_int(char *name, short *var, int count)
{
	char tmp[5];
	short i;
	
	if (count > 10) //avoid overflow
		return;
		
	for (i = 0; i < count; i++)
	{
		sprintf(tmp, "%d", i);
		cfg_map(name, tmp, var, i);
	}
}
/* Mapping */

void cfg_layout()
{

	switch (CFG.layout) {
		case CFG_LAYOUT_ORIG: // 1.0+
			ENTRIES_PER_PAGE = 6;
			MAX_CHARACTERS   = 30;
			CONSOLE_XCOORD   = 260;
			CONSOLE_YCOORD   = 115;
			CONSOLE_WIDTH    = 340;
			CONSOLE_HEIGHT   = 218;
			CONSOLE_FG_COLOR = 15;
			CONSOLE_BG_COLOR = 0;
			break;

		case CFG_LAYOUT_ORIG_12: // 1.2+
			ENTRIES_PER_PAGE = 12;
			MAX_CHARACTERS   = 30;
			CONSOLE_XCOORD   = 258;
			CONSOLE_YCOORD   = 112;
			CONSOLE_WIDTH    = 354;
			CONSOLE_HEIGHT   = 304;
			CONSOLE_FG_COLOR = 15;
			CONSOLE_BG_COLOR = 0;
			COVER_XCOORD     = 24;
			COVER_YCOORD     = 104;
			break;

		case CFG_LAYOUT_SMALL: // same as 1.0 + use more space
			ENTRIES_PER_PAGE = 9;
			MAX_CHARACTERS   = 38;
			CONSOLE_XCOORD   = 260;
			CONSOLE_YCOORD   = 115;
			CONSOLE_WIDTH    = 340;
			CONSOLE_HEIGHT   = 218;
			CONSOLE_FG_COLOR = 15;
			CONSOLE_BG_COLOR = 0;
			break;

		case CFG_LAYOUT_MEDIUM:
			ENTRIES_PER_PAGE = 19; //17;
			MAX_CHARACTERS   = 38;
			CONSOLE_XCOORD   = 260;
			CONSOLE_YCOORD   = 50;
			CONSOLE_WIDTH    = 340;
			CONSOLE_HEIGHT   = 380;
			CONSOLE_FG_COLOR = 15;
			CONSOLE_BG_COLOR = 0;
			COVER_XCOORD     = 28;
			COVER_YCOORD     = 175;
			break;

		// nixx:
		case CFG_LAYOUT_LARGE:
			ENTRIES_PER_PAGE = 21;
			MAX_CHARACTERS   = 38;
			CONSOLE_XCOORD   = 260;
			CONSOLE_YCOORD   = 40;
			CONSOLE_WIDTH    = 340;
			CONSOLE_HEIGHT   = 402;
			CONSOLE_FG_COLOR = 15;
			CONSOLE_BG_COLOR = 0;
			COVER_XCOORD     = 28;
			COVER_YCOORD     = 175;
			break;

		// usptactical:
		case CFG_LAYOUT_LARGE_2:
			ENTRIES_PER_PAGE = 21;
			MAX_CHARACTERS   = 38;
			CONSOLE_XCOORD   = 260;
			CONSOLE_YCOORD   = 40;
			CONSOLE_WIDTH    = 340;
			CONSOLE_HEIGHT   = 402;
			CONSOLE_FG_COLOR = 15;
			CONSOLE_BG_COLOR = 0;
			COVER_XCOORD     = 30;
			COVER_YCOORD     = 180;
			break;

		// Ultimate1: (WiiShizza) White background
		case CFG_LAYOUT_ULTIMATE1:
			ENTRIES_PER_PAGE = 12;
			MAX_CHARACTERS   = 37;
			CONSOLE_XCOORD   = 260;
			CONSOLE_YCOORD   = 30;
			CONSOLE_WIDTH    = 340;
			CONSOLE_HEIGHT   = 290;
			CONSOLE_FG_COLOR = 0;
			CONSOLE_BG_COLOR = 15;
			COVER_XCOORD     = 28;
			COVER_YCOORD     = 105;
			break;

		// Ultimate2: (jservs7 / hungyip84)
		case CFG_LAYOUT_ULTIMATE2:
			ENTRIES_PER_PAGE = 12;
			MAX_CHARACTERS   = 37;
			CONSOLE_XCOORD   = 40;
			CONSOLE_YCOORD   = 71;
			CONSOLE_WIDTH    = 340;
			CONSOLE_HEIGHT   = 290;
			CONSOLE_FG_COLOR = 0;
			CONSOLE_BG_COLOR = 15;
			COVER_XCOORD     = 446;
			COVER_YCOORD     = 109;
			break;
		
		case CFG_LAYOUT_ULTIMATE_W: // Ultimate with widescreen
			ENTRIES_PER_PAGE = 12;
			MAX_CHARACTERS   = 37;
			CONSOLE_XCOORD   = 40;
			CONSOLE_YCOORD   = 71;
			CONSOLE_WIDTH    = 340;
			CONSOLE_HEIGHT   = 290;
			CONSOLE_FG_COLOR = 0;
			CONSOLE_BG_COLOR = 15;
			COVER_XCOORD     = 482;
			COVER_YCOORD     = 110;
			break;
	}
}

static char bg_path[100];

void CFG_Default()
{
	u8 widescreen = CONF_GetAspectRatio();
	
	if (widescreen)
	{
		snprintf(bg_path, sizeof(bg_path), "SD:/wimages/wbg.png");
		CFG.layout = CFG_LAYOUT_ULTIMATE_W;
		CFG.widescreen = 1;
		snprintf(CFG.images_path, sizeof(CFG.images_path), "SD:/wimages/");
	}
	else
	{
		snprintf(bg_path, sizeof(bg_path), "SD:/images/bg.png");
		CFG.layout = CFG_LAYOUT_ULTIMATE2;
		CFG.widescreen = 0;
		snprintf(CFG.images_path, sizeof(CFG.images_path), "SD:/images/");
	}

	CFG.background = bg_path;
	CFG.covers  = 1;
	CFG.simple  = 0;
	CFG.video	= CFG_VIDEO_DEFAULT;
	CFG.home	= CFG_HOME_REBOOT;
	CFG.download = 0;
	CFG.language = CFG_LANG_CONSOLE;
	CFG.ocarina = 0;
//	CFG.themeblack = 1;
	CFG.vipatch = 0;
	CFG.savesettings = 0;
	CFG.parentalcontrol = 0;
	CFG.installdownload = 0;
	CFG.hidesettingmenu = 0;
	cfg_layout();
}


char* strcopy(char *dest, char *src, int size)
{
	strncpy(dest,src,size);
	dest[size-1] = 0;
	return dest;
}

char *cfg_get_title(u8 *id)
{
	int i;
	for (i=0; i<num_title; i++) {
		if (memcmp(id, cfg_title[i].id, 4) == 0) {
			return cfg_title[i].title;
		}
	}
	return NULL;
}

char *get_title(struct discHdr *header)
{
	char *title = cfg_get_title(header->id);
	if (title) return title;
	return header->title;
}

void title_set(char *id, char *title, u8 block)
{
	char *idt = cfg_get_title((u8*)id);
	if (idt) {
		// replace
		strcopy(idt, title, TITLE_MAX);
	} else {
		cfg_title = realloc(cfg_title, (num_title+1) * sizeof(struct ID_Title));
		if (!cfg_title) {
			// error
			num_title = 0;
			return;
		}
		// add
		memcpy(cfg_title[num_title].id, id, 4);
		cfg_title[num_title].id[4] = 0;
		cfg_title[num_title].block = block;
		strcopy(cfg_title[num_title].title, title, TITLE_MAX);
		num_title++;
	}
}

u8 cfg_get_block(u8 *id)
{
	int i;
	for (i=0; i<num_title; i++) {
		if (memcmp(id, cfg_title[i].id, 4) == 0) {
			return cfg_title[i].block;
		}
	}
	return 0;
}

u8 get_block(struct discHdr *header)
{
	return cfg_get_block(header->id);
}

// trim leading and trailing whitespace
// copy at max n or at max size-1
char* trim_n_copy(char *dest, char *src, int n, int size)
{
	int len;
	// trim leading white space
	while (isspace(*src)) { src++; n--; }
	len = strlen(src);
	if (len > n) len = n;
	// trim trailing white space
	while (len > 0 && isspace(src[len-1])) len--;
	if (len >= size) len = size-1;
	strncpy(dest, src, len);
	dest[len] = 0;
	//printf("trim_copy: '%s' %d\n", dest, len); //sleep(1);
	return dest;
}

char* trimcopy(char *dest, char *src, int size)
{
	int len;
	while (*src == ' ') src++;
	len = strlen(src);
	// trim trailing " \r\n"
	while (len > 0 && strchr(" \r\n", src[len-1])) len--;
	if (len >= size) len = size-1;
	strncpy(dest, src, len);
	dest[len] = 0;
	return dest;
}

void widescreen_set(char *name, char *val)
{
	cfg_name = name;
	cfg_val = val;
	
	if (cfg_bool("widescreen", &CFG.widescreen)) //reset default
	{
		if (CFG.widescreen) {
			snprintf(bg_path, sizeof(bg_path), "SD:/wimages/wbg.png");
			CFG.layout = CFG_LAYOUT_ULTIMATE_W;
			snprintf(CFG.images_path, sizeof(CFG.images_path), "SD:/wimages/");
			cfg_layout();
		}
		else
		{
			snprintf(bg_path, sizeof(bg_path), "SD:/images/bg.png");
			CFG.layout = CFG_LAYOUT_ULTIMATE2;
			snprintf(CFG.images_path, sizeof(CFG.images_path), "SD:/images/");
			cfg_layout();
		}
	}
}
void cfg_set(char *name, char *val)
{
	cfg_name = name;
	cfg_val = val;

	if (!CFG.widescreen &&(strcmp(name, "images_path") == 0)) {
		strcopy(CFG.images_path, val, sizeof(CFG.images_path));
		snprintf(bg_path, sizeof(bg_path), "%sbg.png", CFG.images_path); //reset path
		return;
	}

	if (CFG.widescreen && strcmp(name, "wimages_path") == 0) {
		strcopy(CFG.images_path, val, sizeof(CFG.images_path));
		snprintf(bg_path, sizeof(bg_path), "%swbg.png", CFG.images_path); //reset path
		return;
	}

	if (cfg_map_auto("video", map_video, &CFG.video))
		return;
	
	if (cfg_map_auto("language", map_language, &CFG.language))
		return;

	if (cfg_map_auto("layout", map_layout, &CFG.layout)) {
		cfg_layout();
	}
	
	cfg_bool("ocarina", &CFG.ocarina);
	cfg_bool("covers", &CFG.covers);
	cfg_bool("download",  &CFG.download);
	cfg_bool("savesettings", &CFG.savesettings);
	cfg_bool("installdownload", &CFG.installdownload);
	cfg_bool("hidesettingmenu", &CFG.hidesettingmenu);
	cfg_map("home", "exit",   &CFG.home, CFG_HOME_EXIT);
	cfg_map("home", "reboot", &CFG.home, CFG_HOME_REBOOT);
	cfg_int("simple", &CFG.simple, 3);
	cfg_int("parentalcontrol", &CFG.parentalcontrol, 4);
}

void console_set(char *name, char *val)
{
	cfg_name = name;
	cfg_val = val;

	if ((!CFG.widescreen && (strcmp(cfg_name, "background")==0)) || (CFG.widescreen && (strcmp(cfg_name, "wbackground") == 0))){
		if (strcmp(val, "0")==0) {
			CFG.background = NULL;
		} 
		else {
			snprintf(bg_path, sizeof(bg_path), "%s%s", CFG.images_path, val);
			CFG.background = bg_path;
		}
	}
	
	else if ((!CFG.widescreen && (strcmp(cfg_name, "console_coords")==0)) || (CFG.widescreen && (strcmp(cfg_name, "wconsole_coords")==0))) {
		int x,y,w,h;
		if (sscanf(val, "%d,%d,%d,%d", &x, &y, &w, &h) == 4) {
			CONSOLE_XCOORD = x;
			CONSOLE_YCOORD = y;
			CONSOLE_WIDTH  = w;
			CONSOLE_HEIGHT = h;
		}
	}
	
	else if (strcmp(cfg_name, "console_color")==0) {
		int fg,bg;
		if (sscanf(val, "%d,%d", &fg, &bg) == 2) {
			CONSOLE_FG_COLOR = fg;
			CONSOLE_BG_COLOR = bg;
		}
	}
	
	else if ((!CFG.widescreen && (strcmp(cfg_name, "covers_coords")==0)) || (CFG.widescreen && (strcmp(cfg_name, "wcovers_coords")==0))) {
		int x,y;
		if (sscanf(val, "%d,%d", &x, &y) == 2) {
			COVER_XCOORD = x - (x % 4);
			COVER_YCOORD = y;
		}
	}
	
	else if (strcmp(cfg_name, "entry_lines") == 0) {
		int x;
		if (sscanf(val, "%d", &x) == 1) {
			ENTRIES_PER_PAGE = x;
		}
	}
	
	else if (strcmp(cfg_name, "max_characters") == 0) {
		int x;
		if (sscanf(val, "%d", &x) == 1) {
			MAX_CHARACTERS = x;
		}
	}
}
// split line to part1 delimiter part2 
bool trimsplit(char *line, char *part1, char *part2, char delim, int size)
{
	char *eq = strchr(line, delim);
	if (!eq) return false;
	trim_n_copy(part1, line, eq-line, size);
	trimcopy(part2, eq+1, size);
	return true;
}
void cfg_parseline(char *line, void (*set_func)(char*, char*))
{
	// split name = value
	char tmp[200], name[100], val[100];
	strcopy(tmp, line, sizeof(tmp));
	char *eq = strchr(tmp, '=');
	if (!eq) return;
	*eq = 0;
	trimcopy(name, tmp, sizeof(name));
	trimcopy(val, eq+1, sizeof(val));
	//printf("CFG: %s = %s\n", name, val);
	set_func(name, val);
}

void cfg_parsetitleline(char *line, void (*set_func)(char*, char*, u8))
{
	// split name = value
	char tmp[200], name[100], val[100];
	int block = 0;
	strcopy(tmp, line, sizeof(tmp));
	char *eq = strchr(tmp, '=');
	if (!eq) return;
	*eq = 0;
	trimcopy(name, tmp, sizeof(name));
	
	char *blockpos = strrchr(eq+1, '=');
	
	if (!blockpos)
		trimcopy(val, eq+1, sizeof(val));
		
	else
	{
		*blockpos = 0;
		trimcopy(val, eq+1, sizeof(val));
		if (sscanf(blockpos+1, "%d", &block) != 1)
		{
			block = 0;
		}
	}
	set_func(name, val, block);
}
	
bool cfg_parsefile(char *fname, void (*set_func)(char*, char*))
{
	FILE *f;
	char line[200];

	//printf("opening(%s)\n", fname);
	f = fopen(fname, "rb");
	if (!f) {
		//printf("error opening(%s)\n", fname);
		return false;
	}

	while (fgets(line, sizeof(line), f)) {
		// lines starting with # are comments
		if (line[0] == '#') continue;
		cfg_parseline(line, set_func);
	}
	return true;
}

bool cfg_parsetitlefile(char *fname, void (*set_func)(char*, char*, u8))
{
	FILE *f;
	char line[200];

	//printf("opening(%s)\n", fname);
	f = fopen(fname, "rb");
	if (!f) {
		//printf("error opening(%s)\n", fname);
		return false;
	}

	while (fgets(line, sizeof(line), f)) {
		// lines starting with # are comments
		if (line[0] == '#') continue;
		cfg_parsetitleline(line, set_func);
	}
	return true;
}

void cfg_parsearg(int argc, char **argv)
{
	int i;
	char *eq;
	char pathname[200];
	for (i=1; i<argc; i++) {
		//printf("arg[%d]: %s\n", i, argv[i]);
		eq = strchr(argv[i], '=');
		if (eq) {
			cfg_parseline(argv[i], &cfg_set);
		} else {
			snprintf(pathname, sizeof(pathname), "%s%s", cfg_path, argv[i]);
			cfg_parsefile(pathname, &cfg_set);
		}
	}
}


// PER-GAME SETTINGS


// return existing or new
struct Game_CFG* cfg_get_game(u8 *id)
{
	struct Game_CFG *game = CFG_get_game_opt(id);
	if (game) return game;
	if (num_saved_games >= MAX_SAVED_GAMES) return NULL;
	game = &cfg_game[num_saved_games];
	num_saved_games++;
	return game;
}

// current options to game
void cfg_set_game_opt(struct Game_CFG *game, u8 *id)
{
	strncpy((char*)game->id, (char*)id, 6);
	game->id[6] = 0;
	game->video = videoChoice;
	game->language = languageChoice;
	game->ocarina = ocarinaChoice;
	game->vipatch = viChoice;
}

void game_set(char *name, char *val)
{
	// sample line:
	// game:RTNP41 = video:game; language:english; ocarina:0;
	// game:RYWP01 = video:patch; language:console; ocarina:1;
	//printf("GAME: '%s=%s'\n", name, val);
	u8 id[8];
	struct Game_CFG *game;
	if (strncmp(name, "game:", 5) != 0) return;
	trimcopy((char*)id, name+5, sizeof(id)); 
	game = cfg_get_game(id);
	// set id and current options as default
	cfg_set_game_opt(game, id);
	//printf("GAME(%s) '%s'\n", id, val); sleep(1);

	// parse val
	// first split options by ;
	char opt[100], *p, *np;
	p = val;

	while(p) {
		np = strchr(p, ';');
		if (np) trim_n_copy(opt, p, np-p, sizeof(opt));
		else trimcopy(opt, p, sizeof(opt));
		//printf("GAME(%s) (%s)\n", id, opt); sleep(1);
		// parse opt 'language:english'
		char opt_name[100], opt_val[100];
		if (trimsplit(opt, opt_name, opt_val, ':', sizeof(opt_name))){
			//printf("GAME(%s) (%s=%s)\n", id, opt_name, opt_val); sleep(1);
			short opt_v, opt_l, opt_c;
			if (map_auto("video", opt_name, opt_val, map_video, &opt_v)) {
				// valid option, assign
				game->video = opt_v;
			}
			if (map_auto("language", opt_name, opt_val, map_language, &opt_l)) {
				// valid option, assign
				game->language = opt_l;
			}
			if (strcmp("ocarina", opt_name) == 0) {
				if (sscanf(opt_val, "%hd", &opt_c) == 1) {
					game->ocarina = opt_c;
				}
			}
			if (strcmp("vipatch", opt_name) == 0) {
				if (sscanf(opt_val, "%hd", &opt_c) == 1) {
					game->vipatch = opt_c;
				}
			}
		}
		// next opt
		if (np) p = np + 1; else p = NULL;
	}
}

bool cfg_load_games()
{
	return cfg_parsefile("settings.cfg", &game_set);
}

bool cfg_save_games()
{
	FILE *f;
	int i;

	f = fopen("settings.cfg", "wb");
	if (!f) {
		printf("Error saving %s\n", "settings.cfg");
		sleep(1);
		return false;
	}
	fprintf(f, "# USB Loader settings file\n");
	fprintf(f, "# note: this file is automatically generated\n");
	fprintf(f, "# Num Games: %d\n", num_saved_games);
	for (i=0; i<num_saved_games; i++) {
		char *s;
		fprintf(f, "game:%s = ", cfg_game[i].id);
		s = map_get_name(map_video, cfg_game[i].video);
		if (s) fprintf(f, "video:%s; ", s);
		s = map_get_name(map_language, cfg_game[i].language);
		if (s) fprintf(f, "language:%s; ", s);
		fprintf(f, "ocarina:%d; ", cfg_game[i].ocarina);
		fprintf(f, "vipatch:%d;\n", cfg_game[i].vipatch);
	}
	fprintf(f, "# END\n");
	fclose(f);
	return true;
}

struct Game_CFG* CFG_get_game_opt(u8 *id)
{
	int i;
	for (i=0; i<num_saved_games; i++) {
		if (memcmp(id, cfg_game[i].id, 6) == 0) {
			return &cfg_game[i];
		}
	}
	return NULL;
}

bool CFG_save_game_opt(u8 *id)
{
	struct Game_CFG *game = cfg_get_game(id);
	if (!game) return false;
	cfg_set_game_opt(game, id);
	return cfg_save_games();
}

bool CFG_forget_game_opt(u8 *id)
{
	struct Game_CFG *game = CFG_get_game_opt(id);
	int i;
	if (!game) return true;
	// move entries down
	num_saved_games--;
	for (i=game-cfg_game; i<num_saved_games; i++) {
		cfg_game[i] = cfg_game[i+1];
	}
	memset(&cfg_game[num_saved_games], 0, sizeof(struct Game_CFG));
	return cfg_save_games();
}

void chdir_app(char *arg)
{
	char appdir[100] = "SD:";
	char dir[100], *pos1, *pos2;

	pos1 = strchr(arg, ':');
	// trim file name
	pos2 = strrchr(arg, '/');
	if (pos1 && pos2) {
		pos1++;
		pos2++;
		strncpy(dir, pos1, pos2 - pos1);
		dir[pos2 - pos1] = 0;
		strcat(appdir,dir);
		chdir(appdir);
		strncpy(current_path, appdir, sizeof(current_path));
	}
}   

void CFG_Load(int argc, char **argv)
{
	char pathname[200];
	bool ret = false;

	//set app path
	chdir_app(argv[0]);
	
	CFG_Default();
	
	snprintf(pathname, sizeof(pathname), "%s", "config.txt");
	
	cfg_parsefile(pathname, &widescreen_set); //first set widescreen
	cfg_parsefile(pathname, &cfg_set); //then set config and layout options
	ret = cfg_parsefile(pathname, &console_set); //finally set console information
	
	if (!ret)
	{
		cfg_parsefile("SD:/config.txt", &widescreen_set);
		cfg_parsefile("SD:/config.txt", &cfg_set);
		cfg_parsefile("SD:/config.txt", &console_set);
	}
	
	snprintf(pathname, sizeof(pathname), "%s%s", current_path, "titles.txt");
	cfg_parsetitlefile(pathname, &title_set);
	
	// load per-game settings
	cfg_load_games();
		
	
	cfg_parsearg(argc, argv);
}

void CFG_Cleanup(void)
{
	if (cfg_title)
	{
		free(cfg_title);
		cfg_title = NULL;
	}
}