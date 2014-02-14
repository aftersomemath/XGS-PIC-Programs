///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright Nurve Networks LLC 2008
//
// Filename: XGS_PIC_NTSC_160_192_2_V010.H
//
// Original Author: Joshua Hintze
//
// Last Modified: 9.7.08
//
// Description: Header file for XGS_PIC_NTSC_160_192_2_V010.s assembly graphics driver. The purpose of this file
//				is to define the different variations between this engine and the next. This makes it easier
//				for the person who is writting the main code and makes it less prone to error.
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// watch for multiple inclusions
#ifndef XGS_PIC_NTSC_160_192_2
#define XGS_PIC_NTSC_160_192_2

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MACROS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// simply interogate the video driver's g_CurrentLine variable and determine where the scanline is
#define VIDEO_ACTIVE(video_line)          ( ((video_line) >= START_ACTIVE_SCAN && (video_line) < END_ACTIVE_SCAN))
#define VIDEO_INACTIVE(video_line)        ( ((video_line) < START_ACTIVE_SCAN || (video_line) >= END_ACTIVE_SCAN))
#define VIDEO_TOP_OVERSCAN(video_line)    ( ((video_line) >= 0 && (video_line) < START_ACTIVE_SCAN))
#define VIDEO_BOT_OVERSCAN(video_line)    ( ((video_line) >= END_ACTIVE_SCAN))

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SCREEN_WIDTH		160
#define SCREEN_HEIGHT		192
#define SCREEN_BPP			2

#define VERTICAL_OFFSET		40
#define LINE_REPT_COUNT		1

#define START_VIDEO_SCAN    0
#define START_ACTIVE_SCAN   (VERTICAL_OFFSET)
#define END_ACTIVE_SCAN     (SCREEN_HEIGHT*LINE_REPT_COUNT + VERTICAL_OFFSET)
#define END_VIDEO_SCAN      261


#define NUM_PIXELS_PER_BYTE	4

// Used for allocating buffers in main program
#define VRAM_BUF_SIZE		(SCREEN_WIDTH/NUM_PIXELS_PER_BYTE)*SCREEN_HEIGHT
#define PALETTE_MAP_SIZE	(SCREEN_WIDTH/8)*(SCREEN_HEIGHT/8)
#define MIN_PALETTE_SIZE	4

// Common NTSC colors
#define COLOR_LUMA		(0x91)

#define NTSC_GREEN  	(COLOR_LUMA + 0)
#define NTSC_YELLOW   	(COLOR_LUMA + 2)
#define NTSC_ORANGE   	(COLOR_LUMA + 4)
#define NTSC_RED      	(COLOR_LUMA + 5)
#define NTSC_VIOLET   	(COLOR_LUMA + 9)
#define NTSC_PURPLE   	(COLOR_LUMA + 11)
#define NTSC_BLUE     	(COLOR_LUMA + 13)

#define NTSC_BLACK   	(0x4F)

#define NTSC_GRAY0    	(NTSC_BLACK + 0x10)
#define NTSC_GRAY1    	(NTSC_BLACK + 0x20)
#define NTSC_GRAY2    	(NTSC_BLACK + 0x30)
#define NTSC_GRAY3    	(NTSC_BLACK + 0x40)
#define NTSC_GRAY4    	(NTSC_BLACK + 0x50)
#define NTSC_GRAY5    	(NTSC_BLACK + 0x60)
#define NTSC_GRAY6    	(NTSC_BLACK + 0x70)
#define NTSC_GRAY7    	(NTSC_BLACK + 0x80)
#define NTSC_GRAY8    	(NTSC_BLACK + 0x90)
#define NTSC_GRAY9    	(NTSC_BLACK + 0xA0)
#define NTSC_WHITE    	(NTSC_BLACK + 0xB0)


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GLOBALS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EXTERNALS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTOTYPES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// end multiple inclusions
#endif
