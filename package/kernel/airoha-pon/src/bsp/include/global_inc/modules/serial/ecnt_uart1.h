/*
 *	Serial driver for TC3162 SoC
 */
#ifndef _EN7523_UART_H_
#define _EN7523_UART_H_

/*************************
 * UART Module Registers *
 *************************/
#ifdef TCSUPPORT_CPU_ARMV8
#define	CR_UART_BASE    	(0x0)
#else
#define	CR_UART_BASE    	(0xbfbf0000)
#endif
#define	CR_UART_RBR     	(CR_UART_BASE+0x00)
#define	CR_UART_THR     	(CR_UART_BASE+0x00)
#define	CR_UART_IER     	(CR_UART_BASE+0x04)
#define	CR_UART_IIR     	(CR_UART_BASE+0x08)
#define	CR_UART_FCR     	(CR_UART_BASE+0x08)
#define	CR_UART_LCR     	(CR_UART_BASE+0x0c)
#define	CR_UART_MCR     	(CR_UART_BASE+0x10)
#define	CR_UART_LSR     	(CR_UART_BASE+0x14)
#define	CR_UART_MSR     	(CR_UART_BASE+0x18)
#define	CR_UART_SCR     	(CR_UART_BASE+0x1c)
#define	CR_UART_BRDL    	(CR_UART_BASE+0x00)
#define	CR_UART_BRDH    	(CR_UART_BASE+0x04)
#define	CR_UART_WORDA		(CR_UART_BASE+0x20)
#define	CR_UART_MISCC		(CR_UART_BASE+0x24)
#define	CR_UART_HWORDA		(CR_UART_BASE+0x28)
#define	CR_UART_XYD     	(CR_UART_BASE+0x2c)

#ifdef TCSUPPORT_UART2
#ifdef TCSUPPORT_CPU_ARMV8
#define	CR_UART2_BASE    	0x0
#define	CR_UART3_BASE    	0x0
#define	CR_UART4_BASE    	0x0
#define	CR_UART5_BASE    	0x0

#else
#define	CR_UART2_BASE    	0xBFBF0300
#define	CR_UART3_BASE    	0xBFBE1000
#define	CR_UART4_BASE    	0xBFBF0600
#define	CR_UART5_BASE    	0xBFBF0700
#endif

#define UART_DPRINT_MSG() { \
	if(port->unused[1] & UART_DEBUG) \
		printk("[UART debug] iobase = %08x, function : %s\n", port->iobase, __func__); \
	}
#endif

#define	UART_BRD_ACCESS		0x80

#ifndef TCSUPPORT_HIGH_SPEED_UART
#define	UART_XYD_Y          65000
#else
#define	UART_XYD_Y          25000
#define UART_XYD_Y_HIGH_SPEED 12500
#endif
#define	UART_UCLK_115200    0
#define	UART_UCLK_57600     1
#define	UART_UCLK_38400     2
#define	UART_UCLK_28800		3
#define	UART_UCLK_19200		4
#define	UART_UCLK_14400		5
#define	UART_UCLK_9600		6
#define	UART_UCLK_4800		7
#define	UART_UCLK_2400		8
#define	UART_UCLK_1200		9
#define	UART_UCLK_600		10
#define	UART_UCLK_300		11
#define	UART_UCLK_110		12
#define	UART_BRDL			0x03
#define	UART_BRDH			0x00
#define	UART_BRDL_20M		0x01
#define	UART_BRDH_20M		0x00
#define	UART_LCR			0x03
#define	UART_FCR			0x0f
#define	UART_WATERMARK		(0x0<<6)
#define	UART_MCR			0x0
#define	UART_MISCC			0x0
#define	UART_IER			0x01

#define	IER_RECEIVED_DATA_INTERRUPT_ENABLE	0x01
#define	IER_THRE_INTERRUPT_ENABLE			0x02
#define	IER_LINE_STATUS_INTERRUPT_ENABLE	0x04
	
#define	IIR_INDICATOR						UART_RDL(CR_UART_IIR)
#define	IIR_RECEIVED_LINE_STATUS			0x06
#define	IIR_RECEIVED_DATA_AVAILABLE			0x04
#define IIR_RECEIVER_IDLE_TRIGGER			0x0C
#define	IIR_TRANSMITTED_REGISTER_EMPTY		0x02	
#define	LSR_INDICATOR						UART_RDL(CR_UART_LSR)
#define	LSR_RECEIVED_DATA_READY				0x01
#define	LSR_OVERRUN							0x02
#define	LSR_PARITY_ERROR					0x04
#define	LSR_FRAME_ERROR						0x08
#define	LSR_BREAK							0x10
#define	LSR_THRE							0x20
#define	LSR_THE								0x40
#define	LSR_RFIFO_FLAG						0x80

#define TC3162_UART_SIZE			0x30

#define PORT_TC3162					3162

#define UART_BAUDRATE_MIN	110
#ifndef TCSUPPORT_HIGH_SPEED_UART
#define UART_BAUDRATE_MAX	921600
#else
#define UART_BAUDRATE_MAX	115200
#endif

#define UART_BAUDRATE		9600

#define	UART_BRDL_20M		0x01
#define	UART_BRDH_20M		0x00
#define UART_CRYSTAL_CLK_20M	20000000
#define UART_CRYSTAL_CLK_DIV	10
#ifdef TCSUPPORT_HIGH_SPEED_UART
#define UART_CRYSTAL_CLK_DIV_HIGH_SPEED	2
#endif


#define	UART_IER_MSTS		0x08

#define UART_MCR_RTS		0x02
#define UART_MCR_LOOP		0x10

#define UART_MSR_CTS		0x10

#define UART_LCR_DLAB		0x80
#define UART_LCR_BCON		0x40
#define UART_LCR_SPBEN		0x20
#define UART_LCR_EOPCON		0x10
#define UART_LCR_PCEN		0x08
#define UART_LCR_SB		0x04
#define UART_LCR_CLEN_MASK	0x03
#define UART_LCR_CLEN_C8	0x03
#define UART_LCR_CLEN_C7	0x02
#define UART_LCR_CLEN_C6	0x01
#define UART_LCR_CLEN_C5	0x00

#define UART_MISCC_CTSHWFC	0x08
#define UART_MISCC_RTSHWFC	0x04

#define UART_CFLAG_DEBUG	0x8000
#define UART_CFLAG_DEVON	0x10000

#define	CR_UART2_RBR     	0x00
#define	CR_UART2_THR     	0x00
#define	CR_UART2_IER     	0x04
#define	CR_UART2_IIR     	0x08
#define	CR_UART2_FCR     	0x08
#define	CR_UART2_LCR     	0x0c
#define	CR_UART2_MCR     	0x10
#define	CR_UART2_LSR     	0x14
#define	CR_UART2_MSR     	0x18
#define	CR_UART2_SCR     	0x1c
#define	CR_UART2_BRDL    	0x00
#define	CR_UART2_BRDH    	0x04
#define	CR_UART2_WORDA		0x20
#define	CR_UART2_HWORDA	    0x28
#define	CR_UART2_MISCC		0x24
#define	CR_UART2_XYD     	0x2c

#define UART_HWFC_ENABLE	(1 << 0)
#define UART_HWFC_DISABLE	(0 << 0)
#define UART_DEBUG			(1 << 1)


/* crystal clock is 20Mhz */
/*---------------------
| uclk_20M | baudrate |
|---------------------|
| 59904    | 115200   |
| 29952    | 57600    |
| 19968    | 38400    |
| 14976    | 28800    |
| 9984     | 19200    |
| 7488     | 14400    |
| 4992     | 9600     |
| 2496     | 4800     |
| 1248     | 2400     |
| 624      | 1200     |
| 312      | 600      |
| 156      | 300      |
| 57       | 110      |
---------------------*/


#endif

