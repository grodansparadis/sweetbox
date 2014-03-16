//********************************************************************************
//*        Name:  uartutil.c                                                     *
//* Project Number:  MT2192                                                      *                                                   
//*           Date:  10/15/2007                                                  *                                                  
//*                                                                              *
//*   Description:                                                               * 
//*     This file contains all I/O functions used by the PIC "C" compiler.       * 
//* Included are low level functions to initialize the USART, read the USART,    * 
//* and write to the USART.  Also included are the Standard "C" I/O functions    *
//* such as printf, scanf, getc, putc.  A ring buffer will be used to buffer the *
//* serial input from the USART, so that data between the host and the PIC       *
//* micro-controller are not lost.                                               *
//*                                                                              *
//*      Future upgrades to this application include: File I/O, I2C, and         *
//* Internet networking.                                                         *
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
 
#include <stdio.h>
#include <dsp.h>
#include <uart.h> 					// UART library functions
#include <spi.h>
#include <i2c.h>
#include <math.h>
#include <adc12.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <timer.h>                                     // Timer library functions                                                                 
//#include <delays.h>                                     // High resolution delay functions
#include "timeutil.h"                                   // Timer library functions                               
//#include "portutil.h"                                   // My own I/O Port library functions
#include "my_types.h"					// General C/C++ types
//#include "lcd.h"						// LCD Support using SPI2 
#include "uartutil.h"					// UART Utilities 

//********************************************************************************
// Constant Definitions
//********************************************************************************


//#define NETWORK_TEST                                    // Select the PC to PIC Networking tests using the USART
//#define STDIO_FUNCTION_TEST                             // Select the stdio function tests
//#define USART_TEST                                    // Select the USART function test  
 
 
const word Maxtimeout = 9000;                           // Maximum number of cycles to wait for data from host
//#define NULL  0
 
#define OK 1
#define FAIL 0


#define AVAILABLE -1
#define USED      -2

  
//*******************************************************************************
// Define port bit definitions here
//*******************************************************************************
 
// PORT C definitions
 
#define TX             	PORTCbits.RC6           // Used for serial communication to PC Host 
#define RX             	PORTCbits.RC7           // Used for serial communication to PC Host 

#define TXDir         	TRISCbits.TRISC6        // Fire Button LED indicator pin direction 
#define RXDir      		TRISCbits.TRISC7        // Trigger Button LED indicator pin direction
 
 
#define SCOPE1 PORTDbits.RD0                            // Software timing pulse output (used for benchmarking ISRs)

 
//********************************************************************************
// Variable declarations
//********************************************************************************

int LCD_Ready = FALSE;

// USART general variables

#ifdef UARTUTIL_TEST

const  char  ReadyString[] = "\n\rREADY>";
const   char  ErrorString[] = "\n\rERROR!";

#endif

const char HexLookup[] = "0123456789ABCDEF"; 	// Hex look-up table
 
 
//Hex_T Hex;   						// Hex string 

 
char            str[7];                                 // Temporary string
 
 
// Declare global USART variables here

char Ch; 						// Character read from USART
byte HexNumber[9];					// Hex string range (00000000 .. FFFFFFFF) used by type conversion functions


#ifdef DEBUG
 
// Include debug variables used by MPSIM here

#include "mpsim.inc"

#endif


 
// Declare usart serial handshake variables here

//char Attention = '~';                                 // Used to get STAMP's attention
//char Ready = '>';                                     // Ready prompt, indicating that the
	                                                    // host is ready to accept more data.
//char Acknowledge= '#';                                // Ackowledge the STAMP's response.

#ifdef STDIO_INTERRUPTS

// Declare Serial Ring Buffer variables here
volatile int NumberCharacters = 0;                      // Total number of characters in ring buffer.
volatile byte RingBuffer[BUFFER_SIZE];                  // Ring Buffer for incomming data from serial port (USART)

volatile int StartBuffer=0;                            // Start pointer for ring buffer       
volatile int EndBuffer=0;                              // End pointer for ring buffer
volatile byte ReceivedDataFlag = FALSE;                 // Indicates that data was received into the ring buffer
volatile word Timeout = 0;                              // Serial I/O Timeout counter
volatile byte TimeoutFlag;                              // Timeout flag

volatile int SerialCommunicationsError;                 // Serial I/O error flag
                                                        // -1 = Serial Input Bufer overrun error
                                                        // -2 = Communications timeout error
                                                        // -3 = Ring buffer empty error
                  
volatile word  RxSerialInterruptCount1 = 0;             // Interrupt UART 1 RX  count
volatile word  RxSerialInterruptCount2 = 0;             // Interrupt UART 2 RX  count
volatile word  TxSerialInterruptCount1 = 0;             // Interrupt UART 1 TX  count
volatile word  TxSerialInterruptCount2 = 0;             // Interrupt UART 2 TX  count

#endif

// Floating point conversion functions
 
// Calculator specific variables

char Buffer[BufferSize];					// Floating point Buffer
/*
char StackLabel[]={'X','Y','Z','T'};		// Stack register labels
//double Stack[StackSize];					// HP-45 Stack 
//double Registers[RegisterSize];				// HP-45 Registers 1-9
double X = 0.0;								// X register (Display)
double Y = 0.0;								// Y register
double LastX = 0.0;	
byte RegisterNumber = 0;					// Register number 1..9
int BufferCount = 0;								// Buffer count
int MessageBufferIndex = 0;							// Message buffer index
byte KeyValue; 
*/						// Last X register
int CharacterCount = 0;								// Character count

extern char Ch;

// Float conversion characters

char Period[] = ".";
char Plus[] = "+";
char Minus[] = "-";
char Blank[] = " ";
char E[] = "E";

char Operators[] = {'+','-','*','/','.','=',0};          // Math operators 
//extern char Operators[];                               // Math operators declared in keypad.c

char NumericChars[] = {'.','E',0};                       // Numeric characters 

// Power of 10 lookup tables used by the pow10 function

 
//     10 **                 0     1      2      3     4       5      6      7      8      9      10
const double PlusPowerOf10[11] = {1.0, 1.0E1, 1.0E2, 1.0E3, 1.0E4, 1.0E5, 1.0E6, 1.0E7, 1.0E8, 1.0E9, 1.0E10 };
const double MinusPowerOf10[11] = {1.0, 1.0E-1, 1.0E-2, 1.0E-3, 1.0E-4, 1.0E-5, 1.0E-6, 1.0E-7, 1.0E-8, 1.0E-9, 1.0E-10 };
 
// Variables used for stdio function unit tests

  
byte B1, B2;                   							// Test 8-Bit unsigned bytes
word W1, W2;											// Test 16-Bit unsigned words
int I1, I2;                                             // Test 16-Bit signed integers
long L1, L2;                                            // Test 32-Bit signed integers
unsigned long UL1, UL2;                                 // Test 32-Bit unsigned integers
HexByte_T Hb1, Hb2;                                     // Test Hex bytes						
HexWord_T Hw1, Hw2;                                     // Test Hex words
HexLongWord_T Hlw1, Hlw2;								// Test Hex long words

// Declare local function headers here

byte	LookupHexValue(char HexValue);					// Lookup equivalent decimal value(0..15) of hex digit (0..F)


// Declare SPI Slave Buffer variables here
// Message synchronization 
#define SYNC_BYTE	255							// SPI Sync Byte
#define ACK_BYTE	254							// SPI Acknowledge Byte

//#define SPI_BUFFER_SIZE 34							// Size of SPI Tx and Rx Buffers
//byte SPI_Rx_Buffer[SPI_BUFFER_SIZE];                  	// SPI Receive Buffer
//byte SPI_Tx_Buffer[SPI_BUFFER_SIZE];                  	// SPI Transmit Buffer

         


volatile char Buf[80];												//Received data is stored in array Buf
volatile char * Receiveddata=Buf;    								//Receiveddata is a char pointer whose address is the same as Buf[80]
volatile byte TheByte = 0;
volatile byte RX_Data_Received = FALSE;

#ifdef STDIO_INTERRUPTS

// Stdio interrupt code  used for USART with ring buffers
 
void __attribute__((__interrupt__)) _U1TXInterrupt(void)	//This is the UART1 transmit ISR
{
    TxProcessUart1();
 	IFS0bits.U1TXIF=0;
}

void __attribute__((__interrupt__)) _U2TXInterrupt(void)	//This is the UART2 transmit ISR
{
    TxProcessUart1();
 	IFS1bits.U2TXIF=0;
}

void __attribute__((__interrupt__)) _U1RXInterrupt(void)	//This is the UART1 Receive ISR
{
 
    RxProcessUart1();

/*
 	while(DataRdyUART1())                 						//Read the receive buffer as long as at least one or more characters 
															//can be read
 	{
  		( *( Receiveddata)++)=ReadUART1();  
 	}
*/
	IFS0bits.U1RXIF=0;

}

void __attribute__((__interrupt__)) _U2RXInterrupt(void)	//This is the UART2 Receive ISR
{
 
    RxProcessUart2();

 	IFS1bits.U2RXIF=0;

}

#else

// UART code with no ring buffer support

void __attribute__((__interrupt__)) _U1TXInterrupt(void)	//This is the UART1 transmit ISR
{
 	IFS0bits.U1TXIF=0;
}


void __attribute__((__interrupt__)) _U1RXInterrupt(void)	//This is the UART1 Receive ISR
{
	LED_1 = ON;											// Toggle LED when a character is received
    RX_Data_Received =  TRUE;
	while (!U1STAbits.URXDA);
    TheByte = U1RXREG;
	IFS0bits.U1RXIF=0;										// Clear UART1 RX flag
 
/*
	while(DataRdyUART1())                 						//Read the receive buffer as long as at least one or more characters 
															//can be read
	 {
  	( *( Receiveddata)++)=ReadUART1();  
 	}
*/

}


#endif 

#ifdef UARTUTIL_TEST

 
//******************************************************************************
//* main -  Test the RS-232 Port (USART) functions 
//******************************************************************************
int main(void)
{
	int i;
	char c;

  	// Configure UART  1 for current CPU cloc and baud rate,  8-Bits, No parity, Asynchronous
 
  	InitializeUart1();

    // Display message to operator

    printf("DSPIC Uart Utilities Function Test... \r\n");

	LED_1 = OFF;
	LED_1_DIR = OUTPUT;

    while(1)
    {
		printf("Enter any character :");
        c = getch();
        printf("\r\n");
        printf("The character  entered was ");
        putchar(c);
        printf("\r\n");
	}
    
	while(1)
	{
		LED_1 = OFF;
		for (i=0; i<1000; i++)
		{
			Printf("This is a test ");  
            Dec(i);
            Printf(" ");
            Hex(i);
            Printf("\r\n");
		}
 
	}
#ifdef TEST     
	//***********************************************************************************************
    // Test code - Send a ramp 0..SPI_BUFFER_SIZE to the Coprocessor and receive a ramp back from the
    // Coprocessor unlimited number of times.
    
 	for (i=0; i<SPI_BUFFER_SIZE; i++)
	{
 		SPI_Rx_Buffer[i] = 0;                  		// Clear SPI Receive Buffer 
 		//SPI_Tx_Buffer[i] = i;                  	// Initialize SPI Transmit Buffer to a ramp 
 		SPI_Tx_Buffer[i] = SPI_BUFFER_SIZE-i;      	// Initialize SPI Transmit Buffer to a reverse ramp 
	}
 
	while(1)
	{
		// Wait for the Sync byte from Master first

   	   	while (get_byte() != SYNC_BYTE);
   	   	//while (ReadHexByte() != SYNC_BYTE);

		// Send ACK byte to Master indicating it received the message

		send_byte(ACK_BYTE);
		//hex2(ACK_BYTE);

 		//for (i=0; i<SPI_BUFFER_SIZE; i++)
 		for (i=0; i<10; i++)
		{
			SPI_Rx_Buffer[i] = get_byte();
			//SPI_Rx_Buffer[i] = ReadHexByte();
		}

		// Wait for the Sync byte from Master first

   	   	while (get_byte() != SYNC_BYTE);
   	   	//while (ReadHexByte() != SYNC_BYTE);

		// Send ACK byte to Master indicating it received the message

		send_byte(ACK_BYTE);
		//hex2(ACK_BYTE);

 		//for (i=0; i<SPI_BUFFER_SIZE; i++)
 		for (i=0; i<10; i++)
		{
			send_byte(SPI_Tx_Buffer[i]);
			//hex2(SPI_Tx_Buffer[i]);
		}


   	}
 	//***********************************************************************************************
#endif
  
#ifdef NETWORK_TEST

       // Invoke the Networking unit test.  Sends and receives messages

       UnitTest();

#endif


 
#ifdef USART_TEST

    	// Invoke the desired stdio test

   	// Display message to operator indicating that the USART test is being run

        Hb1[2] = 0;
        Hw1[4] = 0;
        Hlw1[8] = 0;
           
        printf("Enter a string of 2 characters, Hb1: ");
		//gets1USART((char *) Hb1,2);
		scanf("%2s", Hb1);

        printf("Hb1 = !");
        string((char *) Hb1);
        printf("! \r\n");
   
        printf("Enter a string of 4 characters, Hw1: ");
		//gets1USART((char *) Hw1,4);
		scanf("%4s", Hw1);
 
        printf("Hw1 = !");
        string((char *) Hw1);
        printf("! \r\n");

        printf("Enter a string of 8 characters, Hlw1: ");
		//gets1USART((char *) Hlw1,8);
		scanf("%8s", Hlw1);
        printf("Hlw1 = !");
        string((char *) Hlw1);
        printf("! \r\n");


    	printf("Starting the USART Test. Type single character commands at prompt ... \r\n");

    	while (TRUE)                                           	// Loop forever
    	{
        	// ClrWdt();					// Clear the WDT

        	// Display prompt 

 		printf(ReadyString);
                Ch = get_byte();
                printf("The byte received was !");
                send_byte(Ch);
                printf("! \r\n");

  

    	}                                                      // end while(TRUE)

#endif

#ifdef STDIO_FUNCTION_TEST
 
   // Initialize timer
   
   //SetupTimers();
 
   for (i=1; i<100;i++)
    { 
   	printf("This is a test ");
        dec(i);
        printf(" times !\r\n");
        //pause(1000);
    }

      // Test the ring buffer by displaying current number of characters recently received in the buffer
/*
      while (1)
      {
          printf("StartBuffer = ");
          dec(StartBuffer);
          printf("  EndBuffer = ");
          dec(EndBuffer);
          printf("  NumberCharacters = ");
          dec(NumberCharacters);
          printf("\r\n");
          B1 = ReadNoWait();
      }
*/
 
  		// Display message to operator indicating that the stdio test is being run

    	printf("Starting the stdio Test. Type commands at prompt ... \r\n");

		// Test of Stamp BS2 compatability functions

		// Test 1 - Read 2 hex digits from the USART, convert it to a 8-Bit unsigned byte and 
        // display it in decimal.  This test covers functions ReadHexByte,  hex2 and dec.

		printf("Test 1 - (ReadHexByte) Enter 2 hex digits XX representing a hex byte: r\n");
        B1 = ReadHexByte();
        printf(" The hex byte value is: "); 
        hex2(B1);
        printf("\r\n");
 
        printf(" The decimal byte value is: ");
        dec(B1);
        printf("\r\n");
 	
	// Test 2 - Read 4 hex bytes from the USART and convert it to a 16-Bit unsigned word and 
        // display it in decimal.  This test covers functions ReadHexWord,  hex4 and dec.

	printf("Test 2 - (ReadHexWord) Enter 4 hex digits XXXX representing a hex word: \r\n");
        W1 = ReadHexWord();
        printf(" The hex word value is: ");
 	hex4(W1);
       	printf("\r\n");
 
        printf(" The decimal word value is: ");
        dec(W1);
        printf("\r\n");
 
 /*	 
	// Test 3 - Read 4 hex bytes from the USART and convert it to a 16-Bit signed integer and 
        // display it in decimal.  This test covers functions ReadHexInteger,  hex4 and dec.

	printf("Test 3 - (ReadHexInteger) Enter 4 hex digits XXXX representing a 16-Bit signed integer: \r\n");
        I1 = ReadHexInteger();
        printf(" The hex word value is: ");
        hex4(I1);
        printf("\r\n");

        printf(" The decimal word value is: ");
        dec(I1);
        printf("\r\n"); 
 
*/
	   
	// Test of type conversion functions

	// Test 4 - Convert from an unsigned byte to a hex byte range (00..FF) 

	printf("Test 4 - (btoh) Enter 2 hex digits XX representing a byte: ");
        B1 = ReadHexByte();
        printf(" The decimal byte value is: ");
        dec(B1);
        printf("\r\n"); 
 
  	printf(" The hex byte value is: ");
        btoh(B1, Hb1); 
        string((char *) Hb1);
        printf("\r\n");
 

	// Test 5 - Convert from 16-Bit unsigned word to a hex word range (0000..FFFF) 

	printf("Test 5 - (wtoh) Enter 4 hex digits XXXX representing a 16-Bit hex word: ");
        W1 = ReadHexWord();
        printf(" The decimal word value is: ");
        dec(W1);
        printf("\r\n"); 
 

        printf(" The hex word value is: ");
	wtoh(W1, Hw1); 
        string((char *)  Hw1);
        printf("\r\n");
 
	// Test 6 - Convert from 16-Bit signed integer to a hex word range (0000..FFFF)
/*
	printf("Test 6 - (itoh) Enter 4 hex digits XXXX representing a 16-Bit signed integer: \r\n");
        I1 = ReadHexInteger();
        printf(" The decimal word value is: ");
        dec(I1);
        printf("\r\n");

 	printf(" The hex word value is: ");
	itoh(I1, Hw1); 
        string((char *) Hw1);
        printf("\r\n");

*/
 	// Test 7 - Convert from  a hex string (00..FF) to  an 8-Bit unsigned hex byte range (0..255) 

	printf("Test 7 - (htob) Enter 2 hex digits XX representing a 8-Bit unsigned byte: \r\n");
        Hb1[0] = getchar();  
        Hb1[1] = getchar(); 
	Hb1[2] = 0;

	B1 = htob(Hb1);

        printf(" The decimal byte value is: ");
 	dec(B1);
        printf("\r\n");

 	printf(" The hex byte value is: ");
 	string((char *) Hb1);
        printf("\r\n");
       
 
 	// Test 8 - Convert from  a hex word string (0000..FFFF) to  an 16-Bit unsigned hex word range (0..65535) 

	printf("Test 8 - (htow) Enter 4 hex digits XXXX representing a 16-Bit unsigned word: \r\n");
        Hw1[0] = getchar();  
        Hw1[1] = getchar();  
        Hw1[2] = getchar();  
        Hw1[3] = getchar(); 
	Hw1[4] = 0;

	I1 = htow(Hw1);

        printf(" The decimal word value is: ");
 	dec(I1);
        printf("\r\n");
 		
 	printf(" The hex word value is: ");
        string((char *) Hw1);
        printf("\r\n");
 

 	// Test 9 - Convert from  a hex word string (0000..FFFF) to  an 16-Bit signed integer range (-32768..32767) 

	printf("Test 9 - (htoi) Enter 4 hex digits XXXX representing a 16-Bit signed integer: \r\n");
        Hw1[0] = getchar();  
        Hw1[1] = getchar();  
        Hw1[2] = getchar();  
        Hw1[3] = getchar(); 
	Hw1[4] = 0;

	I1 = htoi(Hw1);

        printf(" The decimal integer value is: ");
 	dec(I1);
        printf("\r\n");

 	printf(" The hex integer value is: ");
        string((char *) Hw1);
        printf("\r\n");
 

	// Test 10 - Convert from  a hex word string (00000000..FFFFFFFF) to  an 32-Bit unsigned hex word range (0..????) 
/*
	printf("Test 10 - (htol) Enter 8 hex digits XXXXXXXX representing a 32-Bit signed integer \r\n");
        Hlw1[0] = getchar();  
        Hlw1[1] = getchar();  
        Hlw1[2] = getchar();  
        Hlw1[3] = getchar(); 
        Hlw1[4] = getchar(); 
        Hlw1[5] = getchar(); 
        Hlw1[6] = getchar(); 
        Hlw1[7] = getchar(); 
	Hlw1[8] = 0;

	L1 = htol(Hlw1);

        printf(" The decimal long integer value is: ");
 	dec(L1);
        printf("\r\n");

 	printf(" The hex long integer value is: ");
 	string((char *) Hlw1);
        printf("\r\n");
 
 	// Test 11 - Convert from  a hex word string (00000000..FFFFFFFF) to  an 32-Bit unsigned hex word range (0..????) 

	printf("Test 11 - (htoul) Enter 8 hex digits XXXXXXXX representing a 32-Bit unsigned word: \r\n");
        Hlw1[0] = getchar();  
        Hlw1[1] = getchar();  
        Hlw1[2] = getchar();  
        Hlw1[3] = getchar(); 
        Hlw1[4] = getchar(); 
        Hlw1[5] = getchar(); 
        Hlw1[6] = getchar(); 
        Hlw1[7] = getchar(); 
	Hlw1[8] = 0;

	UL1 = htol(Hlw1);

        printf(" The decimal long unsigned integer value is: ");
 	dec(UL1);
        printf("\r\n");

 	printf(" The hex unsigned long integer value is: ");
 	string((char *) Hlw1);
        printf("\r\n");
 
*/
#endif
	return 0;

}

#endif
 
 
//********************************************************************************
//* InitializeUart1 - Initializes variables and registers for UART1 used by stdio 
//* serial I/O functions.
//********************************************************************************

void            InitializeUart1(void)
{
 	unsigned  uartmode,uartsta;			// Uart configuration variables
 

  	unsigned long baud,U1MODEvalue,U1STAvalue;
	CloseUART1();

	ConfigIntUART1(UART_RX_INT_EN & UART_RX_INT_PR6 & UART_TX_INT_DIS & UART_TX_INT_PR2);   // Works!!!
 
	// Configure the UART 1 here ...
 
	// Compute the baud rate constant needed to configure UART1 
 	baud = (((THE_CLOCK_SPEED/THE_BAUD_RATE) /16) - 1);
 
	U1MODEvalue=UART_EN & UART_RX_INT_EN & UART_TX_INT_EN & UART_DIS_LOOPBACK & UART_NO_PAR_8BIT & UART_1STOPBIT;
	U1STAvalue =UART_INT_TX_BUF_EMPTY & UART_TX_PIN_NORMAL & UART_TX_ENABLE & UART_INT_RX_CHAR & UART_ADR_DETECT_DIS & UART_RX_OVERRUN_CLEAR;

	OpenUART1(U1MODEvalue,U1STAvalue,baud);

#ifdef STDIO_INTERRUPTS

 
    // Initialize global variables here
  
    RxSerialInterruptCount1 = 0;                                   // Reset UART 1 RX Serial Interrupt counter
    TxSerialInterruptCount1 = 0;                                   // Reset UART 1 TX Serial Interrupt counter
 
    NumberCharacters = 0;                                     // Reset total number of characters in ring buffer
 
  // Flush the ring buffer

    FlushBuffer();
 
    NumberCharacters = 0;                                     // Reset total number of characters in ring buffer


/* 
 
#ifdef STDIO_INTERRUPTS 

 
    // Configure USART interrupts (1 = High Priority, 0 = Low Priority).   

    IPR1bits.RCIP = 1;
  
    // Enable USART asynchronous interrupt

    PIE1bits.RCIE = 1; 

  // Enable peripheral interrupts
 
   INTCONbits.PEIE = 1;                                    

    // Enable general interrupts

    INTCONbits.GIE = 1;                                    // Enable all interrupts
                                     
#else

     // Disable USART asynchronous interrupt

    //PIE1bits.RCIE = 0; 
                                
 
*/

#endif
 
}

 
#ifdef NEW_CODE
 
 
//********************************************************************************
//* InitializeUart2 - Initializes variables and registers for UART2 used by stdio 
//* serial I/O functions.
//********************************************************************************

void            InitializeUart2(void)
{
 
  	unsigned int baud,uartmode,uartsta;			// Uart configuration variables
   	unsigned long baud,U2MODEvalue,U2STAvalue;

	//**************************************************************************
 	//* Configure the UART 2 
	//**************************************************************************  

	CloseUART2();

	ConfigIntUART2(UART_RX_INT_EN & UART_RX_INT_PR6 & UART_TX_INT_DIS & UART_TX_INT_PR2);   // Works!!!
 
	// Configure the UART 2 here ...
 
	// Compute the baud rate constant needed to configure UART1 
 	baud = (((THE_CLOCK_SPEED/THE_BAUD_RATE) /16) - 1);
 
//	baud=(((unsigned long)TCY_PER_SECOND)/(16*baudRate))-1; 

	U2MODEvalue=UART_EN & UART_RX_INT_EN & UART_TX_INT_EN & UART_DIS_LOOPBACK & UART_NO_PAR_8BIT & UART_1STOPBIT;
	U2STAvalue =UART_INT_TX_BUF_EMPTY & UART_TX_PIN_NORMAL & UART_TX_ENABLE & UART_INT_RX_CHAR & UART_ADR_DETECT_DIS & UART_RX_OVERRUN_CLEAR;

	OpenUART2(U2MODEvalue,U2STAvalue,baud);

#ifdef STDIO_INTERRUPTS

    // Initialize global variables here
  
     RxSerialInterruptCount2 = 0;                                   // Reset UART 2 RX Serial Interrupt counter
     TxSerialInterruptCount2 = 0;                                   // Reset UART 2 TX Serial Interrupt counter

    NumberCharacters = 0;                                     // Reset total number of characters in ring buffer
 
  // Flush the ring buffer

    FlushBuffer();
 
    NumberCharacters = 0;                                     // Reset total number of characters in ring buffer


/* 
 
#ifdef STDIO_INTERRUPTS 

 
    // Configure USART interrupts (1 = High Priority, 0 = Low Priority).   

    IPR1bits.RCIP = 1;
  
    // Enable USART asynchronous interrupt

    PIE1bits.RCIE = 1; 

  // Enable peripheral interrupts
 
   INTCONbits.PEIE = 1;                                    

    // Enable general interrupts

    INTCONbits.GIE = 1;                                    // Enable all interrupts
                                     
#else
*/

     // Disable USART asynchronous interrupt

    //PIE1bits.RCIE = 0; 
                                
#endif
 
}
#endif

#ifdef STDIO_INTERRUPTS 

//******************************************************************************
//* RxProcessUart1 -  Process the UART 1 RX interrupt by storing the incomming character
//* into the ring buffer.
//******************************************************************************
void RxProcessUart1(void)
{
 
  	   RxSerialInterruptCount1++;                                // Increment serial interrupt count
          
       LED_1 = ~LED_1;			// Toggle LED 1 indicating UART 1 RX interrupt processed

    	// First check to see if data is available from the USART and then read it if 
    	// it has been received.  

    	if (DataRdyUART1())
    	{
      	// Save byte read from UART 1 into the serial input ring buffer

    		RingBuffer[EndBuffer] = ReadUART1();
   
			EndBuffer++;
        	EndBuffer = EndBuffer & (BUFFER_SIZE - 1);        // Wrap the EndBuffer pointer if necessary
        	 
            NumberCharacters++;                                   // Increment the number of characters currently in the buffer
        	ReceivedDataFlag = TRUE;
     	}
    
 
                                                                                                                  
}

//******************************************************************************
//* RxProcessUart2 -  Process the UART 2 RX interrupt by storing the incomming character
//* into the ring buffer.
//******************************************************************************
void RxProcessUart2(void)
{
 
  	   RxSerialInterruptCount2++;                                // Increment serial interrupt count
          
       LED_2 = ~LED_2;			// Toggle LED 2 indicating UART 2 RX interrupt processed

    	// First check to see if data is available from the USART and then read it if 
    	// it has been received.  

    	if (DataRdyUART2())
    	{
      	// Save byte read from UART 2 into the serial input ring buffer

    		RingBuffer[EndBuffer] = ReadUART2();
 
			EndBuffer++;
        	EndBuffer = EndBuffer & (BUFFER_SIZE - 1);        // Wrap the EndBuffer pointer if necessary
        	 
            NumberCharacters++;                                   // Increment the number of characters currently in the buffer
        	ReceivedDataFlag = TRUE;
     	}
                                                                                                       
}

//******************************************************************************
//* TxProcessUart1 -  Process the UART 1 TX interrupt by storing the incomming character
//* into the ring buffer.
//******************************************************************************
void TxProcessUart1(void)
{
 
  	   TxSerialInterruptCount1++;                                // Increment serial interrupt count
          
       LED_3 = ~LED_3;			// Toggle LED 3 indicating UART 1 TX interrupt processed

  
                                                            	// ISR code timing                                                           
}
//******************************************************************************
//* TxProcessUart2 -  Process the USART 1 RX interrupt by storing the incomming character
//* into the ring buffer.
//******************************************************************************
void TxProcessUart2(void)
{
 
  	   TxSerialInterruptCount2++;                                // Increment serial interrupt count
          
       LED_4 = ~LED_4;			// Toggle LED 4 indicating UART 2 TX interrupt processed

    	// First check to see if data is available from the USART and then read it if 
    	// it has been received.  

 
                                                            	// ISR code timing                                                           
}
 
//******************************************************************************
//* FlushBuffer - Flush the ring buffer
//******************************************************************************

void    FlushBuffer(void)
{
	int i;															// Local loop index
	
  	StartBuffer = 0;                                   // Start pointer for ring buffer       
  	EndBuffer = 0;                                     // End pointer for ring buffer
	NumberCharacters = 0;                              // Number of characters
	
 	for (i=0; i< BUFFER_SIZE; i++)
	{
 		RingBuffer[i] = 0;
   }

}

#endif



//******************************************************************************
//******************************************************************************
//******************************************************************************



//******************************************************************************
//* string - Writes a string terminated with a null character from RAM to the
//* serial port.                               
//******************************************************************************

void  string(char *s)
{
	// Send the string to the serial port
/*
	do
  	{  
		// Send byte to the Serial display  

 		send_byte(*s);                                // Write character directly to the LCD
 
  	} while( *s++);
*/

#ifdef DEBUG
	do
  	{  
		// Send byte to the Serial display  

 		send_byte(*s);                                // Write character directly to the LCD
 
  	} while( *s++);
#else

  printf("%s", s);

 
#ifdef LCD_DRIVER
 	if (LCD_Ready)
	{
	
		// Send the string 

		do
  		{  
			// Send byte to the LCD display  

 			WriteLCD(*s);                                // Write character directly to the LCD
 
  		} while( *s++);
  	}
  	
#endif
#endif
 
}

//******************************************************************************
//******************************************************************************
//******************************************************************************

#ifdef STDIO_INTERRUPTS
//******************************************************************************
//* RxDataWaiting - Checks to see if the Serial Ring Buffer has data to be read.  
//* Returns TRUE if there is data to be read and returns FALSE otherwise.  
//******************************************************************************

byte     RxDataWaiting(void)
{
    byte Status;

    // Check to see if the serial input ring buffer is empty
    
    if (NumberCharacters > 0)        
    //if (EndBuffer != StartBuffer)
	 {
       Status = TRUE;                    // There is data to be read from Serial Ring Buffer
    }
    else
	 {
       Status = FALSE;                   // Serial Ring Buffer is empty
    }

    return Status;

}


//******************************************************************************
//* ReadNoWait - Read next character from the RS-232 Port (USART) without 
//* waiting.  The function returns a 0 if the ring buffer is empty.   This function
//* is similar to the Inkey function in Basic.
//******************************************************************************
 
byte	ReadNoWait(void)
{

	char	Data;                                     // Data read from USART
       
#ifdef DEBUG

    // Get data from input buffer (MPSIM)

   Data = InputBuffer[InputBufferIndex++];
   return (Data);   	 

#endif

    if (EndBuffer != StartBuffer)
	 {

    	// Disable general interrupts

    	//INTCONbits.GIE = 0;                                    // Disable all interrupts

    	//  Get the oldest byte received from the serial input ring buffer.  
    
    	//Data = RingBuffer[StartBuffer];
    	Data = RingBuffer[StartBuffer];
 
    	StartBuffer++;
		StartBuffer = StartBuffer & (BUFFER_SIZE - 1);    // Wrap the StartBuffer pointer if necessary

    	NumberCharacters  = EndBuffer - StartBuffer;                                   // Decrement the number of characters currently in the buffer
 
      if (NumberCharacters < 0)
      {
      	NumberCharacters = 0;
      }
 
    	// Enable general interrupts

    	//INTCONbits.GIE = 1;                                    // Enable all interrupts
    }
    else
	 {
		Data = 0;
    }

    return (Data);
}
                 
// Serial input with interrupts and ring buffer

//******************************************************************************
//* get_byte - Get next character from the RS-232 Port (USART).  The function 
//* returns a -1 if the routine times out. 
//******************************************************************************
 
byte	get_byte(void)
{

	char	Data[2];                                     // Data read from USART
    	
	// Disable general interrupts

   //INTCONbits.GIE = 0;                                    // Disable all interrupts

	Data[1] = 0;
    
   // Get next valid data character from the serial input ring buffer
       
	// Check for and filter out any CR characters that indicate the end of serial input

   while (RingBuffer[StartBuffer] == CR) 
	{
		StartBuffer++;
      StartBuffer = StartBuffer & (BUFFER_SIZE - 1);    // Wrap the StartBuffer pointer if necessar

 	}

	// Decrement the number of characters currently in the buffer
 	NumberCharacters--;                                   
    if (NumberCharacters < 0)
    {
     	NumberCharacters = 0;
        Data[0] = 0;
    }
	else
	{
		// There is still data to be read from buffer
 
   		//Data = RingBuffer[StartBuffer];
   		Data[0] = RingBuffer[StartBuffer];

   		StartBuffer++;
   		StartBuffer = StartBuffer & (BUFFER_SIZE - 1);    // Wrap the StartBuffer pointer if necessary
	}
    
   
   //printf("trace 7: ReceivedDataFlag = %d  NumberCharacters = %d  Data = !%c' \r\n", ReceivedDataFlag, NumberCharacters, Data[0]);
   //printf("StartBuffer = %d, EndBuffer = %d \r\n", StartBuffer, EndBuffer);

   // Enable general interrupts

   //INTCONbits.GIE = 1;                                    // Enable all interrupts
 

   return (Data[0]);
}

#else

// No serial interrupts or ring buffer

//******************************************************************************
//* get_byte - Get next character from the RS-232 Port (USART).  The function 
//* returns a -1 if the routine times out. 
//******************************************************************************
 
byte	get_byte(void)
{

	char	Data[2];                                     // Data read from USART
 
	Data[1] = 0;
      
#ifdef DEBUG

    // Get data from input buffer (MPSIM)

   Data[0] = InputBuffer[InputBufferIndex++];
   return (Data[0]);   	 

#endif

  
    // First check to see if data is available from the USART and then read it if 
    // it has been received.  
 
    Data[0] = GetByte(); 
  
 
    return (Data[0]);
}	
#endif

#ifdef NEW_CODE


//******************************************************************************
//* get_byte - Get next character from the RS-232 Port (USART).  The function 
//* returns a -1 if the routine times out. 
//******************************************************************************
 
byte	get_byte(void)
{

	char	Data[2];                                     // Data read from USART
 
	Data[1] = 0;
      
#ifdef DEBUG

    // Get data from input buffer (MPSIM)

   Data[0] = InputBuffer[InputBufferIndex++];
   return (Data[0]);   	 

#endif

#ifdef STDIO_INTERRUPTS

#ifdef TIMEOUT_SUPPORT

  	Timeout = 0;
	TimeoutFlag = FALSE;
 
   	// Wait for the bytes to arrive from Host via USART
    
    	do
 	{      
        	Timeout++;    // Increment timeout count
      
        	if (Timeout >= Maxtimeout) 
		{

			// Display error message to console

 			printf("Device did not respond ! \r\n");
                       
            		TimeoutFlag = TRUE;
          
        	}
      
        // Delay and let the Host perform other duties (This delay may cause serial I/O problems !)
        
        //Pause(200);
        
      
    } while ((!RxDataWaiting) && (TimeoutFlag != TRUE));
    
    // Check for timeout

	if (TimeoutFlag) 
	{
        SerialCommunicationsError = -2;              // Communications timeout error
        Data[0] = Null;
        goto ErrorExit;
	}
    
ErrorExit:
    ReceivedDataFlag = FALSE;
 
#endif

   
   // Wait for next valid character from ring buffer
   
   //while (!RxDataWaiting);
   
   if (NumberCharacters > 0)
   {
   	Data[0] = ReadNoWait();
   }

 
#else

/*
    // First check to see if data is available from the USART and then read it if 
    // it has been received.  

    if (DataRdy1USART())
    {
    	Data[0] = Read1USART();
    }
    else
    {
    	Data[0] = 0;
    }
*/
    // First check to see if data is available from the USART and then read it if 
    // it has been received.  
 
    gets1USART(Data,1);
  
#endif

    return (Data[0]);
}

#endif

//******************************************************************************
//* send_byte - Send character to RS-232 Port (USART)
//******************************************************************************
void            send_byte(byte c)
{
 
#ifdef DEBUG

   	// Write data to output buffer (MPSIM)

   	OutputBuffer[OutputBufferIndex++] = c;


#else 

#ifdef LCD_DRIVER
 	if (LCD_Ready)
	{
 		WriteLCD(c);                                // Write character directly to the LCD
	}
#endif

   	// Write data to USART

   	putchar(c);

 

#ifdef FULL_DUPLEX
	// Discard the echo character if full duplex serial I/O is being used.

    	Echo = get_byte();
#endif

	// Delay between sending bytes in order to allow the Host to respond

	//pause(1);

#endif

}

//******************************************************************************
//* Function Name:    GetByte                                                  *
//* Description:      Similar to stdio getch.                                  *   
//*                                                                            *
//******************************************************************************
byte GetByte(void)
{
	byte c;
/*
     if(U1MODEbits.PDSEL == 3)
		return (U1RXREG);
	else
		return (U1RXREG & 0xFF);
*/

	while (!RX_Data_Received);

	RX_Data_Received = FALSE;
	return TheByte;

}


//******************************************************************************
//* Function Name:    SendByte                                                 *
//* Description:      Similarto stdio putch.                                   *   
//*                                                                            *
//******************************************************************************
void SendByte(byte c)
{
	while (!U1STAbits.TRMT);
	U1TXREG = c;
}



/******************************************************************************
 * gets1 - Receive a string of characters from the RS-232 port with no echo.  Terminates
 *        when new line character (CR) is encountered and replaces it with a
 *        null string terminator.
 ******************************************************************************/
char *gets1(char *s)
{
  char *p;
  int c;

  p = s;
  c = 0;

  while (c != 13)
  {
    do
    {
      *s = getchar();
      c = (int) *s;
#ifdef TEST
      /* printf("CLIENT: gets1: c = %c \n\n",c);  */
#endif

    } while((c == EOF) || (c == 0));
 
    s++;
  }

  *s = '\0'; 
  s = p;
  return p;
}

#ifdef STDIO_INTERRUPTS

//******************************************************************************
//* ReadHexByte - Read 2 hex digits representing an 8-Bit unsigned byte from the 
//* USART and convert them to an 8-Bit unsigned byte.
//******************************************************************************

byte ReadHexByte(void)
{
	// Wait for valid data from the serial port
	
	//while (!ReceivedDataFlag);
	
	if (NumberCharacters >= 2 )
	{ 
                HexNumber[0] = toupper(get_byte());        // Get first hex digit (MSB)
                HexNumber[1] = toupper(get_byte());        // Get second hex digit (LSB)
                HexNumber[2] = 0;                          // String terminator
   }
   else
   {
    	HexNumber[0] = '0';        					// Get first hex digit (MSB)
   	HexNumber[1] = '0';        					// Get second hex digit 
   	HexNumber[2] = 0;        					   // String terminator
  	}
   
	ReceivedDataFlag = FALSE;										// Set data flag to FALSE 

   // Combine the two digit hex value and convert it to a byte

   return(htob(HexNumber));
} 

#else

//******************************************************************************
//* ReadHexByte - Read 2 hex digits representing an 8-Bit unsigned byte from the 
//* USART and convert them to an 8-Bit unsigned byte.
//******************************************************************************

byte ReadHexByte(void)
{
                HexNumber[0] = toupper(get_byte());        // Get first hex digit (MSB)
                HexNumber[1] = toupper(get_byte());        // Get second hex digit (LSB)
                HexNumber[2] = 0;                          // String terminator

                // Combine the two digit hex value and convert it to a byte

                return(htob(HexNumber));
} 

#endif

#ifdef STDIO_INTERRUPTS

//******************************************************************************
//* ReadHexWord - Read 4 hex digits representing an 16-Bit unsigned word value
//* from the USART and convert them to a 16-Bit unsigned word.
//******************************************************************************

word ReadHexWord(void)
{
	// Wait for valid data from the serial port
	
	//while (!ReceivedDataFlag);
	
	if (NumberCharacters >= 4 )
	{ 
   	HexNumber[0] = toupper(get_byte());        // Get first hex digit (MSB)
   	HexNumber[1] = toupper(get_byte());        // Get second hex digit 
   	HexNumber[2] = toupper(get_byte());        // Get third hex digit  
   	HexNumber[3] = toupper(get_byte());        // Get fourth hex digit (LSB)
  		HexNumber[4] = 0;                          // String terminator
   }
   else
   {
    	HexNumber[0] = '0';        					// Get first hex digit (MSB)
   	HexNumber[1] = '0';        					// Get second hex digit 
   	HexNumber[2] = '0';        					// Get third hex digit  
   	HexNumber[3] = '0';        					// Get fourth hex digit (LSB)
  		HexNumber[4] = 0;                         // String terminator
  	}
   
	ReceivedDataFlag = FALSE;										// Set data flag to FALSE 

   // Combine the four digit hex value and convert it to a 16-Bit word
   
   // Flush the serial ring buffer to synchronize the data 
   
   FlushBuffer();

  	return(htow(HexNumber));
} 

#else

//******************************************************************************
//* ReadHexWord - Read 4 hex digits representing an 16-Bit unsigned word value
//* from the USART and convert them to a 16-Bit unsigned word.
//******************************************************************************

word ReadHexWord(void)
{
                HexNumber[0] = toupper(get_byte());        // Get first hex digit (MSB)
                HexNumber[1] = toupper(get_byte());        // Get second hex digit 
                HexNumber[2] = toupper(get_byte());        // Get third hex digit  
                HexNumber[3] = toupper(get_byte());        // Get fourth hex digit (LSB)
                HexNumber[4] = 0;                          // String terminator

                // Combine the four digit hex value and convert it to a 16-Bit word

                return(htow(HexNumber));
} 

#endif


#ifdef NETWORK_SUPPORT

//******************************************************************************
//* ReadHexInteger - Read 4 hex digits representing a 16-Bit integer value from
//* the USART and convert them to a 16-Bit signed integer.
//******************************************************************************

int ReadHexInteger(void)
{
                HexNumber[0] = toupper(get_byte());        // Get first hex digit (MSB)
                HexNumber[1] = toupper(get_byte());        // Get second hex digit 
                HexNumber[2] = toupper(get_byte());        // Get third hex digit  
                HexNumber[3] = toupper(get_byte());        // Get fourth hex digit (LSB)
                HexNumber[4] = 0;                          // String terminator

                // Combine the four digit hex value and convert it to a 16-Bit word

                return(htoi(HexNumber));
}  

//******************************************************************************
//* ReadHexLongInteger - Read 8 hex digits representing a 32-Bit integer value from
//* the USART and convert them to a 32-Bit signed integer.
//******************************************************************************

long int ReadHexLongInteger(void)
{
                HexNumber[0] = toupper(get_byte());        // Get first hex digit (MSB)
                HexNumber[1] = toupper(get_byte());        // Get second hex digit 
                HexNumber[2] = toupper(get_byte());        // Get third hex digit 
                HexNumber[3] = toupper(get_byte());        // Get fourth hex digit 
                HexNumber[4] = toupper(get_byte());        // Get fith hex digit 
                HexNumber[5] = toupper(get_byte());        // Get sixth hex digit 
                HexNumber[6] = toupper(get_byte());        // Get seventh hex digit 
                HexNumber[7] = toupper(get_byte());        // Get eighth hex digit (LSB)
                HexNumber[8] = 0;                          // String terminator

                // Combine the four digit hex value and convert it to a 16-Bit word

                return(htol(HexNumber));
}  

//******************************************************************************
//* ReadHexUnsignedLongInteger - Read 8 hex digits representing a 32-Bit unsigned
//* long integer value from the USART and convert them to a 32-Bit signed integer.
//******************************************************************************

long unsigned int ReadHexUnsignedLongInteger(void)
{
                HexNumber[0] = toupper(get_byte());        // Get first hex digit (MSB)
                HexNumber[1] = toupper(get_byte());        // Get second hex digit 
                HexNumber[2] = toupper(get_byte());        // Get third hex digit 
                HexNumber[3] = toupper(get_byte());        // Get fourth hex digit 
                HexNumber[4] = toupper(get_byte());        // Get fith hex digit 
                HexNumber[5] = toupper(get_byte());        // Get sixth hex digit 
                HexNumber[6] = toupper(get_byte());        // Get seventh hex digit 
                HexNumber[7] = toupper(get_byte());        // Get eighth hex digit (LSB)
                HexNumber[8] = 0;                          // String terminator

                // Combine the four digit hex value and convert it to a 16-Bit word

                return(htoul(HexNumber));
}  

//******************************************************************************
//* ReadHexFloat - Read 8 hex digits representing a 32-Bit float value from the 
//* USART and convert them to a 32-Bit IEEE float number.
//******************************************************************************

float ReadHexFloat(void)
{
                HexNumber[0] = toupper(get_byte());        // Get first hex digit (MSB)
                HexNumber[1] = toupper(get_byte());        // Get second hex digit 
                HexNumber[2] = toupper(get_byte());        // Get third hex digit 
                HexNumber[3] = toupper(get_byte());        // Get fourth hex digit 
                HexNumber[4] = toupper(get_byte());        // Get fith hex digit 
                HexNumber[5] = toupper(get_byte());        // Get sixth hex digit 
                HexNumber[6] = toupper(get_byte());        // Get seventh hex digit 
                HexNumber[7] = toupper(get_byte());        // Get eighth hex digit (LSB)
                HexNumber[8] = 0;                          // String terminator

                // Combine the four digit hex value and convert it to a 16-Bit word

                return(htof(HexNumber));
}  

#endif

//******************************************************************************
//* LookupHex - Returns the decimal equivalent (0..15) of any hex digit (0..F).
//******************************************************************************

byte LookupHexValue(char HexValue)
{
 
	byte i;							// Loop index
        byte DecimalValue;				// Decimal equivalent of hex digit (nibble)

     // Hex loop-up table pointer

	for (i=0; i<16; i++)
	{
		if (toupper(HexValue) == HexLookup[i])
		{
			DecimalValue  = i ;
			goto ExitLookupHexValue;
		}
	}

ExitLookupHexValue:

 
		return (DecimalValue);
                                 
}

//******************************************************************************
//* ltoh - Convert a 32-Bit signed integer range (-?????..?????) to a hex word string 
//* range (00000000..FFFFFFFF).
//******************************************************************************
void ltoh(long int LongIntValue, HexLongWord_T HexValue)
{
   
    byte Byte1,Byte2,Byte3,Byte4;
    HexByte_T HexByte1, HexByte2, HexByte3, HexByte4;

    // Extract separate bytes from 32-Bit word

    SplitLongInteger(LongIntValue, &Byte1, &Byte2, &Byte3, &Byte4);   

    // Convert each byte to a hex byte string

    btoh(Byte1, HexByte1);
    btoh(Byte2, HexByte2);
    btoh(Byte3, HexByte3);
    btoh(Byte4, HexByte4);
  
    // Set the hex string value to be returned   

    HexValue[0] = HexByte1[0];
    HexValue[1] = HexByte1[1]; 
    HexValue[2] = HexByte2[0]; 
    HexValue[3] = HexByte2[1]; 
    HexValue[4] = HexByte3[0]; 
    HexValue[5] = HexByte3[1]; 
    HexValue[6] = HexByte4[0]; 
    HexValue[7] = HexByte4[1]; 
    HexValue[8] = 0;  

}


//*******************************************************************************
//* htob - Converts from a 2 character hex string range (00..FF) to an unsigned
//* byte range (0..255).
// ******************************************************************************

byte htob(byte *s )
{
  byte Value;
  byte Lsn, Msn;

  Msn = LookupHexValue(s[0]);        // (MSN)
  Lsn = LookupHexValue(s[1]);        // (LSN)

  //Value = Lsn + Msn * 16;
  Value = (Msn<<4) + Lsn;

  return (Value);
}  

//*******************************************************************************
//* htow - Converts from a 4 character hex string range (0000..FFFF) to a 
//* 16-Bit unsigned word range (0..65535).
// ******************************************************************************

word htow(byte *s )
{  

    typedef volatile union  
    {
 
    word  WordValue;
    struct {
        byte Byte1:4;               // LSN
        byte Byte2:4;                
        byte Byte3:4;                
        byte Byte4:4;               // MSN
 
    } ;
 
  } Word_T ; 

  Word_T Value; 

  // Convert each Ascii hex digit (nibble) to binary and store it in the
  // appropriate bit position within the float value.

  Value.Byte4 = LookupHexValue(s[0]);       // (MSN)
  Value.Byte3 = LookupHexValue(s[1]);        
  Value.Byte2 = LookupHexValue(s[2]);      
  Value.Byte1 = LookupHexValue(s[3]);       // (LSN)

  return (Value.WordValue); 

}


//*******************************************************************************
//* htoi - Converts from a 4 character hex string range (0000..FFFF) to a 
//* 16-Bit integer range (-32768..32767).
// *******************************************************************************

int htoi(byte *s )
{
   
   typedef volatile union  
   {
 
    word  IntValue;
    struct {
        byte Byte1:4;               // LSN
        byte Byte2:4;                
        byte Byte3:4;                
        byte Byte4:4;               // MSN
 
    } ;
 
  } Int_T ; 

  Int_T Value; 

  // Convert each Ascii hex digit (nibble) to binary and store it in the
  // appropriate bit position within the float value.

  Value.Byte4 = LookupHexValue(s[0]);       // (MSN)
  Value.Byte3 = LookupHexValue(s[1]);        
  Value.Byte2 = LookupHexValue(s[2]);      
  Value.Byte1 = LookupHexValue(s[3]);       // (LSN)

  return (Value.IntValue); 

  
}

#ifdef NETWORK_SUPPORT

//*******************************************************************************
//* htol - Converts from a 8 character hex string range (00000000..FFFFFFFF) to a 
//* 32-Bit signed integer range (-????..????).
// *******************************************************************************

long int htol(byte *s )
{
    typedef volatile union  
    {
 
    long  LongValue;
    struct {
        byte Byte1:4;               // LSN
        byte Byte2:4;                
        byte Byte3:4;                
        byte Byte4:4;                
        byte Byte5:4;                
        byte Byte6:4;                
        byte Byte7:4;                
        byte Byte8:4;               // MSN
 
    } ;
 
  } Long_T ; 

  Long_T Value; 

  // Convert each Ascii hex digit (nibble) to binary and store it in the
  // appropriate bit position within the float value.

  Value.Byte8 = LookupHexValue(s[0]);       // (MSN)
  Value.Byte7 = LookupHexValue(s[1]);        
  Value.Byte6 = LookupHexValue(s[2]);     
  Value.Byte5 = LookupHexValue(s[3]);     
  Value.Byte4 = LookupHexValue(s[4]);     
  Value.Byte3 = LookupHexValue(s[5]);     
  Value.Byte2 = LookupHexValue(s[6]);      
  Value.Byte1 = LookupHexValue(s[7]);       // (LSN)

  return (Value.LongValue); 
}

//*******************************************************************************
//* htoul - Converts from a 8 character hex string range (00000000..FFFFFFFF) to a 
//* 32-Bit unsigned long integer range (0..????).
// *******************************************************************************

unsigned long int htoul(byte *s )
{
    typedef volatile union  
    {
 
    unsigned long  UnsignedLongValue;
    struct {
        byte Byte1:4;               // LSN
        byte Byte2:4;                
        byte Byte3:4;                
        byte Byte4:4;                
        byte Byte5:4;                
        byte Byte6:4;                
        byte Byte7:4;                
        byte Byte8:4;               // MSN
 
    } ;
 
  } UnsignedLong_T ; 

  UnsignedLong_T Value; 

  // Convert each Ascii hex digit (nibble) to binary and store it in the
  // appropriate bit position within the float value.

  Value.Byte8 = LookupHexValue(s[0]);       // (MSN)
  Value.Byte7 = LookupHexValue(s[1]);        
  Value.Byte6 = LookupHexValue(s[2]);     
  Value.Byte5 = LookupHexValue(s[3]);     
  Value.Byte4 = LookupHexValue(s[4]);     
  Value.Byte3 = LookupHexValue(s[5]);     
  Value.Byte2 = LookupHexValue(s[6]);      
  Value.Byte1 = LookupHexValue(s[7]);       // (LSN)

  return (Value.UnsignedLongValue); 
}

//*******************************************************************************
//* htouf - Converts from a 8 character hex string range (00000000..FFFFFFFF) to a 
//* 32-Bit IEEE float.
// *******************************************************************************

float htof(byte *s )
{
 
  typedef volatile near union  {
 
    float  FloatValue;
    struct {
        byte Byte1:4;               // MSN
        byte Byte2:4;                
        byte Byte3:4;                
        byte Byte4:4;                
        byte Byte5:4;                
        byte Byte6:4;               
        byte Byte7:4;                
        byte Byte8:4;               // LSN
 
    } ;
 
  } Float_T ; 

  Float_T Value;

  // Convert each Ascii hex digit (nibble) to binary and store it in the
  // appropriate bit position within the float value.

  Value.Byte1 = LookupHexValue(s[0]);       // (MSN)
  Value.Byte2 = LookupHexValue(s[1]);        
  Value.Byte3 = LookupHexValue(s[2]);    
  Value.Byte4 = LookupHexValue(s[3]);        
  Value.Byte5 = LookupHexValue(s[4]);     
  Value.Byte6 = LookupHexValue(s[5]);        
  Value.Byte7 = LookupHexValue(s[6]);       
  Value.Byte8 = LookupHexValue(s[7]);       // (LSN)

    

  return (Value.FloatValue); 
}


//******************************************************************************
//* itoh - Convert a signed integer (-32768..32767) to hex a hex word string 
//* range (0000..FFFF).
//******************************************************************************
void itoh(int IntValue, HexWord_T HexValue)
{
    byte Byte1,Byte2;
    HexByte_T HexByte1, HexByte2;

    // Extract separate bytes from 16-Bit word

    SplitInteger(IntValue, &Byte1, &Byte2);   

    // Convert each byte to a hex byte string

    btoh(Byte1, HexByte1);
    btoh(Byte2, HexByte2);
  
    // Set the hex string value to be returned   

    HexValue[0] = HexByte1[0];
    HexValue[1] = HexByte1[1]; 
    HexValue[2] = HexByte2[0]; 
    HexValue[3] = HexByte2[1]; 
    HexValue[4] = 0;  
 
}


//******************************************************************************
//* ultoh - Convert a 32-Bit unsigned long integer range (0..?????) to a hex word string 
//* range (00000000..FFFFFFFF).
//******************************************************************************
void ultoh(unsigned long int UnsignedLongIntValue, HexLongWord_T HexValue)
{
   
    byte Byte1,Byte2,Byte3,Byte4;
    HexByte_T HexByte1, HexByte2, HexByte3, HexByte4;

    // Extract separate bytes from 32-Bit word

    SplitUnsignedLongInteger(UnsignedLongIntValue, &Byte1, &Byte2, &Byte3, &Byte4);   

    // Convert each byte to a hex byte string

    btoh(Byte1, HexByte1);
    btoh(Byte2, HexByte2);
    btoh(Byte3, HexByte3);
    btoh(Byte4, HexByte4);
  
    // Set the hex string value to be returned   

    HexValue[0] = HexByte1[0];
    HexValue[1] = HexByte1[1]; 
    HexValue[2] = HexByte2[0]; 
    HexValue[3] = HexByte2[1]; 
    HexValue[4] = HexByte3[0]; 
    HexValue[5] = HexByte3[1]; 
    HexValue[6] = HexByte4[0]; 
    HexValue[7] = HexByte4[1]; 
    HexValue[8] = 0;  

}        
                         
#endif


//****************************************************************************
//* btoh - Convert an unsigned byte range (0..255) to a hex byte string 
//* range (00..FF).
//******************************************************************************

void btoh(byte ByteValue, HexByte_T HexValue)
{
    byte Lsn, Msn;

    // Initialize
                                 
    Lsn = ByteValue & 0x0F;        			// Get Least Signifficant nibble
    Msn = (ByteValue & 0xF0) >> 4; 			// Get Most Signifficant nibble
    Msn = Msn & 0x0F;        				// Handle any overflow
  
    // Set the default hex value to 0x00 hex 

    HexValue[0] = 0;
    HexValue[1] = 0; 
    HexValue[2] = 0; 
                                 
    // Search hex lookup table for Hex digit value

    HexValue[0] = HexLookup[Msn];
    HexValue[1] = HexLookup[Lsn];
  
}

//******************************************************************************
//* wtoh - Convert a 16-Bit unsigned word range  (0..65535) to a hex word string 
//* range (00..FFFF).
//******************************************************************************
void wtoh(word WordValue, HexWord_T HexValue)
{
   
    byte Byte1,Byte2;
    HexByte_T HexByte1, HexByte2;

    // Extract separate bytes from 16-Bit word
	    SplitWord(WordValue, &Byte1, &Byte2);   

    // Convert each byte to a hex byte string

    btoh(Byte1, HexByte1);
    btoh(Byte2, HexByte2);
  
    // Set the hex string value to be returned   

    HexValue[0] = HexByte1[0];
    HexValue[1] = HexByte1[1]; 
    HexValue[2] = HexByte2[0]; 
    HexValue[3] = HexByte2[1]; 
    HexValue[4] = 0; 

}

//********************************************************************************
//* printf - Writes a string of characters in program memory to the USART  
//********************************************************************************
#ifdef PIC18F8720
void printf(const rom char *data)
{
/*
	int i;
  	do
  	{  
		// Send character byte to the serial port.  Handle any special control characters here. 

 		send_byte(*data);                                // Write character directly to the LCD

  	} while( *data++);
*/	
	putrs1USART(data);


#ifdef LCD_DRIVER
/*
	
	for (i=0; i<strlen((rom char *) data); i++)
	{
 		WriteLCD(data[i]);                                // Write character directly to the LCD
	}
*/
 	if (LCD_Ready)
	{	
  		do
  		{  
			// Send character byte to the LCD display.  Handle any special control characters here. 

 			//WriteLCD(*data);                                // Write character directly to the LCD
 			lcd_putc(*data);                                // Write character directly to the LCD
 
  		} while( *data++);
  	}

 
#endif

}
#endif

//****************************************************************************************
//* Converts a byte to an ascii string of 2 hex characters and writes                                                          
//* them to the USART.
//****************************************************************************************

int 		hex2(byte Value)
{   
    	HexByte_T HexByte;
 
 	// Convert binary value to an 8-Bit unsigned hex byte

        btoh(Value, HexByte);
 
        string((char *) HexByte);
 

	return TRUE;
}
			   
//****************************************************************************************
//* Converts an integer to an ascii string of 4 hex characters and writes                                                          
//* them to the USART.
//****************************************************************************************

int 		hex4(word Value)
{   
    	HexWord_T HexWord;
 
 	// Convert binary value to a 16-Bit unsigned hex word

        wtoh(Value, HexWord); 
 
        string((char *) HexWord);
 
 
	return TRUE;
}

//****************************************************************************************
// hex8 - Converts a 32-Bit unsigned integer to an ascii string of 8 hex characters and writes                                                          
// them to the USART.	
//****************************************************************************************
		   
int 		hex8(unsigned long int  Value)
{
	HexLongWord_T HexLongWord;

	// Convert binary value to a 32-Bit unsigned long hex word

        ltoh(Value, HexLongWord); 
 
        string((char *) HexLongWord);
 
	return TRUE;
}		   

 
//****************************************************************************************
//* dec - Converts an integer to an ascii string of numeric characters and writes 
//* them to the USART
//****************************************************************************************
  
int             dec(int Value)
{
    // Convert the value from integer to a numeric string and write it to the USART

	printf("%d", Value); 
    return TRUE;
}

//*********************************************************************
//* SplitWord - Breaks up a 16-bit unsigned word into 2 bytes.
//*********************************************************************
void SplitWord(word WordValue,     /* Word value  */
                  byte *DataByte1,  /* Current data byte (MSB) */
                  byte *DataByte2)  /* Current data byte (LSB) */
{
  *DataByte1 = (unsigned char)((WordValue & 0x0000FF00)>>8);           // MSB
  *DataByte2 = (unsigned char) (WordValue & 0x000000FF);               // LSB
}
 
//******************************************************************************
//* Member - Determines if a character, Ch, is an element of a given set.
//******************************************************************************
int Member(char Ch, char * Set)
{
	int i;
	int FoundFlag;

	FoundFlag =  FALSE;

	for (i = 0; i< strlen(Set); i++)
	{
		if (Ch == Set[i]) 
		{
			FoundFlag = TRUE;
			goto ExitLoop;
		}
	}
ExitLoop:
	return FoundFlag;
}

//******************************************************************************
//* pow10 - Computes 10 raised to the integer power of argument, x, 
//* range -10 <= x <= 10, using a table lookup.
//******************************************************************************
double pow10(int x)
{
	double TempValue;
 
	if ((x >= 0) && (x <= 10))
	{
   		TempValue = PlusPowerOf10[x];
	}
	else
	{
   		TempValue = MinusPowerOf10[-x];
	}

	return TempValue;
}


#ifdef SINGLE_PRECISION

//******************************************************************************
//* GetFloat - Scans a single precision floating point number from the keypad
//* or reads it from the serial port (USART). Also checks for the mantissa sign and 
//* checks for exponent the number is entered in exponential notation.  
//******************************************************************************
float GetFloat(void)
{	
	// Use GetDouble function for now

	return ((float) GetDouble());

}

#endif
 

//******************************************************************************
//* GetKey - Scans a character from the keypad or reads it from the serial 
//* port (USART) or reads it from the co-processor message buffer.    
//******************************************************************************
char GetKey(void)
{
	char TheKey;
#ifdef SPI_BUFFER_INTERFACE
	MessageBufferIndex++;
	MessageBufferIndex = MessageBufferIndex % MESSAGE_BUFFER_SIZE;
	TheKey = CoProcessorMessage.CalculateCommand.Buffer[MessageBufferIndex];
	
#else
	TheKey = get_byte();
#endif 
 	return TheKey;
}


#ifdef NETWORK_SUPPORT

//****************************************************************************************
// tofloat - Converts a float to an ascii string of numeric characters and writes                                                          
//* them to the USART.
//****************************************************************************************
int             tofloat(float Value)    
{		     
  	// ********************************** Missing logic **********************************

	return TRUE;
}


//*********************************************************************
//* SplitInteger - Breaks up a 16-bit signed integer into 2 bytes.
//*********************************************************************
void SplitInteger(int IntValue,     /* Integer value  */
                  byte *DataByte1,  /* Current data byte (MSB) */
                  byte *DataByte2)  /* Current data byte (LSB) */
{
  *DataByte1 = (unsigned char)((IntValue & 0x0000FF00)>>8);           // MSB
  *DataByte2 = (unsigned char) (IntValue & 0x000000FF);               // LSB
}



//*********************************************************************
//* SplitUnsignedLongInteger - Breaks up a 32-bit unsigned long integer 
//*into 4 bytes.
//*********************************************************************
void SplitUnsignedLongInteger(unsigned long int UnsignedLongIntValue,     /* Long integer value   */
                  byte *DataByte1,  /* Current data byte (MSB) */
                  byte *DataByte2,  /* Current data byte  */
                  byte *DataByte3,  /* Current data byte  */
                  byte *DataByte4)  /* Current data byte (LSB) */
{
  *DataByte1 = (unsigned char) (UnsignedLongIntValue >>24);                        // MSB
  *DataByte2 = (unsigned char)((UnsignedLongIntValue & 0x00FF0000l)>>16);
  *DataByte3 = (unsigned char)((UnsignedLongIntValue & 0x0000FF00l)>>8);
  *DataByte4 = (unsigned char) (UnsignedLongIntValue & 0x000000FFl);               // LSB
}
#endif

#ifdef DEBUG
   
//********************************************************************************
//* putrs1USART - Writes a string of characters in ROM memory to the USART (stub)
//********************************************************************************
void putrs1USART(auto const MEM_MODEL rom char *data)
//void            putrs1USART(const rom char *data)
{
    OutputBuffer[OutputBufferIndex++] = *data;                          // Copy character to output buffer index
    OutputBufferIndex = (OutputBufferIndex % BUFFER_SIZE);
 
    return;

 
}
 
#endif

//*********************************************************************
//* SplitLongInteger - Breaks up a 32-bit long signed integer into 4 bytes.
//*********************************************************************
void SplitLongInteger(long int LongIntValue,     /* Long integer value   */
                  byte *DataByte1,  /* Current data byte (MSB) */
                  byte *DataByte2,  /* Current data byte  */
                  byte *DataByte3,  /* Current data byte  */
                  byte *DataByte4)  /* Current data byte (LSB) */
{
  *DataByte1 = (unsigned char) (LongIntValue >>24);                        // MSB
  *DataByte2 = (unsigned char)((LongIntValue & 0x00FF0000l)>>16);
  *DataByte3 = (unsigned char)((LongIntValue & 0x0000FF00l)>>8);
  *DataByte4 = (unsigned char) (LongIntValue & 0x000000FFl);               // LSB
} 

#ifdef NETWORK_SUPPORT

	// Support for PC to PIC Networking using the USART


/*****************************************************************************
 * ComputeChecksum - Function to compute a message packet checksum.
 *****************************************************************************/

void ComputeChecksum(Packet_U *Message, word *Checksum)
{
  int i;                            /* Local loop index  */
  word LocalChecksum;                    /* Local message checksum  */

  // Calculate message checksum of message data only 

  LocalChecksum = 0;
  for (i=0; i < DATA_SIZE; i++)
  {
    /* Calculate local checksum */

    LocalChecksum += Message->PacketData.Data[i];
  }

  *Checksum = LocalChecksum;                  /* Return computed checksum */


}

//*****************************************************************************
//* ComputeChecksum - Function to compute a short message packet checksum.   
//*****************************************************************************

void ComputeShortChecksum(ShortPacketData_T *ShortMessage, word *Checksum)
{
  int i;                            // Local loop index
  word LocalChecksum;               // Local message checksum

  // Calculate checksum of message data only

  LocalChecksum = 0;
  for (i=0; i < SHORT_DATA_SIZE; i++)
  {
    // Calculate local checksum

    LocalChecksum += ShortMessage->Data[i];
 
  }

  *Checksum = LocalChecksum;                  // Return computed checksum


}

/*****************************************************************************
 * SendMessage - Send next message packet to the PC server and wait for
 *               acknowledgement from it.
 *****************************************************************************/

void   SendMessage(void)
{
  int i;                            /* Local loop Index   */

  int Delay;
  int Try;                              /* Current number of tries */
  Packet_U *LocalMessage;

  LocalMessage = &Message;

  /*****************************************************************/
  /* Test code                                                     */
/*
  for (i=0; i < PACKET_SIZE; i++)
  {
    Message.PacketData.Data[i] = (unsigned int) (i+10000);
  }
*/
  /*****************************************************************/

  /* Compute message packet checksum before packet is sent to the PC server */

  ComputeChecksum(LocalMessage, &TheChecksum);
  Message.PacketData.Data[PACKET_CHECKSUM_OFFSET] = TheChecksum;

#ifdef TEST
  /*****************************************************************/
  /* Test code                                                     */

  printf("CLIENT: trace 123: TheChecksum = %lx \n", TheChecksum);

  /*****************************************************************/
#endif

  NumberOfTries = 0;
  
    /* Prepare the PC server to receive a message packet */

    putch(RECEIVE_MESSAGE_PACKET);

     // Wait for next valid message to be sent from the PC server.

    Try = 0;
    do                 
    {

      PCCommand = getch();
#ifndef TEST
      //*******************************************************************
      //* Test code
      //printf("CLIENT: trace 48: Try = %d, c = %c, %d \n",Try, PCCommand, PCCommand);
      //*******************************************************************
#endif

      // Check to see if maximum number of tries has been exceeded

      Try++;
      if (Try > MAX_NUMBER_OF_TRIES)
      {
        printf("CLIENT: Communications error in ReceiveMessage, check connections !!! \n");
        return;
      }

    } while (PCCommand != SEND_MESSAGE_PACKET);
    //} while (PCCommand != 'R');

   /* Send acknowledge command to the PC server.  */

   putch(COMMAND_RECEIVED);
   //putch('O');

  for (i=0; i < PACKET_SIZE; i++)
  {
    /* Send next 32-bit word */

    DataWord = Message.PacketData.Data[i];


    /* Extract 4 data bytes from current data word */

    //ExtractBytes(DataWord, &DataByte1, &DataByte2, &DataByte3, &DataByte4);

    /* Send individual ascii hex message data bytes to the PC server */

    //printf("%02X", DataByte1);                 /* (MSB} */
    //printf("%02X", DataByte2);
    //printf("%02X", DataByte3);
    //printf("%02X", DataByte4);                 /* (LSB)  */

/*
    putch(DataByte1);
    putch(DataByte2);
    putch(DataByte3);
    putch(DataByte4);
*/

  }

  /* Wait for acknowledgement that the message packet was received from
     the PC server. */

  do
  {
    PCAcknowledge = getch();
  } while (PCAcknowledge != PACKET_RECEIVED_OK);
  //} while (PCAcknowledge != 'O');

  /* Send acknowledge command to the PC server.  */

  putch(COMMAND_RECEIVED);
  //putch('O');

#ifdef TEST
  /*****************************************************************/
  /* Test code                                                     */

  printf("CLIENT: SendMessage Packet sent OK ! \n");   
  /*****************************************************************/
#endif

}

/*****************************************************************************
 * SendShortMessage - Send short message packet to the PC server and wait for
 *               acknowledgement from it.
 *****************************************************************************/

void   SendShortMessage(void)
{
  int i;                            /* Local loop Index   */

  int Delay;
  int Try;                              /* Current number of tries */

  ShortPacketData_T *LocalShortMessage;

  LocalShortMessage = &ShortMessage;

  /*****************************************************************/
  /* Test code                                                     */
/*
  for (i=0; i < PACKET_SIZE; i++)
  {
    Message.PacketData.Data[i] = (unsigned int) (i+10000);
  }
*/
  /*****************************************************************/

  /* Compute message packet checksum before packet is sent to the PC server */

  ComputeShortChecksum(LocalShortMessage, &TheChecksum);
  ShortMessage.Data[SHORT_PACKET_CHECKSUM_OFFSET] = TheChecksum;

#ifdef TEST
  /*****************************************************************/
  /* Test code                                                     */

  printf("CLIENT: trace 123: LocalShortChecksum = %x \n", TheChecksum);

  /*****************************************************************/
#endif

  NumberOfTries = 0;
  
  /* Prepare the PC server to receive a short message packet */

  putch(RECEIVE_SHORT_MESSAGE_PACKET);
 
     // Wait for next valid message to be sent from the PC server.

    Try = 0;
    do                 
    {
      PCCommand = getch();
#ifndef TEST
      //*******************************************************************
      //* Test code
      //printf("CLIENT: trace 48s: Try = %d, c = %c, %d \n",Try, PCCommand, PCCommand);
      //*******************************************************************
#endif

      // Check to see if maximum number of tries has been exceeded

      Try++;
      if (Try > MAX_NUMBER_OF_TRIES)
      {
        printf("CLIENT: Communications error in ReceiveMessage, check connections !!! \n");
        return;
      }

    } while (PCCommand != SEND_SHORT_MESSAGE_PACKET);
    //} while (PCCommand != 'R');

   /* Send acknowledge command to the PC server.  */

   putch(COMMAND_RECEIVED);
   //putch('O');

  for (i=0; i < SHORT_PACKET_SIZE; i++)
  {
    /* Send next 32-bit word */

    DataByte1 = ShortMessage.Data[i];

    /* Send individual ascii hex message data bytes to the PC server */

    //printf("%02X", DataByte1);                 /* (MSB} */

  }

  /* Wait for acknowledgement that the message packet was received from
     the PC server. */

  do
  {
    PCAcknowledge = getch();
  } while (PCAcknowledge != PACKET_RECEIVED_OK);
  //} while (PCAcknowledge != 'O');

  /* Send acknowledge command to the PC server.  */

  putch(COMMAND_RECEIVED);
  //putch('O');

#ifdef TEST
  /*****************************************************************/
  /* Test code                                                     */

  printf("CLIENT: SendShortmessage Packet sent OK ! \n");   
  /*****************************************************************/
#endif

}

/*****************************************************************************
 * ReceiveMessage - Receive next message from the PC server and acknowledge
 *                  it.
 *****************************************************************************/

void   ReceiveMessage(void)
{
  int i;                                /* Local loop Index  */
  int j;                                /* Local loop Index   */
  char StringValue[9];                  /* Used to read an ascii hex value  */
  unsigned char PCCommand;              /* Command from PC server */
  int Try;                              /* Current number of tries */

  NumberOfTries = 0;

     // Wait for next valid message data packet to be sent from the PC server.

    Try = 0;
    do
    {
      PCCommand = getch();
#ifndef TEST
      //*******************************************************************
      //* Test code
      //printf("CLIENT: trace 49: Try = %d, c = %c, %d \n",Try, PCCommand, PCCommand);
      //*******************************************************************
#endif

      // Check to see if maximum number of tries has been exceeded

      Try++;
      if (Try > MAX_NUMBER_OF_TRIES)
      {
        printf("CLIENT: Communications error in ReceiveMessage, check connections !!! \n");
        return;
      }

    } while (PCCommand != RECEIVE_MESSAGE_PACKET);
    //} while (PCCommand != 'R');

   /* Send acknowledge command to the PC server.  */

   putch(COMMAND_RECEIVED);
   //putch('O');


  /* Read next message and calculate its checksum  */

  for (i=0; i < PACKET_SIZE; i++)
  {
    /* Read next 32-bit word  */


    /* This statement will read a string which is
       terminated with 0, from the PC server */
/*
    for (j=0; j < 8; j++)
    {
      StringValue[j] = getch();
    }
    StringValue[8] = '\0';
*/
    gets1(StringValue);

    /* Convert hex string to unsigned integer value  */

//    sscanf(StringValue, "%lx", &DataWord);
#ifdef TEST
    printf("CLIENT: trace 50: i = %d, DataWord = %lx,  StringValue = '%s'\n", i,
           DataWord, StringValue);
#endif

    /* Compose 32-bit word from 4 data bytes  */

    /*
      DataWord = 0;
      DataWord = (DataByte1<<24) + (DataByte2<<16) + (DataByte3<<8) + DataByte4;
   */

    Message.PacketData.Data[i] = DataWord;

  }


  /* Calculate message checksum  */

   //Checksum = ComputeChecksum(Message);
#ifdef TEST
  /*****************************************************************/
  /* Test code                                                     */

  printf("CLIENT: trace 51: Checksum = %lx \n", Checksum);

  /*****************************************************************/
#endif

  /* Check message checksum to insure that it was received correctly.  If the
     local checksum does not agree with the checksum contained in the message
     then send a command to the PC server indicating BAD message, so that it can
     transmit the current message again.  Otherwise send acknowledge command
     to the PC server.   */

  if (Checksum != Message.PacketData.Data[PACKET_CHECKSUM_OFFSET])
  {
    NumberOfTries++;
    //printf("CLIENT: Did not receive message packet # %d after %d tries\n",
    //       Message.PacketData.Data[PACKET_ID_OFFSET], NumberOfTries);

    /* Check to see if maximum number of tries has been exceeded  */

    if (NumberOfTries > MAX_NUMBER_OF_TRIES)
    {
      printf("CLIENT: Communications error, check connections !!! \n");

      return;
    }

    /* Send command to the PC server to re-transmit last message */

    putch(SEND_MESSAGE_PACKET);

  }
  else
  {
#ifdef TEST
      //*******************************************************************
      //* Test code
      printf("CLIENT: trace 52: \n");
      //*******************************************************************
#endif

    // Send acknowledge to PC server that packet was received.

    putch(PACKET_RECEIVED_OK);

    // Wait for command acknowledgement from PC server.

    Try = 0;

    do
    {

      PCCommand = getch();
#ifndef TEST
      //*******************************************************************
      //* Test code
      //printf("CLIENT: trace 60: Try = %d, c = %c, %d \n",Try, PCCommand, PCCommand);
      //*******************************************************************
#endif

      // Check to see if maximum number of tries has been exceeded

      Try++;
      if (Try > MAX_NUMBER_OF_TRIES)
      {
        printf("CLIENT: Communications error in ReceiveMessage, check connections !!! \n");
        return;
      }

    } while (PCCommand != COMMAND_RECEIVED);
    //} while (PCCommand != 'R');

  }

}
/*****************************************************************************
 * ReceiveShortMessage - Receive short message from the PC server and acknowledge
 *                  it.
 *****************************************************************************/

void   ReceiveShortMessage(void)
{
  int i;                                /* Local loop Index  */
  int j;                                /* Local loop Index   */
  char StringValue[9];                  /* Used to read an ascii hex value  */
  unsigned char PCCommand;              /* Command from PC server */
  int Try;                              /* Current number of tries */

  ShortPacketData_T *LocalShortMessage;

  LocalShortMessage = &ShortMessage;

  NumberOfTries = 0;

     // Wait for next valid message data packet to be sent from the PC server.

    Try = 0;
    do
    {
      PCCommand = getch();
#ifndef TEST
      //*******************************************************************
      //* Test code
      //printf("CLIENT: trace 49s: Try = %d, c = %c, %d \n",Try, PCCommand, PCCommand);
      //*******************************************************************
#endif

      // Check to see if maximum number of tries has been exceeded

      Try++;
      if (Try > MAX_NUMBER_OF_TRIES)
      {
        printf("CLIENT: Communications error in ReceiveMessage, check connections !!! \n");
        return;
      }

    } while (PCCommand != RECEIVE_SHORT_MESSAGE_PACKET);
    //} while (PCCommand != 'R');

   /* Send acknowledge command to the PC server.  */

   putch(COMMAND_RECEIVED);
   //putch('O');


  /* Read next message and calculate its checksum  */

  for (i=0; i < SHORT_PACKET_SIZE; i++)
  {
    /* Read next 32-bit word  */


    /* This statement will read a string which is
       terminated with 0, from the PC server */
/*
    for (j=0; j < 8; j++)
    {
      StringValue[j] = getch();
    }
    StringValue[8] = '\0';
*/
    gets1(StringValue);

    /* Convert hex string to unsigned integer value  */

//    sscanf(StringValue, "%x", &DataByte1);
#ifdef TEST
    printf("CLIENT: trace 50: i = %d, DataWord = %lx,  StringValue = '%s'\n", i,
           DataWord, StringValue);
#endif


    ShortMessage.Data[i] = DataByte1;

  }


  /* Calculate message checksum  */

  ComputeShortChecksum(LocalShortMessage, &TheChecksum);

#ifdef TEST
  /*****************************************************************/
  /* Test code                                                     */

  printf("CLIENT: trace 51: Checksum = %x \n", Checksum);

  /*****************************************************************/
#endif

  /* Check message checksum to insure that it was received correctly.  If the
     local checksum does not agree with the checksum contained in the message
     then send a command to the PC server indicating BAD message, so that it can
     transmit the current message again.  Otherwise send acknowledge command
     to the PC server.   */

  if (Checksum != ShortMessage.Data[SHORT_PACKET_CHECKSUM_OFFSET])
  {
    NumberOfTries++;
    //printf("CLIENT: Did not receive short message packet after %d tries\n",
    //       NumberOfTries);

    /* Check to see if maximum number of tries has been exceeded  */

    if (NumberOfTries > MAX_NUMBER_OF_TRIES)
    {
      printf("CLIENT: Communications error, check connections !!! \n");

      return;
    }

    /* Send command to the PC server to re-transmit last message */

    putch(SEND_SHORT_MESSAGE_PACKET);

  }
  else
  {
#ifdef TEST
      //*******************************************************************
      //* Test code
      printf("CLIENT: trace 52: \n");
      //*******************************************************************
#endif

    // Send acknowledge to PC server that packet was received.

    putch(PACKET_RECEIVED_OK);

    // Wait for command acknowledgement from PC server.

    Try = 0;

    do
    {
      PCCommand = getch();
#ifndef TEST
      //*******************************************************************
      //* Test code
      //printf("CLIENT: trace 60s: Try = %d, c = %c, %d \n",Try, PCCommand, PCCommand);
      //*******************************************************************
#endif

      // Check to see if maximum number of tries has been exceeded

      Try++;
      if (Try > MAX_NUMBER_OF_TRIES)
      {
        printf("CLIENT: Communications error in ReceiveMessage, check connections !!! \n");
        return;
      }

    } while (PCCommand != COMMAND_RECEIVED);
    //} while (PCCommand != 'R');

  }

}


/*********************************************************************
 * UnitTest - Used to test the stdio functions.
 ********************************************************************/

void UnitTest(void)
{
  int i,j;


  // Test the Send Short Message function here...

  for (j=0; j<5; j++)
  {

    printf("CLIENT: Calling UnitTest - SendShortMessage ");
    dec(j);
    printf(" times... \r\n");

    for (i=0; i<SHORT_DATA_SIZE; i++)
    {
      ShortMessage.Data[i] = i;
    }

    SendShortMessage();
  }
  printf("CLIENT: End of Unit Test - SendShortMessage \n");

  // Test the Receive Short Message function here...

  for (j=0; j<5; j++)
  {

    printf("CLIENT: Calling UnitTest - ReceiveShortMessage ");
    dec(j);
    printf(" times... \r\n");

    for (i=0; i<SHORT_DATA_SIZE; i++)
    {
      ShortMessage.Data[i] = 0;
    }

    ReceiveShortMessage();

    for (i=0; i<SHORT_DATA_SIZE; i++)
    {
      printf("CLIENT: i = ");
      dec(i);
      printf(" ShortMessage.Data = ");
      dec(ShortMessage.Data[i]);
      printf("\r\n");
    }
  }

  printf("CLIENT: End of Unit Test - ReceiveShortMessage. \n");


  // Test the Send Long Message function here...

  for (j=0; j<5; j++)
  {

    printf("CLIENT: Calling UnitTest - SendMessage ");
    dec(j);
    printf(" times... \r\n");

    for (i=0; i<DATA_SIZE; i++)
    {
      Message.PacketData.Data[i] = i;
    }

    SendMessage();
  }
  printf("CLIENT: End of Unit Test - SendMessage. \n");

  // Test the Receive Message function here...

  for (j=0; j<5; j++)
  {

    printf("CLIENT: Calling UnitTest - ReceiveMessage ");
    dec(j);
    printf(" times... \r\n");

    for (i=0; i<DATA_SIZE; i++)
    {
      Message.PacketData.Data[i] = 0;
    }

    ReceiveMessage();

    for (i=0; i<DATA_SIZE; i++)
    {
      printf("CLIENT: i = ");
      dec(i);
      printf(" Message.PacketData.Data = ");
      dec(Message.PacketData.Data[i]);
      printf("\r\n");
    }
  }
  printf("CLIENT: End of Unit Test - ReceiveMessage. \n");

}


#endif
 
