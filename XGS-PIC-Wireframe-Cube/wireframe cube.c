//////////////////////////////////////////
//
//Levi Burner
//last revision 11/29/10
//
//Just seeing if I can get a little cube going
//I've never programmed any 3D before. It's going to be interesting
//////////////////////////////////////////

//Includes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//defines
#define viewing_distance 125

// Include main system header
#include "XGS_PIC_SYSTEM_V010.h"

// Include the gamepad driver
#include "XGS_PIC_GAMEPAD_DRV_V010.h"

// Include our graphics definition file
#include "XGS_PIC_NTSC_160_192_2_V010.h"
#include "XGS_PIC_GFX_DRV_V010.h"

// Include our audio file
#include "XGS_PIC_SOUND_DRV_V010.h"

#include "FSIO.h"

void draw_3d_line(int x,int y,int z, int x2, int y2, int z2, int color);
void render_cube(void);
void rotate_cube(int x_deg, int y_deg, int z_deg);
void generate_lookup(void);
void load_cube(void);
//variables
unsigned char 	g_VRAMBuffer[VRAM_BUF_SIZE] __attribute__((far));	
unsigned short 	g_PaletteMap[PALETTE_MAP_SIZE];			// Table that contains which color table to assign 8x8 group
unsigned char	g_Palettes[MIN_PALETTE_SIZE];			// Our Color Table

int world_pos_x, world_pos_y;//world position
int world_pos_z;
float points[8][3] = { //xyz number of points
{-50, 50,-50},
{ 50, 50,-50},
{ 50, 50,50},
{-50, 50,50},
{-50,-50,-50},
{ 50,-50,-50},
{ 50,-50, 50},
{-50,-50, 50},
};

int lines[12][3]={ // which point for line number of lines
{0,1,2}, //top front
{1,2,2},
{2,3,2},
{3,0,2},

{4,5,2},
{5,6,2},
{6,7,2},
{7,4,2},

{0,4,1},
{1,5,2},
{2,6,2},
{3,7,2},
};

//struct  object cube;

float sint[360];
float cost[360];

int main (void)
{
int gamepad_value;
// Always call ConfigureClock first


	SYS_ConfigureClock(FCY_NTSC);
	
	
	//init SD card
	
	if(FSInit() == FALSE){
		while(1){}
	}
	
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
	g_Palettes[2] = NTSC_YELLOW;
	g_Palettes[3] = NTSC_GRAY7;

	// Lets fill the screen with index 0 (NTSC_BLACK)
	GFX_FillScreen_2BPP(0,g_VRAMBuffer);
	// Start the NTSC timer so we start drawing to the screen
render_cube();
generate_lookup();
	GFX_StartDrawing(SCREEN_TYPE_NTSC);

world_pos_x = 0;
world_pos_y = 0;
//world_pos_z = 150;

load_cube();

while(1) {
WAIT_FOR_VSYNC_START();

gamepad_value = Gamepad_Read(0);

if(gamepad_value & GAMEPAD_RED)
world_pos_z ++;

if(gamepad_value & GAMEPAD_YELLOW)
world_pos_z --;

if(gamepad_value & GAMEPAD_UP)
world_pos_y++;

if(gamepad_value & GAMEPAD_DOWN)
world_pos_y--;

if(gamepad_value & GAMEPAD_LEFT)
world_pos_x--;

if(gamepad_value & GAMEPAD_RIGHT)
world_pos_x++;

GFX_FillScreen_2BPP(0,g_VRAMBuffer);
rotate_cube(1,0,1);
render_cube();	
WAIT_FOR_VSYNC_END();

}

return 0;
}

void draw_3d_line(int x, int y, int z, int x2, int y2, int z2, int color){

x = (viewing_distance * x) / z;
y = (viewing_distance * y) / z;

x2 = (viewing_distance * x2) / z2;
y2 = (viewing_distance * y2) / z2;

x = x + (SCREEN_WIDTH / 2);   //convert from cartesian to screen coordinates
y = -y + (SCREEN_WIDTH / 2);

x2 = x2 + (SCREEN_WIDTH / 2);
y2 = -y2 + (SCREEN_WIDTH / 2);

GFX_Line_Clip_2BPP(x, y * 1.3, x2, y2 * 1.3, color, g_VRAMBuffer);

}

void render_cube(void) {
int i,h;
int x1,x2;
int y1,y2;
int z1,z2;
int p[2];

/*draw_3d_line(points[0][0],points[0][1],points[0][2],points[1][0],points[1][1],points[1][2]);
draw_3d_line(points[1][0],points[1][1],points[1][2],points[2][0],points[2][1],points[2][2]);
draw_3d_line(points[2][0],points[2][1],points[2][2],points[3][0],points[3][1],points[3][2]);
draw_3d_line(points[3][0],points[3][1],points[3][2],points[0][0],points[0][1],points[0][2]);
*/

for(i = 0; i < 12; i++){
	for(h = 0; h < 2; h++){
		p[h] = lines[i][h];
	}
	x1 = points[p[0]][0] + world_pos_x;
	y1 = points[p[0]][1] + world_pos_y;
	z1 = points[p[0]][2] + world_pos_z;
	
	x2 = points[p[1]][0] + world_pos_x;
	y2 = points[p[1]][1] + world_pos_y;
	z2 = points[p[1]][2] + world_pos_z;
	
	draw_3d_line(x1,y1,z1,x2,y2,z2,lines[i][2]);
}
}	

void rotate_cube(int x_deg,int y_deg, int z_deg) {
	float x_temp, y_temp, z_temp; //rotate cube on x-z axis hope this works later I should make sin and cosine lookup tables
	int i;
	
	
	if(y_deg != 0){
	for(i = 0; i < 8; i++) { // y axis
	x_temp = points[i][0] * cost[y_deg] - points[i][2] * sint[y_deg];
	z_temp = points[i][0] * sint[y_deg] + points[i][2] * cost[y_deg];
	
	points[i][0] = x_temp;
	points[i][2] = z_temp;
	}
	}
	
	if(x_deg != 0) {
	for(i = 0; i < 8; i++){ //x axis
		y_temp = points[i][1] * cost[x_deg] - points[i][2] * sint[x_deg];
		z_temp = points[i][1] * sint[x_deg] + points[i][2] * cost[x_deg];
		
		points[i][1] = y_temp;
		points[i][2] = z_temp;
	}
	}
	
	if(z_deg != 0) {
	for(i = 0; i < 8; i++){ //x axis
		x_temp = points[i][0] * cost[z_deg] - points[i][1] * sint[z_deg];
		y_temp = points[i][0] * sint[z_deg] + points[i][1] * cost[z_deg];
		
		points[i][0] = x_temp;
		points[i][1] = y_temp;
	}
	}
}	

void generate_lookup(void){
	int i;
	/*for (i = 0; i < 360; i += 1){
		sint[i] = sin((2 * 3.1459) / i);
	}
	
	for (i = 0; i < 360; i += 1){
		cost[i] = cos((2 * 3.1459) / i);
	}*/
/*	for(i = 0; i < 360;	i++){
		sint[i] = sin((i / 180) * 3.1415);
	}
	for(i = 0; i < 360; i++){
		cost[i] = cos((i / 180) * 3.1415);
	}*/
	
	for(i = 0; i < 360;	i++){
		sint[i] = sin((float)i * .017);
	}
			
	for(i = 0; i < 360; i++){
		cost[i] = cos((float)i * .017);
	}
}

void load_cube(void){
	FSFILE * file_pointer;
	char ReadBuffer[4];
	int i;
		
	file_pointer = FSfopen("CUBE.TXT", "r");
	if(file_pointer != NULL){ //get z
		FSfseek(file_pointer, 5,0);
		FSfread(ReadBuffer, 1, 4, file_pointer);
	}
	
	world_pos_z += (ReadBuffer[0] - 48) * 1000;
	world_pos_z += (ReadBuffer[1] - 48) * 100;
	world_pos_z += (ReadBuffer[2] - 48) * 10;
	world_pos_z += (ReadBuffer[3] - 48);
	
	world_pos_z -= 1000;
	
	//get y
	
	FSfseek(file_pointer,2,1);
	FSfread(ReadBuffer, 1, 4, file_pointer);
	
	world_pos_y += (ReadBuffer[0] - 48) * 1000;
	world_pos_y += (ReadBuffer[1] - 48) * 100;
	world_pos_y += (ReadBuffer[2] - 48) * 10;	
	world_pos_y += (ReadBuffer[3] - 48);
	
	world_pos_y -= 1000;
	
	FSfseek(file_pointer,2,1);
	FSfread(ReadBuffer, 1, 4, file_pointer);
	
	world_pos_x += (ReadBuffer[0] - 48) * 1000;
	world_pos_x += (ReadBuffer[1] - 48) * 100;
	world_pos_x += (ReadBuffer[2] - 48) * 10;
	world_pos_x += (ReadBuffer[3] - 48);
	
	world_pos_x -= 1000;
		
	FSfclose(file_pointer);
	
}	
