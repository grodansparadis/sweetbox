 
//********************************************************************************
//*                                                                              *
//* Description:                                                                 *
//*                                                                              *
//*        Name:  usb_motor.c                                                    *
//* Project Number:  MT2192                                                      *                                                   
//*           Date:  10/15/2007                                                  *                                                  
//*                                                                              *
//*   Description:                                                               * 
//*     This application contains all the data constants and structures          *
//* necessary for the USB portion of the 8-Bit SPI DC Motor Controller Master and*
//* Slave functions that are needed to send messages to the DC Motor Controller  *
//* slave application or to act as a Client to any SPI Slave devices.  These     * 
//* include devices include my SPI Math Co-processor, the Microchip's 24XCXX SPI *
//*  Memory chips, SPI based digital potentiometers and A/D devices, and SPI     *
//* Client/Server networking, using the USB and SPI hardware modules  contained  *
//* in the PIC18f4550 DSPIC30F4011 and  DSPIC30F6014 micro-controllers.          *
//*                                                                              *
//* 	The application writes data to the Math Co-Processor or EEPROM and reads *
//* back the data.  Also included is a diagnostic function that will write a     *
//* ramp (0..n) to the Math Co-Processor or EEPROM using SPI and read it back to *
//* the Host PC via the UART.                                                    *
//*                                                                              *
//*		The SPI networking is handled using a simple data packet messaging       *
//* protocol.  Each message includes an SPI source address field (Master), an SPI*
//* destination address field, the actual data and a CRC or checksum field.  Data*
//* packets that are determined to be corrupted will be retransmitted up to a    *
//* specified number of times.  The SPI Slave responds by reading these messages *
//* into local buffers and proceeds to process them.  Results are sent back to   *
//* the Master SPI processor via the SPI Bus.    These SPI networking functions  *
//* are also available to other applications that require the SPI interface.     *                                
//*                                                                              *
//* 	Message definitions are defined in the file spimsg.h which are common to * 
//* the spimast.c and spislave.c applications.  SPI transmit and receive buffers *
//* are available to process messages to/from the Master controller.             *                             
//*                                                                              *
//*                                                                              *
//*		The following are simple code examples illustrating the SSP module       *
//* communicating with a Microchip 24C080 SPI EE Memory Device. In all the       *
//* examples provided no error checking utilizing the value returned from a      *
//* function is implemented.                                                     *
//*                                                                              *
//*     This controller can be used with a PC host in order to  give any PC the  *
//* capability to serve as an SPI Master.  It can also be used for robotic       *
//* applications where multiple controllers and SPI devices are used, with a     *
//* minimum of data and control lines needed to connect them.  Each SPI device   *
//* has its own unique address and only listens to commands associated with it.  *
//*                                                                              *
//*     The SPI message format used by this application is for the master to     *
//* send commands to the Digital Pot SPI slave device.  This translates to the   *
//* following sequence of operations for the Master:                             *                    
//*                                                                              *
//*  2) Data is latched in on the rising edge of the SCK.                        *
//*                                                                              *
//*  3) Data is clocked out the SO pin after the falling edge of SCK.            *
//*                                                                              *
//*  4) The Clock Iddle is low, Mode 0,0.                                        *
//*                                                                              *
//*  5) The SPI Clock speed is FOSC/16                                           *
//*                                                                              *
//*  6) The Reset Pin, RS_BAR needs to be brough low to select the MCP3002 A/D.  *
//*                                                                              *
//*                                                                              *
//*  7) This device can also operate in SPI Mode 1,1, in which the clock idles   *
//*  in the high state.                                                          *
//*                                                                              *
//*  8) The following SPI data transfers are available:                          *
//*                                                                              *
//*    		Master sends data byte  -  Slave sends dummy data byte               *                                                                        
//*    		Master sends data byte  -  Slave sends data byte                     *                                                                  
//*    		Master sends dummy data byte  -  Slave sends data byte               *                                                                         
//*                                                                              * 
//*  9) The Receiver Buffer has a one byte delay which requires a data offset    *    
//*     into the SPI Receiver Buffer.                                            *                             
//*                                                                              *                                                                            
//*     The Slave responds with the following sequence of operations:            * 
//*                                                                              *
//*                                                                              *
//*                                                                              *
//*      Future upgrades to this application include: the ability to communicate * 
//* with other SPI slave devices such as Dallas Semiconductor temperature        *
//* sensors, Phillips serial SPI devices and other micro-controllers (both       *
//* master and slave).  It may also be used to communicate as a master or a      *
//* slave for applications requiring that dual capability.  Other upgrades       * 
//* include using SPI to implement the C printf function to be used for a "cheap"*
//* embedded debugger similar to JTAG used by high end micro-processors.         *
//* Explore using communication protocols and error detection/correction         *  
//* algorithms such as parity, CRC and various other codes for robust operation  * 
//* and fault tolerance. TMR and N-Module redundancy could also be investigated  *
//* for fault tolerant systems that work in hazardous environments.  The same    *
//* SPI networking functions may also be used with the asynchronous serial RS-232*
//* interface and the I2C interface.                                             *             
//*                                                                              *
//*                                                                              *
//********************************************************************************
/** I N C L U D E S **********************************************************/
#include <p18cxxx.h>
#include <usart.h>
#include <stdio.h>
#include <stdlib.h>

#include "system\typedefs.h"

#include "system\usb\usb.h"

#include "io_cfg.h"             // I/O pin mapping
#include "user\gateway.h"       // USB Gateway functions
#include "user\temperature.h"
#include "user\network.h"
#include "user\coproc.h"        // USB Messages 
#include "user\UARTIntC.h"      // USART Support 
#include "user\FLAGS.h"  

#define DELAY 10000    // Small delay
//#define DELAY 10    // Small delay
//#define DELAY 500    // Small delay

// Declare USB Buffer used for sending and receiving messages from to and from host
volatile byte USB_Buffer[UARTINTC_RX_BUFFER_SIZE];    // USB Data Buffer 

// Declare USART Master Buffer variables here

volatile byte USART_Rx_Buffer[UARTINTC_RX_BUFFER_SIZE];                  	// USART and SPI Receive Buffer
volatile byte USART_Tx_Buffer[UARTINTC_TX_BUFFER_SIZE];                  	// USART Transmit Buffer
volatile byte USART_Start_Buffer=0;                         		// Start pointer for ring buffer
volatile byte USART_End_Buffer = 0;								// USART End Buffer Index

volatile byte USART_Rx_Flag = FALSE;                        		// Indicates that data was received into the ring buffer
volatile int USART_Timeout = 0;                            		// USART I/O Timeout counter
volatile byte USART_Timeout_Flag;                           		// USART Timeout flag
volatile int USART_Number_Characters = 0;						 	// USART Number of characters in buffer
volatile int USART_Communications_Error;                    		// USART I/O error flag
                                                 		// -1 = USART Input Bufer overrun error
                                                 		// -2 = USART Communications timeout error
                                                 		// -3 = USART buffer empty error

volatile int USART_Interrupt_Count = 0;                   	// Number of MSSP interrupts
volatile int USART_Data_In = 0;								// Data byte read from the USART Master
volatile int USART_Data_Out = 0;							// Data byte sent to the USART Master
volatile int USART_Status;									// USART Status returned from routines
volatile int MessageIndex = 0;							// Message ID number
 
/** V A R I A B L E S ********************************************************/
#pragma udata
byte old_sw2,old_sw3;


//---Global Varibles --------------------
unsigned int Tx_IP = 0;    //SPI Transmit in progress flag
int Tx_Data;
int count = 0;
long int j;
int i = 0;
int Checksum = 0;
int Local_Checksum = 0;
int Message_Count = 0;
int Data;
 
rom char welcome[]={"PIC18F4550 Full-Speed USB - CDC RS-232 Emulation Demo\r\n\r\n"};
rom char ansi_clrscr[]={"\x1b[2J"};         // ANSI Clear Screen Command

//------Function Proto-types ----------
void main (void);
void SPI_INIT(void);
void SPI_WR(int);
int SPI_Read(void);
int SPI_Write(int SPI_Data);

void InterruptHandlerHigh(void);
void InitMotorController(void);
void Transfer_SPI_To_USB(void); 
void Transfer_USB_To_SPI(void);

/** P R I V A T E  P R O T O T Y P E S ***************************************/
void InitializeUSART(void);
void BlinkUSBStatus(void);
BOOL Switch2IsPressed(void);
BOOL Switch3IsPressed(void);

void Exercise_Example(void);

void Exercise_01(void);
void Exercise_02(void);
void Exercise_03(void);
void Exercise_04(void);
void Exercise_05(void);
void Exercise_06(void);
void Exercise_07(void);
void Exercise_08(void);
void Exercise_09(void);

#pragma code InterruptVectorHigh = 0x08
void InterruptVectorHigh (void)
{
    _asm
        goto InterruptHandlerHigh //jump to interrupt routine
    _endasm    
}

#pragma code
#pragma interrupt InterruptHandlerHigh
void InterruptHandlerHigh()
{
    int i;
    
    PIR1bits.SSPIF = 0;     //Clr SPI int flag
    USART_Rx_Buffer[count] = SSPBUF; //Rd Rx_SPI
    //count++;                //Inc read count
    LATAbits.LATA5 = 1;     //SS1 high
    for(i=0; i<16; i++);    //Delay
    Tx_IP = 0;              //Transmit Idle, In-Progress flag clr
}

/** D E C L A R A T I O N S **************************************************/
 
void UserInit(void)
{
    mInitAllLEDs();
    mInitAllSwitches();
    old_sw2 = sw2;
    old_sw3 = sw3;
    
    
#ifdef USART_SUPPORT
	// Configure the on-chip USART, SPI and USB hardware - works!!!
    InitializeUSART(); 


/*
	// Simple USART Test 1.  Works!!!
	for (i=0; i<100; i++)
	{ 
		sprintf(tempString, "This is a test %d times \r\n", i);
		Printf((char *) tempString);         
	}

	// Simple USART Test 2.  Works!!!
	while(1)
	{
		for (i=0; i<100; i++)
		{ 
			sprintf(tempString, "This is a test %d times \r\n", i);
			Printf((char *) tempString);         
		}
	}
*/
	
#endif  

#ifdef TEMP_SENSOR_SUPPORT
    // Initialize the temperature sensor
   InitTempSensor();
#endif
    
  
    InitMotorController();

 

}//end UserInit

//*******************************************************************************
//* InitMotorController - Configure the SPI interface to communicate with the
//* USB Motor Controller by initializing the SPI Master hardware for SPI Clock
//* of Fosc/64, clock Iddle low (SPI Mode 0,0), and sample data after the
//* falling edge of SCK.   
//******************************************************************************
void InitMotorController(void)
{
 	long int i,j;
	char Rx_Data[66];

	// Initialize the SPI Master Pin directions
	SPI_SCK_DIR = OUTPUT_PIN; 					// Set SPI Clock Direction output

 	SPI_SI_DIR = INPUT_PIN;						// Set SPI Input Direction input
 

    SPI_SO = 0;
    SPI_SO_DIR = OUTPUT_PIN; 					// Set SPI Output Direction output


	// Configure the SPI interface to communicate with USB Motor Controller
    CS_USB_MOTOR = 1;  
  
    TRIS_CS_USB_MOTOR = OUTPUT_PIN;
    TRIS_BUSY = INPUT_PIN;

	//SPI_CS_DIR = OUTPUT;					// Set SPI Slave select direction
	//SPI_CS = DISABLE; 						// Ensure SPI peripheral is disabled  

   
    //OpenSPI(SPI_FOSC_64, MODE_00, SMPMID);      // SPI Mode 0,0  - CKE=0, CKP=0   
 
   	SPI_INIT();

    // Test the USB Interface with SPI    Works!!!
    for (i=0; i<63; i++)
	{
		//USB_Buffer[i] = 64-i;
		Rx_Data[i] = 0;
		USB_Buffer[i] = 0;

	}
	//USB_Buffer[63] = 0;

 

}//end InitMotorController

void InitializeUSART(void)
{
    TRISCbits.TRISC7=1; // RX
    TRISCbits.TRISC6=0; // TX


	// Compute the baud rate constant needed to configure UART1 

    // Fosc/[4(N+1)] or Fosc/[16(N+1)] or Fosc/[64(N+1)] 
 	// baud = (((THE_CLOCK_SPEED/THE_BAUD_RATE) /4) - 1);   // BRGH = 1   BRG16 =1
 	// baud = (((THE_CLOCK_SPEED/THE_BAUD_RATE) /16) - 1);  // BRGH = 1   BRG16 =0
 	// baud = (((THE_CLOCK_SPEED/THE_BAUD_RATE) /64) - 1);  // BRGH = 0   BRG16 =0

   // Configure USART  
     // Setup the USART for 9600 baud 


#ifdef OSC_4_MHZ


   // Configure USART for 4 MHz, 9600 Baud, 8-Bits, No parity, Asynchronous
    //TRISC = 0xdb; 
    TXSTA = 0x20;                                          // setup USART transmit
    RCSTA = 0x90;                                          // setup USART receive
    
   TXSTAbits.BRGH = 1;
   SPBRG = 25;                                             // 9600 baud @ 4MHz

#endif

#ifdef OSC_20_MHZ    

  // Configure USART for 20 MHz, 9600 Baud, 8-Bits, No parity, Asynchronous
     //TRISC = 0xdb; 
    TXSTA = 0x20;                                          // setup USART transmit
    RCSTA = 0x90;                                          // setup USART receive

   //TXSTAbits.BRGH = 0;
   TXSTAbits.BRGH = 1;
   //SPBRG = 32;                                            // 9600 baud rate @ 20 MHz
   SPBRG = 64;                                            // 19200 baud rate @ 20 MHz
   //SPBRG = baud;                                            //Selected baud rate @ 20 MHz


#endif

#ifdef OSC_40_MHZ    

   // Configure USART for 40 MHz, 9600 Baud, 8-Bits, No parity, Asynchronous
 
   TXSTAbits.BRGH = 0;
   SPBRG = 64;                                            // 9600 baud @ 40 MHz

#endif

#ifdef OSC_50_MHZ    

   // Configure USART for 50 MHz, 9600 Baud, 8-Bits, No parity, Asynchronous
    //TRISC = 0xdb; 
    TXSTA = 0x20;                                          // setup USART transmit
    RCSTA = 0x90;                                          // setup USART receive
 
   TXSTAbits.BRGH = 0;
   SPBRG = 80;                                            // 9600 baud @ 50 MHz

#endif

#ifdef OSC_48_MHZ    

#ifdef BAUD_9600
    //TRISC = 0xdb; 
    TXSTA = 0x20;                                          // setup USART transmit
    RCSTA = 0x90;                                          // setup USART receive
    SPBRG = 78;
    SPBRGH = 0;      // 9600 Baud
    TXSTAbits.BRGH = 0;   
#endif

#ifdef BAUD_19200
    SPBRG = 0x71;
    SPBRGH = 0x02;      // 0x0271 for 48MHz -> 19200 baud   Works !!!!
    TXSTA = 0x24;       // TX enable BRGH=1
    RCSTA = 0x90;       // continuous RX
    BAUDCON = 0x08;     // BRG16 = 1
#endif

#ifdef BAUD_38400
     //TRISC = 0xdb; 
    TXSTA = 0x20;                                          // setup USART transmit
    RCSTA = 0x90;                                          // setup USART receive
   SPBRG = 18;
    SPBRGH = 0;      // 38400 Baud
    TXSTAbits.BRGH = 0;   
#endif

#ifdef BAUD_115200
    //TRISC = 0xdb; 
    TXSTA = 0x20;                                          // setup USART transmit
    RCSTA = 0x90;                                          // setup USART receive
    SPBRG = 25;
    SPBRGH = 0;      // 115200 Baud
    TXSTAbits.BRGH = 1;   
#endif

#endif

   
}//end InitializeUSART

/******************************************************************************
 * Function:        void ProcessIO(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is a place holder for other user routines.
 *                  It is a mixture of both USB and non-USB tasks.
 *
 * Note:            None
 *****************************************************************************/
void ProcessIO(void)
{   
    BlinkUSBStatus();
    // User Application USB tasks
    if((usb_device_state < CONFIGURED_STATE)||(UCONbits.SUSPND==1)) return;

    Exercise_Example();

    Exercise_01();  // Works!!!
    Exercise_02();    // Works!!!
    //Exercise_03();    // Works!!!
    //Exercise_04();    // Works!!!
    //Exercise_05();  // Temperature test Works!!!
    //Exercise_06();    // Works!!!
    //Exercise_07();    // Serial Communications USART Test

    //Transfer_USB_To_USART();    // Serial Communications from USB -> USART Test  Works!!!
    //Transfer_USART_To_USB();    // Serial Communications from USART -> USB Test
    Transfer_USB_To_SPI(); // Transfer data from (laptop USB motor application) USB to SPI  (dsPIC30F4011)  
    //Transfer_SPI_To_USB(); //  Transfer data from SPI (dsPIC30F4011) to USB (laptop USB sensor reader application) Works !!! 


}//end ProcessIO

void Exercise_Example(void)
{
    static byte start_up_state = 0;
    
    if(start_up_state == 0)
    {
        if(Switch2IsPressed())
            start_up_state++;
    }
    else if(start_up_state == 1)
    {
        if(mUSBUSARTIsTxTrfReady())
        {
            putrsUSBUSART(ansi_clrscr);
            start_up_state++;
        }
    }
    else if(start_up_state == 2)
    {
        if(mUSBUSARTIsTxTrfReady())
        {
            putrsUSBUSART("\rMicrochip Technology Inc., 2004\r\n");
            start_up_state++;
        }
    }
    else if(start_up_state == 3)
    {
        if(mUSBUSARTIsTxTrfReady())
        {
            putrsUSBUSART(welcome);
            start_up_state++;
        }
    }
    
}//end Exercise_Example

void Exercise_01(void)
{
    /*
     * Write code in this function that sends a literal null-terminated
     * string of text ("Hello World!\r\n") to the PC when switch 2 is
     * pressed.
     *
     * Useful functions:
     *  Switch2IsPressed() returns '1' when switch 2 is pressed.
     *  putrsUSBUSART(...);
     *
     * See examples in Exercise_Example();
     *
     * Remember, you must check if cdc_trf_state is ready for another
     * transfer or not. When it is ready, the value will equal CDC_TX_READY,
     * or use macro: mUSBUSARTIsTxTrfReady()
     */
     
    /* Insert code here - 3 lines */
    if (Switch2IsPressed())
	{
        if(mUSBUSARTIsTxTrfReady())
        {
            putrsUSBUSART("Hello World!\r\n");

	    }
    }
    /* End */
    
}//end Exercise_01

rom char ex02_string[]={"Calling all cars, testing one two three...\r\n"};
void Exercise_02(void)
{
    /*
     * Write code in this function that sends a null-terminated string
     * of text stored in program memory pointed to by "ex02_string" to
     * the PC when switch 3 is pressed.
     *
     * ex02_string is declared right above this function.
     *
     * Useful functions:
     *  Switch3IsPressed() returns '1' when switch 3 is pressed.
     *  putrsUSBUSART(...);
     *
     * See examples in Exercise_Example();
     *
     * Remember, you must check if cdc_trf_state is ready for another
     * transfer or not. When it is ready, the value will equal CDC_TX_READY,
     * or use macro: mUSBUSARTIsTxTrfReady()
     */

    /* Insert code here - 3 lines*/
    if (Switch3IsPressed())
	{
		if (mUSBUSARTIsTxTrfReady())
			putrsUSBUSART(ex02_string);
	}

    /* End */
    
}//end Exercise_02


void Exercise_03(void)
{
 	byte index, count, total_sum;

    /*
     * Write code in this function that reads data from USB and
     * toggles LED D4 when the data read equals ASCII character '1' (0x31)
     *
     * Useful functions:
     *  byte getsUSBUSART(char *buffer, byte len)   See cdc.c for details
     *  mLED_4_Toggle();
     *
     * Use USART_Rx_Buffer[] to store data read from USB.
     */

     /* Insert code here - 3 lines */

	 // Here is the working code
/*
     if (getsUSBUSART(USB_Buffer, 8)
  	 {
		count = mCDCGetRXLength();
		total_sum = 0;
		for (index =0; index < count; index++)
		{
			total_sum += USB_Buffer[index];
		}

	 }
*/
     if (getsUSBUSART(USB_Buffer, CDC_BULK_OUT_EP_SIZE))
     //if (getsUSBUSART(USB_Buffer, 1))
	 {
			// Route data USB End Point buffer to USART.  Works!!!
			//USB_Buffer[1] = 0;
			USB_Buffer[27] = 0;
			//Printf_USB((char *) USB_Buffer);    // Works but slows down transfers
			Printf((char *) USB_Buffer);      // Works!!!  

  	 }

	
/*
	 // Here is the working code
	 if (getsUSBUSART((char *) USB_Buffer, (byte) 1))    // Works!!!
     {
     	if (USB_Buffer[0] == '1')
        {  
          	mLED_4_Toggle();
    		Buffer[0] = 'H';
    		Buffer[1] = 'e';
    		Buffer[2] = 'l';
    		Buffer[3] = 'l';
    		Buffer[4] = 'o';
    		Buffer[5] = ' ';
    		Buffer[6] = 'W';
    		Buffer[7] = 'o';
    		Buffer[8] = 'r';
    		Buffer[9] = 'l';
    		Buffer[10] = '!';
    		Buffer[11] = '!';
    		Buffer[12] = '!';
    		Buffer[13] = '\r';
    		Buffer[14] = '\n';
   		 	Buffer[15] = 0;

   		 	if (mUSBUSARTIsTxTrfReady())   
        		putsUSBUSART((char *) Buffer);

			Printf((char *) Buffer);  
   	 	}
 	}





	 // Here is the working code
	 if (getsUSBUSART(input_buffer,1))    // Works!!!
     {
     	if (input_buffer[0] == '1')
            
          	mLED_4_Toggle();
   	 }


     Length = 10;

     if (getsUSBUSART(USART_Rx_Buffer, (byte) 1) != 0)
	 {
     	if (USART_Rx_Buffer[1] == '1')
	 	{
			mLED_3_Toggle();
			mLED_4_Toggle();

    		Buffer[0] = 'H';
    		Buffer[1] = 'e';
    		Buffer[2] = 'l';
    		Buffer[3] = 'l';
    		Buffer[4] = 'o';
    		Buffer[5] = ' ';
    		Buffer[6] = 'W';
    		Buffer[7] = 'o';
    		Buffer[8] = 'r';
    		Buffer[9] = 'l';
    		Buffer[10] = '!';
    		Buffer[11] = '!';
    		Buffer[12] = '!';
    		Buffer[13] = '\r';
    		Buffer[14] = '\n';
   		 	Buffer[15] = 0;

   		 	if (mUSBUSARTIsTxTrfReady())   
        		putsUSBUSART(Buffer);

			Printf(Buffer);  

     	}
	 }
*/


    /* End */
    
}//end Exercise_03

void Exercise_04(void)
{
    /*
     * Before starting Exercise_04(), comment out the call to Exercise_01()
     * in ProcessIO(); This function will need to check Switch2IsPressed().
     *
     * Write code in this function that sends the following 4 bytes of
     * data: 0x30,0x31,0x32,0x33 when switch 2 is pressed.
     * Note that these data is not null-terminated and is located in
     * the data memory.
     *
     * Useful functions:
     *  Switch2IsPressed() returns '1' when switch 2 is pressed.
     *  mUSBUSARTTxRam(byte *pData, byte len) See cdc.h for details.
     *
     * Use USART_Tx_Buffer[] to store the four bytes data.
     *
     * Remember, you must check if cdc_trf_state is ready for another
     * transfer or not. When it is ready, the value will equal CDC_TX_READY,
     * or use macro: mUSBUSARTIsTxTrfReady()
     */
    
    /* Insert code here - 7 lines */
    if (Switch2IsPressed())
	{

 
		USART_Tx_Buffer[0] = 0x30;
		USART_Tx_Buffer[1] = 0x31;
		USART_Tx_Buffer[2] = 0x32;
		USART_Tx_Buffer[3] = 0x33;
		if (mUSBUSARTIsTxTrfReady())
			mUSBUSARTTxRam((byte *) USART_Tx_Buffer, (byte) 4);            // Works!!!
	}

    /* End */
	
}//end Exercise_04

void Exercise_05(void)
{
    /*
     * The PICDEM Full-Speed USB Demo Board is equipped with a 
     * temperature sensor. See temperature.c & .h for details.
     *
     * All necessary functions to collect temperature data have
     * been called. This function updates the data a few times
     * every second. The program currently sends out the
     * temperature data to the PC via UART.
     *
     * You can check this by hooking up a serial cable and
     * set your serial port to 19200 baud, 8 bit data, 1 Stop,
     * no parity.
     *
     * The program assumes CPU Frequency = 48 MHz to generate
     * the correct SPBRG value for 19200 baud transmission.
     *
     * Modify the code to send the ASCII string stored in
     * tempString to the PC via USB instead of UART.
     *
     * The temperature data is stored in tempString array in
     * ASCII format and is null-terminated.
     *
     * Useful function:
     *  putsUSBUSART(...);
     *
     *  Note: It is 'puts' and not 'putrs' to be used here.
     *
     * Remember, you must check if cdc_trf_state is ready for another
     * transfer or not. When it is ready, the value will equal CDC_TX_READY,
     * or use macro: mUSBUSARTIsTxTrfReady()
     */
    
    static word ex05_count;
    char Line[64];
    int i = 0;

    if(ex05_count == 0)
    {
        AcquireTemperature();               // Read temperature from sensor   Works!!!
        UpdateCelsiusASCII();               // Convert to ASCII, stored in

         // Send temperature data to USB   
         // "tempString", See temperature.c
 	
 
    	USART_Tx_Buffer[0] = 'H';
    	USART_Tx_Buffer[1] = 'e';
    	USART_Tx_Buffer[2] = 'l';
    	USART_Tx_Buffer[3] = 'l';
    	USART_Tx_Buffer[4] = 'o';
    	USART_Tx_Buffer[5] = ' ';
    	USART_Tx_Buffer[6] = 'W';
    	USART_Tx_Buffer[7] = 'o';
    	USART_Tx_Buffer[8] = 'r';
    	USART_Tx_Buffer[9] = 'l';
    	USART_Tx_Buffer[10] = 'd';
    	USART_Tx_Buffer[11] = '\r';
    	USART_Tx_Buffer[12] = '\n';
    	USART_Tx_Buffer[13] = 0;

  
		Printf_USB((char *) USART_Tx_Buffer);   // Works
		Printf((char *) USART_Tx_Buffer);   // Works

/*
        Printf_USB((char *) tempString);    // Works !!!
        Printf((char *) tempString);        // Works !!!
*/


/*

         if (mUSBUSARTIsTxTrfReady())
		 { 
			sprintf(Line, " This is a test %d times !!!\r", i++); 
            //mUSBUSARTTxRam(Line, (byte) 30);
	  	 }
        if(mUSBUSARTIsTxTrfReady())             // Works!!!
        {
            putrsUSBUSART(welcome);            
        }

*/
         // Send temperature data to USART   
         //putsUSART(tempString);
    	 //sprintf(Line, " This is a test %d times !!!\r", i++); 
         //putsUSART(Line);

         ex05_count = 10000;      // Slow temperature updates   Works!!!
         //ex05_count = 5000;       // Fast temperature updates   Works!!!
         //ex05_count = 2500;       // Fast temperature updates   Works!!!
         //ex05_count = 1500;       // Fast temperature updates   Works!!!
         //ex05_count = 1250;       // Fast temperature updates   Works!!!
  
        /* End */
    }
    else
        ex05_count--;
    
}//end Exercise_05

static int State = 0;
void Exercise_06(void)
{
    int i;

    /*
     * Before starting Exercise_06(), comment out the call to Exercise_01()
     * in ProcessIO(); This function will need to check Switch2IsPressed().
     *
     * Write code in this function that sends the following 4 bytes of
     * data: 0x30,0x31,0x32,0x33 when switch 2 is pressed.
     * Note that these data is not null-terminated and is located in
     * the data memory.
     *
     * Useful functions:
     *  Switch2IsPressed() returns '1' when switch 2 is pressed.
     *  mUSBUSARTTxRam(byte *pData, byte len) See cdc.h for details.
     *
     * Use USART_Tx_Buffer[] to store the four bytes data.
     *
     * Remember, you must check if cdc_trf_state is ready for another
     * transfer or not. When it is ready, the value will equal CDC_TX_READY,
     * or use macro: mUSBUSARTIsTxTrfReady()
     */
    
    /* Insert code here - 7 lines */
    if (Switch2IsPressed())
	{

    	if ((State % 2) == 0)            // Works!!!
		{
			USART_Tx_Buffer[0] = 0x30;
			USART_Tx_Buffer[1] = 0x31;
			USART_Tx_Buffer[2] = 0x32;
			USART_Tx_Buffer[3] = 0x33;	
  			if (mUSBUSARTIsTxTrfReady())
    		{
				mUSBUSARTTxRam((byte *) USART_Tx_Buffer, (byte) 4);            
        		State++;
			}
		}
		else                             // Works!!!
		{
			USART_Tx_Buffer[0] = 0x34;
			USART_Tx_Buffer[1] = 0x35;
			USART_Tx_Buffer[2] = 0x36;
			USART_Tx_Buffer[3] = 0x37;
			if (mUSBUSARTIsTxTrfReady())
			{
				mUSBUSARTTxRam((byte *) USART_Tx_Buffer, (byte) 4);           
        		State++;
			}
		}

/*
		USART_Tx_Buffer[0] = 0x30;
		USART_Tx_Buffer[1] = 0x31;
		USART_Tx_Buffer[2] = 0x32;
		USART_Tx_Buffer[3] = 0x33;
		//sprintf(USART_Tx_Buffer, " This is a test %d times !!!\r", i++);   // sprintf does not seem to work here
 
		if (mUSBUSARTIsTxTrfReady())
			mUSBUSARTTxRam((byte *) USART_Tx_Buffer, (byte) 40);            // Works!!!
		//if (mUSBUSARTIsTxTrfReady())
		//	mUSBUSARTTxRam((byte *) USART_Tx_Buffer, (byte) 4);            // Works!!!

*/
	}  
  /* End */
	
}//end Exercise_06

//static int State = 0;
void Exercise_07(void)
{
	// Test data array
	unsigned char cArray[10]
	   = { 'A','B', 'C', 'D', 'E', 'F', 'G','H','I' , 'J'};
	
	// temporary local array	   
	unsigned char writtenArray[20];
	
	// local variables for all test cases and their initialisation
	unsigned char chData;
	unsigned char *pReadArray;
	unsigned char *pWriteArray;
	int j = 0;
	int i  = 0;
	int l = 0;
	int k = 0;			
	pReadArray = pWriteArray = writtenArray;	

 

    /*
     * Before starting Exercise_07(), comment out the call to Exercise_01() trough 
     * Exercise_06() in ProcessIO(); This function will need to check Switch2IsPressed().
     *
     * Write code in this function that sends the following 4 bytes of
     * data: 0x30,0x31,0x32,0x33 when switch 2 is pressed.
     * Note that these data is not null-terminated and is located in
     * the data memory.
     *
     * Useful functions:
     *  Switch2IsPressed() returns '1' when switch 2 is pressed.
     *  mUSBUSARTTxRam(byte *pData, byte len) See cdc.h for details.
     *
     * Use USART_Tx_Buffer[] to store the four bytes data.
     *
     * Remember, you must check if cdc_trf_state is ready for another
     * transfer or not. When it is ready, the value will equal CDC_TX_READY,
     * or use macro: mUSBUSARTIsTxTrfReady()
     */

	// Initialize buffer 
   	for (i=0; i<UARTINTC_TX_BUFFER_SIZE; i++)
	{
  			 
		//USART_Tx_Buffer[i] =  CoProcessorMessage.Message.Data[i];
		USART_Tx_Buffer[i] =  i + 48;
	}

	// First get the coprocessor's attention by synchronizing with it. The SYNC byte is sent and
    // an ACK response is expected

    //SyncWithSlave();

	// Test the printf command...   Works!!!
  
	//sprintf(USART_Tx_Buffer, "This is a test !\r\n");

    USART_Tx_Buffer[0] = 'H';
    USART_Tx_Buffer[1] = 'e';
    USART_Tx_Buffer[2] = 'l';
    USART_Tx_Buffer[3] = 'l';
    USART_Tx_Buffer[4] = 'o';
    USART_Tx_Buffer[5] = ' ';
    USART_Tx_Buffer[6] = 'W';
    USART_Tx_Buffer[7] = 'o';
    USART_Tx_Buffer[8] = 'r';
    USART_Tx_Buffer[9] = 'l';
    USART_Tx_Buffer[10] = 'd';
    USART_Tx_Buffer[11] = '\r';
    USART_Tx_Buffer[12] = '\n';
    USART_Tx_Buffer[13] = 0;

    if (mUSBUSARTIsTxTrfReady())     // Works!!!!
        putsUSBUSART((char *) USART_Tx_Buffer);

	Printf((char *) USART_Tx_Buffer);   // Works


	//}
/*	     	
	// Copy the response message to the SPI Tx buffer and send it back to the SPI Master  
  	//memcpy(USART_Tx_Buffer, &CoprocessorMessage.Message, sizeof(CoprocessorMessage.Message));    

	//**********************************************************************************
	// Send an USART message command here
	//**********************************************************************************
	// Test the printf command...
	while (1)
	{ 
		i = 0;
 		while (i<UARTINTC_TX_BUFFER_SIZE)
		{
   	   		
         											// We may want handshaking here..
 
   			//status flag
			if(!vUARTIntStatus.UARTIntTxBufferFull)                        
			{
				UARTIntPutChar(USART_Tx_Buffer[i]);		                   // Works!!!
				//UARTIntPutChar(cArray[i]);		
 				i++;
	 		}
  		
  		}
 
 	}
 	

*/

/*

// TEST 1
// Sending Test array data, 100 times, from micro-controller to PC
// and waiting infinitely at the end.

   for(j = 0; j < 100; j++)
	{
		k = 0;
		while(k < 10)
		{
			if(UARTIntPutChar(cArray[k]))
				 k++;			
		}		   
	}	
	while(1);
*/

// TEST 2
// Similar to TEST1  Works!!!!
/*
	for(j = 0; j < 100; j++)
	{
		k = 0;
		while(k < 20)
		{  
 
			if(!vUARTIntStatus.UARTIntTxBufferFull)    // Works!!!
			  UARTIntPutChar(cArray[k++]);
		}		   
	}	
	while(1);	
*/

/*
// My new code ....
	//sprintf(writtenArray, "This is a test %d times! \r\n", i++);
	for(j = 0; j < 100; j++)
	{
 
    	strcpy(writtenArray, "01234567890123456789");
        k = 0;
        while (writtenArray[k] != 0)
		{
			if(!vUARTIntStatus.UARTIntTxBufferFull)
			 UARTIntPutChar(writtenArray[k++]);
 
		}		   
	}	
	while(1);
	
*/	

	

// TEST 3
//	Sending Test array data, continuously, from micro-controller to PC
//  Usage of status flag can be seen in below example.  Works!!!!
	
/*	
	while(1)  
	{
		i = 0;
		while(i<10)
		{   //status flag
			if(!vUARTIntStatus.UARTIntTxBufferFull)
			{
				 UARTIntPutChar(cArray[i]);		
				 i++;
			 }
		 }		
	}
*/
	
/*	
//	TEST 4
//	Sending Test array data, continuously, from micro-controller to PC
// 	Usage of UARTIntGetTxBufferEmptySpace() function can be seen below.  Works!!!!
	
	
	while(1)
	{
		i = 0;
		while(i < 10) //works  change to pointer
		{   
			l = 0;
			l = UARTIntGetTxBufferEmptySpace();
			for(k = 0; k < l; k++)
			{   if((i+k) < 10)
				UARTIntPutChar(cArray[i+k]);			
			}
			i = i+ k;						
		}					
	}	

*/	


//	TEST 5
//  Checking transmission and reception simultaneously.
//  Sending Test array data, 100 times ,from micro-controller to PC.
//  During this transmission, a single character 'A' or 'B' or 'C' 
//  can be sent from PC to micro-controller. This can be seen as LED
//  glows on PICDEM2 board. At the end of transmission micro-controller
//  waits infinitely at the end.
	/*		
  
  	TRISB = 0;
	PORTB = 0;
    k = 0;

	while(k < 100)
	{
		for(j = 0; j < 100; j++)
		{
			i = 0;
			do{
				if(vUARTIntStatus.UARTIntTxBufferEmpty)
					UARTIntPutChar(cArray[i++]);
			}while(i < 10);
		}
		
		if( !(vUARTIntStatus.UARTIntRxError) && 
		    !(vUARTIntStatus.UARTIntRxOverFlow) &&
		    !(vUARTIntStatus.UARTIntRxBufferEmpty))
		{    
			if(UARTIntGetChar(&chData))
			PORTB = chData;
		}
		
		k++;
	}
	
	while(1);			
*/


//	TEST 6
//  Chunk of 30 characters can be sent from PC to micro-controller.
//  Each received character at micro-controller is read and immediately 
//	sent back to PC.
	/*
	
	while(1)
	{
		while(!vUARTIntStatus.UARTIntRxBufferEmpty)
		{
			UARTIntGetChar(&chData);			
			if(!vUARTIntStatus.UARTIntTxBufferFull)
				UARTIntPutChar(chData);
		};
	};

	*/


//	TEST 7
// 	Chunk of data is sent from PC to micro. Data received at micro is
//	read in to temporary buffer. The data is immediately sent back to 
//  PC. Usage of pointers is displayed in below example
	/*
	
	while(1)   
	{		   
		l = 0;
		l = UARTIntGetRxBufferDataSize();	
		while(l > 0)
		{
			if(UARTIntGetChar(pWriteArray))
			{				
				if(!vUARTIntStatus.UARTIntTxBufferFull)
				    UARTIntPutChar(*pWriteArray);
				l--; pWriteArray++;
				if(pWriteArray == (writtenArray + 10))
					pWriteArray = writtenArray;
			}
		}
	}
		
	*/
	

//	TEST 8
// 	Chunk of data is sent from PC to micro. Data received at micro is
//	read in to temporary buffer. The data is sent back to PC. Here 
//  reception and transmission is done independently.
	/*
	
	
	while(1)   
	{					   
		l = 0;
		l = UARTIntGetRxBufferDataSize();	
		while(l > 0)
		{
			if(UARTIntGetChar(pReadArray))
			{				
				l--; pReadArray++;
				if(pReadArray == (writtenArray + 10))
					pReadArray = writtenArray;
			}
		}		
		if((pWriteArray < pReadArray) || ((pWriteArray - pReadArray) == 9 ))
		{  
			if(!vUARTIntStatus.UARTIntTxBufferFull)
			{
				UARTIntPutChar(*pWriteArray);
	    		i--; pWriteArray++;
 			    if(pWriteArray == (writtenArray + 10))				  			
					pWriteArray = writtenArray;		
			}
		}									
	}
	
	*/
}//end Exercise_07

//*******************************************************************************
//*  Transfer_USB_To_USART - Route data USB End Point buffer to USART.   Works!!!
//******************************************************************************
void Transfer_USB_To_USART(void)
{
 	byte index, count, total_sum;
 

	 // Here is the working code
/*
     if (getsUSBUSART(USB_Buffer, 8)
  	 {
		count = mCDCGetRXLength();
		total_sum = 0;
		for (index =0; index < count; index++)
		{
			total_sum += USB_Buffer[index];
		}

	 }
*/
     if (getsUSBUSART(USB_Buffer, CDC_BULK_OUT_EP_SIZE))   // Works!!!
     //if (getsUSBUSART(USB_Buffer, 1))
	 {
			// Route data USB End Point buffer to USART.  Works!!!
			//USB_Buffer[1] = 0;
			USB_Buffer[CDC_BULK_OUT_EP_SIZE] = 0;  // Works!!!
			//Printf_USB((char *) USB_Buffer);    // Works but slows down transfers
			Printf((char *) USB_Buffer);      // Works!!!  

  	 }
 
  
	
}//end Transfer_USB_To_USART


//*******************************************************************************
//*  Transfer_USART_To_USB - Route data from USART to the USB End Point buffer   
//******************************************************************************
void Transfer_USART_To_USB(void)
{
/* 
	 if	(!vUARTIntStatus.UARTIntRxBufferEmpty)
 	 {
		// Read a block of data from the USART  		
		//UARTIntGetChar((byte *) USART_Tx_Buffer);

		// <<<<<<<<<<<<<<  Process message from USART here  >>>>>>>>>>>>>>>>>>>> 

     	// Copy the response message to the USB buffer    
  	    //memcpy((char *) USB_Buffer, USART_Tx_Buffer, (byte) UARTINTC_TX_BUFFER_SIZE);    
   
        // Send the message back to the laptop using the USB Virtual COM Port
		Printf_USB((char *) USB_Buffer);  // Works
     }
*/	
}//end Transfer_USART_To_USB

//*******************************************************************************
//* Transfer_USB_To_SPI - Route data from USB End Point buffer to SPI port  
//******************************************************************************
void Transfer_USB_To_SPI(void)
{
 	byte index, count, total_sum;
    int i;
	int Local_Checksum = 2016;

    if (getsUSBUSART(USB_Buffer, CDC_BULK_OUT_EP_SIZE))   // Works!!!
    //if (getsUSBUSART(USB_Buffer, 1))
	{
		// Route data USB End Point buffer to USART.  Works!!!
		//USB_Buffer[1] = 0;
		//USB_Buffer[CDC_BULK_OUT_EP_SIZE] = 0;  // Works!!!

		//Printf_USB((char *) USB_Buffer);    // Works but slows down transfers
		//Printf((char *) USB_Buffer);      // Works!!!  
        
		// Write data from USB Bus to the SPI BUS
		//SPI_WR(USB_Buffer[0]); //Send USB data even data  
 		for (i=0; i<CDC_BULK_OUT_EP_SIZE; i++)
		{
     		SPI_WR(USB_Buffer[i]); //Send USB data even data  
		}  

		for (j=0; j<1000; j++);

		// Send the message back to the laptop using the USB Virtual COM Port
		if (mUSBUSARTIsTxTrfReady())
		{
			mUSBUSARTTxRam((byte *) USART_Rx_Buffer, (byte) CDC_BULK_OUT_EP_SIZE);           
			//mUSBUSARTTxRam((byte *) USB_Buffer, (byte) CDC_BULK_OUT_EP_SIZE);           
			//mUSBUSARTTxRam((byte *) USB_Buffer, (byte) 1);           
			//mUSBUSARTTxRam((byte *) (byte *) USART_Rx_Buffer[(count-1)], (byte) 1);           
        	State++;
		}  

		//for (j=0; j<DELAY; j++);
 
 
#ifdef NEW_CODE
	
     if (getsUSBUSART((byte *)USB_Buffer, CDC_BULK_OUT_EP_SIZE))
     //if (getsUSBUSART((byte *)USB_Buffer, 1))
	 {
		for (i=0; i<CDC_BULK_OUT_EP_SIZE; i++)
		{
     		SPI_WR(USB_Buffer[i]); //Send USB data even data  
		}  

          // Send the message back to the laptop using the USB Virtual COM Port
		if (mUSBUSARTIsTxTrfReady())
		{
			mUSBUSARTTxRam((byte *) USART_Rx_Buffer, (byte) CDC_BULK_OUT_EP_SIZE);           
        	State++;
		}  

		//for (j=0; j<DELAY; j++);

#endif

#ifdef NEW_CODE
 		// Route data from USB End Point buffer to I2C or SPI and can be used to read sensor data   -  Works!!!
		PORTD =0x00;  // Turn off all LEDs

     	for(Tx_Data=0; Tx_Data<64; Tx_Data++)                 
    	{
       		PORTD = USB_Buffer[Tx_Data];
			for (j=0; j<DELAY; j++);

		}
#endif
 
#ifdef NEW_CODE
 		// Route data from USB End Point buffer to I2C or SPI and can be used to read sensor data   -  
		PORTD =0x00;  // Turn off all LEDs


		// Compute checksum from message received from laptop via the USB Bus
    	Checksum = 0;
    	for(Tx_Data=0; Tx_Data<64; Tx_Data++)                 // Works
    	{
         	Data = USB_Buffer[Tx_Data];

			// Compute checksum
        	Checksum += Data;

		}

		// Compare the checksum to validate message received from SPI Slave	

        if (Checksum == Local_Checksum)
		{
			// Increment number of valid messages received from SPI Slave
			Message_Count++;
			PORTD = Message_Count;
			for (j=0; j<DELAY; j++);

/*
			for (i=0;i<10;i++)
			{
				PORTD = Message_Count;
				for (j=0; j<DELAY; j++);
				PORTD = 0x00;
				for (j=0; j<DELAY; j++);
			}
*/

		}
		else
		{

/*
			// Display Local_Checksum MSB
		 	PORTD = Local_Checksum / 256;
			for (j=0; j<10*DELAY; j++);

			// Display Local_Checksum 	LSB
			PORTD = Local_Checksum % 256; 
			for (j=0; j<10*DELAY; j++);
*/

			PORTD = 0xFF;
			for (j=0; j<DELAY; j++);

/*
			for (i=0;i<10;i++)
			{
				//PORTD = 0xAA;
				PORTD = 0xFF;
				for (j=0; j<DELAY; j++);
				PORTD = 0x00;
				for (j=0; j<DELAY; j++);
			}	
*/
		
		}

#endif

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

#ifdef NEW_CODE

 		// Route data from USB End Point buffer to I2C or SPI   -  This code Works!!!
		PORTD =0x00;  // Turn off all LEDs

    	Checksum = 0;

    	for(Tx_Data=0; Tx_Data<64; Tx_Data++)                 
    	{

         	Data = USB_Buffer[Tx_Data];
         	//Data = 64 - Tx_Data;

#ifdef USART_SUPPORT
			// Display USB Buffer data 
     		sprintf(tempString, "Tx_Data =  %4d, USB_Buffer = %5d \r\n", i, Data);
			Printf((char *) tempString);         
#endif
			// Compute checksum
        	Checksum += Data;
                     
        	SPI_WR(Data); //Send USB data even data    
			//Rx_Data[Tx_Data] = SPI_Write(Data);                         
    	}

#ifdef USART_SUPPORT
		// Display USB message checksum here
     	sprintf(tempString, "Checksum =  %d \r\n", Checksum);
		Printf((char *) tempString);         
#endif

/* 
		// Compute the checksum received to local checksum
		Local_Checksum = 0;
    	for(Tx_Data=0; Tx_Data<64; Tx_Data++)                  
    	{
			Local_Checksum += USART_Rx_Buffer[Tx_Data]; 
 		}	

		// Compare the checksum to validate message received from SPI Slave	
        if (Checksum == Local_Checksum)
		{
			// Increment number of valid messages received from SPI Slave
			Message_Count++;

			// Display number of valid messages received on LEDs
			for (i=0;i<10;i++)
			{
				PORTD = Message_Count;
				for (j=0; j<DELAY; j++);
				PORTD = 0x00;
				for (j=0; j<DELAY; j++);
			}
		}
		else
		{
			// Display error message on LEDs
			for (i=0;i<10;i++)
			{
				//PORTD = 0xAA;
				PORTD = 0xFF;
				for (j=0; j<DELAY; j++);
				PORTD = 0x00;
				for (j=0; j<DELAY; j++);
			}	
		
		}

*/

		for (j=0; j<DELAY; j++);   // Works!!!

/*
		// Generate Test SPI data received to USB Buffer  - Works !!!
		for(Tx_Data=0; Tx_Data<64; Tx_Data++)                 
     	{
			USART_Rx_Buffer[Tx_Data] =  Tx_Data;      // Ramp 0..63
			//USART_Rx_Buffer[Tx_Data] =  64 - Tx_Data;   // Ramp 64..1  Works!!!
		}
*/
        // Send the message back to the laptop using the USB Virtual COM Port
		if (mUSBUSARTIsTxTrfReady())
		{
			mUSBUSARTTxRam((byte *) USART_Rx_Buffer, (byte) 64);           
        	State++;
		}

/*
		// Copy SPI data received to USB Buffer
		for(Tx_Data=0; Tx_Data<64; Tx_Data++)                 
    	{
			USB_Buffer[Tx_Data] = USART_Rx_Buffer[Tx_Data];
		}

        // Send the message back to the laptop using the USB Virtual COM Port
		//Printf_USB((char *) USB_Buffer);  // Works
*/
    
#endif


//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

#ifdef NEW_CODE

  		// Route data from USB End Point buffer to I2C or SPI and can be used to read sensor data   -  This code Works!!!
		PORTD =0x00;  // Turn off all LEDs


    	//LATAbits.LATA5 = 0;     //SS1 Low
		//for (j=0; j<DELAY; j++);


		// Initialize SPI Buffers
   		for(Tx_Data=0; Tx_Data<64; Tx_Data++)                 
    	{
			USART_Rx_Buffer[Tx_Data] = 0;
			//USART_Tx_Buffer[Tx_Data] = USB_Buffer[Tx_Data];
		}


    	Checksum = 0;
    	//for(Tx_Data=0; Tx_Data<64; Tx_Data++)
    	for(Tx_Data=0; Tx_Data<64; Tx_Data++)                 // Works
    	{

         	Data = Tx_Data;
         	//Data = USART_Tx_Buffer[Tx_Data];
         	//Data = USB_Buffer[Tx_Data];
         	//Data = 64 - Tx_Data;
     
			// Compute checksum
        	Checksum += Data;

        	//SPI_WR(2 * Tx_Data); //Send even data, slave echos odd data
                             //Tx_Send = 0,2,4,6,8 ....
                             //Rx_Receive = 0,1,3,5,7,9 ....  
                            
        	//SPI_WR(Data); //Send even data, slave echos  data + 10          Works!!!
        	SPI_WR(Data); //Send even data, slave echos data + 10 Works!!!
                             //Tx_Send = 0,1,2,3,4 ....
                             //Rx_Receive = 10,11,12,13,14 ....     
			//Rx_Data[Tx_Data] = SPI_Write(Data);                         
    	}

	 
   		//LATAbits.LATA5 = 1;     //SS1 high

		// Compute the checksum received to local checksum
		Local_Checksum = 0;

    	for(Tx_Data=0; Tx_Data<64; Tx_Data++)                 // Works
    	{
			Local_Checksum += USART_Rx_Buffer[Tx_Data]; 
			//Local_Checksum += USB_Buffer[Tx_Data];
		}	
		// Compare the checksum to validate message received from SPI Slave	

        if (Checksum == Local_Checksum)
		{
			// Increment number of valid messages received from SPI Slave
			Message_Count++;
			PORTD = Message_Count;
/*
			for (i=0;i<10;i++)
			{
				PORTD = Message_Count;
				for (j=0; j<DELAY; j++);
				PORTD = 0x00;
				for (j=0; j<DELAY; j++);
			}
*/
		}
		else
		{

/*
			// Display Local_Checksum MSB
		 	PORTD = Local_Checksum / 256;
			for (j=0; j<10*DELAY; j++);

			// Display Local_Checksum 	LSB
			PORTD = Local_Checksum % 256; 
			for (j=0; j<10*DELAY; j++);
*/

			PORTD = 0xFF;
/*
			for (i=0;i<10;i++)
			{
				//PORTD = 0xAA;
				PORTD = 0xFF;
				for (j=0; j<DELAY; j++);
				PORTD = 0x00;
				for (j=0; j<DELAY; j++);
			}	
*/		
		}

          // Send the message back to the laptop using the USB Virtual COM Port
		if (mUSBUSARTIsTxTrfReady())
		{
			mUSBUSARTTxRam((byte *) USART_Rx_Buffer, (byte) 64);           
        	State++;
		}  

		for (j=0; j<DELAY; j++);
#endif

 	}

	 // Here is the working code
/*
	// Test the SPI port 
   	while(1)
	{
       	for (i=0; i<255; i++)
		{
			// Write the value
			SPI_WR(i);
			
			// Delay a bit between writess to be able to see the LEDs change state
			for (j=0; j<DELAY; j++);
	
		}

	}     

     if (getsUSBUSART(USB_Buffer, 8)
  	 {
		count = mCDCGetRXLength();
		total_sum = 0;
		for (index =0; index < count; index++)
		{
			total_sum += USB_Buffer[index];
		}

	 }
     if (getsUSBUSART(USB_Buffer, CDC_BULK_OUT_EP_SIZE))
     //if (getsUSBUSART(USB_Buffer, 1))
	 {
			// Route data USB End Point buffer to USART.  Works!!!
			//USB_Buffer[1] = 0;
			USB_Buffer[27] = 0;
			//Printf_USB((char *) USB_Buffer);    // Works but slows down transfers
			Printf((char *) USB_Buffer);      // Works!!!  

			// Route data from USB End Point buffer to SPI 

    		CS_TEMP_SENSOR = 0;         // Select DC Motor Controller
			putsSPI(USB_Buffer);
     		CS_TEMP_SENSOR = 1;         // De-select DC Motor Controller

			// Route data from USB End Point buffer to I2C or SPI

  	 }
*/

	
}//end Transfer_USB_To_SPI

//*******************************************************************************
//* Transfer_SPI_To_USB - Route data from SPI port to USB End Point buffer.   Works!!!   
//******************************************************************************
void Transfer_SPI_To_USB(void)
{
 	byte index, count, total_sum;
    int i;

     if (getsUSBUSART((byte *)USB_Buffer, CDC_BULK_OUT_EP_SIZE))
     //if (getsUSBUSART((byte *)USB_Buffer, 1))
	 {
#ifdef NEW_CODE

 		// Route data from USB End Point buffer to I2C or SPI   -  This code Works!!!
		PORTD =0x00;  // Turn off all LEDs

    	Checksum = 0;

    	for(Tx_Data=0; Tx_Data<64; Tx_Data++)                 
    	{

         	Data = USB_Buffer[Tx_Data];
         	//Data = 64 - Tx_Data;
     
			// Compute checksum
        	Checksum += Data;
                     
        	SPI_WR(Data); //Send USB data even data    
			//Rx_Data[Tx_Data] = SPI_Write(Data);                         
    	}
/* 
		// Compute the checksum received to local checksum
		Local_Checksum = 0;
    	for(Tx_Data=0; Tx_Data<64; Tx_Data++)                  
    	{
			Local_Checksum += USART_Rx_Buffer[Tx_Data]; 
 		}	

		// Compare the checksum to validate message received from SPI Slave	
        if (Checksum == Local_Checksum)
		{
			// Increment number of valid messages received from SPI Slave
			Message_Count++;

			// Display number of valid messages received on LEDs
			for (i=0;i<10;i++)
			{
				PORTD = Message_Count;
				for (j=0; j<DELAY; j++);
				PORTD = 0x00;
				for (j=0; j<DELAY; j++);
			}
		}
		else
		{
			// Display error message on LEDs
			for (i=0;i<10;i++)
			{
				//PORTD = 0xAA;
				PORTD = 0xFF;
				for (j=0; j<DELAY; j++);
				PORTD = 0x00;
				for (j=0; j<DELAY; j++);
			}	
		
		}

*/

		for (j=0; j<DELAY; j++);   // Works!!!

/*
		// Generate Test SPI data received to USB Buffer  - Works !!!
		for(Tx_Data=0; Tx_Data<64; Tx_Data++)                 
     	{
			USART_Rx_Buffer[Tx_Data] =  Tx_Data;      // Ramp 0..63
			//USART_Rx_Buffer[Tx_Data] =  64 - Tx_Data;   // Ramp 64..1  Works!!!
		}
*/
        // Send the message back to the laptop using the USB Virtual COM Port
		if (mUSBUSARTIsTxTrfReady())
		{
			mUSBUSARTTxRam((byte *) USART_Rx_Buffer, (byte) 64);           
        	State++;
		}

/*
		// Copy SPI data received to USB Buffer
		for(Tx_Data=0; Tx_Data<64; Tx_Data++)                 
    	{
			USB_Buffer[Tx_Data] = USART_Rx_Buffer[Tx_Data];
		}

        // Send the message back to the laptop using the USB Virtual COM Port
		//Printf_USB((char *) USB_Buffer);  // Works
*/
    
#endif

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
 
#ifndef NEW_CODE
 		// Route data from USB End Point buffer to I2C or SPI and can be used to read sensor data   -  This code Works!!!
		PORTD =0x00;  // Turn off all LEDs

 
    	//LATAbits.LATA5 = 0;     //SS1 Low
		//for (j=0; j<DELAY; j++);


		// Initialize SPI Buffers
   		for(Tx_Data=0; Tx_Data<64; Tx_Data++)                 
    	{
			USART_Rx_Buffer[Tx_Data] = 0;
			//USART_Tx_Buffer[Tx_Data] = USB_Buffer[Tx_Data];
		}


    	Checksum = 0;
    	//for(Tx_Data=0; Tx_Data<64; Tx_Data++)
    	for(Tx_Data=0; Tx_Data<64; Tx_Data++)                 // Works
    	{

         	Data = Tx_Data;
         	//Data = USART_Tx_Buffer[Tx_Data];
         	//Data = USB_Buffer[Tx_Data];
         	//Data = 64 - Tx_Data;
     
			// Compute checksum
        	Checksum += Data;

        	//SPI_WR(2 * Tx_Data); //Send even data, slave echos odd data
                             //Tx_Send = 0,2,4,6,8 ....
                             //Rx_Receive = 0,1,3,5,7,9 ....  
                            
        	//SPI_WR(Data); //Send even data, slave echos  data + 10          Works!!!
        	SPI_WR(Data); //Send even data, slave echos data + 10 Works!!!
                             //Tx_Send = 0,1,2,3,4 ....
                             //Rx_Receive = 10,11,12,13,14 ....     
			//Rx_Data[Tx_Data] = SPI_Write(Data);                         
    	}

	 
   		//LATAbits.LATA5 = 1;     //SS1 high

		// Compute the checksum received to local checksum
		Local_Checksum = 0;

    	for(Tx_Data=0; Tx_Data<64; Tx_Data++)                 // Works
    	{
			Local_Checksum += USART_Rx_Buffer[Tx_Data]; 
			//Local_Checksum += USB_Buffer[Tx_Data];
		}	
		// Compare the checksum to validate message received from SPI Slave	

        if (Checksum == Local_Checksum)
		{
			// Increment number of valid messages received from SPI Slave
			Message_Count++;
			PORTD = Message_Count;
/*
			for (i=0;i<10;i++)
			{
				PORTD = Message_Count;
				for (j=0; j<DELAY; j++);
				PORTD = 0x00;
				for (j=0; j<DELAY; j++);
			}
*/
		}
		else
		{

/*
			// Display Local_Checksum MSB
		 	PORTD = Local_Checksum / 256;
			for (j=0; j<10*DELAY; j++);

			// Display Local_Checksum 	LSB
			PORTD = Local_Checksum % 256; 
			for (j=0; j<10*DELAY; j++);
*/

			PORTD = 0xFF;
/*
			for (i=0;i<10;i++)
			{
				//PORTD = 0xAA;
				PORTD = 0xFF;
				for (j=0; j<DELAY; j++);
				PORTD = 0x00;
				for (j=0; j<DELAY; j++);
			}	
*/		
		}

          // Send the message back to the laptop using the USB Virtual COM Port
		if (mUSBUSARTIsTxTrfReady())
		{
			mUSBUSARTTxRam((byte *) USART_Rx_Buffer, (byte) 64);           
        	State++;
		}  

		for (j=0; j<DELAY; j++);
#endif

 	}

	 // Here is the working code
/*
	// Test the SPI port 
   	while(1)
	{
       	for (i=0; i<255; i++)
		{
			// Write the value
			SPI_WR(i);
			
			// Delay a bit between writess to be able to see the LEDs change state
			for (j=0; j<DELAY; j++);
	
		}

	}     

     if (getsUSBUSART(USB_Buffer, 8)
  	 {
		count = mCDCGetRXLength();
		total_sum = 0;
		for (index =0; index < count; index++)
		{
			total_sum += USB_Buffer[index];
		}

	 }
     if (getsUSBUSART(USB_Buffer, CDC_BULK_OUT_EP_SIZE))
     //if (getsUSBUSART(USB_Buffer, 1))
	 {
			// Route data USB End Point buffer to USART.  Works!!!
			//USB_Buffer[1] = 0;
			USB_Buffer[27] = 0;
			//Printf_USB((char *) USB_Buffer);    // Works but slows down transfers
			Printf((char *) USB_Buffer);      // Works!!!  

			// Route data from USB End Point buffer to SPI 

    		CS_TEMP_SENSOR = 0;         // Select DC Motor Controller
			putsSPI(USB_Buffer);
     		CS_TEMP_SENSOR = 1;         // De-select DC Motor Controller

			// Route data from USB End Point buffer to I2C or SPI

  	 }
*/


}//end Transfer_SPI_To_USB


/******************************************************************************
 * Function:        void BlinkUSBStatus(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        BlinkUSBStatus turns on and off LEDs corresponding to
 *                  the USB device state.
 *
 * Note:            mLED macros can be found in io_cfg.h
 *                  usb_device_state is declared in usbmmap.c and is modified
 *                  in usbdrv.c, usbctrltrf.c, and usb9.c
 *****************************************************************************/
void BlinkUSBStatus(void)
{
    static word led_count=0;
    
    if(led_count == 0)led_count = 10000U;
    led_count--;

    #define mLED_Both_Off()         {mLED_1_Off();mLED_2_Off();}
    #define mLED_Both_On()          {mLED_1_On();mLED_2_On();}
    #define mLED_Only_1_On()        {mLED_1_On();mLED_2_Off();}
    #define mLED_Only_2_On()        {mLED_1_Off();mLED_2_On();}

    if(UCONbits.SUSPND == 1)
    {
        if(led_count==0)
        {
            mLED_1_Toggle();
            mLED_2 = mLED_1;        // Both blink at the same time
        }//end if
    }
    else
    {
        if(usb_device_state == DETACHED_STATE)
        {
            mLED_Both_Off();
        }
        else if(usb_device_state == ATTACHED_STATE)
        {
            mLED_Both_On();
        }
        else if(usb_device_state == POWERED_STATE)
        {
            mLED_Only_1_On();
        }
        else if(usb_device_state == DEFAULT_STATE)
        {
            mLED_Only_2_On();
        }
        else if(usb_device_state == ADDRESS_STATE)
        {
            if(led_count == 0)
            {
                mLED_1_Toggle();
                mLED_2_Off();
            }//end if
        }
        else if(usb_device_state == CONFIGURED_STATE)
        {
            if(led_count==0)
            {
                mLED_1_Toggle();
                mLED_2 = !mLED_1;       // Alternate blink                
            }//end if
        }//end if(...)
    }//end if(UCONbits.SUSPND...)

}//end BlinkUSBStatus

BOOL Switch2IsPressed(void)
{
    if(sw2 != old_sw2)
    {
        old_sw2 = sw2;                  // Save new value
        if(sw2 == 0)                    // If pressed
            return TRUE;                // Was pressed
    }//end if
    return FALSE;                       // Was not pressed
}//end Switch2IsPressed

BOOL Switch3IsPressed(void)
{
    if(sw3 != old_sw3)
    {
        old_sw3 = sw3;                  // Save new value
        if(sw3 == 0)                    // If pressed
            return TRUE;                // Was pressed
    }//end if
    return FALSE;                       // Was not pressed
}//end Switch3IsPressed

/** EOF user.c ***************************************************************/

/******************************************************************************
 * Function:        void Printf
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Printf sends ascii text data to the USART serial similar to C printf.  
 *
 * Note:            mLED macros can be found in io_cfg.h
 *                  usb_device_state is declared in usbmmap.c and is modified
 *                  in usbdrv.c, usbctrltrf.c, and usb9.c
 *****************************************************************************/
void Printf(char Buffer[UARTINTC_TX_BUFFER_SIZE])
{
    int i;

   	// Write data to USART using Microchip C Library

   	putsUSART(Buffer);

/*
	// Send the string to the serial port using Maestro.  Works!!!
	i = 0;
 	while (i<UARTINTC_TX_BUFFER_SIZE)
	{   	
		// Check for end of string (0 = NULL) 
		if (Buffer[i] == 0) break;	
         											// We may want handshaking here...
   		//status flag
		if(!vUARTIntStatus.UARTIntTxBufferFull)                        
		{
				UARTIntPutChar(Buffer[i]);		                   // Works!!!
				//UARTIntPutChar(*s);		
 				i++;
	 	}
  		
 	} 
*/
}

/******************************************************************************
 * Function:        void Printf_USB
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Printf_USB sends ascii text data to the USB virtual serial COM port in a manner similar to C printf.  
 *
 * Note:            mLED macros can be found in io_cfg.h
 *                  usb_device_state is declared in usbmmap.c and is modified
 *                  in usbdrv.c, usbctrltrf.c, and usb9.c
 *****************************************************************************/
void Printf_USB(char Buffer[UARTINTC_TX_BUFFER_SIZE])
{
    int i;

	// Send the string to the USB serial Virtual COM port
           											// We may want handshaking here...
   	//status flag
    if (mUSBUSARTIsTxTrfReady())     // Works!!!!
        putsUSBUSART(Buffer);
 
}

void SPI_INIT(void) 
{
    ADCON1 = 0x0F;          //Configure all pins as digital
		 
    LATAbits.LATA5 = 1;     //SS1 logic high
    TRISAbits.TRISA5 = 0;   //Master SS1 pin out, drv=1

    TRISBbits.TRISB0 = 1;   //Init SDI input

    LATCbits.LATC7 = 1;     //Init SDO high
    TRISCbits.TRISC7 = 0;   //Init SDO output

    LATBbits.LATB1 = 0;     //Init SCK low
    TRISBbits.TRISB1 = 0;   //Init SCK output

    SSPSTAT = 0x00;         //Clr SPI status reg
    SSPSTATbits.SMP = 0;    //Input data sampled at middle of data output time
    //SSPSTATbits.CKE = 0;    //xmit occurs on Idle to active clock state
    SSPSTATbits.CKE = 1;    //xmit occurs on active to Idle clock state (must be set for dsPIC Slave Mode???)

    SSPCON1 = 0x00;         //SPI Master mode, clock = FOSC/4
    SSPCON1bits.CKP = 0;    //Idle state for clock is a low level       

    //IPR1 = 0x08;            //SPI high Priority interrupt
    PIR1bits.SSPIF = 0;     //Clr SPI int flag
    //PIE1bits.SSPIE = 1;     //Enb SPI interrupt
    //INTCON = 0xC0;          //Enb global & Peripheral interrupts
     
    SSPCON1bits.SSPEN = 1;  //Enb & config SCK, SDO, SDI and SS as serial port pins

}


//*******************************************************************************
//* SPI_Read - Read data from the SPI Bus   
//******************************************************************************
int SPI_Read(void)
{
    volatile int i;
    volatile unsigned char  data_temp = SSPSTAT; 
 	int The_Value; 

    //while(Tx_IP == 1);      //Wait if transmit in progress
    //while(SSPSTATbits.BF == 0); //Wait until Tx/Rx complete
                            //This necessary because SSPIF bit gets
                            //set before last Tx_SPI bit is complete

    LATAbits.LATA5 = 0;     //SS1 Low
    //for(i=0; i<4; i++);     //Delay
    //for(i=0; i<100; i++);    //Delay Works!!!
 
    //Tx_IP = 1;              //Set SPI Tx In-Process flag

    SSPBUF = 0;      //write a dummy byte to SPI Tx data so that the SPI slave will send a data byte  
    while (!PIR1bits.SSPIF);
    The_Value = SSPBUF; //Read Rx_SPI data
    //count++;                //Inc read count

    PIR1bits.SSPIF = 0;     //Clr SPI int flag
    //data_temp = SSPBUF;   // Clear the receive buffer and the BF flag 

    //for(i=0; i<100; i++);    //Delay Works!!!
    //for(i=0; i<16; i++);    //Delay Works!!!

    LATAbits.LATA5 = 1;     //SS1 high
	return The_Value;
	 
		 
}

//*******************************************************************************
//* SPI_Write - Write data to SPI Bus and return data read from the SPI Bus  
//******************************************************************************
int SPI_Write(int SPI_Data)
{
    volatile int i;
    volatile unsigned char  data_temp = SSPSTAT; 
 	int The_Value; 

    //while(Tx_IP == 1);      //Wait if transmit in progress
    //while(SSPSTATbits.BF == 0); //Wait until Tx/Rx complete
                            //This necessary because SSPIF bit gets
                            //set before last Tx_SPI bit is complete

    LATAbits.LATA5 = 0;     //SS1 Low
    //for(i=0; i<4; i++);     //Delay
    //for(i=0; i<100; i++);    //Delay Works!!!
 
    //Tx_IP = 1;              //Set SPI Tx In-Process flag

    SSPBUF = SPI_Data;      //SPI Tx data   
    while (!PIR1bits.SSPIF);
    The_Value = SSPBUF; //Read Rx_SPI data
    //count++;                //Inc read count

    PIR1bits.SSPIF = 0;     //Clr SPI int flag
    //data_temp = SSPBUF;   // Clear the receive buffer and the BF flag 

    //for(i=0; i<100; i++);    //Delay Works!!!
    //for(i=0; i<16; i++);    //Delay Works!!!

    LATAbits.LATA5 = 1;     //SS1 high
	return The_Value;
	 
		 
}

//*******************************************************************************
//* SPI_WR - Write data to SPI Bus and collect data read from the SPI Bus  
//******************************************************************************
void SPI_WR(int SPI_Data)
{
    volatile int i;
    volatile unsigned char  data_temp = SSPSTAT; 
 
    //while(Tx_IP == 1);      //Wait if transmit in progress
    //while(SSPSTATbits.BF == 0); //Wait until Tx/Rx complete
                            //This necessary because SSPIF bit gets
                            //set before last Tx_SPI bit is complete

    LATAbits.LATA5 = 0;     //SS1 Low
    //for(i=0; i<4; i++);     //Delay
    //for(i=0; i<100; i++);    //Delay Works!!!
 
    //Tx_IP = 1;              //Set SPI Tx In-Process flag

    SSPBUF = SPI_Data;      //SPI Tx data   
    while (!PIR1bits.SSPIF);
    USART_Rx_Buffer[count] = SSPBUF; //Rd Rx_SPI
    count++;                //Inc read count
    count = count % 64; 
    PIR1bits.SSPIF = 0;     //Clr SPI int flag
    //data_temp = SSPBUF;   // Clear the receive buffer and the BF flag 

    //for(i=0; i<100; i++);    //Delay Works!!!
    //for(i=0; i<16; i++);    //Delay Works!!!

    LATAbits.LATA5 = 1;     //SS1 high
	 
		 
}
 
