//********************************************************************************
//*                                                                              *
//*           Name:  lcd.h                                                       *
//* Project Number:  MT2192                                                      *                                                   
//*           Date:  10/15/2007                                                  *                                                  
//*                                                                              *
//*   Description:                                                               * 
//*     This application implements an SPI Based LCD Controller controller that  *
//* communicates with Microchip's DSPICDEMO Board LCD by using the SPI2 interface*
//* hardware.                                                                    *
//*                                                                              *
//*     This controller can be used with a PC host in order to  give any PC the  *
//* capability to serve as an SPI Master.  It can also be used for robotic       *
//* applications where multiple controllers and I2C devices are used, with a     *
//* minimum of data and control lines needed to connect them.  Each I2C device   *
//* has its own unique address and only listens to commands associated with it.  *
//*                                                                              *
//* CHARACTER COMMANDS															 *
//* The character commands treat the display like a 20-column, 4-row character 	 *
//* display where the characters occupy 6 x 8 pixel elements. Note that there    *
//* are 20 character locations on a row, each 6 pixels wide. This current 		 *       
//* implementation only uses 120 pixels of the 122-pixel wide display. The spare *
//*  two-pixel columns are on the right of the display and cannot be affected by * 
//* the character commands. The controller has a fixed space 5 x 7 font set for  *
//* the printable ASCII characters in range: 0x20 -> 0x7E. The					 *
//* controller also recognizes three special characters that do not affect		 *
//*  the display.																 *
//* 1. CR: (0x0D) Increments ChrPosR.											 *
//* 2. LF: (0x0A) Sets ChrPosC = 0 and increments ChrPosR.  					 *
//* 3. BS: (0x08) Decrements ChrPosC if ChrPosC > 0; it is non-destructive 		 *
//*                                                                              *
//*                                                                              *
//********************************************************************************
   
#ifndef LCD_LOADED  /* prevent multiple loading */
#define LCD_LOADED
       
// Include file headers here

//#include "flags.h"                                      // Global flags used for MPSIM debugging 
 
#ifdef DSPIC30F2010     
#include <p30F2010.h>					// Register definitions for DSPIC30F2010
#endif

#ifdef DSPIC30F6014     
#include <p30F6014.h>					// Register definitions for DSPIC30F6014
#endif
  
#include "my_types.h"					// General C/C++ types

#define	 esc			0x1B

#define	 CR				0x0D			// Increments ChrPosR
#define	 LF				0x0A			// Sets ChrPosC=0 and increments ChrPosR.
#define	 BS				0x08			// Decrements ChrPosC if ChrPosC > 0; Non Destructive


//********************************************************************************
// LCD Function Prototypes
//********************************************************************************

// General LCD commands 

void Lcd_Initialize(void);
void Lcd_Reset(void);
void Lcd_Home(void);
void Lcd_Clear(void);
void Lcd_Scroll(byte ScrollY);

// Character Commands

void Lcd_Set_Character_Position(byte ChrPosC, byte ChrPosR);
void Lcd_Increment_Character_Position(void);
void Lcd_Write_Character(byte ChrPosC, byte ChrPosR, char Ascii_Char);
void Lcd_Write_Character_Increment(byte ChrPosC, byte ChrPosR, char Ascii_Char);
void Lcd_Write_Character_Next(char Ascii_Char);
void Lcd_Clear_Row(byte ChrPosR);
void Lcd_Clear_End (void);
void Lcd_Cursor_Off(void);
void Lcd_Cursor_On(void);
void Lcd_Blink_Cursor(byte Blink_Time);
void Lcd_Display_Characteraracter(char Ascii_Char);

//  stdio equivalent functions  

//******************************************************************************
//* Function Name:    Lcd_Printf                                               *
//* Description:      LCD display equivalent to stdio printf.                  *
//*                                                                            *
//******************************************************************************
int Lcd_Printf(const char *format, ...);

//******************************************************************************
//* Function Name:    Lcd_Puts                                                 *
//* Description:      LCD display equivalent to stdio puts.                    *                                                          
//*                                                                            *
//******************************************************************************
void Lcd_Puts(char *s);
 
                 
//******************************************************************************
//* Function Name:    Lcd_Hex2                                                 *
//* Description:      LCD equivalent to Parallax Stamp hex2 function that      *                 
//*                   displays a byte Value as a 2 digit hex character.        *                  
//******************************************************************************
void Lcd_Hex2(byte Value);
        
//******************************************************************************
//* Function Name:    Lcd_Dec                                                  *
//* Description:      LCD equivalent to Parallax Stamp dec function that       *                 
//*                   displays an integer Value in ASCII format.               * 
//*                                                                            *
//******************************************************************************
void Lcd_Dec(int Value); 
 
         
//******************************************************************************
//* Function Name:    Lcd_Scroll_Left                                          *
//* Description:      Scrolls LCD screen left N positions.                     *
//*                                                                            *
//******************************************************************************
void Lcd_Scroll_Left(byte N);
         
    
//******************************************************************************
//* Function Name:    Lcd_Scroll_Right                                         *
//* Description:      Scrolls LCD screen right N positions.                    *
//*                                                                            *
//******************************************************************************
void Lcd_Scroll_Right(byte N);
     
//******************************************************************************
//* Function Name:    Lcd_Goto_XY                                              *
//* Description:      Go to LCD position (X, Y)                                *
//*                                                                            *
//******************************************************************************

void Lcd_Goto_XY(char X, char Y);
                         
// Pixel Commands

void Lcd_Pixel_Position(byte PixPosX, byte PixPosY);
void Lcd_Pixel_On(byte PixPosX, byte PixPosY);
void Lcd_Pixel_Off (byte PixPosX, byte PixPosY);
void Lcd_Draw_Line_Rel(byte PixPosX, byte PixPosY);
void Lcd_Draw_Line_Abs(byte PixPosX1, byte PixPosY1, byte PixPosX2, byte PixPosY2);

// Column Commands

#endif
