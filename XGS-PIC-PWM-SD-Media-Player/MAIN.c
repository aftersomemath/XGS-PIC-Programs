///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright Corey Shuman 2010
//
// Filename: MAIN.c
//
// Original Author: Corey Shuman
//
// Modified by: Levi Burner (Warning I changed just about everything in this file)
//
// Last Modified: 7-12-11
//
// Description: This file is a test script for my prototype PWM sound driver. It includes
//				five sounds mapped to the direction buttons and action buttons.
//
//              Levi: After I worked on this it became a test script for XGS Media player that showcases that PWM engine
//              that has  been extensively modified for my purposes. XGS Medai player has some glitches and they are
//              pretty much unfixable because I just through the whole thing together to show stuff off and has almost no
//              comments. So if anyone ever tries to clean this up do so at the risk of your sanity!
// 
// Controls:
//				D-PAD UP:		Move selector up
//				D-PAD DOWN:		Move selector down
//				D-PAD LEFT:		nothing
//				D-PAD RIGHT:	Fast Forward
//				YELLOW:         Rewind (for some reason the left button wouldn't work
//				RED:		nothing
//				START:			Play/Pause
//				SELECT:			Stop playback
//
// Required Files: XGS_PIC_SYSTEM_V010.c/h
//				   XGS_PIC_PWM_SOUND.c/h
//				   XGS_PIC_GAMEPAD_DRV_V010.c/h
//				   
//				   
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include main system header
#include "XGS_PIC_SYSTEM_V010.h"
#include "p24HJ256GP206.h"

// Include the sound driver
#include "XGS_PIC_PWM_SOUND.h"

// Include controller driver
#include "XGS_PIC_GAMEPAD_DRV_V010.h"

// Include graphics drivers
#include "XGS_PIC_GFX_DRV_V010.h"
#include "XGS_PIC_NTSC_TILE1_V010.h"

// Include SD card driver
#include "FSIO.h"

// Include our font bitmaps so that I can print debug information on the screen
#include "XGS_PIC_C64_FONT_8x8_V010.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sound Files:


//////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////

#define MAX_PALLETE_SIZE 256
#define MAX_FILENAMES 15


//////////////////////////////////////////////////////
//Globals
/////////////////////////////////////////////////////

unsigned char g_Palettes[MAX_PALLETE_SIZE];
Sprite g_Sprites[MAX_SPRITES];

unsigned char g_TileMap[SCREEN_TILE_MAP_WIDTH*SCREEN_TILE_MAP_HEIGHT] __attribute__((far)) = {};

//sprites
unsigned char g_SpriteData[] __attribute__((far)) = {};

int red = 0;
int yellow = 0;
int left = 0;
int right = 0;
int up = 0;
int down = 0;
int start = 0;
int select = 0;
//int samplespeed = 2500;
char song_length[4];

int old_selection;

int playing = 0;
int paused = 0;

int frames;
int seconds;

int skip_timer;

char file[12] = {"MONSTER.WAV"};

// Array to hold list of file names found
char g_FileNames[MAX_FILENAMES][30];
int g_FileLengths[MAX_FILENAMES];
int g_FilesInList = 0;

int file_selector = 0;
int current_song_name = 0;

void GenerateFileList();

void DisplayFileList();

void show_selection(int selection);

void setup_screen(void);

void display_state(void);

void start_playing_song(void);

// Our main start location
int main (void)
{
	int i;
	// Always call ConfigureClock first
	SYS_ConfigureClock(FCY_NTSC);
	
	// Init Sd Card
	if(FSInit() == 0){
	while(1);
	}

	// Init our gamepad  - we will use controller 1 only
	Gamepad_Init();
	unsigned char GamepadVal;
	GFX_InitTile(g_TileFontBitmap,g_TileMap,g_Palettes,g_Sprites);

  for (i = 0; i < MAX_PALLETE_SIZE; i++)
           g_Palettes[i] = i;
           
    	// Set palette 0 to blue for text background, white for fon
	g_Palettes[0] = NTSC_BLUE;
	g_Palettes[1] = NTSC_WHITE;       
           
	// Initiate our PWM sound driver
	PWM_Init();
        PWM_Play("HERO3.WAV");
	
	GFX_TMap_Size(SCREEN_TILE_MAP_WIDTH, SCREEN_TILE_MAP_HEIGHT);
	
	GFX_TMap_CLS(' ');
	
	GFX_StartDrawing(SCREEN_TYPE_NTSC);

    //GenerateFileList();

	//setup_screen();

	while(1)
	{
		//DELAY_MS(5);	// synthesis "in-game" calculation time
		// Get the controller input on every frame
		WAIT_FOR_VSYNC_START();
		GamepadVal = Gamepad_Read(GAMEPAD_0);

		PWM_Load_Buffer();

		/*if(GamepadVal & GAMEPAD_UP)
		{
			if(!up)
			{
				file_selector--;
				if(file_selector < 0){
					file_selector = 0;
				}
				show_selection(file_selector);
				up = 1;
			}
		}
		else
			up = 0;

		if(GamepadVal & GAMEPAD_DOWN)
		{
			if(!down)
			{
				file_selector++;
				if(file_selector > g_FilesInList){
					file_selector = g_FilesInList;
				}
				show_selection(file_selector);
				down = 1;
			}
		}
		else
			down = 0;
*/
		if(GamepadVal & GAMEPAD_START)
		{
			if(!start){
				if(playing == 0){
					start_playing_song();
				}
				else if(playing == 1){
					PWM_Pause();
					if(paused == 0){
					paused = 1;
					}
					else if (paused == 1){
						paused = 0;
					}
				}
			start = 1;
			}
		}
		else
			start = 0;

		/*if(GamepadVal & GAMEPAD_SELECT)
		{
			// Decrease sample speed
			PWM_Stop();
			playing = 0;
			
		}
		else
			select = 0;
			
		if(GamepadVal & GAMEPAD_RED)
		{
			if(!red){
				red = 1;
			}
		}
		else
			red = 0;
			display_state();
			
		if(playing){	
			if(GamepadVal & GAMEPAD_RIGHT)
			{
				PWM_Skip_Bytes(18016);//one second
				seconds++;
			}
			
			if(GamepadVal & GAMEPAD_YELLOW)
				{
				PWM_Skip_Bytes(-18016);//one second
				if(seconds != 0)
				seconds--;
				}	
		
		
		}*/
		
		//	display_state();
	WAIT_FOR_VSYNC_END();
	}
	

}

void display_state(){
	int sound_length;
	char time[2];
	
	GFX_TMap_SetCursor(0,21);
	if(playing){
		GFX_TMap_SetCursor(0,22);
		
		sprintf(time,"%d",seconds / 60);
		GFX_TMap_Print_String(time);
		GFX_TMap_Print_Char(' ');
		sound_length = seconds % 60;
		if(sound_length < 10){
			GFX_TMap_Print_Char('0');
		}
		sprintf(time,"%d",sound_length);
		GFX_TMap_Print_String(time);
		
		
		sound_length = PWM_Find_Sound_Length();
		
		if(sound_length == seconds){
			playing = 0;
		}
		
		sprintf(time,"%d",sound_length / 60);
		GFX_TMap_SetCursor(16,22);
		GFX_TMap_Print_String(time);
		GFX_TMap_Print_Char(' ');
		sound_length = sound_length % 60;
		if(sound_length < 10){
			GFX_TMap_Print_Char('0');
		}
		sprintf(time,"%d",sound_length);
		GFX_TMap_Print_String(time);
		GFX_TMap_SetCursor(0,21);
		if(paused){
		GFX_TMap_Print_String("paused ");
		}
		else{
				frames++;
			if(frames == 120){ //this is strange it should be 60 but for some reason its screwed up
				frames = 0;
			  seconds++;
			}
			GFX_TMap_Print_String("playing ");
			GFX_TMap_Print_String(g_FileNames[current_song_name]);
		}
	}
	else{
		GFX_TMap_Print_String("stopped                                 ");
	}
} 

// Generates a list of files in the root directory on the card
void GenerateFileList()
{
	SearchRec file;
	int i;

	// Reset vars
	g_FilesInList = 0;

	// All file types except for directories
	unsigned char Attributes = ATTR_HIDDEN | ATTR_SYSTEM | ATTR_READ_ONLY | ATTR_VOLUME| ATTR_ARCHIVE;

	if(FindFirst("*.*", Attributes, &file) == 0)// 0 is success
	{
		g_FilesInList = 1;
		strcpy(g_FileNames[0], file.filename);
		g_FileLengths[0] = file.filesize;

		for(i=1; i < MAX_FILENAMES; i++)
		{
			if(FindNext(&file) == 0)
			{
				strcpy(g_FileNames[i], file.filename);
				g_FileLengths[i] = file.filesize;
			}
			else
				break;

			g_FilesInList++;
		}
	}
}

// Displays the files that are in the file name array onto the screen
void DisplayFileList()
{
	int i;
	char String[10];

	// Clear out any previous list of file names and size
	for(i = 0; i < MAX_FILENAMES; i++)
	{
		// Set the cursor position
		GFX_TMap_SetCursor(0,i+4);

		// Clear the line
		GFX_TMap_Print_String("                            ");
	}
	
	for(i = 0; i < g_FilesInList; i++)
	{
		// Set the cursor position
		GFX_TMap_SetCursor(1,i+4);

		// Print the filename
		GFX_TMap_Print_String(g_FileNames[i]);

		// Get the file length into a string
		sprintf(String, "(%d)", g_FileLengths[i]);

		int StrLength = strlen(String);
		GFX_TMap_SetCursor(27-StrLength,i+4);

		// Print the file size
		//GFX_TMap_Print_String(String);
	}
}


	

void show_selection(int selection){

	
	if((selection >= 0) && (selection < g_FilesInList)){
	GFX_TMap_SetCursor(0,old_selection + 4);
	GFX_TMap_Print_Char(' ');
	GFX_TMap_SetCursor(0,selection + 4);
	GFX_TMap_Print_Char('*');
	
	}
	
	old_selection = selection;
}

void setup_screen(void){
		GFX_TMap_Print_String("Welcome to XGS MediaPlayer\n");
		DisplayFileList();
		show_selection(0);
}

void start_playing_song(void){
	PWM_Play(g_FileNames[file_selector]);
	current_song_name = file_selector;
	playing = 1;
	seconds = 0;
}
	

