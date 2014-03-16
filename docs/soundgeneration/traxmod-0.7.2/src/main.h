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

#ifndef MAIN_H
#define MAIN_H

#ifndef NULL
#define NULL 0 
#endif

#define rprintf(...)
#define snprint(...)

// MMC/SD Card
// RE7
#define MMC_SS_PIN	(1<<7)

// Audio DAC
// RG3,2, 6, 8
#define DAC_PD_PIN (1<<3)
#define DAC_MS_PIN (1<<2)
#define DAC_MC_PIN (1<<6)	// shared with MMC/SD Card SDSCK signal
#define DAC_MD_PIN (1<<8)	// shared with MMC/SD Card SDSDI signal

// RF2,3,6
#define DAC_LRCK_PIN (1<<2)
#define DAC_SDO_PIN (1<<3)
#define DAC_SCK_PIN (1<<6)

// RG
#define MMC_SCK_PIN (1<<6)
#define MMC_SDI_PIN (1<<7)
#define MMC_SDO_PIN (1<<8)

#define OFF_SWITCH (1<<9)

// LEDs
// PORTD
#define LED1 (1<<0)
#define LED2 (1<<1)
#define LED3 (1<<2)
#define LED4 (1<<3)
#define LED5 (1<<11)

// Buttons
// RC13/14
#define BTN1 (1<<14)
#define BTN2 (1<<13)

// RD4..7
#define BTN3 (1<<4)
#define BTN4 (1<<5)
#define BTN5 (1<<6)
#define BTN6 (1<<7)

// RB15
#define BTN7 (1<<15)

#define BTNDEBOUNCE 25
#define BTNHOLDWAIT 15000
#define BTNHOLD		3000
#define BTNHOLDSLOW	7000

#define BTNWAIT		50000
#define BTNREPEAT	2000

#include "efsl/efs.h"

extern EmbeddedFileSystem efs;
extern EmbeddedFile fp;

extern int bExit;
extern euint16 junkRead;

// File Types
#define UNKNOWNFILE 0
#define MODFILE 	1
#define RAWFILE 	2

typedef struct
{	
	euint32 FileListJump;
	euint8 MasterVolume;
} TraxmodConfigType;
extern TraxmodConfigType config;


void doIdle(void);
int getFiletype(char* fileName);
euint32 file_getAddress(File *file, euint32 offset);
void shortWait(void);
void wait(void);
void __attribute__((nomips16)) EnableInts(void);
void __attribute__((nomips16)) DisableInts(void);
void SleepProcessor(void);
void SwitchHighClockSpeed(void);
void SwitchLowClockSpeed(void);
void ExternalClockPLL(void);

#endif
