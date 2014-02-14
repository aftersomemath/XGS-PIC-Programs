///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright Nurve Networks LLC 2008
//
// Filename: XGS_PIC_SOUND_DRV_V010.c
//
// Original Author: Joshua Hintze
//
// Last Modified: 8.31.08
//
// Description: Sound library file
//
//
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// First and foremost, include the chip definition header
#include "p24HJ256GP206.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// include local XGS API header files
#include "XGS_PIC_SYSTEM_V010.h"
#include "XGS_PIC_SOUND_DRV_V010.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MACROS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GLOBALS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int g_Prescale_256_Min = 0;
unsigned int g_Prescale_64_Min = 0;
unsigned int g_Prescale_8_Min = 0;
unsigned int g_Prescale_1_Min = 0;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EXTERNALS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTOTYPES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// I/O pin map for game controllers
//
// SOUND_OUT          | RD7 | Pin 55 | Output
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SND_Init()
{
	// Make sure the timer 2 is disabled before we start messing around
	T2CONbits.TON = 0; 				// Disable Timer

	// Audio output is on OC8/RD7 pin
	// Initialize Output Compare Module
	OC8CONbits.OCM = 0b000; 		// Disable Output Compare Module
	OC8R = 251; 					// Write the duty cycle for the first PWM pulse
	OC8RS = 251; 					// Write the duty cycle for the second PWM pulse
	OC8CONbits.OCTSEL = 0; 			// Select Timer 2 as output compare time base

	// Initialize and enable Timer2
	T2CONbits.TCS = 0; 				// Select internal instruction cycle clock
	T2CONbits.TGATE = 0; 			// Disable Gated Timer mode


	// Now lets pre-compute our ranges for the different pre-scalar. We will use this as zones,
	// for determining which prescalar best suits a tone to play. For example, larger prescalars (256:1) will
	// allow for more accuracy at lower frequncies, smaller prscalars (1:1) will allow us to have slightly better
	// accuracy at the higher frequencies.
	g_Prescale_256_Min = (g_FCY >> 8)/65535;		// g_FCY/(256*65535);		// Not really needed since we bounds limit to 4 Hz
	g_Prescale_64_Min = (g_FCY >> 6)/65535;		// g_FCY/(64*65535);
	g_Prescale_8_Min = (g_FCY >> 3)/65535;		// g_FCY/(8*65535);
	g_Prescale_1_Min = (g_FCY)/65535;				// g_FCY/(1*65535);

	// Now set the sound pin to an output for the TogglePin function. When the output compare module is running,
	// it will take over control of the sound pin anyways so this is really only needed for the TogglePin()
	SND_OUT = 0;
	SND_OUT_TRIS = PIN_DIR_OUTPUT;

}



// Toggles a pin at an approximate frequency for a duration Time. Make sure no tone is currently playing by calling
// SND_PlayTone(0) before hand.
// Time is specified in milliseconds, Freq is upper bounded to 15KHz
void SND_TogglePin(unsigned int Freq, int Time)
{
	long counter;
//	float scale = (   ((float)100000-   ((float)Freq*(float)Freq/(float)5000)     )    /(float)100000);

	if(Freq > 15000) Freq = 15000;

	float scale = 0.99621f + (-1.46972e-05f*Freq);		// Calculated using a linear least squares regression.
	long cycles = (long)(scale * (float)Freq*((float)Time/(float)1000));

	// generate the sound for the proper time
	for (counter = 0; counter < cycles; counter++)
	{
		SND_OUT = 1;		// Pin high
		DELAY_CLOCKS( (g_FCY/(2*Freq)) );
		SND_OUT = 0;		// Pin low
		DELAY_CLOCKS( (g_FCY/(2*Freq)) );
	}
}


// Call SND_Init before calling this function for the first time.
// 4 Hz -> 40KHz, 0 shuts off tone
void SND_PlayTone(unsigned int Freq)
{
	unsigned int NewPeriod;

	// Special case 0
	if(Freq == 0)
	{
		T2CONbits.TON = 0; 				// Disable Timer
		OC8CONbits.OCM = 0b000; 		// Disable Output Compare Module
		return;							// Done
	}

	// Now we should have our
	if(OC8CONbits.OCM != 0b110)
		OC8CONbits.OCM = 0b110;

	if(T2CONbits.TON == 0)
		T2CONbits.TON = 1;

	// Bounds check
	if(Freq < 4) Freq = 4;
	if(Freq > 40000) Freq = 40000;

	// To get the most accurate ranges we will determine what prescalar is best suited for the required
	// frequencies
	if(Freq < g_Prescale_64_Min)
	{
		// Use the 256:1 prescalar
		T2CONbits.TCKPS = 0b11; 		// Select 256:1 Prescaler
		NewPeriod = (g_FCY >> 8)/Freq;	// Calculate the new period
	}
	else if(Freq < g_Prescale_8_Min)
	{
		// Use the 64:1 prescalar
		T2CONbits.TCKPS = 0b10; 		// Select 64:1 Prescaler
		NewPeriod = (g_FCY >> 6)/Freq;	// Calculate the new period
	}
	else if(Freq < g_Prescale_1_Min)
	{
		// Use the 8:1 prescalar
		T2CONbits.TCKPS = 0b01; 		// Select 8:1 Prescaler
		NewPeriod = (g_FCY >> 3)/Freq;	// Calculate the new period
	}
	else
	{
		// Use the 1:1 prescalar
		T2CONbits.TCKPS = 0b00; 		// Select 1:1 Prescaler
		NewPeriod = (g_FCY)/Freq;			// Calculate the new period
	}

	// Update the new period
	PR2 = NewPeriod;				// Load the period value

	// Update the duty cycle count to keep us at 50% duty cycle
	OC8RS = NewPeriod >> 1;
}

