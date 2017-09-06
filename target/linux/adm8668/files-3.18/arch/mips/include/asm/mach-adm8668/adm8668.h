/************************************************************************
 *
 *    Copyright (c) 2005
 *    Infineon Technologies AG
 *    St. Martin Strasse 53; 81669 Muenchen; Germany
 *
 ************************************************************************/

#ifndef  __ADM8668_H__
#define  __ADM8668_H__

/*=======================  Physical Memory Map  ============================*/
#define ADM8668_SDRAM_BASE	0
#define ADM8668_SMEM1_BASE	0x10000000
#define ADM8668_MPMC_BASE	0x11000000
#define ADM8668_USB_BASE	0x11200000
#define ADM8668_CONFIG_BASE	0x11400000
#define ADM8668_WAN_BASE	0x11600000
#define ADM8668_WLAN_BASE	0x11800000
#define ADM8668_LAN_BASE	0x11A00000
#define ADM8668_INTC_BASE	0x1E000000
#define ADM8668_TMR_BASE	0x1E200000
#define ADM8668_UART0_BASE	0x1E400000
#define ADM8668_SMEM0_BASE	0x1FC00000
#define ADM8668_NAND_BASE	0x1FFFFF00

#define ADM8668_PCICFG_BASE	0x12200000
#define ADM8668_PCIDAT_BASE	0x12400000

/* interrupt levels */
#define ADM8668_SWI_IRQ		1
#define ADM8668_COMMS_RX_IRQ	2
#define ADM8668_COMMS_TX_IRQ	3
#define ADM8668_TIMER0_IRQ	4
#define ADM8668_TIMER1_IRQ	5
#define ADM8668_UART0_IRQ	6
#define ADM8668_LAN_IRQ		7
#define ADM8668_WAN_IRQ		8
#define ADM8668_WLAN_IRQ	9
#define ADM8668_GPIO_IRQ	10
#define ADM8668_IDE_IRQ		11
#define ADM8668_PCI2_IRQ	12
#define ADM8668_PCI1_IRQ	13
#define ADM8668_PCI0_IRQ	14
#define ADM8668_USB_IRQ		15
#define ADM8668_IRQ_MAX		ADM8668_USB_IRQ

/* register access macros */
#define ADM8668_CONFIG_REG(_reg)	\
	(*((volatile unsigned int *)(KSEG1ADDR(ADM8668_CONFIG_BASE + (_reg)))))

/* lan registers */
#define	NETCSR6			0x30
#define	NETCSR7			0x38
#define	NETCSR37		0xF8

/* known/used CPU configuration registers */
#define ADM8668_CR0		0x00
#define ADM8668_CR1		0x04
#define ADM8668_CR3		0x0C
#define ADM8668_CR66		0x108

/** For GPIO control **/
#define	GPIO_REG		0x5C	/* on WLAN */
#define CRGPIO_REG		0x20	/* on CPU */

void adm8668_init_clocks(void);

#endif /* __ADM8668_H__ */
