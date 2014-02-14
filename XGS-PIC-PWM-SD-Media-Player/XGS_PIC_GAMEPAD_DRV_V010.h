///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright Nurve Networks LLC 2008
//
// Filename: XGS_PIC_GAMEPAD_DRV_V010.H
//
// Original Author: Joshua Hintze
//
// Last Modified: 8.31.08
//
// Description: Header file for XGS_PIC_GAMEPAD_DRV_V010.c
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
#ifndef XGS_PIC_GAMEPAD_DVR
#define XGS_PIC_GAMEPAD_DVR

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MACROS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// I/O pin map for game controllers
//
// GAMEPAD_DATA0 	  | RD0 | Pin 46 | Input
// GAMEPAD_DATA1 	  | RD1 | Pin 49 | Input
// GAMEPAD_DATA_LATCH | RD2 | Pin 50 | Output
// GAMEPAD_CLK        | RD3 | Pin 51 | Output
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// gamepad I/O declarations
#define GAMEPAD_PORT			PORTDbits
#define GAMEPAD_TRIS			TRISDbits

#define GAMEPAD_DATA0       	GAMEPAD_PORT.RD0
#define GAMEPAD_DATA0_TRIS     	GAMEPAD_TRIS.TRISD0

#define GAMEPAD_DATA1       	GAMEPAD_PORT.RD1
#define GAMEPAD_DATA1_TRIS     	GAMEPAD_TRIS.TRISD1

#define GAMEPAD_DATA_LATCH 		GAMEPAD_PORT.RD2
#define GAMEPAD_DATA_LATCH_TRIS	GAMEPAD_TRIS.TRISD2

#define GAMEPAD_CLK        		GAMEPAD_PORT.RD3
#define GAMEPAD_CLK_TRIS   		GAMEPAD_TRIS.TRISD3


// joystick bit mask defines
#define GAMEPAD_RIGHT       0x01
#define GAMEPAD_LEFT        0x02
#define GAMEPAD_DOWN        0x04
#define GAMEPAD_UP          0x08

#define GAMEPAD_START       0x10
#define GAMEPAD_SELECT      0x20

#define GAMEPAD_ACTION1     0x40
#define GAMEPAD_RED        	0x40

#define GAMEPAD_ACTION2     0x80
#define GAMEPAD_YELLOW      0x80

// Gamepad selection
#define GAMEPAD_0			0
#define GAMEPAD_1			1

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GLOBALS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EXTERNALS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTOTYPES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Gamepad_Init();
unsigned char Gamepad_Read(int GamepadNum);


// end multiple inclusions
#endif