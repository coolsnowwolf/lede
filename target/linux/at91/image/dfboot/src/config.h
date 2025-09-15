
#ifndef _CONFIG_H
#define _CONFIG_H

//#define	PAGESZ_1056			1
#undef PAGESZ_1056
#define	SPI_LOW_SPEED			1
#define AT91C_DELAY_TO_BOOT 		1500

#define	CRC_RETRIES			0x100

#define AT91C_MASTER_CLOCK              59904000
#define AT91C_BAUD_RATE                 115200

#define AT91C_ALTERNATE_USART	AT91C_BASE_US0

#endif
