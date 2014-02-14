///////////////////////////////////////////////
// Levi Burner
// Start date 12/5/10
// I'm just trying to make a raycasting demo for my XGS Pic
// Warning I've have almost no 3D experience
//
//  1/24/2011
//  Ok it works at very low framerate I'm done for now though
//  I borrowed heavily from the raycaster at this address http://lodev.org/cgtutor/raycasting.html
//  very good tutorial still not sure how it all works though.
//   
//  6/6/11
//  My interest in this has revived and I have now got my trigonomic lookup tables working
//  I also removed the two square root functions that were being used and replaced it with trig
//  I'm going to start cleaning the code up. After that comes fixed point arithmetic. Getting 40fps now.
//
//  6/23/11
//  Again I have lost interest and I want to begin work on a real polygon engine for the XGS. So Levi if you ever
//  come back here is the state of the current engine and the steps you can take to improve it.
//
//  Bug 1: There is slight fishbowl effect. The code that I believe causes this is lines 58,60 of raycasting_engine.c.
//  These lines use fixed point math and I think there is too much inaccuracy in the division.
//  
//  Bug 2: There is also a glitch where the world suddenly changes. It happens when you move and turn at the same time
//  not sure why this is happening.
//
//  Accomplishment 1: Fixed point arithmetic has been accomplished
//
//  Accomplishment 2:fps is up to 60
//
//  Todo 1: It would be nice if the engine had more horizontal resolution. I'm only casting 40 rays right now
//  and the resolution is 160 X 192 so I could cast up to 160 rays. But that would require doubling the resolution of the
//  sine and cosine lookup tables because they only support -360 - 360 degrees in one degree increments and the since my
//  FOV is 80 degrees wide 80 / 160 is .5. thus I would need to support .5 degree increments.
//
//  Bug 2: One more gremlin. I believe this is due to replacing the vector math in Lode's raycasting tutorial with trigonometry.
//  (I made the change for speed.) If a ray is cast at a 0,90,280, or 270 degree angle then something goes really weird.
//  I suspect it has to do with how much  the ray is incremented in order to move it to the next box. So it may have to do
//  with lines 58,60 of raycasting_engine.c. These two lines replaced lines 57 and 59 which used square root functions.
//  Thus they were replaced with trigonometry. Unfortunately I suspect this has introduced both  of the current bugs.
//
//  Note 1: There are some weird bit shifting techniques being used when multiplying and dividing fixed point numbers. I
//  did this to preserve as much accuracy as possible. Might want to clean that up sometime though.
//  
//  Wow I did alot in the past month.
////////////////////////////////////////////////////

//Includes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <libq.h> /* include fixed point library */

// Include main system header
#include "XGS_PIC_SYSTEM_V010.h"

// Include the gamepad driver
#include "XGS_PIC_GAMEPAD_DRV_V010.h"

// Include our graphics definition file
#include "XGS_PIC_NTSC_160_192_2_V010.h"
#include "XGS_PIC_GFX_DRV_V010.h"

// Include my new raycasting engine
#include "raycaster_engine.h"

unsigned char   g_VRAMBuffer[VRAM_BUF_SIZE] __attribute__((far));	
unsigned short 	g_PaletteMap[PALETTE_MAP_SIZE];			// Table that contains which color table to assign 8x8 group
unsigned char	g_Palettes[MIN_PALETTE_SIZE];			// Our Color Table

//these are my raycasting variables
_Q16 posX = 655360,posY = 655360;
int camera_angle = 0;
_Q16 dirX = 0, dirY = 0;
_Q16 planeX = 0, planeY = .8391;
//int   dirDeg = 0;

_Q16  sint[90] __attribute__((far));
_Q16  cost[90] __attribute__((far));

int ray_index[number_of_columns * 2];
char  ray_colors[number_of_columns * 2];
int   ray_index_selector;
/*
unsigned char worldMap[10][10] __attribute__((far)) = {
{1,1,1,1,1,1,1,1,1,1},
{1,0,0,0,0,0,0,0,0,1},
{1,0,2,0,0,0,0,3,0,1},
{1,0,0,0,0,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,1},
{1,0,1,0,0,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,1},
{1,1,1,1,1,1,1,1,1,1},
};*/

unsigned char worldMap[mapWidth][mapHeight]=
{
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,3,0,3,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,3,0,0,0,0,1,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,3,0,3,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,3,0,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

int main (void)
{	
int gamepad_value;
	
// Always call ConfigureClock first
	SYS_ConfigureClock(FCY_NTSC);
	
	//Init the Gamepad
	Gamepad_Init();
	// Init the graphics system
	GFX_InitBitmap(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, g_VRAMBuffer, g_Palettes, g_PaletteMap);

	// Fill the palette map with 0, this way all 8x8 pixel sections use the first color table
	memset(g_PaletteMap, 0, sizeof(g_PaletteMap));

	// Fill the entire palette with black and then we'll manually setup some colors
	memset(g_Palettes, NTSC_BLACK, sizeof(g_Palettes));
	g_Palettes[0] = NTSC_BLACK;
	g_Palettes[1] = NTSC_BLUE;
	g_Palettes[2] = NTSC_RED;
	g_Palettes[3] = NTSC_GREEN;

	// Lets fill the screen with index 0 (NTSC_BLACK)
	GFX_FillScreen_2BPP(0,g_VRAMBuffer);
	
	//start drawing screen
	GFX_StartDrawing(SCREEN_TYPE_NTSC);
	
	//raycaster init
	build_trig_tables();
	rotate_player(0);
	
	while(1) {
	WAIT_FOR_VSYNC_START(); 	
		render_view();
	WAIT_FOR_VSYNC_END();		
	gamepad_value = Gamepad_Read(0);
	
	if(gamepad_value & GAMEPAD_YELLOW){
	rotate_player(-1);
	}
	if(gamepad_value & GAMEPAD_RED){
		rotate_player(1);
	}
			

	if(gamepad_value & GAMEPAD_UP){
		//posX += dirX/6;
        //posY += dirY/6;

        if(worldMap[(posX + dirX / 3) >> 16][posY >> 16] == 0) posX += dirX / 3;        
        if(worldMap[posX >> 16][(posY + dirY / 3) >> 16] == 0) posY += dirY / 3;

	}
	if(gamepad_value & GAMEPAD_DOWN){
		//posX -= dirX/3;
        //posY -= dirY/3;
		if(worldMap[(posX - dirX / 3) >> 16][posY >> 16] == 0) posX -= dirX / 3;        
        if(worldMap[posX >> 16][(posY - dirY / 3) >> 16] == 0) posY -= dirY / 3;
	}
	
	find_rays();
	}
return 0;
}
	

