//*********************************************************************************
//*                                                                               *
//*           Name:  viport.c                                                     *
//* Project Number:  MT2192                                                       *                                                   
//*           Date:  10/15/2007                                                   *                                                  
//*                                                                               *
//*   Description:                                                                * 
//*   	This is a DSPIC utility  which will implement the following               * 
//* "C" versions of the BS2 Stamp functions: high, low, toggle, input, output,    *
//* pulsin, pulsout, shiftin, shiftout.  For test purposes, switches are          *
//* connected to Port D in order select each pin's state.  It also provides       *
//* two 8-Bit virtual ports for the DSPIC30F2010, Virtual PORT B and PORT E.      *              
//*                                                                               * 
//*     Future upgrades to this application include: Implementing the Stamp       *
//* BS2 functions: pwm, dtmfout and freqout and diagnostics functions.            *
//*                                                                               *
//* Note:                                                                         *
//* 	1) The names of all BS2 equivalent functions use lower case.              *
//*                                                                               *
//*     2) The following BS2 to eZ80F91 I/O pin map is used by most of the        *
//*        Port Utility functions:                                                *                                                                                                             *
//*                                                                               *
//*       BS2        PIC        PIC           PIC                                 *
//*       PIN        PIN        PORT          BIT                                 *
//*     ---------------------------------------------                             *
//*         0         2           0             0                                 *
//*         1         3           0             1                                 *
//*         2         4           0             2                                 *
//*         3         5        A  0             3                                 *
//*         4         6           0             4                                 *
//*         5         7           0             5                                 *
//*         6        14           0             6                                 *
//*     ---------------------------------------------                             *
//*         7        33           1             0                                 *
//*         8        34           1             1                                 *
//*         9        35           1             2                                 *
//*        10        36           1             3                                 *
//*        11        37        B  1             4                                 *
//*        12        38           1             5                                 *
//*        13        39           1             6                                 *
//*        14        40           1             7                                 *
//*     ---------------------------------------------                             *
//*        15        15           2             0                                 *
//*        16        16           2             1                                 *
//*        17        17           2             2                                 *
//*        18        18           2             3                                 *
//*        19        23        C  2             4                                 *
//*        20        24           2             5                                 *
//*        21        25           2             6                                 *
//*        22        26           2             7                                 *
//*     ---------------------------------------------                             *
//*        23        19           3             0                                 *
//*        24        20           3             1                                 *
//*        25        21           3             2                                 *
//*        26        22           3             3                                 *
//*        27        27        D  3             4                                 *
//*        28        28           3             5                                 *
//*        29        29           3             6                                 *
//*        30        30           3             7                                 *
//*     ---------------------------------------------                             *
//*        31         8           4             0                                 *
//*        32         9        E  4             1                                 *
//*        33        10           4             2                                 *
//*     ---------------------------------------------                             *
//*                                                                               *
//*                                                                               *
//*********************************************************************************
       
  
// Include file headers here
#include "flags.h"

#ifdef DSPIC30F2010     
#include <p30F2010.h>					// Register definitions for DSPIC30F2010
#endif

#ifdef DSPIC30F4011     
#include <p30F4011.h>					// Register definitions for DSPIC30F4011
#endif


#ifdef DSPIC30F6014     
#include <p30F6014.h>					// Register definitions for DSPIC30F6014
#endif
 
  
//#include <stdarg.h>
//#include <ctype.h>
#include <string.h>
#include <uart.h>
#include <ports.h>
//#include <stdio.h>
#include "my_types.h"
#include "viport.h"                                   // Virtual Port functions
#include "timeutil.h"                                   // Timer utility functions
#include "stdio.h"                                      // I/O support functions
 
//********************************************************************************
// Constant Definitions
//********************************************************************************

//#define TEST                                          // Flag used for debug I/O
//#define DIAGNOSTICS									// Flag used to include diagnostic menu
//#define BIT_TEST										// Bit Functions Test 
#define PORT_FUNCTIONS_TEST								// Port Functions Test 

#define TRUE  1                                         // Boolean TRUE value
#define FALSE 0                                         // Boolean FALSE value



 
 
//********************************************************************************
// Constant Definitions
//********************************************************************************

//#define PORT_FUNCTIONS_TEST                             	// Test port functions
#define BIT_TEST                                        	// Test bit operations
//#define PORT_DISPLAY_TEST                               		// Test port display
#define REGISTER_DISPLAY_TEST                           	// Test register display 
#define STATUS_DISPLAY_TEST											// Test Status register display     
#define DIAGNOSTICS                                        // Diagnostics 
//#define DISPLAY_SUPPORT                                     // Support for register display functions

#define SYSTEM_FREQUENCY 20         			// System frequency, Fs = 20 MHz

// Compute timer clock frequency which depends on the system frequency.  This constant is 
// used by DelayUs function. 

#define CLOCK_FREQUENCY   (SYSTEM_FREQUENCY / 4)     	//  Fc = Fs / 4 
 
#define  SCALE_FACTOR 10      			// 10 micro-second granularity for delays

  
#define T_2_Us     1  // 1 = (2  /  SCALE_FACTOR * CLOCK_FREQUENCY) =  2 micro-second delay
 

#ifdef VIRTUAL_PORT_TEST

//******************************************************************************
//* main - main function
//****************************************************************************** 
int main(void)
{
	int i,j; 						// Loop index variables
   word PinID, PortID;			// Pin number and port (A=0..E=4)
   word ProcessorFlag = 0;		// Processor flag (PIC=0, BS2 = 1)
   char Command;              // Menu command
   word Result;               // Data read from pulsin and shiftin

	VR_8_BIT_PORT_T VR_Port_Data;	// Virtual port data (8-Bits)
   
   //static word TheValue = 255;

	// Note: Avoid modifiying PORT C which is used by the USART !

	// Initialize Port Utility variables

	//InitializePorts();

 	// Configure all analog pins as digital

	ADPCFG = 0xFFFF;

   // Initialize 16-Bit Digital I/O ports (PORTE does not exist for the DSPIC30FXXXX)

    PORTB = 0x0000;                                          // Clear PORTB
    PORTE = 0x0000;                                          // Clear PORTE
 
    // Initialize port I/O direction

    TRISB = 0x000F;                                          // PORT B all inputs and outputs.
    TRISE = 0x0000;                                          // PORT E all outputs.
 
  // Configure PORT interrupts with Interrupt on change off and CN pull-up resistors disabled

	//ConfigCNPullups(0x0000);					// Disable CN pin pullups (inputs)  
	//ConfigCNPullups(0xFFF);					// Enable CN pin pullups (inputs) 

    // Setup timers, configure ports and initialize variables

    SetupTimers();  
 
	for (i=0; i<256; i++)
	{
		VR_Port_Data.Port_Value = i;
	 	Write_Virtual_Port(PORT_E, VR_Port_Data);
		pause(500);
	}

 
}
#endif

#ifdef NEW_CODE 
 		
//******************************************************************************
//* DisplayPort - Display the contents of the selected PIC I/O port.
//****************************************************************************** 

void DisplayPort(word PortID)
{
	word PortValue;                    	// Contents of selected port 
   
	// Read the selected port 

	PortValue =   *PortAddress[PortID];

	// Display the port contents in hex

   printf(PortName[PortID]);
  	printf("  = ");
   hex2(PortValue);
   printf("\r\n");

}

//******************************************************************************
//* DisplayDirPort - Display the contents of the selected PIC I/O port direction 
//* register (TRISA..TRISE).
//****************************************************************************** 

void DisplayDirPort(word PortID)
{
	word PortValue;                    	// Contents of selected port 
   
	// Read the selected port 

	PortValue =   *PortDirAddress[PortID];

	// Display the port contents in hex

   printf(PortDirName[PortID]);
  	printf("  = ");
   hex2(PortValue);
   printf("\r\n");

}

#endif 

 	 		
//******************************************************************************
//* Read_Virtual_Port - Read a byte of data from the selected Port,  
//* Port_ID range (PORT_A..PORTE).  It reads the bits from the
//* assigned ports and saves them to Virtual Port B, bits B(0:5), Port E bits(4:5) or
//* for Virtual Port E: bits (0:5), Port B bits (4:5)the virtual data byte
//******************************************************************************

VR_8_BIT_PORT_T Read_Virtual_Port(byte Port_ID)
{
	VR_8_BIT_PORT_T The_Virtual_Port;

	switch(Port_ID)
	{
		case PORT_B:
		{
	 		The_Virtual_Port.Port_Value = PORTB;
            The_Virtual_Port.VR_BIT_4 = PORTEbits.RE4;
		 	The_Virtual_Port.VR_BIT_5 = PORTEbits.RE5;
			break;
		}

		case PORT_E:
		{
			The_Virtual_Port.Port_Value = PORTE;
            The_Virtual_Port.VR_BIT_4 = PORTBbits.RB4;
			The_Virtual_Port.VR_BIT_5 = PORTBbits.RB5;
			break;
		}

	}
	return The_Virtual_Port;	
}


//******************************************************************************
//* Write_Virtual_Port - Write a byte of data from the selected port, 
//* Port_ID range (PORT_A..PORTE). It Distributes the bits from the virtual data byte to the
//* assigned for Virtual Port B, bits B(0:5), Port E bits(4:5) or
//* for Virtual Port E: bits (0:5), Port B bits (4:5)
//******************************************************************************
void Write_Virtual_Port(byte Port_ID, VR_8_BIT_PORT_T The_Data)
{
	// Distribute the bits from the virtual data byte to the
	// assigned for Virtual Port B, bits B(0:5), Port E bits(4:5) or
	// for Virtual Port E: bits (0:5), Port B bits (4:5)

	switch(Port_ID)
	{
		case PORT_B:
		{
			PORTB = The_Data.Port_Value;
            PORTEbits.RE4 = The_Data.VR_BIT_4;
			PORTEbits.RE5 = The_Data.VR_BIT_5;
			break;
		}

		case PORT_E:
		{
			PORTE = The_Data.Port_Value;
            PORTBbits.RB4 = The_Data.VR_BIT_4;
			PORTBbits.RB5 = The_Data.VR_BIT_5;
			break;
		}

	}
}
