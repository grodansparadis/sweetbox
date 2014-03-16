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

#include "main.h"
#include "audio.h"
#include "malloc.h"
#define NULL 0
//#include "mmc.h"
#define iSD_BlockLength 512

//#include "interfaces/sd.h"
//#include "interfaces/lpc2000_dbg_printf.h"
#define rprintf(...)
#define snprint(...)
//#define rprintf lpc2000_debug_printf

#include "rawplay.h"

#define MULT 2
euint16 readBuffer[256*MULT];
eint16 buffer[2048*MULT];
//euint16* readBuffer;

euint16* (*PostProcessor)(euint32*) = NULL;

/*
euint32 sd_read(hwInterface* hw, euint32 iAddress, euint32 iLen, euint8* buf);
euint32 sd_begin_read(hwInterface* hw, euint32 iAddress, euint32 iLen, euint8* buf);

euint32 sd_read(hwInterface* hw, euint32 iAddress, euint32 iLen, euint8* buf)
{
	euint32 iRead = 0;

	while(iRead < iLen)
	{
		SD_ReadBlock(iAddress, buf, 512);
		iAddress++;
		buf 	 += 512;
		iRead    += 512;
	}
	
	return iRead;
}

euint32 sd_begin_read(hwInterface* hw, euint32 iAddress, euint32 iLen, euint8* buf)
{
	SD_BeginRead(iAddress*512, buf, 512);	
	return iLen;
}
*/


// Plays a RAW audio file using the FAT filesystem to initially
// find the first sector address of the file, then reads from
// the SD Card directly (no filesystem layer). Will play garbage
// on fragmented files.
void playRAW(char* fileName)
{
	bExit = 0;
	
	//readBuffer = malloc(iSD_BlockLength);

	audioSize = 1024*MULT;
	audioBuffer = buffer; //malloc(audioSize * sizeof(euint16));
	audioBufferEnd = &audioBuffer[audioSize];
	
	if ( file_ropen(&fp, &efs.myFs, fileName) == 0 ) 
	{
		euint32 iAddress, iEndAddress;
		unsigned short i;
		euint32 e;
		eint16* p;
		hwInterface* hw = fp.fs->part->disc->ioman->iface;
		
		iAddress = file_getAddress(&fp, 0);
//		iAddress = (iAddress / 512) * 512;
		iEndAddress = iAddress + (fp.FileSize / 512);
		rprintf("LBA: %d Size: %d\n", iAddress, fp.FileSize);
		
		//e = file_read( &fp, audioSize * sizeof(euint16), (euint8*)audioBuffer );
		p = audioBuffer;
		for(i = 0; i < 2; i++)
		{
			e = file_read( &fp, iSD_BlockLength*MULT, (euint8*)readBuffer);
			//e = sd_read(hw, iAddress, iSD_BlockLength, (euint8*)readBuffer);
			iAddress++;
			if(readBuffer[0] > 0x6000 && readBuffer[0] < 0xA000)
			{
				PostProcessor = &Unsigned16Bit;
			}
			else
			{
				PostProcessor = &Signed16Bit;			
			}
			p = (eint16*)PostProcessor((euint32*)p);
		}

		audioFrequency(44100);
		filterFlags &= ~FILTER_16BIT;
		//filterFlags |= FILTER_16BIT;

		audioStart();
//		printHeap("[rawplay]");
		i = 0;

		euint8 iNeedPostProcess = 0;
		euint16 leftSample = 0;
		euint16 rightSample = 0;
		while(bExit == 0) 
		{
//			while(iPlayPtr < (audioSize >> 1))
			while(playPointer < &audioBuffer[audioSize >> 1])
			{
				if(iNeedPostProcess)
				{
//					if(BytesLeftToRead == 0)
//					{
						SwitchHighClockSpeed();
						PostProcessor((euint32*)p);
						e = file_read( &fp, iSD_BlockLength*MULT, (euint8*)readBuffer);
						if(e != (iSD_BlockLength*MULT))
						{
							if(e >= 4)
							{
								leftSample = readBuffer[(e >> 1) - 2];
								rightSample = readBuffer[(e >> 1) - 1];
							}
							while(e < iSD_BlockLength*MULT)
							{
								readBuffer[(e >> 1)] = leftSample;
								e += 2;
								if(e >= (iSD_BlockLength*MULT))
									break;

								readBuffer[(e >> 1)] = rightSample;
								e += 2;
							}
						}

						SwitchLowClockSpeed();

//						SD_StopTransmission();
						iNeedPostProcess = 0;
//					}
				}
				doIdle();
			}

			p = audioBuffer;

			//e = sd_begin_read(hw, iAddress, iSD_BlockLength, (euint8*)readBuffer);
			iAddress++;
			iNeedPostProcess = 1;
			
//			while(iPlayPtr >= (audioSize >> 1))
			while(playPointer >= &audioBuffer[audioSize >> 1])
			{
//				rprintf("Feed: %d Read: %d\n", iSD_FeedBytes, iSD_ReadBytes);
				if(iNeedPostProcess)
				{
					//if(BytesLeftToRead == 0)
					//{
						SwitchHighClockSpeed();
						p = (eint16*)PostProcessor((euint32*)p);
						e = file_read( &fp, iSD_BlockLength*MULT, (euint8*)readBuffer);
						if(e != (iSD_BlockLength*MULT))
						{
							if(e >= 4)
							{
								leftSample = readBuffer[(e >> 1) - 2];
								rightSample = readBuffer[(e >> 1) - 1];
							}
							while(e < iSD_BlockLength*MULT)
							{
								readBuffer[(e >> 1)] = leftSample;
								e += 2;
								if(e >= (iSD_BlockLength*MULT))
									break;

								readBuffer[(e >> 1)] = rightSample;
								e += 2;
							}
						}
						SwitchLowClockSpeed();
						
					//	SD_StopTransmission();
						iNeedPostProcess = 0;
					//}
				}

				doIdle();				
			}


			//e = sd_begin_read(hw, iAddress, iSD_BlockLength, (euint8*)readBuffer);
			iAddress++;
			iNeedPostProcess = 1;				
			
//			if(iAddress >= iEndAddress)
			if(fp.FilePtr >= fp.FileSize)
			{
				if(repeatMode == REPEAT_OFF)
				{
/*					while(playPointer < &audioBuffer[audioSize >> 1])
					{
						if(iNeedPostProcess)
						{
							SwitchHighClockSpeed();
							PostProcessor((euint32*)p);
						}
					}					
*/					break;
				}
				else
				{
					fp.FilePtr = 0;
					iAddress = file_getAddress(&fp, 0);
				}
			}
		}

		// make sure to stop interrupt driven card read before accessing
		// card using other functions.
		//while(BytesLeftToRead);
		//SD_StopTransmission();
		
		file_fclose( &fp );
		audioStop();
		SwitchHighClockSpeed();
	}
	
//	free(audioBuffer);
//	free(readBuffer);
}

euint16* Unsigned16Bit(euint32* buf)
{
	unsigned int i = 0, iReadPtr = 0;
	unsigned int iSample;
	
	while(iReadPtr < ((iSD_BlockLength*MULT) >> 1))
	{
		iSample = readBuffer[iReadPtr++] - 0x8000;
	//	buf[i++] = 0;
		buf[i++] = iSample << 15;
	}
	return &(buf[i]);
}

euint16* Signed16Bit(euint32* buf)
{
	unsigned int i = 0, iReadPtr = 0;
	unsigned int iSample;
	
	while(iReadPtr < ((iSD_BlockLength*MULT) >> 1))
	{
		iSample = readBuffer[iReadPtr++];
		//buf[i++] = 0;
		buf[i++] = iSample << 15;
	}
	return &(buf[i]);
}


/*

euint16* PostProcess16(euint16* buf)
{
	unsigned int i = 0, iReadPtr = 0;
	unsigned int iSample, iMSB, iLSB;
	
	while(iReadPtr < (READ_BUFFER_SIZE >> 1))
	{
		iSample = readBuffer[iReadPtr++] + CALIBDACo;
		iMSB = ((iSample * CALIBDACa) >> 16);
		iLSB = (iSample - (iMSB * CALIBDACb));
		while(iLSB > (PWMPERIOD - LSBOFFSET))
		{
			iLSB -= CALIBDACb;
			iMSB++;			
		}
		buf[i++] = (PWMPERIOD - MSBOFFSET) - iMSB; // MSB

		if(iPlayLSB)
		{		
			if(iLSB > PWMPERIOD || iLSB < 0)
			{
				if(i & 2)
				{
					iRGTDAC = iSample;
				}
				else
				{
					iLFTDAC = iSample;
				}
			}
			
			buf[i++] = (PWMPERIOD - LSBOFFSET) - iLSB; // LSB
		}
		else
		{
			buf[i++] = PWMPERIOD + 1; // disable LSB output.
		}
	}
	return &(buf[i]);
}
*/

/*
// Plays a RAW audio file from a FAT file system. 
void playRAWfat(char* fileName)
{
	bExit = 0;
	
	audioSize = 2048;
	audioBuffer = malloc(audioSize * sizeof(euint16));
	if(audioBuffer == NULL)
	{
		rprintf("Can't allocate play buffer.\n");
		return;
	}
	
	if ( file_fopen( &fp, &efs.myFs, fileName , 'r' ) == 0 ) 
	{
		euint32 iAddress;
		unsigned short e, i;
		euint16* p;

		iAddress = file_getAddress(&fp, 0);
		
		e = file_read( &fp, audioSize * sizeof(euint16), (euint8*)audioBuffer );
		PostProcess16(audioBuffer, audioSize, e >> 1);

		audioFrequency(44100);
		audioStart();
		i = 0;
		while ( e != 0 && bExit == 0) 
		{
			while(iPlayPtr < (audioSize >> 1))
			{
				doIdle();
			}
			e = file_read( &fp, audioSize, (euint8*)audioBuffer );
			PostProcess16(audioBuffer, audioSize>>1, e >> 1);

			while(iPlayPtr >= (audioSize >> 1))
			{
				doIdle();
			}

			p = &(audioBuffer[audioSize>>1]);
			e = file_read( &fp, audioSize, (euint8*)p );
			PostProcess16(p, audioSize>>1, e >> 1);
			
			i++;
			if(i > 100)
			{
				i = 0;
				ledToggle();
			}
		}
		file_fclose( &fp );
		audioStop();
	}
	
	free(audioBuffer);
}
*/

/*
void ledToggle(unsigned int iLEDPIN)
{
	static unsigned char state=0;
	
	state = !state;
	if (state) FIOCLR = BIT(iLEDPIN);	// set Bit = LED on
	else FIOSET = BIT(iLEDPIN);	// set Bit = LED off (active low)
}

int ledRead(unsigned int iLEDPIN)
{
	return (FIOPIN & (1 << iLEDPIN));
}
*/

