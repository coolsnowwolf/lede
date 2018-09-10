#include <linux/io.h>
#include <linux/amba/serial.h>
#include <adm8668.h>

#define UART_READ(r) \
	__raw_readl((void __iomem *)(KSEG1ADDR(ADM8668_UART0_BASE) + (r)))

#define UART_WRITE(v, r) \
	__raw_writel((v), (void __iomem *)(KSEG1ADDR(ADM8668_UART0_BASE) + (r)))

void prom_putchar(char c)
{
	UART_WRITE(c, UART01x_DR);
	while ((UART_READ(UART01x_FR) & UART01x_FR_TXFF) != 0)
		;
}
