//********************************************************************************
//*                                                                              *
//*        Name:  dcmotor.h                                                      *
//* Project Number:  MT2192                                                      *                                                   
//*           Date:  10/15/2007                                                  *                                                  
//*                                                                              *
//*   Description:                                                               * 
//*     This application contains all the data constants and structures          *
//* necessary for the 8-Bit and 16-Bit SPI DC Motor Controller Master and Slave functions that are    *
//* needed to send messages to the DC Motor Controller slave application or to   *
//* act as a Client to any SPI Slave devices.  These include devices include     *
//* my SPI Math Co-processor, the Microchip's 24XCXX SPI Memory chips, SPI based * 
//* digital potentiometers and A/D devices, and SPI Client/Server networking,    *
//* using the SPI hardware module  contained in the DSPIC30F6014 micro-controller.  *
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
  
#ifndef DC_MOTOR_FLAG
#define DC_MOTOR_FLAG
 
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
#include <spi.h>
#include <uart.h>
#include <math.h>
#include <adc12.h>
#include "timeutil.h"
#include "uartutil.h"

//--------------------------------------------------------------------------------------------------------------------------

//#include "network.h"                                   	// SPI Networking header
//#define SPI_BYTE_INTERFACE   						// SPI Byte I/O
//#define SPI_NETWORK_INTERFACE  					// SPI Message Passing I/O (using SPI interrupt)

//#define SPI_BUFFER_INTERFACE  						// SPI Buffered I/O (using SPI interrupt)
#define USART_BUFFER_INTERFACE  					// SPI Buffered I/O (using SPI interrupt)

 #define GEARBOT                                         // Control a GearBot

//#define MASTER_TEST_0                                 // Select the SPI Master Controller test #0
//#define MASTER_TEST_1                                 // Select the SPI Master Controller test #1
//#define MASTER_TEST_2                                 // Select the SPI Master Controller test #2
//#define MASTER_TEST_3                                 // Select the SPI Master Controller test #3
//#define MASTER_TEST_4                      

// Message synchronization 
#define SYNC_BYTE	0xAB							// SPI Sync Byte
#define ACK_BYTE	0xCD							// SPI Acknowledge Byte

#define TX_DATA_OFFSET 0							// Transmit Buffer Offset (word)
#define RX_DATA_OFFSET 1							// Receive Buffer Offset (word)
#define CHECKSUM_OFFSET 33							// Location of message checksum (word)

#ifdef SPI_8
//#define SPI_BUFFER_SIZE 36							// Size in words of SPI Tx and Rx Buffers
#define SPI_BUFFER_SIZE 64							// Size in bytes of SPI Tx and Rx Buffers
#define DATA_BUFFER_SIZE 32 						// Message Data buffer size in words
#define MESSAGE_BUFFER_SIZE 32							// Size  in words of Motor Message data only
#endif

#ifdef SPI_8A
//#define SPI_BUFFER_SIZE 36							// Size in words of SPI Tx and Rx Buffers
#define SPI_BUFFER_SIZE 72							// Size in bytes of SPI Tx and Rx Buffers
#define DATA_BUFFER_SIZE 34 						// Message Data buffer size in words
#define MESSAGE_BUFFER_SIZE 34							// Size  in words of Motor Message data only
#endif
#ifdef SPI_16
#define SPI_BUFFER_SIZE 36							// Size in words of SPI Tx and Rx Buffers
#define DATA_BUFFER_SIZE 34 						// Message Data buffer size in words
#define MESSAGE_BUFFER_SIZE 34							// Size  in words of Motor Message data only
#endif

//const int DataSize = (DATA_BUFFER_SIZE - 2); 	// Adjusted data size



#define NUMBER_CYCLES 	100								// Number of PWM Cycles to be sent to H-Bridge

  
#define MAX_MOTORS 6                                    // Maximum number of DC motors connected to the controller

#define LEFT_MOTOR      0x00                            // Left Motor ID
#define RIGHT_MOTOR     0x01 						 	// Right Motor ID
#define MOTOR_0 0x00                           			// Motor #0 ID
#define MOTOR_1 0x01 									// Motor #1 ID

 
   
#ifdef GARDENBOT

// GardenBot motor constants

#define NEUTRAL       	152                             // Pwm value for 6.0 V out for Neutral
#define FULL_REVERSE  	201 					     	// Pwm value for 8.0 V out for Full Reverse Speed
#define MEDIUM_REVERSE 	0x032E 						   	// Pwm value for ??? V out for Medium Reverse Speed
#define FULL_FORWARD  	103                             // Pwm value for 4.0 V out for Full Forward Speed
#define MEDIUM_FORWARD  0x00A5                          // Pwm value for ??? V out for Medium Forward Speed
#define PAUSE_TIME      250	 							// Pause between PWM commands (Acceleration)

#endif

#ifdef GEARBOT

// GearBot motor constants

#define NEUTRAL       	0x0200                          // Pwm value for for Neutral
#define FULL_REVERSE  	0x03FF 						    // Pwm value for Full Reverse Speed
#define MEDIUM_REVERSE 	0x032E 						    // Pwm value for Medium Reverse Speed

#define FULL_FORWARD  	0x0010                          // Pwm value for Full Forward Speed
#define MEDIUM_FORWARD  0x00A5                          // Pwm value for Medium Forward Speed
#define PAUSE_TIME      100	 					  		// Pause between PWM commands (Acceleration)

#endif  

 // Offsets for Slave SPI Receive Buffer

#define SYNC_INDEX 0								// Sync byte index
#define COMMAND_INDEX 3								// Command code index
#define STATE_INDEX 4								// Controller State index
#define ERROR_CODE_INDEX 5							// Controller Error code index
#define DIAGNOSTIC_CODE_INDEX 6					    // Diagnostic code index


//******************************************************************************
// Define error, diagnostic and status enumerations here.
//******************************************************************************
 
enum CONTROLLER_STATE_TYPE { NO_STATES=0, INITIALIZED=1, DONE=2, BUSY=3, ERROR=4}; 	// Status of selected slave controller, range (0..255) 
													// 		0 => NO_STATES	    No State    
													//		1 => INITIALIZED 	The selected slave contoller is Initialzed 
													//		2 => DONE 	 		The selected slave contoller is done processing current command
													// 		3 => BUSY 	 		The selected slave contoller is busy processing current command 
													// 		4 => ERROR 	 		The selected slave contoller encountered an error processing the current command 

enum CONTROLLER_ERROR_TYPE { NO_ERRORS=0, FAILURE=1, SUCCESS=2 }; 	// Error code returned by the selected slave controller, range (0..255) 
													// 		0 => NO_ERRORS 		No error codes  
													// 		1 => FAILURE 	 	Failed to complete command
													//  	2 => SUCCESS  	 	Completed command successfully 
enum DIAGNOSTIC_MESSAGE_TYPE { NO_DIAGNOSTICS=0, DIAGNOSTIC1=1, DIAGNOSTIC2=2 };
													// 		0 => NO_DIAGNOSTICS No Diagnostic messages
													// 		1 => DIAGNOSTIC 1 	Diagnostic message 1
													//  	2 => DIAGNOSTIC 2  	Diagnostic message 2 

//******************************************************************************
//* Define message IDs for each type of command that is sent by the SPI Master 
//* applications and recognized by the selected SPI Slave applications. Up to 255 
//* different commands can be defined.
//******************************************************************************
typedef enum command_type
{
	//******************************************************************************
	// DC Motor Controller Commands Message
	//******************************************************************************

	CALIBRATE_MOTOR_COMMAND = 0,					// Send calibrate command to the selected, motor
	DUTY_COMMAND = 1, 								// Send duty command to the selected motor
 	GET_POT_COMMAND =2, 							// Send get pot command to the selected motor
	HOLD_POT_COMMAND =3,							// Send hold pot command to the selected motor


	//******************************************************************************
	// Sensor Controller Commands
	//******************************************************************************

	CALIBRATE_SENSOR_COMMAND =4, 					// Calibrate the selected sensor 
	GET_SENSOR_CALIBRATION_COMMAND =5,				// Get sensor calibration data from the Sensor Controller
	SET_SENSOR_WINDOW_COMMAND =6,					// Set the sensor window used to scale data for the Sensor
	GET_RAW_SENSOR_DATA_COMMAND =7, 				// Get raw sensor data from the Sensor Controller
	GET_SCALED_SENSOR_DATA_COMMAND =8,				// Get scaled sensor data from the Sensor Controller
	SELECT_ALGORITHM_COMMAND=9,						// Select the algorithm to be used for processing the sensors

	//******************************************************************************
	// Animation Controller Commands
	//******************************************************************************

	CALIBRATE_JOYSTICK_COMMAND =10, 				// Calibrate the selected joystick
	GET_JOYSTICK_CALIBRATION_COMMAND =11,			// Get joystick calibration data from the Animation Controller
	SET_JOYSTICK_WINBOW_COMMAND =12,				// Set the joystick window used to scale data for the Animation Controller
	GET_RAW_JOYSTICK_DATA_COMMAND =13, 				// Get raw joystick data from the Animation Controller
	GET_SCALED_JOYSTICK_DATA_COMMAND =14,			// Get scaled joystick data from the Animation Controller

	//******************************************************************************
	// Math Co-processor Commands
	//******************************************************************************

	CALCULATE_COMMAND =15,							// Calculate command evaluates RPN expression
	GET_ANSWERS_COMMAND =16,						// Gets the floating point answers
	CLEAR_MEMORY_COMMAND =17,						// Clears the floating point scratch registers
	STORE_COMMAND =18,								// Stores a floating point value to a selected float register
	RECALL_COMMAND =19,								// Retrieves a floating point value from a selected float register	
	FILTER_COMMAND =20, 							// Performs selected filter operation (FIR, IFR)
	FFT_COMMAND =21, 								// Performs an FFT operation
	IFFT_COMMAND =22, 								// Performs an inverse FFT operation
	KALMAN_FILTER_COMMAND =23, 						// Performs a Kalman filter operation
	MEDIAN_FILTER_COMMAND =24, 						// Performs a Median filter operation
	HISTOGRAM_COMMAND =25, 							// Performs a Histogram operation
	SORT_COMMAND =26, 								// Performs a Sort operation
	
	//******************************************************************************
	// Commonly used (Overloaded) Commands
	//******************************************************************************

	RECORD_COMMAND =242,                            // Record sensor data or animation script
	PLAY_COMMAND =243,								// Play back sensor data or animation script
	NUMBER_SAMPLES_COMMAND =244,					// Send number of samples to the selected controller
	PERIOD_COMMAND =245,							// Send period to the selected controller
 	DELAY_COMMAND =246, 							// Send delay to the selected controller 
 	READ_PORT_COMMAND =247, 						// Read from a specified port address on the slave 
 	WRITE_PORT_COMMAND =248, 						// Write to a specified port address on the slave 
 	GET_INTERRUPT_COUNTS_COMMAND =249, 				// Get latest interrupt counts from the slave
 	SELECT_FILTER_COMMAND =250, 					// Specify a filter
 	TEST_COMMAND =251,								// Test a specific function
  	SEND_TELEMETRY_COMMAND =252, 					// Send Telemetry using specified formats
	GET_STATUS_COMMAND =253,						// Get the status of the selected slave controller
  	RESET_COMMAND =254,								// Resets the selected slave controller

	END_OF_MESSAGES =255 							// End of command messages



} COMMAND_TYPE;

/******************************************************************************
 * Define Message Type used to hold the unformated command messages 
 ******************************************************************************/
typedef struct message_type
{   
	word Field_1;					// Start of message field                                         +1
  	word Data[DATA_BUFFER_SIZE];    					// Byte buffer used for to hold a message     +34
 	word Field_2;                                                                                //   +1
} MESSAGE_TYPE;                                                                                  //   ----
                                                                                                 //   36 Words
/******************************************************************************
 * Define Generic Command Message Type to be used to hold the selected 
 * command for processing by the SPI SLave Controller. 
 ******************************************************************************/

typedef struct command_message_type
{
 
	word Field_1;					// Start of message field                                         +1

   	word MessageID;    				// Message ID range (0000 .. FFFF) Hex                            +1

  	word Command;         			// Generic command to SPI Slave                                   +1

	word CommandStatus;				// Command status                                                 +1

    word Filler[30];				// Padding (total of 34 words)                                    +30
                                                                                             //         ----
                                                                                             //       34 Words

 
} COMMAND_MESSAGE_TYPE;

//**********************************************************************************
// Define Global variables common to all controllers here
//**********************************************************************************

extern byte ControllerState; 		// State of selected slave controller, range (0..255) 
											// 		0 => NO_STATES		No States    
											//		1 => INITIALIZED	The selected slave contoller is Initialzed 
											//		2 => DONE 	 	 	The selected slave contoller is done processing current command
											// 		3 => BUSY 	 		The selected slave contoller is busy processing current command 
											// 		4 => ERROR 	 		The selected slave contoller encountered an error processing the current command 

extern byte ControllerErrorCode; 	// Error code returned by the selected slave controller, range (0..255) 
											// 		0 => NO_ERRORS 		No error codes  
											// 		1 => FAILURE 	 	Failed to complete command
											//  	2 => SUCCESS  	 	Completed command successfully 
 
extern byte ControllerDiagnosticCode; 	// Diagnostic message code returned by the selected slave controller, range (0..255) 
											// 		0 => NO_DIAGNOSTICS 	No Diagnostic messages
											//  	1 => DIAGNOSTIC 1  	Diagnostic message 2 
											// 		2 => DIAGNOSTIC 2 	Diagnostic message 1
 
//**********************************************************************************
// Define message commands common to all controllers.  Thes messages are sent by 
// the SPI Master and  recognized by the selected SPI Slave.
//**********************************************************************************

//**********************************************************************************
//* GetStatus - Get the latest status from the selected Slave controller.  The     *
//* message includes the controller state, any errors detected and any diagnostic  * 
//* messages reported.                                                             *
//**********************************************************************************
typedef struct get_status_command_type
{
 
	word Field_1;					// Start of message field

  	word MessageID;    				// Message ID range (0000 .. FFFF) Hex 

  	word Command;         			// Send the get status command to selected slave controller

  	word ControllerState; 			// State of selected slave controller, range (0..255) 
											// 		0 => NO_STATES		No States    
											//		1 => INITIALIZED	The selected slave contoller is Initialzed 
											//		2 => DONE 	 	 	The selected slave contoller is done processing current command
											// 		3 => BUSY 	 		The selected slave contoller is busy processing current command 
											// 		4 => ERROR 	 		The selected slave contoller encountered an error processing the current command 

	word ControllerErrorCode; 		// Error code returned by the selected slave controller, range (0..255) 
											// 		0 => NO_ERRORS 		No error codes  
											// 		1 => FAILURE 	 	Failed to complete command
											//  	2 => SUCCESS  	 	Completed command successfully 
  

	word ControllerDiagnosticCode; 	// Diagnostic message code returned by the selected slave controller, range (0..255) 
											// 		0 => NO_DIAGNOSTICS 	No Diagnostic messages
											//  	1 => DIAGNOSTIC 1  	Diagnostic message 2 
											// 		2 => DIAGNOSTIC 2 	Diagnostic message 1

	word GetStatusCommandStatus;	// GetStatus command status

    word Filler[27];				// Padding (total of 34 words)

 
} GET_STATUS_COMMAND_TYPE;

//**********************************************************************************
//*                                                                                *
//*     Nxxxx     - Accept total number of samples message.  Allows up to XXXX     *
//*                 samples to be read from selected Potentiometer, where xxxx is  *
//*                 in 4 digit hex byte format  range (00..FFFF).                  *
//**********************************************************************************
typedef struct sample_command_type
{
 
	word Field_1;					// Start of message field

  	word MessageID;    				// Message ID range (0000 .. FFFF) Hex

  	word Command;         			// Get new motor commands from the current POT readings
  	                                // Command (see enumeration declaration above.)  
 
 	word NumberOfSamples;			// Number of samples 

	word SampleCommandStatus;		// Sample command status

    word Filler[29];				// Padding (total of 34 words)

    
} SAMPLE_COMMAND_TYPE;

//**********************************************************************************
//*                                                                                *
//* 	Pmm,xx    - Send PWM period command range xxxxxxxx (00000000..FFFFFFFF) to *
//*                 motor mm range (00..01). This command is used to vary the      *
//*                 PWM update frequency.                                          *
//**********************************************************************************
typedef struct period_command_type
{
 
	word Field_1;					// Start of message field                                            +1

  	word MessageID;    				// Message ID range (0000 .. FFFF) Hex                               +1

  	word Command;         			// Send period command to selected motor (see enumeration            +1
                                	// declaration above.)  
 
    longword Period ; 	            // Period range xxxxxxxx	(00000000..FFFFFFFF)                     +2

	word PeriodCommandStatus;		// Period command status                                             +1

    word Filler[17];				// Padding (total of 34 words)                                      +28
                                                                                                   //   ----
                                                                                                   //    34                                                                         

} PERIOD_COMMAND_TYPE;

//**********************************************************************************
//* 	Z         - Reset the SPI Slave Controller.  This command resets the       *
//*                 selected controller.  It may be used to implement an emergency *
//*                 stop function using the PIC's watchdog timer                   *                                                      
//**********************************************************************************
typedef struct reset_command_type
{
 
	word Field_1;					// Start of message field

  	word MessageID;    				// Message ID range (0000 .. FFFF) Hex 

  	word Command;         			// Send period command to selected motor (see enumeration
                                	// declaration above.)  
  	word MotorID; 					// Selected motor range (0..1)  

	word ResetCommandStatus;		// Reset command status

    word Filler[29];				// Padding (total of 34 words)

   
} RESET_COMMAND_TYPE;



//--------------------------------------------------------------------------------------------------------------------------
 

// motor enumeration types

typedef enum   {CW, CCW} Direction_T;                   // Motor Direction (CW, CCW)
 

//**********************************************************************************
// Define messages for each type of command that is sent by the SPI Master and 
// recognized by the selected SPI Slave.
//**********************************************************************************

//**********************************************************************************
//* 	Cmm       - Calibrate motor speed message for motor mm range (00..01). This*
//*                 command is used to vary the motor speed and direction using    *
//*                 two potentiometers with the blades connected to AN0 and AN1.   *
//*                 The true motor speed (RPM) is determined from the counts       *
//*                 received from the optical encoders (if used) and displayed.    *
//**********************************************************************************
typedef struct calibrate_motor_type
{
 
	word Field_1;					// Start of message field

  	word MessageID;    				// Message ID (0000 - FFFF) Hex

  	word Command;         			// Calibrate Command to SPI Slave (see enumeration
                                	// declaration above.)  
 
 	word MotorID; 					// Selected motor range (0..MAX_MOTORS)  

	word CalibrateCommandStatus;	// Calibrate command status

    word Filler[29];				// Padding (total of 34 words)

      
} CALIBRATE_MOTOR_TYPE;

//**********************************************************************************
//* 	Dmm,xxxx  - Send PWM duty command message range xxxx (0000..FFFF) to motor *
//*                 mm range (00..01). This command is used to vary                *
//*                 the motor speed and direction.                                 *
//**********************************************************************************
typedef struct duty_command_type
{
 
	word Field_1;					// Start of message field                                               +1

  	word MessageID;    				// Message ID range (0000 - FFFF) Hex                                   +1

  	word Command;         			// Duty Command to SPI Slave (see enumeration                           +1
                                	// declaration above.)  
 
 	word NumberOfMotors; 			// Total number of motors range (0..MAX_MOTORS)                         +1
  
 	//word Duty[MAX_MOTORS]; 			// Duty Cycle range (0000 - FFFF) Hex                               
	longword Duty[MAX_MOTORS]; 			// Duty Cycle range (00000000 - FFFFFFFF) Hex                      +12

    //word EncoderCounts[MAX_MOTORS];	// Encoder counts for each motor range ((0000 - FFFF) Hex            
    longword EncoderCounts[MAX_MOTORS];	// Encoder counts for each motor range (00000000 - FFFFFFFF) Hex   +12  
  
    word DirectionFlags;			// Direction bits for each motor (CW=0, CCW=1)                          +1

    word StatusFlags;				// Status bits                                                          +1

    word Filler[4];			    	// Padding (total of 34 words)                                          +4
                                                                                                 //         -----
                                                                                                 //         34 Words
 
} DUTY_COMMAND_TYPE;

//**********************************************************************************
//* 	Gmm       - Get new motor commands message.  Used to get the current POT   *
//*                 readings and display them for the specific motor mm range      *
//*                 (00..01).  This mode will remain in effect until the Hold      *
//*                 command is issued.                                             *
//**********************************************************************************
typedef struct get_pot_command_type
{
 
	word Field_1;					// Start of message field

  	word MessageID;    				// Message ID (0000 - FFFF) Hex

  	word Command;         			// Get new motor commands from the current POT readings
  	                                // Command (see enumeration declaration above.)  
 
 	word MotorID; 					// Selected motor range (0..MAX_MOTORS)  

	word GetPotCommandStatus;		// Get Pot command status

    word Filler[29];				// Padding (total of 34 words)

    
} GET_POT_COMMAND_TYPE;

//**********************************************************************************
//*                                                                                *
//* 	Hmm       - Hold current POT reading message. It is used for the next PWM  *
//*                 duty command range (0000..FFFF) to motor                       *
//*                 mm range (00..01). This command is used to vary                * 
//*                 the motor speed and direction.                                 *
//**********************************************************************************
typedef struct hold_pot_type
{
 
	word Field_1;					// Start of message field

  	word MessageID;    				// Message ID (0000 - FFFF) Hex

  	word Command;         			// Get new motor commands from the current POT readings
  	                                // Command (see enumeration declaration above.)  
  
 	word MotorID; 					// Selected motor range (0..MAX_MOTORS)  

	word HoldPotCommandStatus;		// Hold Pot command status

    word Filler[29];				// Padding (total of 34 words)

     
} HOLD_POT_TYPE;


/******************************************************************************
 * Define union of unformatted message and command message structures in order to 
 * overlay the memory. 
 ******************************************************************************/

typedef union message_union_type
{
    CALIBRATE_MOTOR_TYPE CalibrateCommand;						// Calibrate motor command message
	DUTY_COMMAND_TYPE DutyCommand;								// Duty command message
	GET_POT_COMMAND_TYPE GetPotCommand;							// Get Pot command message
	HOLD_POT_TYPE HoldPotCommand;								// Hold pot command message
	SAMPLE_COMMAND_TYPE SampleCommand;                          // Sample command message	
	PERIOD_COMMAND_TYPE PeriodCommand;					 		// Period command message
	GET_STATUS_COMMAND_TYPE GetStatusCommand;					// Get status message
	RESET_COMMAND_TYPE ResetCommand;					 		// Reset command message

	COMMAND_MESSAGE_TYPE SelectedCommand;						// Selected command	
	MESSAGE_TYPE Message;										// Unformatted message
} MESSAGE_UNION_TYPE;

//--------------------------------------------------------------------------------------------------------------------------




//********************************************************************************
// Master DC Motor Function Prototypes
//********************************************************************************

byte FlushSPIBuffers(void);									// Clear the SPI RX and TX buffers

// Functions used to send commands to the slave DC Motor Controller

byte SendCalibrateMotorCommand(byte TheMotorID);  
byte SendDutyCommand(longword *TheDuty, byte NumberOfMotors);
byte SendGetPotCommand(byte TheMotorID);
byte SendHoldPotCommand(byte TheMotorID);
byte SendNumberSamplesCommand(byte TheMotorID, word TheNumberOfSamples);
byte SendPeriodCommand(longword ThePeriod);
byte SendGetStatusCommand(void);
byte SendResetCommand(void);

// Functions used to process responses from the slave DC Motor Controller

byte ReceiveCalibrateMotorResponse(void);  
byte ReceiveDutyResponse(void);
byte ReceiveGetPotResponse(void);
byte ReceiveHoldPotResponse(void);
byte ReceiveNumberSamplesResponse(void);
byte ReceivePeriodResponse(void);
byte ReceiveGetStatusResponse(void);
byte ReceiveResetResponse(void);

//**********************************************************************************
//* SendSPIMessage - Sends a command message to the SPI Slave and stores the data received 
//* into the SPI_Rx_Buffer so that it may be processed by Reveive Message. 
//**********************************************************************************
byte SendSPIMessage(void);

//**********************************************************************************
//* ReceiveSPIMessage - Receives and processes the message that is in the SPI_Rx_Buffer 
//* which was simultaneously sent back from the SPI Slave Controller.  The message may
//* contain data or status information.
//**********************************************************************************
byte ReceiveSPIMessage(void);

// High Level Master DC Motor Controller functions

void Neutral(void);									// Motor is in Neutral or Stopped
void Neutral_To_Forward(void);						// Accelerate from Neutral to maximum Forward  Speed
void Forward_To_Neutral(void);						// De-accelerate from maximum Forward speed to Neutral
void Neutral_To_Reverse(void);						// Accelerate from Neutral to maximum Reverse speed
void Reverse_To_Neutral(void);						// De-accelerate from maximum Reverse speed to Neutral
void Left(void);									// Turn left
void HardLeft(void);								// Turn hard left
void Right(void);									// Turn right
void HardRight(void);								// Turn hard right

 

//********************************************************************************
// Slave DC Motor Function Prototypes
//********************************************************************************
 
// DC Motor Message handling Functions

word ProcessCalibrateMotorCommand(word TheMotorID);
word ProcessDutyCommand(longword *TheDuty, word NumberOfMotors);
word ProcessGetPotCommand(word TheMotorID);
word ProcessHoldPotCommand(word TheMotorID);
word ProcessNumberSamplesCommand(word TheNumberOfSamples);
word ProcessPeriodCommand(longword ThePeriod);
void ProcessResetCommand(word TheMotorID);
void ProcessGetStatusCommand(void);

// PWM DC motor Control Functions

void SetupDCMotors(void);                       		// Initialize DC motor variables
void UpdateDCmotors(void);                           	// Update DC motor positions (ISR)
void ConfigureInterrupts(void);                      	// Configure interrupts
void GetMotorPosition(word MotorID);                 	// Get motor position information (encoder counts and direction)
void GetMotorStatus(word MotorID);                   	// Get current motor status information (temperature, current draw)
void SendmotorCommand(word MotorID, word Velocity, Direction_T Direction);   // Send the latest PWM DC motor Velocity and direction Command
void SetPeriod(longword Period);                    	// Set the PWM period for all three PWM channels
void SetDuty(word MotorID, longword Duty);             	// Set the PWM duty
void SetDirection(word MotorID, word Direction);     	// Set the motor's direction (CW, CCW)
void Shutdown(void);                                 	// Emergency shutdown
void StopMotor(word MotorID);                        	// Slowly de-accelarate motor to a stop
void StartMotor(word MotorID);                       	// Slowly accelarate motor to its set Velocity and direction
// Slowly accelarate motor to its set Velocity and direction

#endif
