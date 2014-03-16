// -*- tab-width: 4 -*-
// Audio Player
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

#ifndef AUDIO_H
#define AUDIO_H

#include "efsl/types.h"

#define AUDIO_BUFFER_SIZE audioSize /*(audioBuffer[-1])*/

extern euint16 audioSize;
extern eint16* audioBuffer;
extern eint16* audioBufferEnd;

// Our internal software flags:
#define FILTER_16BIT		(1<<6)
#define FILTER_ALIGNEXACT	(1<<2)
#define FILTER_ALIGNMENT	(1<<1)
#define FILTER_DIRTY		(1<<0)

// Hardware DAC Filter bits:
#define FILTER_OVERSAMPLE 	(1<<7)
#define FILTER_INVERT		(1<<5)
#define FILTER_AMIX			(1<<4)
#define FILTER_DEM			(1<<3)
#define FILTER_DACMASK		((1<<7)|(1<<5)|(1<<4)|(1<<3))
extern unsigned char filterFlags;

#define DAC_ZERO_VOLTS 0x80000000
#define DAC_MINIMUM (0x80000000 >> 1)	// minimum voltage
#define DAC_MAXIMUM (0x7FFFFFFF >> 1)	// maximum voltage

extern unsigned int iPlayFrequency;

extern volatile unsigned int ringCounter;
extern volatile eint32* playPointer;
extern volatile unsigned int playingFrame;
extern volatile euint32 playSample;
extern volatile unsigned int playMode;

#define PLAYMODE_IDLE 0
#define PLAYMODE_GO	  1
#define PLAYMODE_RAMP 2
#define PLAYMODE_STOP 3
#define PLAYMODE_STARTUP 4
#define PLAYMODE_SHUTDOWN 5

extern int repeatMode;
#define REPEAT_OFF 	0
#define REPEAT_SONG 1

//extern volatile unsigned int iPlayPtr;

//void __attribute__ ((interrupt("IRQ"))) DAC32_ISR(void);
//void __attribute__ ((interrupt("IRQ"))) DAC16_ISR(void);
//void __attribute__ ((interrupt("IRQ"))) DAC_Silent_ISR(void);
//void __attribute__ ((interrupt ("FIQ"))) DAC_FIQ(void);

void audioInit(void);
void audioFrequency(unsigned int iFreq);
void audioStart(void);
void audioStop(void);
void audioPause(void);
void volumeDown(void);
void volumeUp(void);

void DACControl(unsigned int value);

#endif // AUDIO_H
