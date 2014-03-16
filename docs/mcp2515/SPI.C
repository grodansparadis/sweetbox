/*************************************************************
SPI.C

This program contains SPI routines in firmware (not for SSP).

Written for Hi-Tech PICC compiler vX.XX
**************************************************************/
#include <pic.h>
#include "SPI.H"


/********************************************************************
*     Function Name:    WriteSPI                                    *
*     Return Value:     None                                        *
*     Parameters:       Single data byte for SPI bus.               *
*     Description:      This routine writes a single byte to the    * 
*                       SPI bus.                                    *
********************************************************************/
void WriteSPI(unsigned char DataOut)
{
  unsigned char n;
  
  mSCK = 0;   //Make sure clock is low
  
  for(n=0; n<8; n++)
  {
    mSCK = 0;                 //
    if((DataOut << n) & 0x80)  //Clock a 1 0xFE
      mSO = 1;
    else                        //Clock a 0
      mSO = 0;

      mSCK = 1;                 //Pulse clock
//      mSCK = 0;                 //
  }
}

/********************************************************************
*     Function Name:    ReadSPI                                     *
*     Return Value:     DataIn value                                *
*     Parameters:       void                                        *
*     Description:      Read single byte from SPI bus.              *
*                       Note: Clock comes in low and slave is       *
*                       already driving data.                       *
********************************************************************/
unsigned char ReadSPI( void )
{
  unsigned char n, DataIn = 0;
  
  mSCK = 0;   //Make sure clock is low
  for(n=0; n<7; n++)
  {
    mSCK = 1;
    DataIn = DataIn + mSI;
    DataIn = (DataIn << 1);
    mSCK = 0;
  }

    //last bit
    mSCK = 1;
    DataIn = DataIn + mSI;
    mSCK = 0;
  return DataIn;
}
