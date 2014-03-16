//*************************************************************
//  I2C.h
//  
//  This header file is for SPI firmware routines.
//
//  Written for Hi-Tech PICC compiler vX.XX
//**************************************************************

#define mSCK    RC4
#define mSO     RA4
//#define mSO     RA2
#define mSI     RC5


extern void WriteSPI( unsigned char data_out );
extern unsigned char ReadSPI( void );
