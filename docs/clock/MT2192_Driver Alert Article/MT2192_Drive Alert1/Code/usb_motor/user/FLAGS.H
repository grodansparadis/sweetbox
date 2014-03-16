//********************************************************************************
//*                                                                              *
//*                                                                              *
//* Project Number:  MT2192                                                      *                                                   
//*           Date:  10/15/2007                                                  *                                                  
//*                                                                              *
//*   Description:                                                               * 
//*     This file includes the global Flags used by MPSIM.                       *
//*                                                                              *
//*                                                                              *
//********************************************************************************

#ifndef FLAGS_LOADED  /* prevent multiple loading */
#define FLAGS_LOADED

// General flags used by all applications

//#define PIC18C252 												  	 // Select the PIC18C252 micro-controller
//#define PIC18C452														 // Select the PIC18C452 micro-controller
//#define PIC18F452														 // Select the PIC18C452 micro-controller
#define PIC18F4550														 // Select the PIC18F4550 micro-controller
//#define PIC18F6620														 // Select the PIC18F6620 micro-controller
//#define PIC18F8720														 // Select the PIC18F8720 micro-controller

// Initialize compilation flags here

#define SERIAL_DRIVER                                     // Include serial driver      
//#define KEYPAD_DRIVER                                     // Include Keypad driver
//#define LED_DRIVER                                      // Include Max7219 LED Display
//#define LCD_DRIVER                                      // Include LCD Display 
//#define SERIAL_LCD_DISPLAY							  // Select a serial LCD display driver

//#define I2C_DRIVER                                      // Include I2C driver     
//#define SPI_DRIVER                                      // Include SPI driver     

//#define DEBUG                                        	  // Flag used for MPSIM debugging 
                                                          // (must be commented out for actual use)

#define USART_SUPPORT                                     // Support USART on PIC18F4550 
//#define STDIO_INTERRUPTS                               	  // Include interrupt code for stdio functions
//#define FULL_DUPLEX                                     // Full duplex serial I/O support
//#define TIMEOUT_SUPPORT                             	  // Timeout support for serial I/O (USART)
//#define NETWORK_SUPPORT                                 // Support for PC to PIC Networking using the USART


//#define TIMER_INTERRUPTS                           	  // Include timer interrupt code
   
// Select the correct module test

//#define SW_USART_TEST                                   // Software UART unit test
//#define STDIO_TEST                                        // Stdio unit test
//#define TIMEUTIL_TEST							   	      // Timeutil unit test
//#define PORTUTIL_TEST									  // Portutil unit test
//#define KEYPAD_TEST								 	  // Keypad unit test
//#define IRCONTRL_TEST								 	  // Ircontrl unit test
//#define SONYRC_TEST								 	  // Sonyrc unit test
//#define MASTER_SENSOR_TEST							  // Master Sensor Controller unit test
//#define SLAVE_SENSOR_TEST								  // Slave Sensor Controller unit test
//#define PORTBINT_TEST                                   // Test Port B Change on State
//#define DISPLAY_TEST                  				  // Display unit test
//#define JOYSTICK_TEST               					  // Joystick unit test
//#define ANIMATION_TEST               					  // Animation unit test
//#define RELAYDRV_TEST               					  // Relay Driver unit test
//#define X10_TEST										  // X10 unit test
//#define CLOCK_TEST                                      // NJU6355 real-time clock unit test
//#define TIMERW_TEST                                     // X10 Timer unit test 
//#define MAX7219_TEST                                    // MAX7219 LED display unit test
//#define SONAR_TEST                                      // Polaroid 6500 Sonar Ranger unit test
//#define SW_I2C_MASTER_TEST                              // Software I2C Master test 
//#define I2C_MASTER_TEST                                 // I2C Master test 
//#define I2C_SLAVE_TEST                                  // I2C Slave test 
//#define VIDEOCAM_TEST                                   // VideoCam test 
//#define CALCULATOR_TEST								  // Calculator unit test
#define MASTER_COPROCESSOR							  // Master co-processor unit test
#define SLAVE_COPROCESSOR								  // Slave co-processor unit test
//#define LCD_TEST                                        // LCD unit test
//#define DC_MOTOR_MASTER_TEST							  // DC motor Controller unit test
//#define DC_MOTOR_SLAVE_TEST							  // DC motor Controller unit test
//#define SPI_MASTER_TEST                                 // SPI Master test
//#define SPI_SLAVE_TEST                                  // SPI Slave Co-Processor test
//#define SPI_ADC_TEST
//#define SPI_TEST                                        // SPI unit test
//#define SERVO_TEST                                      // DC Servo Motor Test 
//#define RC_SERVO_TEST                                   // RC Servo Test 
//#define PWMUTIL_TEST                                    // PWM Utility test
//#define PICTERM_TEST                                    // PIC Terminal test
//#define SILVER1_TEST                                    // PIC I2C Master Serial EEPROM MSSP test (Serial PIC'n)
//#define ORANGE1_TEST                                    // PIC I2C Master Serial EEPROM test Bit-Banged (Serial PIC'n)
//#define ORANGE4_TEST                                    // PIC I2C Master test Bit-Banged (Serial PIC'n)
//#define ORANGE5_TEST                                    // PIC I2C Master test Bit-Banged (Serial PIC'n)
//#define GARDENBOT_TEST                                  // GardenBot robot application
//#define GEARBOT_TEST                                    // GearBot robot application
//#define MATH_TEST                                      // MATHTEST application
//#define JMATH_TEST                                      // JMATH test application

// Define constants and flags used by stdio.c application

//#define OSC_4_MHZ                                       // 4 MHz system clock
//#define OSC_20_MHZ                                        // 20 MHz system clock           
//#define OSC_40_MHZ										  // 40 MHz system clock
#define OSC_48_MHZ											// 48 MHz system clock
//#define OSC_50_MHZ										  // 50 MHz system clock

//#define BAUD_1200										  // 1200 Baud Asynchronous Serial rate
//#define BAUD_4800										  // 4800 Baud Asynchronous Serial rate
//#define BAUD_9600	                                      // 9600 Baud Asynchronous Serial rate
//#define BAUD_19200                                        // 19200 Baud Asynchronous Serial rate
//#define BAUD_38400                                      // 38.4 K Baud Asynchronous Serial rate
//#define BAUD_57600										  // 56 K Baud Asynchronous Serial rate
#define BAUD_115200										  // 115.2 K Baud Asynchronous Serial rate
	
// Define constants and flags used by the timeutil.c application

#define USE_TIMER1                                        // Use Timer 1 for BS2 pause function
//#define USE_TIMER2                                       // Use Timer 2  
//#define USE_TIMER3                                       // Use Timer 3 for BS2 pause function
#define WATCHDOG_OFF									  // Watchdog timer off
//#define WATCHDOG_ON                                     // Watchdog timer on 


#endif
