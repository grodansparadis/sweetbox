// -*- tab-width: 4 -*-
// TRAXMOD Digital Audio Player
//
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

//#include "lpc_config.h"
#include <p32xxxx.h>

#include "main.h"
#include "audio.h"
#include "picuart.h"
#include "picspi.h"

//#include "mmc.h"

//#include "lpc2000_dbg_printf.h"
//#define rprintf lpc2000_debug_printf

eint16* audioBuffer;
eint16* audioBufferEnd;
euint16 audioSize;
unsigned int iPlayFrequency;	

unsigned char filterFlags;

// SPI0 (all on PINSEL0)
#define ENABLE_SPI0_SCK0 	(1<<8)
#define MASK_SPI0_SCK0 		(3<<8)

#define ENABLE_SPI0_MISO0 	(1<<10)
#define MASK_SPI0_MISO0 	(3<<10)

#define ENABLE_SPI0_SSEL0 	(1<<14)
#define MASK_SPI0_SSEL0		(3<<14)

void audioInit(void)
{	
	PORTGSET = DAC_MS_PIN | DAC_MC_PIN;		// de-select DAC chip select, DAC control port clock high (idle)
	PORTGCLR = DAC_PD_PIN | DAC_MD_PIN;		// hold DAC in power down (reset) mode

	playPointer = audioBuffer;
	playSample = DAC_MINIMUM;
	playMode = PLAYMODE_STARTUP;

	// Configure SPI1 / DAC Audio Data Interface
	IEC0CLR = SPI1EIE | SPI1RXIE | SPI1TXIE; 		// disable SPI1 interrupts for now.
	SPI1CON = 0; 									// Stops and resets the SPI1.
	SPI1STAT = 0;
	IFS0 = 0; 										// clear any existing event
	IPC5CLR = 0x1f000000; 							// clear the priority
	IPC5SET = (7<<26) | (2<<24); 					// Set interrupt priority to level 7, subpriority 2
	IEC0 = SPI1TXIE;								// Enable transmit buffer empty interrupt
	IEC1 = 0;
	SPI1BRG = 13; 									// use FPB/(2 * (BRG + 1) = 64Fs DAC clock frequency
	SPI1STATCLR = 0x40; 							// clear the Overflow
	SPI1CON = SPIMODE32 | SPICKE | SPICKP;// | SPICKP; // SPI On, 32 bit transfers, slave mode, idle high clock polarity

//	junkRead = SPI1BUF; // clears the receive buffer
//	SPI1BUF = DAC_MINIMUM;		// start with silence
	SPI1CON = SPION | SPIMODE32;//| SPIMSTEN; // SPI On, 32 bit transfers, master mode
	SPI1BUF = DAC_MINIMUM;		// start with silence
	SPI1BUF = DAC_MINIMUM;		// start with silence
	
	DACControl(0x305);		// put DAC in master mode
	PORTGSET = DAC_PD_PIN;	// release DAC from power down (reset)
	shortWait();
	DACControl(0x305);		// put DAC in master mode

	while(playSample != 0);
	DACControl(0x100 | config.MasterVolume);
	DACControl(0x200 | config.MasterVolume);
}

void DACControl(unsigned int value)
{
	unsigned int origIEC1 = IEC1;
	unsigned int origSPI2CON = SPI2CON;
	unsigned int origPORTE = PORTE;
	unsigned int origSPI2BRG = SPI2BRG;
	
	PORTESET = MMC_SS_PIN;								// de-select MMC/SD card
	TRISECLR = MMC_SS_PIN;
	PORTGCLR = DAC_MS_PIN;								// select DAC chip select

	IEC1CLR = SPI2EIE | SPI2RXIE | SPI2TXIE; 		// disable SPI2 interrupts for now.
	SPI2CON = 0; 		// Stops and resets the SPI2.
	junkRead = SPI2BUF; // clears the receive buffer
	SPI2BRG = 0; //63; 			// use FPB/128 clock frequency
	SPI2STATCLR = 0x40; 	// clear the Overflow
	SPI2CON = SPION | SPIMODE16 | SPIMSTEN; // SPI On, 16 bit transfers, master mode

	//shortWait();

	SPI2BUF = value;
	while((SPI2STAT & SPIRBF) == 0);	// wait until SPI read buffer is filled
	junkRead = SPI2BUF;					// read back junk data
	PORTGSET = DAC_MS_PIN;				// de-assert DAC slave select
	//shortWait();
		
	// restore all control registers
//	IFS1CLR = SPI2EIF | SPI2RXIF | SPI2TXIF; // clear any existing event
	SPI2BRG = origSPI2BRG;
	SPI2CON = origSPI2CON;
	IEC1 = origIEC1;
	PORTE = origPORTE;
}

void audioFrequency(unsigned int iFreq)
{
	iPlayFrequency = 44100;	// now that DAC is externally clocked, we only support 44.1KHz period.
}

void audioStart(void)
{
//	rprintf("[audioStart]\n");

	playPointer = audioBuffer;
	playMode = PLAYMODE_RAMP;
}

void audioStop(void)
{
	playMode = PLAYMODE_IDLE;
}

void audioPause(void)
{
	if(playMode == PLAYMODE_GO)
	{
		playMode = PLAYMODE_IDLE;
	}
	else
	{
		playMode = PLAYMODE_GO;
	}
}

void volumeDown(void)
{
	if(config.MasterVolume != 0)
	{
		config.MasterVolume--;
		filterFlags |= FILTER_DIRTY;
	}			
}

void volumeUp(void)
{
	if(config.MasterVolume != 0x3F)
	{
		config.MasterVolume++;
		filterFlags |= FILTER_DIRTY;
	}
}
