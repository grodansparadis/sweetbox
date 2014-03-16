//********************************************************************************
//*                                                                              *
//*        Name:  dcmotors.c                                                     *
//* Project Number:  MT2192                                                      *                                                   
//*           Date:  10/15/2007                                                  *                                                  
//*                                                                              *
//*   Description:                                                               * 
//*     This is an embedded 8-Bit or 16-Bit SPI DC motor controller that uses Pulse Width Modulation * 
//* (PWM), that is used to control DC motors using 8-Bit or 16-Bit SPI. The DC motor controller allows     *
//* the operator to send motion control commands from a Master Controller, PC or * 
//* laptop host.  It receives DC motor commands from the host (PC or laptop)     *
//* using the serial port, it then computes the latest PWM command to send to    *
//* the motor and proceeds in turn to send the PWM motor commands to the         *
//* selected DC motor. The controller will also report encoder counts to the     *
//* host for position, speed and direction information using the                 *
//* tachometer/encoder hardware.  Other functions performed by this application  *
//* include sending velocity profiles (ramp, trapezoid, sine) to the motors for  *
//* motion trajectory control.                                                   * 
//*                                                                              *
//* 	  Calibration of motor speeds can be accomplished by setting the         *
//* Calibration flag.  The calibration process includes using two potentiometers *
//* connected to PIC analog inputs AN0 and AN1, so that the speed of each DC     *
//* motor may be varied accordingly.  The actual motor speed may be determined   *
//* using optical encoders.  The PIC has the capability to capture the encoder   *
//* counts using the CCP1 and CCP2 capture registers and also by using the PortB *
//* interrupt source.  These counts may then be converted to RPM(s) and displayed*
//* as the motor's true current speed for calibration.                           *
//*                                                                              *
//*     Future upgrades to this application will include implementing a Fuzzy    *  
//* logic DC motor control algorithm for use in advanced robots.  Providing for  *
//* control of up to 16 DC motors, by using 2 additional multiplexers in order   *
//* to save I/O bits or by using available I/O bits available to the PIC18C452.  *  
//* Another important upgrade to this application includes using the PIC Port B  *
//* interrupts for an emergency stop function.  The high priority ISR will       *
//* invoke the "Z;" reset motor command.                                         *
//*                                                                              *
//* Usage:                                                                       *
//*	The following DC motor Controller commands are available:                    *
//*                                                                              * 
//* 	Cmm       - Calibrate motor speed for motor mm range (00..01). This      *
//*                 command is used to vary the motor speed and direction using  *
//*                 two potentiometers with the blades connected to AN0 and AN1. *
//*                 The true motor speed (RPM) is determined from the counts     *
//*                 received from the optical encoders (if used) and displayed.  *
//*                                                                              * 
//* 	Dmm,xxxx  - Send PWM duty command range xxxx (0000..FFFF) to motor       *
//*                 mm range (00..01). This command is used to vary              *
//*                 the motor speed and direction.                               *
//*                                                                              *
//* 	Emm       - Read DC motor encoders for position, speed and               *
//*                 acceleration and display the information for motor           *
//*                 mm range (00..01).                                           *
//*                                                                              *
//*   Fxx       - Select filter xx, where the filter ID                          *
//*                 range (00..FF) is in a 2 digit hex word format,              *
//*                 where 00 => No Filter, 01 => Average Filter,                 *
//*                 02 => Kalman Filter2, 03 => Filter3,...                      *
//*                                                                              *
//* 	Gmm       - Get new motor commands from the current POT readings and     *
//*                display them for the specific motor mm range (00..01).  This  *
//*                mode will remain in effect until the Hold command is issued.  *
//*                                                                              *
//* 	Hmm       - Hold current POT reading and use it for the next PWM duty    *
//*                 command range (0000..FFFF) to motor                          *
//*                 mm range (00..01). This command is used to vary              * 
//*                 the motor speed and direction.                               *
//*                                                                              *
//*   Nxxxx     - Accept total number of samples, XXXX to be read from           *
//*                 selected Potentiometer, where xxxx is in 4 digit             *
//*                 hex word format  range (00..FFFF)                            *
//*                                                                              *
//* 	Pmm,xx    - Send PWM period command range xx (00..FF) to motor           *
//*                 mm range (00..01). This command is used to vary the          *
//*                 PWM update frequency.                                        *
//*                                                                              *
//* 	Smm       - Read DC motor status including temperature, direction,       *
//*                 speed (RPM read from optical encoders and power state        * 
//*                 (ON/OFF) from motor mm range (00..01).                       *
//*                                                                              *
//* 	Tmm,xx    - Send trajectory profile xx range (00..FF) to motor mm        *
//*                 range (00..01).                                              *
//*                                                                              *
//* 	Vmm,xxxx  - Send velocity command range xxxx (0000..FFFF) to motor       *
//*                 mm range (00..01). This command is used to vary              * 
//*                 the motor speed and direction.                               *
//*                                                                              *
//* 	Zmm;       - Reset the DC motor Controller (brake the motor).  This      *
//*                 command can be used to implement an emergency stop           *
//*                 function.                                                    *
//*                                                                              *
//*	Note:  Each command is a single character followed immediatly by             *
//*	0, 2 or 4 digit hex numbers, and terminated with a semi-colon. Commands      *
//* 	may be sent at any time after the controller has powered up.             *
//*   All commands are terminated with a Carriage Return <CR>                    *
//*                                                                              *
//* The ICD2 Programming Flags are set as follows:                               *
//*     HS                                                                       *
//*     RB3/CCP2                                                                 *
//*                                                                              * 
//*   In order to use PWM2, the CCP2/RB3 pin fuse must be selected when          *
//*   programming the PIC.                                                       *                                                                                                       
//*                                                                              *
//* 	Example:                                                                 *
//*   		Send the following command string to the DC motor controller         *
//* 	to set PWM period to 0xFF using the string:                              *
//*       "P01,FF;"                                                              *
//* 	Where:                                                                   *
//*      	P - Is the motor command to change the PWM period.                   *
//*     	01 - Is the motor ID.                                                *
//*     	FF - Is the 2 character hex PWM period command.                      *
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
#include <stdlib.h>
#include <spi.h>
#include <pwm.h>						// Pulse Width Modulation support
#include <qei.h>						// Quadrature Encoder Interface support
#include "my_types.h"								// Common C Types
//#include "network.h"
#include "spislave.h"
#include "dcmotor.h"								// DC Motor Headers
#include "viport.h" 								// Virtual port driver
#include "uartutil.h"
#include "timeutil.h"

#define SERIAL_DEBUG                                // Turn on debugging
#define SPI_BUFFER_INTERFACE  						// SPI Buffered I/O (using SPI interrupt)
//#define SPI_NETWORK_INTERFACE  					// SPI Message Passing I/O (using SPI interrupt)
#define CHECKSUM_FLAG								// Use checksum processing 
//#define QEI_SUPPORT                               // Quadrature Encoder Interface (QEI)

//#define PWM_TEST				// test the PWM hardware

//#define SPI_TEST_0				// Echo message back to master
//#define SPI_TEST_1     // Works !!!!
//#define SPI_TEST_2
//#define SPI_TEST_3
//#define SPI_TEST_4

// LED patterns used for testing SPI slave functions.  They may not work with the PWM 
// functions because of bit conflicts!
//#define LED_DISPLAY 								// Includes LED Display logic 

#define CALIBRATION_DISPLAY 		3	 	// "11"
#define DUTY_DISPLAY 				7		// "111"
#define PERIOD_DISPLAY				15		// "1111"
#define GET_STATUS_DISPLAY			31		// "11111"
#define NUMBER_OF_SAMPLES_DISPLAY 	63    	// "111111"
#define RESET_DISPLAY				127		// "1111111"


// Constants used for PWM
#define PLL_VALUE 16
#define CLOCK_TICKS 7372800

#define TCY_PER_SECOND (PLL_VALUE*CLOCK_TICKS/4.0)

 
volatile byte CurrentState;									// Current state of motor controller
volatile int NumberOfSamples = 0;							// Current number of samples to be read
volatile word MotorPeriod = 0;								// Period of DC Motor PWM 
byte ControllerState = NO_STATES;					// DC Motor Controller State
byte ControllerErrorCode = NO_ERRORS;				// DC Motor Controller Error code
byte ControllerDiagnosticCode = NO_DIAGNOSTICS;     // DC Motor Controller Diagnostic code

MESSAGE_UNION_TYPE MotorMessage;

VR_8_BIT_PORT_T VR_Port_Data;						// Virtual port data (8-Bits) used for debug

// PWM Configuration variables	
volatile unsigned int config1;								// Holds Period configuration and prescale information for PTCON

volatile unsigned int config2;								// Holds the value be loaded into PWMCON1 register

volatile unsigned int config3;								// Holds the value to configure the special event trigger
													// postscale and dutycycle

volatile unsigned int sptime; 								// Holds the value to be loaded into special 
                                                    // event compare register 

volatile unsigned int config; 								// Holds the PWM interrupt configuration value

volatile unsigned int period;								// Holds the value to be loaded into dutycycle register
												
volatile unsigned int dutycyclereg;							// Holds PWM configuration value 
													// The value of ‘dutycyclereg’ determines the duty cycle
													//register(PDCx) to be written 


volatile unsigned int dutycycle;
volatile unsigned char updatedisable;


volatile unsigned long PeriodInUS = 10000;
volatile unsigned long Duty1InUS = 200;
volatile unsigned long Duty2InUS = 300;
volatile unsigned long Duty3InUS = 400;

 
// declare global QEI variables here
 
volatile unsigned int pos_value;
volatile unsigned int max_value;
volatile unsigned int QEI_Interrupt_Count = 0;   // Count number of times the QEI ISR is entered
byte Buffer[80];


//************************************************************************************************************
//* Declare local functions here
//************************************************************************************************************
//********************************************************************************
// Function Prototypes
//********************************************************************************

// QEI Specific functions
void init_qei(void); 

// PWM Specific functions

void SetPWMCycles(unsigned long PeriodInUS,unsigned long Duty1InUS,unsigned long Duty2InUS, unsigned long Duty3InUS);
void SetupPWM(unsigned long ThePeriod,unsigned long TheDuty1,unsigned long TheDuty2, unsigned long TheDuty3);

#ifdef QEI_SUPPORT
//******************************************************************************
//* Function Name:    _QEIInterrupt                                            *
//* Description:      Is an Interrupt Service Routine that handles the         *
//* Quadrature Encoder Interface (QEI) interrupt each time the POSCNT register *
//* overflows.                                                                 *   
//*                                                                            *
//******************************************************************************
void __attribute__((__interrupt__)) _QEIInterrupt(void)
{
	// Turn off LED on RD1 
	//LED_1 = OFF;
  
	// Increment the QEI Interrupt Count each time the ISR is entered
 	QEI_Interrupt_Count++;

/*
    Printf("trace 1 - ISR");
	Printf("  QEI Interrupt Count = ");  
    Dec(QEI_Interrupt_Count);
	Printf("\r\n");
*/
 	POSCNT = 0;

	IFS2bits.QEIIF = 0; // Clear QEI interrupt flag
}
#endif

//************************************************************************************************************
//* PWM Interrupt service routine
//************************************************************************************************************

void __attribute__((__interrupt__)) _PWMInterrupt(void)
{
	IFS2bits.PWMIF = 0;
}


long int Counter = 0;
char Line[80];  // Temporary buffer

//********************************************************************************
//* main - This is the main Slave SPI DC Motor application  
//********************************************************************************
int main(void)
{

	int i,j;									// Local Loop Index variables
 	word LocalChecksum = 0;				// Local message checksum received

	unsigned int max_value;			// Used to read the Encoders
    char c;
	long int CycleCount = 0;
   word PinID, PortID;			// Pin number and port (A=0..E=4)
   word ProcessorFlag = 0;		// Processor flag (PIC=0, BS2 = 1)
   char Command;              // Menu command
   word Result;               // Data read from pulsin and shiftin
 	VR_8_BIT_PORT_T VR_Port_Data;	// Virtual port data (8-Bits)


     // Setup timers

    SetupTimers(); 

 	// Configure all analog pins as digital


	ADPCFG = 0xFFFF;

#ifdef SERIAL_DEBUG
	// Initialize the UART
	//InitializeUart1(115200);
	InitializeUart2(115200);

	Printf_2("trace 1: \r\n");  
#endif 



	// Initialize the DC Motor application
 	SetupDCMotors();

#ifdef SERIAL_DEBUG
	Printf_2("trace 2: SetupDCMotors\r\n");  
#endif 

#ifdef QEI_SUPPORT
	// Initialize the Quadrature Encoder Interface (QEI)
	init_qei(); 
#endif

	// Initialize variables and hardware used by the SPI slave device.
 
 	InitializeSPISlave();
#ifdef SERIAL_DEBUG
	Printf_2("trace 4: InitializeSPISlave\r\n");  
#endif 

/*
	// Test 1 - UART 1 Test 
	LED_2 = OFF;
	LED_2_DIR = OUTPUT;

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
			LED_2 = ~LED_2;
		}
 
	}

	LED_1 = OFF;
	LED_1_DIR = OUTPUT;


	// Test 2 - UART 2 Test  Works!!!
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
*/

/*
// Test 2 - LED Test Works!!!
   // Initialize 16-Bit Digital I/O ports (PORTE does not exist for the DSPIC30FXXXX) 

    //PORTB = 0x0000;                                          // Clear PORTB
    PORTE = 0x0000;                                          // Clear PORTE
 
    // Initialize port I/O direction

    //TRISB = 0x000F;                                          // PORT B all inputs and outputs.
    TRISE = 0x0000;                                          // PORT E all outputs.
 
	for (i=0; i<256; i++)
	{
		PORTE = i;
 		pause(500);
	}

	for (i=0; i<256; i++)
	{
		VR_Port_Data.Port_Value = i;
	 	Write_Virtual_Port(PORT_E, VR_Port_Data);
		pause(1000);
	}
*/

	//********************************************************************************************
	// Main Control loop
	//********************************************************************************************
/*
// Test the SPI Interface  
while(1)
{
  			// Read a single word from the SPI Bus (SSPBUF) into the Rx buffer.
 			while(!DataRdySPI1());
 			c = ReadSPI1();
		
			// Write response word from Tx buffer to the SPI Bus
     		WriteSPI1(c);
  			while(SPI1STATbits.SPITBF);
			sprintf (Buffer, "%c",c);
            Printf_2(Buffer);
 			 
      
}

*/
	// Initialize the PWM 1, 2 and 3 functions using default values for PWM period and duty cycles 

  	SetPWMCycles(PeriodInUS, Duty1InUS, Duty2InUS, Duty3InUS);
#ifdef SERIAL_DEBUG
	Printf_2("trace 3: SetPWMCycles\r\n");  
#endif 
/*
// Test 3 - Send PWM Commands.  Works!!!!
	while(1)
	{


		// Test the PWM function 

  		SetPWMCycles(PeriodInUS, Duty1InUS, Duty2InUS, Duty3InUS);
 	
		// Vary the duty
		for (i=0; i<10000; i+=1000)
		{
			Duty1InUS = i;
			Duty2InUS = i;
			Duty3InUS = i;
 			SetPWMCycles(PeriodInUS, Duty1InUS, Duty2InUS, Duty3InUS);
			pause(1000);
		}

    	Duty1InUS = 7200;
    	Duty2InUS = 1100;
   		Duty3InUS = 1200;
	
		// Vary the period

		for (i=0; i<10000; i+=1000)
		{
			PeriodInUS = i;
 			SetPWMCycles(PeriodInUS, Duty1InUS, Duty2InUS, Duty3InUS);
			pause(1000);
		}

	}
 
*/

 

#ifdef LED_DISPLAY
	//********************************************************************************************
	// Initialize Virtual Port Utility variables
	//********************************************************************************************

	//for (i=0; i<256; i++)
	for (i=0; i<15; i++)
	{
		VR_Port_Data.Port_Value = i;
	 	Write_Virtual_Port(PORT_E, VR_Port_Data);
		pause(250);
	}
 
	// Initialize LED Display for "rolling bits" when everything is working OK 

	VR_Port_Data.Port_Value = 0xAA;
#endif

	// Preload SPI buffer with valid data to  back to the SPI master (SPI data Exchange)

	// Send a 32 word (16-Bit) string of data from the SPI_TX_Buffer to the SPI slave

	// Preload SPI buffer with valid data to  back to the SPI master (SPI data Exchange) 
    // This logic works !!!!

   	WriteSPI1(SPI_Tx_Buffer[0]);
  	while(SPI1STATbits.SPITBF);

	// Flush the Rx, Tx and message buffers (words)
	for (i=0; i<SPI_BUFFER_SIZE; i++)
	{
 		SPI_Rx_Buffer[i] = 0;                  			// Clear SPI Receive Buffer 
  		SPI_Tx_Buffer[i] = 0;        					// Clear SPI Transmit Buffer   
 	}
#ifdef SERIAL_DEBUG
	Printf_2("trace 5: \r\n");  
#endif 
   	while(1)
   	{
 		// Clear the watchdog timer each time through the major menu loop

		//ClrWdt();

  		// Check to see if a message was received and process it...
 			
   		if (SPI_Rx_Flag == TRUE)
   		{
   			SPI_Rx_Flag = FALSE;

			// Set busy flag indicating to the SPI Master that the peripheral is busy
    		BUSY_FLAG = 1;

#ifdef LED_DISPLAY
			// Display data using LEDs
   			for (i=0; i<SPI_BUFFER_SIZE; i++)
			{
 				// Display the Duty Command to the LEDs  
				VR_Port_Data.Port_Value = (byte) SPI_Rx_Buffer[i];
	 			Write_Virtual_Port(PORT_E, VR_Port_Data);
            	pause(1000);

            }
#endif			

            // Mainline processing is done here ...

   			for (i=0; i<SPI_BUFFER_SIZE; i++)
			{
 				// Clear the watchdog timer each time through the major menu loop

				//ClrWdt();

				// Disable interrupts
				//DISICNT = 0x3FFF;   
				// Process SPI RX buffer value and copy it to SPI TX buffer  
				//SPI_Tx_Buffer[i] =  SPI_Rx_Buffer[i] / 100;			// Scale the input
				//SPI_Tx_Buffer[i] =  SPI_Rx_Buffer[i] * 100;			// Scale the input
				SPI_Tx_Buffer[i] = SPI_Rx_Buffer[i];					// Echo the input works !!!!
#ifdef SERIAL_DEBUG
				sprintf(Line, "Received: i = %d, SPI_Rx_Buffer[%d] = %0d \r\n",i, i, SPI_Rx_Buffer[i]);
                Printf_2(Line);
#endif

#ifdef LED_DISPLAY
 				// Display the Duty Command to the LEDs  
 
				//VR_Port_Data.Port_Value = (byte) SPI_Rx_Buffer[i];
	 			//Write_Virtual_Port(PORT_E, VR_Port_Data);
            	//pause(1000);				// Enable interrupts
#endif
				//DISICNT = 0x0000; 
			}
   			// Clear busy flag indicating to the SPI Master that the peripheral is available
    		goto Next_Message;


  			// Copy the entire DC Motor Controller message received from the SPI Slave
   			// to local storage for processing.  It may include sensor data, telemetry and status 
   			// information and is received concurrently with each message that the Master SPI controller sends.
 			
    		memcpy(&MotorMessage.Message, SPI_Rx_Buffer+1, sizeof(MotorMessage.Message));   
#ifdef SERIAL_DEBUG
#ifdef SPI_8
 			Printf_2("trace 6: 8-Bit SPI - SPI_Rx_Flag == TRUE\r\n");  
            sprintf(Line, "SPI_Interrupt_Count = %d \r\n", SPI_Interrupt_Count);

            Printf_2(Line);
			//pause(2000);
/*
   			for (i=0; i<MESSAGE_BUFFER_SIZE; i++)
			{
				if ((i<15) || (i>30))
				{
					sprintf(Line, "Received: i = %d, MotorMessage.Message.Data[%d] = %04X\r\n",i, i, MotorMessage.Message.Data[i]);
                	Printf_2(Line);
				}
            }
*/
   			for (i=0; i<SPI_BUFFER_SIZE; i++)
			{
				sprintf(Line, "Received: i = %d, SPI_Rx_Buffer[%d] = %0d \r\n",i, i, SPI_Rx_Buffer[i]);
                Printf_2(Line);
            }

   			// Clear busy flag indicating to the SPI Master that the peripheral is available
    		goto Next_Message;
#endif
#ifdef SPI_16
 			Printf_2("trace 6: 16-Bit SPI - SPI_Rx_Flag == TRUE\r\n");  
            sprintf(Line, "SPI_Interrupt_Count = %d \r\n", SPI_Interrupt_Count);
            Printf_2(Line);
			pause(2000);
   			for (i=0; i<MESSAGE_BUFFER_SIZE; i++)
			{
				if ((i<15) || (i>30))
				{
					sprintf(Line, "Received: i = %d, MotorMessage.Message.Data[%d] = %04X\r\n",i, i, MotorMessage.Message.Data[i]);
                	Printf_2(Line);
				}
			}
/*
   			for (i=0; i<SPI_BUFFER_SIZE; i++)
			{
				sprintf(Line, "Received: i = %d, SPI_Rx_Buffer[%d] = %0ld\r\n",i, i, SPI_Rx_Buffer[i]);
                Printf_2(Line);
            }
*/
#endif

#endif 

#ifdef CHECKSUM_FLAG
	 		// Compute message checksum
		
   	 	 	LocalChecksum = 0;				// clear message checksum

      		// Process the selected command
  
  			for (i=0; i<MESSAGE_BUFFER_SIZE; i++)
 			{
				if (i < (MESSAGE_BUFFER_SIZE-2))
				{ 
					// Compute local checksum
					LocalChecksum += MotorMessage.Message.Data[i];

					// Return test pattern
        			//MotorMessage.Message.Data[i] = DATA_BUFFER_SIZE - SPI_Rx_Buffer[i];
					//MotorMessage.Message.Data[i] = -MotorMessage.Message.Data[i]*100;
					//MotorMessage.Message.Data[i] = -MotorMessage.Message.Data[i];
				}
 			}	

	

#ifndef TEST				  	
/*

    		for (i=0; i<DATA_BUFFER_SIZE; i++)
 			{
				MotorMessage.Message.Data[i] = MotorMessage.Message.Data[i] *100;				
			}
*/
 			  	
       		// Process the selected command

    		for (i=0; i<DATA_BUFFER_SIZE; i++)
 			{
				//MotorMessage.Message.Data[i] = MotorMessage.Message.Data[i] + 5;
				//MotorMessage.Message.Data[i] = DATA_BUFFER_SIZE - i;
				//MotorMessage.Message.Data[i] = i;
				//MotorMessage.Message.Data[i] = Counter;
				//MotorMessage.Message.Data[i] = DATA_BUFFER_SIZE - MotorMessage.Message.Data[i];
				//MotorMessage.Message.Data[i] = - MotorMessage.Message.Data[i]*100;
				//MotorMessage.Message.Data[i] = - MotorMessage.Message.Data[i]*100;
				
			}

 			//Counter++;

     		// Echo the SPI Rx data received back to the SPI Master (need to move twice as many bytes)
            //       to             from
   			//memcpy(SPI_Tx_Buffer, SPI_Rx_Buffer, SPI_BUFFER_SIZE*2);    // Works !!!
   	
			// Copy the Motor Message to the SPI Transmit Buffer so that it can be sent to the SPI peripheral
    		//      Destination                   Source
    		//memcpy( SPI_Tx_Buffer+TX_DATA_OFFSET+1, &MotorMessage.Message, DATA_BUFFER_SIZE*2);   // This works !!!!

			// Mainline processing is done here ...

   			for (i=0; i<SPI_BUFFER_SIZE; i++)
			{
 				// Clear the watchdog timer each time through the major menu loop

				//ClrWdt();

				// Disable interrupts
				//DISICNT = 0x3FFF;   
				// Process SPI RX buffer value and copy it to SPI TX buffer  
				//SPI_Tx_Buffer[i] =  SPI_Rx_Buffer[i] / 100;			// Scale the input
				//SPI_Tx_Buffer[i] =  SPI_Rx_Buffer[i] * 100;			// Scale the input
				SPI_Tx_Buffer[i] = SPI_Rx_Buffer[i];					// Echo the input works !!!!
				// Enable interrupts
				//DISICNT = 0x0000; 
			}

 
#endif    
   

    		// Change controller state to BUSY
			//ControllerState = BUSY; 

      		// Process the selected command
            //********************************  Missing Logic here ***************************


/*
			// Display the Duty Command to the LEDs works !!!!
			VR_Port_Data.Port_Value = (byte) MotorMessage.DutyCommand.Duty[0];
	 		Write_Virtual_Port(PORT_E, VR_Port_Data);
            pause(1000);

			Duty1InUS = MotorMessage.DutyCommand.Duty[0]*1000;
			Duty2InUS = MotorMessage.DutyCommand.Duty[1]*1000;
 			SetPWMCycles(PeriodInUS, Duty1InUS, Duty2InUS);

			Duty1InUS = SPI_Rx_Buffer[5]*1000;
			Duty2InUS = SPI_Rx_Buffer[5]*1000;
 			SetPWMCycles(PeriodInUS, Duty1InUS, Duty2InUS);

#ifdef LED_DISPLAY

			// Display the Selected Command to the LEDs 
			VR_Port_Data.Port_Value = (byte) MotorMessage.SelectedCommand.Command;
	 		Write_Virtual_Port(PORT_E, VR_Port_Data);
#endif

  */

#ifdef TEST	  
 					
			// Test the get status command -  Works !!!
			//ProcessGetStatusCommand();
    		MotorMessage.GetStatusCommand.MessageID = 0x444; 		 			// Store message ID
   			MotorMessage.GetStatusCommand.ControllerState = 0x333; 		 			// Store current Controller State
    		MotorMessage.GetStatusCommand.ControllerErrorCode = 0x222; 			// Store current Controller Error Code
    		MotorMessage.GetStatusCommand.ControllerDiagnosticCode = 0x111;  // Store current Diagnostic Code

      	
    		// Generic Message ID received  from the SPI Slave here ...
    		MotorMessage.DutyCommand.MessageID = 777;
   			 
  			// Generic command received  from the SPI Slave here  
    		MotorMessage.DutyCommand.Command = 888;
  
			// Generic Motor Message  status received  from the SPI Slave here
	 		MotorMessage.SelectedCommand.CommandStatus = 999;

			// Update motor controller status flags and return it to SPI master
			//MotorMessage.DutyCommand.StatusFlags  = 0xAAAA; 		 			// Store current DC Motor status flags 
 
#endif
/* 
// Include this code for simple SPI tests

     		// Copy the response message to the SPI Tx buffer and send it back to the SPI Master  
    		memcpy(SPI_Tx_Buffer, &MotorMessage.Message, sizeof(MotorMessage.Message));    

			// Block the SPI master for 3 seconds
			//pause(1000);
			//pause(3000);
			//pause(500);

  			// Clear busy flag indicating to the SPI Master that the peripheral is available
    		BUSY_FLAG = 0;
  						  	
 		}
	}	

*/

#ifndef NEW_CODE

// Include this code for the application

    		// Change controller state to BUSY
			ControllerState = BUSY; 
#ifdef SERIAL_DEBUG
			Printf_2("trace 9: ControllerState = BUSY\r\n");  
#endif 
      		// Process the selected command
  			        	
 			switch ((byte) MotorMessage.SelectedCommand.Command)
 			{
				case CALIBRATE_MOTOR_COMMAND:
				{
 
#ifdef SERIAL_DEBUG
 					Printf_2("trace 10: CALIBRATE_MOTOR_COMMAND\r\n");  
#endif 
					// Process the DC Motor Controller Calibrate Command
 					ProcessCalibrateMotorCommand(MotorMessage.CalibrateCommand.MotorID);
 				
					break;
				} 
			
				case DUTY_COMMAND:
				{
 		
#ifdef SERIAL_DEBUG
					Printf_2("trace 11: DUTY_COMMAND\r\n");  
#endif 
 					// Process the PWM duty command message 
  					ProcessDutyCommand(MotorMessage.DutyCommand.Duty, MotorMessage.DutyCommand.NumberOfMotors);
  					
  					break;
				}
			
				case GET_POT_COMMAND:
				{
#ifdef SERIAL_DEBUG
					Printf_2("trace 12: GET_POT_COMMAND\r\n");  
#endif 
 					// Process the Get Pot command message
					ProcessGetPotCommand(MotorMessage.GetPotCommand.MotorID);
					
 					break;
				}
			
				case HOLD_POT_COMMAND:
				{

#ifdef SERIAL_DEBUG
  					Printf_2("trace 13: HOLD_POT_COMMAND\r\n");  
#endif 
					// Process Hold Pot Command message
					ProcessHoldPotCommand(MotorMessage.HoldPotCommand.MotorID);
					
   					break;
				}
			
				case NUMBER_SAMPLES_COMMAND:
				{

#ifdef SERIAL_DEBUG
 					Printf_2("trace 14: NUMBER_SAMPLES_COMMAND\r\n");  
#endif 
  					// Process Get number of samples command message
  					ProcessNumberSamplesCommand(MotorMessage.SampleCommand.NumberOfSamples);
   					break;
				}

				case PERIOD_COMMAND:
				{
#ifdef SERIAL_DEBUG
  					Printf_2("trace 15: PERIOD_COMMAND\r\n");  
#endif 
 					// Process the Send period command message

					ProcessPeriodCommand(MotorMessage.PeriodCommand.Period);
   					break;
				}
				
				case GET_STATUS_COMMAND:
				{
  					// Process the Get Status Command message
                   
#ifdef SERIAL_DEBUG
  					Printf_2("trace 16:  GET_STATUS_COMMAND\r\n");  
#endif 
					ProcessGetStatusCommand();
					
   					break;
				}
				
				case RESET_COMMAND:
				{
  					// Process the Reset controller command message

#ifdef SERIAL_DEBUG
  					Printf_2("trace 17:  RESET_COMMANDD\r\n");  
#endif 
					ProcessResetCommand(MotorMessage.ResetCommand.MotorID);
					
   					break;
				}

			}
 
 			// Compare the Message checksum to the local checksum

			if (MotorMessage.Message.Data[CHECKSUM_OFFSET] == LocalChecksum)
			{
				// Success - Message checksum compares to local checksum
  				MotorMessage.Message.Data[CHECKSUM_OFFSET] = LocalChecksum; // This works !!!
#ifdef SERIAL_DEBUG
				Printf_2("trace 7: MotorMessage.Message.Data[CHECKSUM_OFFSET] == LocalChecksum\r\n");  
#endif 

#ifdef LED_DISPLAY

				// Rolling bit display indicating everything is OK
	 			Write_Virtual_Port(PORT_E, VR_Port_Data);
				VR_Port_Data.Port_Value = ~VR_Port_Data.Port_Value;

				// Rolling LED display
				VR_Port_Data.Port_Value = 0xAA;
	
				for (j=0; j<5; j++)
				{

	 				Write_Virtual_Port(PORT_E, VR_Port_Data);
					VR_Port_Data.Port_Value = ~VR_Port_Data.Port_Value;
					pause(200);
				}

#endif

			}
			else
			{
				// Fail - All LEDs Blinking at once indicating that the message checksum does not compare to local checksum

#ifdef SERIAL_DEBUG
				Printf_2("trace 8: \r\n");  
				Printf_2("SPI Communications Error!!! \r\n");  
#endif 	
 			    VR_Port_Data.Port_Value = LocalChecksum;
	 			Write_Virtual_Port(PORT_E, VR_Port_Data);

				pause(1000);
				// Blink LEDs indicating error condition
				VR_Port_Data.Port_Value = 0xFF;

				while(1)
				{
	 				Write_Virtual_Port(PORT_E, VR_Port_Data);
					VR_Port_Data.Port_Value = ~VR_Port_Data.Port_Value;
					pause(500);
				} 
			}
 
#endif			  		
    		// Change controller state to BUSY
			//ControllerState = BUSY; 

       		// Copy the updated message response (if any) to the transmit buffer so that
    		// it can be sent back to the SPI Master Controller ( 
      		//      Destination                   Source
    		memcpy(SPI_Tx_Buffer, &MotorMessage.Message, sizeof(MotorMessage.Message));    
			
			// Cause the SPI master to wait 3 seconds 
  			//pause(3000);
			//pause(5000);

Next_Message:
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


			//pause(3000);
			pause(1000);

  			// Clear busy flag indicating to the SPI Master that the peripheral is available
    		BUSY_FLAG = 0;
  						  	
 		}
	}	
	        	
#endif

  

#ifdef SPI_TEST_1
 
	// Test #1 - Send a ramp of data back to SPI master using SPI interrupts, it works !!!!!


	// Preload SPI buffer with valid data to  back to the SPI master (SPI data Exchange)

	// Send a 32 word (16-Bit) string of data from the SPI_TX_Buffer to the SPI slave

	// Preload SPI buffer with valid data to  back to the SPI master (SPI data Exchange) 
    // This logic works !!!!

   	WriteSPI1(SPI_Tx_Buffer[0]);
  	while(SPI1STATbits.SPITBF);

	// Flush the Rx, Tx and message buffers (words)
	for (i=0; i<SPI_BUFFER_SIZE; i++)
	{
 		SPI_Rx_Buffer[i] = 0;                  			// Clear SPI Receive Buffer 
  		SPI_Tx_Buffer[i] = 0;        					// Clear SPI Transmit Buffer   
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
   			SPI_Rx_Flag = FALSE;

			// Set busy flag indicating to the SPI Master that the peripheral is busy
    		BUSY_FLAG = 1;

			for (i=0; i<DATA_BUFFER_SIZE; i++)
			{
       			MotorMessage.Message.Data[i] = 0;
 			}		 				

  			// Copy the entire DC Motor Controller message received from the SPI Slave
   			// to local storage for processing.  It may include sensor data, telemetry and status 
   			// information and is received concurrently with each message that the Master SPI controller sends.
   			
   			memcpy(&MotorMessage.Message.Data, SPI_Rx_Buffer, DATA_BUFFER_SIZE);   // This works!!!!

				  	
       		// Process the selected command

    		for (i=0; i<DATA_BUFFER_SIZE; i++)
    		//for (i=0; i<DataSize; i++)
			{
				//MotorMessage.Message.Data[i] = MotorMessage.Message.Data[i] + 5;
				MotorMessage.Message.Data[i] = DATA_BUFFER_SIZE - i;
				//MotorMessage.Message.Data[i] = i;
			}

     		// Echo the SPI Rx data received back to the SPI Master (need to move twice as many bytes)
            //       to             from
   			//memcpy(SPI_Tx_Buffer, SPI_Rx_Buffer, SPI_BUFFER_SIZE*2);    // Works !!!
   	
			// Copy the Motor Message to the SPI Transmit Buffer so that it can be sent to the SPI peripheral
    		//      Destination                   Source
    		memcpy( SPI_Tx_Buffer+TX_DATA_OFFSET+1, &MotorMessage.Message.Data, DATA_BUFFER_SIZE);   // This works !!!!

/*
   			for (i=0; i<SPI_BUFFER_SIZE; i++)
			{
 				// Clear the watchdog timer each time through the major menu loop

				//ClrWdt();

				// Disable interrupts
				//DISICNT = 0x3FFF;   
				// Process SPI RX buffer value and copy it to SPI TX buffer  
				//SPI_Tx_Buffer[i] =  SPI_Rx_Buffer[i] / 100;			// Scale the input
				SPI_Tx_Buffer[i] = SPI_Rx_Buffer[i];					// Scale the input
				//SPI_Tx_Buffer[i] = SPI_Rx_Buffer[i];					// Echo the input
				// Enable interrupts
				//DISICNT = 0x0000; 
			}
*/
		

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
			//ControllerState = BUSY; 

      		// Process the selected command
            //********************************  Missing Logic here ***************************
			pause(3000);
  			// Clear busy flag indicating to the SPI Master that the peripheral is available
    		BUSY_FLAG = 0;
  						  	
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
//******************************************************************************
//* Function Name:    init_qei                                                 *
//* Description:      Configures the Quadrature Encoder Interface and sets all *
//* ana;og I/O bits to digital inputs.                                         *   
//*                                                                            *
//******************************************************************************
void init_qei(void) 
{ 

	// Configure all analog pins as digital

	//ADPCFG = 0xFFFF;

	// Reset previous QEI settings
	CloseQEI(); 

	// Configure the QEI Interrupt for priority 1 and enable it
	ConfigIntQEI(QEI_INT_PRI_1 & QEI_INT_ENABLE); 

	// Clear the POSCNT register and set the MAXCNT register with the desired maximum count
	POSCNT = 0x0000; 
	MAXCNT = 0x007F; 

	// Open the QEI Interface with Index Reset disabled
	OpenQEI(QEI_MODE_x4_MATCH & 
		QEI_IDLE_CON & 
		QEI_UP_COUNT & 
		QEI_LOGIC_CONTROL_IO & 
		QEI_INPUTS_SWAP & 
		QEI_CLK_PRESCALE_1 & 
		QEI_INDEX_RESET_DISABLE,0); 

/*

	TRISDbits.TRISD1 = 0;
	PORTDbits.RD1 = 1; // turn off LED on RD1 

	// Enable QEI Interrupt and Priority to "1" 
	ConfigIntQEI(QEI_INT_PRI_1 & QEI_INT_ENABLE);

	POSCNT = 0;
	MAXCNT = 0xFFFF;
	// Open the Quadrature Encoder Interface (QEI)
	//OpenQEI(QEI_INT_CLK & QEI_INDEX_RESET_ENABLE &
	OpenQEI(QEI_INT_CLK & QEI_INDEX_RESET_DISABLE &
		QEI_CLK_PRESCALE_256 &
		QEI_GATED_ACC_DISABLE & QEI_INPUTS_NOSWAP &
		QEI_MODE_TIMER & QEI_DIR_SEL_CNTRL &
		QEI_IDLE_CON, 0);
	QEICONbits.UPDN = 1;
*/

} 

//********************************************************************************
//* SetupPWM - This functions sets up the PWM unit. It uses units 1 & 2, sets them
//* to have a common baseline of 'period', and duty cycles 'duty1', 'duty2': all 
//* variables are measured in ticks. 
//********************************************************************************

void SetupPWM(unsigned long ThePeriod,unsigned long TheDuty1,unsigned long TheDuty2, unsigned long TheDuty3)
{
	ConfigIntMCPWM(PWM_INT_DIS & PWM_FLTA_DIS_INT & PWM_INT_PR1 & PWM_FLTA_INT_PR0);
	SetDCMCPWM(1,2*TheDuty1-1,0);
	SetDCMCPWM(2,2*TheDuty2-1,0);
	SetDCMCPWM(3,2*TheDuty3-1,0);

/*
	OpenMCPWM(ThePeriod,0,PWM_EN & PWM_OP_SCALE1 & PWM_IPCLK_SCALE16 & PWM_MOD_FREE,
			PWM_MOD1_COMP & PWM_MOD2_COMP & PWM_PDIS3H &PWM_PDIS2H & PWM_PEN1H &PWM_PDIS3L & PWM_PEN2L & PWM_PDIS1L,
			PWM_SEVOPS1 & PWM_OSYNC_PWM & PWM_UEN);
*/
	OpenMCPWM(ThePeriod,0,PWM_EN & PWM_OP_SCALE1 & PWM_IPCLK_SCALE16 & PWM_MOD_FREE,
			PWM_MOD1_COMP & PWM_MOD2_COMP & PWM_MOD3_COMP & PWM_PEN3H & PWM_PEN2H & PWM_PEN1H &PWM_PEN3L & PWM_PEN2L & PWM_PEN1L,
			PWM_SEVOPS1 & PWM_OSYNC_PWM & PWM_UEN);
}


//********************************************************************************
//* SetPWMCycles - This function actually computes the PWM1 amd PWM2 period and the duty cycle 
//* factors converting times measured in microseconds into ticks. It's all pretty obvious: 
//*   For modifying purposes, you need to: 
//*    (1) Possibly change to different clock scale factors 
//*    (2) Change the various PWM_PDIS* and PWM_PEN* statements 
//*    (3) Change from free-running mode (which is what I use) to whichever mode you prefer 
//* Note: It uses the TCY_PER_SECOND conversion factor defined above.  
//********************************************************************************
void SetPWMCycles(unsigned long PeriodInUS,unsigned long Duty1InUS,unsigned long Duty2InUS, unsigned long Duty3InUS)
{
	unsigned long Period;
	unsigned long Duty1Clocks,Duty2Clocks,Duty3Clocks;
	
	Period=(unsigned long)(((float)TCY_PER_SECOND/16.0)*(PeriodInUS/1000000.0))-1;
	
	Duty1Clocks=(unsigned long)(Period*Duty1InUS/(float)PeriodInUS);
	Duty2Clocks=(unsigned long)(Period*Duty2InUS/(float)PeriodInUS);
	Duty3Clocks=(unsigned long)(Period*Duty2InUS/(float)PeriodInUS);

	/* now call function */

	SetupPWM(Period,Duty1Clocks,Duty2Clocks, Duty3Clocks);
}



//********************************************************************************
//* SetupDCMotors - Initializes Timers, program variables and peripheral registers
//* for the motor interrupt test code.
//********************************************************************************
void            SetupDCMotors(void)
{
	int i,j;									// Local Loop Index variables
	unsigned int spi2cfg_1,spi2cfg_2,x,y,z;		// SPI Configuration variables
	unsigned int channel,adc12cfg_1,adc12cfg_2,adc12cfg_3,adc12cfg_p,adc12cfg_s; // ADC Configuration variables
	unsigned int baud,uartmode,uartsta;

  
	/* Holds the information about SPI configuartion */
	unsigned int SPICONValue;
	/* Holds the information about SPI Enable/Disable */
	unsigned int SPISTATValue;
	/*Timeout value during which timer1 is ON */
	int timeout;

    // Configure Port D as output

 	PORTD = 0;
	TRISD = 0;

 
    // configure all analog pins in digital mode  by writing ADPCFG register
 	ADPCFG = 0xFFFF;
    //ADPCFG = 0x003F;


   // Initialize 16-Bit Digital I/O ports for the virtual ports B and E

    //PORTB = 0x0000;                                          // Clear PORTB
    PORTE = 0x0000;                                          // Clear PORTE
 
    // Initialize port I/O direction

    //TRISB = 0x000F;                                          // PORT B all inputs and outputs.
    TRISE = 0x0000;                                          // PORT E all outputs.
 
    // Configure PORT interrupts with Interrupt on change off and CN pull-up resistors disabled

	//ConfigCNPullups(0x0000);					// Disable CN pin pullups (inputs)  
	//ConfigCNPullups(0xFFF);					// Enable CN pin pullups (inputs) 

 
	PORTDbits.RD0 = 1;		// Turn off LED indicating entry into SPI ISR
	BUSY_FLAG_DIR = OUTPUT;			// Busy line set to output
  

    // Initialize SPI device address pointer

    //AddressPointer = 0;
    SPI_Interrupt_Count = 0;                      // Number of MSSP interrupts                           

    // Initialize ring buffer to zero  
 
    for (i=0; i< SPI_BUFFER_SIZE; i++)
    {

       SPI_Rx_Buffer[i] = 0;
       SPI_Tx_Buffer[i] = 0;
    } 

/*
 
    OpenADC(ADC_0ANA, ADC_INT_OFF);
    
    // Initialize ports  

    PORTA = 0x00;                                          // Clear PORTA
    PORTB = 0x00;                                          // Clear PORTB
    //PORTC = 0x00;                                        // Clear PORTC (Some Port C bits are reserved for USART)
    PORTD = 0x00;                                          // Clear PORTD
    PORTE = 0x00;                                          // Clear PORTE

    // Initialize port I/O direction for output

 
    TRISA = 0x00;                                          // PORT A all outputs.
    TRISB = 0x00;                                          // PORT B all outputs.
    //TRISC = 0x00;                                        // PORTC all outputs (Some Port C bits are reserved for USART).
    TRISD = 0x00;                                          // PORTD all outputs.
    TRISE = 0x00;                                          // PORTE all outputs.

	// Initialize the SPI Slave Pin directions
	
	SPI_SCL_DIR = INPUT; 									// Set SPI Clock Direction to Input
 	SPI_SI_DIR = INPUT;										// Set SPI Input Direction  to Input
    SPI_SO_DIR = OUTPUT; 									// Set SPI Output Direction to Output

    SS_DIR = INPUT;									  		// Set Slave Select line to Input

    TRACE0_DIR   = OUTPUT;								   // Set TRACE 0 line to output

    // Turn OFF trace 0

    TRACE0 = OFF;
*/
    
#ifdef NEW_CODE
 
/*
   TRISCbits.TRISC2 = OUTPUT;				// PWM1 Channel 
   TRISCbits.TRISC1 = OUTPUT;				// PWM2 Channel
   TRISEbits.TRISE7 = OUTPUT;				// PWM2 Channel
*/
	
/*
   The period, Tpwm for PWM1 is initialized using the following equation.  This can be changed to 
   any appropriate value using the following information:
   
   Tpwm =  (Tcy X (PTPER + 1)) / (PTPER prescale value)

where:
   Tcy = (1/7.37 MHz) X 4

example:
   With a value of 0x1FFF (8191 Decimal) and a PTMR prescale of one, the PWM period is
   4.44 ms, which results in a PWM frequency of 225 Hz. 
 
   open PW1, Arguments: period
	The value of period can be any value from 0x00 to 0xff.
	This value determines the PWM frequency by using the
 
*/

	// Configure pwm interrupt enable/disable and set interrupt
	// priorties. 

/*
	config = (PWM_INT_DIS & PWM_FLTA_DIS_INT & PWM_INT_PR1
				& PWM_FLTA_INT_PR0);
*/

	config = (PWM_INT_EN & PWM_FLTA_DIS_INT & PWM_INT_PR1
				& PWM_FLTA_INT_PR0
				& PWM_FLTA_DIS_INT & PWM_FLTA_INT_PR0);
                                      
	ConfigIntMCPWM( config );

/* 
        Set duty cycle:
	Arguments: dutycycle
	The value of dutycycle can be any 10-bit number. Only
	the lower 10-bits of dutycycle are written into the duty-cycle
	registers. The dutycycle, or more specifically the
	high time of the PWM waveform, can be calculated from
	the following formula:
	PWM x Dutycycle = (DCx<9:0>) x Tosc????????????????????
	where DCx<9:0> is the 10-bit value from the
	PWxDCH:PWxDCL registers.???????????????????????????????

	Remarks: This function writes the new value for dutycycle to the
	specified PWM channel dutycycle registers.
	The maximum resolution of the PWM waveform can be
	calculated from the period using the following formula:
	Resolution (bits) = log(Fosc/Fpwm) / log(2) ????????????
 
*/
	// Configure PWM to generate square wave of 50% duty cycle 
	dutycyclereg = 1;
	dutycycle = 0x3FFF;
	updatedisable = 0;
	SetDCMCPWM(dutycyclereg,dutycycle,updatedisable);

	period = 0x7fff;
 	sptime = 0x0;

	// PTCON1 Register configuration 
	config1 = (PWM_EN & PWM_IDLE_STOP & PWM_OP_SCALE16
				& PWM_IPCLK_SCALE16 &
				  	PWM_MOD_UPDN);

	// Configure PWM1
	//config2 = (PWM_MOD1_COMP & PWM_PDIS3H &
	//				PWM_PDIS2H & PWM_PEN1H &  
	//				PWM_PDIS3L & PWM_PDIS2L & PWM_PEN1L);

	config2 = (PWM_MOD1_COMP & PWM_PEN3H &
					PWM_PEN2H & PWM_PEN1H &  
					PWM_PEN3L & PWM_PEN2L & PWM_PEN1L);

	// Configure PWM2
	config3 = (PWM_SEVOPS1 & PWM_OSYNC_PWM & PWM_UEN);

	// Open PWM1 and PWM2 for selected period

	OpenMCPWM(period,sptime,config1,config2,config3);

	//OpenMCPWM(MotorPeriod,sptime,config1,config2,config3);
	
       
/*
	//open timer
	//OpenTimer1(TIMER_INT_OFF&T1_SOURCE_INT&T1_T2_8BIT);
	OpenTimer1(TIMER_INT_OFF&T1_SOURCE_INT&T1_8BIT_RW);
	//OpenTimer3(TIMER_INT_OFF&T3_SOURCE_INT&T3_8BIT_RW);
	
*/
 

#endif


    // Main loop receives messages from SPI Master and processes them

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


#ifdef NEW_CODE
 
    // Initialize sensor table

    for (i = 0; i < MAX_MOTORS; i++)
    {
 
        SensorTable[i].RawSensorValue = 0;
        SensorTable[i].AverageSensorValue = 0.0;
        SensorTable[i].SensorGain = 0.0;
        SensorTable[i].SensorOffset = 0.0;
        SensorTable[i].MinSensorValue = 0.0;
        SensorTable[i].MaxSensorValue = 0.0;
  
    }


    // Initialize sensor id array

    SensorTable[0].Channel = ADC_CH0;
    SensorTable[1].Channel = ADC_CH1;
/*
    SensorTable[2].Channel = ADC_CH2;
    SensorTable[3].Channel = ADC_CH3;
    SensorTable[4].Channel = ADC_CH4;
    SensorTable[5].Channel = ADC_CH5;
    SensorTable[6].Channel = ADC_CH6;
    SensorTable[7].Channel = ADC_CH7;	
*/
    
#endif

	// Initialize DC Motor Controller State, Error codes and Diagnostic codes
	
	ControllerState = INITIALIZED; 	  
   	ControllerErrorCode =  NO_ERRORS;	   
  	ControllerDiagnosticCode = NO_DIAGNOSTICS;  
}

//********************************************************************************                                                                                      ;
//* ReadSensor - Reads the raw sensor value from the selected A/D channel.
//********************************************************************************                                                                                      ;

int             ReadSensor(word SensorID)
{
    int	RawSensorValue;                        					// Raw sensor value
/*
    //SetChanADC(SensorTable[SensorID].Channel);              	// Select the sensor A/D channel
 
 	 // Select the desired A/D channel
 	 
    if (SensorID == 0)
	 {
    	SetChanADC(ADC_CH0);              	// Select the sensor A/D channel #0
	 } 
    else
	 {
    	SetChanADC(ADC_CH1);              	// Select the sensor A/D channel #1
	 }

    //DelayMilliSeconds(1);                                    	// Delay 1 millisecond for A/D conversion to complete
    //pause(1);                                    	// Delay 1 millisecond for A/D conversion to complete
 
    //Delay10TCYx(5);                                        	// Delay for 50TCY
    Delay10TCYx(10);                                        	// Delay for 100TCY

    // Convert the analog reading to a digital value
    
    ConvertADC();                                          		// Start Conversion
    //Delay10TCYx(5);                                        	// Delay for 50TCY
    Delay10TCYx(10);                                        	// Delay for 100TCY
    
    //DelayMilliSeconds(20);                                     // Delay 20 milliseconds for A/D conversion to complete
    //pause(1);                                     // Delay 1 millisecond for A/D conversion to complete
 
    // Read the converted value
    
    RawSensorValue = ReadADC();                            		// read result and store it in RawSensorValue
    //DelayMilliSeconds(5);                                    // Delay at least 5 milliseconds between readings  
    //pause(1);                                     // Delay 1 millisecond for A/D conversion to complete
*/
    return RawSensorValue;                                  	// Return actual sensor value read
}

//********************************************************************************                                                                                      ;
//* AverageSensor - Reads the raw sensor value from the selected A/D channel for
//* a number of samples and filters it using an averaging filter.
//********************************************************************************                                                                                      ;

float             AverageSensor(word SensorID)
{
    float	AverageSensorValue;                           	// Averaged sensor value
    int i;                                                  // Loop index 

	AverageSensorValue = 0.0;

	for (i = 0; i < NumberOfSamples; i++) 
	{
		AverageSensorValue = AverageSensorValue + (float) ReadSensor(SensorID);
	}

	AverageSensorValue = AverageSensorValue / (float) NumberOfSamples;

   return (AverageSensorValue);

}

//******************************************************************************
//* GetMotorPosition - Get current motor position information (encoder counts
//* and direction) and send it as status to the host or use it during the calibration 
//* process.
//******************************************************************************

void GetMotorPosition(word MotorID)
{
}

//******************************************************************************
//* GetMotorStatus - Get current motor status information (temperature, 
//* current draw, etc.) and send it to the Host.
//******************************************************************************

void GetMotorStatus(word MotorID)
{
}

//******************************************************************************
//* SetPeriod - Set a new PWM period to be used for the next PWM DC motor Command, range 
//* (0x0000..FFFF) in microseconds to the selected motor device, range (01..02).
//******************************************************************************

void SetPeriod(longword ThePeriod)
{
  
	// Update the selected duty cycle here

  	SetPWMCycles(ThePeriod, Duty1InUS, Duty2InUS, Duty3InUS);

}

//******************************************************************************
//* SetDuty - a new PWM duty to be used for the next PWM DC motor Command, range 
//* (0000..FFFF) microseconds to the selected motor device, range (00..01).
//******************************************************************************

void SetDuty(word MotorID, longword Duty)
{

	switch(MotorID)
	{
		case 0:
		{
			Duty1InUS = Duty;
			break;
		}
		case 1:
		{
			Duty2InUS = Duty;
			break;
		}

	}

	// Update the selected duty cycle here

  	SetPWMCycles(PeriodInUS, Duty1InUS, Duty2InUS, Duty3InUS);


#ifdef NEW_CODE

	unsigned char updatedisable;

	PORTDbits.RD0 = 1;		// Turn off LED indicating entry into SPI ISR
	// Configure PWM to generate square wave of 50% duty cycle 
	dutycyclereg = 1; // Should this be MotorID ????
	dutycycle = Duty;
	updatedisable = 0;
	SetDCMCPWM(dutycyclereg,dutycycle,updatedisable);
	
	sptime = 0x0;

	// PTCON1 Register configuration 
	config1 = (PWM_EN & PWM_IDLE_STOP & PWM_OP_SCALE16
				& PWM_IPCLK_SCALE16 &
				  	PWM_MOD_UPDN);

	// Configure PWM1
	//config2 = (PWM_MOD1_COMP & PWM_PDIS3H &
	//				PWM_PDIS2H & PWM_PEN1H &  
	//				PWM_PDIS3L & PWM_PDIS2L & PWM_PEN1L);

	config2 = (PWM_MOD1_COMP & PWM_PEN3H &
					PWM_PEN2H & PWM_PEN1H &  
					PWM_PEN3L & PWM_PEN2L & PWM_PEN1L);

	// Configure PWM2
	config3 = (PWM_SEVOPS1 & PWM_OSYNC_PWM & PWM_UEN);

	// Open PWM1 and PWM2 for selected period

	OpenMCPWM(period,sptime,config1,config2,config3);

	//OpenMCPWM(MotorPeriod,sptime,config1,config2,config3);
	
       
/*
	//open timer
	//OpenTimer1(TIMER_INT_OFF&T1_SOURCE_INT&T1_T2_8BIT);
	OpenTimer1(TIMER_INT_OFF&T1_SOURCE_INT&T1_8BIT_RW);
	//OpenTimer3(TIMER_INT_OFF&T3_SOURCE_INT&T3_8BIT_RW);
	
*/
 
	// Configure PWM to generate square wave of 50% duty cycle 
 
	updatedisable = 0;
	SetDCMCPWM(MotorID, Duty, updatedisable);

#endif

}

//******************************************************************************
//* SendmotorCommand - Send the latest PWM DC motor Velocity range (0000..FFFF) 
//* and direction range (CW, CCW) to the selected motor device, range (00..01).
//******************************************************************************

void SendMotorCommand(word MotorID, word Velocity, Direction_T Direction)
{
  
   
}

//******************************************************************************
//* SetDirection - Set the motor's direction (CW, CCW)
//******************************************************************************

void SetDirection(word MotorID, word Direction)
{
}

//******************************************************************************
//* StopMotor - Slowly de-accelarate motor to a stop or neutral speed.                   
//******************************************************************************
void StopMotor(word MotorID)
{
}

//******************************************************************************
//* StartMotor - Slowly accelarate motor to its set Velocity and direction
//******************************************************************************
                   
void StartMotor(word MotorID)
{
}    
                           
//****************************************************************************************
//* Shutdown - Emergency shut down used to cut power to all motors, relays and other 
//* electrical systems.  The Shutdown function is called when the emergency button is pressed.
//****************************************************************************************
 
void Shutdown(void)
{
}

//****************************************************************************************
//****************************************************************************************
//****************************************************************************************




//**********************************************************************************
//* ProcessCalibrateMotorCommand - Process the DC Motor Controller Calibrate Command  
//* message.  Return a status word indicating the outcome of the calibration process.
//**********************************************************************************
word ProcessCalibrateMotorCommand(word TheMotorID)  
{
 	word CalibrationStatus = TRUE;										// Status return
 	
 	// Start DC Motor Calibration process ...

#ifdef LED_DISPLAY

	// Display "11" bit pattern indicating Motor Calibration processing 
	VR_Port_Data.Port_Value = (byte) CALIBRATION_DISPLAY;
 	Write_Virtual_Port(PORT_E, VR_Port_Data);
 	pause(2000);
	VR_Port_Data.Port_Value = 0;
 	Write_Virtual_Port(PORT_E, VR_Port_Data);

#endif

 	// Calibrate motor speed by getting the latest Optical Encoder counts the selected motor
    // and displaying it.  This command can be used to vary the motor speed and direction 
    // during calibration by comparing to the actual speed and direction...
  					
 
	// Read the optical encoder counts here...
  				
  	// Get current motor position information (encoder counts and direction) and send it to the Host.
 
    GetMotorPosition(TheMotorID);
	
	// Calibrate the DC Motors here ...
	
 	// *************** Missing logic to calibrate the DC Motors *********************

    // ***************************   missing logic    **********************************

    // Send Acknowledgement (ACK) back to the Host PC or Stamp, indicating that the 
    // Calibration command was succesfully carried out...
	
 	
#ifdef SERIAL_DEBUG
    printf("Calibration Command:");
    printf("  Motor ID = ");
    dec(TheMotorID);
   	printf("\r\n\r\n");
#endif
				
	return CalibrationStatus;
}

//**********************************************************************************
//* ProcessDutyCommand - Process the Duty Command message used to set the PWM Duty.  
//* Return a status word indicating if the Duty Command was succesfully applied to 
//* each of the selected motors.
//**********************************************************************************
word ProcessDutyCommand(longword *TheDuty, word NumberOfMotors)
{
 	word DutyCommandStatus = TRUE;										// Status return
   	word TheMotorID;													// Local Loop Index

#ifdef LED_DISPLAY
	// Display "111" bit pattern indicating Duty Command processing    
	VR_Port_Data.Port_Value = DUTY_DISPLAY;
 	Write_Virtual_Port(PORT_E, VR_Port_Data);
 	pause(2000);
	VR_Port_Data.Port_Value = 0;
 	Write_Virtual_Port(PORT_E, VR_Port_Data);
#endif
   				
  	// Send the new duty to each of the selected motors

#ifdef SERIAL_DEBUG
    	Printf_2("Duty Command: \r\n");
#endif

	for (TheMotorID = 0; TheMotorID < NumberOfMotors; TheMotorID++)
	{
		// Read the encoder results from previous motor command
		MotorMessage.DutyCommand.EncoderCounts[TheMotorID] = ReadQEI();		
		SetDuty(TheMotorID, TheDuty[TheMotorID]);

		//pause(1); 
        //DelayUs(10);
		
#ifdef SERIAL_DEBUG
  		sprintf(Line, "Received:  Motor ID = %d, TheDuty = %0lX", TheMotorID, TheDuty[TheMotorID]);  
        Printf_2(Line);
    	Printf_2("\r\n");
#endif
	}

	//pause(1000);

	//PORTDbits.RD0 = 0;		// Turn on LED indicating entry into Process Duty Command

	return DutyCommandStatus;
}
			
//**********************************************************************************
//* ProcessGetPotCommand - Process the Get Pot command message that will be used for
//* setting the duty command. Return a status word indicating if the Get Pot Command
//* was succesfully applied to the selected potentiometer.
//**********************************************************************************
word ProcessGetPotCommand(word TheMotorID)
{
 	word GetPotCommandStatus = TRUE;										// Status return
 	
 	// Read the Pot control duty value and send it to the corresponding DC motor 
#ifdef NEW_CODE 					
 	// Read the latest PWM motor command range (0000..FFFF) from the Potentiometer, send it to the H-Bridge
	// and display it. This command can be used to vary the motor speed and direction during calibration.        
	// It remains in effect until the Hold command is issued or the system is reset.
					
	UpdateFlag[TheMotorID] = TRUE; 
#endif	   		
    // Send Acknowledgement (ACK) back to the Host PC or Stamp, indicating that the 
    // Get command was received...
 
#ifdef SERIAL_DEBUG
    printf("Get Pot Command:");
   	printf("  Motor ID = ");
   	dec(TheMotorID);
    printf("\r\n\r\n");
#endif

	return GetPotCommandStatus;
}

//**********************************************************************************
//* ProcessHoldPotCommand - Process the Hold Pot Command message.  Save the
//* current Pot value that will be used for setting the duty command.  Return a status word 
//* indicating if the Hold Pot Command was succesfully applied to the selected 
//* potentiometer.
//**********************************************************************************
word ProcessHoldPotCommand(word TheMotorID)
{
 	word HoldPotCommandStatus = TRUE;										// Status return
			
 	// Hold the Pot control duty value and send it to the corresponding DC motor.  
					
	// Hold the latest PWM motor command range (0000..FFFF) from the Potentiometer and display it. 
	// This command can be used to vary the motor speed and direction.        
#ifdef NEW_CODE
	// Save the sensor value from selected A/D channel (Motor 0 = AN0, Motor 1 = AN1)
			
	MotorCommandSaved[TheMotorID] = MotorCommand[TheMotorID];
	
	// Clear the update flag for the selected motor so that no new pot readings are read
									
	UpdateFlag[MotorID] = FALSE;    
#endif

    // Send Acknowledgement (ACK) back to the Host PC or Stamp, indicating that the 
    // Hold command was received...
 	 
#ifdef SERIAL_DEBUG
    printf("Hold Pot Command:");
   	printf("  Motor ID = ");
   	dec(TheMotorID);
/*
    printf("  Hold Motor Command  = ");
    hex4(MotorCommandSaved[MotorID]);
*/
    printf("\r\n\r\n");

#endif 

	return HoldPotCommandStatus;
}
 					
//**********************************************************************************
//* ProcessNumberSamplesCommand - Process Get number of samples command message. 
//* Return a status word indicating that the Number of Samples was accepted.
//**********************************************************************************
word ProcessNumberSamplesCommand(word TheNumberOfSamples)
{
 	word NumberSamplesCommandStatus = TRUE;									// Status return

#ifdef LED_DISPLAY

  	// Display "111111" bit pattern indicating Number of Samples Command processing    
	VR_Port_Data.Port_Value = NUMBER_OF_SAMPLES_DISPLAY;
 	Write_Virtual_Port(PORT_E, VR_Port_Data);
 	pause(2000);
	VR_Port_Data.Port_Value = 0;
 	Write_Virtual_Port(PORT_E, VR_Port_Data);
#endif

    // Accept number of samples xxxx to be used for calibration and data
   	// collection used for reading the DC Motor Control Pots, where the 
   	// number of samples range 0000..FFFF.
	
  	NumberOfSamples = TheNumberOfSamples;
  	
#ifdef SERIAL_DEBUG
    Printf_2("Number of Samples Command:");
    sprintf(Line, "Received:  Number of Samples = %0X\r\n", TheNumberOfSamples);  
	Printf_2(Line);
    Printf_2("\r\n\r\n");
	
#endif 
	return NumberSamplesCommandStatus;
}

//**********************************************************************************
//* ProcessPeriodCommand - Process the Period Command message used to set the PWM 
//* period. Return a status word indicating if the Period Command was succesfully
//* applied to the selected motor.
//**********************************************************************************
word ProcessPeriodCommand(longword ThePeriod)
{
 	word PeriodCommandStatus = TRUE;									// Status return
    word TheMotorID;

#ifdef LED_DISPLAY
 	// Display "1111" bit pattern indicating Period Command processing    
	VR_Port_Data.Port_Value = PERIOD_DISPLAY;
 	Write_Virtual_Port(PORT_E, VR_Port_Data);
 	pause(2000);
	VR_Port_Data.Port_Value = 0;
 	Write_Virtual_Port(PORT_E, VR_Port_Data);
#endif
 	
 	// Send PWM period command range xx (00..FF) to motor mm range (00..01).   This command
	// can be used to vary the update frequency.
   
	// Set the new PWM period for the selected motor
	SetPeriod(ThePeriod);

    // Send Acknowledgement (ACK) back to the Host PC or Stamp, indicating that the 
    // Period command was received...
	 
#ifdef SERIAL_DEBUG
    sprintf(Line, "Received: Period = %0lX\r\n", ThePeriod);  
    Printf_2(Line);
    Printf_2("\r\n\r\n");
#endif

	return PeriodCommandStatus;
}

//**********************************************************************************
//* ProcessGetStatusCommand - Process the the SPI Slave Controller Get Status 
//* command and return the latest command execution status. 
//**********************************************************************************
void ProcessGetStatusCommand(void)
{

#ifdef LED_DISPLAY
 	// Display "11111" bit pattern indicating Get Status Command processing    
	VR_Port_Data.Port_Value = GET_STATUS_DISPLAY;
 	Write_Virtual_Port(PORT_E, VR_Port_Data);
 	pause(2000);
	VR_Port_Data.Port_Value = 0;
 	Write_Virtual_Port(PORT_E, VR_Port_Data);
#endif
 
/*
    MotorMessage.GetStatusCommand.ControllerState = ControllerState; 		 			// Store current Controller State
    MotorMessage.GetStatusCommand.ControllerErrorCode = ControllerErrorCode; 			// Store current Controller Error Code
    MotorMessage.GetStatusCommand.ControllerDiagnosticCode = ControllerDiagnosticCode;  // Store current Diagnostic Code
*/

    MotorMessage.GetStatusCommand.ControllerState = 0x9999; 		 			// Store current Controller State
    MotorMessage.GetStatusCommand.ControllerErrorCode = 0x8888; 			// Store current Controller Error Code
    MotorMessage.GetStatusCommand.ControllerDiagnosticCode = 0x7777;  // Store current Diagnostic Code
 
#ifdef SERIAL_DEBUG
     
    Printf_2("Slave: Received Get Status Command:");
  	sprintf(Line, "  MessageID = %0X, ControllerState = %0X,  ControllerErrorCode = %0X,  ControllerDiagnosticCode = %0X\r\n", 
             MotorMessage.GetStatusCommand.MessageID, ControllerState, ControllerErrorCode, ControllerDiagnosticCode);
    Printf_2(Line);
    Printf_2("\r\n");
#endif

}
 			

//**********************************************************************************
//* ProcessResetCommand - Process the Reset the SPI Slave Controller.  This command 
//* resets the selected controller.  It may be used to implement an emergency stop 
//* function using the PIC's watchdog timer. 
//**********************************************************************************
void ProcessResetCommand(word TheMotorID)
{
int i;

#ifdef LED_DISPLAY
 	// Display "1111111" bit pattern indicating Reset Command processing    
	VR_Port_Data.Port_Value = RESET_DISPLAY;
 	Write_Virtual_Port(PORT_E, VR_Port_Data);
 	pause(2000);
	VR_Port_Data.Port_Value = 0;
 	Write_Virtual_Port(PORT_E, VR_Port_Data);
#endif


#ifndef TEST
    Printf_2("Reset Command: ");
    Printf_2("  Motor ID = ");
    Dec_2(TheMotorID);
    Printf_2("\r\n\r\n");
#endif

	// Reset the Controller by allowing the Watchdog Timer to timeout.
	
	//while (1);
}
#ifdef VIRTUAL_PORT_TEST


//******************************************************************************
//* Test virtual port E and virtual port B
//****************************************************************************** 
   
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

/*
#ifdef LED_DISPLAY
	//********************************************************************************************
	// Initialize Virtual Port Utility variables
	//********************************************************************************************

	//for (i=0; i<256; i++)
	for (i=0; i<15; i++)
	{
		VR_Port_Data.Port_Value = i;
	 	Write_Virtual_Port(PORT_E, VR_Port_Data);
		pause(250);
	}
 
	// Initialize LED Display for "rolling bits" when everything is working OK 

	VR_Port_Data.Port_Value = 0xAA;
#endif
*/
 
#ifdef NEW_CODE


/*
 
	// Initialize the UART 1 for serial I/O
	InitializeUart1();

	printf("This is a test...\r\n");
	for(i=0; i<100; i++)
	{
		printf("i = %d \r\n", i);
	}

	// Initialize the Quadrature Encoder Interface (QEI)
	init_qei(); 

	//LED_1 = OFF;
	//LED_1_DIR = OUTPUT;

	// QEI Count test (CW or CCW)  

	pos_value = 0;
    QEI_Interrupt_Count = 0;

	while(1)
	{
		CycleCount++;
 		LED_1 = OFF; //Turn off LED if Backward direction
		if ((CycleCount % 100) == 0)
		{
 			pos_value = ReadQEI();
			Printf("Encoder Count = ");  
        	Dec(pos_value);
			Printf("  QEI Interrupt Count = ");  
        	Dec(QEI_Interrupt_Count);
			// Check the UPDN bit for motor direction
			if(QEICONbits.UPDN==1) 
			{ 
				Printf(" Forward.\r\n");  
 				//LED_1 = ON; //Turn on LED if Forward direction
			} 
			else 
			{ 
				Printf(" Backward.\r\n");  
 				//LED_1 = OFF; //Turn off LED if Backward direction
			}
		} 
	}

	// QEI Count test (CW or CCW)  Works!!!
	Printf("QEI Count Test...\r\n");

	pos_value = 0;
	while(1)
	{
		pos_value = ReadQEI();
		//if(pos_value >= 0x7FFF)
		//{
			PORTDbits.RD1 = 0; // turn on LED on RD1 
			Printf("Encoder Count = ");  
            Dec(pos_value);
            Printf("\r\n");
		//}
	}

	// QEI Direction test (CW or CCW)  Works!!!
	while(1) 
	{ 
		// Check the UPDN bit for motor direction
		if(QEICONbits.UPDN==1) 
		{ 
			Printf("Forward...\r\n");  
 			LED_1 = ON; //Turn on LED if Forward direction
		} 
		else 
		{ 
			Printf("Backward...");  
 			LED_1 = OFF; //Turn off LED if Backward direction
		} 
	} 

	Printf("QEI Count Test...\r\n");
	for (i=0; i<1000; i++)
	{
		Printf("This is a test");
		Dec(i);
		Printf("times!\r\n");
		
	}

	CloseQEI();
*/

// Example of using PWM functions

//DS51456A-page 180 Advance Information ? 2004 Microchip Technology Inc.

#define __dsPIC30F6010__
#include <p30fxxxx.h>
#include<pwm.h>
void __attribute__((__interrupt__)) _PWMInterrupt(void)
{
	IFS2bits.PWMIF = 0;
}

int main()
{
	/* Holds the PWM interrupt configuration value*/
	unsigned int config;
	/* Holds the value to be loaded into dutycycle register */
	unsigned int period;
	/* Holds the value to be loaded into special event compare register */
	unsigned int sptime;
	/* Holds PWM configuration value */
	unsigned int config1;
	/* Holds the value be loaded into PWMCON1 register */
	unsigned int config2;
	/* Holds the value to configure the special event trigger
	postscale and dutycycle */
	unsigned int config3;
	/* The value of ‘dutycyclereg’ determines the duty cycle
	register(PDCx) to be written */
	unsigned int dutycyclereg;
	unsigned int dutycycle;
	unsigned char updatedisable;
	/* Configure pwm interrupt enable/disable and set interrupt
	priorties */
	config = (PWM_INT_EN & PWM_FLTA_DIS_INT & PWM_INT_PR1
				& PWM_FLTA_INT_PR0
				& PWM_FLTB_DIS_INT & PWM_FLTB_INT_PR0);
	ConfigIntMCPWM( config );

	/* Configure PWM to generate square wave of 50% duty cycle */
	dutycyclereg = 1;
	dutycycle = 0x3FFF;
	updatedisable = 0;
	SetDCMCPWM(dutycyclereg,dutycycle,updatedisable);
	period = 0x7fff;
	sptime = 0x0;
	config1 = (PWM_EN & PWM_PTSIDL_DIS & PWM_OP_SCALE16
				& PWM_IPCLK_SCALE16 &
				  	PWM_MOD_UPDN);

	config2 = (PWM_MOD1_COMP & PWM_PDIS4H & PWM_PDIS3H &
					PWM_PDIS2H & PWM_PEN1H & PWM_PDIS4L &
					PWM_PDIS3L & PWM_PDIS2L & PWM_PEN1L);

	config3 = (PWM_SEVOPS1 & PWM_OSYNC_PWM & PWM_UEN);

	OpenMCPWM(period,sptime,config1,config2,config3);

	while(1);
}
#endif
