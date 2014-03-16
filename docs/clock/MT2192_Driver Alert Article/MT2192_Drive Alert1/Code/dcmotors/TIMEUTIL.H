//********************************************************************************
//*        Name:  timeutil.h                                                     *
//* Project Number:  MT2192                                                      *                                                   
//*           Date:  10/15/2007                                                  *                                                  
//*                                                                              *
//*   Description:                                                               * 
//*     This application tests the DSPIC30FXXXX Timer Code by running a series   *
//* of PIC timer tests.  The first test implements a general purpose stop watch  *
//* using TIMER0, which can be used to time events or profile code. It allows    *
//* allows the operator to push a button connected to the PortB RB3 pin,         *
//* and then reports the elapsed time. The timer interrupt test uses Timers 0..4 *
//* to generate interrupts and blink an LED corresponding to each timer at       *
//* specific frequencies. Finally a simple clock function allows the operator    *
//* to set the current time,  and proceeds to display the time in HH:MM:SS       *
//* format.  The base internal clock frequency set to 1.843200 MHz and           *
//* multiplied by the PLL factor of (4x, 8x, or 16x).                            *                                                                              *
//*     The system clock frequency is assumed to be one of the following values  * 
//* set in the flags.h file:                                                     *
//* 		XT_4X_PLL = (7.3728 MIPS)      @ 1.843200 MHz  Internal Crystal      *
//* 		XT_8X_PLL = (14.7456 MIPS)     @ 1.843200 MHz  Internal Crystal      *
//* 		XT_16X_PLL = (29.4912 MIPS)    @ 1.843200 MHz  Internal Crystal      *
//*                                                                              *
//* 		EC_16X_PLL = (24 MIPS)         @ 6 MHz External Oscillator           *
//*                                                                              *
//* The Timer clock frequency = fc/4 MHz. Timer 0 is configured to count down the*
//* total number of milli-seconds, using a prescalar value of 1:256. Timer 1  is *
//* configured to count down the total number of micro-seconds, using a          *
//* prescalar value of 1:8. Timers 2/3 are used in the 32-Bit mode to obtain high*
//* resolution timing functions required for accurate benchmarking and precise   *
//* delays.                                                                      *  
//*                                                                              *
//* 	The following Stamp and general timing functions are tested by this      *
//* application: rctime, pause, sleep, nap, Delay_ms, Delay_us, Delay_s.         *
//*                                                                              *
//*      Future upgrades to this application include: Implementing the Stamp     *
//* BS2 RCTIME function using TIMER0;  using the PIC high resolution timers for  *
//* remote control by transmitting Sony IR Remote codes to appliances;           *
//* implementing a task scheduler for simple state machine processes using       *
//* the timers;  building an X-10 Timer module.                                  *
//*                                                                              *
//*                                                                              *
//********************************************************************************
  
 
// Include file headers here

#ifndef TIMEUTIL_LOADED  /* prevent multiple loading */
#define TIMEUTIL_LOADED

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
 
#include <stdio.h>
#include <dsp.h>
#include <ports.h>						// Include support for ports
#include <uart.h> 						// UART library functions
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <timer.h>                                     // Timer library functions                                                                 
//#include <delays.h>                                     // High resolution delay functions
#include "timeutil.h"                                   // Timer library functions                               
//#include "portutil.h"                                   // My own I/O Port library functions
#include "my_types.h"					// General C/C++ types
//#include "lcd.h"						// LCD Support using SPI2 

// Port A Bits
  
#define right_LDRpin           PORTAbits.RA2      // Right light sensor
#define left_LDRpin	      	   PORTAbits.RA3      // Left light sensor or


#define WriteTimer WriteTimer1_16                       // Needed because of the Microchip C library bug (wrong name)

 // Timer System dependent constants

#define SYSTEM_FREQUENCY  20                            // System frequency, Fs = 20 MHz 
 
//#define TIME_SCALE_FACTOR 10                          // Timer scale factor used for DelayUs (10 micro-seconds granularity)
#define TIME_SCALE_FACTOR 100                           // Timer scale factor used for DelayUs (100 micro-seconds granularity)
 
// Compute timer clock frequency which depends on the system frequency.  This constant is 
// used by DelayUs function. 

#define CLOCK_FREQUENCY   (SYSTEM_FREQUENCY / 4)     //  Fc = Fs / 4 
 
//********************************************************************************
// Variable declarations
//********************************************************************************

//#ifdef PIC18C452
//extern const rom char  ready[] = "\n\rREADY>";
//extern const rom char  error[] = "\n\rERROR!";

//#else
//extern const char  ready[] = "\n\rREADY>";
//extern const char  error[] = "\n\rERROR!";
extern const char  ready[] ;
extern const char  error[] ;
//#endif


//extern char            CR[2];                                     // Carriage return
 // character
//extern char            LF[2];                                     // Line feed character

extern char            inpbuf[8];                                 // Input command buffer
 // (USART)
extern char            str[7];                                    // Temporary string
extern char            blanks[5];                                 // Blank delimiter field

extern unsigned int             TimerCount0;                           // Timer interrupt 0 count
extern unsigned int             TimerCount1;                           // Timer interrupt 1 count
extern unsigned int             TimerCount2;                           // Timer interrupt 2 count
extern unsigned int             TimerCount3;                           // Timer interrupt 3 count

extern int             result;
 
  
//********************************************************************************
// Function Prototypes
//********************************************************************************


void            SetupTimers(void);
void            SetupInterrupts(void);                     // Configures timer interrupts and general interrupts
 
void            ProcessTimer0(void);                       // Process Timer 0 interrupt
void            ProcessTimer1(void);                       // Process Timer 1 interrupt
void            ProcessTimer2(void);                       // Process Timer 2 interrupt
void            ProcessTimer3(void);                       // Process Timer 3 interrupt
void            ProcessTimer4(void);                       // Process Timer 4 interrupt
void            ProcessTimer5(void);                       // Process Timer 5 interrupt
 

void            DelayUs(unsigned int n);                            // Delay for n micro-seconds
void            DelayMs(unsigned int n);                            // Delay for n milli-seconds
void            DelayS(unsigned int n);                             // Delay for n seconds

void            Stopwatch(void);                           // Stopwatch function
void            Beep(int n);                               // Beep function

// Stamp BS2/BSX Compatible functions

void            pause(unsigned int n);                              // Pause for n units of 1000 instruction
 							   // cycles (milli-seconds at 40 MHz)
void            Sleep1(int n);                             // Sleep for n units of 2.3 seconds
void            nap(int n);                                // Nap for a short duration of 2**Period * 18 milli-seconds
void            rctime(byte Pin, byte State, word *Count);  // RCTIME function

void            Sound(int n);                              // Sound function


// Timer tests
void            GetTime(void);                             // Get the time and send it to the host
void            SetTime(void);                             // Set the time sent from host

void            PauseTest(void);                           // Pause test
void            DelayTest(void);                           // Delay test
void            StopwatchTest(void);                       // Stopwatch test
void            ClockTest(void);                           // Realtime clock test
void            TimerInterruptTest(void);                  // Timer interrupt test

// Writes a string from ROM to the USART

//void            putrsUSART(const rom char *data);


 
 

#endif

