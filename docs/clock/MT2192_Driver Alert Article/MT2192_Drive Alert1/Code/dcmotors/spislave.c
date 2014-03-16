//********************************************************************************
//*                                                                              *
//*        Name:  spislave.c                                                     *
//* Project Number:  MT2192                                                      *                                                   
//*           Date:  10/15/2007                                                  *                                                  
//*                                                                              *
//*   Description:                                                               * 
//*     This application implements an 8-Bit or 16-Bit SPI Slave Co-Processor controller that    *
//* emulates Microchip's 24LCXX SPI Memory chip interface and acts as a smart    *
//* peripheral, by using the SPI hardware module contained in the PIC18C452      *  
//* micro-controller.  In addition it can act as an 16-Bit SPI network server to an     *
//* SPI Client or function as an SPI based RPN Floating Point co-processor.  In  *
//* the simplest mode of operation, it simulates writing to the EEPROM and       *
//* reading it back under control from the Master SPI controller.  Included is a *
//* diagnostic function that will write a ramp to the EEPROM using SPI and read  *
//* it back to the Host PC via the USART when used with the spimast.c            *
//* application. A more sophisticated SPI Networking mode is also available to   *
//* applications that require message passing with CRC or checksum.  The common  *
//* message format between the SPI Master and SPI Slave is defined in the        *
//* file spimsg.h.  SPI transmit and receive buffers are available to process    *
//* messages to/from the Master controller.                                      *                         
//*                                                                              *
//*     This SPI slave controller can be used with a PC host or any SPI master,  *
//* in order to  give any PC the capability to serve as an SPI controller.       * 
//* It can also be used for robotic applications where multiple controllers and  *
//* SPI devices are used, with a minimum of data and control lines needed to     *
//* connect them.  Each SPI device has its own unique slave select enable line   *
//* (SS_BAR) and only listens to the SPI Master commands when SS_BAR is cleared. *  
//*                                                                              *
//*    This application emulates a Microchip 24LCXX EEPROM that has 8 registers  *
//* accessed as sub-addresses 1:8 modulo 7.  Sub-address 0 is used as a data     *
//* channel.  All the read and write data modes available for this device        *
//* including: Byte Write, Page Write, Current Address Read, Random Read and     *
//* Sequential Read are emulated by this application.  As far as the Master SPI  * 
//* controller knows, it is talking to a real EEPROM instead of another PIC.     *
//*                                                                              *
//*                                                                              * 
//*     The SPI message format used by this application is for the master to     *
//* send commands to the Digital Pot SPI slave device.  This translates to the   *
//* following sequence of operations for the Master:                             *                    
//*                                                                              *
//*  1) The SPI Co-Processors Command Bytes: X|X|C1|C0|X|X|P1|P0                 *
//*                                                                              *
//*                                                                              *
//*  2) Data is latched in on the rising edge of the SCK.                        *
//*                                                                              *
//*  3) Data is clocked out the SO pin after the falling edge of SCK.            *
//*                                                                              *
//*  4) The Clock Iddle is high.                                                 *
//*                                                                              *
//*  5) The SPI Clock speed is FOSC/16                                           *
//*                                                                              *
//*  6) The Reset Pin, RS_BAR will automatically set all potentiometer data      *
//*     latches to mid scale (Code 80h).                                         *
//*                                                                              *
//*  7) The Shutdown Pin, SHDN_BAR when held low will (provided that the pin is  *
//*     held low for at least 150 ns and CS_BAR is high, will cause the          *
//*     application circuit to into a power saver mode.                          *
//*                                                                              *
//*  8) The device when powered up for the first time, will automatically set    *
//*     reset all potentiometer data latches to mid scale (Code 80h).  A power   *
//*     on will insure that the device powers up in this known state             *
//*                                                                              *
//*  9) This device can operate in SPI Mode 0,0, in which the clock idles in the *
//*     low state.                                                               *
//*                                                                              *
//*  10) The following SPI data transfers are available:                         *
//*                                                                              *
//*    		Master sends data byte  -  Slave sends dummy data byte               *                                                                        
//*    		Master sends data byte  -  Slave sends data byte                     *                                                                  
//*    		Master sends dummy data byte  -  Slave sends data byte               *                                                                         
//*                                                                              *
//*                                                                              *
//* Note:                                                                        *
//*		The SPI networking is handled using a simple data packet messaging       *
//* protocol.  Each message includes an SPI source address field (Master), an SPI*
//* destination address field, the actual data and a CRC or checksum field.  Data*
//* packets that are determined to be corrupted will be retransmitted up to a    *
//* specified number of times.                                                   *
//*                                                                              * 
//*                                                                              *
//*      Future upgrades to this application include: the ability to communicate * 
//* with other SPI slave devices such as Microchip MPC3002 A/D converter, the    *
//* Maxim MAX7219 LED Driver, the Microchip SPI Serial EEPROMS such as the       *
//* 93XXXX Series and the ISD 33270 Voice Recorder IC.  It may also be used to   *
//* communicate as a master or a slave for applications requiring that dual      *
//* capability.  Other upgrades include using SPI to implement the C printf      *
//* function to be used for a "cheap" embedded debugger similar to JTAG used by  *
//* high end micro-processors. Explore using communication protocols and error   *
//* detection/correction algorithms such as parity, CRC and various other codes  *
//* for robust operation and fault tolerance. TMR and N-Module redundancy could  *
//* also be investigated for fault tolerant systems that work in hazardous       *
//* environments. The same SPI networking functions may also be used with the    *
//* asynchronous serial RS-232 interface and the I2C interface.                  *                         
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
 
#include <spi.h>
#include <pwm.h>
#include "my_types.h"
//#include "network.h"
#include "spislave.h"
#include "dcmotor.h"
 
//#define SPI_BYTE_INTERFACE   						// SPI Byte I/O
#define SPI_BUFFER_INTERFACE  						// SPI Buffered I/O (using SPI interrupt)
//#define SPI_NETWORK_INTERFACE  					// SPI Message Passing I/O (using SPI interrupt)

//#define PWM_TEST				// test the PWM hardware

//#define SPI_TEST_0				// Echo message back to master
//#define SPI_TEST_1     // Works
//#define SPI_TEST_2
//#define SPI_TEST_3
//#define SPI_TEST_4

 
// Port B definitions
//#define BUSY_FLAG PORTBbits.RB3					 			//  (0=Not Busy, 1=Busy)
//#define BUSY_FLAG_DIR TRISBbits.TRISB3					 	//  Direction (0=output, 1=input)
//#define BUSY_FLAG PORTDbits.RD1					 			//  (0=Not Busy, 1=Busy)
//#define BUSY_FLAG_DIR TRISDbits.TRISD1					 	//  Direction (0=output, 1=input)
// Port B definitions
#define BUSY_FLAG PORTBbits.RB8					 			//  (0=Not Busy, 1=Busy)
#define BUSY_FLAG_DIR TRISBbits.TRISB8					 	//  Direction (0=output, 1=input)


// Declare SPI Master Buffer variables here

#ifdef SPI_8
volatile byte SPI_Rx_Buffer[SPI_BUFFER_SIZE];                // SPI Receive Buffer (bytes)
volatile byte SPI_Tx_Buffer[SPI_BUFFER_SIZE];                // SPI Transmit Buffer (bytes)
#endif

#ifdef SPI_16
volatile word SPI_Rx_Buffer[SPI_BUFFER_SIZE];                // SPI Receive Buffer (words)
volatile word SPI_Tx_Buffer[SPI_BUFFER_SIZE];                // SPI Transmit Buffer (words)
#endif

volatile int  SPI_Start_Buffer = 0;                         	// Start pointer for ring buffer
volatile int SPI_End_Buffer = 0;								// SPI End Buffer Index

volatile byte SPI_Rx_Flag = FALSE;                        	// Indicates that data was received into the ring buffer
volatile word SPI_Timeout = 0;                            	// SPI I/O Timeout counter
volatile byte SPI_Timeout_Flag;                           	// SPI Timeout flag
volatile int SPI_Number_Characters = 0;						 // SPI Number of characters in buffer
volatile int SPI_Communications_Error;                    	// SPI I/O error flag
                                                 	// -1 = SPI Input Bufer overrun error
                                                 	// -2 = SPI Communications timeout error
                                                 	// -3 = SPI buffer empty error

volatile int SPI_Interrupt_Count = 0;                   		// Number of MSSP interrupts
volatile int SPI_Data_In = 0;								// Data byte read from the SPI Master
volatile int SPI_Data_Out = 0;								// Data byte sent to the SPI Master
volatile byte SPI_Status;										// SPI Status returned from routines


// Types used to split and join 16-bit words from bytes
typedef volatile union  
{
 
	word  WordValue;
    struct {
        byte Byte1:8;               // LSN
        byte Byte2:8;               // MSN
 
    } ;
 
} Word_T ; 

volatile Word_T The_Data;

//************************************************************************************************************
//* Declare local functions here
//************************************************************************************************************

//************************************************************************************************************
//* SPI1 Interrupt Service routine - Data received at SPI1 
//************************************************************************************************************
  
void __attribute__((__interrupt__)) _SPI1Interrupt(void)
{
 
	IFS0bits.SPI1IF = 0;
	SPI1STATbits.SPIROV = 0; /* Clear SPI1 receive overflow
	flag if set */
		
	// Interrupt Logic for SPI Slave Buffer Interface goes here ...

	// Check for buffer full indicating the end of message

    if (SPI_End_Buffer >= SPI_BUFFER_SIZE)
     {
 			       	
       	SPI_End_Buffer = 0;
       	SPI_Rx_Flag = TRUE;				// Set the SPI message Received Data flag
       	
 	}
    else
	{    
 		// Don't accept any data if busy processing
		if (!BUSY_FLAG)
		{

  			// Read a single word from the SPI Bus (SSPBUF) into the Rx buffer.
 			while(!DataRdySPI1());
 			SPI_Rx_Buffer[SPI_End_Buffer]  = ReadSPI1();
		
			// Write response word from Tx buffer to the SPI Bus
     		WriteSPI1(SPI_Tx_Buffer[SPI_End_Buffer]);
  			while(SPI1STATbits.SPITBF);
 			
			SPI_End_Buffer++;										 			// Increment the SPI Ring Buffer pointer
    		//SPI_End_Buffer = SPI_End_Buffer % SPI_BUFFER_SIZE;        		// Wrap the EndBuffer pointer if necessary
      
    		// Increment SPI Interrupt counter
   			SPI_Interrupt_Count++;
		}
	}
 
}

#ifdef SPI_SLAVE_TEST                                  // SPI Slave Co-Processor test

//********************************************************************************
//* main - This is the main Slave SPI application  
//********************************************************************************
int main(void)
{
	int i;								// Local loop index

 	// Configure USART for 20 MHz, 9600 Baud, 8-Bits, No parity, Asynchronous
   
	InitializeUsart(); 

	printf("Initializing the SPI Slave Application ...\r\n");

	// Configure the SPI hardware as a slave

	InitializeSPISlave();

	SetupTimers();                                    // Setup timers and software
                                                      // variables.
	// Check timer code by delaying 5 seconds ...

	printf("Start of 5 second delay...\r\n");
	pause(5000);
	printf("End of 5 second delay. \r\n");

 
#ifdef SPI_TEST_1

	// Test #1 - Send a ramp of data back to SPI master using SPI interrupts


	// Preload SPI buffer with valid data to  back to the SPI master (SPI data Exchange)

	// Send a 32 word (16-Bit) string of data from the SPI_TX_Buffer to the SPI slave

	// Preload SPI buffer with valid data to  back to the SPI master (SPI data Exchange) 
    // This logic works !!!!

   	WriteSPI1(SPI_Tx_Buffer[0]);
  	while(SPI1STATbits.SPITBF);

 
   	while(1)
   	{
 		// Clear the watchdog timer each time through the major menu loop

		//ClrWdt();


      	// Copy the current message response (if any) to the transmit buffer so that
    	// it can be sent back to the SPI Master Controller.
         		 			
     	//memcpy(SPI_Tx_Buffer+1, &MotorMessage.Message.Data, DATA_BUFFER_SIZE);

      	// Copy the entire DC Motor Controller message sent from the SPI Master
     	// to local storage for processing.
   			
     	//memcpy(&MotorMessage.Message.Data, SPI_Rx_Buffer+1, DATA_BUFFER_SIZE);  


 		// Check to see if a message was received and process it...
 			
   		if (SPI_Rx_Flag == TRUE)
   		{
   			SPI_Rx_Flag = FALSE;

			// Set busy flag indicating to the SPI Master that the peripheral is busy
    		BUSY_FLAG = 1;

 
   			for (i=0; i<SPI_BUFFER_SIZE; i++)
			{
 				// Clear the watchdog timer each time through the major menu loop

				//ClrWdt();

				// Disable interrupts
				//DISICNT = 0x3FFF;   
				// Process SPI RX buffer value and copy it to SPI TX buffer  
				SPI_Tx_Buffer[i] =  SPI_Rx_Buffer[i] / 100;			// Scale the input
				//SPI_Tx_Buffer[i] = SPI_Rx_Buffer[i];					// Echo the input
				// Enable interrupts
				//DISICNT = 0x0000; 
			}

		

#ifdef TEST  
 			
			printf("spislave: Mainline, SPI_Interrupt_Count = ");
			dec(SPI_Interrupt_Count);
			printf("\r\n");
 			
			// Display contents of SPI Send and Receive Buffers 	
			
			for (i=0; i<SPI_BUFFER_SIZE; i++)
			{
			 
				printf("DCMotor: i = ");
				dec(i);
				printf("  SPI_Tx_Buffer = ");
				dec(SPI_Tx_Buffer[i]);
			
				printf("  SPI_Rx_Buffer = ");
				dec(SPI_Rx_Buffer[i]);
			
				printf("\r\n");
			
				//pause(1000);
			}		

       		printf("Slave: Mainline, Received DC Motor ");
#endif       

    		// Change controller state to BUSY
			//ControllerState = BUSY; 

      		// Process the selected command
            //********************************  Missing Logic here ***************************
			pause(5000);
  			// Clear busy flag indicating to the SPI Master that the peripheral is available
    		BUSY_FLAG = 0;
  						  	
 		}
	}	
	        	
#endif

 
 
#ifdef SPI_TEST_0
    datard = 0x1234;
	// Test #0 - Send a ramp of data back to SPI master using SPI interrupts


	// Preload SPI buffer with valid data to  back to the SPI master (SPI data Exchange)

	// Send a 32 word (16-Bit) string of data from the SPI_TX_Buffer to the SPI slave

	// Preload SPI buffer with valid data to  back to the SPI master (SPI data Exchange) 
    // This logic works !!!!

   	WriteSPI1(SPI_TX_Buffer[0]);
  	while(SPI1STATbits.SPITBF);

	while (1)
	{

		// Initialize SPI RX and TX buffers for slave

    	for (i=0; i<32; i++)
		{
			SPI_RX_Buffer[i] = 0;	
			SPI_TX_Buffer[i] = 0;	
		}

		// Read a block of 32 words from SPI master 
		for (j=0; j<32; j++)
		{
			// Read next word from SPI master
			while(!DataRdySPI1());
			SPI_RX_Buffer[j] = ReadSPI1();

			// Echo the data just read from the SPI Master
			SPI_TX_Buffer[j] = SPI_RX_Buffer[j]; 

			// Send back a word to the SPI master (SPI data Exchange)

   			WriteSPI1(SPI_TX_Buffer[j]);
  			while(SPI1STATbits.SPITBF);

           // Send PWM commands to motors

			SetDuty(0, SPI_RX_Buffer[0]);
			SetDuty(1, SPI_RX_Buffer[1]);
			SetDuty(2, SPI_RX_Buffer[2]);

 		}

 
   
	}	

#endif

#ifdef SPI_TEST_1

	// Test #1 - Send a ramp of data back to SPI master using SPI interrupts


	// Preload SPI buffer with valid data to  back to the SPI master (SPI data Exchange)

	// Send a 32 word (16-Bit) string of data from the SPI_TX_Buffer to the SPI slave

	// Preload SPI buffer with valid data to  back to the SPI master (SPI data Exchange) 
    // This logic works !!!!

   	WriteSPI1(SPI_Tx_Buffer[0]);
  	while(SPI1STATbits.SPITBF);

	// Initialize SPI RX and TX buffers for slave

   	for (i=0; i<32; i++)
	{
		SPI_Rx_Buffer[i] = 0;	
		SPI_Tx_Buffer[i] = 0;	
	}

   	while(1)
   	{
 		// Clear the watchdog timer each time through the major menu loop

		//ClrWdt();

      	// Copy the current message response (if any) to the transmit buffer so that
    	// it can be sent back to the SPI Master Controller.
         		 			
     	//memcpy(SPI_Tx_Buffer+1, &MotorMessage.Message.Data, DATA_BUFFER_SIZE);

      	// Copy the entire DC Motor Controller message sent from the SPI Master
     	// to local storage for processing.
   			
     	//memcpy(&MotorMessage.Message.Data, SPI_Rx_Buffer+1, DATA_BUFFER_SIZE);  


 		// Check to see if a message was received and process it...
 			
   		if (SPI_Rx_Flag == TRUE)
   		{

   			for (i=0; i<SPI_BUFFER_SIZE; i++)
			{
 				// Clear the watchdog timer each time through the major menu loop

				//ClrWdt();

				// Disable interrupts
				//DISICNT = 0x3FFF;   
				// Process SPI RX buffer value and copy it to SPI TX buffer  
				//SPI_Tx_Buffer[i] =  5.0 * SPI_Rx_Buffer[i];			// Scale the input
				SPI_Tx_Buffer[i] = SPI_Rx_Buffer[i];					// Echo the input
				// Enable interrupts
				//DISICNT = 0x0000; 
			}

   			SPI_Rx_Flag = FALSE;
		

#ifdef TEST  
 			
			printf("DCMotor: Mainline, SPI_Interrupt_Count = ");
			dec(SPI_Interrupt_Count);
			printf("\r\n");
 			
			// Display contents of SPI Send and Receive Buffers 	
			
			for (i=0; i<SPI_BUFFER_SIZE; i++)
			{
			 
				printf("DCMotor: i = ");
				dec(i);
				printf("  SPI_Tx_Buffer = ");
				dec(SPI_Tx_Buffer[i]);
			
				printf("  SPI_Rx_Buffer = ");
				dec(SPI_Rx_Buffer[i]);
			
				printf("\r\n");
			
				//pause(1000);
			}		

       		printf("Slave: Mainline, Received DC Motor ");
#endif       
    		
    		// Change controller state to BUSY
			ControllerState = BUSY; 
				  	
       		// Process the selected command
		}
	}	
	        	
#endif

#ifdef SPI_TEST_2

	// Test #2 - Send a ramp of data back to SPI master using SPI interrupts


	// Preload SPI buffer with valid data to  back to the SPI master (SPI data Exchange)

	// Send a 32 word (16-Bit) string of data from the SPI_TX_Buffer to the SPI slave

	// Preload SPI buffer with valid data to  back to the SPI master (SPI data Exchange) 
    // This logic works !!!!

   	WriteSPI1(SPI_TX_Buffer[0]);
  	while(SPI1STATbits.SPITBF);

	while (1)
	{

		// Initialize SPI RX and TX buffers for slave

    	for (i=0; i<32; i++)
		{
			SPI_RX_Buffer[i] = 0;	
			SPI_TX_Buffer[i] = 0;	
		}

		// Read a block of 32 words from SPI master 
		for (j=0; j<32; j++)
		{
			// Read next word from SPI master
			while(!DataRdySPI1());
			SPI_RX_Buffer[j] = ReadSPI1();

			// Echo the data just read from the SPI Master
			SPI_TX_Buffer[j] = SPI_RX_Buffer[j]; 

			// Send back a word to the SPI master (SPI data Exchange)

   			WriteSPI1(SPI_TX_Buffer[j]);
  			while(SPI1STATbits.SPITBF);
  		}

 
   
	}	
#endif


#ifdef TEST_3
	
    // Test #3 - Send a ramp of data back to SPI master a word at a time (Works !!!!)


	// Preload SPI buffer with valid data to  back to the SPI master (SPI data Exchange) 
    // This logic works !!!!

   	WriteSPI1(datard);
  	while(SPI1STATbits.SPITBF);

	// Wait for data from SPI master using 
	while(1)
	{
 
		// Read next word from SPI master
		while(!DataRdySPI1());
		datard = ReadSPI1();

		// Send back a word to the SPI master (SPI data Exchange)

   		WriteSPI1(datard);
  		while(SPI1STATbits.SPITBF);
  
	}
#endif

#ifdef TEST_4

	// Test #4 - Send a ramp of data back to SPI master using strings

    for (i=0; i<32; i++)
	{
		SPI_RX_Buffer[i] = 0;	
		SPI_TX_Buffer[i] = 32 - i;	
	}

	// Preload SPI buffer with valid data to  back to the SPI master (SPI data Exchange)

	// Send a 32 word (16-Bit) string of data from the SPI_TX_Buffer to the SPI slave

	putsSPI1(32, SPI_TX_Buffer);
	while(SPI1STATbits.SPITBF);

	while (1)
	{

 		// Read string of 32 words (16-Bit) of data exchanged by the SPI master into the SPI_RX_Buffer using 1000 millisecond ??? timeout

 		while(!DataRdySPI1());
		getsSPI1(32, SPI_RX_Buffer, 1000);

 
		// Send a 32 word (16-Bit) string of data from the SPI_TX_Buffer to the SPI master

		putsSPI1(32, SPI_TX_Buffer);
		while(SPI1STATbits.SPITBF);

   
	}	

#endif 

	DisableIntSPI1;              // Disable SPI1 interrups

	/* Turn off SPI module and clear IF bit */
	//CloseSPI1();
 
 
	return 0;
}



#endif

//******************************************************************************
//* InitializeSPISlave - Initialize variables and hardware used by the SPI 
//* slave device.
//******************************************************************************

void InitializeSPISlave(void)
{
	int i,j;									// Local Loop Index variables
	unsigned int spi2cfg_1,spi2cfg_2,x,y,z;		// SPI Configuration variables
 	unsigned int baud,uartmode,uartsta;
 
	/* Holds the information about SPI configuartion */
	unsigned int SPICONValue;
	/* Holds the information about SPI Enable/Disable */
	unsigned int SPISTATValue;
	/*Timeout value during which timer1 is ON */
	int timeout;


    // configure all analog pins in digital mode  by writing ADPCFG register

     ADPCFG = 0x003F;

    // Configure the ports as outputs

 	PORTD = 0;
	TRISD = 0;

    // Initialize SPI device address pointer

    //AddressPointer = 0;
    SPI_Interrupt_Count = 0;                      // Number of MSSP interrupts                           

    // Initialize SPI Tx and Rx buffers  
 
    for (i=0; i< SPI_BUFFER_SIZE; i++)
    {

       SPI_Rx_Buffer[i] = 0;
       SPI_Tx_Buffer[i] = 0;
    } 

 
	/* Turn off SPI modules */

	CloseSPI1();


#ifdef SPI_8
	/* Configure SPI1 module to receive 8-Bit timer value in
	slave mode */
 	SPICONValue = FRAME_ENABLE_OFF & 
        FRAME_SYNC_OUTPUT &
		ENABLE_SDO_PIN & 
        //SPI_MODE16_ON &
        SPI_MODE16_OFF &
		SPI_SMP_OFF & 
        SPI_CKE_ON &                  // Works
		SLAVE_ENABLE_ON &
		CLK_POL_ACTIVE_HIGH &         // Works
		MASTER_ENABLE_OFF &
		//SEC_PRESCAL_7_1 &
 		PRI_PRESCAL_64_1 &            // Works
 		//PRI_PRESCAL_16_1 &              
 		//PRI_PRESCAL_1_1 &              
		SEC_PRESCAL_1_1;
#endif

#ifdef SPI_8A
	/* Configure SPI1 module to receive 8-Bit timer value in
	slave mode */
	SPICONValue = FRAME_ENABLE_OFF & 
        FRAME_SYNC_OUTPUT &
		ENABLE_SDO_PIN & 
        //SPI_MODE16_ON &
        SPI_MODE16_OFF &
		SPI_SMP_OFF & 
        SPI_CKE_ON &
		SLAVE_ENABLE_ON &
		CLK_POL_ACTIVE_HIGH &
		MASTER_ENABLE_OFF &
		//SEC_PRESCAL_7_1 &
 		//PRI_PRESCAL_64_1;
		SEC_PRESCAL_1_1 &
 		PRI_PRESCAL_16_1;
#endif


#ifdef SPI_16
	/* Configure SPI1 module to receive 16-Bit timer value in
	slave mode */
	SPICONValue = FRAME_ENABLE_OFF & 
        FRAME_SYNC_OUTPUT &
		ENABLE_SDO_PIN & 
        SPI_MODE16_ON &
 		SPI_SMP_OFF & 
        SPI_CKE_ON &
		SLAVE_ENABLE_ON &
		CLK_POL_ACTIVE_HIGH &
		MASTER_ENABLE_OFF &
		//SEC_PRESCAL_7_1 &
 		//PRI_PRESCAL_64_1;
		SEC_PRESCAL_1_1 &
 		PRI_PRESCAL_16_1;
#endif
 
	SPISTATValue = SPI_ENABLE & SPI_IDLE_CON & SPI_RX_OVFLOW_CLR;
	OpenSPI1(SPICONValue,SPISTATValue );

 	// Configure the SPI1 slave interrupts
	ConfigIntSPI1(SPI_INT_EN & SPI_INT_PRI_5);

 	SetPriorityIntSPI1(SPI_INT_PRI_5);      // Set SPI1 interrupt priority level
 
	EnableIntSPI1;        // Enable SPI1 interrupts
 
 
}

// SPI input with interrupts and SPI buffer

//******************************************************************************
//* SPI_Get_Byte - Get next character from the SPI Rx Buffer.  The function
//* returns a -1 if the routine times out. 
//******************************************************************************
 
byte	SPI_Get_Byte(void)
{
	byte Data = 0;

	return Data;
}
