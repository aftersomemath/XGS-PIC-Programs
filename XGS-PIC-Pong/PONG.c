//////////////////////////////////////////////////
//
// Levi Burner
//
// Last modified: xx.xx.xxxx
//
// Descripition: A simple Pong game
//
//////////////////////////////////////////////////

//Includes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include main system header
#include "XGS_PIC_SYSTEM_V010.h"

// Include the gamepad driver
#include "XGS_PIC_GAMEPAD_DRV_V010.h"

// Include our graphics definition file
#include "XGS_PIC_NTSC_160_192_2_V010.h"
#include "XGS_PIC_GFX_DRV_V010.h"

// Include our audio file
#include "XGS_PIC_SOUND_DRV_V010.h"

// Include tech font
#include "XGS_PIC_TECH_FONT_6x8_V010.h"

// Draws a single bitmap font character
void DrawBitmapFontChar(char ch, int x, int y);

// Plays final sounds of game
void PlayFinalSounds(void);

int  Random(void); // Generates a random number by counting 
//how long it takes the player to press start

void get_number_of_players(void); //gets number of players 1 - 2

//#define
#define paddle_size  14
#define forward 1
#define backward 0
#define up 1
#define down 0
#define on 1
#define off 0
//variables
unsigned char 	g_VRAMBuffer[VRAM_BUF_SIZE] __attribute__((far));	
unsigned short 	g_PaletteMap[PALETTE_MAP_SIZE];			// Table that contains which color table to assign 8x8 group
unsigned char	g_Palettes[MIN_PALETTE_SIZE * 5];			// Our Color Table

//game variables

int angle[14] = {
2,2,2,1,1,1,0,0,1,1,
1,2,2,2,
};

int directiony[14] = {
up,up,up,up,up,up,up,
down,down,down,down,down,down,down,
};

int Win_sound[8] = {
10,1046,1318,1568,1318,1568,1568,0
};

int Lose_sound[7] = {
20,391,329,293,261,261
};

unsigned char GamepadValue; 
int i;
int difference;
int sound;
int frequency;
int delay;
int Vsync_counter;
int Vsync_Counter2;
int speed;
int threshold;
int players;

struct score_counter {
int player;
int computer;
};

struct score_counter score;

struct ball_specs {
int directionX;
int directionY;
int X;
int Y;
int angle;
};

struct ball_specs ball;

struct paddle_position {
int Y;
int speed;
} ;
struct paddle_position paddle[1];

int main (void)
{
int i;

// Always call ConfigureClock first
	SYS_ConfigureClock(FCY_NTSC);

// Init the audio system
SND_Init();

// Init the graphics system
GFX_InitBitmap(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, g_VRAMBuffer, g_Palettes, g_PaletteMap);

// Init our gamepad  - we will use controller 1 only
Gamepad_Init();

GFX_SetClipRegion(0, SCREEN_HEIGHT-1, 0, SCREEN_WIDTH-1);

// Fill the entire palette with black and then we'll manually setup some colors
memset(g_Palettes, NTSC_BLACK, sizeof(g_Palettes));

// General game variable initiation

paddle[0].Y = 80;
paddle[1].Y = 80;
paddle[0].speed = 2;
paddle[1].speed = 1;

ball.X = 80;
ball.Y = 80;
ball.angle = 1;

ball.directionX = 1;
ball.directionY = 1;

score.player = 0;
score.computer = 0;
Vsync_Counter2 = 0;
speed = 1;
threshold = 600;
players = 1;

//difference = 0;

// color pallete values

g_Palettes[0] = NTSC_BLACK;
g_Palettes[1] = NTSC_RED;
g_Palettes[2] = NTSC_YELLOW;
g_Palettes[3] = NTSC_PURPLE;
/*
g_Palettes[4] = NTSC_PURPLE;
g_Palettes[5] = NTSC_ORANGE;
g_Palettes[6] = NTSC_GREEN;
g_Palettes[7] = NTSC_YELLOW;

g_Palettes[8] = NTSC_BLACK;
g_Palettes[9] = NTSC_RED;
g_Palettes[10] = NTSC_YELLOW;
g_Palettes[11] = NTSC_PURPLE;

g_Palettes[12] = NTSC_PURPLE;
g_Palettes[13] = NTSC_ORANGE;
g_Palettes[14] = NTSC_GREEN;
g_Palettes[15] = NTSC_YELLOW;

g_Palettes[16] = NTSC_BLACK;
g_Palettes[17] = NTSC_RED;
g_Palettes[18] = NTSC_RED;
g_Palettes[19] = NTSC_RED;
*/

/*for(i=0; i < (SCREEN_WIDTH/8); i++)
	{
		// Rows (8 lines) 6-8 use first palette
		g_PaletteMap[(SCREEN_WIDTH/8)*6 + i] = 0;
		g_PaletteMap[(SCREEN_WIDTH/8)*7 + i] = 0;
		g_PaletteMap[(SCREEN_WIDTH/8)*8 + i] = 0;

		// Rows (8 lines) 9-11 use next palette
		g_PaletteMap[(SCREEN_WIDTH/8)*9 + i] = 1;
		g_PaletteMap[(SCREEN_WIDTH/8)*10 + i] = 1;
		g_PaletteMap[(SCREEN_WIDTH/8)*11 + i] = 1;


		// Rows (8 lines) 12-14 use next palette
		g_PaletteMap[(SCREEN_WIDTH/8)*12 + i] = 2;
		g_PaletteMap[(SCREEN_WIDTH/8)*13 + i] = 2;
		g_PaletteMap[(SCREEN_WIDTH/8)*14 + i] = 2;


		// Rows (8 lines) 15-17 use next palette
		g_PaletteMap[(SCREEN_WIDTH/8)*15 + i] = 3;
		g_PaletteMap[(SCREEN_WIDTH/8)*16 + i] = 3;
		g_PaletteMap[(SCREEN_WIDTH/8)*17 + i] = 3;


		// Row (8 lines) 18 use next palette
		g_PaletteMap[(SCREEN_WIDTH/8)*18 + i] = 4;
	}
*/
// Start the NTSC timer so we start drawing to the screen
GFX_StartDrawing(SCREEN_TYPE_NTSC);

get_number_of_players();

while(1) {
WAIT_FOR_VSYNC_START();
Vsync_Counter2++;
if (Vsync_Counter2 == threshold){
speed ++;
paddle[0].speed ++;
paddle[1].speed ++;
threshold += 600;
}

GFX_FillScreen_2BPP(0, g_VRAMBuffer);

GFX_HLine_2BPP(0, 159, 8, 1, g_VRAMBuffer); //Draw gameboard top and bottom sides
GFX_HLine_2BPP(0, 159, 191, 1,g_VRAMBuffer); 



GamepadValue = Gamepad_Read(0); //interogate gamepad

if(GamepadValue & GAMEPAD_UP)       // Move player//
{
	paddle[0].Y = paddle[0].Y - paddle[0].speed;
}

if(GamepadValue & GAMEPAD_DOWN)
{
	paddle[0].Y = paddle[0].Y + paddle[0].speed;
}

if(paddle[0].Y < 10) {    //player bound control
paddle[0].Y = 10;
}

if(paddle[0].Y >= (191 - paddle_size)) {
paddle[0].Y = (190 - paddle_size);
}

if((ball.X <= 8) && (ball.X >= 7)) {                 // check for left paddle and adjust angle
	if(ball.Y - paddle[0].Y >= 0 && ball.Y - paddle[0].Y <= 13) {
		difference = ball.Y - paddle[0].Y;
        difference++;
		ball.angle = angle[difference];
        ball.directionY = directiony[difference];
        ball.directionX = forward;
        sound = 0;       
		
 
	}
}

if((ball.X >= 152) && (ball.X <= 153)) {                 // check for right paddle and adjust angle
	if(ball.Y - paddle[1].Y >= 0 && ball.Y - paddle[1].Y <= 13) {
		difference = ball.Y - paddle[1].Y;
		difference++;
		ball.angle = angle[difference];
        ball.directionY = directiony[difference];
        ball.directionX = backward;
        sound = 0;
 	    	
	}
}
if(players == 1) {
if(ball.directionX == forward) {        // Move computer paddle
	if(paddle[1].Y + 1 < ball.Y) {
		paddle[1].Y += paddle[1].speed;
	}

	if(paddle[1].Y + 1 > ball.Y) {
		paddle[1].Y -= paddle[1].speed;
	}
}
}

if(players == 2) {

	 if(GamepadValue & GAMEPAD_YELLOW) {
		paddle[1].Y += paddle[1].speed;
		}
	 if(GamepadValue & GAMEPAD_RED){
		paddle[1].Y -= paddle[1].speed;
		}
}

if(paddle[1].Y < 10) {    //player bound control
paddle[1].Y = 10;
}

if(paddle[1].Y >= (191 - paddle_size)) {
paddle[1].Y = (190 - paddle_size);
}

if(ball.directionX == forward) { // Move ball on x axis
ball.X += speed;
}

if(ball.directionX == backward) {
ball.X -= speed;
}

if(ball.Y <= 10) {              // Check for wall and adjust direction
ball.directionY = down;
}

if(ball.Y >= 189) {
ball.directionY = up;
}

if(ball.directionY == up) {     // Move ball on y axis
ball.Y -= ball.angle;
}

if(ball.directionY == down) {
ball.Y += ball.angle;
}


if (sound == 0) {
	SND_PlayTone(1400);
	
}
sound++;
if(sound == 15) {
   SND_PlayTone(0);
}


DrawBitmapFontChar(score.player + 48, 25,25);
DrawBitmapFontChar(score.computer + 48, 121, 25);
GFX_VLine_2BPP(6,  paddle[0].Y, paddle[0].Y + paddle_size, 1, g_VRAMBuffer ); //plot paddles
GFX_VLine_2BPP(154,paddle[1].Y, paddle[1].Y + paddle_size, 1, g_VRAMBuffer );

for(i = 0; i < 2; i++) {
GFX_Circle_2BPP(ball.X, ball.Y, i, 3, g_VRAMBuffer); // plot ball
}

if (ball.X >= 156) {
SND_PlayTone(500);
score.player++;
DrawBitmapFontChar(score.player + 48, 25,25);
DrawBitmapFontChar(score.computer + 48, 121, 25);
paddle[0].Y = 80;
paddle[1].Y = 80;
ball.X = 80;
Vsync_Counter2 = 0;
speed = 1;
paddle[0].speed = 2;
paddle[1].speed = 1;
if(players == 2)
paddle[1].speed++;

threshold = 600;

PlayFinalSounds();

Vsync_counter = 0;
while(Vsync_counter < 60) {
WAIT_FOR_VSYNC_START();
Vsync_counter++;
WAIT_FOR_VSYNC_END();
}

Vsync_counter = 0;
SND_PlayTone(0);

while(1) {
WAIT_FOR_VSYNC_START();
GamepadValue = Gamepad_Read(0);
ball.directionX ++;
ball.directionY ++;
ball.angle++;
ball.Y++;
if(ball.Y > 191) {
ball.Y = 10;
}
if(ball.angle > 2){
ball.angle = 0;
}
if (ball.directionX > 1) {
ball.directionX = 0;
}
if(ball.directionY > 1) {
ball.directionY = 0;
}
if(GamepadValue & GAMEPAD_START) {
break;
}
WAIT_FOR_VSYNC_END();
}
}

if (ball.X <= 3) {
SND_PlayTone(500);
score.computer++;
DrawBitmapFontChar(score.player + 48, 25,25);
DrawBitmapFontChar(score.computer + 48, 121, 25);
paddle[0].Y = 80;
paddle[1].Y = 80;
ball.X = 80;
Vsync_Counter2 = 0;
speed = 1;
paddle[0].speed = 2;
paddle[1].speed = 1;
if(players == 2)
paddle[1].speed++;

threshold = 600;

PlayFinalSounds();

Vsync_counter = 0;
while(Vsync_counter < 60) {
WAIT_FOR_VSYNC_START();
Vsync_counter++;
WAIT_FOR_VSYNC_END();
}
Vsync_counter = 0;
SND_PlayTone(0);

while(1) {
WAIT_FOR_VSYNC_START();
ball.directionX ++;
ball.directionY ++;
ball.angle++;
ball.Y++;
if(ball.Y > 191) {
ball.Y = 10;
}
if(ball.angle > 2){
ball.angle = 0;
}
if (ball.directionX > 1) {
ball.directionX = 0;
}
if(ball.directionY > 1) {
ball.directionY = 0;
}
GamepadValue = Gamepad_Read(0);
if(GamepadValue & GAMEPAD_START) {
break;
}

WAIT_FOR_VSYNC_END();
}

}


WAIT_FOR_VSYNC_END();
}
return 0;
}

void DrawBitmapFontChar(char ch, int x, int y)
{
		
	// now test if character is in range
	if (ch >= ' ' && ch <= '_') ch = ch - ' ' + 0;
	else 
	ch = 0;

	int FontIndex = ch * 48;
	int h, j;

	for(j=0; j < 8; j++)
	{
		for(h=0; h < 6; h++)
		{
			if(g_TileFontBitmap[FontIndex++] == 0)
			{
				// Blank out the region
				GFX_Plot_2BPP(x  ,y  ,0,g_VRAMBuffer);
				GFX_Plot_2BPP(x+1,y  ,0,g_VRAMBuffer);
				
				GFX_Plot_2BPP(x  ,y+1,0,g_VRAMBuffer);
				GFX_Plot_2BPP(x+1,y+1,0,g_VRAMBuffer);
			}
			else
			{
				// Draw pixel
				GFX_Plot_2BPP(x  ,y  ,2,g_VRAMBuffer);
				GFX_Plot_2BPP(x+1,y  ,2,g_VRAMBuffer);
				
				GFX_Plot_2BPP(x  ,y+1,2,g_VRAMBuffer);
				GFX_Plot_2BPP(x+1,y+1,2,g_VRAMBuffer);
			}

			// Increment X
			x += 3;
		}

		// Increment y
		y += 3;
		x -= 18;
	}

}

void PlayFinalSounds(void) {
	int y;
 	y = 0;
    	if (score.player == 5) {
		delay = Win_sound[0];
        y++;
        frequency = Win_sound[y];
		while(1) {
			WAIT_FOR_VSYNC_START();
			Vsync_counter++;
			if(Vsync_counter == delay) {
				y++;
				frequency = Win_sound[y];
				Vsync_counter = 0;
			}
            if(frequency == 0){
			break;
			}
			SND_PlayTone(frequency);
			WAIT_FOR_VSYNC_END();
		}
		SND_PlayTone(0);

		while(1) {
		WAIT_FOR_VSYNC_START();
ball.directionX ++;
ball.directionY ++;
ball.angle++;
ball.Y++;
if(ball.Y > 191) {
ball.Y = 10;
}
if(ball.angle > 2){
ball.angle = 0;
}
if (ball.directionX > 1) {
ball.directionX = 0;
}
if(ball.directionY > 1) {
ball.directionY = 0;
}
GamepadValue = Gamepad_Read(0);
if(GamepadValue & GAMEPAD_START) {
score.player = 0;
score.computer = 0;
break;
		}
		WAIT_FOR_VSYNC_END();
		}
		}		  
		if ((score.computer == 5) && (players != 2)) {
		delay = Lose_sound[0];
        y++;
        frequency = Lose_sound[y];
		while(1) {
			WAIT_FOR_VSYNC_START();
			Vsync_counter++;
			if(Vsync_counter == delay) {
				y++;
				frequency = Lose_sound[y];
				Vsync_counter = 0;
			}
            if(frequency == 0){
			break;
			}
			SND_PlayTone(frequency);
			WAIT_FOR_VSYNC_END();
		}
		SND_PlayTone(0);

		while(1) {
		WAIT_FOR_VSYNC_START();
ball.directionX ++;
ball.directionY ++;
ball.angle++;
ball.Y++;
if(ball.Y > 191) {
ball.Y = 10;
}
if(ball.angle > 2){
ball.angle = 0;
}
if (ball.directionX > 1) {
ball.directionX = 0;
}
if(ball.directionY > 1) {
ball.directionY = 0;
}
GamepadValue = Gamepad_Read(0);
if(GamepadValue & GAMEPAD_START) {
score.player = 0;
score.computer = 0;
break;
		}
		WAIT_FOR_VSYNC_END();
		}
	}
	if((players == 2) && (score.computer == 5)) { 
	delay = Win_sound[0];
        y++;
        frequency = Win_sound[y];
		while(1) {
			WAIT_FOR_VSYNC_START();
			Vsync_counter++;
			if(Vsync_counter == delay) {
				y++;
				frequency = Win_sound[y];
				Vsync_counter = 0;
			}
            if(frequency == 0){
			break;
			}
			SND_PlayTone(frequency);
			WAIT_FOR_VSYNC_END();
		}
		SND_PlayTone(0);

		while(1) {
		WAIT_FOR_VSYNC_START();
ball.directionX ++;
ball.directionY ++;
ball.angle++;
ball.Y++;
if(ball.Y > 191) {
ball.Y = 10;
}
if(ball.angle > 2){
ball.angle = 0;
}
if (ball.directionX > 1) {
ball.directionX = 0;
}
if(ball.directionY > 1) {
ball.directionY = 0;
}
GamepadValue = Gamepad_Read(0);
if(GamepadValue & GAMEPAD_START) {
score.player = 0;
score.computer = 0;
break;
		}
		WAIT_FOR_VSYNC_END();
		}
		}		  
}

void get_number_of_players(void) {


while(1) {
	WAIT_FOR_VSYNC_START();
     GamepadValue = Gamepad_Read(0);
		if(GamepadValue & GAMEPAD_UP) {
		players = 2;
        paddle[1].speed = 2;
		}
		if(GamepadValue & GAMEPAD_DOWN) {
		players = 1;
		paddle[1].speed = 1;
		}
        DrawBitmapFontChar(players + 48, 70,86);
        if(GamepadValue & GAMEPAD_START) 
		break;
		WAIT_FOR_VSYNC_END();
}

}



