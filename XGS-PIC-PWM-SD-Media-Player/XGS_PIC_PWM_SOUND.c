///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright Corey Shuman 2010
//
// Filename: XGS_PIC_PWM_SOUND.c
//
// Original Author: Corey Shuman
//
// Changed by: Levi  Burner (I changed almost everything here
//
// Last Modified: 7-12-11
//
// Description: PWM sound library file
//
//
// Alright this thing works pretty good with a video driver. It's not perfect but it works
// In the future I can rewrite the interrupt in assembly and inject it into the video driver that
// should get rid of all the artifacts.
//
// 7/7/11
// Just thought I'd step in today and add some features like ff/rw and a progress bar
//
// 7/12/11
// I think I have finally finished this. I did not end up rewriting the interrupt but that is ok. Basically this driver
// now has a lot of useful  functions. I showed them off by writing XGS Media player in the main(); control loop.
// I don't think the engine has any glitches but it could use some optimization at some point. Now I know XGS Media
// Player has problems and don't ask me why. I just through that thing together
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// First and foremost, include the chip definition header
#include "p24HJ256GP206.h"
#include "XGS_PIC_GFX_DRV_V010.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// include local XGS API header files
#include "XGS_PIC_SYSTEM_V010.h"
#include "XGS_PIC_PWM_SOUND.h"
#include "XGS_PIC_GFX_DRV_V010.h" //Include GFX so I can print debug information

// include SD card driver 
#include "FSIO.h" //need this so I can load Wav files off of the SD card

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MACROS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PWM_FREQ 15720 //actuall NTSC is 15724 khz but this is what the driver does so this is what it gets
// The PWM system is running at 255682 hz
// Clock period = 168 (42954540/256000)
// Target speed is 256 kHz, rounded to closes period

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GLOBALS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned long PHASE_ACC = 0;     //this is the where I am in the file
int buffer_index;      //this is the index into the buffer it could be an integer
//unsigned int  PHASE_INC = 0;   //This variable is no longer used
//unsigned long PHASE_COUNT = 0;   //neither is this one
unsigned long SAMPLE_COUNT = 0;   //This holds the length of the file

unsigned char snd_buffer_1[sound_buf_size] __attribute__((far)); //Sound buffer 1
unsigned char snd_buffer_2[sound_buf_size] __attribute__((far)); //Sound buffer 2
int buffer_select;    //Tells the interrupt which buffer to use
int buffer_1_state;   //Tells the load function when the interrupt has used buffer 1
int buffer_2_state;   //Tells the load function when the interrupt has used buffer 2
FSFILE * soundfile;   //This is the soundfile
int stop = 0;

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
// I/O pin map for sound port
//
// SOUND_OUT          | RD7 | Pin 55 | Output | Output Compare 8
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PWM_Init()
{
	// Make sure that timer 2 is disabled
	T2CONbits.TON = 0; 				// Disable Timer
	T3CONbits.TON = 0;

	// Audio output is on OC8/RD7 pin
	// Initialize Output Compare Module
	OC8CONbits.OCM = 0b000; 		// Disable Output Compare Module
	OC8R = 0; 					    // Write the duty cycle for the first PWM pulse
	OC8RS = 0;  					// Write the duty cycle for the second PWM pulse
	OC8CONbits.OCTSEL = 1; 			// Select Timer 3 as output compare time base
									//Note: Timer 3 runs the output compare module
									//and timer two controls the PWM interrupt

	// Initialize and enable Timer2
	T2CONbits.TCS = 0; 				// Select internal instruction cycle clock
	T2CONbits.TGATE = 0; 			// Disable Gated Timer mode


    // Initialize and enable Timer3
	T3CONbits.TCS = 0; 				// Select internal instruction cycle clock
	T3CONbits.TGATE = 0; 			// Disable Gated Timer mode

	OC8CONbits.OCM = 0b110; 		//pwm mode no fault
	T2CONbits.TCKPS = 0b00;			//1:1 prescaller
	T3CONbits.TCKPS = 0b00;
	TMR2 = 0x00; 					// Clear timer register
	TMR3 = 0x00;

	PR2 = 2732;//42954540 / PWM_FREQ;		// Load the period value interrupt runs at 15720 hz
									// PWM frequency locked at 256kHz
	PR3 = 256;							

	IPC1bits.T2IP = 0x01; 			// Set Timer 2 Interrupt Priority Level
	IFS0bits.T2IF = 0; 				// Clear Timer 2 Interrupt Flag
	IEC0bits.T2IE = 1; 				// Enable Timer 2 interrupt
	T2CONbits.TON = 1; 					//timer go!
	T3CONbits.TON = 1;
}

void PWM_Play(char *file_name)
{
	// Files on SD card
	// 8000 hz HERO.WAV 
	// 11025 hz POWERUP.WAV SELECT.WAV SKILL0.WAV TEST.WAV HERO6.WAV
	// 16000 hz HERO3.WAV TONE.WAV MONSTER.WAV
	// SILENCE.WAV
	FSfclose(soundfile);         //Close soundfile if one is already open
	soundfile = FSfopen(file_name, "r");	 //Open the sound file
		
	FSfseek(soundfile,0,2);                  //find out how many samples to play
	SAMPLE_COUNT = FSftell(soundfile) - 100; //subtract 100 bytes to account for any header information on the end
	FSfseek(soundfile,0,0);					 //seek back to the begining
		
	FSfread(snd_buffer_1, 1, sound_buf_size, soundfile); //Pre-fill the first buffer
	
	buffer_select = 0;                       //Set the first buffer to buffer 1
	buffer_index = 0;                        // Reset buffer index
	buffer_1_state = 0xFFFF;                 //Tell the load function buffer 1 is full
	buffer_2_state = 0;                      //buffer two needs filling
	
	stop = 0;
		
	//PHASE_INC = PWM_FREQ / 16000 + 1;
	//	if(PHASE_INC < 1)
	//		PHASE_INC = 1;
//	PHASE_ACC = 0;						     // Reset Sample Count

	
}

void PWM_Load_Buffer(){	

	//	char character[10]; //debug information variable
	
	//call_count++;       //debug information variable
	
	/*if(call_count == 60){ //every sixty frames display which sample we are on
		call_count = 0;
		sprintf(character, "%lu", PHASE_ACC);
		GFX_TMap_Print_String(character);
		GFX_TMap_Print_String("\n");
	}*/	
	if(stop == 0){
		
	
		if(buffer_1_state == 0){  //Check if buffer one needs filling
	 		FSfread(snd_buffer_1, 1, sound_buf_size, soundfile);
	 		buffer_1_state = !buffer_1_state;
			}
	
		if(buffer_2_state == 0){ //Check if buffer two needs filling
			FSfread(snd_buffer_2, 1, sound_buf_size, soundfile);
			buffer_2_state = !buffer_2_state;
 			}
	}
} 	


// Timer 2 ISR steps through file during playback
void __attribute__((__interrupt__, no_auto_psv)) _T2Interrupt( void )
{

//	PHASE_COUNT++;
//if(PHASE_COUNT == PHASE_INC)
//	{
		//PHASE_ACC++;     //Increment Sample
		buffer_index++;  //Increment buffer index
//		PHASE_COUNT= 0;
//	}

/*if(PHASE_ACC > SAMPLE_COUNT){	        //If entire file has been played
	T2CONbits.TON = 0; //timer stop!
	T3CONbits.TON = 0;
	FSfclose(soundfile);         //Close soundfile
	for(buffer_index = 0; buffer_index < sound_buf_size; buffer_index++) {
		snd_buffer_1[buffer_index] = 0;
		snd_buffer_2[buffer_index] = 0;
	}
	PHASE_ACC = 0;
	SAMPLE_COUNT = 0;	
}*/	
	
if(buffer_index == sound_buf_size){ //See if buffer is empty and set appropriate flags
	buffer_index = 0;
	if(buffer_select == 0){
		buffer_select = 0xffff;
		buffer_1_state = 0;
	}
	else {
		buffer_select = 0;
		buffer_2_state = 0;
	}

	}	
	
	if(buffer_select == 0){        //select the right buffer and read out of it
	OC8RS = snd_buffer_1[buffer_index]; // Write Duty Cycle value for next PWM cycle
	snd_buffer_1[buffer_index] = 0;
	}
	else{
		OC8RS = snd_buffer_2[buffer_index]; // Write Duty Cycle value for next PWM cycle
		snd_buffer_2[buffer_index] = 0;
	}
	IFS0bits.T2IF = 0; // Clear Timer 2 interrupt flag
}

void PWM_Stop(){
	stop = 1;
	IEC0bits.T2IE == 0;
}

void PWM_Pause(){
	if(IEC0bits.T2IE == 0) IEC0bits.T2IE = 1;
	else if(IEC0bits.T2IE == 1) IEC0bits.T2IE = 0;
}

int PWM_Find_Sound_Length(){
	int i;
	i = SAMPLE_COUNT / 15720;
	
	return i;
}		 	

void PWM_Skip_Bytes(long skip){
FSfseek(soundfile,skip,1);


}	
	
