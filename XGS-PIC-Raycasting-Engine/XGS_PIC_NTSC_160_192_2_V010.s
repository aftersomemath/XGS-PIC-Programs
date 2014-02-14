;//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
;/ 
;/ Copyright Nurve Networks LLC 2008
;/ 
;/ Filename: XGS_PIC_NTSC_160_192_2_V010.S
;/
;/ Original Author: Joshua Hintze
;/ 
;/ Description: 160x192 resolution NTSC driver with 4-bits per pixel and multiple color palettes, thus
;/ every 4-bit pixel is used as an index into the color palette for that particular pixel's 8x8 group palette as shown 
;/ below: 
;/
;/                                Color Palette for 8x8 block
;/                                                    
;/ Pixel Value     Palette Index             Index      Color Encoding
;/   00       =         0    ------------->   0   - [ LUMA3..0 : CHROMA3..0 ] - 8-bit color (some values for sync etc.)
;/   01       =         1    ------------->   1   - [ LUMA3..0 : CHROMA3..0 ] - " "
;/   10       =         2    ------------->   2   - [ LUMA3..0 : CHROMA3..0 ] - " "
;/   11       =         3    ------------->   3   - [ LUMA3..0 : CHROMA3..0 ] - " "
;/
;/ Memory for bitmap modes is laid out from top left corner of the screen from left to right, top to bottom.
;/ Each byte represents 4 pixels where pixel 0,1,2,3 on screen are encoded frow high bit to low bit as shown below:
;/ 
;/ Pixel Encoding
;/
;/     P0  |   P1  |   P2  |   P3     <------ pixel #
;/ [ b7 b6 | b5 b4 | b3 b2 | b1 b0 ]  <------ bit encodings
;/ msb                            lsb        
;/ 
;/                Video RAM Layout
;/ (0,0).....................................(159,0)
;/ .P0 P1 ...                                 P159
;/ .
;/ .
;/ .
;/ .
;/ .
;/ .
;/ (0,191)...................................(159, 191)                          
;/ 
;/ For example, if you wanted to change the color of the pixel (0,0), then you would write the upper 4-bits
;/ of the very first byte of Video RAM.
;/
;/ The interface between the C caller and this driver is accomplished thru a pointer to VRAM from C,
;/ the palette base address exported back to C from this driver, along with a few status variables
;/ from the driver to interogate the current line, etc. See below:
;/
;/ There is a single pointer to VRAM, the driver expects the C caller to allocate memory and then 
;/ initialize a pointer with the following name to point to the VRAM:
;/ 
;/ .extern _g_CurrentVRAMBuff      // 16-bit pointer to video RAM. Note that C code strips the leading "_" underscore
;/                                 // so it is not needed in C code
;/
;/ .extern _g_ColorTable           // 16-bit pointer to the 16 entry color table array. C code must provide this
;/ .extern _g_AttribTable          // 16-bit pointer to a int array that contains the index of the 8x8 blocks that maps to the color tables
;/
;/
;/ This driver uses an Attribute Table simular to the Nintendo's PPU unit on the original NES. Basically this
;/ is just one more level of indirection allowing us to choose a seperate color table for each 8x8 pixel block
;/ by doing this we can have a lot more colors on the screen than just one 16 color table.
;/
;/ The way the attribute table works is that it is filled with an index to an array of color tables. If the index is 0
;/ then the color table that the pixel grouping will use is the first table. If the index is 1, then its the second table
;/ located 16 bytes after the first color table. Note: All color tables must be continous in memory.
;/
;/ In the case of this video driver being 160x192 we have 20x24 different attribute indices. As shown below
;/ ---------------------------------------------
;/ |   8x8  |   8x8  |   8x8  | ........|   8x8  |
;/ | Att. 0 | Att. 1 | Att. 2 |.........| Att. 19|
;/ | _ _ _ __ _ _ __ _ _ __ _ _ __ _ _ __ _ _ _ _|
;/ |   8x8  |   8x8  |   8x8  | ........|   8x8  |
;/ | Att. 20| Att. 21| Att. 22|.........| Att. 39|
;/ | _ _ _ __ _ _ __ _ _ __ _ _ __ _ _ __ _ _ _ _|
;/ |        |        |        | ........|        |
;/ |  ....  |  ....  |  ....  |.........|  ....  |
;/ | _ _ _ __ _ _ __ _ _ __ _ _ __ _ _ __ _ _ _ _|
;/ |   8x8  |   8x8  |   8x8  | ........|   8x8  |
;/ | Att.440| Att.441| Att.442|.........| Att.479|
;/ | _ _ _ __ _ _ __ _ _ __ _ _ __ _ _ __ _ _ _ _|
;/
;/ One last thing is that if you want to simplify this, just set the entire Attribute table to 0 and have only
;/ a single color table. In otherwords you can have more than one index pointing to the same table.
;/
;/ Next, the driver passses back the current absolute video line 0..261 via g_CurrentLine and the current
;/ virtual raster line via g_RasterLine. It also passes back a flag such that when we are NOT in Vsync it is 0,
;/ otherwise the VsyncFlag is set all bits high.
;/
;/ .global _g_CurrentLine  ; read only, current physical line 0...524
;/ .global _g_RasterLine   ; read only, current logical raster line
;/ .global _g_VsyncFlag    ; read only, Flag to indicate we are in v-sync

;/
;/ Modifications|Bugs|Additions:
;/
;/ Author Name: 
;/
;/
;/
;/
;//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

;//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
;/ INCLUDES 
;//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

; Include our register and chip definitions file
.include "p24HJ256GP206.inc"


;//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
;/ DEFINES
;//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

; width and height of screen 
.equ NTSC_SCREEN_WIDTH,     160
.equ NTSC_SCREEN_HEIGHT,    192

; controls the number of lines to shift the display vertically, later assign to global variable, so caller can control
.equ NTSC_VERTICAL_OFFSET,  40

; number of bits each pixel is encoded with
.equ NTSC_PIXELS_PER_BYTE,  4            

; Number of clocks per pixel (12 in the case of NTSC) Modifying this won't really change anything,
; you will need to change code delays to get the correct clocks per pixel change
.equ CLOCKS_PER_PIXEL, 12

; Number of lines we should repeat to reduce the vertical resolution
.equ LINE_REPT_COUNT,   1

; with the current D/A converter range 0 .. 15, 1 = 62mV
.equ NTSC_SYNC,           (0x0F)     ; 0V  
.equ NTSC_BLACK,          (0x4F)     ; 0.3V (about 30-40 IRE)

.equ NTSC_CBURST0,        (0x30) 
.equ NTSC_CBURST1,        (0x31) 
.equ NTSC_CBURST2,        (0x32) 

.equ NTSC_CHROMA_OFF, 	0x000F
.equ NTSC_OVERSCAN_CLR,	0x005F

.equ COLOR_LUMA,    (NTSC_CBURST1 + 0x40)

.equ NTSC_GREEN,    (COLOR_LUMA + 0)        
.equ NTSC_YELLOW,   (COLOR_LUMA + 2)        
.equ NTSC_ORANGE,   (COLOR_LUMA + 4)        
.equ NTSC_RED,      (COLOR_LUMA + 5)        
.equ NTSC_VIOLET,   (COLOR_LUMA + 9)        
.equ NTSC_PURPLE,   (COLOR_LUMA + 11)        
.equ NTSC_BLUE,     (COLOR_LUMA + 14)        

.equ NTSC_GRAY0,    (NTSC_BLACK + 0x10)        
.equ NTSC_GRAY1,    (NTSC_BLACK + 0x20)        
.equ NTSC_GRAY2,    (NTSC_BLACK + 0x30)        
.equ NTSC_GRAY3,    (NTSC_BLACK + 0x40)        
.equ NTSC_GRAY4,    (NTSC_BLACK + 0x50)        
.equ NTSC_GRAY5,    (NTSC_BLACK + 0x60)        
.equ NTSC_GRAY6,    (NTSC_BLACK + 0x70)        
.equ NTSC_GRAY7,    (NTSC_BLACK + 0x80)        
.equ NTSC_GRAY8,    (NTSC_BLACK + 0x90)        
.equ NTSC_GRAY9,    (NTSC_BLACK + 0xA0)        
.equ NTSC_WHITE,    (NTSC_BLACK + 0xB0) 



;//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
;/ EXTERNALS
;//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

.extern _g_CurrentVRAMBuff      ; 16-bit pointer to the vram that is meant to be drawn
.extern _g_ColorTable           ; 16-bit pointer to the array of color tables.
.extern _g_AttribTable          ; 16-bit pointer to a int array that contains the index of the 8x8 blocks that maps to the color tables

;//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
;/ SRAM SECTIONS
;//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

; The following section describes initialized (data) near section. The reason we want
; to place these variables into the near SRAM area is so that ASM codes, that don't use many
; bits to encode data, will be able to access the variables.
.section *,data,near
_g_CurrentLine:		.int 245		        ; Line count 0->261
_g_RasterLine:      .int 0                  ; Virtual Raster count 0->96
ReptLine:	        .int LINE_REPT_COUNT	; Repeated line counter 0->2
CurrVRAMPtr:	    .int 0			        ; Line count 0->525
CurrAttribPtr:	    .int 0			        ; Points to g_AttributeBuf that is defined in the C program as a global
_g_VsyncFlag:       .int 0			        ; Flag to indicate we are in v-sync
ReptAttrib:	        .int 8			        ; Repeat attribute table increment 8 pixel lines

; Un-initialized Data sections
.section *,bss,near
AttribScanLine: .space ((NTSC_SCREEN_WIDTH/8)*2)	; Contains pointers to the palette table for current scanline.


;//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
;/ GLOBAL EXPORTS
;//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

.global __T1Interrupt   ; Declare Timer 1 ISR name global
.global _g_VsyncFlag    ; read only, Flag to indicate we are in v-sync
.global _g_CurrentLine  ; read only, current physical line 0...261
.global _g_RasterLine   ; Virtual Raster count 0->96


;//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
;/ ASM FUNCTIONS
;//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

; The following are code sections
.text           

__T1Interrupt:

		; Our instruction clock is running at 12*NTSC dot clock
		; so every 12 clock cycles is a normal 227 pixels
		disi #0x3FFF								; (1) Disable interupts for up to 10000 cycles

		; Save off variables we will use
		push.s										; (1) Push status registers
		push SR
		push W4										; (1) Temp variable
		push W5
		push RCOUNT									; (1) Make sure we are pushing Rcount since we are using repeat

		; Increment which line we are on
		inc _g_CurrentLine							; (1)
		mov _g_CurrentLine, W0						; (1) Set our line variable into W0

		; Test if we are done with this frame (262 lines)
		mov #261, W2								; (1)
		cp W0, W2									; (1) See if we are at line 262 = 0
		bra nz, DONE_LINE_TEST						; (1/2) if(_g_CurrentLine != 262) jump
		setm _g_CurrentLine							; (1) Reset Line counter set to 0xFFFF
DONE_LINE_TEST: 

		; Total = 10

        ; The following section of code determines what kind of line we should be 
        ; drawing to the screen. Is it a Vsync Line, Black Top/Bottom, or an
        ; Active Video line. All of this is determined by the _g_CurrentLine counter
        ; variable

        ; if(_g_CurrentLine < VSYNC_LINE_COUNT)
		mov #5, W2									; (1)
		cp W0, W2									; (1) Compare to 5
		bra gt, NOT_VSYNC_LINE						; (1/2) if(Line > 5) jump

		; Vsync line (18)
		repeat	#11									; (1)
		nop											; (1+n)
		goto DRAW_VSYNC_LINE						; (2) draw a black line to the screen

NOT_VSYNC_LINE:

		; Test to see if this is a top blanking line
		mov #NTSC_VERTICAL_OFFSET, W2				; (1)
		cp W0, W2									; (1) Compare to NTSC_VERTICAL_OFFSET
		bra ge, NOT_TOP_OVERSCAN_LINE				; (1/2) if(Line >= NTSC_VERTICAL_OFFSET) jump

		; Blank Line Top (18)
		nop											; (1)
        nop											; (1)
        clr _g_RasterLine                           ; (1) Clear out our raster line count
		mov _g_CurrentVRAMBuff, W1					; (1) Copy over CurrVRAMPtr = _g_CurrentVRAMBuff
		mov W1, CurrVRAMPtr							; (1) copy
		mov _g_AttribTable, W1					    ; (1) Copy over ATTRIBTABLE = g_AttributeTable
		mov W1, CurrAttribPtr						; (1) copy
		mov #LINE_REPT_COUNT, W1					; (1) Reset repeat line back to count
		mov W1, ReptLine							; (1)
		goto DRAW_OVERSCAN_LINE						; (2) draw a black line to the screen

NOT_TOP_OVERSCAN_LINE:

		; Test to see if this is a bottom blanking line
		mov #((NTSC_SCREEN_HEIGHT*LINE_REPT_COUNT)+NTSC_VERTICAL_OFFSET), W2	; (1)
		cp W0, W2									; (1) Compare to 245
		bra lt, NOT_BOTTOM_OVERSCAN_LINE			; (1/2) if(Line < 245) jump

		; Blank Line Bottom (18)
		repeat	#2									; (1)
		nop											; (1+n)
		setm _g_VsyncFlag							; (1) set the vsync flag 
		goto DRAW_OVERSCAN_LINE						; (2) draw a black line to the screen

NOT_BOTTOM_OVERSCAN_LINE:

		; This must be an active line (18)
		clr _g_VsyncFlag							; (1) g_VsyncFlag = 0
		nop											; (1) delay
		nop											; (1) delay
		nop											; (1) delay
		goto DRAW_ACTIVE_LINE						; (2) draw an active line to the screen



; ----------------- VSYNC LINE -------------------------
DRAW_VSYNC_LINE:

		; This draws a vsync line with h-sync pulse inverted for serration
		; H-sync serration (4.7us) 236
		mov #NTSC_BLACK,W0 	                        ; (1) turn hsync on
		mov W0, LATB								; (1)
		repeat #232									; (1)
		nop											; (1+n)
		mov #NTSC_SYNC, W0	                        ; (1)
		mov W0, LATB								; (1)

		; Done with this completely
		goto FINISHED_NTSC							; Dont need to count because interrupts will keep us locked



; ----------------- OVERSCAN LINE -------------------------
DRAW_OVERSCAN_LINE:

		; This draws a overscan line with h-sync (used for blanking lines)
		; H-sync (4.7uS) = 202
		mov #NTSC_SYNC, W0	                        ; (1) turn hsync on
		mov W0, LATB								; (1)
		repeat #198									; (1)
		nop											; (1+n)

		; Pre-burst (.6uS) = 26 clocks
		mov #NTSC_BLACK, W0	                        ; (1) turn hsync off black with no color signal
		mov W0, LATB								; (1)
		repeat #22									; (1)
		nop											; (1+n)


		; Color burst (10 NTSC clocks) = 120 clocks
		mov #NTSC_CBURST1, W0						; (1) turn hsync color burst on at phase 0
		mov W0, LATB								; (1)
		repeat #116									; (1)
		nop											; (1+n)


		; Post-burst (.6uS) = 26 clocks
		mov #NTSC_BLACK, W0	                        ; (1) turn hsync off black with no color signal
		mov W0, LATB								; (1)
		repeat #22									; (1)
		nop											; (1+n)


		; Overscan color
		mov #NTSC_OVERSCAN_CLR, W0					; (1) turn hsync off black with a color signal
		mov W0, LATB								; (1)

		; Done with this completely
		goto FINISHED_NTSC							; Dont need to count because interrupts will keep us locked




; ----------------- ACTIVE LINE -------------------------
DRAW_ACTIVE_LINE:


		; ------- HSYNC ON ---------
		; H-sync (4.7uS) = 202
		mov #NTSC_SYNC, W0 	                        ; (1) turn hsync on
		mov W0, LATB								; (1)

        ; Now we are going to pre-comnputer the pointers for each 8 pixel section of this scanline
        ; to its corresponding color table. We have plenty of time to do this in the hsync.
        ; Essentially the final resulting pointer is equal to:
        ;
        ;   FinalPointer[pixel] = g_ColorTable (base) + g_AttribTable[pixel] * 16 (size of an individual color table)

		; Copy over the pointers to the different palettes
		mov CurrAttribPtr, W0 						; (1) Get point to beginning of attribtute table for this line
		mov #4, W1									; (1) Move our multiplier to W1...this is for indexing into the attribute table
		mov _g_ColorTable, W2						; (1) Get starting pointer to Color Table
		mov #AttribScanLine, W3					    ; (1) Get start to this lines Attribute line

		; Time taken 20 * 2 = 40 clocks
.rept (NTSC_SCREEN_WIDTH/8)
		mul.uu W1, [W0++], W4						; (1) Figure out offset pointer into palette
		add W2, W4, [W3++]							; (1) Add ColorTable Start to Attribute*4
.endr

		repeat #154									; (1)
		nop											; (1+n)

		; Pre-burst (.6uS) = 26 clocks
		mov #NTSC_BLACK, W0	                        ; (1) turn hsync off black with no color signal
		mov W0, LATB								; (1)
		repeat #22									; (1)
		nop											; (1+n)


		; Color burst (10 NTSC clocks) = 120 clocks
		mov #NTSC_CBURST1, W0 					    ; (1) turn hsync color burst on at phase 0
		mov W0, LATB								; (1)
		repeat #116									; (1)
		nop											; (1+n)


		; FROM HERE ON it is imparative that we stay in sync with the color clock signal
		; (i.e. we don't change any pixels unless it is an integer multiple of 12)
		; If we don't do this then color changes will not be correct.
		; Post burst
		mov #NTSC_BLACK, W0	                        ; (1) turn hsync off black with no color signal
		mov W0, LATB								; (1)
		repeat #19									; (1)
		nop											; (1+n)


		; Left overscan color
		mov #(NTSC_OVERSCAN_CLR | NTSC_CHROMA_OFF), W0 ; (1) turn hsync off black with no color signal
		mov W0, LATB								; (1)
;		mov _g_Breazeway2, W0						; (1)
;		repeat #198									; (1)
;		repeat W0									; (1)
		repeat #168                                 ; (1)
		nop											; (1+n)


		; Now draw out some pixels (10 clocks per pixel)
		mov #(NTSC_SCREEN_WIDTH/8), W4				; (1) Rendering out 80*2=160 pixels
		mov CurrVRAMPtr, W0							; (1) Move frame pointer to W0
		mov AttribScanLine, W3					    ; (1)
		mov #AttribScanLine, W5					    ; (1) pointer into our attribtable line

		; Spend 12 clocks per pixel
PIXEL_LOOP:

		; ----------------
		; Pixels 0, 1, 2, 3
		; ----------------
		; Rendering Loop
		mov.b [W0], W2								; (1) Get the next 4 pixels (2-bits each)
		lsr W2, #6, W1								; (1) Shift right and store in W1
		and #0x03, W1								; (1) Mask off bottom two pixels
		mov.b [W1+W3], W2							; (2) Use indirect addressing to get assigned color
		mov W2, PORTB								; (1) Copy to port B
		repeat #4									; (1) delay
		nop											; (1+n)

		mov.b [W0], W2								; (1) Get the next 4 pixels (2-bits each)
		lsr W2, #4, W1								; (1) Shift right and store in W1
		and #0x03, W1								; (1) Mask off bottom two pixels
		mov.b [W1+W3], W2							; (2) Use indirect addressing to get assigned color
		mov W2, PORTB								; (1) Copy to port B
		repeat #4									; (1) delay
		nop											; (1+n)

		mov.b [W0], W2								; (1) Get the next 4 pixels (2-bits each)
		lsr W2, #2, W1								; (1) Shift right and store in W1
		and #0x03, W1								; (1) Mask off bottom two pixels
		mov.b [W1+W3], W2							; (2) Use indirect addressing to get assigned color
		mov W2, PORTB								; (1) Copy to port B
		repeat #4									; (1) delay
		nop											; (1+n)

		mov.b [W0++], W1							; (1) Get the next 4 pixels (2-bits each)
		nop											; (1) delay
		and #0x03, W1								; (1) Mask off bottom two pixels
		mov.b [W1+W3], W2							; (2) Use indirect addressing to get assigned color
		mov W2, PORTB								; (1) Copy to port B
		repeat #4									; (1) delay
		nop											; (1+n)


		; ----------------
		; Pixels 4, 5, 6, 7
		; ----------------
		; Rendering Loop
		mov.b [W0], W2								; (1) Get the next 4 pixels (2-bits each)
		lsr W2, #6, W1								; (1) Shift right and store in W1
		and #0x03, W1								; (1) Mask off bottom two pixels
		mov.b [W1+W3], W2							; (2) Use indirect addressing to get assigned color
		mov W2, PORTB								; (1) Copy to port B
		repeat #4									; (1) delay
		nop											; (1+n)

		mov.b [W0], W2								; (1) Get the next 4 pixels (2-bits each)
		lsr W2, #4, W1								; (1) Shift right and store in W1
		and #0x03, W1								; (1) Mask off bottom two pixels
		mov.b [W1+W3], W2							; (2) Use indirect addressing to get assigned color
		mov W2, PORTB								; (1) Copy to port B
		repeat #4									; (1) delay
		nop											; (1+n)

		mov.b [W0], W2								; (1) Get the next 4 pixels (2-bits each)
		lsr W2, #2, W1								; (1) Shift right and store in W1
		and #0x03, W1								; (1) Mask off bottom two pixels
		mov.b [W1+W3], W2							; (2) Use indirect addressing to get assigned color
		mov W2, PORTB								; (1) Copy to port B
		repeat #4									; (1) delay
		nop											; (1+n)

		mov.b [W0++], W1							; (1) Get the next 4 pixels (2-bits each)
		nop											; (1) delay
		and #0x03, W1								; (1) Mask off bottom two pixels
		mov.b [W1+W3], W2							; (2) Use indirect addressing to get assigned color
		mov W2, PORTB								; (1) Copy to port B
		mov [++W5], W3								; (1) Get next color table loaded into W3
		nop											; (1)
		nop											; (1)

		
		; Branch until line is done
		sub #1, W4									; (1) W4--
		bra nz, PIXEL_LOOP							; (1/2) if(W4 != 0) jmp


		; Now set PORTB to overscan
		mov #(NTSC_OVERSCAN_CLR | NTSC_CHROMA_OFF), W0 ; (1) turn hsync off black with no color signal
		nop											; (1)
		nop											; (1)
		mov W0, LATB								; (1)

		; Now increment our framepointer if we are done with this line (every 2 lines)
		dec ReptLine								; (1)
		bra nz, FINISHED_NTSC						; (1/2)
		mov CurrVRAMPtr, W0							; (1)
		add #(NTSC_SCREEN_WIDTH/NTSC_PIXELS_PER_BYTE), W0	; (1) increment our pointer to the next line 
		mov W0, CurrVRAMPtr							; (1) save
		mov #LINE_REPT_COUNT, W0					; (1) 
		mov W0, ReptLine							; (1) Reset our line repeate counter
        inc _g_RasterLine                           ; (1) Increment our raster line count
		
		; Now test if we should increment our attribute table pointer. We only do this
		; everly 8 pixel lines so that our attribute table covers 8x8 tiles in a sense.
		dec ReptAttrib								; (1)
		bra nz, FINISHED_NTSC						; (1/2)
		mov #((NTSC_SCREEN_WIDTH/8)*2), W0	    	; (1) increment pointer by sizeof(int)*20 tiles
		add CurrAttribPtr							; (1) increment it
		mov #8, W0									; (1)
		mov W0, ReptAttrib							; (1)



		; Done with this completely
FINISHED_NTSC:

        bclr IFS0, #T1IF           					; Clear the Timer1 Interrupt flag Status
                                   					; bit.

		pop RCOUNT									; (1) Make sure we are pushing Rcount since we are using repeat
		pop W5
		pop W4                   					; Retrieve context POP-ping from Stack
		pop SR
		pop.s										; (1) Push status registers

		disi #0x0									; Re-enable interrupts

        RETFIE                     					;Return from Interrupt Service routine




;--------End of All Code Sections ---------------------------------------------

.end                               ;End of program code in this file
        