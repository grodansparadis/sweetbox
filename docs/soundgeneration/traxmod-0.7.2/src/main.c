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

#include <p32xxxx.h>

#include "efsl/efs.h"
#include "efsl/types.h"
#include "efsl/ls.h"

#include "../Generated Files/VDIInit/init_PIC32MX320F128H.sinit_vi.h"

#include "main.h"
#include "audio.h"
#include "picspi.h"
#include "rawplay.h"

EmbeddedFileSystem efs;
EmbeddedFile fp;

TraxmodConfigType config;

int bExit = 0;
int repeatMode = REPEAT_OFF;
int restartDirectoryList;

euint16 junkRead; // temporary storage for dummy reads from SPI data register

int buttonWait = 1000;

const char *playDirectory = "/"; // "/MOD/";

void init(void);

int main(void)
{
	int i;
	VisualInitialization();
	config.MasterVolume = 63;
	config.FileListJump = 0;

	init();
	PORTD = LED4;
	euint8 result = efs_init(&efs, 0);
	audioInit();

	PORTD = LED2;
	if(result == 0) 
	{
		DirList list;

		result = 0;
		
		if ( file_fopen(&fp, &efs.myFs, "/TRAXMOD.CFG", 'r') == 0 ) 
		{
			// Found a TRAXMOD config file, read it in!
			file_read( &fp, sizeof(TraxmodConfigType), (unsigned char*)&config );
			file_fclose( &fp );
			
			rprintf("Read TRAXMOD.CFG Vol: %d File: %d\n", config.MasterVolume, config.FileListJump);
			filterFlags |= FILTER_DIRTY;
			restartDirectoryList = 1;
		}		

		while(1)
		{
			ls_openDir(&list, &(efs.myFs) , playDirectory);
			if(restartDirectoryList);
			{
				restartDirectoryList = 0;	
				for(i = 0; i < config.FileListJump; i++)
				{
					if(ls_getNext(&list) != 0)
					{
						ls_openDir(&list, &(efs.myFs) , playDirectory);
					}
				}
			}

			while((restartDirectoryList == 0) && (ls_getNext(&list) == 0))
//			while(ls_getNext(&list) == 0)
			{
				int fileType = getFiletype(list.currentEntry.FileName);
				config.FileListJump++;

				if(fileType == UNKNOWNFILE)
				{
					rprintf("Skipping %d - [", config.FileListJump - 1);
					snprint(list.currentEntry.FileName, LIST_MAXLENFILENAME);
//					uart0Putch(']');
				}
				else
				{
					rprintf("Playing %d - [", config.FileListJump - 1);
					snprint(list.currentEntry.FileName, LIST_MAXLENFILENAME);
					//uart0Puts("]\r\n");

					switch(fileType)
					{
						case RAWFILE:
							playRAW(list.currentEntry.FileName);
							break;
						
						case MODFILE:
							//playMOD(list.currentEntry.FileName);
							break;
					}

					if(file_fopen(&fp, &efs.myFs, "/TRAXMOD.CFG", 'w') == 0)
					{
						file_fwrite( &fp, 0, sizeof(TraxmodConfigType), (unsigned char*)&config );
						file_fclose( &fp );			
						rprintf("Wrote TRAXMOD.CFG Vol: %d File: %d\n", config.MasterVolume, config.FileListJump);						
					}
				    else if(file_fopen(&fp, &efs.myFs, "/TRAXMOD.CFG", 'a') == 0)
					{
						file_fwrite( &fp, 0, sizeof(TraxmodConfigType), (unsigned char*)&config );
						file_fclose( &fp );			
						rprintf("Overwrote TRAXMOD.CFG Vol: %d File: %d\n", config.MasterVolume, config.FileListJump);											
					}
				}

				//uart0Puts("\r\n");
			}
			
			if(restartDirectoryList == 0)
			{
				config.FileListJump = 0;
			}
		}
			
		// execution never reaches here
		// fs_umount( &efs.myFs ) ;

		for(;;)
		{
			PORTD = LED1;
			wait();
			PORTD = LED2;
			wait();
			PORTD = LED3;
			wait();
			PORTD = LED4;
			wait();
		}
	}
	else
	{
		for(;;)
		{
			PORTD = LED1;
			wait();
			PORTD = LED4;
			wait();
		}
	}
}

#define DISPLAYMAX 500000
int displayCount = DISPLAYMAX;
int displayLed = 0;
int masterVolumeRamp = -1;
int poweringDown = 0;
void doIdle(void)
{
	static int button1 = 0, button2 = 0, button3 = 0, button4 = 0, button5 = 0, button6 = 0, button7 = 0;

	if((PORTG & OFF_SWITCH) == 0)
	{
		// uh-oh, we're powering down!
		poweringDown = 1;
/*
		if(playMode == PLAYMODE_SHUTDOWN)
		{
			if(playSample == DAC_MINIMUM)
			{
				PORTD = LED1|LED4;
				TRISGCLR = DAC_PD_PIN;
				PORTGCLR = DAC_PD_PIN;	// put DAC in power down (reset)
			}
		}
		else
		{
			playMode = PLAYMODE_SHUTDOWN;
*/
			PORTD = LED1|LED2|LED3|LED4;
//		}
	}
	else
	{
/*		if(poweringDown && playSample == DAC_MINIMUM)
		{
			poweringDown = 0;
			PORTD = LED2|LED3;
			audioInit();	
			audioStart();
			//PORTGSET = DAC_PD_PIN;	// release DAC from power down (reset)
			//DACControl(0x400);		// pull DAC out of soft power down
			//playMode = PLAYMODE_GO;
			//filterFlags |= FILTER_DIRTY;
		}*/
	}

	//GRAPHDBG('i');		
	if(filterFlags & FILTER_DIRTY)
	{
		if((SPI2STAT & SPIBUSY) == 0)
		{
			// MMC/SD Card is not busy using the SPI2 interface, let's update the DAC registers.
			DACControl(0x100 | config.MasterVolume);
			DACControl(0x200 | config.MasterVolume);
			//DACControl(0x305 | (filterFlags & FILTER_DACMASK));
			filterFlags &= ~FILTER_DIRTY;
		}
	}

	if(PORTC & BTN1)
	{
		button1 = 0;
		PORTDCLR = LED1;
	}
	else
	{
		button1 += ringCounter;
		if(button1 > (BTNDEBOUNCE + BTNHOLDWAIT + BTNHOLDSLOW))
		{
			button1 -= BTNHOLDSLOW;

			PORTD = LED1;

			if(config.FileListJump >= 2)
			{
				config.FileListJump -= 2;
				restartDirectoryList = 1;
				bExit = 1;
			}
		}
		else if((button1 > BTNDEBOUNCE) && ((button1 - ringCounter) <= BTNDEBOUNCE))
		{
			PORTD = LED1;

			if(config.FileListJump >= 2)
			{
				config.FileListJump -= 2;
				restartDirectoryList = 1;
				bExit = 1;
			}
		}
	}
	
	if(PORTC & BTN2)
	{
		button2 = 0;
	}
	else
	{
		button2 += ringCounter;
		if(button2 > (BTNDEBOUNCE + BTNHOLDWAIT + BTNHOLD))
		{
			button2 -= BTNHOLD;
			volumeUp();
		}
		else if((button2 > BTNDEBOUNCE) && ((button2 - ringCounter) <= BTNDEBOUNCE))
		{
			volumeUp();
		}
	}

	if(PORTD & BTN3)
	{
		button3 = 0;
	}
	else
	{
		button3 += ringCounter;
		if(button3 > (BTNDEBOUNCE + BTNHOLDWAIT + BTNHOLD))
		{
			button3 -= BTNHOLD;
			volumeDown();
		}
		else if((button3 > BTNDEBOUNCE) && ((button3 - ringCounter) <= BTNDEBOUNCE))
		{
			volumeDown();
		}
	}
	
	if(PORTD & BTN4)
	{
		button4 = 0;
	}
	else
	{	
		button4 += ringCounter;
		if(button4 > (BTNDEBOUNCE + BTNHOLDWAIT + BTNHOLDSLOW))
		{
			button4 -= BTNHOLDSLOW;
			PORTD = LED4;
			bExit = 1;
		}
		else if((button4 > BTNDEBOUNCE) && ((button4 - ringCounter) <= BTNDEBOUNCE))
		{
			PORTD = LED4;
			bExit = 1;
		}
	}

	if(PORTD & BTN5)
	{
		button5 = 0;
	}
	else
	{	
		button5 += ringCounter;
		if((button5 > BTNDEBOUNCE) && ((button5 - ringCounter) <= BTNDEBOUNCE))
		{
			if(repeatMode == REPEAT_OFF)
			{
				PORTD = LED2|LED3;
				repeatMode = REPEAT_SONG;
			}
			else
			{
				PORTD = LED2;
				repeatMode = REPEAT_OFF;
			}
		}
	}

	if(PORTD & BTN6)
	{
		button6 += ringCounter;
	}
	else
	{	
		if(button6 >= BTNHOLD)
		{
			PORTD = LED5;
			audioPause();
			button6 = 0;
			displayCount = DISPLAYMAX - (DISPLAYMAX >> 6);
		}
	}

	if(PORTB & BTN7)
	{
		button7 = 0;
	}
	else
	{
		button7 += ringCounter;
		if(button7 > (BTNDEBOUNCE + BTNHOLDWAIT + BTNHOLD))
		{
		}
		else if((button7 > BTNDEBOUNCE) && ((button7 - ringCounter) <= BTNDEBOUNCE))
		{
			// uh-oh, we're powering down!
			PORTD = LED1|LED2|LED3|LED4;
			poweringDown = 1;
			DACControl(0x100 | 63);
			DACControl(0x200 | 63);
			playMode = PLAYMODE_IDLE;
		}
	}

	displayCount += ringCounter;
	if(displayCount > DISPLAYMAX)
	{
		displayCount = 0;
		switch(displayLed++)
		{
			case 0:
				PORTD = LED5;
				displayCount = DISPLAYMAX - (DISPLAYMAX >> 6);
				break;

			case 1:
				PORTD = 0;
				displayCount = 0;
				break;

			case 2:
				break;

			case 3:
			default:
				displayLed = 0;
		}
	}

	ringCounter = 0;
	while(ringCounter == 0);
	SleepProcessor();
	// code execution will resume here after wake
}

int getFiletype(char* fileName)
{
	char e1 = fileName[LIST_MAXLENFILENAME-3];
	char e2 = fileName[LIST_MAXLENFILENAME-2];
	char e3 = fileName[LIST_MAXLENFILENAME-1];

	if(e1 == 'R' && e2 == 'A' && e3 == 'W')
	{
		// filename ends with RAW, assume raw wave audio file.
		return RAWFILE;
	}
	
	if(e1 == 'M' && e2 == 'O' && e3 == 'D')
	{
		// filename ends with MOD, assume MOD file.
		return MODFILE;
	}

	e1 = fileName[0];
	e2 = fileName[1];
	e3 = fileName[2];
	if(e1 == 'M' && e2 == 'O' && e3 == 'D')
	{
		// filename begins with MOD, assume it's an Amiga named MOD file.
		return MODFILE;
	}
	
	return UNKNOWNFILE;
}


/**
 * Takes an EFSL file handle and offset, figures out the real MMC/SD Card physical
 * address of the given file+offset, and returns the physical address.
 */
euint32 file_getAddress(File *file, euint32 offset)
{
	euint32 coffset = offset;
	euint32 cclus, csec, cbyte;
	euint32 rclus, rsec;
		
	if(!file_getAttr(file, FILE_STATUS_OPEN))
	{
		return(0);
	}
	
	if(offset >= file->FileSize)
	{
		return(0);
	}
	
	cclus = coffset / (512 * file->fs->volumeId.SectorsPerCluster);
	csec = (coffset / (512)) % file->fs->volumeId.SectorsPerCluster;
	cbyte = coffset % 512;
		
	if((fat_LogicToDiscCluster(file->fs, &(file->Cache), cclus)) != 0)
	{
		return(0);
	}

	rclus = file->Cache.DiscCluster;
	rsec = fs_clusterToSector(file->fs, rclus);
		
	return part_getRealLBA(file->fs->part, rsec+csec);
}

void wait(void)
{
	int i;

	for(i = 0; i < 5555555; i++)
	{
		asm volatile("nop"	"\n\t" : : );
	}
}

void shortWait(void)
{
	int i;
	for(i = 0; i < 1000; i++)
	{
		asm volatile("nop" 		"\n\t"
					 : /* output operands */ : /*input operands */);
	}
}

#define INTCON_MVEC	(1<<12)
void init(void)
{
	// Enable prefetching cacheable code and access FLASH with only 2 wait states 
	// (30MHz FLASH re-specified by Microchip)
	CHECON = (2<<_CHECON_PFMWS_POSITION)| (1<<_CHECON_PREFEN_POSITION);

	PORTESET = MMC_SS_PIN;	// de-assert MMC/SD card
	PORTGSET = DAC_MS_PIN | DAC_MC_PIN;		// de-select DAC chip select, DAC control port clock high (idle)
	PORTGCLR = DAC_PD_PIN | DAC_MD_PIN;		// hold DAC in power down (reset) mode

	// define output pins
	TRISGCLR = DAC_PD_PIN | DAC_MS_PIN | DAC_MC_PIN | DAC_MD_PIN;
	TRISFCLR = DAC_SDO_PIN;

	TRISGCLR = MMC_SCK_PIN | MMC_SDO_PIN;
	TRISECLR = MMC_SS_PIN;

	// define input pins
	TRISGSET = MMC_SDI_PIN;
	CNPUESET = (1<<9) | (1<<11) | // enable pull up resistor on CN9/SDI2, CN11/POWER_OFF,
		(1<<0)|(1<<1)|(1<<12)|(1<<13)|(1<<14)|(1<<15)|(1<<16); // CN0/BTN1, CN1/BTN2, CN12/BTN7, CN13/BTN3, CN14/BTN4, CN15/BTN5, CN16/BTN6

	SYSKEY = 0xAA996655; // Write Key1 to SYSKEY
	SYSKEY = 0x556699AA; // Write Key2 to SYSKEY
	// OSCCON is now unlocked
	OSCCONSET = 0x10; 			// set Power-Saving mode to Sleep
	SYSKEY = 0x12345678; // Write any value other than Key1 or Key2
	// OSCCON is relocked

    INTCONSET = INTCON_MVEC;	// use multivector interrupt handling
	EnableInts();
}
