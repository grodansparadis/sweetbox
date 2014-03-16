
dsPIC33 MAD MP3 decoder
-----------------------

This project is MP3 decoder for Microchip dsPIC33 DSC. Project based on well
known fixed point MAD MP3 decoder:
http://www.underbit.com/products/mad/

Generally speaking this project based on another MAD port for TI TMS320C55xx
DSP. I found TMS320 port in net and you can download it from:

I rewrite all assemblers routines, check alingment (TI CCS assume that char
size is 16 bit), check all endian-sensitive operations (TMS320 is big-endian,
while dsPIC is little-endian). I use only static data allocation in my port.

dsPIC33 MAD MP3 decoder is available under the terms of the GPL:
http://www.underbit.com/resources/license/gpl/

Resources requirements:
    Flash: ~9.5 kBytes of code + ~11 kBytes of const (8% of dsPIC33FJ256GP710)
    SRAM:  ~25 kBytes of data + ~1.5 kBytes of stack (88% of dsPIC33FJ256GP710)
    CPU usage:
        33.6 MIPS (32 kHz / 128 kbps / stereo)
         6.8 MIPS ( 8 kHz /  20 kbps / stereo)

Limitations:
    MPEG1 Layer 1 and Layer 2 is not support
    16-bit arithmetic used
    mad_port_stereo_intensity_lsf() func work incorrectly (TODO)

Project archive contain source codes of dsPIC MAD MP3 decoder, project for
MPLAB IDE + C30 and test mp3 streams.
/doc/test_result.txt contain report of test streams decoding. Hardware timer
used for measure decode time (decode_time column). Input and output functions
excluded from measure.