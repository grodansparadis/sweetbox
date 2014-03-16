//********************************************************************************
//*           Name:  DriverAlertPorts.h                                          *
//* Project Number:  MT2192                                                      *                                                   
//*           Date:  10/15/2007                                                  *                                                  
//*                                                                              *
//*   Description:                                                               * 
//*     This file includes the port definitions used for the Driver Alert System * 
//*                                                                              * 
//*                                                                              *
//********************************************************************************

#ifndef DRIVER_ALERT_PORTS  /* prevent multiple loading */
#define DRIVER_ALERTS_PORTS

// Port  Direction (0=output, 1=input)

// Define Port A bit definitions

#define X_Y_ACCELEROMETER_ST          PORTAbits.RA12      	// X,Y accelerometer MMA6261Q Module start test (output)
#define X_Y_ACCELEROMETER_ST_DIR      TRISAbits.TRISA12    	// X,Y accelerometer MMA6261Q Module start test (output)
 
#define X_Y_ACCELEROMETER_STATUS	    PORTAbits.RA13    	// X,Y accelerometer MMA6261Q Module status (input)
#define X_Y_ACCELEROMETER_STATUS_DIR	TRISAbits.TRISA13   // X,Y accelerometer MMA6261Q Module status (input)
 

//#define Z_ACCELEROMETER_ST          PORTAbits.RA14      	// Z accelerometer MMA1260D Module start test (output)
//#define Z_ACCELEROMETER_ST_DIR      TRISAbits.TRISA14      	// Z accelerometer MMA1260D Module start test (output)
 
#define Z_ACCELEROMETER_STATUS	    PORTAbits.RA15       	// Z accelerometer MMA1260D Module status (input)
#define Z_ACCELEROMETER_STATUS_DIR	TRISAbits.TRISA15     	// Z accelerometer MMA1260D Module status (input)


// Define Port B bit definitions
#define BUSY_FLAG PORTBbits.RB3					 	 	 	//  (0=Not Busy, 1=Busy)
#define BUSY_FLAG_DIR TRISBbits.TRISB3				 	 	//  Direction (0=output, 1=input)

// Define Port D bit definitions

#define Z_ACCELEROMETER_ST          PORTDbits.RD0      		// Z accelerometer MMA1260D Module start test (output)
#define Z_ACCELEROMETER_ST_DIR      TRISDbits.TRISD0      	// Z accelerometer MMA1260D Module start test (output)

#endif


