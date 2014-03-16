/*********************************************************************
 *
 *                  MS Timer
 *
 *********************************************************************
 * FileName:        mstimer.c
 * Processor:       PIC24 /  Daytona
 * Complier:        MPLAB C30/C32
 *
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement:
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the �Company�) for its dsPIC30F and PICmicro� Microcontroller is intended 
 * and supplied to you, the Company�s customer, for use solely and
 * exclusively on Microchip's dsPIC30F and PICmicro Microcontroller products. 
 * The software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN �AS IS� CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 * Author               Date      Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * <Too Shy to say>               Original Implementation 
 * Bud Caldwell         4/24/2008 Added 2nd timer counter
 *
 ********************************************************************/

#include "basicdriverdefs.h"
#include "peripheral\timer.h"
#include "mstimer.h"

static volatile unsigned int _MSTimer;
static volatile unsigned long _MSTimer32;


///////////////////////////////////////////////////////////////////////////////
// Interrupt Timer 1 handler
//
   
INTERRUPT( ipl2, _TIMER_1_VECTOR, Timer1Handler )
{
    // clear the interrupt flag
    mT1ClearIntFlag();
   
    // Update 1 ms timebase
    _MSTimer++;
    _MSTimer32++;
}  

/*********************************************************************
 * Function:       
 *
 * PreCondition:    
 *
 * Input:           
 *
 * Output:          
 *
 * Side Effects:    
 *
 * Overview:        
 *
 * Note:           
 ********************************************************************/
void MSTimerInit( unsigned int pbclock )
{
    unsigned int period;
    unsigned int i;
    
    period = pbclock / 1000;        // number of ticks to a ms;

	i = 0;
	
	while ( ( period > 0xFFFF ) && ( i < 7 ) )
	{
		period >>= 1;
		i++;
	}
	
    if ( i == 7 )
    {
        period = pbclock / 1000;
        period /= 256;
    }

	i <<= _T1CON_TCKPS_POSITION;
				  
    OpenTimer1( ( T1_ON | i ), period);
    
    ConfigIntTimer1( ( T1_INT_PRIOR_1 ) );

    _MSTimer = 0;
    _MSTimer32 = 0;
}


/*********************************************************************
 * Function:       
 *
 * PreCondition:    
 *
 * Input:           
 *
 * Output:          
 *
 * Side Effects:    
 *
 * Overview:        
 *
 * Note:           
 ********************************************************************/
void MSTimerClear(void)
{
    DisableIntT1;
    _MSTimer = 0;
    EnableIntT1;
}


/*********************************************************************
 * Function:       
 *
 * PreCondition:    
 *
 * Input:           
 *
 * Output:          
 *
 * Side Effects:    
 *
 * Overview:        
 *
 * Note:           
 ********************************************************************/
void MSTimer32Clear(void)
{
    DisableIntT1;
    _MSTimer32 = 0;
    EnableIntT1;
}


/*********************************************************************
 * Function:       
 *
 * PreCondition:    
 *
 * Input:           
 *
 * Output:          
 *
 * Side Effects:    
 *
 * Overview:        
 *
 * Note:           
 ********************************************************************/
unsigned int MSTimerGetTime( void )
{
    unsigned int time;

    DisableIntT1;
    time = _MSTimer;
    EnableIntT1;
    return time;
}


/*********************************************************************
 * Function:       
 *
 * PreCondition:    
 *
 * Input:           
 *
 * Output:          
 *
 * Side Effects:    
 *
 * Overview:        
 *
 * Note:           
 ********************************************************************/
unsigned long MSTimer32GetTime( void )
{
    unsigned long time;

    DisableIntT1;
    time = _MSTimer32;
    EnableIntT1;
    return time;
}


/*********************************************************************
 * Function:       
 *
 * PreCondition:    
 *
 * Input:           
 *
 * Output:          
 *
 * Side Effects:    
 *
 * Overview:        
 *
 * Note:           
 ********************************************************************/
void MSTimerWait( unsigned int ms_wait )
{
    MSTimerClear();

    while ( MSTimerGetTime() < ms_wait ) {
        ;
    }    
        
	DisableIntT1;	        	
}


/*********************************************************************
 * Function:       
 *
 * PreCondition:    
 *
 * Input:           
 *
 * Output:          
 *
 * Side Effects:    
 *
 * Overview:        
 *
 * Note:           
 ********************************************************************/
#pragma interrupt MSTimerIntHandler ipl1 vector 8
void MSTimerIntHandler( void )
{
    mT2ClearIntFlag();
    
}

