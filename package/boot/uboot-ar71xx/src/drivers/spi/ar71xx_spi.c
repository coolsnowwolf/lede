/*
 * (C) Copyright 2010
 * Michael Kurz <michi.kurz@googlemail.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <malloc.h>
#include <spi.h>

#include <asm/addrspace.h>
#include <asm/types.h>
#include <asm/ar71xx.h>

/*-----------------------------------------------------------------------
 * Definitions
 */

#ifdef DEBUG_SPI
#define PRINTD(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTD(fmt,args...)
#endif

struct ar71xx_spi_slave {
	struct spi_slave slave;
	unsigned int mode;
};

static inline struct ar71xx_spi_slave *to_ar71xx_spi(struct spi_slave *slave)
{
	return container_of(slave, struct ar71xx_spi_slave, slave);
}

/*=====================================================================*/
/*                         Public Functions                            */
/*=====================================================================*/

/*-----------------------------------------------------------------------
 * Initialization
 */
 
void spi_init()
{
	PRINTD("ar71xx_spi: spi_init");

	// Init SPI Hardware, disable remap, set clock
	__raw_writel(0x43, KSEG1ADDR(AR71XX_SPI_BASE + SPI_REG_CTRL));
	
	PRINTD(" ---> out\n");
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	struct ar71xx_spi_slave *ss;

	PRINTD("ar71xx_spi: spi_setup_slave");
	
	if ((bus != 0) || (cs > 2))
		return NULL;

	ss = malloc(sizeof(struct ar71xx_spi_slave));
	if (!ss)
		return NULL;

	ss->slave.bus = bus;
	ss->slave.cs = cs;
	ss->mode = mode;

	/* TODO: Use max_hz to limit the SCK rate */

	PRINTD(" ---> out\n");
	
	return &ss->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct ar71xx_spi_slave *ss = to_ar71xx_spi(slave);

	free(ss);
}

int spi_claim_bus(struct spi_slave *slave)
{

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{

}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
		void *din, unsigned long flags)
{
	struct ar71xx_spi_slave *ss = to_ar71xx_spi(slave);
	uint8_t *rx = din;
	const uint8_t *tx = dout;
	uint8_t curbyte, curbitlen, restbits;
	uint32_t bytes = bitlen / 8;
	uint32_t out;
	uint32_t in;
	
	PRINTD("ar71xx_spi: spi_xfer: slave:%p bitlen:%08x dout:%p din:%p flags:%08x\n", slave, bitlen, dout, din, flags);
	
	if (flags & SPI_XFER_BEGIN) {
		__raw_writel(SPI_FS_GPIO, KSEG1ADDR(AR71XX_SPI_BASE + SPI_REG_FS));
		__raw_writel(SPI_IOC_CS_ALL, KSEG1ADDR(AR71XX_SPI_BASE + SPI_REG_IOC));
	}
	
	restbits = (bitlen % 8);
	if (restbits != 0)
		bytes++;

	// enable chip select
	out = SPI_IOC_CS_ALL & ~(SPI_IOC_CS(slave->cs));

	while (bytes--) {
		
		curbyte = 0;
		if (tx) {
			curbyte = *tx++;
		}
		
		if (restbits != 0) {
			curbitlen = restbits;
			curbyte <<= 8 - restbits;
		} else {
			curbitlen = 8;
		}
		
		PRINTD("ar71xx_spi: sending: data:%02x length:%d\n", curbyte, curbitlen);
		
		/* clock starts at inactive polarity */
		for (curbyte <<= (8 - curbitlen); curbitlen; curbitlen--) {

			if (curbyte & (1 << 7))
				out |= SPI_IOC_DO;
			else
				out &= ~(SPI_IOC_DO);

			/* setup MSB (to slave) on trailing edge */
			__raw_writel(out, KSEG1ADDR(AR71XX_SPI_BASE + SPI_REG_IOC));

			__raw_writel(out | SPI_IOC_CLK, KSEG1ADDR(AR71XX_SPI_BASE + SPI_REG_IOC));

			curbyte <<= 1;
		}
		
		in = __raw_readl(KSEG1ADDR(AR71XX_SPI_BASE + SPI_REG_RDS));
		PRINTD("ar71xx_spi: received:%02x\n", in);
		
		if (rx) {
			if (restbits == 0) {
				*rx++ = in;
			} else {
				*rx++ = (in << (8 - restbits));
			}
		}
	}
	
	if (flags & SPI_XFER_END) {
		__raw_writel(SPI_IOC_CS(slave->cs), KSEG1ADDR(AR71XX_SPI_BASE + SPI_REG_IOC));
		__raw_writel(SPI_IOC_CS_ALL, KSEG1ADDR(AR71XX_SPI_BASE + SPI_REG_IOC));
		__raw_writel(0, KSEG1ADDR(AR71XX_SPI_BASE + SPI_REG_FS));
	}

	PRINTD(" ---> out\n");
	
	return 0;
}
