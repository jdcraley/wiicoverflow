/*---------------------------------------------------------------------------------

	CoverFlow by Beardface

---------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include "wpad.h"

#include "wbfs.h"
#include "wdvd.h"
#include "libwbfs/libwbfs.h"

#include "disc.h"

//#include <iostream> 
 
#include "GRRLIB.h"
#include "pngu/pngu.h"
#include "fat.h"
#include "sys.h"
 
#define COVER_WIDTH		160
#define COVER_HEIGHT		224

//#include <vector>
 
#define DEFAULT_FIFO_SIZE	(256*1024)

//#define TEST_MODE 1

/* Constants */
#define ENTRIES_PER_PAGE	12
#define MAX_CHARACTERS		30

/* Gamelist buffer */
static struct discHdr *gameList = NULL;


static wbfs_t *hdd = NULL;

/* Gamelist variables */
static s32 gameCnt = 0, gameSelected = 0, gameStart = 0;

/* WBFS device */
static s32 my_wbfsDev = WBFS_DEVICE_USB;

#define BACK_BUTTON   1000
#define CANCEL_BUTTON 1001
#define OK_BUTTON     1002


float shift = 0.0;
float select_shift = 0.0;

bool selected = false;

float progress = 0.0;

int DRAW_WINDOW = 7;

#ifdef TEST_MODE
int COVER_COUNT = 29;
#else
int COVER_COUNT = 0;
#endif

float animate_rotate = 0.0;
float animate_flip = 0.0;
float FLIP_SPEED   = 0.016;

float SCROLL_SPEED = 0.050;
/*
extern const u8		RC8P7D_png[];
extern const u32	RC8P7D_png_size;

extern const u8		R2AE7D_png[];
extern const u32	R2AE7D_png_size;

extern const u8		R2DEAP_png[];
extern const u32	R2DEAP_png_size;

extern const u8		R2DEEB_png[];
extern const u32	R2DEEB_png_size;

extern const u8		R2FE5G_png[];
extern const u32	R2FE5G_png_size;

extern const u8		R2HE41_png[];
extern const u32	R2HE41_png_size;
*/
extern const u8		no_cover_png[];
extern const u32	no_cover_png_size;

extern const u8		back_cover_png[];
extern const u32	back_cover_png_size;

extern const u8		no_disc_png[];
extern const u32	no_disc_png_size;

extern const u8		select_menu_png[];
extern const u32	select_menu_png_size;

extern const u8     font1_png[];
extern const u32    font1_png_size;

extern const u8     loading_main_png[];
extern const u32    loading_main_png_size;

extern const u8     progress_png[];
extern const u32    progress_png_size;

extern const u8     gradient_bg_png[];
extern const u32    gradient_bg_png_size;

extern const u8     slide_png[];
extern const u32    slide_png_size;

extern const u8     slide_bar_png[];
extern const u32    slide_bar_png_size;

extern const u8     usb_error_png[];
extern const u32    usb_error_png_size;

extern const u8     generic_point_png[];
extern const u32    generic_point_png_size;


GRRLIB_texImg cover_texture;
GRRLIB_texImg back_texture;

GRRLIB_texImg empty_texture;

GRRLIB_texImg no_disc_texture;
GRRLIB_texImg current_cover_texture;

GRRLIB_texImg select_menu_texture;

GRRLIB_texImg text_font1;

GRRLIB_texImg loader_main_texture;

GRRLIB_texImg progress_texture;

GRRLIB_texImg gradient_texture;

GRRLIB_texImg slide_texture;
GRRLIB_texImg slide_bar_texture;

GRRLIB_texImg usb_error_texture;

/*--------------------------------------
  Button Textures
---------------------------------------*/
#include "button.h"

extern const u8     add_button_png[];
extern const u8    add_button_hover_png[];

Button addButton;

/*--------------------------------------*/

#define USBLOADER_PATH		"SD:/usb-loader"

Mtx GXmodelView2D;

#define MAX_COVERS 20
int array_size = 0;
GRRLIB_texImg covers[MAX_COVERS];      //std::vector<GRRLIB_texImg> covers;

float p_ang = 0;
float p_x   = 0;
float p_y   = 0;

WPADData *wd;

GRRLIB_texImg pointer_texture;

void Init_Buttons()
{
	addButton = Button_Init(add_button_png, add_button_hover_png, 550, 400);
}

void Hover_Buttons()
{
	Button_Hover(&addButton, p_x, p_y);
}

float change_scale_without_containing(float val, float in_min, float in_max, 
                                      float out_min, float out_max)
{
  float percent = 0;
  if (in_min == in_max) {
    return 0;
  }
  percent = (val - in_min) / (in_max - in_min);
  return (out_min + percent * (out_max - out_min));
}

float change_scale(float val, float in_min, float in_max, 
                   float out_min, float out_max)
{
  if(val > in_max)
  {
	val = in_max;
  }
  
  if(val < in_min)
  {
     val = in_min;
  }
  
  return change_scale_without_containing(val, in_min, in_max, out_min, out_max);
}


void Paint_Progress(float v)
{
	int count = (int)(v*10);
	
	if(count > 40)
		count = 40;
	int i;
	
	GRRLIB_DrawImg(0, 0, gradient_texture, 0, 1, 1, 0xFFFFFFFF);
	
	for(i = 0; i < count; i++)
	{
		GRRLIB_DrawImg(165+12*i, 231, progress_texture, 0, 1, 1, 0xFFFFFFFF);
	}

	GRRLIB_DrawImg(0, 0, loader_main_texture, 0, 1, 1, 0xFFFFFFFF);
    
	GRRLIB_Render();
}

void LoadCurrentCover(int id)
{

	#ifndef TEST_MODE
	void *imgData;// = (void *)no_cover_png;

	char filepath[128];
	s32  ret;

	struct discHdr *header = &gameList[id];
	
//		substr
	
	sprintf(filepath, USBLOADER_PATH "/disks/%c%c%c.png", header->id[0],header->id[1],header->id[2]);

	ret = Fat_ReadFile(filepath, &imgData);
	
	
	if (ret > 0) {
		current_cover_texture = GRRLIB_LoadTexture((const unsigned char*)imgData);
	}
	else
	{
		current_cover_texture = no_disc_texture;
	}
	
	#else
	current_cover_texture = no_disc_texture;
	#endif
	
}

void AddCover(GRRLIB_texImg tex)
{
	if(array_size < MAX_COVERS)
	{
		covers[array_size] = tex;
		array_size++;
	}
	/*
	array_size = array_size + 1;
	covers = (GRRLIB_texImg*)realloc(covers, (array_size * sizeof(GRRLIB_texImg)));

	if (covers == NULL)
	{
	   // fprintf(stdout, "ERROR: Couldn't realloc memory!");
	   return;
	}

    covers[array_size - 1] = tex;
	*/
}

void Init_Covers()
{
	int i;
	
	cover_texture = GRRLIB_LoadTexture(no_cover_png);
	back_texture = GRRLIB_LoadTexture(back_cover_png);
    no_disc_texture = GRRLIB_LoadTexture(no_disc_png);
    select_menu_texture = GRRLIB_LoadTexture(select_menu_png);
	text_font1 = GRRLIB_LoadTexture(font1_png);
	
	slide_texture = GRRLIB_LoadTexture(slide_png);
	slide_bar_texture = GRRLIB_LoadTexture(slide_bar_png);
	
    GRRLIB_InitTileSet(&text_font1, 32, 36, 32);
	
	progress+=0.05;
	Paint_Progress(progress);
	
	float max_progress = 2.1;
	
	float per_game_prog = max_progress/gameCnt;
	
	#ifndef TEST_MODE
	for(i = 0; i < gameCnt; i++)
	{
		void *imgData;// = (void *)no_cover_png;

		char filepath[128];
		s32  ret;

		struct discHdr *header = &gameList[i];
		
		if(array_size < MAX_COVERS)
		{
			sprintf(filepath, USBLOADER_PATH "/covers/%s.png", header->id);

			ret = Fat_ReadFile(filepath, &imgData);
			
			
			if (ret > 0) {

				GRRLIB_texImg tmpTex = GRRLIB_LoadTexture((const unsigned char*)imgData);
				
				if ((tmpTex.w > COVER_WIDTH) || (tmpTex.h > COVER_HEIGHT))
				{
					AddCover(cover_texture);
				}
				else
				{
					AddCover(tmpTex);
				}
			}
			else
			{
				AddCover(cover_texture);
			}
		}
		progress+=per_game_prog;
		Paint_Progress(progress);
	}
	
	#else
	
	int CoverCount = COVER_COUNT;
	
	for(i = 0; i < CoverCount; i++)
	{
		AddCover( GRRLIB_LoadTexture(no_cover_png) );
		progress+=per_game_prog;
		Paint_Progress(progress);
	}
	
	#endif
}

void GRRLIB_Cover(float pos, int texture_id)
{
		if(pos == 0)
			gameSelected = texture_id;

	  if((selected || animate_flip > 0) && pos == 0)
	  {
		return;
	  }

	  static const float SPACING = 3.5;
	  float dir = 1;
	  float loc, scale, angle;

	  if (pos < 0) {
		dir *= -1;
		pos *= -1;
	  }
	  
	  loc = SPACING * dir * (pow(pos + 1, -1) - 1);
	  scale = pow(pos + 1, -2);
	  angle = -1 * dir * change_scale(scale, 0, 1, 45, 0);
	
	  if(texture_id != -1 && texture_id < array_size)
	  {
			GRRLIB_DrawCoverImg(loc*1.2,covers[texture_id],angle,1.0,0xFFFFFFFF);
	  }
	  else
	  {
			GRRLIB_DrawCoverImg(loc*1.2,cover_texture,angle,1.0,0xFFFFFFFF);
	  }
}

void draw_selected()
{
	
	if(selected && animate_flip < 1.0)
	{
		animate_flip += FLIP_SPEED;
		if(animate_flip > 1.0)
			animate_flip = 1.0;
			
		if(animate_flip > 0.3 && animate_flip < 0.7)
		{
			animate_flip = 0.7;
		}
			
	}
	else if(!selected)
	{
		animate_flip -= FLIP_SPEED;
		
		if(animate_flip > 0.3 && animate_flip < 0.7)
		{
			animate_flip = 0.3;
		}
		
		if(animate_flip < 0)
		{
			animate_flip = 0;
		}
	}
	
	
	  static const float SPACING = 2.8;
	  float dir = 1;
	  float loc, scale, angle;
	  
	  loc = SPACING * dir * (pow(1, -1) - 1);
	  scale = change_scale(animate_flip, 0, 1, 0, 360);
	  angle = -1 * dir * scale;
	
	  if(scale >= 180)
	  {
		//Use back art texture
		GRRLIB_DrawCoverImg(loc*1.2,back_texture,angle,2.0,0xFFFFFFFF);
		
		if(scale >= 360)
		{
			int i ;
			int len;
			animate_rotate++;
			if(animate_rotate == 360) animate_rotate = 0;
			
			GRRLIB_DrawImg(230,100, current_cover_texture, animate_rotate, 1, 1, 0xFFFFFFFF);
			GRRLIB_DrawImg(170,260, select_menu_texture, 0, 1, 1, 0xFFFFFFFF);
			
			#ifndef TEST_MODE
			struct discHdr *header = NULL;
			header = &gameList[gameSelected];
			f32 size = 0.0;

			/* Get game size */
			WBFS_GameSize(header->id, &size);
			char name[64];
			
			for(i = 0; i < 64; i++)
				name[i] = toupper(header->title[i]);
				
			len = strlen(name);
			float tsize = .8;

			if(len > 20)
				tsize = .6;

			if(len > 28)
				tsize = .4;
				
			if(len > 40)
				tsize = .3;
			
			int offset = (len*10);
			
			if(offset > 240) offset = 240;
			
            GRRLIB_Printf(300 - offset, 10, text_font1, 0XFFFFFFFF, tsize, "%s", name);
            GRRLIB_Printf(210, 50, text_font1, 0XFFFFFFFF, .4, "(%c%c%c%c) (%.2fGB)", header->id[0], header->id[1], header->id[2], header->id[3], size);
			#else
            GRRLIB_Printf(90, 10, text_font1, 0XFFFFFFFF, .8, "%s", "JUSTINS GAME");
            GRRLIB_Printf(180, 50, text_font1, 0XFFFFFFFF, .5, "%s", "JUSTINS GAME");
			#endif
		}
	  }
	  else
	  {
		GRRLIB_DrawCoverImg(loc*1.2,covers[gameSelected],angle,1.0,0xFFFFFFFF);
	  }
}

void draw_game_title(int index)
{
	if(index != -1)
	{
		    int i = 0;
			int len = 0;
			struct discHdr *header = NULL;
			header = &gameList[index];
			
			char name[17];
			
			for(i = 0; i < 16; i++)
				name[i] = toupper(header->title[i]);
				
			name[16] = 0;
			
			float tsize = .5;

			len = strlen(name);
			
			int offset = (len*10);
			
			if(offset > 240) offset = 240;
			
            GRRLIB_Printf(340 - offset, 400, text_font1, 0XFFFFFF40, tsize, "%s", name);
				
	}

}

void draw_covers()
{
	int i;
	
	for(i = (-1*(COVER_COUNT/2.0)); i < (COVER_COUNT/2.0); i++)
	{
		
		/*Some logic to avoid drawing everything*/
		if(abs(shift+i) < DRAW_WINDOW)
		{
			//printf("Shift: %f, %d\n", shift, i);
			GRRLIB_Cover(i+shift, i+(COVER_COUNT/2.0));
		}
	}
}

s32 __Menu_EntryCmp(const void *a, const void *b)
{
	struct discHdr *hdr1 = (struct discHdr *)a;
	struct discHdr *hdr2 = (struct discHdr *)b;

	/* Compare strings */
	return strcmp(hdr1->title, hdr2->title);
}

s32 GetEntries(void)
{
	struct discHdr *buffer = NULL;

	u32 cnt, len;
	s32 ret;

	/* Get list length */
	ret = WBFS_GetCount(&cnt);
	if (ret < 0)
		return ret;

	/* Buffer length */
	len = sizeof(struct discHdr) * cnt;

	/* Allocate memory */
	buffer = (struct discHdr *)memalign(32, len);
	if (!buffer)
		return -1;

	/* Clear buffer */
	memset(buffer, 0, len);

	progress+=0.05;
	Paint_Progress(progress);
	
	/* Get header list */
	ret = WBFS_GetHeaders(buffer, cnt, sizeof(struct discHdr));
	if (ret < 0)
		goto err;

	progress+=0.05;
	Paint_Progress(progress);
	
	/* Sort entries */
	qsort(buffer, cnt, sizeof(struct discHdr), __Menu_EntryCmp);

	progress+=0.05;
	Paint_Progress(progress);
	
	/* Free memory */
	if (gameList)
		free(gameList);

	/* Set values */
	gameList = buffer;
	gameCnt  = cnt;
	COVER_COUNT = gameCnt;
	
	Init_Covers();

	progress+=0.05;
	Paint_Progress(progress);
	
	/* Reset variables */
	gameSelected = gameStart = 0;

	return 0;

err:
	/* Free memory */
	if (buffer)
		free(buffer);

	return ret;
}

bool init_usbfs()
{    
   // __Disc_SetLowMem();

	s32 ret;

	
	/* Initialize system */
	Sys_Init();

	///* Initialize subsystems */
	Wpad_Init();

	/* Mount SDHC */
	Fat_MountSDHC();
	
	progress+=0.05;
	Paint_Progress(progress);
	

	/* Initialize DIP module */
	ret = Disc_Init();
	progress+=0.05;
	Paint_Progress(progress);
	
	if (ret < 0) {
		printf("[+] ERROR:\n");
		printf("    Could not initialize DIP module! (ret = %d)\n", ret);

		return false;
	}

	return true;
}

bool Init_Game_List(void)
{

	Paint_Progress(progress);
	
	/* Try to open device */
	if (WBFS_Open() >= 0) {
		/* Get game list */
		
		progress+=0.05;
		Paint_Progress(progress);
		GetEntries();
		return true;
	}
	else
	{
		return false;
	}
}

void DrawSlider(void)
{
	int min_loc = 0;
	int max_loc = 313;
	
	GRRLIB_DrawImg(120, 410, slide_bar_texture, 0, 1, 1, 0xFFFFFFFF);
	
	int x = change_scale(shift, -1*(COVER_COUNT/2.0), COVER_COUNT/2.0, min_loc, max_loc);
	
	
	GRRLIB_DrawImg(126+x, 426, slide_texture, 0, 1, 1, 0xFFFFFFFF);
	
	
}
int DiscWait()
{
    u32 cover = 0;
	s32 ret = 0;

	while(!(cover & 0x2))
	{
		//TODO Add GUI For Cancel Button
		
		ret = WDVD_GetCoverStatus(&cover);
		if (ret < 0)
			return ret;
	}


	return ret;
}

int WindowPrompt(char* title, char* txt, int choice_a, int choice_b)
{
	/*TODO Create Graphical Prompt*/
	
	return choice_a;
}
/****************************************************************************
 * ShowProgress
 *
 * Updates the variables used by the progress window for drawing a progress
 * bar. Also resumes the progress window thread if it is suspended.
 ***************************************************************************/
void
ShowProgress (s32 done, s32 total)
{

    static time_t start;
	static u32 expected;

    f32 percent; //, size;
	u32 d, h, m, s;

	//first time
	if (!done) {
		start    = time(0);
		expected = 300;
	}

	//Elapsed time
	d = time(0) - start;

	if (done != total) {
		//Expected time
		if (d)
			expected = (expected * 3 + d * total / done) / 4;

		//Remaining time
		d = (expected > d) ? (expected - d) : 0;
	}

	//Calculate time values
	h =  d / 3600;
	m = (d / 60) % 60;
	s =  d % 60;

	//Calculate percentage/size
	percent = (done * 100.0) / total;
	//size    = (hdd->wbfs_sec_sz / GB_SIZE) * total;

    //progressTotal = total;
	//progressDone = done;

	//sprintf(prozent, "%0.2f%%", percent);
    //prTxt.SetText(prozent);
    //sprintf(timet,"Time left: %d:%02d:%02d",h,m,s);
    //timeTxt.SetText(timet);
	//progressbarImg.SetTile(100*progressDone/progressTotal);

	/*Update and Draw Progress Window Here*/
	
}


int ProgressWindow(char* title, char* msg)
{
	/*TODO Draw Window*/
	int ret = wbfs_add_disc(hdd, __WBFS_ReadDVD, NULL, ShowProgress, ONLY_GAME_PARTITION, 0);
	
	return ret;

}

bool Menu_Install(void)
{

    static struct discHdr headerdisc ATTRIBUTE_ALIGN(32);
	
	WDVD_SetWBFSMode(WBFS_DEVICE_USB, NULL);

    int ret, choice = 0;
	char *name;
	static char buffer[MAX_CHARACTERS + 4];

	ret = DiscWait();
	if (ret < 0) {
		WindowPrompt ("Error reading Disc",0,BACK_BUTTON,0);
		return false;
	}
	ret = Disc_Open();
	if (ret < 0) {
		WindowPrompt ("Could not open Disc",0,BACK_BUTTON,0);
		return false;
	}

	ret = Disc_IsWii();
	
	if (ret < 0) {
		choice = WindowPrompt ("Not a Wii Disc","Insert a Wii Disc!",OK_BUTTON,CANCEL_BUTTON);

		if (choice != 1) {
			return false;
		}
		else
		{
			return Menu_Install();
		}
	}
	
	Disc_ReadHeader(&headerdisc);
	name = headerdisc.title;
	if (strlen(name) < (22 + 3)) {
			memset(buffer, 0, sizeof(buffer));
			sprintf(name, "%s", name);
		} else {
			strncpy(buffer, name,  MAX_CHARACTERS);
			strncat(buffer, "...", 3);
			sprintf(name, "%s", buffer);
	}

	ret = WBFS_CheckGame(headerdisc.id);
	if (ret) {
		WindowPrompt ("Game is already installed:",name,BACK_BUTTON,0);
		return false;
	}
	hdd = GetHddInfo();
	if (!hdd) {
		WindowPrompt ("No HDD found!","Error!!",BACK_BUTTON,0);
		return false;
		}

	f32 freespace, used;

	WBFS_DiskSpace(&used, &freespace);
	u32 estimation = wbfs_estimate_disc(hdd, __WBFS_ReadDVD, NULL, ONLY_GAME_PARTITION);
	f32 gamesize = ((f32) estimation)/1073741824;
	char gametxt[50];
	
	sprintf(gametxt, "Installing game %.2fGB:", gamesize);
	
	if (gamesize > freespace) {
		char errortxt[50];
		sprintf(errortxt, "Game Size: %.2fGB, Free Space: %.2fGB", gamesize, freespace);
		choice = WindowPrompt("Not enough free space!",errortxt,CANCEL_BUTTON, 0);
		return false;
	}
	else {
		ret = ProgressWindow(gametxt, name);
		if (ret != 0) {
			WindowPrompt ("Install error!",0,BACK_BUTTON,0);
			return false;
		} else {
			GetEntries();
			WindowPrompt ("Successfully installed:",name,OK_BUTTON,0);
			return true;
		}
	}

	return false;
}

bool Menu_Boot(void)
{
	#ifndef TEST_MODE
	struct discHdr *header = NULL;
	int i = 0;
	s32 ret;

	/* No game list */
	if (!gameCnt)
		return false;

	/* Selected game */
	header = &gameList[gameSelected];

    GRRLIB_Exit();
	
	for(i = 0; i < array_size; i++)
	{
		free(covers[i].data);
	}
	
	free(cover_texture.data);
	free(back_texture.data);
	free(empty_texture.data);
	free(no_disc_texture.data);
	free(current_cover_texture.data);
	free(select_menu_texture.data);
	free(text_font1.data);

	//free(covers);
	
	/* Set WBFS mode */
	Disc_SetWBFS(WBFS_DEVICE_USB,header->id);
		
	/* Open disc */
	ret = Disc_Open();
	if (ret < 0) {
		return false;
	}

	ret = Disc_WiiBoot();
    if (ret < 0) {
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
    }

	#endif
	
	return true;
}


void quit()
{
	exit(0);
}

//---------------------------------------------------------------------------------
int main( int argc, char **argv ){
//---------------------------------------------------------------------------------
	#ifndef TEST_MODE
	//__Disc_SetLowMem();
	/* Load Custom IOS */
	int ret = IOS_ReloadIOS(249);
	/* Check if Custom IOS is loaded */
	if (ret < 0) {
		printf("[+] ERROR:\n");
		printf("    Custom IOS could not be loaded! (ret = %d)\n", ret);

		return 0;
	}
	#endif
		
	GRRLIB_Init();
    GRRLIB_FillScreen(0x000000FF);
    GRRLIB_Render();
    GRRLIB_FillScreen(0x000000FF);
    GRRLIB_Render();
	
    gradient_texture = GRRLIB_LoadTexture(gradient_bg_png);
    loader_main_texture = GRRLIB_LoadTexture(loading_main_png);
    progress_texture = GRRLIB_LoadTexture(progress_png);
	
	usb_error_texture = GRRLIB_LoadTexture(usb_error_png);
	
	pointer_texture = GRRLIB_LoadTexture(generic_point_png);
		
	Paint_Progress(progress);
	
	#ifndef TEST_MODE
	if(!init_usbfs())
		return 0;
	
	Paint_Progress(progress);
	
	my_wbfsDev = WBFS_DEVICE_USB;

  INIT_RETRY:
	/* Initialize WBFS */
	ret = WBFS_Init(my_wbfsDev);
	
	if(ret < 0)
	{
		while(1)
		{
			WPAD_ScanPads();
		
			GRRLIB_DrawImg(0, 0, usb_error_texture, 0, 1, 1, 0xFFFFFFFF);
			GRRLIB_Render();
				
			if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A)
			{
				goto INIT_RETRY;
			}
			
			if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B)
			{
				GRRLIB_Exit();
				SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
			}
		}
	}
	
	//bool flip = true;
  USB_RETRY:
	if(!Init_Game_List())
	{
		while(1)
		{
			WPAD_ScanPads();
		
			GRRLIB_DrawImg(0, 0, usb_error_texture, 0, 1, 1, 0xFFFFFFFF);
			GRRLIB_Render();
				
			if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A)
			{
				goto USB_RETRY;
			}
			
			if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B)
			{
				GRRLIB_Exit();
				SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
			}
		}
	}
	
	#else
	gameCnt = 29;
	Init_Covers();
	#endif
	
	Init_Buttons();
	
	progress += 0.5;
	Paint_Progress(progress);
	
	free(gradient_texture.data);
	free(loader_main_texture.data);
	free(progress_texture.data);
	free(usb_error_texture.data);
	

	selected = false;
	
	bool select_ready = false;
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	
	#ifdef TEST_MODE
	PAD_Init();
	#endif
	
	while(1) {

		WPAD_ScanPads();
		
		#ifdef TEST_MODE
		PAD_ScanPads();
		#endif
		
		ir_t ir; // The struct for infrared
		
		WPAD_IR(WPAD_CHAN_0, &ir); // Let's get our infrared data
		wd = WPAD_Data(WPAD_CHAN_0);

		p_x = ir.sx-200;
		p_y = ir.sy-250;
		p_ang = ir.angle/2; // Set angle/2 to translate correctly

		Hover_Buttons();

		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
		{
			quit();
		}
		
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B ||
			PAD_ButtonsDown(0) & PAD_BUTTON_B)
		{
			if(selected && animate_flip >= 1.0)
			{
				selected = false;
			}
		}
		
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A ||
			PAD_ButtonsDown(0) & PAD_BUTTON_A)
		{
			if(gameCnt)
			{
				if(!selected && animate_flip <= 0.0)
				{
					if(p_x < 360 && p_x > 200 &&
						p_y > 60 && p_y < 380)
					{
						if(select_ready && select_shift == 0.0)
						{
							selected = true;
							LoadCurrentCover(gameSelected);
						}
					}
					else if(p_x < 200 &&
						p_y > 60 && p_y < 380)
					{
						select_shift = (-4)*((200-p_x)/200.0);
					}
					else if(p_x > 360 &&
						p_y > 60 && p_y < 380)
					{
						select_shift = 5*(p_x-330.0-15)/280.0;
					}
				}
				
				if(selected && animate_flip == 1.0)
				{
					//TODO Prompt to boot game...
					if(!Menu_Boot())
					{
						selected = false;
						animate_flip = 0;
					}
					else
					{
						return 0;
					}
				}
			}
		}

		if(!selected && animate_flip == 0)
		{
			if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_LEFT  ||
				PAD_ButtonsDown(0) & PAD_BUTTON_LEFT)
			
			{	
				select_ready = false;
					
				if(!((int)shift-1 <= (-1)*(COVER_COUNT/2.0)))
					shift -= SCROLL_SPEED;
			}
			else if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_RIGHT ||
				PAD_ButtonsDown(0) & PAD_BUTTON_RIGHT)
			{
				select_ready = false;
					
				if(!((int)shift+.5 >= (COVER_COUNT/2.0)))
					shift += SCROLL_SPEED;
			}
			else
			{
				if(abs(select_shift) > SCROLL_SPEED)
				{
					int mult = abs((int)select_shift);
					if(select_shift > 0)
					{
					
						select_shift -= mult*SCROLL_SPEED;
						if(!((int)shift-1 <= (-1)*(COVER_COUNT/2.0)))
							shift -= mult*SCROLL_SPEED;
					}
					else
					{
						select_shift += mult*SCROLL_SPEED;
						if(!((int)shift+.5 >= (COVER_COUNT/2.0)))
							shift += mult*SCROLL_SPEED;
					}
					
				}
				else if(abs(((int)shift * 10000.0) - (shift*10000.0))/10000.0 > (SCROLL_SPEED+SCROLL_SPEED/2.0))
				{
					select_ready = false;
					if((int)((int)(shift+0.5) - (int)shift) == 0)
					{
						shift -= SCROLL_SPEED;
					}
					else
					{
						shift += SCROLL_SPEED;
					}
				}
				else
				{
					select_shift = 0;
					shift = (int)shift;
					select_ready = true;
					
					/*Draw Game Title*/
					draw_game_title(gameSelected);
				}
					
			}
		}
		draw_covers();


		if(selected || animate_flip != 0)
		{
			draw_selected();
		}
		else
		{
			DrawSlider();
		}
		
		Button_Paint(addButton);
		GRRLIB_DrawImg(p_x, p_y, pointer_texture, p_ang, 1, 1, 0xFFFFFFFF);
        GRRLIB_Render();

	}
	
	//Preview
	
    GRRLIB_Exit(); 
	
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
	
	return 0;
}
 
