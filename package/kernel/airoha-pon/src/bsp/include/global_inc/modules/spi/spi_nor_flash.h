 /***************************************************************************************
 *      Copyright(c) 2014 ECONET Incorporation All rights reserved.
 *
 *      This is unpublished proprietary source code of ECONET Incorporation
 *
 *      The copyright notice above does not evidence any actual or intended
 *      publication of such source code.
 ***************************************************************************************
 */

#ifndef __SPI_NOR_FLASH_H__
#define __SPI_NOR_FLASH_H__

void spi_nor_read(unsigned char *data, unsigned long addr, int len);
unsigned char spi_nor_read_byte(unsigned long addr);

#endif
/* End of [spi_nor_flash.h] package */

