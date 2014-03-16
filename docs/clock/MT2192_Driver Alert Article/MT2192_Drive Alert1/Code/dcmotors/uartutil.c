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
//* micro-controller are not lost.  These functions are for the DSPIC30F2010.    *
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
//#include <stdio.h>
#include "my_types.h"
#include "uartutil.h"

// Try this code to initialize the UART on the DSPIC30F2010....

 
//#define PLL_VALUE 4.0
//#define PLL_VALUE 8.0
#define PLL_VALUE 16.0

#define CLOCK_TICKS 7372800
#define TCY_PER_SECOND (PLL_VALUE*CLOCK_TICKS/4.0)


#define STRING_SIZE 80										// String Size
//char Buf[10];										//Received data is stored in array Buf
//char * Receiveddata=Buf;    								//Receiveddata is a char pointer whose address is the same as Buf[80]

byte TheByte = 0;
byte TheByte2 = 0;
byte RX_Data_Received = FALSE;
byte RX_Data_Received_2 = FALSE;

void __attribute__((__interrupt__)) _U1TXInterrupt(void)	//This is the UART1 transmit ISR
{
 	IFS0bits.U1TXIF=0;
}
void __attribute__((__interrupt__)) _U2TXInterrupt(void)	//This is the UART2 transmit ISR
{
 	IFS1bits.U2TXIF=0;
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

void __attribute__((__interrupt__)) _U2RXInterrupt(void)	//This is the UART2 Receive ISR
{
	LED_2 = ON;											// Toggle LED 2 when a character is received
    RX_Data_Received_2 =  TRUE;
	while (!U2STAbits.URXDA);
    TheByte2 = U1RXREG;
	IFS1bits.U2RXIF=0;										// Clear UART2 RX flag
 
/*
	while(DataRdyUART1())                 						//Read the receive buffer as long as at least one or more characters 
															//can be read
	 {
  	( *( Receiveddata)++)=ReadUART1();  
 	}
*/

}

 
#ifdef UARTUTIL_TEST

int main(void)
{
  	byte c;

	//char Txdata[]__attribute__((aligned(2)))={'I','S','R',' ','D','r','i','v','e','n',' ', 'S','e','r','i','a','l', ' ',
	//									  'T','e','s','t','\r','\n','\0'};     //Data to be transmitted

	unsigned int baudvalue;

	unsigned int U1MODEvalue;

	unsigned int U1STAvalue;
	int i; 

	// Initialize the UART

	//InitializeUart1(115200);
	//InitializeUart1(9600);
	InitializeUart2(115200);
	// Configure all analog pins as digital

	ADPCFG = 0xFFFF;

   // Initialize 16-Bit Digital I/O ports (PORTE does not exist for the DSPIC30FXXXX)

    //PORTB = 0x0000;                                          // Clear PORTB
    PORTE = 0x0000;                                          // Clear PORTE
 
    // Initialize port I/O direction

    //TRISB = 0x000F;                                          // PORT B all inputs and outputs.
    TRISE = 0x0000;                                          // PORT E all outputs.
 

	LED_2 = OFF;
	LED_2_DIR = OUTPUT;

	while(1)
	{
		for (i=0; i<1000; i++)
		{
			PORTE = i;
			Printf_2("This is a test ");  
            Dec_2(i);
            Printf_2(" ");
            Hex_2(i);
            Printf_2("\r\n");
			LED_2 = ~LED_2;
		}
 
	}

	LED_1 = OFF;
	LED_1_DIR = OUTPUT;


	while(1)
	{
		for (i=0; i<1000; i++)
		{
			PORTE = i;
			Printf("This is a test ");  
            Dec(i);
            Printf(" ");
            Hex(i);
            Printf("\r\n");
			LED_1 = ~LED_1;
		}
 
	}

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


/*


	putsUART1 ((unsigned int *)Txdata);

	while(BusyUART1());


	while(DataRdyUART1())
	{
 		(*(Receiveddata)++)=ReadUART1();

		Printf("Receiveddata = %s \r\n", Receiveddata);

	}
	CloseUART1();
*/
	return 0;

}
#endif

//  Stamp and stdio equivalent functions for DSPIC30XXXX    

 
//********************************************************************************
//* InitializeUart1 - Initializes variables and registers for UART1 used by stdio 
//* serial I/O functions.
//********************************************************************************

void            InitializeUart1(unsigned long The_Baud_Rate)
{
 	unsigned  uartmode,uartsta;			// Uart configuration variables
 

  	unsigned long baud,U1MODEvalue,U1STAvalue;
	CloseUART1();

//	InitPorts();

	ConfigIntUART1(UART_RX_INT_EN & UART_RX_INT_PR6 & UART_TX_INT_DIS & UART_TX_INT_PR2);   // Works!!!
 
	// Configure the UART 1 here ...
 
	// Compute the baud rate constant needed to configure UART1 
 	baud = (((THE_CLOCK_SPEED/The_Baud_Rate) /16) - 1);

//	baud=(((unsigned long)TCY_PER_SECOND)/(16*baudRate))-1; 
	U1MODEvalue=UART_EN & UART_RX_TX & UART_DIS_LOOPBACK & UART_NO_PAR_8BIT & UART_1STOPBIT;
	U1STAvalue =UART_INT_TX_BUF_EMPTY & UART_TX_PIN_NORMAL & UART_TX_ENABLE & UART_INT_RX_CHAR & UART_ADR_DETECT_DIS & UART_RX_OVERRUN_CLEAR;

	OpenUART1(U1MODEvalue,U1STAvalue,baud);


/*
	// Configure the UART 1 here ...
 
	// Compute the baud rate constant needed to configure UART1 
 	baud = (((THE_CLOCK_SPEED/THE_BAUD_RATE) /16) - 1);

	InitPorts();

    SetupUART();

   	U1BRG = baud;
*/
 
}
//********************************************************************************
//* InitializeUart2 - Initializes variables and registers for UART1 used by stdio 
//* serial I/O functions.
//********************************************************************************

void            InitializeUart2(unsigned long The_Baud_Rate)
{
 	unsigned  uartmode,uartsta;			// Uart configuration variables
 

  	unsigned long baud,U2MODEvalue,U2STAvalue;
	CloseUART1();

//	InitPorts();

	ConfigIntUART2(UART_RX_INT_EN & UART_RX_INT_PR6 & UART_TX_INT_DIS & UART_TX_INT_PR2);   // Works!!!
 
	// Configure the UART 1 here ...
 
	// Compute the baud rate constant needed to configure UART1 
 	baud = (((THE_CLOCK_SPEED/The_Baud_Rate) /16) - 1);

//	baud=(((unsigned long)TCY_PER_SECOND)/(16*baudRate))-1; 
	U2MODEvalue=UART_EN & UART_RX_TX & UART_DIS_LOOPBACK & UART_NO_PAR_8BIT & UART_1STOPBIT;
	U2STAvalue =UART_INT_TX_BUF_EMPTY & UART_TX_PIN_NORMAL & UART_TX_ENABLE & UART_INT_RX_CHAR & UART_ADR_DETECT_DIS & UART_RX_OVERRUN_CLEAR;

	OpenUART2(U2MODEvalue,U2STAvalue,baud);


/*
	// Configure the UART 1 here ...
 
	// Compute the baud rate constant needed to configure UART1 
 	baud = (((THE_CLOCK_SPEED/THE_BAUD_RATE) /16) - 1);

	InitPorts();

    SetupUART();

   	U1BRG = baud;
*/
 
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
//* Function Name:    GetByte2                                                  *
//* Description:      Similar to stdio getch.                                  *   
//*                                                                            *
//******************************************************************************
byte GetByte2(void)
{
	byte c;
/*
     if(U1MODEbits.PDSEL == 3)
		return (U1RXREG);
	else
		return (U1RXREG & 0xFF);
*/

	while (!RX_Data_Received_2);

	RX_Data_Received_2 = FALSE;
	return TheByte2;

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

//******************************************************************************
//* Function Name:    SendByte2                                                 *
//* Description:      Similarto stdio putch.                                   *   
//*                                                                            *
//******************************************************************************
void SendByte2(byte c)
{
	while (!U2STAbits.TRMT);
	U2TXREG = c;
}
//******************************************************************************
//* Function Name:    Printf                                                   *
//* Description:      Similar to stdio printf for UART1.                                  *   
//*                                                                            *
//******************************************************************************
void Printf(char *s)
{
	int k;
  
    for (k=0; k<strlen(s); k++)
    {
		SendByte(s[k]);
	}

}
//******************************************************************************
//* Function Name:    Printf_2                                                   *
//* Description:      Similar to stdio printf for UART2.                                  *   
//*                                                                            *
//******************************************************************************
void Printf_2(char *s)
{
	int k;
  
    for (k=0; k<strlen(s); k++)
    {
		SendByte2(s[k]);
	}

}

//******************************************************************************
//* Function Name:    Dec                                                      *
//* Description:      LCD equivalent to Parallax Stamp dec function that       *                 
//*                   displays an integer Value in ASCII format.               * 
//*                                                                            *
//******************************************************************************
void Dec(int Value) 
{
	char s[15];

	sprintf(s, "%d", Value);
    Printf(s);
   
}
//******************************************************************************
//* Function Name:    Dec_2                                                      *
//* Description:      LCD equivalent to Parallax Stamp dec function that       *                 
//*                   displays an integer Value in ASCII format.               * 
//*                                                                            *
//******************************************************************************
void Dec_2(int Value) 
{
	char s[15];

	sprintf(s, "%d", Value);
    Printf_2(s);
   
}

//******************************************************************************
//* Function Name:    Hex2                                                     *
//* Description:      LCD equivalent to Parallax Stamp hex2 function that      *                 
//*                   displays a byte Value as a 2 digit hex character.        *                                          
//******************************************************************************
void Hex2(byte Value)
{
	char s[15];

	sprintf(s, "0x%02X", Value);
    Printf(s);
 
}

//******************************************************************************
//* Function Name:    Hex2_2                                                     *
//* Description:      LCD equivalent to Parallax Stamp hex2 function that      *                 
//*                   displays a byte Value as a 2 digit hex character.        *                                          
//******************************************************************************
void Hex2_2(byte Value)
{
	char s[15];

	sprintf(s, "0x%02X", Value);
    Printf_2(s);
 
}

//******************************************************************************
//* Function Name:    Hex                                                      *
//* Description:      LCD equivalent to Parallax Stamp hex2 function that      *                 
//*                   displays a byte Value as a 2 digit hex character.        *                                          
//******************************************************************************
void Hex(word Value)
{
	char s[15];

	sprintf(s, "0x%0X", Value);
    Printf(s);
 
}      
//******************************************************************************
//* Function Name:    Hex_2                                                      *
//* Description:      LCD equivalent to Parallax Stamp hex2 function that      *                 
//*                   displays a byte Value as a 2 digit hex character.        *                                          
//******************************************************************************
void Hex_2(word Value)
{
	char s[15];

	sprintf(s, "0x%0X", Value);
    Printf_2(s);
 
}       


     
#ifdef NEW_CODE

//******************************************************************************
//* Function Name:    Puts                                                     *
//* Description:      Equivalent to stdio puts.                                *   
//*                                                                            *
//******************************************************************************
void Puts(char *s)
{
	int k;
  
    for (k=0; k<strlen(s); k++)
    {
		WriteChar(s[k]);
	}

}
        
//******************************************************************************
//* Function Name:    Printf                                                   *
//* Description:      Equivalent to stdio printf.                              *
//*                                                                            *
//******************************************************************************
int Printf(const char *format, ...)
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

    Puts(Buffer);

    return ret;

}

 //******************************************************************************
//* Function Name:    Puts                                                     *
//* Description:      Equivalent to stdio puts.                                *   
//*                                                                            *
//******************************************************************************
void Puts(char *s)
{
	int k;
  
    for (k=0; k<strlen(s); k++)
    {
		WriteChar(s[k]);
	}

}
              

#endif
