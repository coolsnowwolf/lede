/*
 * (C) Copyright 2010
 * Michael Kurz <michi.kurz@googlemail.com>.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <asm/addrspace.h>
#include <asm/types.h>
#include <config.h>
#include <asm/ar71xx.h>

#define		REG_SIZE		4

/* === END OF CONFIG === */

/* register offset */
#define         OFS_RCV_BUFFER          (0*REG_SIZE)
#define         OFS_TRANS_HOLD          (0*REG_SIZE)
#define         OFS_SEND_BUFFER         (0*REG_SIZE)
#define         OFS_INTR_ENABLE         (1*REG_SIZE)
#define         OFS_INTR_ID             (2*REG_SIZE)
#define         OFS_DATA_FORMAT         (3*REG_SIZE)
#define         OFS_LINE_CONTROL        (3*REG_SIZE)
#define         OFS_MODEM_CONTROL       (4*REG_SIZE)
#define         OFS_RS232_OUTPUT        (4*REG_SIZE)
#define         OFS_LINE_STATUS         (5*REG_SIZE)
#define         OFS_MODEM_STATUS        (6*REG_SIZE)
#define         OFS_RS232_INPUT         (6*REG_SIZE)
#define         OFS_SCRATCH_PAD         (7*REG_SIZE)

#define         OFS_DIVISOR_LSB         (0*REG_SIZE)
#define         OFS_DIVISOR_MSB         (1*REG_SIZE)

#define         UART16550_READ(y)   readl(KSEG1ADDR(AR71XX_UART_BASE+y))
#define         UART16550_WRITE(x, z)  writel(z, KSEG1ADDR((AR71XX_UART_BASE+x)))

void 
ar71xx_sys_frequency(u32 *cpu_freq, u32 *ddr_freq, u32 *ahb_freq)
{
#ifndef CONFIG_AR91XX
    u32 pll, pll_div, cpu_div, ahb_div, ddr_div, freq;

    pll = readl(KSEG1ADDR(AR71XX_PLL_REG_CPU_CONFIG + AR71XX_PLL_BASE));

    pll_div = 
        ((pll & AR71XX_PLL_DIV_MASK) >> AR71XX_PLL_DIV_SHIFT) + 1;

    cpu_div = 
        ((pll & AR71XX_CPU_DIV_MASK) >> AR71XX_CPU_DIV_SHIFT) + 1;

    ddr_div = 
        ((pll & AR71XX_DDR_DIV_MASK) >> AR71XX_DDR_DIV_SHIFT) + 1;

    ahb_div = 
       (((pll & AR71XX_AHB_DIV_MASK) >> AR71XX_AHB_DIV_SHIFT) + 1)*2;

    freq = pll_div * 40000000; 

    if (cpu_freq)
        *cpu_freq = freq/cpu_div;

    if (ddr_freq)
        *ddr_freq = freq/ddr_div;

    if (ahb_freq)
        *ahb_freq = (freq/cpu_div)/ahb_div;

#else
    u32 pll, pll_div, ahb_div, ddr_div, freq;

    pll = readl(KSEG1ADDR(AR91XX_PLL_REG_CPU_CONFIG + AR71XX_PLL_BASE));

    pll_div = 
        ((pll & AR91XX_PLL_DIV_MASK) >> AR91XX_PLL_DIV_SHIFT);

    ddr_div = 
        ((pll & AR91XX_DDR_DIV_MASK) >> AR91XX_DDR_DIV_SHIFT) + 1;

    ahb_div = 
       (((pll & AR91XX_AHB_DIV_MASK) >> AR91XX_AHB_DIV_SHIFT) + 1)*2;

    freq = pll_div * 5000000; 

    if (cpu_freq)
        *cpu_freq = freq;

    if (ddr_freq)
        *ddr_freq = freq/ddr_div;

    if (ahb_freq)
        *ahb_freq = freq/ahb_div;
#endif
}


int serial_init(void)
{
    u32 div;
    u32 ahb_freq = 100000000;

    ar71xx_sys_frequency  (0, 0, &ahb_freq);  
    div  = ahb_freq/(16 * CONFIG_BAUDRATE);  

	// enable uart pins
#ifndef CONFIG_AR91XX
    writel(AR71XX_GPIO_FUNC_UART_EN, KSEG1ADDR(AR71XX_GPIO_BASE + GPIO_REG_FUNC));
#else
	writel(AR91XX_GPIO_FUNC_UART_EN, KSEG1ADDR(AR71XX_GPIO_BASE + GPIO_REG_FUNC));
#endif

    /* set DIAB bit */
    UART16550_WRITE(OFS_LINE_CONTROL, 0x80);

    /* set divisor */
    UART16550_WRITE(OFS_DIVISOR_LSB, (div & 0xff));
    UART16550_WRITE(OFS_DIVISOR_MSB, ((div >> 8) & 0xff));

    /* clear DIAB bit*/ 
    UART16550_WRITE(OFS_LINE_CONTROL, 0x00);

    /* set data format */
    UART16550_WRITE(OFS_DATA_FORMAT, 0x3);

    UART16550_WRITE(OFS_INTR_ENABLE, 0);

	return 0;
}

int serial_tstc (void)
{
    return(UART16550_READ(OFS_LINE_STATUS) & 0x1);
}

int serial_getc(void)
{
    while(!serial_tstc());

    return UART16550_READ(OFS_RCV_BUFFER);
}


void serial_putc(const char byte)
{
    if (byte == '\n') serial_putc ('\r');

    while (((UART16550_READ(OFS_LINE_STATUS)) & 0x20) == 0x0);
    UART16550_WRITE(OFS_SEND_BUFFER, byte);
}

void serial_setbrg (void)
{
}

void serial_puts (const char *s)
{
	while (*s)
	{
		serial_putc (*s++);
	}
}
