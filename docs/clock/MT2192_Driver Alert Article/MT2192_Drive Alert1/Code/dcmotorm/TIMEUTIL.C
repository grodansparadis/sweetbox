//********************************************************************************
//*        Name:  timeutil.c                                                     *
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
//* 		XT_4X_PLL = (7.3728 MIPS)                                            *
//* 		XT_8X_PLL = (14.7456 MIPS)                                           *
//* 		XT_16X_PLL = (29.4912 MIPS)                                          *
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
#include "uartutil.h"                          // UART support functions
#include "my_types.h"					// General C/C++ types
//#include "lcd.h"						// LCD Support using SPI2 


 
//********************************************************************************
// Constant Definitions
//********************************************************************************
 
//#define USE_TIMER_1									// Configure and use timer 1
//#define USE_TIMER_2									// Configure and use timer 2
//#define USE_TIMER_3									// Configure and use timer 3
//#define USE_TIMER_4									// Configure and use timer 4
//#define USE_TIMER_5									// Configure and use timer 5

#define  USE_TIMER_2_AND_3								// Use TIMER 2/3 as the 32-Bit timer (works)
//#define  USE_TIMER_4_AND_5 								// Use TIMER 4/5 as the 32-Bit time (works)

#define TIMER_INTERRUPTS								// Configure and enable timer interrupts
 
//#define ADVANCED_TIME_FUNCTIONS						// Include support for advanced time utility functions
                                                        // including RCTIME and a Stopwatch function. 

//#define TIMER_TEST_1   								// Uses 32-bit timers 2/3 and 4/5 for preiodic interrupts (1 sec, 5 sec)  (works!!!)                                 
//#define TIMER_TEST_2   									// 5 second pause test   (works!!!)                             
//#define DELAY_TEST                                    // Select the Delay function test  
#define PAUSE_TEST                                    	// Select the Pause function test
//#define STOPWATCH_TEST                                // Select the stop watch function test (works)
//#define CLOCK_TEST                                    // Select realtime clock 
//#define TIMER_INTERRUPTS                              	// Select the timer interrupt test (works)
//#define RCTIME_TEST   				                // Select the RCTIME function test                               
//#define RCTIME_FLAG                                   // If defined species RC circuits otherwise CR circuits
                                                        // are assumed to be in operation

// Define 2's complement timer constants for various delays for 16-Bit timers 1..5 
// and 32-Bit timer 2/3 and 32-Bit timer 4/5.

#ifdef 		XT_4X_PLL 									//XT 4x PLL = (7.3728 MIPS)                                    

// 16 Bit timers 1..5
#define TIMER_16_COUNT_1S 	   	28800					// 1 second count period for 16-Bit timer period register with 1:256 Prescalar
#define TIMER_16_COUNT_1MS   	28						// 1 milli-second count for 16-Bit timer period register with 1:256 Prescalar
#define TIMER_16_COUNT_US 	   	1						// Not Possible

// 32 Bit timers 2/3 and 4/5
#define TIMER_32_COUNT_1S 	   	7372800					// 1 second count period for 32-Bit timer period register with 1:1 Prescalar
#define TIMER_32_COUNT_1MS   	7372					// 1 milli-second count for 32-Bit timer period register with 1:1 Prescalar
#define TIMER_32_COUNT_US 	   	7 						// 1 micro-second count for 32-Bit timer period register with 1:1 Prescalar

#endif

#ifdef 		XT_8X_PLL 									//XT 8x PLL = (14.7456 MIPS)                          

// 16 Bit timers 1..5
#define TIMER_16_COUNT_1S 	   	57597					// 1 second count period for 16-Bit timer period register with 1:256 Prescalar
#define TIMER_16_COUNT_1MS   	57						// 1 milli-second count for 16-Bit timer period register with 1:256 Prescalar
#define TIMER_16_COUNT_US 	   	1						// Not possible  

// 32 Bit timers 2/3 and 4/5
#define TIMER_32_COUNT_1S 	   	14745600				// 1 second count period for 32-Bit timer period register with 1:1 Prescalar
#define TIMER_32_COUNT_1MS   	14745					// 1 milli-second count for 32-Bit timer period register with 1:1 Prescalar
#define TIMER_32_COUNT_US 	   	14						// 1 micro-second count for 32-Bit timer period register with 1:1 Prescalar

#endif

#ifdef 		XT_16X_PLL 									//XT 16x PLL = (29.4912 MIPS)                             

// 16 Bit timers 1..5
#define TIMER_16_COUNT_1S 	   	1		 				// Not possible 
#define TIMER_16_COUNT_1MS   	1 						// Not possible 
#define TIMER_16_COUNT_US 	   	1						// Not possible 

// 32 Bit timers 2/3 and 4/5
#define TIMER_32_COUNT_1S 	   	29491200				// 1 second count for 32-Bit timer period register  with 1:1 Prescalar
#define TIMER_32_COUNT_1MS   	29491					// 1 milli-second count for 32-Bit timer period register with 1:1 Prescalar
#define TIMER_32_COUNT_US 	   	29						// 1 micro-second count for 32-Bit timer period register with 1:1 Prescalar

 
#endif

// Port A Bits

 
// Port E Bits
  
   
 
//********************************************************************************
// Variable declarations
//********************************************************************************


// Timer general variables


byte Command;
unsigned int T1_Match_Value = 0;								// 16-Bit Timer 1 match value
unsigned int T2_Match_Value = 0;								// 16-Bit Timer 2 match value
unsigned int T3_Match_Value = 0;								// 16-Bit Timer 3 match value
unsigned int T4_Match_Value = 0;								// 16-Bit Timer 4 match value
unsigned int T5_Match_Value = 0;								// 16-Bit Timer 5 match value

unsigned long int T23_Match_Value = 0;							// 32-Bit Timer 2/3 match value
unsigned long int T45_Match_Value = 0;							// 32-Bit Timer 4/5 match value


#ifdef TIMER_INTERRUPTS

volatile unsigned long int             TimerCount0 = 0;                  // Timer interrupt 0 count (Seconds)
volatile unsigned long int             TimerCount1 = 0;                  // Timer interrupt 1 count (Seconds)
volatile unsigned long int             TimerCount2 = 0;                  // Timer interrupt 2 count (Milliseconds)
volatile unsigned long int             TimerCount3 = 0;                  // Timer interrupt 3 count 
volatile unsigned long int             TimerCount4 = 0;                  // Timer interrupt 4 count (Microseconds)
volatile unsigned long int             TimerCount5 = 0;                  // Timer interrupt 5 count

volatile unsigned int             Hours = 0;                        // Hours counter
volatile unsigned int             Minutes = 0;                      // Minutes counter
volatile unsigned int             Seconds = 0;                      // Seconds counter

#endif


int             result;


extern byte Command;					// Timer command read from host (S=set time, G = get time)
extern char Ch; 						// Character read from USART
extern byte HexNumber[];					// Hex string

//********************************************************************************
// Declare function headers here
//********************************************************************************

int main(void);
#ifdef PAUSE_TEST

//********************************************************************************
//* pauseTest - Performs the pause function test.
//********************************************************************************
void            pauseTest(void);
#endif

#ifdef TIMER_INTERRUPTS

//********************************************************************************
// Interrupt Code
//********************************************************************************

// Designate ServiceInterrupt as an interrupt function and save key registers

unsigned int timer_value;

//********************************************************************************
//* _T1Interrupt - Processes timer 1 interrupt, serviced every milli-second for  
//* at a 1 KHz frequency or used for calendar, time and date functions.
//********************************************************************************
void __attribute__((__interrupt__)) _T1Interrupt(void)
{

/*
The DISI instruction can be used in a C program through in-line assembly. For
example, the in-line assembly statement:
__asm__ volatile ("disi #16");
will emit the specified DISI instruction at the point it appears in the source program. A
disadvantage of using DISI in this way is that the C programmer cannot always be sure
how the C compiler with translate C source to machine instructions, so it may be difficult
to determine the cycle count for the DISI instruction. It is possible to get around this
difficulty by bracketing the code that is to be protected from interrupts by DISI
instructions, the first of which sets the cycle count to the maximum value, and the
second of which sets the cycle count to zero. For example,
__asm__ volatile("disi #0x3FFF"); // disable interrupts
// ... protected C code ...
__asm__ volatile("disi #0x0000"); // enable interrupts 
An alternative approach is to write directly to the DISCNT register, which has the same
effect in hardware as the DISI instruction, but it has the advantage for the C
programmer that it avoids the use of in-line assembly. This is desirable because there
are certain optimizations that the compiler will not perform if in-line assembly is used in
a function. So, instead of the sequence shown above, use
DISICNT = 0x3FFF; // disable interrupts 
// ... protected C code ... 
DISICNT = 0x0000; // enable interrupts 
*/ 
 	// Disable interrupts

	//DISICNT = 0x3FFF;   

	// ... protected C code ... 
 
	// Process the Timer 1 interrupt
    ProcessTimer1();

	// Enable interrupts

//	DISICNT = 0x0000; 

	IFS0bits.T1IF = 0; // Clear Timer 1 interrupt flag 
}

//********************************************************************************
//* _T2Interrupt - Processes timer 2 interrupt, serviced every milli-second   
//*  
//********************************************************************************
void __attribute__((__interrupt__)) _T2Interrupt(void)
{
 	// Disable interrupts

	//DISICNT = 0x3FFF;   

 
	// Process the Timer 2 interrupt
    ProcessTimer2();
 
	// Enable interrupts

//	DISICNT = 0x0000; 

	IFS0bits.T2IF = 0; // Clear Timer 2 interrupt flag 


}
//********************************************************************************
//* _T3Interrupt - Processes timer 3 interrupt, serviced every milli-second   
//*  
//********************************************************************************
void __attribute__((__interrupt__)) _T3Interrupt(void)
{
 	// Disable interrupts

//	DISICNT = 0x3FFF;   

 
	// Process the Timer 3 interrupt
   ProcessTimer3();
  
	// Enable interrupts

//	DISICNT = 0x0000; 

	IFS0bits.T3IF = 0; // Clear Timer 3 interrupt flag 
}

//********************************************************************************
//* _T4Interrupt - Processes timer 4 interrupt, serviced every milli-second   
//*  
//********************************************************************************
void __attribute__((__interrupt__)) _T4Interrupt(void)
{
 	// Disable interrupts

//	DISICNT = 0x3FFF;   

 
	// Process the Timer 4 interrupt
    ProcessTimer4();
 
	// Enable interrupts

//	DISICNT = 0x0000; 

	IFS1bits.T4IF = 0; // Clear Timer 4 interrupt flag 
}
//********************************************************************************
//* _T5Interrupt - Processes timer 5 interrupt, serviced every milli-second   
//*  
//********************************************************************************
void __attribute__((__interrupt__)) _T5Interrupt(void)
{
 	// Disable interrupts

//	DISICNT = 0x3FFF;   

 
	// Process the Timer 5 interrupt
    ProcessTimer5();
 
	// Enable interrupts

//	DISICNT = 0x0000; 
	IFS1bits.T5IF = 0; // Clear Timer 5 interrupt flag
}

#endif

//********************************************************************************
//* SetupTimers - Initializes program variables and peripheral registers
//* the timer interrupt test code.
//********************************************************************************
void            SetupTimers(void)
{

#ifdef TIMER_INTERRUPTS


    Hours = 0;                                             // Reset hours counter
    Minutes = 0;                                           // Reset minutes counter
    Seconds = 0;                                           // Reset seconds counter

    TimerCount0 = 0;                                       // Reset Timer Interrupt 0 count (Elapsed seconds)
    TimerCount1 = 0;                                       // Reset Timer Interrupt 1 count (Elapsed milli-seconds)
    TimerCount2 = 0;                                       // Reset Timer Interrupt 2 count
    TimerCount3 = 0;                                       // Reset Timer Interrupt 3 count
    TimerCount4 = 0;                                       // Reset Timer Interrupt 4 count
    TimerCount5 = 0;                                       // Reset Timer Interrupt 5 count

 
#endif

    // configure port A for all digital inputs

    //OpenADC(ADC_FOSC_32 &
    //        ADC_RIGHT_JUST & ADC_0ANA_0REF, ADC_CH0 & ADC_CH1 & ADC_CH2 & ADC_CH3 & ADC_CH4 & ADC_CH5 & ADC_CH6 & ADC_CH7 & ADC_INT_OFF);
    //OpenADC(ADC_0ANA_0REF, ADC_INT_OFF);

   // Configure PORT interrupts with Interrupt on change off and CN pull-up resistors disabled

	ConfigCNPullups(0x0000);					// Disable CN pin pullups (inputs) 
	//ConfigCNPullups(0xFFF);					// Enable CN pin pullups (inputs)

 

    // Clear port D 
    PORTD = 0xFF;                                          // Turn off LEDs (D0..D4)
   
    // Initialize port D I/O direction bits
    TRISD = 0x00;                                          // PORTD all outputs for LEDs (D0..D4)
  
    // Clear timer registers


    // Turn LEDs OFF (Off = 1)

    LED_1 = OFF;
    LED_2 = OFF;
    LED_3 = OFF;
    LED_4 = OFF;

     // turn on LEDs (On = 0)

    LED_1 = ON;
    LED_2 = ON;
    LED_3 = ON;
    LED_4 = ON;
  

    // Turn LEDs OFF (Off = 1)

    LED_1 = OFF;
    LED_2 = OFF;
    LED_3 = OFF;
    LED_4 = OFF;
  

#ifdef USE_TIMER_1

	// Configure 16-Bit Timer 1 here with no interrupts
    // Use timer 1 for realtime clock time stamp and calendar functions  
  
	ConfigIntTimer1(T1_INT_PRIOR_1);

	// Clear 16-Bit timer 1 (Type A timer)
    WriteTimer1(0);

	// Setup Timer 1 for 1 millisecond delay

	//T1_Match_Value = TIMER_16_COUNT_1MS;
	T1_Match_Value = TIMER_16_COUNT_1S;

	OpenTimer1(T1_ON & T1_GATE_OFF & T1_IDLE_STOP & T1_PS_1_1 & T1_SOURCE_INT, T1_Match_Value);  

#endif

#ifdef USE_TIMER_2

	// Configure 16-Bit Timer 2 here with no interrupts
    // Use timer 2 for BS2 pause function (Type B timer)
  
	ConfigIntTimer2(T2_INT_PRIOR_2);

	// Clear 16-Bit Timer 2 
    WriteTimer2(0);

	// Setup Timer 2 for 1 millisecond delay

	//T2_Match_Value = TIMER_16_COUNT_1MS;
	T2_Match_Value = TIMER_16_COUNT_1S;

	OpenTimer2(T2_ON & T2_GATE_OFF & T2_IDLE_STOP & T2_PS_1_1 & T2_SOURCE_INT, T2_Match_Value);  

#endif

#ifdef USE_TIMER_3

	// Configure 16-Bit Timer 3 here with no interrupts
    // Use Timer 3 for BS2 pause function
  
	ConfigIntTimer3(T3_INT_PRIOR_3);

	// Clear 16-Bit Timer 3
    WriteTimer3(0);

	// Setup Timer 3 for 1 millisecond delay

	//T3_Match_Value = TIMER_16_COUNT_1MS;
	T3_Match_Value = TIMER_16_COUNT_1S;

	OpenTimer3(T3_ON & T3_GATE_OFF & T3_IDLE_STOP & T3_PS_1_1 & T3_SOURCE_INT, T3_Match_Value);  

#endif

#ifdef USE_TIMER_4

	// Configure 16-Bit Timer 4 here with no interrupts
    // Use Timer 4 for BS2 pause function
  
	ConfigIntTimer4(T4_INT_PRIOR_4);

	// Clear 16-Bit timer 4
    WriteTimer4(0);

	// Setup Timer 4 for 1 millisecond delay

	//T4_Match_Value = TIMER_16_COUNT_1MS;
	T4_Match_Value = TIMER_16_COUNT_1S;

	OpenTimer4(T4_ON & T4_GATE_OFF & T4_IDLE_STOP & T4_PS_1_1 & T4_SOURCE_INT, T4_Match_Value);  

#endif

#ifdef USE_TIMER_5

	// Configure 16-Bit Timer 5 here with no interrupts
    // Use Timer 5 for BS2 pause function
  
	ConfigIntTimer5(T5_INT_PRIOR_5);

	// Clear 16-Bit Timer 5
    WriteTimer5(0);

	// Setup Timer 5 for 1 millisecond delay

	//T5_Match_Value = TIMER_16_COUNT_1MS;
	T5_Match_Value = TIMER_16_COUNT_1S;

	OpenTimer5(T5_ON & T5_GATE_OFF & T5_IDLE_STOP & T5_PS_1_1 & T5_SOURCE_INT, T5_Match_Value);  

#endif

#ifdef USE_TIMER_2_AND_3

 	// Configure 32-Bit Timer 2/3 here with no interrupts

	ConfigIntTimer23(T3_INT_PRIOR_3);
 
	// Clear 32-Bit timer 2/3
    WriteTimer23(0);

	// Setup Timer 2/3 for 1 millisecond delay

	T23_Match_Value = TIMER_32_COUNT_1MS;

	// Setup Timer 2/3 for 1 second delay (works!!)

	//T23_Match_Value = TIMER_32_COUNT_1S;

	// Setup Timer 2/3 for 5 second delay (works!!)

	//T23_Match_Value = TIMER_32_COUNT_1S * 5;

	OpenTimer23(T2_ON & T2_GATE_OFF & T2_IDLE_STOP & T2_PS_1_1 & T2_SOURCE_INT, T23_Match_Value);  

  
#endif

#ifdef USE_TIMER_4_AND_5

 	// Configure 32-Bit Timer 4/5 here with no interrupts
 
	ConfigIntTimer45(T5_INT_PRIOR_5);

	// Clear 32-Bit timer 4/5
    WriteTimer45(0);

	// Setup Timer 4/5 for 1 millisecond delay

	T45_Match_Value = TIMER_32_COUNT_1MS;

	// Setup Timer 4/5 for 1 second delay (works!!)

	//T45_Match_Value = TIMER_32_COUNT_1S;

	// Setup Timer 2/3 for 5 second delay (works!!)
	//T45_Match_Value = TIMER_32_COUNT_1S * 5;

	OpenTimer45(T4_ON & T4_GATE_OFF & T4_IDLE_STOP & T4_PS_1_1 & T4_SOURCE_INT, T45_Match_Value);  

  
#endif

// Enable or disable timer interrupts

#ifdef TIMER_INTERRUPTS

	// Enable all timer interrupts here
 	EnableIntT1;
 	EnableIntT2;
 	EnableIntT3;
 	EnableIntT4;
 	EnableIntT5;

#else

	// Disable all timer interrupts here
 	DisableIntT1;
 	DisableIntT2;
 	DisableIntT3;
 	DisableIntT4;
 	DisableIntT5;


#endif

	//ConfigCNPullups(0x0);					// Disable CN pin pullups (inputs)
	//ConfigCNPullups(0xFFF);				// Enable CN pin pullups (inputs)
  
}
 

#ifdef TIMEUTIL_TEST

//********************************************************************************
//* main - Displays the latest interrupt counts from interrupts Timer0..Timer3.
//********************************************************************************

int main(void)
{
 
   // Setup timers, configure ports and initialize variables

   SetupTimers();  

   // Configure UART for selected baud rate with 8-Bits, No parity, Asynchronous
   
    InitializeUart();
       
#ifdef TIMER_TEST_1
       
	// Display timer interrupt counts here...  (This code works!!!)
    // Note: Timer Interrupts must be enabled using compilation flag                             
    while (1)
   {
        printf("TimerCount3 = %d, TimerCount5 = %d \r\n",
                TimerCount3, TimerCount5 );                         // Display timer  counts
/*
        printf("TimerCount1 = %d, TimerCount2 = %d, TimerCount3 = %d, TimerCount4 = %d, TimerCount5 = %d \r\n",
               TimerCount1, TimerCount2, TimerCount3, TimerCount4, TimerCount5 );                         // Display timer  counts
*/
  	}
#endif

#ifdef TIMER_TEST_2

    // Display message to operator

    Beep(1);
    printf("DSPIC30FXXXX Timer Test Application 5 second test.... \r\n");
    pause(5000);
    Beep(1);
#endif

#ifdef RCTIME_TEST

   printf("DSPIC30FXXXX RcTime Test Application \r\n");

#endif

#ifdef PORTBINT_TEST

    printf("DSPIC30FXXXX Port B Interrupt Test Application \r\n");

 

    SetupInterrupts();                                     // Setup interrupts and software
    // variables.



    while (1)                                           // Loop forever
    {
        // ClrWdt();				// Clear the WDT

        // Display the latest interrupt counts

        printf("InterruptCount0 = ");
        dec(InterruptCount0);                        // Convert interrupt count 0 to a string
        printf("\r\n");

  
        printf("InterruptCount1 = ");
        dec(InterruptCount1);                        // Convert interrupt count 1 to a string
        printf("\r\n");

        printf("InterruptCount2 = ");
        dec(InterruptCount2);                        // Convert interrupt count 2 to a string
        printf("\r\n");


    }                                                      // end while(1)

#endif



#ifdef PAUSE_TEST

    // Test the Pause function

    printf("DSPIC30FXXXX Start Pause Test ... \r\n");

    PauseTest();
#endif

#ifdef DELAY_TEST

    // Test the Delay function

    printf("DSPIC30FXXXX Start Delay Test ... \r\n");

    DelayTest();
#endif

 
#ifdef STOPWATCH_TEST
 
   //SetupInterrupts();                                     // Setup interrupts and software
    // variables.


    // Test the Stopwatch function

    printf("DSPIC30FXXXX Start Stopwatch Test ... \r\n");

    Stopwatch();
#endif


#ifdef CLOCK_TEST

   // Clock Test  

    printf("DSPIC30FXXXX Start Clock Test ... \r\n");

    //TimerInterruptTest();

    // Configure the timers and interrupts

    SetupInterrupts();

    //ClockTest();

    while (TRUE)
    {
	// Get the time and send it to the host

        //GetTime();

        printf("Hours = ");                              // Display number of hours elapsed
        dec(Hours );                                    // convert to string
    
        printf("  Minutes = ");                          // Display Number of minutes elapsed
        dec(Minutes );                                  // convert to string
 
        printf("  Seconds = ");                          // Display number of seconds elapsed 
        dec(Seconds);                                  // convert to string
 
	printf("\r\n");                                     // Send carriage return
 
   }

#endif

#ifdef TIMER_INTERRUPTS

    // Test the Timer interrupts

    printf("DSPIC30FXXXX Start Timer Interrupt Test ... \r\n");
/*
	TRISDbits.TRISD1 = 0;
	PORTDbits.RD1 = 1; // turn off LED on RD1 

	// Wait till the timer matches with the period value 
	while(1)
	{
		timer_value = ReadTimer1();
		if(timer_value >= 0x7FF)
		{
			PORTDbits.RD1 = 0; // turn on LED on RD1 
		}
	}

	CloseTimer1();

*/

    //TimerInterruptTest();

    // Configure the timers and interrupts

    SetupTimers();

    while (1)
   {
  

        printf("TimerCount0 = ");                         // Display timer 0 counts
        dec(TimerCount0 );                              // convert to string
   
        printf("  TimerCount1 = ");                         // Display timer 0 counts
        dec(TimerCount1);                              // convert to string
  
        printf("  TimerCount2 = ");                         // Display timer 0 counts
        dec(TimerCount2 );                              // convert to string
  
        printf("  TimerCount3 = ");                         // Display timer 0 counts
        dec(TimerCount3 );                              // convert to string
	printf("\r\n");                                          // Send carriage return

 
  }

#endif
 
    return;
}

#endif

#ifdef PAUSE_TEST

//********************************************************************************
//* PauseTest - Performs the pause function test.
//********************************************************************************
void            PauseTest(void)
{
    int             i;
    unsigned int pauseTime = 1000;

	LED_1 = 1;
	LED_1_DIR = 0;

    // This is a simple 15 second delay test using the pause function

    Beep(3);
    printf("Starting a 15 second pause test .. \r\n");

    for (i = 0; i < 15; i++)
    {
        pause(pauseTime);
    }

    Beep(5);

    printf("End of 15 second pause test. \r\n");

    printf("Start of LED flashing, every second... \r\n");


    // Test to flash LED every second

    while (TRUE)
    {
        // Toggle an LED

        // Toggle(LED_1);

        LED_1 = ~LED_1;
        pause(pauseTime);                                        // Pause for 0.5 seconds
        Beep(1);
    }


}
#endif

//********************************************************************************
//* Beep - Emits n beeps.
//********************************************************************************
void            Beep(int n)
{
    char            Bell[2] = {0x07, 0x00};                // Bell character
#ifdef DEBUG
    return;
#endif

    do
    {
        string(Bell);                                   // print string
    } while (--n);

}
#ifdef TIMER_INTERRUPTS
//********************************************************************************
//* ProcessTimer1 - Processes Timer 1 interrupt, serviced every second at 
//* a 1 Hz frequency. Also test the realtime clock features using a 32 KHz oscillator
//********************************************************************************

void            ProcessTimer1(void)
{  
  

    TimerCount1++;                                         // Increment Timer Interrupt counter 1

#ifdef CLOCK_TEST

    // Keep track of elapsed hours, minutes and seconds 
 
    Seconds = (Seconds++) % 60;				   // Increment seconds counter

    if (Seconds == 0)                                      
    {
	Minutes = (Minutes++) % 60;                        // Increment minutes counter
 
    	if (Minutes == 0)                                  
        {
           Hours = (Hours++) % 24;   			   // increment hours counter (24 hour clock)                         
        }
    }
#endif    

    // count
    //LED_1 = ~LED_1;                                        // Toggle the timer 1 LED
                                                           // interrupt  LED indicator
   
	// Clear Timer 1
    //WriteTimer1(0);                            
    
}
  
//********************************************************************************
//* ProcessTimer2 - Processes Timer 2 interrupt, serviced every milli-second for  
//* at a 1 KHz frequency.
//********************************************************************************

void            ProcessTimer2(void)
{
 
    TimerCount2++;                                         // Increment Timer Interrupt counter 2  

     //LED_2 = ~LED_2;                                          // Toggle the Timer 2 LED every second
                                                           // interrupt LED indicator
 
 
	// Clear Timer 2
  
  //	WriteTimer2(0);                           
 
 
}

//********************************************************************************
//* ProcessTimer3 - Processes Timer 3 interrupt.
//********************************************************************************

void            ProcessTimer3(void)
{
  
    TimerCount3++;                                         // Increment timer interrupt count 3

    //LED_3 = ~LED_3;                                          // Toggle the Timer 3 LED every second
                                                           // interrupt LED indicator

  	// Clear Timer 3
  
  //	WriteTimer3(0);                           
 
}

//********************************************************************************
//* ProcessTimer4 - Processes Timer 4 interrupt.
//********************************************************************************

void            ProcessTimer4(void)
{
  
    TimerCount4++;                                         // Increment Timer 4 count

    //LED_4 = ~LED_4;                                          // Toggle the timer 4
                                                           // interrupt LED indicator
 
   	// Clear Timer 4
  
  //	WriteTimer4(0);                           

}
 
//********************************************************************************
//* ProcessTimer5 - Processes Timer 5 interrupt.
//********************************************************************************

void            ProcessTimer5(void)
{
  
    TimerCount5++;                                         // Increment timer 5 count

    //LED_4 = ~LED_4;                                          // Toggle the timer 5
                                                           // interrupt LED indicator
   

}

#endif

// Stamp BS2/BSX Compatible function implementation

// Stub out delay routines

#ifdef DEBUG

void pause(word Delay)
{
}

#else

#ifdef TIMER_INTERRUPTS
//********************************************************************************
//* pause - Will pause for n  milli-seconds at the current system clock speed  
//* using interrupt timer clicks. Works the same was as the Stamp "pause" function.  This function
//*  uses Timer 1 or Timer 3, with with the prescaler set to 1:8 and a system clock of 20 Mhz.
//********************************************************************************
void            pause(unsigned long int n)
{
	unsigned long int Start_Time;

	Start_Time = TimerCount3;			// Reset timer

	// Wait for n milliseconds
 	while ((TimerCount3 - Start_Time) < n); 
 
}

#else

//********************************************************************************
//* pause - Will pause for n units of 1000 instruction cycles (milli-seconds at the current system clock)  
//* Works the same was as the Stamp "pause" function.  This function uses Timer 1 or Timer 3, 
//* with with the prescaler set to 1:8 and a system clock of 20 Mhz.
//********************************************************************************
void            pause(unsigned long int n)
{


#ifdef USE_TIMER_2_AND_3

   // Use Timer 2/3
    
    // Clear the Timer 3 overflow flag 

	IFS0bits.T3IF = 0; // Clear Timer 3 interrupt flag 


    do
    {
   
		// Clear 32-Bit timer 2/3
    	WriteTimer23(0);

            
        // Wait until Timer 2/3 counts up to period match and timer overflow flag is set

   
        while (!IFS0bits.T3IF);
 
        // Clear the Timer 3 overflow flag 

        IFS0bits.T3IF = 0;

    } while (--n);

#endif

#ifdef USE_TIMER_4_AND_5

   // Use Timer 4/5
    
    // Clear the Timer 5 overflow flag 

	IFS1bits.T5IF = 0; // Clear Timer 5 interrupt flag 


    do
    {
  
		// Clear 32-Bit timer 4/5
    	WriteTimer45(0);

       // Wait until Timer 4/5 counts up to period match and  timer overflow flag is set
  
        while (!IFS1bits.T5IF);
 
        // Clear the Timer 5 overflow flag 

        IFS1bits.T5IF = 0;

    } while (--n);
    
#endif



}   
#endif 
#endif

//********************************************************************************
//* Sleep1 - Sleep for n units of 2.3 seconds.  Works the same was as the Stamp
//* "sleep" function.
//********************************************************************************
void            Sleep1(int n)
{
    do
    {
        DelayMs(2300);                                     // Sleep for 2.3 seconds
        // inside loop

        //******* Missing logic to put processor in a sleep state ***********
    } while (n--);
}
  

//********************************************************************************
//* Nap - Nap for a short duration of n delay periods.  Each delay period is 
//* set to 2**Period * 18 milli-seconds.  Works the same was as the Stamp 
//* "nap" function.
//********************************************************************************
void            nap(int n)
{
    int             Duration;

    Duration = n * 18;
    DelayMs(Duration);                                     // Nap for 2**n * 18
    // milli-seconds ????

}

// General timing functions implementation

//******************************************************************************
//* DelayUs - Delay for n units of 10 TCY (micro-seconds at 20 MHz) using library 
//* delay function.
//******************************************************************************
void            DelayUs(unsigned int n)
{

#ifdef OSC_20_MHZ
    /* Delay10TCYx Delay multiples of 10 Tcy Passing 0 (zero) results in a delay of 2560 cycles. The 18Cxxx version of
     * this function supports the full range [0,255] The 17Cxxx version supports [2,255] and 0. */

    Delay10TCYx(n);                    // Use this call for delay of 10 X 1 TCY  @20 MHz = 0.5 microseconds
    Delay10TCYx(n);                    // Use this call for delay of 10 X 1 TCY  @20 MHz = 0.5 microseconds
 
  /* Delay100TCYx Delay multiples of 10 Tcy Passing 0 (zero) results in a delay of 2560 cycles. The 18Cxxx version of
     * this function supports the full range [0,255] The 17Cxxx version supports [2,255] and 0. */

    //Delay100TCYx(n);
#endif

}

//******************************************************************************
//* DelayMs - Delay for n milli-seconds using Timer 1 or Timer 3, with the prescaler
//* set to 1:8 and a system clock of 20 Mhz.
//******************************************************************************
void            DelayMs(unsigned int n)
{

#ifdef USE_TIMER1

    // Use Timer 1
    
    // Clear the Timer 1 overflow flag 

    PIR1bits.TMR1IF = 0;

    do
    {

        WriteTimer1(TIMER_COUNT_1MS);                   // 2's complement of 625 (0xFD8F) for a 1 milli-second delay
                                                           // (Use LSB..MSB order to compensate for Microchip 
                                                           // timer library bug)  
                                                           // (Use "WriteTimer1" instead of "WriteTimer1" to compensate for another Microchip 
                                                           // timer library bug)  
            

       // Wait until Timer 1 counts down to 0 or the Timer 1 overflow flag is set
 
       //while ((Count = ReadTimer1()) != 0xFFFF);

        while (!PIR1bits.TMR1IF);
 
        // Clear the Timer 1 overflow flag 

        PIR1bits.TMR1IF = 0;

    } while (--n);
#else

   // Use Timer 3
    
    // Clear the Timer 3 overflow flag 

  	IFS0bits.T3IF = 0; // Clear Timer 3 interrupt flag 


    do
    {

        WriteTimer3(TIMER_16_COUNT_1MS);                      // 2's complement period for a 1 milli-second delay
                                                           // (Use LSB..MSB order to compensate for Microchip 
                                                           // timer library bug)  
                                                           // (Use "WriteTimer3_16" instead of "WriteTimer3" to compensate for another Microchip 
                                                           // timer library bug)  
            

       // Wait until Timer 3 counts down to 0 or the Timer 3 overflow flag is set
 
       //while ((Count = ReadTimer3()) != 0xFFFF);

        while (!IFS0bits.T3IF);
 
        // Clear the Timer 3 overflow flag 

        IFS0bits.T3IF = 0;

    } while (--n);
    
#endif



}
  
//******************************************************************************
//* DelayS - Delay for n seconds using Timer 1, with the prescaler set to 1:256
//* and a system clock of 20 Mhz.
//******************************************************************************
void            DelayS(unsigned int n)
{
      static unsigned int Count = 0;             // Timer count

     
                        
 	IFS0bits.T1IF = 0;							// Clear the Timer 1 overflow flag
    do
    {
 
 
        WriteTimer1(TIMER_16_COUNT_1S);             // Initialize Timer 1 registers 

        // Wait until Timer 1 counts to 0xFFFF or the Timer 1 overflow flag is set
 
        //while (ReadTimer0() != 0xFFFF);
        //while ((Count = ReadTimer0()) != 0xFFFF);

        while (!IFS0bits.T1IF);

        // Clear the Timer 1 overflow flag 

        IFS0bits.T1IF = 0; 
  
    } while (--n);

}


#ifdef ADVANCED_TIMER_FUNCTIONS

//********************************************************************************
//* Stopwatch - This function uses TIMER0 to implement a general purpose stopwatch
//* that can be used to time events or profile code. It allows the operator to push
//* a button connected to the PortB RB3 pin, and then reports the elapsed time.
//********************************************************************************
void            Stopwatch(void)
{
    unsigned int             StartTime = 0;                         // Start time
    unsigned int             StopTime = 0;                          // Stop time
    unsigned int             ElapsedTime = 0;                       // Elapsed time

    // Enable Port weak pull-ups

    EnablePullups();

    while (TRUE)
    {
        printf("Push the START button to start stopwatch \n");
        printf("Push the STOP button to stop the watch and display the elapsed time. \n\n");

        // Wait for operator to press the START button

        while (BUTTON0 != 0);
        //while (InterruptCount0 == 1);

        //StartTime = ReadTimer0();                          // Read Timer 0 elapsed seconds
		StartTime = TimerCount0;

        // Wait for operator to press the STOP button

        while (BUTTON1 != 0);
        //while (InterruptCount1 == 1);

        //StopTime = ReadTimer0();                           // Read Timer 0
        StopTime = TimerCount0;                              // Read Timer 0 elapsed seconds

        // Compute elapsed time

        ElapsedTime = StopTime - StartTime;

        Beep(3);
        printf("StartTime = ");
        dec(StartTime);                                // convert to string
        printf("\r\n");  

        printf("StopTime = ");
        dec(StopTime);                                 // convert to string
        printf("\r\n");  

        printf("ElapsedTime = ");
        dec(ElapsedTime );                              // convert to string
        printf("\r\n");  

    }
}
  
//********************************************************************************
//* rctime - Determines the time it takes for a capacitor to discharge from 5 Volts
//* to 0 Volts.  If the time exceeds 65535 time units then the fuction returns a
//* count of 0.  Works the same was as the Stamp "rctime" function.  This function
//* uses the BS2 pin notations which range from 0..33.
//********************************************************************************
void            rctime(byte Pin, byte State, word *Count)
{
/*  
      byte PinValue;
      
#ifdef RCTIME_FLAG

     // RC circuit code
     
    	// Make the selected BS2 pin an output to charge capacitor for CR cuircuit time
 
     //low(Pin);
     
    TRISE = 0x00;                                          // PORTE all outputs
    XJoystickPin = 0;                                     // Discharge the capacitor
     
     
    	pause(5);                                              // Allow time for discharge

    	// Make the selected BS2 pin an input
	
	  //input(Pin);

    TRISE = 0xFF;                                          // PORTE all inputs
    	

        //WriteTimer0(0);                                    // Reset timer 0
        WriteTimer1(0);                                   // Reset timer 1
        
        //while (getpin(Pin) != State)
        //while (TRUE)
        while(XJoystickPin == State);
        
        
        // wait for Pin to equal state

        *Count  = ReadTimer1();                              // read timer 1
        
      	printf("  Count = ");                         // Display timer 0 counts
        hex4(*Count);                              // convert to string
        printf("\r\n");
        
#else

     // CR circuit code (This logic works !!!!)
     
    	// Make the selected BS2 pin an output to charge capacitor for CR cuircuit time
 
     //high(Pin);
     
    TRISE = 0x00;                                          // PORTE all outputs
    XJoystickPin = 1;                                     // Charge the capacitor
     
     
    	pause(1);                                              // Allow time for charge

    	// Make the selected BS2 pin an input
	
	  //input(Pin);

    TRISE = 0xFF;                                          // PORTE all inputs
    	

        //WriteTimer0(0);                                    // Reset timer 0
        WriteTimer1(0);                                   // Reset timer 1
        
        //while (getpin(Pin) != State)
        //while (TRUE)
        while(XJoystickPin == State);
        
        
        // wait for Pin to equal state

        *Count  = ReadTimer1();                              // read timer 1
        
      	printf("  Count = ");                         // Display timer 0 counts
        hex4(*Count);                              // convert to string
        printf("\r\n");

#endif
*/

}

/*

//********************************************************************************
//* ReadJoystick - Reads the connected joystick using the rctime function and returns
//* the raw x and y axis counts.
//********************************************************************************
void ReadJoystick(word *XCount, word *YCount)
{

   byte PinValue;
   byte State;

   State = 0;
     
   // Read Joystick X axis using RC circuit code
     
   TRISE = 0x00;                                          // PORTE all outputs
   XJoystickPin = 0;                                      // Discharge the capacitor
     
   //pause(5);                                              // Allow time for discharge
   pause(10);                                              // Allow time for discharge

   // Make the selected BS2 pin an input
	
   //input(Pin);
   TRISE = 0xFF;                                          // PORTE all inputs
    	
   WriteTimer1(0);                                     // Reset timer 1
       
   while(XJoystickPin == State);
          
   // wait for Pin to equal state

   *XCount  = ReadTimer1();                              // read timer 1
  
   // Read Joystick Y axis using RC circuit code
 	// Make the selected BS2 pin an output to charge capacitor for CR cuircuit time
 
   //low(Pin);
     
   TRISE = 0x00;                                          // PORTE all outputs
   YJoystickPin = 0;                                      // Discharge the capacitor
     
   //pause(5);                                              // Allow time for discharge
   pause(10);                                              // Allow time for discharge

   // Make the selected BS2 pin an input
	
   //input(Pin);
   TRISE = 0xFF;                                          // PORTE all inputs
    	
   WriteTimer1(0);                                     // Reset timer 1
       
   while(YJoystickPin == State);
          
   // wait for Pin to equal state

   *YCount  = ReadTimer1();                              // read timer 1
        
}
*/
  


//********************************************************************************
//* Sound - Emits a sound of a specified frequency and duration to a speaker
//* connected to any I/O pin. Works the same was as the Stamp "sound" function.
//********************************************************************************
void            Sound(int n)
{
    char            Bell[2] = {0x07, 0x00};                // Bell character
#ifdef DEBUG
    return;
#endif

    do
    {
        string(Bell);                                   // print string
    } while (--n);

}

#ifdef TIMER_INTERRUPTS
//********************************************************************************
//* GetTime - Gets the clock time and sends it to the host using (HH:MM:SS) format.
//********************************************************************************
void            GetTime(void)               // Get the time and send it to the host
{

        printf("Hours = ");                              // Display number of hours elapsed
        dec(Hours);                                    // convert to string
   
        printf("  Minutes = ");                          // Display Number of minutes elapsed
        dec(Minutes);                                  // convert to string
   
        printf("  Seconds = ");                          // Display number of seconds elapsed 
        dec(Seconds );                                  // convert to string
        printf("\r\n");
 
}

//********************************************************************************
//* SetTime - Sets the clock time using (HH:MM:SS) format.
//********************************************************************************
void            SetTime(void)               // Set the time sent from host 
{

	        // Get hours from host 

                HexNumber[0] = toupper(get_byte());        // Get first hex digit (MSB)
                HexNumber[1] = toupper(get_byte());        // Get second hex digit (LSB)
                Ch = get_byte();                           // Get colon delimiter

               // Combine to a two digit hex value into Hours

                Hours = htoi(HexNumber);

	        // Get Minutes from host 

                HexNumber[0] = toupper(get_byte());        // Get first hex digit (MSB)
                HexNumber[1] = toupper(get_byte());        // Get second hex digit (LSB)
                Ch = get_byte();                          // Get colon delimiter

                // Combine to a two digit hex value into minutes

                Minutes = htoi(HexNumber);

	        // Get Seconds from host 

                HexNumber[0] = toupper(get_byte());        // Get first hex digit (MSB)
                HexNumber[1] = toupper(get_byte());        // Get second hex digit (LSB)
                Ch = get_byte();                           // Get semi-colon delimiter

                // Combine to a two digit hex value into Seconds

                Seconds = htoi(HexNumber);


}
#endif



#ifdef DELAY_TEST

//********************************************************************************
//* DelayTest - Performs the Delay function test.
//********************************************************************************
void            DelayTest(void)
{
    int             i;
    unsigned int TimeDelay = 1;     

    // This is a simple 25 second delay test using the DelayS function

    Beep(3);
    printf("Starting a 25 second delay test.. \n");

    for (i = 0; i < 25; i++)
    {
        DelayMs(1000);                              // Delay for 1 second
        //DelayS(TimeDelay);                            // Delay for 1 second
   }

    Beep(5);
    printf("End of 25 second delay test. \n");
    printf("Start of LED flashing, every second... \n");


    // Test to flash LED every second

    while (TRUE)
    {
        // Toggle an LED

        // Toggle(LED0);

        LED0 = ~LED0;
        DelayMs(1000);                                      // Delay for 1 second
        //DelayS(TimeDelay);                                    // Delay for 1 second
    }

}
#endif

#ifdef CLOCK_TEST

//********************************************************************************
//* ClockTest - Performs the clock controller function test.
//********************************************************************************
void ClockTest(void)
{

    // Main Timer Controller menu

menu:

    printf("        The following Timer Controller commands are available: 				\r\n");
    printf(" 		 									\r\n");
    printf("       'SHH:MM:SS;   -  Set the time using HHMMSS format.                         	\r\n");
    printf("											\r\n");
    printf("        G;'          -  Get the current time in HH:MM:SS format     	                \r\n");
    printf("                        format.	 							\r\n");
    printf("			 	 							\r\n");
    printf("        'Z;'         -  Reset the Timer.	 			                \r\n");
    printf("Enter timer command:  								        \r\n");
 
   // Configure the timers and interrupts

    SetupInterrupts();
    return;

  
    do
    {
        // Get next command from RS-232 Port

        Command = toupper(get_byte());

        // Execute command

        switch (Command)
        {
 
        case 'S':
        {
                // Set the time in HH:MM:SS format from the host

 		SetTime();
 
#ifdef TEST
//*******************************************************************************
// Test code
                send_byte('S');                            // Verify that the set time command was received

//*******************************************************************************
#endif

                break;
        }

 
        case 'G':
        {    
           
                Ch = get_byte();                           // Get terminator ";" character from host

		// Get the current time in HH:MM:SS format and send it to the host

 		GetTime();


#ifdef TEST
//*******************************************************************************
// Test code
                send_byte('G');                            // Verify that the get time command was received

//*******************************************************************************
#endif
                break;

        }
   

        case 'Z':
        {
		SetupTimers();

#ifdef TEST
//*******************************************************************************
// Test code
                send_byte('Z');                            // Verify that the reset command was received
//*******************************************************************************
#endif


		break;
	}
	}
 
        } while (TRUE);  
 
	// Close PORT interrupt handlers 

	CloseINT0();
	CloseINT1();
	CloseINT2();
	CloseINT3();
	CloseINT4();

    CloseTimer0();                                     // close modules
    CloseUSART();

}

#endif

#ifdef TIMER_INTERRUPTS

//********************************************************************************
//* TimerInterruptTest - Performs the Timer Interrupt test.  It will generate interrupts
//* at selected periods of 250 milli-seconds, 500 milli-seconds, and 1 second.  Each
//* ISR will flash and LED and increment a timer count corresponding to the individual
//* timer interrupt.  Timers 0..3 are used for this test.
//* The interrupt periods for each timer are set as follows:
//*  	Timer 0  - 1 Second 
//*     Timer 1  - 1 milli-second  
//*     Timer 2  - 500 micro-seconds
//*     Timer 3  - 250 milli-seconds
//*
//********************************************************************************
void            TimerInterruptTest(void)
{
    // Configure the timers and interrupts

    SetupInterrupts();

                                                       
}

#endif
 
#endif

 
