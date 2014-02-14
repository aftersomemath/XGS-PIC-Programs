///////////////////////////////////////////////
// Levi Burner
// Start date 12/5/10
// I'm just trying to make a raycasting demo for my XGS Pic
// Warning I've have almost no 3D experience
////////////////////////////////////////////////////

#include "XGS_PIC_NTSC_160_192_2_V010.h"

//make an externed video buffer so that my raycaster engine
// can get it

extern unsigned char g_VRAMBuffer[VRAM_BUF_SIZE] __attribute__((far));

