 //********************************************************************************
//*                                                                              
//*           Name:  DriverAlert.c                                               
//* Project Number:  MT2192                                                      *                                                   
//*           Date:  10/15/2007                                                  *                                                  
//*                                                                              *
//*   Description:                                                               * 
//* 		This is a Motorola Freescale/Zigbee based Driver Alert System application that auto and truck drivers can
//* can use to help keep them alert while driving.  While Coffee and getting plenty of sleep at night is paramount to 
//* safe-driving, sometimes long boring trips or traveling by car on vacation or commuting everyday to work
//* along with getting hypnotized by the road are all hazzards that vehicle operators face each day on our modern
//* highways.
//* 
//*  The system consists of the vehicle operator wearing a baseball cap glasses 
//* or headband with the XY and Z accelerometer running a data collection application that transmits the
//* position and orientation to the Freescale/Zigbee  micro-controller based Driver Alert Module receiver. 
//*
//* The Driver Alert Module further processes the acceleration data into messages and transmit it via the 
//* serial RS-232 interface at 38400 Baud, 8 Data Bits and 1 Stop bit to an IPAQ Pocket PC or laptop conveniently 
//* mounted on the vehicle dashboard.   
//* Initially the Driver Alert System prototype will provide data collection and primitive algorithms to 
//* detect if the driver is awake or driving erratically and to alert the driver if he/she is falling
//* asleep at the wheel. Sophisticated alertness algorithms could be developed using Neural Networks (NNs) 
//* and Fuzzy Logic algorithms to determine if the driver is alert, but these are beyond my capabilities 
//* and resources at this time.  Instead, I intend to collect the data and perform some simple analysis 
//* using MATLAB to determine if the data collected can used to determine if the vehicle operator is driving normally 
//* or erratically by measuring the position (X,Y,Z) and the orientation (pitch, yaw and roll) of the operator's head
//* with respect to the operator's normal field of view (FOV). In other words the driver should be roughly horizontally
//* using maybe up to 120 degrees (panoramic) and not jerking his/her head or staring down at the floor or the looking up 
//* at the ceiling while driving.  
//*
//* This system could also benefit operators who may have unknown medical conditions such as a weak heart,
//* encounter sudden (first time) fainting spells, or seizures, where their behavior suddenly changes. 
//* It can even warn those who are driving under the influence to pull over and call a taxi or their 
//* loved ones to get them safely home. 
//*
//* Correlation of the data during a normal (safe) commute and an abnormal (simulated by contrived means 
//* in a vehicle at rest) is accomplished using the TRIAX data collected by the reescale/Zigbee  
//* micro-controller. The final determination and selected alert processed with the IPAQ Pocket PC or 
//* laptop using Embedded Visual C++ or Embedded Visual Basic.  In order to obtain the absolute 
//* orientation it may be necessary to include a second  X,Y and Z accelerometer fixed on the vehicle's 
//* dashboard, if it turns out that I can't get this data any other way.

//* So as not to startle the vehicle operator and unintentionally cause an accident, the audible alerts
//* will consist of gradually playing recordings made by the operator's loved ones urging them to pull 
//* over or to get a cup of coffee, or using a recording of their pet dog barking. The alerts will be 
//* issued whenever the Driver Alert Controller detects abnormal or erratic driving. These alerts are 
//* recorded and played back using the IPAQ Pocket PC and maybe even the vehicle's speakers. Another 
//* alert can be provided by playing a track (*.WAV)  or MP3 from their favorite music (guaranteed to 
//* wake them up).  I myself prefer music from the mercurial Finish group NIGHTWISH which brings me back
//* to Earth it every time.  In the event that the system detects an emergency situation, it could be 
//* connected to the vehicles emergency switch (panic button with a red triangle) so as to warn other 
//* drivers on the road.
//*
//* Disclaimer:  This system represents only a prototype and should be treated as such and not used as a safety device 
//* since it has not had the rigorous development and testing required for an automotive application.  
//* Rather think of it as device that hopefully will be included in the array of vehicle safety devices 
//* such as the Air Bag now common in all new vehicles.  The commercial version would consist of a high
//* end Renesas M16C micro-controller integrating all the functionality of the IPAQ Pocket PC or the 
//* laptop, thus reducing the unit price needed for the highly competitive automotive industry.
//* Future upgrades include advanced features such as adding a realtime clock for data collection and
//* event logging during development.   
//* 
//*********************************************************************************************

  
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
 
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <dsp.h>
#include <uart.h>
#include <spi.h>
#include <i2c.h>
#include <math.h>
#include <adc12.h>

#include "my_types.h"					// General C/C++ types
#include "lcd.h"						// LCD Support using SPI2 
#include "DriverAlertPorts.h"   		// Driver Alert Port bit definitions
#include "timeutil.h"                   // Timer utilities
#include "graphutil.h"                  // Graphics Utilities
#include "uartutil.h"

#define UART_SUPPORT
#define SPI_SUPPORT

//#define LCD_TEST_1
//#define LCD_TEST_2
#define LCD_TEST_3

#define STRING_SIZE 80 					// String size 

 
// Select ADC Channels used by Virtual SARD here...
#define X_INDEX 12						// X axis index 
#define Y_INDEX 10						// Y axis index				
#define Z_INDEX 11                      // Z axis index

// Virtual SARD Calibration Constants derived from Freescale AN1986 SARD Calibration Procedures 
// using 12-Bit ADC values
                            				// Corresponding 8-Bit values
#define X_AXIS_MINUS_1G 	1124			// 70
#define Y_AXIS_MINUS_1G 	1075			// 67
#define Z_AXIS_MINUS_1G 	1605			// 100

#define X_AXIS_0G 			2135			// 133
#define Y_AXIS_0G 			2087			// 130
#define Z_AXIS_0G 			2729			// 170

#define X_AXIS_PLUS_1G 		3163			// 197
#define Y_AXIS_PLUS_1G 		3179	 		// 198
#define Z_AXIS_PLUS_1G 		3854			// 240

// Define Driver Alert Thresholds here...

#define X_AXIS_THRESHOLD 30.0  // 30.0 Degrees (requires more research!)
#define Y_AXIS_THRESHOLD 45.0  // 45.0 Degrees (requires more research!)
#define Z_AXIS_THRESHOLD 35.0  // 35.0 Degrees (requires more research!)

#define X_AXIS_TIME_LIMIT 150.0  // 150 milliseconds (requires more research!)
#define Y_AXIS_TIME_LIMIT 250.0  // 250 milliseconds (requires more research!)
#define Z_AXIS_TIME_LIMIT 350.0  // 350 milliseconds (requires more research!)

// Conversion from 12-Bit values to volts (0..3.3 Volts)

const float ConvertThreeVolts = 3.3 / 4096.0;

// Conversion from 12-Bit values to volts (0..5 Volts)

const float ConvertFiveVolts = 5.0 / 4096.0;

// Conversion from 12-Bit values to (-90..+90) degrees

const float ConvertToDegrees = 180.0 / 4096.0;

// Declare global variables here

 unsigned int Channel, PinConfig, Scanselect, Adcon3_reg, Adcon2_reg,
Adcon1_reg;


//********************************************************************************
// Function Prototypes
//********************************************************************************
void InitAD(void);
unsigned int GetADValue(char pot);

// Local LCD Application Functions  

void Initialize_Driver_Alert(void);

#ifdef DRIVER_ALERT_TEST
int main ()
{
 	byte Buffer[6];				// Buffer holds x,y,z acceleration data from the HEADSET SARD 
	char Line_Buffer[80];

	unsigned int result[20];
	float f1 = 3.1415;
 	unsigned int adc4_in, adc5_in, adc6_in; 											// ADC Input variables
    float X_Axis_Accel8, Y_Axis_Accel8, Z_Axis_Accel8;      							// 8-Bit X-Y and Z acceleration values range 0..255.0
    word X_Axis_Accel12, Y_Axis_Accel12, Z_Axis_Accel12;      							// 12-Bit X-Y and Z acceleration values range 0..4095
	float X_Axis_Volts, Y_Axis_Volts, Z_Axis_Volts;           							// Adjusted ADC values in volts
	float X_Axis_Angle, Y_Axis_Angle, Z_Axis_Angle;           							// Adjusted xyz angles in degrees
    float X_Axis_Headset_Angle, Y_Axis_Headset_Angle, Z_Axis_Headset_Angle;     	    // Headset SARD xyz  angles in degrees
    float X_Axis_Reference_Angle, Y_Axis_Reference_Angle, Z_Axis_Reference_Angle;     	// Reference SARD or Virtual SARD xyz angle in degrees
	word X_Axis_Time_Count, Y_Axis_Time_Count, Z_Axis_Time_Count;  						// Elapsed time counters for angles exceeding thresholds

	int x;										// Temporary variable
	int i,j;									// Local Loop Index variables
   	int timeout;								// Timeout value during which timer1 is ON
	byte TriaxData; 					        // Data byte read from SARD
	char Ch;

   // Setup timers, configure ports and initialize variables

   SetupTimers();  

  	// Configure UART  1 for current CPU cloc and baud rate,  8-Bits, No parity, Asynchronous
 
  	InitializeUart1();
 
 	// Configure UART  2 for current CPU cloc and baud rate,  8-Bits, No parity, Asynchronous
 
  	//InitializeUart2();

/*
	//Test #3 Serial Output // This works!!!
	sprintf(Line_Buffer,"Enter a character:\r\n", i);
	string(Line_Buffer);
	while (1)
	{
		Ch = getch();
		sprintf(Line_Buffer,"The character typed is !%c!\r\n", Ch);
		string(Line_Buffer);	
	}

*/


 	// Initialize the dsPIC30F6014 Development Board LCD using SPI2 here
  	Lcd_Initialize();

 	// Initialize Driver Alert application here
    Initialize_Driver_Alert();
 
	//**************************************************************************
 	//* USART Test Here
	//**************************************************************************

    printf(" Hello World \r\n");
	
	for (i=0;i<10;i++)
	{
		putchar('X');
		putchar('Y');
		putchar('Z');
		printf(" This is a test %d times \r\n",i);
	}
 
	// LCD printf test...
	// Clear LCD and home the cursor
 	Lcd_Clear();
	Lcd_Printf("f1 = %f ", f1);
 
	// Clear LCD and home the cursor
 	Lcd_Clear();

    for (i=0; i<4; i++)
	{
		Lcd_Set_Character_Position(1,i);
		Lcd_Printf("TestA # %d", i);
		printf("Test # %d \r\n", i);

	}

	// Calibration of Virtual TRIAX board 



 	// Clear LCD and home the cursor
 	Lcd_Clear();



	//**************************************************************************
	//* 12-Bit Analog to Digital Conversion test with 1 millisecond timer interrupts
	//**************************************************************************



/*
	// Reset timer interrupt counts here
   	TimerCount0 = 0;                  // Timer interrupt 0 count (Seconds)
    TimerCount1 = 0;                  // Timer interrupt 1 count (Seconds)
    TimerCount2 = 0;                  // Timer interrupt 2 count (Milliseconds)
    TimerCount3 = 0;                  // Timer interrupt 3 count 
    TimerCount4 = 0;                  // Timer interrupt 4 count (Microseconds)
    TimerCount5 = 0;                  // Timer interrupt 5 count


 	printf("Delay 5 seconds ...\r\n");
 	pause(5000);
 	printf("End of 5 second delay. \r\n");

    printf("TimerCount1 = %ld, TimerCount2 = %ld, TimerCount3 = %ld, TimerCount4 = %ld, TimerCount5 = %ld \r\n",
               TimerCount1, TimerCount2, TimerCount3, TimerCount4, TimerCount5 );                         // Display timer  counts
  
	pause(3000);
*/


	// Reset timer interrupt counts here
   	TimerCount0 = 0;                  // Timer interrupt 0 count (Seconds)
    TimerCount1 = 0;                  // Timer interrupt 1 count (Seconds)
    TimerCount2 = 0;                  // Timer interrupt 2 count (Milliseconds)
    TimerCount3 = 0;                  // Timer interrupt 3 count 
    TimerCount4 = 0;                  // Timer interrupt 4 count (Microseconds)
    TimerCount5 = 0;                  // Timer interrupt 5 count

    // Set global viewport coordinates here to scale (decimate) 12-Bit sensor data to 8-Bits

    set_view(0, 0, 4095, 4095);

	// Use calibrated constants to set the viewport for sensor scaling and offset computations

   //set_view(X_AXIS_MINUS_1G, Y_AXIS_MINUS_1G, X_AXIS_PLUS_1G, X_AXIS_PLUS_1G);

    // set global window coordinates here to scale (decimate) 12-Bit sensor data to 8-Bits

    set_window(0.0, 0.0, 255.0, 255.0);



//**************************************************************************************
//**************************************************************************************
//*                      Main Driver Alert Control Loop
//**************************************************************************************
//**************************************************************************************

	// Reset Counters Here
	X_Axis_Time_Count = 0;
	Y_Axis_Time_Count = 0;
	Z_Axis_Time_Count = 0;

	for (;;)
	{
    	Nop();
		i = 0;
		while( i <16 )
		{
/*
 			// Start sampling the ADC 
			ADCON1bits.SAMP = 1;
 			while(!ADCON1bits.SAMP);
*/
			// Convert the data from SARD X-Y and Z accelerometers to raw 12-Bit values

			ConvertADC12();
 			while(ADCON1bits.SAMP);
			while(!BusyADC12());
 			//while(BusyADC12());

 			// Read the ADC for selected channels 
			result[i] = ReadADC12(i);
			i++;
		}

		// Calibrate the X,Y,Z accelerometers here ....
/*
		// Voltage values by POT channel
    	printf("  Pot 1  = %f",result[5] * .00122);
     	printf("  Pot 2  = %f",result[6] * .00122);
    	printf("  Pot 3  = %f Volts\r\n",result[4] * .00122);
		
		// Binary values from the (X,Y,Z) accelerometers 
    	printf("  X  = %d",result[10] );
     	printf("  Y  = %d",result[11]);
    	printf("  Z  = %d Bits\r\n",result[12]);


		// Voltage values from the (X,Y,Z) accelerometers 
    	printf("  X  = %f",result[10] * .00122);
    	printf("  Y  = %f",result[11] * .00122);
     	//printf("  Y  = %f Volts\r\n",result[11] * .00122 );
     	//printf("  Y  = %f",result[11] * .00122);
     	printf("  Z  = %f Volts\r\n",result[12] * .00122);
*/

		// Display acceleration values to the LCD every 0.5 seconds (500 1 ms ticks)
		//if ((TimerCount3 % 1000) == 0) 
		if ((TimerCount3 % 500) == 0)
		{ 
 			// Clear LCD and home the cursor
 			Lcd_Clear();

			// Subtract the reference xyz angles from the Headset xyz angles

 			// Angles (X,Y,Z) read from the  Headset SARD accelerometers.  
    		X_Axis_Headset_Angle =
			Y_Axis_Headset_Angle =
            Z_Axis_Headset_Angle =     		

			// Angles (X,Y,Z) read from the  Reference SARD or Virtual SARD accelerometers.  Works!!!
			X_Axis_Reference_Angle = result[X_INDEX] * ConvertToDegrees - 90.0;
			Y_Axis_Reference_Angle = result[Y_INDEX] * ConvertToDegrees - 90.0;
			Z_Axis_Reference_Angle = result[Z_INDEX] * ConvertToDegrees - 90.0;
   
			// Adjusted xyz angles in degrees
			X_Axis_Angle = abs(X_Axis_Headset_Angle - X_Axis_Reference_Angle);
            Y_Axis_Angle = abs(Y_Axis_Headset_Angle - Y_Axis_Reference_Angle);
            Z_Axis_Angle = abs(Z_Axis_Headset_Angle - Z_Axis_Reference_Angle);        						
       
			Lcd_Set_Character_Position(1,0);
			Lcd_Printf("X,Y,Z Tilt Angles");
			Lcd_Set_Character_Position(1,1);
    		Lcd_Printf("X = %3.2f Degrees",X_Axis_Angle);
 			Lcd_Set_Character_Position(1,2);
   	  		Lcd_Printf("Y = %3.2f Degrees",Y_Axis_Angle);			
  			Lcd_Set_Character_Position(1,3);
     		Lcd_Printf("Z = %3.2f Degrees",Z_Axis_Angle);

			// Simple Threshold Driver Alert Algorithm
			// Compare against angles exceeded for too long (Driver Falling Asleep
            // at the wheel or not paying attention to the road)

			if (X_Axis_Angle > X_AXIS_THRESHOLD)
			{
				X_Axis_Time_Count++;		// Increment # of 1 ms ticks that threshold was exceeded in X direction
				if (X_Axis_Time_Count > X_AXIS_TIME_LIMIT)
				{
					X_Axis_Time_Count = 0;

					// Trigger Alarm play road noise and turn on motor here...

					// Clear LCD and home the cursor
 					Lcd_Clear();
					Lcd_Set_Character_Position(1,0);
					Lcd_Printf("W A K E  U P !!!");
					while(1);
				}
			}

			if (Y_Axis_Angle > Y_AXIS_THRESHOLD)
			{
				Y_Axis_Time_Count++;		// Increment # of 1 ms ticks that threshold was exceeded in X direction
				if (Y_Axis_Time_Count > Y_AXIS_TIME_LIMIT)
				{
					Y_Axis_Time_Count = 0;

					// Trigger Alarm play road noise and turn on motor here...

					// Clear LCD and home the cursor
 					Lcd_Clear();
					Lcd_Set_Character_Position(1,0);
					Lcd_Printf("W A K E  U P !!!");
					while(1);
				}
			}

			if (Z_Axis_Angle > Z_AXIS_THRESHOLD)
			{
				Z_Axis_Time_Count++;		// Increment # of 1 ms ticks that threshold was exceeded in X direction
				if (Z_Axis_Time_Count > Z_AXIS_TIME_LIMIT)
				{
					Z_Axis_Time_Count = 0;

					// Trigger Alarm play road noise and turn on motor here...

					// Clear LCD and home the cursor
 					Lcd_Clear();
					Lcd_Set_Character_Position(1,0);
					Lcd_Printf("W A K E  U P !!!");
					while(1);
				}
			}

 
/*
			// 12-Bit values read from the (X,Y,Z) accelerometers.  Works!!!
			Lcd_Set_Character_Position(1,0);
			Lcd_Printf(" 12-Bit ADC Values ");
			Lcd_Set_Character_Position(1,1);
    		Lcd_Printf(" X  = %d Bits",result[X_INDEX]);
 			Lcd_Set_Character_Position(1,2);
    		Lcd_Printf(" Y  = %d Bits",result[Y_INDEX]);			
  			Lcd_Set_Character_Position(1,3);
      		Lcd_Printf(" Z  = %d Bits",result[Z_INDEX]);


			// Voltage values read from the (X,Y,Z) accelerometers.  Works!!!
			X_Axis_Volts = result[X_INDEX] * ConvertThreeVolts;
			Y_Axis_Volts = result[Y_INDEX] * ConvertThreeVolts;
			Z_Axis_Volts = result[Z_INDEX] * ConvertFiveVolts;
          
 			Lcd_Set_Character_Position(1,0);
			Lcd_Printf(" X,Y,Z Accel. Volts");
			Lcd_Set_Character_Position(1,1);
    		Lcd_Printf(" X  = %1.2f Volts",X_Axis_Volts);
 			Lcd_Set_Character_Position(1,2);
   	  		Lcd_Printf(" Y  = %1.2f Volts",Y_Axis_Volts);			
  			Lcd_Set_Character_Position(1,3);
     		Lcd_Printf(" Z  = %1.2f Volts",Z_Axis_Volts);


			// Angles values read from the (X,Y,Z) accelerometers.  Works!!!
			X_Axis_Angle = result[X_INDEX] * ConvertToDegrees;
			Y_Axis_Angle = result[Y_INDEX] * ConvertToDegrees;
			Z_Axis_Angle = result[Z_INDEX] * ConvertToDegrees;
          
 			Lcd_Set_Character_Position(1,0);
			Lcd_Printf(" X,Y,Z Tilt Angles");
			Lcd_Set_Character_Position(1,1);
    		Lcd_Printf(" X  = %3.2f Degrees",X_Axis_Angle);
 			Lcd_Set_Character_Position(1,2);
   	  		Lcd_Printf(" Y  = %3.2f Degrees",Y_Axis_Angle);			
  			Lcd_Set_Character_Position(1,3);
     		Lcd_Printf(" Z  = %3.2f Degrees",Z_Axis_Angle);


			// 12-Bit values read from the (X,Y,Z) accelerometers.  Works!!!
			Lcd_Set_Character_Position(1,0);
			Lcd_Printf(" 12-Bit ADC Values ");
			Lcd_Set_Character_Position(1,1);
    		Lcd_Printf(" X  = %d Bits",result[X_INDEX]);
 			Lcd_Set_Character_Position(1,2);
    		Lcd_Printf(" Y  = %d Bits",result[Y_INDEX]);			
  			Lcd_Set_Character_Position(1,3);
      		Lcd_Printf(" Z  = %d Bits",result[Z_INDEX]);

			// Voltage values read by POT channel Works!!!
			Lcd_Set_Character_Position(1,0);
			Lcd_Printf(" Pots 1-3 Volts");
			Lcd_Set_Character_Position(1,1);
    		Lcd_Printf("  Pot 1  = %1.2f",result[5] * ConvertFiveVolts);
 			Lcd_Set_Character_Position(1,2);
     		Lcd_Printf("  Pot 2  = %1.2f",result[6] * ConvertFiveVolts);
  			Lcd_Set_Character_Position(1,3);
    		Lcd_Printf("  Pot 3  = %1.2f Volts\r\n",result[4] * ConvertFiveVolts);
*/
		}


		if ((TimerCount3 % 100) == 0) 
		{
			// Flash LED 3
			LED_3 = ~ LED_3;


/*
			// Voltage values by POT channel Works!!!
    		printf("  Pot 1  = %f",result[5] * ConvertFiveVolts);
     		printf("  Pot 2  = %f",result[6] * ConvertFiveVolts);
    		printf("  Pot 3  = %f Volts\r\n",result[4] * ConvertFiveVolts);

			// Voltage values from the (X,Y,Z) accelerometers.  Works!!!
    		printf("  X  = %f",result[X_INDEX] * ConvertThreeVolts);
    		printf("  Y  = %f",result[Y_INDEX]  * ConvertThreeVolts);			
     		//printf("  Y  = %f Volts\r\n",result[Y_INDEX]  * ConvertThreeVolts);
     		//printf("  Y  = %f",result[Y_INDEX]  * ConvertThreeVolts);
     		printf("  Z  = %f Volts\r\n",result[Z_INDEX] * ConvertFiveVolts);


			// 12-Bit values read from the (X,Y,Z) accelerometers.  Works!!!
    		printf("  X  = %d",result[X_INDEX]);
    		printf("  Y  = %d",result[Y_INDEX]);			
     		//printf("  Y  = %d Bits\r\n",result[Y_INDEX]);
     		//printf("  Y  = %d",result[Y_INDEX]);
     		printf("  Z  = %d Bits\r\n",result[Z_INDEX]);
 
			// Voltage values from the (X,Y,Z) accelerometers.  Works!!!
    		printf("  X  = %f",result[X_INDEX] * ConvertThreeVolts);
    		printf("  Y  = %f",result[Y_INDEX]  * ConvertThreeVolts);			
     		//printf("  Y  = %f Volts\r\n",result[Y_INDEX]  * ConvertThreeVolts);
     		//printf("  Y  = %f",result[Y_INDEX]  * ConvertThreeVolts);
     		printf("  Z  = %f Volts\r\n",result[Z_INDEX] * ConvertFiveVolts);

			// Voltage values from the (X,Y,Z) accelerometers.  Works!!!
    		printf("  X  = %f",result[X_INDEX] * .00122);
    		printf("  Y  = %f",result[Y_INDEX] * .00122);			
     		//printf("  Y  = %f Volts\r\n",result[Y_INDEX] * .00122 );
     		//printf("  Y  = %f",result[Y_INDEX] * .00122);
     		printf("  Z  = %f Volts\r\n",result[Z_INDEX] * .00122);

			// Voltage values from the (X,Y,Z) accelerometers.  Works!!!
    		printf("  X  = %f",result[X_INDEX] * .00122);
    		printf("  Y  = %f",result[Y_INDEX] * .00122);			
     		//printf("  Y  = %f Volts\r\n",result[Y_INDEX] * .00122 );
     		//printf("  Y  = %f",result[Y_INDEX] * .00122);
     		printf("  Z  = %f Volts\r\n",result[Z_INDEX] * .00122);

			// Display timer interrupt counts here...  (This code works!!!)
    		// Note: Timer Interrupts must be enabled using compilation flag                             
        	printf("TimerCount1 = %ld, TimerCount2 = %ld, TimerCount3 = %ld, TimerCount4 = %ld, TimerCount5 = %ld \r\n",
               TimerCount1, TimerCount2, TimerCount3, TimerCount4, TimerCount5 );                         // Display timer  counts
  
      		//Nop();
			//pause(500);
*/
    		// Scale the accelerometer X,Y,Z reading from 12-Bits to 8-Bits to emulate the 8-Bit ADC on the TRIAX Boards
 	     	//map_window_to_viewport (result[X_INDEX], result[Y_INDEX], &X_Axis_Accel8, &Y_Axis_Accel8);
	     	//map_window_to_viewport (result[Z_INDEX], result[Z_INDEX], &Z_Axis_Accel8, &Z_Axis_Accel8);


 	  	    map_viewport_to_window (result[X_INDEX], result[Y_INDEX], &X_Axis_Accel8, &Y_Axis_Accel8);
      	    map_viewport_to_window (result[Z_INDEX], result[Z_INDEX], &Z_Axis_Accel8, &Z_Axis_Accel8);

 			// Send  data read from the BASE (Reference) Virtual TRIAX back to the laptop TRIAX application
     	    //printf("X_Axis_Accel8=%d, Y_Axis_Accel8=%d, Z_Axis_Accel8=%d\r\n ", (byte)X_Axis_Accel8, (byte)Y_Axis_Accel8, (byte)Z_Axis_Accel8);
 
			send_byte('x');
			send_byte((byte)X_Axis_Accel8);
			send_byte('y');
			send_byte((byte)Y_Axis_Accel8);
			send_byte('z');
			send_byte((byte)Z_Axis_Accel8);
 

		}



/*
for (j=0; j<20; j++)
{
	printf("j = %d, result = %d \r\n", j, result[j]);
}
pause(1000);

		printf("adc4_in = %d ", result[4]);
		printf("adc5_in = %d ", result[5]);
		printf("adc6_in = %d \r\n", result[6]);

		// Voltage values by ADC channel
   		printf("Voltage 1 (AN6/RP1) = %f",result[6] * .00122);
    	printf("  Voltage 2 (AN4/RP2) = %f",result[4] * .00122);
    	printf("  Voltage 3 (AN5/RP3) = %f\r\n",result[5] * .00122);
     	Nop();
*/

	}


/*

	// Read TRIAX data from the HEADSET SARD board and process it along with data read from the BASE (Reference) Virtual TRIAX board
	while(1)
	{
 
 		// Read TRIAX acceleration data from HEADSET SARD using UART1 echo it to the display in raw (x,y,z) format

		// Read raw X acceleration value
		TriaxData = get_byte();
		if (TriaxData == 'x')
		{
			Buffer[0] = TriaxData;
			Buffer[1] = get_byte();
		}
		// Read raw Y acceleration value
		else if (TriaxData == 'y')
		{
			Buffer[2] = TriaxData;
			Buffer[3] = get_byte();
		}
		// Read raw Z acceleration value
		else if (TriaxData == 'z')
		{
			Buffer[4] = TriaxData;
			Buffer[5] = get_byte();

			// Display the data 

			printf("x%3d, y%3d, z%3d \r\n", Buffer[1], Buffer[3], Buffer[5]);
		}
		else	
		{
			printf("Bad data read from SARD!!!\r\n");
		}

	}

	for (;;)
	{
    	Nop();

 
 
		// Start sampling the ADC 
		ADCON1bits.SAMP = 1;
    	while(!ADCON1bits.SAMP);
    	ConvertADC12();
    	while(ADCON1bits.SAMP);

		//while(!BusyADC12());
		while(BusyADC12());
	
		// Read the ADC channels 4,5 and 6 
    	adc4_in = ReadADC12(4);
    	adc5_in = ReadADC12(5);
    	adc6_in = ReadADC12(6);

		// Display the values read to the serial port and LCD
		//Lcd_Printf("adc4_in = %d", adc4_in);
		printf("adc4_in = %d ", adc4_in);
		printf("adc5_in = %d ", adc5_in);
		printf("adc6_in = %d\r\n", adc6_in);
		pause(200);
	

     printf("%c[1;1H Voltage 1 (AN6/RP1) = %f",esc,adc6_in * .00122);
    printf("%c[2;1H Voltage 2 (AN4/RP2) = %f",esc,adc4_in * .00122);
    printf("%c[3;1H Voltage 3 (AN5/RP3) = %f",esc,adc5_in * .00122);
     Nop();
	
    }while(1);
 	
	}
*/

#ifdef TEST
// Unit test code ...
 

    // Map 8-Bit ADC calibration values to corresponding 12-Bit ADC values

   	X_Axis_Accel8 = (float) X_AXIS_MINUS_1G;
   	Y_Axis_Accel8 = (float) Y_AXIS_MINUS_1G;
   	Z_Axis_Accel8 = (float) Z_AXIS_MINUS_1G;

	map_window_to_viewport (X_Axis_Accel8, Y_Axis_Accel8, &X_Axis_Accel12, &Y_Axis_Accel12);
	map_window_to_viewport (Z_Axis_Accel8, Z_Axis_Accel8, &Z_Axis_Accel12, &Z_Axis_Accel12);


    printf("-1g: X_Axis_Accel12=%d, Y_Axis_Accel12=%d, Z_Axis_Accel12=%d, X_Axis_Accel8=%d, Y_Axis_Accel8=%d, Z_Axis_Accel8=%d\r\n ", 
            X_Axis_Accel12, Y_Axis_Accel12, Z_Axis_Accel12, (byte)X_Axis_Accel8, (byte)Y_Axis_Accel8, (byte)Z_Axis_Accel8);


   	X_Axis_Accel8 = (float) X_AXIS_0G;
   	Y_Axis_Accel8 = (float) Y_AXIS_0G;
   	Z_Axis_Accel8 = (float) Z_AXIS_0G;

	map_window_to_viewport (X_Axis_Accel8, Y_Axis_Accel8, &X_Axis_Accel12, &Y_Axis_Accel12);
	map_window_to_viewport (Z_Axis_Accel8, Z_Axis_Accel8, &Z_Axis_Accel12, &Z_Axis_Accel12);
    printf("0g: X_Axis_Accel12=%d, Y_Axis_Accel12=%d, Z_Axis_Accel12=%d, X_Axis_Accel8=%d, Y_Axis_Accel8=%d, Z_Axis_Accel8=%d\r\n ", 
            X_Axis_Accel12, Y_Axis_Accel12, Z_Axis_Accel12, (byte)X_Axis_Accel8, (byte)Y_Axis_Accel8, (byte)Z_Axis_Accel8);


   	X_Axis_Accel8 = (float) X_AXIS_PLUS_1G;
   	Y_Axis_Accel8 = (float) Y_AXIS_PLUS_1G;
   	Z_Axis_Accel8 = (float) Z_AXIS_PLUS_1G;
	map_window_to_viewport (X_Axis_Accel8, Y_Axis_Accel8, &X_Axis_Accel12, &Y_Axis_Accel12);
	map_window_to_viewport (Z_Axis_Accel8, Z_Axis_Accel8, &Z_Axis_Accel12, &Z_Axis_Accel12);
    printf("+1g: X_Axis_Accel12=%d, Y_Axis_Accel12=%d, Z_Axis_Accel12=%d, X_Axis_Accel8=%d, Y_Axis_Accel8=%d, Z_Axis_Accel8=%d\r\n ", 
            X_Axis_Accel12, Y_Axis_Accel12, Z_Axis_Accel12, (byte)X_Axis_Accel8, (byte)Y_Axis_Accel8, (byte)Z_Axis_Accel8);



	while(1);

    // Ramp test, map viewport 0..4095 to window 0.0..255.0 Works!!!
    for (i = 0; i<4096; i++)
    {
    	X_Axis_Accel12 = i;
    	Y_Axis_Accel12 = i;
		//map_window_to_viewport (X_Axis_Accel12, Y_Axis_Accel12, &X_Axis_Accel8, &Y_Axis_Accel8);
		map_viewport_to_window (X_Axis_Accel12, Y_Axis_Accel12, &X_Axis_Accel8, &Y_Axis_Accel8);

     	printf("X_Axis_Accel12=%d, Y_Axis_Accel12=%d, X_Axis_Accel8=%d, Y_Axis_Accel8=%d\r\n ", 
            X_Axis_Accel12, Y_Axis_Accel12, (byte)X_Axis_Accel8, (byte)Y_Axis_Accel8);

    }

    // Test the mapping of Window to Viewport coordinate functions here.  Works !!!!

    X_Axis_Accel8 = 0;
    Y_Axis_Accel8 = 0;
    X_Axis_Accel12 = 0.0;
    Y_Axis_Accel12 = 0.0;
   	map_viewport_to_window(X_Axis_Accel8, Y_Axis_Accel8, &X_Axis_Accel12, &Y_Axis_Accel12);

    printf("X_Axis_Accel12=%f, Y_Axis_Accel12=%f, X_Axis_Accel8=%d, Y_Axis_Accel8=%d\r\n ", 
            X_Axis_Accel12, Y_Axis_Accel12, X_Axis_Accel8, Y_Axis_Accel8);


    X_Axis_Accel8 = 255;
    Y_Axis_Accel8 = 255;
    X_Axis_Accel12 = 0.0;
    Y_Axis_Accel12 = 0.0;
 
   	map_viewport_to_window(X_Axis_Accel8, Y_Axis_Accel8, &X_Axis_Accel12, &Y_Axis_Accel12);

    printf("X_Axis_Accel12=%f, Y_Axis_Accel12=%f, X_Axis_Accel8=%d, Y_Axis_Accel8=%d\r\n ", 
            X_Axis_Accel12, Y_Axis_Accel12, X_Axis_Accel8, Y_Axis_Accel8);

    X_Axis_Accel8 = 127;
    Y_Axis_Accel8 = 127;
    X_Axis_Accel12 = 0.0;
    Y_Axis_Accel12 = 0.0;
 
   	map_viewport_to_window(X_Axis_Accel8, Y_Axis_Accel8, &X_Axis_Accel12, &Y_Axis_Accel12);

    printf("X_Axis_Accel12=%f, Y_Axis_Accel12=%f, X_Axis_Accel8=%d, Y_Axis_Accel8=%d\r\n ", 
            X_Axis_Accel12, Y_Axis_Accel12, X_Axis_Accel8, Y_Axis_Accel8);
 
     // Ramp test, map 0..255 to 0..4095  Works!!!
    for (i = 0; i<256; i++)
    {
    	X_Axis_Accel8 = i;
    	Y_Axis_Accel8 = i;
     	map_viewport_to_window(X_Axis_Accel8, Y_Axis_Accel8, &X_Axis_Accel12, &Y_Axis_Accel12);
  		X_Axis_Accel12 = ceil(X_Axis_Accel12);
    	Y_Axis_Accel12 = ceil(Y_Axis_Accel12);
     	printf("X_Axis_Accel12=%f, Y_Axis_Accel12=%f, X_Axis_Accel8=%d, Y_Axis_Accel8=%d\r\n ", 
            X_Axis_Accel12, Y_Axis_Accel12, X_Axis_Accel8, Y_Axis_Accel8);

    }

     // Wait five seconds...
    while(1);


    // Test the mapping of Window to Viewport coordinate functions here.   These work!!!

    X_Axis_Accel8 = 0;
    Y_Axis_Accel8 = 0;
    X_Axis_Accel12 = 0.0;
    Y_Axis_Accel12 = 0.0;

	map_window_to_viewport (X_Axis_Accel12, Y_Axis_Accel12, &X_Axis_Accel8, &Y_Axis_Accel8);

    printf("X_Axis_Accel12=%f, Y_Axis_Accel12=%f, X_Axis_Accel8=%d, Y_Axis_Accel8=%d\r\n ", 
            X_Axis_Accel12, Y_Axis_Accel12, X_Axis_Accel8, Y_Axis_Accel8);

    // Wait five seconds...
    //while(1);

    X_Axis_Accel8 = 0;
    Y_Axis_Accel8 = 0;
    X_Axis_Accel12 = 4096.0;
    Y_Axis_Accel12 = 4096.0;
    //X_Axis_Accel12 = 4080.0;
    //Y_Axis_Accel12 = 4080.0;

	map_window_to_viewport (X_Axis_Accel12, Y_Axis_Accel12, &X_Axis_Accel8, &Y_Axis_Accel8);
    X_Axis_Accel12 = ceil(X_Axis_Accel12);
    Y_Axis_Accel12 = ceil(Y_Axis_Accel12);

     printf("X_Axis_Accel12=%f, Y_Axis_Accel12=%f, X_Axis_Accel8=%d, Y_Axis_Accel8=%d\r\n ", 
            X_Axis_Accel12, Y_Axis_Accel12, X_Axis_Accel8, Y_Axis_Accel8);

    // Wait five seconds...
    //while(1);

    X_Axis_Accel8 = 0;
    Y_Axis_Accel8 = 0;
    //X_Axis_Accel12 = 2000.0;
    //Y_Axis_Accel12 = 2000.0;
    X_Axis_Accel12 = 2048.0;
    Y_Axis_Accel12 = 2048.0;

	map_window_to_viewport (X_Axis_Accel12, Y_Axis_Accel12, &X_Axis_Accel8, &Y_Axis_Accel8);

    printf("X_Axis_Accel12=%f, Y_Axis_Accel12=%f, X_Axis_Accel8=%d, Y_Axis_Accel8=%d\r\n ", 
            X_Axis_Accel12, Y_Axis_Accel12, X_Axis_Accel8, Y_Axis_Accel8);

    // Ramp test, map window 0..4095.0 to viewport 0..255 Works!!!
    for (i = 0; i<4096; i++)
    {
    	X_Axis_Accel12 = (float)i;
    	Y_Axis_Accel12 = (float)i;
		map_window_to_viewport (X_Axis_Accel12, Y_Axis_Accel12, &X_Axis_Accel8, &Y_Axis_Accel8);

     	printf("X_Axis_Accel12=%f, Y_Axis_Accel12=%f, X_Axis_Accel8=%d, Y_Axis_Accel8=%d\r\n ", 
            X_Axis_Accel12, Y_Axis_Accel12, X_Axis_Accel8, Y_Axis_Accel8);

    }

     // Wait five seconds...
    while(1);

#endif

}

#endif

//******************************************************************************
//* Function Name:    Initialize_Driver_Alert                                  *
//* Description:      Initializes the Driver Alert Application                 *
//*                                                                            *
//******************************************************************************
void Initialize_Driver_Alert(void)
{
	int i,j;									// Local Loop Index variables
 	unsigned int channel,adc12cfg_1,adc12cfg_2,adc12cfg_3,adc12cfg_p,adc12cfg_s; // ADC Configuration variables
 	int timeout;								// Timeout value during which timer1 is ON

	 // Initialize PORTs 

    // Configure Port D as outputs
	PORTD = 0x0000; 
	TRISD = 0x0000;  

	// LEDs using for debug
  	LED_1   = 1;					 			// LED 1 (0=On, 1=Off)
  	LED_2   = 1;					 			// LED 2 (0=On, 1=Off)
  	LED_3   = 1;					 			// LED 3 (0=On, 1=Off)
  	LED_4  	= 1;				 	 	 		// LED 4 (0=On, 1=Off)
 
	// Direction bits for LEDs
 	LED_1_DIR  = 0;				 			// LED 1 DIRECTION =0
 	LED_2_DIR  = 0;					 		// LED 2 DIRECTION =0 
 	LED_3_DIR  = 0;					 		// LED 3 DIRECTION =0 
 	LED_4_DIR  = 0; 					 	// LED 4 DIRECTION =0 
 
/*
	PORTA = 0; 
	TRISA = 0xFFFF; 
	PORTB = 0; 
	TRISB = 0xFFFF; 
	PORTC = 0; 
	TRISC = 0xFFFF; 
	PORTE = 0; 
	TRISE = 0xFFFF; 
	PORTF = 0; 
	TRISF = 0xFFFF; 
	PORTG = 0; 
	TRISG = 0xFFFF; 

	ADPCFG = 0xFFFF; //Digitial I/O

*/
	// Configure ADC Port B, AN15:AN0,  analog=0 and digital=1  

	ADPCFG = 0xE38F;  

	//**************************************************************************
	//* Setup the 12-Bit Analog to Digital Converter (works)
	//**************************************************************************
      
	ADCON1bits.ADON = 0; // turn off ADC 
    channel = //ADC_CH0_POS_SAMPLEA_AN4 &    // Pot 3
              //ADC_CH0_POS_SAMPLEA_AN5 &    // Pot 1
              //ADC_CH0_POS_SAMPLEA_AN6 &    // Pot 2
              //ADC_CH0_POS_SAMPLEA_AN8 &
              //ADC_CH0_POS_SAMPLEA_AN9 &         
              ADC_CH0_POS_SAMPLEA_AN10 &    // X
           	  ADC_CH0_POS_SAMPLEA_AN11 &    // Y
              ADC_CH0_POS_SAMPLEA_AN12 &    // Z
              //ADC_CH0_POS_SAMPLEA_AN13 &
              ADC_CH0_POS_SAMPLEA_AN15;


    SetChanADC12(channel);
	ConfigIntADC12(ADC_INT_DISABLE);

    adc12cfg_1 = ADC_MODULE_ON &
				 ADC_IDLE_CONTINUE &
				 ADC_FORMAT_INTG &
				 ADC_CLK_AUTO &
				 //ADC_CLK_MANUAL &
				 //ADC_AUTO_SAMPLING_OFF;
				 ADC_AUTO_SAMPLING_ON;
				 //ADC_SAMP_ON;

	adc12cfg_2 = ADC_VREF_AVDD_AVSS &
				 ADC_SCAN_ON &
	             ADC_SAMPLES_PER_INT_16 &
				 ADC_ALT_BUF_OFF &
				 ADC_ALT_INPUT_OFF;

	adc12cfg_3 = //ADC_SAMPLE_TIME_10 &
				 ADC_SAMPLE_TIME_31 &
				 ADC_CONV_CLK_SYSTEM &
                 ADC_CONV_CLK_INTERNAL_RC; // Works!
                 //ADC_CONV_CLK_13Tcy;
				 //ADC_CONV_CLK_32Tcy;  // Works!



	adc12cfg_p = //ENABLE_AN4_ANA &    // Pot 3
				 //ENABLE_AN5_ANA &    // Pot 1
				 //ENABLE_AN6_ANA &    // Pot 2
				 //ENABLE_AN8_ANA &
				 //ENABLE_AN9_ANA &
				 ENABLE_AN10_ANA &   // X
				 ENABLE_AN11_ANA &   // Y
				 ENABLE_AN12_ANA &   // Z
				 //ENABLE_AN13_ANA &
			  	 ENABLE_AN15_ANA,
				(BIT4|BIT5|BIT6|BIT8|BIT15);	//errata requires ch15 ???


	adc12cfg_s = SKIP_SCAN_AN0 &
                 SKIP_SCAN_AN1 &
                 SKIP_SCAN_AN2 &
                 SKIP_SCAN_AN3 &
                 SKIP_SCAN_AN4 &
                 SKIP_SCAN_AN5 &
                 SKIP_SCAN_AN6 &
                 SKIP_SCAN_AN7 &
                 SKIP_SCAN_AN8 &
                 SKIP_SCAN_AN9 &
                 //SKIP_SCAN_AN10 &  // X
                 //SKIP_SCAN_AN11 &  // Y
                 //SKIP_SCAN_AN12 &  // Z
                 SKIP_SCAN_AN13 &
                 SKIP_SCAN_AN14 &
                 SKIP_SCAN_AN15;

 	OpenADC12(adc12cfg_1,adc12cfg_2,adc12cfg_3,adc12cfg_p,adc12cfg_s);






/*
	// Port  Direction (0=output, 1=input)
	// Initialize Port A Bits

	X_Y_ACCELEROMETER_ST  	  =  0;       		// X,Y accelerometer MMA6261Q Module start test = 1 (output)
	X_Y_ACCELEROMETER_ST_DIR  = OUTPUT;        	// X,Y accelerometer MMA6261Q Module start test = 1 (output)
 
	//X_Y_ACCELEROMETER_STATUS 	   	    		// X,Y accelerometer MMA6261Q Module status (input)
	X_Y_ACCELEROMETER_STATUS_DIR = INPUT;  		// X,Y accelerometer MMA6261Q Module status (input)
 
	Z_ACCELEROMETER_ST        = 0;       		// Z accelerometer MMA1260D Module start test = 1 (output)
	Z_ACCELEROMETER_ST_DIR    = OUTPUT;      	// Z accelerometer MMA1260D Module start test = 1 (output)
 
	//Z_ACCELEROMETER_STATUS	           		// Z accelerometer MMA1260D Module status (input)
	Z_ACCELEROMETER_STATUS_DIR	= INPUT;     	// Z accelerometer MMA1260D Module status (input)

*/

 
/*
// Initialize ADC 
ADCON1 = 0; 
ADCON1bits.SSRC = 7; //auto convert conversion trigger mode 
ADCON1bits.ASAM = 1; //auto sample start mode 
ADCON1bits.FORM = 3; //signed fractional data format 
ADCHS = 13; //sample Size_Start 
ADPCFG = 0xFFFE; //AN0 is an analog input 
ADCON3 = 0; 
ADCON3bits.SAMC = 1; //sets the sample/conversion clock cycles 
ADCON2 = 0; //interrupt after every sample 
*/


/*
	//**************************************************************************
	//* Setup the 12-Bit Analog to Digital Converter (works)
	//**************************************************************************
      
	ADCON1bits.ADON = 0; // turn off ADC 
    channel = ADC_CH0_POS_SAMPLEA_AN4 &
              ADC_CH0_POS_SAMPLEA_AN5 &
              ADC_CH0_POS_SAMPLEA_AN6;
              //ADC_CH0_POS_SAMPLEA_AN8 &
              //ADC_CH0_POS_SAMPLEA_AN9;
              //ADC_CH0_POS_SAMPLEA_AN10;
           	  //ADC_CH0_POS_SAMPLEA_AN11;
              //ADC_CH0_POS_SAMPLEA_AN12;


    SetChanADC12(channel);
	ConfigIntADC12(ADC_INT_DISABLE);

    adc12cfg_1 = ADC_MODULE_ON &
				 ADC_IDLE_CONTINUE &
				 ADC_FORMAT_INTG &
				 ADC_CLK_MANUAL &
				 ADC_AUTO_SAMPLING_OFF;
				 //ADC_SAMP_ON;

	adc12cfg_2 = ADC_VREF_AVDD_AVSS &
				 ADC_SCAN_ON &
	             ADC_SAMPLES_PER_INT_16 &
				 ADC_ALT_BUF_OFF &
				 ADC_ALT_INPUT_OFF;

	adc12cfg_3 = //ADC_SAMPLE_TIME_10;
                 ADC_CONV_CLK_INTERNAL_RC;
                 //ADC_CONV_CLK_13Tcy;

	adc12cfg_p = ENABLE_AN4_ANA &
				 ENABLE_AN5_ANA &
				 ENABLE_AN6_ANA;
				 //ENABLE_AN8_ANA &
				 //ENABLE_AN9_ANA;
				 //ENABLE_AN10_ANA;
				 //ENABLE_AN11_ANA;
				 //ENABLE_AN12_ANA;

	adc12cfg_s = SKIP_SCAN_AN0 &
                 SKIP_SCAN_AN1 &
                 SKIP_SCAN_AN2 &
                 SKIP_SCAN_AN3 &
                 //SKIP_SCAN_AN4 &
                 //SKIP_SCAN_AN5 &
                 //SKIP_SCAN_AN6 &
                 SKIP_SCAN_AN7 &
                 SKIP_SCAN_AN8 &
                 SKIP_SCAN_AN9 &
                 SKIP_SCAN_AN10 &
                 SKIP_SCAN_AN11 &
                 SKIP_SCAN_AN12 &
                 SKIP_SCAN_AN13 &
                 SKIP_SCAN_AN14 &
                 SKIP_SCAN_AN15;

 	OpenADC12(adc12cfg_1,adc12cfg_2,adc12cfg_3,adc12cfg_p,adc12cfg_s);

*/


#ifdef NEW_CODE
	TRISBbits.TRISB4 = 1;	//make sure A/D port pins are inputs
	TRISBbits.TRISB5 = 1;
	TRISBbits.TRISB6 = 1;

	ADCON1bits.ADON = 0; // turn off ADC 
	SetChanADC12(   ADC_CH0_POS_SAMPLEA_AN4 &
					ADC_CH0_POS_SAMPLEA_AN5 &
					ADC_CH0_POS_SAMPLEA_AN6 &
					ADC_CH0_POS_SAMPLEA_AN8 &
					ADC_CH0_POS_SAMPLEA_AN15 &
					ADC_CH0_NEG_SAMPLEA_NVREF &
					ADC_CH0_POS_SAMPLEB_AN0 &
					ADC_CH0_NEG_SAMPLEB_NVREF);
	ConfigIntADC12(ADC_INT_DISABLE);
	OpenADC12(	ADC_MODULE_ON &
					ADC_IDLE_CONTINUE &
					ADC_FORMAT_INTG &
					ADC_CLK_AUTO &
					ADC_AUTO_SAMPLING_ON,
				ADC_VREF_AVDD_AVSS &
					ADC_SCAN_ON &
					ADC_ALT_BUF_OFF &
					ADC_ALT_INPUT_OFF &
					ADC_SAMPLES_PER_INT_4,
				ADC_SAMPLE_TIME_31 &
					ADC_CONV_CLK_SYSTEM &
					ADC_CONV_CLK_32Tcy,
				ENABLE_AN4_ANA &
					ENABLE_AN5_ANA &
					ENABLE_AN6_ANA &
					ENABLE_AN8_ANA &
					ENABLE_AN15_ANA,
				(BIT4|BIT5|BIT6|BIT8|BIT15)	//errata requires ch15 ???
			);


	//**************************************************************************
	//* Setup the 12-Bit Analog to Digital Converter
	//**************************************************************************

    ADCON1bits.ADON = 0;
    channel = ADC_CH0_POS_SAMPLEA_AN4 &
              ADC_CH0_POS_SAMPLEA_AN5 &
              ADC_CH0_POS_SAMPLEA_AN6 &
              ADC_CH0_POS_SAMPLEA_AN8;

    SetChanADC12(channel);
	ConfigIntADC12(ADC_INT_DISABLE);
    adc12cfg_1 = ADC_MODULE_ON &
				 ADC_IDLE_CONTINUE &
				 ADC_FORMAT_INTG &
				 ADC_CLK_MANUAL &
				 ADC_AUTO_SAMPLING_OFF;
				 //ADC_SAMP_ON;
	adc12cfg_2 = ADC_VREF_AVDD_AVSS &
				 ADC_SCAN_ON &
	             ADC_SAMPLES_PER_INT_16 &
				 ADC_ALT_BUF_OFF &
				 ADC_ALT_INPUT_OFF;
	adc12cfg_3 = //ADC_SAMPLE_TIME_10 &
                 ADC_CONV_CLK_INTERNAL_RC;
                 //ADC_CONV_CLK_13Tcy;
	adc12cfg_p = ENABLE_AN4_ANA &
				 ENABLE_AN5_ANA &
				 ENABLE_AN6_ANA;
				 //ENABLE_AN8_ANA;
	adc12cfg_s = SKIP_SCAN_AN0 &
                 SKIP_SCAN_AN1 &
                 SKIP_SCAN_AN2 &
                 SKIP_SCAN_AN3 &
                 //SKIP_SCAN_AN4 &
                 //SKIP_SCAN_AN5 &
                 //SKIP_SCAN_AN6 &
                 SKIP_SCAN_AN7 &
                 SKIP_SCAN_AN8 &
                 SKIP_SCAN_AN9 &
                 SKIP_SCAN_AN10 &
                 SKIP_SCAN_AN11 &
                 SKIP_SCAN_AN12 &
                 SKIP_SCAN_AN13 &
                 SKIP_SCAN_AN14 &
                 SKIP_SCAN_AN15 ;

 	OpenADC12(adc12cfg_1,adc12cfg_2,adc12cfg_3,adc12cfg_p,adc12cfg_s);
 


	//**************************************************************************
	//* Setup the 12-Bit Analog to Digital Converter
	//**************************************************************************



	ADCON1bits.ADON = 0; /* turn off ADC */
	Channel = ADC_CH0_POS_SAMPLEA_AN4 &
			ADC_CH0_NEG_SAMPLEA_NVREF &
			ADC_CH0_POS_SAMPLEB_AN2&
			ADC_CH0_NEG_SAMPLEB_AN1;

	SetChanADC12(Channel);

	ConfigIntADC12(ADC_INT_DISABLE);

	PinConfig = ENABLE_AN4_ANA;

	Scanselect = SKIP_SCAN_AN2 & SKIP_SCAN_AN5 &
		SKIP_SCAN_AN9 & SKIP_SCAN_AN10 &
		SKIP_SCAN_AN14 & SKIP_SCAN_AN15 ;

	Adcon3_reg = ADC_SAMPLE_TIME_10 &
		ADC_CONV_CLK_SYSTEM &
		ADC_CONV_CLK_13Tcy;

	Adcon2_reg = ADC_VREF_AVDD_AVSS &
		ADC_SCAN_OFF &
		ADC_ALT_BUF_OFF &
		ADC_ALT_INPUT_OFF &
		ADC_SAMPLES_PER_INT_16;

	Adcon1_reg = ADC_MODULE_ON &
		ADC_IDLE_CONTINUE &
		ADC_FORMAT_INTG &
		ADC_CLK_MANUAL &
		ADC_AUTO_SAMPLING_OFF;

	OpenADC12(Adcon1_reg, Adcon2_reg, Adcon3_reg,PinConfig, Scanselect);




#endif 
}

#ifdef NEW_CODE 
//****************************************************************************************
//* ScaleSensorReading - This functions maps selected raw sensor readings scaled sensor readings   
//* in order to use their widest dynamic range.
//****************************************************************************************

void  ScaleSensorReading(void)
{
  
   // Scale the sensor values  

   SensorRecord.CurrentSensorValue[SensorID]  = 
     ((float) (SensorTable[SensorID].RawSensorValue - SensorTable[SensorID].MinSensorValue) / 
      SensorTable[SensorID].SensorGain +  SensorTable[SensorID].WindowMin);
 
}
 
#endif
