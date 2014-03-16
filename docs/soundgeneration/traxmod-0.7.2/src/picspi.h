#ifndef PICSPI_H
#define PICSPI_H

#include "main.h"

// IEC0
#define SPI1RXIE (1<<25)	// Receive data available
#define SPI1TXIE (1<<24)	// Transmit buffer empty
#define SPI1EIE (1<<23)		// Errors (receive buffer overflows)

// IFS0
#define SPI1RXIF (1<<25)	// Receive data available
#define SPI1TXIF (1<<24)	// Transmit buffer empty
#define SPI1EIF (1<<23)		// Errors (receive buffer overflows)

// IEC1
#define SPI2RXIE	(1<<7)
#define SPI2TXIE	(1<<6)
#define SPI2EIE		(1<<5)

// IFS1
#define SPI2RXIF	(1<<7)
#define SPI2TXIF	(1<<6)
#define SPI2EIF		(1<<5)

// SPIxCON
#define SPION	(1<<15)
#define SPIMODE32 (1<<11)
#define SPIMODE16 (1<<10)
#define SPISSEN   (1<<7)
#define SPICKE		(1<<8)
#define SPICKP		(1<<6)
#define SPIMSTEN	(1<<5)

// SPIxSTAT
#define SPIBUSY		(1<<11)
#define SPIROV		(1<<6)
#define SPIRBF		(1<<0)
#endif
