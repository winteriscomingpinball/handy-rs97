/*

	Main menu items:
	LOAD ROM
	CONFIG
	LOAD STATE: 0
	SAVE STATE: 0
	RESET
	EXIT

	Config menu items;
	IMAGE SCALING: x2 / FULLSCREEN
	FRAMESKIP: 1 - 9
	SHOW FPS: YES / NO
	LIMIT FPS: YES / NO
	SWAP A/B: YES / NO


*/
#include <cassert>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <SDL/SDL.h>
#include <libgen.h>
#include <errno.h>



#include "gui.h"
#include "font.h"

#include "../handy_sdl_main.h"
#include "../handy_sdl_graphics.h"

/* defines and macros */
#define MAX__PATH 1024
#define FILE_LIST_ROWS 24
#define FILE_LIST_POSITION 8
#define DIR_LIST_POSITION 208

#define color16(red, green, blue) ((red << 11) | (green << 5) | blue)

//#define COLOR_BG            color16(05, 03, 02)

#define COLOR_BG            color16(0, 0, 0)

#define COLOR_ROM_INFO      color16(22, 36, 26)
#define COLOR_ACTIVE_ITEM   color16(255, 0, 0)
#define COLOR_INACTIVE_ITEM color16(255, 255, 255)
#define COLOR_FRAMESKIP_BAR color16(15, 31, 31)
#define COLOR_HELP_TEXT     color16(16, 40, 24)

#define ROM_COUNT_LIMIT 30
#define ROM_PER_PAGE_COUNT 5
#define ROM_DIR "./roms"


/* external references */
extern int Throttle; // show fps, from handy_sdl_main.cpp
extern char rom_name_with_no_ext[128]; // name if current rom, used for load/save state
extern char romname[512];


/* SDL declarations */
extern SDL_Surface *HandyBuffer; // Our Handy/SDL display buffer
extern SDL_Surface *mainSurface; // Our Handy/SDL primary display
SDL_Surface *menuSurface = NULL, *Game_Surface_Preview; // menu rendering
extern bool runRomBrowser;
extern void handy_sdl_core_init(char *romname);
extern void handy_sdl_core_reinit(char *romname);
extern int emulation;

int Invert = 0;

int allowExit=1;

char *foundRoms[ROM_COUNT_LIMIT];
short romCount=0;
short curRomNum=0;
short curRomPage=0;
short romPageCount=0;
bool romsChecked=0;

void gui_LoadState();
void gui_SaveState();
void gui_FileBrowserRun();
void gui_ConfigMenuRun();
void gui_Reset();
void gui_Init();
void gui_Flip();
void print_string(const char *s, u16 fg_color, u16 bg_color, int x, int y);
void get_config_path();

void gui_RunRomBrowser();


void setRom();


int gui_LoadSlot = 0;
int gui_ImageScaling = 0;
int gui_Frameskip = 0;
int gui_FPS = 0;
int gui_Show_FPS = 0;
int gui_LimitFPS = 0;
int gui_SwapAB = 0;

int loadslot = -1; // flag to reload preview screen
int done = 0; // flag to indicate exit status

char config_full_path[MAX__PATH];

typedef struct {
	char *itemName;
	int *itemPar;
	int itemParMaxValue;
	const char **itemParName;
	void (*itemOnA)();
} MENUITEM;

typedef struct {
	int itemNum; // number of items
	int itemCur; // current item
	MENUITEM *m; // array of items
} MENU;

#ifdef RS90
const char* gui_ScaleNames[] = {"Original", "Fullscreen"};
#else
const char* gui_ScaleNames[] = {"Keep Aspect", "Fullscreen"};
#endif
const char* gui_YesNo[] = {"no", "yes"};

MENUITEM gui_MainMenuItems[] = {
	/* It's unusable on the RS-90, disable */
	{(char *)"ROM browser", 0, 0, 0, &gui_RunRomBrowser},
	{(char *)"Config", 0, 0, 0, &gui_ConfigMenuRun},
	{(char *)"Load state: ", &gui_LoadSlot, 9, 0, &gui_LoadState},
	{(char *)"Save state: ", &gui_LoadSlot, 9, 0, &gui_SaveState},
	{(char *)"Reset", 0, 0, 0, &gui_Reset},
	{(char *)"Exit", 0, 0, 0, &handy_sdl_quit} // extern in handy_sdl_main.cpp
};


MENUITEM gui_RomBrowserItems[] = {
	{(char *)"Exit", 0, 0, 0, &handy_sdl_quit}, // extern in handy_sdl_main.cpp
	{(char *)".", 0, 0, 0, &setRom}, // extern in handy_sdl_main.cpp
	{(char *)".", 0, 0, 0, &setRom}, // extern in handy_sdl_main.cpp
	{(char *)".", 0, 0, 0, &setRom}, // extern in handy_sdl_main.cpp
	{(char *)".", 0, 0, 0, &setRom}, // extern in handy_sdl_main.cpp
	{(char *)".", 0, 0, 0, &setRom} // extern in handy_sdl_main.cpp

};

MENU gui_MainMenu = { 6, 0, (MENUITEM *)&gui_MainMenuItems };

MENU gui_RomBrowser = { ROM_PER_PAGE_COUNT+1, 0, (MENUITEM *)&gui_RomBrowserItems };

MENUITEM gui_ConfigMenuItems[] = {
#ifndef IPU_SCALE
	{(char *)"Upscale  : ", &gui_ImageScaling, 1, (const char **)&gui_ScaleNames, NULL},
#endif
	{(char *)"Swap A/B : ", &gui_SwapAB, 1, (const char **)&gui_YesNo, NULL}
};

MENU gui_ConfigMenu = { 2
	#ifdef IPU_SCALE
	-1
	#endif
, 0, (MENUITEM *)&gui_ConfigMenuItems };



void gui_MainMenuRun(MENU *menu);

int cmpfunc (const void * a, const void * b ) {
    const char *pa = *(const char**)a;
    const char *pb = *(const char**)b;

    return strcmp(pa,pb);
}



void setupRomPage(int pageNum){
	char i;
	
	puts("Applying ROM names to list");
   for (i=0;i<ROM_PER_PAGE_COUNT;i++){
    if(foundRoms[(curRomPage * ROM_PER_PAGE_COUNT) + i]){
		gui_RomBrowserItems[i+1].itemName = foundRoms[(curRomPage * ROM_PER_PAGE_COUNT) + i];
		
	}else{
		break;
	}
   }
   
   puts("Finished that...");
   gui_RomBrowser.itemNum=i+1;
}


void findRoms(){
	char dir[20]="";
	 char savdir[20]="";
	 char i;
	 
	 int remainder=0;
	 
	  struct dirent *files;
   
   romCount=-1;
   romPageCount=0;
   
   
   //if(!romsChecked){
	   DIR *dirX = opendir(ROM_DIR);
	   if (dirX == NULL){
		  printf("ROM Directory cannot be opened!" );
	   }
	   
	   puts("Found these ROMS...");
	   
	   while ((files = readdir(dirX)) != NULL){
				   //printf("%s\n", files->d_name);
				   if(files->d_name[0] != '.' &&   (strstr(files->d_name, ".lnx") != NULL || strstr(files->d_name, ".zip") != NULL )){
					   printf("%s\n", files->d_name);
					   //directories[counter]=(wchar_t)files->d_name;
					   if(romCount<ROM_COUNT_LIMIT)romCount++;
					   foundRoms[romCount]=files->d_name;
					   
					   
				   }
			   }
			   
	   printf("ROM Count: %d\n",romCount);
	   
	   romPageCount=(int)(romCount/ROM_PER_PAGE_COUNT);
	   remainder = romCount - (romPageCount*ROM_PER_PAGE_COUNT);
	   if (!remainder) romPageCount--;
	   
	   printf("ROM Page Count: %d\n",romPageCount);
	   
	   closedir(dirX);
	   
	   //puts("Sorting found ROMs by name");
	   
	   //qsort(foundRoms,romCount,sizeof(char),cmpfunc);
   //}
   
   setupRomPage(0);
   
   romsChecked=1;
}


void setRom(){
	
	
	printf("Setting ROM name to: %s\n",foundRoms[(curRomPage * ROM_PER_PAGE_COUNT) + gui_RomBrowser.itemCur - 1]);
	snprintf(romname, sizeof(romname), "%s/%s", ROM_DIR,foundRoms[(curRomPage * ROM_PER_PAGE_COUNT) + gui_RomBrowser.itemCur - 1]);
	
	
	loadslot = -1;
	runRomBrowser=0;
	//emulation=1;
	//allowExit=1;
	
	gui_MainMenu.itemCur = 0;
	gui_RomBrowser.itemCur = 0;
	//gui_MainMenuRun(&gui_MainMenu);
	handy_sdl_core_reinit(romname);
	
}


/*
	Clears mainSurface
*/
void gui_ClearScreen()
{
	uint8_t i;
	for(i=0;i<3;i++)
	{
		SDL_FillRect(mainSurface,NULL,SDL_MapRGB(mainSurface->format, 0, 0, 0));
		SDL_Flip(mainSurface);
	}
}

/*
	Prints char on a given surface
*/
void ShowChar(SDL_Surface *s, int x, int y, unsigned char a, int fg_color, int bg_color)
{
	Uint16 *dst;
	int w, h;

	if(SDL_MUSTLOCK(s)) SDL_LockSurface(s);
	for(h = 8; h; h--) {
		dst = (Uint16 *)s->pixels + (y+8-h)*s->w + x;
		for(w = 8; w; w--) {
			Uint16 color = bg_color; // background
			if((gui_font[a*8 + (8-h)] >> w) & 1) color = fg_color; // test bits 876543210
			*dst++ = color;
		}
	}
	if(SDL_MUSTLOCK(s)) SDL_UnlockSurface(s);
}

void ShowString(int x, int y, const char *s)
{
	int i, j = strlen(s);
	for(i = 0; i < j; i++, x += 8) ShowChar(menuSurface, x, y, s[i], 0xFFFF, 0);
}

void ShowStringEx(int x, int y, const char *s)
{
	int i, j = strlen(s);
	for(i = 0; i < j; i++, x += 8) ShowChar(mainSurface, x, y, s[i], 0xFFFF, 0);
}

void ShowMenuItem(int x, int y, MENUITEM *m, int fg_color)
{
	static char i_str[24];

	// if no parameters, show simple menu item
	if(m->itemPar == NULL) print_string(m->itemName, fg_color, COLOR_BG, x, y);
	else {
		if(m->itemParName == NULL) {
			// if parameter is a digit
			snprintf(i_str, sizeof(i_str), "%s%i", m->itemName, *m->itemPar);
		} else {
			// if parameter is a name in array
			snprintf(i_str, sizeof(i_str),  "%s%s", m->itemName, *(m->itemParName + *m->itemPar));
		}
		print_string(i_str, fg_color, COLOR_BG, x, y);
	}
}

void gui_LoadState()
{
	char savename[512];

	snprintf(savename, sizeof(savename), "%s/%s.%i.sav", config_full_path, rom_name_with_no_ext, gui_LoadSlot);
	//printf("Load state: %s\n", savename);

	// check if file exists otherwise mpLynx->ContextLoad will crash
	FILE *fp = fopen(savename, "rb");
	if(!fp) return;
	fclose(fp);

	if(!mpLynx->ContextLoad(savename)) printf("Error loading state: %s\n", savename);
		else { mpLynx->SetButtonData(0); done = TRUE; }
}

void gui_SaveState()
{
	char savename[512];

	snprintf(savename, sizeof(savename), "%s/%s.%i.bmp", config_full_path, rom_name_with_no_ext, gui_LoadSlot);
				
	SDL_SaveBMP(HandyBuffer, savename);

	snprintf(savename, sizeof(savename), "%s/%s.%i.sav", config_full_path, rom_name_with_no_ext, gui_LoadSlot);
	//printf("Save state: %s\n", savename);

	if(!mpLynx->ContextSave(savename)) printf("Error saving state: %s\n", savename); //else done = TRUE;

	loadslot = -1; // show preview immediately
}

/*
	copy-pasted mostly from gpsp emulator by Exophaze
	thanks for it
*/
void print_string(const char *s, u16 fg_color, u16 bg_color, int x, int y)
{
	int i, j = strlen(s);
	for(i = 0; i < j; i++, x += 8) ShowChar(menuSurface, x, y, s[i], fg_color, bg_color);
}

/*
	Shows previews of load/save and pause
*/
void ShowPreview(MENU *menu)
{
	char prename[256];
	/*static uint8_t prebuffer[160 * 160 * 2];*/
	/*uint32_t i;*/
	SDL_Rect dst, dst2;
	SDL_Surface *tmp;

	if(menu == &gui_MainMenu && (menu->itemCur == 2 || menu->itemCur == 3)) 
	{
		if(loadslot != gui_LoadSlot) 
		{
			// create preview name
			snprintf(prename, sizeof(prename), "%s/%s.%i.bmp", config_full_path, rom_name_with_no_ext, gui_LoadSlot);
			
			if (Game_Surface_Preview)
			{
				SDL_FreeSurface(Game_Surface_Preview);
				Game_Surface_Preview = NULL;
			}
			
			// check if file exists
			tmp = SDL_LoadBMP(prename);
			if (tmp)
			{
				Game_Surface_Preview = SDL_DisplayFormat(tmp);
				SDL_FreeSurface(tmp);
				tmp = NULL;
			}
			loadslot = gui_LoadSlot; // do not load img file each time
		}
		
		
		if (Game_Surface_Preview) 
		{
			//#ifdef RS90
			dst.x = 72;
			dst.y = 32;
			dst.w = 96;
			dst.h = 64;
			dst2.x = 0;
			dst2.y = 0;
			dst2.w = LynxWidth;
			dst2.h = LynxHeight;
			// #else
			// dst.x = 80;
			// dst.y = 24;
			// dst.w = 160;
			// dst.h = 102;
			// dst2.x = 0;
			// dst2.y = 0;
			// dst2.w = LynxWidth;
			// dst2.h = LynxHeight;
			// #endif
			SDL_SoftStretch(Game_Surface_Preview, &dst2, menuSurface, &dst);
		}
		/*for(int y = 0; y < 102; y++) memcpy((char *)menuSurface->pixels + (24 + y) * 320*2 + 80*2, prebuffer + y * 320, 320);*/
	}
	else
	{
		if (HandyBuffer) 
		{
			//#ifdef RS90
			dst.x = 72;
			dst.y = 32;
			dst.w = 96;
			dst.h = 64;
			dst2.x = 0;
			dst2.y = 0;
			dst2.w = LynxWidth;
			dst2.h = LynxHeight;
			// #else
			// dst.x = 80;
			// dst.y = 24;
			// dst.w = 160;
			// dst.h = 102;
			// dst2.x = 0;
			// dst2.y = 0;
			// dst2.w = LynxWidth;
			// dst2.h = LynxHeight;
			// #endif
			SDL_SoftStretch(HandyBuffer, &dst2, menuSurface, &dst);
		}
	}
	
}

/*
	Shows menu items and pointing arrow
*/
void ShowMenu(MENU *menu)
{
	int i;
	MENUITEM *mi = menu->m;
	char buf[64];
	
	
	*(unsigned long *)buf = 0;
	*(unsigned long *)&buf[4] = 0;
	*(unsigned long *)&buf[8] = 0;
	*(unsigned long *)&buf[12] = 0;
	*(unsigned long *)&buf[16] = 0;
	sprintf(buf, "Page %d of %d", curRomPage+1, romPageCount+1);
          
	

	// clear buffer
	assert(menuSurface);
	SDL_FillRect(menuSurface, NULL, COLOR_BG);

	// show menu lines
	for(i = 0; i < menu->itemNum; i++, mi++) {
		int fg_color;
		if(menu->itemCur == i) fg_color = COLOR_ACTIVE_ITEM; else fg_color = COLOR_INACTIVE_ITEM;
	//#ifdef RS90
	if(runRomBrowser){
		ShowMenuItem(10, 40 + (i * 8), mi, fg_color);
		
	}
	else{
		ShowMenuItem(36, 112 + (i * 8), mi, fg_color);
		// show preview screen
		ShowPreview(menu);
	}
	}


	// print info string
	//#ifdef RS90
	
	
	
	print_string("Handy: Lynx Emulator", color16(0, 255, 255), COLOR_BG, 5, 2);
	if(!runRomBrowser){
	//print_string("Port by gameblabla", COLOR_HELP_TEXT, COLOR_BG, 48, 88);
	print_string("[1] = Return to game", COLOR_HELP_TEXT, COLOR_BG, 4, 11);
	print_string("[START] = Choose Item", COLOR_HELP_TEXT, COLOR_BG, 4, 19);
	
	}else{
		print_string("[START] = Choose ROM", COLOR_HELP_TEXT, COLOR_BG, 4, 15);
		
		
		print_string("[LEFT/RIGHT] = Change page", COLOR_HELP_TEXT, COLOR_BG, 4, 23);
		
		print_string(buf, color16(0, 40, 255), COLOR_BG, 4, 33);
		
	}
	
	
	
	

	//#else
	//print_string("Press B to return to the game", COLOR_HELP_TEXT, COLOR_BG, 56, 220);
	//print_string("Handy libretro " __DATE__ " build", COLOR_HELP_TEXT, COLOR_BG, 40, 2);
	//print_string("Port by gameblabla", COLOR_HELP_TEXT, COLOR_BG, 80, 12);
	//#endif
}

/*
	Main function that runs all the stuff
*/
void gui_MainMenuRun(MENU *menu)
{
	SDL_Event gui_event;
	MENUITEM *mi;
	
          

	done = FALSE;

	while(!done) 
	{
		mi = menu->m + menu->itemCur; // pointer to highlit menu option

		while(SDL_PollEvent(&gui_event)) 
		{
			if(gui_event.type == SDL_KEYDOWN) {
				// DINGOO A - apply parameter or enter submenu
				//if(gui_event.key.keysym.sym == SDLK_RETURN) if(mi->itemOnA != NULL) (*mi->itemOnA)();
				if(gui_event.key.keysym.sym == SDLK_F1)
				{
					if(mi->itemOnA != NULL) (*mi->itemOnA)();
					
					if(!allowExit) return;
				}
				// DINGOO B - exit or back to previous menu
				//if(gui_event.key.keysym.sym == SDLK_ESCAPE) return;
				if(gui_event.key.keysym.sym == SDLK_RSHIFT && allowExit) return;
				// DINGOO UP - arrow down
				if(gui_event.key.keysym.sym == SDLK_UP) if(--menu->itemCur < 0) menu->itemCur = menu->itemNum - 1;
				// DINGOO DOWN - arrow up
				if(gui_event.key.keysym.sym == SDLK_DOWN) if(++menu->itemCur == menu->itemNum) menu->itemCur = 0;
				// DINGOO LEFT - decrease parameter value
				if(gui_event.key.keysym.sym == SDLK_LEFT ) {
					if(mi->itemPar != NULL && *mi->itemPar > 0) *mi->itemPar -= 1;
					if(runRomBrowser && curRomPage>0){
						curRomPage--;
						setupRomPage(curRomPage);
						menu->itemCur=1;
					}
				}
				// DINGOO RIGHT - increase parameter value
				if(gui_event.key.keysym.sym == SDLK_RIGHT) {
					if(mi->itemPar != NULL && *mi->itemPar < mi->itemParMaxValue) *mi->itemPar += 1;
					if(runRomBrowser && curRomPage<romPageCount){
						curRomPage++;
						setupRomPage(curRomPage);
						menu->itemCur=1;
					}
				}
			}
		}
		if(!done) ShowMenu(menu); // show menu items
		SDL_Delay(16);
		gui_Flip();
	}
	SDL_FillRect(mainSurface, NULL, 0);
	SDL_Flip(mainSurface);
	SDL_FillRect(mainSurface, NULL, 0);
	SDL_Flip(mainSurface);
	#ifdef SDL_TRIPLEBUF
	SDL_FillRect(mainSurface, NULL, 0);
	SDL_Flip(mainSurface);
	#endif
}

void get_config_path()
{
	//snprintf(config_full_path, sizeof(config_full_path), "%s/.handy", getenv("HOME"));
	snprintf(config_full_path, sizeof(config_full_path), "./save");
	mkdir(config_full_path, 0755);
}

void gui_Init()
{
	get_config_path();
}

void gui_Run()
{
	gui_ClearScreen();
	if(runRomBrowser){
		allowExit=0;
		findRoms();
		gui_MainMenuRun(&gui_RomBrowser);
		//runRomBrowser=0;
	}
	else{
		allowExit=1;
		gui_MainMenuRun(&gui_MainMenu);
		
	}
	//gui_RomBrowser
	//gui_MainMenuRun(&gui_RomBrowser);
	gui_ClearScreen();
}

void gui_RunRomBrowser(){
	runRomBrowser=1;
	allowExit=0;
	gui_Run();
}


void gui_ConfigMenuRun()
{
	gui_MainMenuRun(&gui_ConfigMenu);
}

void gui_Reset()
{
	mpLynx->Reset();
	done = TRUE; // mark to exit
}

void gui_Flip()
{
	if (mainSurface->w == 320 || mainSurface->w == 240)
	{
		SDL_BlitSurface(menuSurface, NULL, mainSurface, NULL);
	}
	else
	{
		SDL_SoftStretch(menuSurface, NULL, mainSurface, NULL);
	}
	SDL_Flip(mainSurface);
}
