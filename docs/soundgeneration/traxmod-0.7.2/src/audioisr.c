// -*- tab-width: 4 -*-
// Audio Player ISRs
// Copyright (c) 2006-2008, K9spud LLC.
// http://www.k9spud.com/traxmod/
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <proc/p32mx320f128h.h>
#include <p32xxxx.h>
#include <plib.h> //PIC32 peripheral library     

//#include "lpc2103.h"
#include "audio.h"
//#include "mmc.h"
#include "main.h"
//#include "modplay/mixer.h"
#include "picspi.h"

volatile eint32* playPointer = 0;
volatile unsigned int playingFrame = 0;


volatile int faultStatus;
volatile int rxStatus;
volatile int txStatus;

/**
 * SPI1 interrupt service routine. Gets called each time the SSP1BUF
 * register is empty and needs the next audio sample to be loaded.
 */
volatile euint32 playSample = DAC_ZERO_VOLTS;
volatile euint32 leftSample = DAC_ZERO_VOLTS;
volatile euint32 rightSample = DAC_ZERO_VOLTS;
volatile int leftRight = 0;
volatile unsigned int playMode = PLAYMODE_IDLE;
volatile unsigned int ringCounter = 0;
volatile unsigned int junkRead1;
volatile unsigned int junkRead2;

void volatile __ISR(_SPI1_VECTOR, ipl7) DAC32_ISR(void)
{
	SPI1BUF = playSample;
/*	if(SPI1STAT & (1<<0))	
	{
		// SPI receive buffer full
		junkRead1 = SPI1BUF;
		if(junkRead1 & (0x80000000 >> 9))
		{
			
		}
	}*/

	IFS0CLR = SPI1TXIF; 	// clear SPI1 interrupt flags before exiting the service routine.

	switch(playMode)
	{
		default:
		case PLAYMODE_IDLE:
			if(leftRight == 0)
			{
				playSample = leftSample;
				leftRight = 1;
			}
			else
			{
				playSample = rightSample;
				leftRight = 0;
			}
			break;						// idle, waiting for data

		case PLAYMODE_STARTUP:
			if(playSample != 0)
			{
				playSample <<= 1;
				if((playSample <= 0x00010000) && (playSample >= 0xFFFF0000))
				{
					playSample = 0;
				}
				else
				{
					playSample += 0x10000;
				}
				playSample >>= 1;
				//rightSample = leftSample = playSample;
			}
			break;

		case PLAYMODE_SHUTDOWN:
			if(playSample != (0x80000000 >> 1))
			{
				playSample <<= 1;
				if((playSample <= 0x80010000) && (playSample >= 0x7FFF0000))
				{
					playSample = 0x80000000;
				}
				else
					{
					playSample -= 0x10000;
				}
				playSample >>= 1;
				//rightSample = leftSample = playSample;
			}
			break;

		case PLAYMODE_GO:
			playSample = *playPointer++;
			if(leftRight == 0)
			{
				leftSample = playSample;
			}
			else
			{
				rightSample = playSample;
			}
			leftRight ^= 1;

			if(playPointer >= audioBufferEnd)
			{
				playPointer = audioBuffer;
			}
			break;

		case PLAYMODE_RAMP:
			if(playSample > *playPointer)
			{
				playSample -= (1<<15);
			}
			else
			{
				playSample += (1<<15);
			}
			//rightSample = leftSample = playSample;

			if((playSample >> 15) == ((*playPointer) >> 15))
			{
				playMode = PLAYMODE_GO;
			}
			break;

		case PLAYMODE_STOP:
			if(playSample > DAC_ZERO_VOLTS)
			{
				playSample -= (1<<15);
			}
			else
			{
				playSample += (1<<15);
			}
			//rightSample = leftSample = playSample;
			if((playSample >> 15) == ((*playPointer) >> 15))
			{
				playMode = PLAYMODE_IDLE;
			}
			break;
	}

	ringCounter++;
}

void __attribute__((nomips16)) EnableInts(void)
{
    asm volatile("ei");
}

void __attribute__((nomips16)) DisableInts(void)
{
    asm volatile("di");
}

void SleepProcessor(void)
{
	//asm volatile("wait" ); // put device in selected Power-Saving mode
}

void SwitchHighClockSpeed(void)
{
    asm volatile("di");
	SYSKEY = 0xAA996655; // Write Key1 to SYSKEY
	SYSKEY = 0x556699AA; // Write Key2 to SYSKEY
	// OSCCON is now unlocked
	OSCCONCLR = (3 << 19) |		// set PB divisor to minimum (1:1)
				(7 << 27) |		// clear pll output divider 
				(7 << 16); 		// clear pll multiplier
	OSCCONSET = //(1 << 27) |		// set pll output divide by 2
				5 << 16;		// set pll multiplier x18
    asm volatile("ei");

	// Relock the SYSKEY
	SYSKEY = 0x12345678; // Write any value other than Key1 or Key2
	// OSCCON is relocked
}

void SwitchLowClockSpeed(void)
{
    asm volatile("di");
	SYSKEY = 0xAA996655; // Write Key1 to SYSKEY
	SYSKEY = 0x556699AA; // Write Key2 to SYSKEY
	// OSCCON is now unlocked
	OSCCONCLR = (7 << 27) | 	// clear pll output divider
				(7 << 16); 		// clear pll multiplier
	OSCCONSET = (1 << 27) |		// set pll output divide by 2
				(3 << 16) |		// mult by 15
				(3 << 19); 		// set PB divisor to maximum (1:8)
    asm volatile("ei");
	
	// Relock the SYSKEY
	SYSKEY = 0x12345678; // Write any value other than Key1 or Key2
	// OSCCON is relocked
}

void ExternalClockPLL(void)
{
    asm volatile("di");
	SYSKEY = 0x12345678; // write invalid key to force lock
	SYSKEY = 0xAA996655; // Write Key1 to SYSKEY
	SYSKEY = 0x556699AA; // Write Key2 to SYSKEY
	// OSCCON is now unlocked
	// make the desired change
	// This can be in ?C? or assembly

	OSCCONCLR = 0x3 << 19; // set PB divisor to minimum (1:1)
	OSCCONCLR = 7 << 27; // clear pll output divider
	OSCCONSET = 0 << 27; // set pll output divide by 2
	OSCCONCLR = 7 << 16; // clear pll multiplier
	OSCCONSET = 3 << 16; // clear pll multiplier x3

	OSCCONCLR = 7 << 8; // clear the new osc selection bits
	OSCCONSET = 3 << 8; // set the new osc selection value (Primary OSC with PLL)
	OSCCONSET = 1; // request clock switch
	
	// Relock the SYSKEY
	SYSKEY = 0x12345678; // Write any value other than Key1 or Key2
	// OSCCON is relocked
    asm volatile("ei");
}
