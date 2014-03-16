//********************************************************************************
//*                                                                              *
//*          Name:  lcd.c                                                        *
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
  
  
// Include file headers here
#include "flags.h"

#ifdef DSPIC30F2010     
#include <p30F2010.h>					// Register definitions for DSPIC30F2010
#endif

#ifdef DSPIC30F6014     
#include <p30F6014.h>					// Register definitions for DSPIC30F6014
#endif
 
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <dsp.h>
#include <uart.h>
#include <spi.h>
#include <i2c.h>
#include <math.h>
#include <adc12.h>

#include "my_types.h"					// General C/C++ types
#include "lcd.h"						// LCD Support using SPI2 
  
#define UART_SUPPORT
#define SPI_SUPPORT

//#define LCD_TEST_1
//#define LCD_TEST_2
#define LCD_TEST_3

#define STRING_SIZE 80 					// String size 

//*******************************************************************************
// Define DSPIC Demo Board LCD commands (op-codes) here.  These are issued via the 
// SPI2 interface.
//*******************************************************************************


// General Commands

#define Reset			0x80
#define Home  			0x81
#define HomeClear		0x82 
#define Scroll			0xA3

// Character Commands

#define ChrPos   		0xC5
#define ChrPosInc		0x8E
#define WrtChr 			0xE6
#define WrtChrInc		0xE7
#define WrtChrNext		0xA8
#define ChrClearRow		0xA9
#define ChrClearEol		0x8A
#define ChrCursorOff	0x8B
#define ChrCursorOn 	0x8C
#define ChrCursorBlink 	0x8D

// Pixel Commands
 
#define PixPos  		0xD7
#define PixOn  			0xD8
#define PixOff  		0xD9
#define PixLine 		0xDA
 
// Column Commands

#define ColPos 			0xDB
#define WrtColNext	 	0xBC
#define WrtColNextOR	0xBD
#define WrtColNextAND	0xBE
#define WrtColNextXOR	0xBF
#define WrtColNext	 	0xBC
#define WrtColNext	 	0xBC

// Declare global variables here

int vector1[8],vector2[8],vector3[8],vector4[8],vector5[8];

unsigned int spi_in,adc4_in,adc5_in,adc6_in,adc8_in;
int *textin;
char realtext;
//const __attribute__((section(".const_cc"),space(prog)))
char ccllar_text[20]   = {"   Circuit Cellar   "};
//const __attribute__((section(".primer_cc"),space(prog)))
char primer_text[20]   = {"    dsPIC Primer    "};
//const __attribute__((section(".complicated_cc"),space(prog)))
char compli_text[20]   = {"It's NOT Complicated"};
//const __attribute__((section(".embedded_cc"),space(prog)))
char embedded_text[20] = {"   It's EMBEDDED!   "};

//********************************************************************************
// Function Prototypes
//********************************************************************************

// Local LCD Application Functions  

void Lcd_Send_Control(byte LCDControlByte);
void Lcd_Send_Data(byte LCDDataByte);

  


#ifdef LCD_TEST
int main ()
{
	float f1 = 3.1415;

	int i,j;									// Local Loop Index variables
	unsigned int spi2cfg_1,spi2cfg_2,x,y,z;		// SPI Configuration variables
 	int timeout;								// Timeout value during which timer1 is ON

    char sbuf[] = "Hello World!!!\0";

   	// Configure UART  1 for current CPU cloc and baud rate,  8-Bits, No parity, Asynchronous
 
  	InitializeUart1();

	// Initialize using LCD here...
    Lcd_Initialize(); 

 
	//**************************************************************************
 	//* USART Test Here
	//**************************************************************************

    printf(" Hello World \r\n");
	
	for (i=0;i<10;i++)
	{
		putchar('X');
		putchar('Y');
		putchar('Z');
		printf(" This is a test %d times \r\n",i);
	}

	//**************************************************************************
	// Test LCD on dsPIC demo board using the SPI interface here
	//**************************************************************************
		 


	LATGbits.LATG9 = 1;
    LATGbits.LATG9 = 0;
    spi_in = ReadSPI2();
    WriteSPI2(0x82);
    while(!(DataRdySPI2()));

	LATGbits.LATG9 = 1;
    LATGbits.LATG9 = 0;
    spi_in = ReadSPI2();
    WriteSPI2(0x82);
    while(!(DataRdySPI2()));

#ifdef LCD_TEST_1

    for(x=0;x<20;++x)
    {
    	textin = &ccllar_text[x] + 0x8000;
    	realtext = *textin;
    	Lcd_Display_Character(realtext);
    }
    for(x=0;x<20;++x)
    {
    	textin = &primer_text[x] + 0x8000;
    	realtext = *textin;
    	Nop();
    	Lcd_Display_Character(realtext);
    }
     for(x=0;x<20;++x)
    {
    	textin = &compli_text[x] + 0x8000;
    	realtext = *textin;
    	Lcd_Display_Character(realtext);
    }
    for(x=0;x<20;++x)
    {
    	textin = &embedded_text[x] + 0x8000;
   	 	realtext = *textin;
    	Nop();
    	Lcd_Display_Character(realtext);
    }


    Nop();

	// Clear LCD and home the cursor
 	Lcd_Clear();
   
    // Draw a diagonal line 1,1 to 121, 31

    Lcd_Draw_Line_Abs(1,1,121,31);
    Lcd_Draw_Line_Abs(121,1,1,31);
#endif

#ifdef LCD_TEST_2
 
	// Clear LCD and home the cursor
 	Lcd_Clear();
	Lcd_Puts(sbuf);
    //while(1);

	//Lcd_Puts("Hello World!!!");

	// Clear LCD and home the cursor
 	Lcd_Clear();

    for (i=0; i<4; i++)
	{
		Lcd_Set_Character_Position(1,i);
		Lcd_Puts("Hello World!!!");
		//Lcd_Puts(sbuf);

	}
	while(1);

#endif

#ifdef LCD_TEST_3

	// LCD printf test...
	// Clear LCD and home the cursor
 	Lcd_Clear();
	Lcd_Printf("f1 = %f ", f1);
	while(1);

	// Clear LCD and home the cursor
 	Lcd_Clear();

    for (i=0; i<4; i++)
	{
		Lcd_Set_Character_Position(1,i);
		Lcd_Printf("TestA # %d", i);
		printf("Test # %d \r\n", i);

	}

	while(1);

#endif
 	while(1);

	/* Turn off SPI module and clear IF bit */
	CloseSPI1();
	CloseSPI2();

    while(1);
}

#endif

//******************************************************************************
//* Function Name:    Lcd_Initialize                                           *
//* Description:      Initalize the dsPIC30F6014 Development Board LCD using   *
//*                   using SPI2.                                              *
//*                                                                            *
//******************************************************************************
void Lcd_Initialize(void)
{
	int i,j;									// Local Loop Index variables
	unsigned int spi2cfg_1,spi2cfg_2,x,y,z;		// SPI Configuration variables
	unsigned int baud,uartmode,uartsta;
	unsigned int SPICONValue;					// Holds the information about SPI configuartion
	unsigned int SPISTATValue;					// Holds the information about SPI Enable/Disable
/* 
#ifdef UART_SUPPORT
	//**************************************************************************
 	//* Configure the USART 
	//**************************************************************************

	// Configure the UART here ...
 
	// Compute the baud rate constant needed to configure UART1 
 	baud = (((THE_CLOCK_SPEED/THE_BAUD_RATE) /16) - 1);

	uartmode = 	UART_EN & 
				UART_IDLE_CON & 
				UART_DIS_WAKE & 
				UART_DIS_LOOPBACK & 
				UART_DIS_ABAUD & 
				UART_NO_PAR_8BIT & 
				UART_1STOPBIT;   

	// Note that UART1 is Port B on the dsPIC Development Board.
	//uartmode = UART_EN & UART_NO_PAR_8BIT & UART_1STOPBIT;

	uartsta = UART_TX_ENABLE & UART_TX_PIN_NORMAL;
    OpenUART1(uartmode,uartsta,baud);
    //OpenUART2(uartmode,uartsta,baud);
#endif
*/

 
	//**************************************************************************
 	//* Configure the SPI module for LCD and interprocessor communications here ...
	//**************************************************************************

	// Turn off SPI modules 
	CloseSPI1();
	CloseSPI2();

	// Initialize SPI2 for LCD here
	CORCONbits.PSV = 1;
    PSVPAG = 0;

    LATGbits.LATG9 = 1;
    TRISGbits.TRISG9 = 0;

	spi2cfg_1 = FRAME_ENABLE_OFF &
		FRAME_SYNC_OUTPUT &
	            ENABLE_SDO_PIN &
	            SPI_MODE16_OFF &
		        SPI_SMP_ON &
		        SPI_CKE_OFF &
	  	SLAVE_ENABLE_OFF &
		        CLK_POL_ACTIVE_HIGH &
		        MASTER_ENABLE_ON &
		SEC_PRESCAL_7_1 &
		        PRI_PRESCAL_64_1;
	spi2cfg_2 = SPI_ENABLE &
	            SPI_IDLE_CON &
	            SPI_RX_OVFLOW_CLR; 
	OpenSPI2(spi2cfg_1,spi2cfg_2); 

}

//******************************************************************************
//* Function Name:    Lcd_Send_Control                                         *
//* Description:      Write LCD Control byte to the LCD using SPI2             *
//*                                                                            *
//******************************************************************************
void Lcd_Send_Control(byte LCDControlByte)
{
    LATGbits.LATG9 = 1;
    LATGbits.LATG9 = 0;
    spi_in = ReadSPI2();
    WriteSPI2(LCDControlByte);
    while(!(DataRdySPI2()));
}

//******************************************************************************
//* Function Name:    Lcd_Send_Data                                            *
//* Description:      Write LCD data byte to the LCD using SPI2                *
//*                                                                            *
//******************************************************************************
void Lcd_Send_Data(byte LCDDataByte)
{
    LATGbits.LATG9 = 1;
    LATGbits.LATG9 = 0;
    spi_in = ReadSPI2();
    WriteSPI2(LCDDataByte);
    while(!(DataRdySPI2()));
}

// General LCD commands 

//******************************************************************************
//* Function Name:    Lcd_Reset                                                *
//* Description:      The Reset command resets the LCD display and             *
//*                   PIC18F242 to their power-up state.                       *
//*                                                                            *
//******************************************************************************
void Lcd_Reset(void)
{
	// Send Reset control byte to LCD Controller 
	Lcd_Send_Control(Reset);
}

//******************************************************************************
//*     Function Name:    Lcd_Home                                             *
//*     Description:      The Home command sets all coordinate variables to    *
//*                       their home values and leaves the display unchanged.  *
//*                                                                            *
//******************************************************************************
void Lcd_Home(void)
{
	// Send Home control byte to LCD Controller and home the cursor
	Lcd_Send_Control(Home);
}

//******************************************************************************
//*     Function Name:    Lcd_Clear                                            *
//*     Description:      Clear the LCD and home the cursor.                   *
//*                                                                            *
//******************************************************************************
void Lcd_Clear(void)
{
	// Send Home Clear control byte to LCD Controller to home and clear the cursor
	Lcd_Send_Control(HomeClear);
}

//******************************************************************************
//* Function Name:    Lcd_Scroll                                               *
//* Description:                                                               *
//* The Scroll command rolls the display in the vertical axis by the amount    *
//* ScrollY. The LCD data array consists of 32 lines of 122 pixels each. If    *
//* scrolling, value ScrollY is set to zero and the top row of the data array  *
//* is displayed on the top row of the display.          					   *
//* Field Form Value														   *
//* 	Opcode 0xA3															   *
//* 	Data 0 0 0 Y4 Y3 Y2 Y1 Y0 0-31										   *
//* Note: The Scroll command has no effect on the various coordinate systems.  *
//* It only changes the mapping of the data to the display (e.g., the top      *
//* character row would be rolled to the bottom row of the display surface     *
//* if ScrollY = 8 but its location in the data array is still at 0).          *
//*                                                                            *
//******************************************************************************
void Lcd_Scroll(byte ScrollY)
{
	// Send Scroll control byte to LCD Controller to scroll the LCD display
	Lcd_Send_Control(Scroll);

	// Send Line coordinate 0-31  
	Lcd_Send_Data(ScrollY);

}



//******************************************************************************
// CHARACTER COMMANDS
// The character commands treat the display like a 20-column, 4-row character display
// where the characters occupy 6 x 8 pixel elements. Note that there are 20 character
// locations on a row, each 6 pixels wide. This current implementation only uses 120
// pixels of the 122-pixel wide display. The spare two-pixel columns are on the right of the
// display and cannot be affected by the character commands. The controller has a fixed
// space 5 x 7 font set for the printable ASCII characters in range: 0x20 -> 0x7E. The
// controller also recognizes three special characters that do not affect the display.
// 1. CR: (0x0D) Increments ChrPosR.
// 2. LF: (0x0A) Sets ChrPosC = 0 and increments ChrPosR.
// 3. BS: (0x08) Decrements ChrPosC if ChrPosC > 0; it is non-destructive
//******************************************************************************

//******************************************************************************
//* Function Name:    Lcd_Set_Character_Position                               *
//* Description:                                                               *
//* The ChrPos command sets the character position to ChrPosC, ChrPosR. This   *
//* command has no effect on the display except for moving character cursor if *
//* it is turned on. 														   *
//* where:                                                                     *
//*  	ChrPosC 0 0 0 C4 C3 C2 C1 C0 Column 0-19                               *
//*  	ChrPosR 0 0 0 0 0 0 R1 R2 Row 0-3                                      *
//******************************************************************************
void Lcd_Set_Character_Position(byte ChrPosC, byte ChrPosR)
{
  
 	// Send ChrPos control byte to LCD Controller
	Lcd_Send_Control(ChrPos);

	// Send Column coordinate 0-19  
	Lcd_Send_Data( ChrPosC);

	// Send Row coordinate 0-3  
	Lcd_Send_Data(ChrPosR);
}


 
//******************************************************************************
//* Function Name:    Lcd_Increment_Character_Position                         *
//* Description:                                                               *
//* The ChrPosInc command increments the character position. This command has  *
//* no effect on the display except for moving the character cursor if it is   *
//* turned on.                                                                 *
//******************************************************************************
void Lcd_Increment_Character_Position(void)
{ 
 	// Send ChrPosInc control byte to LCD Controller
	Lcd_Send_Control(ChrPosInc);


}
 
//******************************************************************************
//* Function Name:    Lcd_Write_Character                                      *
//* Description:                                                               *
//* The WrtChr command sets ChrPos to (ChrPosC,ChrPosR), then writes ASCII     *
//* character to ChrPos.                                                       *
//******************************************************************************
void Lcd_Write_Character(byte ChrPosC, byte ChrPosR, char Ascii_Char)
{ 
 	// Send WrtChr control byte to LCD Controller
	Lcd_Send_Control(WrtChr);

	// Send Column coordinate 0-19  
	Lcd_Send_Data( ChrPosC);

	// Send Row coordinate 0-3  
	Lcd_Send_Data(ChrPosR);

	// Send data byte to LCD Controller
	Lcd_Send_Data(Ascii_Char);
}
 

//******************************************************************************
//* Function Name:    Lcd_Write_Character_Increment                            *
//* Description:                                                               *
//* The WrtChrInc command sets ChrPos to (ChrPosC,ChrPosR), writes an ASCII    *
//* character to ChrPos, and then increments ChrPos.                           *
//* Note: The ChrPos increment after writing is suppressed for the special     *
//* characters.                                                                *
//******************************************************************************
void Lcd_Write_Character_Increment(byte ChrPosC, byte ChrPosR, char Ascii_Char)
{ 
 	// Send WrtChrInc control byte to LCD Controller
	Lcd_Send_Control(WrtChrInc);

	// Send Column coordinate 0-19  
	Lcd_Send_Data( ChrPosC);

	// Send Row coordinate 0-3  
	Lcd_Send_Data(ChrPosR);

	// Send data byte to LCD Controller
	Lcd_Send_Data(Ascii_Char);

}
 

//******************************************************************************
//* Function Name:    Lcd_Write_Character_Next                                 *
//* Description:                                                               *
//* The WrtChrNext command writes an ASCII character to the current ChrPos,    *
//* then increments ChrPos.                                                    *
//******************************************************************************
void Lcd_Write_Character_Next(char Ascii_Char)
{ 
 	// Send WrtChrNext control byte to LCD Controller
	Lcd_Send_Control(WrtChrNext);

	// Send data byte to LCD Controller
	Lcd_Send_Data(Ascii_Char);
}


//******************************************************************************
//* Function Name:    Lcd_Clear_Row                                            *
//* Description:                                                               *
//* The ChrClearRow command clears the entire row ChrPosR (0-3) and leaves     *
//* ChrPosC = 0.                                                               *
//******************************************************************************
void Lcd_Clear_Row(byte ChrPosR)
{ 
 	// Send ChrClearRow control byte to LCD Controller
	Lcd_Send_Control(ChrClearRow);

	// Send Row coordinate 0-3  
	Lcd_Send_Data(ChrPosR);

}
 
//******************************************************************************
//* Function Name:    Lcd_Clear_End                                            *
//* Description:                                                               *
//* The ChrClearEol command clears row ChrPosR from the current location to the*
//* end of the line and leaves ChrPos unchanged.                               *
//******************************************************************************
void Lcd_Clear_End (void)
{ 
 	// Send ChrClearEol control byte to LCD Controller
	Lcd_Send_Control(ChrClearEol);
}
 
 

//******************************************************************************
//* Function Name:    Lcd_Cursor_Off                                           *
//* Description:                                                               *
//* The ChrCursorOff command turns off the character cursor.                   *                                                           *
//******************************************************************************
void Lcd_Cursor_Off(void)
{ 
 	// Send ChrCursorOff control byte to LCD Controller
	Lcd_Send_Control(ChrCursorOff);
}

//******************************************************************************
//* Function Name:    Lcd_Cursor_On                                            *
//* Description:                                                               *
//* The ChrCursorOn command turns on the character cursor at the current       *
//* ChrPos.                                                                    *
//******************************************************************************
void Lcd_Cursor_On(void)
{ 
 	// Send ChrCursorOn control byte to LCD Controller
	Lcd_Send_Control(ChrCursorOn);
}
 
//******************************************************************************
//* Function Name:    Lcd_Blink_Cursor                                         *
//* Description:                                                               *
//* The ChrCursorBlink command controls cursor blinking. If the time is set to *
//* zero, the cursor will not blink, else the cursor blinks with equal on and  *
//* off times, with the on time being given by the blink time.                 *
//* 	Blink Time 0 0 0 0 0 T2 T1 T0 (0-7) x 0.125 seconds                    *
//******************************************************************************
void Lcd_Blink_Cursor(byte Blink_Time)
{ 
 	// Send ChrCursorBlink control byte to LCD Controller
	Lcd_Send_Control(ChrCursorBlink);

	// Send blink time in milliseconds T2 T1 T0 (0-7) x 0.125 seconds
	Lcd_Send_Data(Blink_Time);
}


//******************************************************************************
//* Function Name:    Lcd_Display_Character                                    *
//* Description:      Write single Ascii character data byte to the LCD        *
//*                   using SPI2                                               *
//*                                                                            *
//******************************************************************************
void Lcd_Display_Character(char Ascii_Char)
{
 	// Send control byte to LCD Controller
	Lcd_Send_Control(WrtChrNext);

	// Send data byte to LCD Controller
	Lcd_Send_Data(Ascii_Char);

}
 
//  stdio equivalent functions  

//******************************************************************************
//* Function Name:    Lcd_Printf                                               *
//* Description:      LCD display equivalent to stdio printf.                  *
//*                                                                            *
//******************************************************************************
//int Lcd_Printf(const char *, ...)
int Lcd_Printf(const char *format, ...)
{
    va_list  arg;                // ARGUMENT POINTER                         
    int ret;
    int Length;                  // Buffer Length 
    char Buffer[STRING_SIZE];    // I/O Buffer 

    va_start( arg, format );

    ret = vsprintf( Buffer, format, arg );

    va_end( arg );

    Length = strlen(Buffer);

    /* Write output string to serial port */

    Buffer[Length] = '\0';

    Lcd_Puts(Buffer);

    return ret;

}

//******************************************************************************
//* Function Name:    Lcd_Puts                                                 *
//* Description:      LCD display equivalent to stdio puts.                    *                                                          *
//*                                                                            *
//******************************************************************************
void Lcd_Puts(char *s)
{
	int k;

    for (k=0; k<strlen(s); k++)
    {
		Lcd_Write_Character_Next(s[k]);
	}

}
                 
//******************************************************************************
//* Function Name:    Lcd_Hex2                                                 *
//* Description:      LCD equivalent to Parallax Stamp hex2 function that      *                 
//*                   displays a byte Value as a 2 digit hex character.        *                                          
//******************************************************************************
void Lcd_Hex2(byte Value)
{
//*********************** Missing Logic ***************************
/*
char sbuf[100], s[]="Print this string";
int x = 1, y;
char a = '\n';
y = sprintf(sbuf, "%s %d time%c", s, x, a);
printf("Number of characters printed to "
"string buffer = %d\n", y);
*/
}
        
//******************************************************************************
//* Function Name:    Lcd_Dec                                                  *
//* Description:      LCD equivalent to Parallax Stamp dec function that       *                 
//*                   displays an integer Value in ASCII format.               * 
//*                                                                            *
//******************************************************************************
void Lcd_Dec(int Value) 
{
//*********************** Missing Logic ***************************
/*
char sbuf[100], s[]="Print this string";
int x = 1, y;
char a = '\n';
y = sprintf(sbuf, "%s %d time%c", s, x, a);
printf("Number of characters printed to "
"string buffer = %d\n", y);
*/
}
         
//******************************************************************************
//* Function Name:    Lcd_Scroll_Left                                          *
//* Description:      Scrolls LCD screen left N positions.                     *
//*                                                                            *
//******************************************************************************
void Lcd_Scroll_Left(byte N)
{
//*********************** Missing Logic ***************************
}        
    
//******************************************************************************
//* Function Name:    Lcd_Scroll_Right                                         *
//* Description:      Scrolls LCD screen right N positions.                    *
//*                                                                            *
//******************************************************************************
void Lcd_Scroll_Right(byte N)
{
//*********************** Missing Logic ***************************
}
     
//******************************************************************************
//* Function Name:    Lcd_Goto_XY                                              *
//* Description:      Go to LCD position (X, Y)                                *
//*                                                                            *
//******************************************************************************

void Lcd_Goto_XY(char X, char Y)
{
//*********************** Missing Logic ***************************
}
                         

 
//******************************************************************************
// PIXEL COMMANDS
// The pixel commands treat the display like a 122-column, 32-line set of pixels, which is
// the native resolution of the display. The pixel coordinate system differs from the others
// in having its origin at the lower left instead of at the upper left. Pixel coordinates are
// intended for graphs rather than letters or bitmap pictures.
//******************************************************************************
 
//******************************************************************************
//* Function Name:   Lcd_Pixel_Position                                        *
//* Description:     The Pixel_Position command sets PixPos to (PixPosX,PixPosY)*
//*                  and leaves the display unchanged. This command is         *
//*                  intended to be used in conjunction with the PixLine       *
//*                  command.                                                  *
//*                                                                            *
//******************************************************************************
void Lcd_Pixel_Position(byte PixPosX, byte PixPosY)
{
 	// Send Pixel Position control byte to LCD Controller
	Lcd_Send_Control(PixPos);

	// Send X coordinate
	Lcd_Send_Data(PixPosX);

	// Send Y coordinate
	Lcd_Send_Data(PixPosY);
}
 
//******************************************************************************
//* Function Name:    Lcd_Pixel_On                                             *
//* Description:      The Pixel_On command sets PixPos to (PixPosX,PixPosY)    *
//*                   and turns on the pixel at that location. This            *
//*                   command does not increment PixPos.                       *
//*                                                                            *
//******************************************************************************
void Lcd_Pixel_On(byte PixPosX, byte PixPosY)
{
 	// Send Pixel On control byte to LCD Controller
	Lcd_Send_Control(PixOn);

	// Send X coordinate
	Lcd_Send_Data(PixPosX);

	// Send Y coordinate
	Lcd_Send_Data(PixPosY);
}
 

//******************************************************************************
//* Function Name:    Lcd_Pixel_Off                                            *
//* Description:      The Pixel_Off   command sets PixPos to (PixPosX,PixPosY) *
//*                   and turns off the pixel at that location. This           *
//*                   command does not increment PixPos.                       *
//*                                                                            *
//******************************************************************************
void Pixel_Off (byte PixPosX, byte PixPosY) 
{
 	// Send control byte to LCD Controller
	Lcd_Send_Control(PixOff);

	// Send X coordinate
	Lcd_Send_Data(PixPosX);

	// Send Y coordinate
	Lcd_Send_Data(PixPosY);

}


//******************************************************************************
//* Function Name:    Lcd_Draw_Line_Rel                                        *
//* Description:      The  Draw_Line_Rel command draws a straight line from the*
//*                   current PixPos to the specified location and leaves      *
//*                   PixPos set to the new location.                          *
//*                                                                            *
//******************************************************************************
void Lcd_Draw_Line_Rel(byte PixPosX, byte PixPosY)
{
 	// Send Pixel Line control byte to LCD Controller
	Lcd_Send_Control(PixLine);

	// Send X coordinate
	Lcd_Send_Data(PixPosX);

	// Send Y coordinate
	Lcd_Send_Data(PixPosY);
}

//******************************************************************************
//* Function Name:    Lcd_Draw_Line_Abs                                        *
//* Description:      The Draw_Line_Abs command draws a straight line from the *
//*                   starting point (PixPosX1, PixPosY)  to the end point     *
//*                   (PixPosX2, PixPos2) and leaves PixPos set to the new     * 
//*                   location.                                                *
//*                                                                            *
//******************************************************************************
void Lcd_Draw_Line_Abs(byte PixPosX1, byte PixPosY1, byte PixPosX2, byte PixPosY2)
{

	// Set pixel position to first coordinate 
	Lcd_Pixel_Position(PixPosX1, PixPosY1);

 	// Send Pixel Line control byte to LCD Controller
	Lcd_Send_Control(PixLine);

	// Send X coordinate
	Lcd_Send_Data(PixPosX2);

	// Send Y coordinate
	Lcd_Send_Data(PixPosY2);

}
