//////////////////////////////////////////
//Levi Burner
//start date 12.5.10
//This is my header file for
//a raycasting engine API for the XGS PIC 16 bit
//////////////////////////////////////////

//make an externed video buffer so that my raycaster engine
// can get it
#include "XGS_PIC_NTSC_160_192_2_V010.h"

#define mapWidth 24
#define mapHeight 24

#define number_of_columns 40
#define lines_in_column   160 / number_of_columns

extern unsigned char g_VRAMBuffer[VRAM_BUF_SIZE] __attribute__((far));

extern unsigned char worldMap[mapWidth][mapHeight] __attribute__((far));

extern _Q16 posX;
extern _Q16 posY;
extern _Q16 dirX;
extern _Q16 dirY;

extern _Q16   planeX, planeY;
extern int    camera_angle;
extern int ray_index[number_of_columns * 2];
extern char  ray_colors[number_of_columns * 2];

extern _Q16 sint[90]__attribute__((far));
extern _Q16 cost[90]__attribute__((far));
extern int   ray_index_selector;

void render_view(void);
void find_rays(void);
void rotate_player(int deg);
void build_trig_tables(void);
_Q16 my_sine(int deg);
_Q16 my_cosine(int deg);
void draw_Vline(int x, int y_top, int y_bottom, int color);
