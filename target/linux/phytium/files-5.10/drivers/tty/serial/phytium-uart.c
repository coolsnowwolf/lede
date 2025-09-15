// SPDX-License-Identifier: GPL-2.0
/*
 * Driver for Phytium PCI UART controller
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <linux/console.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>

#define DRV_NAME		"phytium_uart"

#define REG_DR			0x00
#define REG_FR			0x18
#define REG_IBRD		0x24
#define REG_FBRD		0x28
#define REG_LCRH_RX		0x2c
#define REG_LCRH_TX		0x2c
#define REG_CR			0x30
#define REG_IFLS		0x34
#define REG_IMSC		0x38
#define REG_RIS			0x3c
#define REG_MIS			0x40
#define REG_ICR			0x44

#define REG_DR_OE		(1 << 11)
#define REG_DR_BE		(1 << 10)
#define REG_DR_PE		(1 << 9)
#define REG_DR_FE		(1 << 8)

#define REG_LCRH_SPS		0x80
#define REG_LCRH_WLEN_8		0x60
#define REG_LCRH_WLEN_7		0x40
#define REG_LCRH_WLEN_6		0x20
#define REG_LCRH_WLEN_5		0x00
#define REG_LCRH_FEN		0x10
#define REG_LCRH_STP2		0x08
#define REG_LCRH_EPS		0x04
#define REG_LCRH_PEN		0x02
#define REG_LCRH_BRK		0x01

#define REG_FR_RI		0x100
#define REG_FR_TXFE		0x080
#define REG_FR_RXFF		0x040
#define REG_FR_TXFF		0x020
#define REG_FR_RXFE		0x010
#define REG_FR_BUSY		0x008
#define REG_FR_DCD		0x004
#define REG_FR_DSR		0x002
#define REG_FR_CTS		0x001
#define REG_FR_TMSK		(REG_FR_TXFF + REG_FR_BUSY)

#define REG_CR_CTSEN		0x8000		/* CTS hardware flow control */
#define REG_CR_RTSEN		0x4000		/* RTS hardware flow control */
#define REG_CR_OUT2		0x2000		/* OUT2 */
#define REG_CR_OUT1		0x1000		/* OUT1 */
#define REG_CR_RTS		0x0800		/* RTS */
#define REG_CR_DTR		0x0400		/* DTR */
#define REG_CR_RXE		0x0200		/* receive enable */
#define REG_CR_TXE		0x0100		/* transmit enable */
#define REG_CR_LBE		0x0080		/* loopback enable */
#define REG_CR_RTIE		0x0040
#define REG_CR_TIE		0x0020
#define REG_CR_RIE		0x0010
#define REG_CR_MSIE		0x0008
#define REG_CR_IIRLP		0x0004		/* SIR low power mode */
#define REG_CR_SIREN		0x0002		/* SIR enable */
#define REG_CR_UARTEN		0x0001		/* UART enable */

#define REG_IFLS_RX1_8		(0 << 3)
#define REG_IFLS_RX2_8		(1 << 3)
#define REG_IFLS_RX4_8		(2 << 3)
#define REG_IFLS_RX6_8		(3 << 3)
#define REG_IFLS_RX7_8		(4 << 3)
#define REG_IFLS_TX1_8		(0 << 0)
#define REG_IFLS_TX2_8		(1 << 0)
#define REG_IFLS_TX4_8		(2 << 0)
#define REG_IFLS_TX6_8		(3 << 0)

#define REG_IMSC_OEIM		(1 << 10)	/* overrun error interrupt mask */
#define REG_IMSC_BEIM		(1 << 9)	/* break error interrupt mask */
#define REG_IMSC_PEIM		(1 << 8)	/* parity error interrupt mask */
#define REG_IMSC_FEIM		(1 << 7)	/* framing error interrupt mask */
#define REG_IMSC_RTIM		(1 << 6)	/* receive timeout interrupt mask */
#define REG_IMSC_TXIM		(1 << 5)	/* transmit interrupt mask */
#define REG_IMSC_RXIM		(1 << 4)	/* receive interrupt mask */
#define REG_IMSC_DSRMIM		(1 << 3)	/* DSR interrupt mask */
#define REG_IMSC_DCDMIM		(1 << 2)	/* DCD interrupt mask */
#define REG_IMSC_CTSMIM		(1 << 1)	/* CTS interrupt mask */
#define REG_IMSC_RIMIM		(1 << 0)	/* RI interrupt mask */

#define REG_ICR_OEIS		(1 << 10)	/* overrun error interrupt status */
#define REG_ICR_BEIS		(1 << 9)	/* break error interrupt status */
#define REG_ICR_PEIS		(1 << 8)	/* parity error interrupt status */
#define REG_ICR_FEIS		(1 << 7)	/* framing error interrupt status */
#define REG_ICR_RTIS		(1 << 6)	/* receive timeout interrupt status */
#define REG_ICR_TXIS		(1 << 5)	/* transmit interrupt status */
#define REG_ICR_RXIS		(1 << 4)	/* receive interrupt status */
#define REG_ICR_DSRMIS		(1 << 3)	/* DSR interrupt status */
#define REG_ICR_DCDMIS		(1 << 2)	/* DCD interrupt status */
#define REG_ICR_CTSMIS		(1 << 1)	/* CTS interrupt status */
#define REG_ICR_RIMIS		(1 << 0)	/* RI interrupt status */

#define UART_NR			12

#define UART_DR_ERROR		(REG_DR_OE|REG_DR_BE|REG_DR_PE|REG_DR_FE)
#define UART_DUMMY_DR_RX	(1 << 16)

#define DEFAULT_UARTCLK		48000000	/* 48 MHz */

/*
 * We wrap our port structure around the generic uart_port.
 */
struct phytium_uart_port {
	struct uart_port	port;
	unsigned int		im;		/* interrupt mask */
	unsigned int		old_status;
	unsigned int		old_cr;		/* state during shutdown */
	char			type[12];
};

static unsigned int phytium_uart_read(const struct phytium_uart_port *pup,
	unsigned int reg)
{
	void __iomem *addr = pup->port.membase + reg;

	return readl_relaxed(addr);
}

static void phytium_uart_write(unsigned int val, const struct phytium_uart_port *pup,
	unsigned int reg)
{
	void __iomem *addr = pup->port.membase + reg;

	writel_relaxed(val, addr);
}

static int phytium_fifo_to_tty(struct phytium_uart_port *pup)
{
	u16 status;
	unsigned int ch, flag, fifotaken;

	for (fifotaken = 0; fifotaken < 256; fifotaken++) {
		status = phytium_uart_read(pup, REG_FR);
		if (status & REG_FR_RXFE)
			break;

		/* Take chars from the FIFO and update status */
		ch = phytium_uart_read(pup, REG_DR) | UART_DUMMY_DR_RX;
		flag = TTY_NORMAL;
		pup->port.icount.rx++;

		if (unlikely(ch & UART_DR_ERROR)) {
			if (ch & REG_DR_BE) {
				ch &= ~(REG_DR_FE | REG_DR_PE);
				pup->port.icount.brk++;
				if (uart_handle_break(&pup->port))
					continue;
			} else if (ch & REG_DR_PE)
				pup->port.icount.parity++;
			else if (ch & REG_DR_FE)
				pup->port.icount.frame++;
			if (ch & REG_DR_OE)
				pup->port.icount.overrun++;

			ch &= pup->port.read_status_mask;

			if (ch & REG_DR_BE)
				flag = TTY_BREAK;
			else if (ch & REG_DR_PE)
				flag = TTY_PARITY;
			else if (ch & REG_DR_FE)
				flag = TTY_FRAME;
		}

		if (uart_handle_sysrq_char(&pup->port, ch & 255))
			continue;

		uart_insert_char(&pup->port, ch, REG_DR_OE, ch, flag);
	}

	return fifotaken;
}

static void phytium_rx_chars(struct phytium_uart_port *pup)
__releases(&pup->port.lock)
__acquires(&pup->port.lock)
{
	phytium_fifo_to_tty(pup);

	spin_unlock(&pup->port.lock);
	tty_flip_buffer_push(&pup->port.state->port);
	spin_lock(&pup->port.lock);
}

static void phytium_stop_tx(struct uart_port *port)
{
	struct phytium_uart_port *pup =
	    container_of(port, struct phytium_uart_port, port);

	pup->im &= ~REG_IMSC_TXIM;
	phytium_uart_write(pup->im, pup, REG_IMSC);
}

static bool phytium_tx_char(struct phytium_uart_port *pup, unsigned char c,
			    bool from_irq)
{

	if (unlikely(!from_irq) &&
	    phytium_uart_read(pup, REG_FR) & REG_FR_TXFF)
		return false; /* unable to transmit character */

	phytium_uart_write(c, pup, REG_DR);
	pup->port.icount.tx++;

	return true;
}

static bool phytium_tx_chars(struct phytium_uart_port *pup, bool from_irq)
{
	struct circ_buf *xmit = &pup->port.state->xmit;
	int count = pup->port.fifosize >> 1;

	if (pup->port.x_char) {
		if (!phytium_tx_char(pup, pup->port.x_char, from_irq))
			return true;
		pup->port.x_char = 0;
		--count;
	}
	if (uart_circ_empty(xmit) || uart_tx_stopped(&pup->port)) {
		phytium_stop_tx(&pup->port);
		return false;
	}

	do {
		if (likely(from_irq) && count-- == 0)
			break;

		if (!phytium_tx_char(pup, xmit->buf[xmit->tail], from_irq))
			break;

		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
	} while (!uart_circ_empty(xmit));

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(&pup->port);

	if (uart_circ_empty(xmit)) {
		phytium_stop_tx(&pup->port);
		return false;
	}
	return true;
}

static void phytium_modem_status(struct phytium_uart_port *pup)
{
	unsigned int status, delta;

	status = phytium_uart_read(pup, REG_FR) & (REG_FR_DCD|REG_FR_DSR|REG_FR_CTS);

	delta = status ^ pup->old_status;
	pup->old_status = status;

	if (!delta)
		return;

	if (delta & REG_FR_DCD)
		uart_handle_dcd_change(&pup->port, status & REG_FR_DCD);

	if (delta & REG_FR_DSR)
		pup->port.icount.dsr++;

	if (delta & REG_FR_CTS)
		uart_handle_cts_change(&pup->port, status & REG_FR_CTS);

	wake_up_interruptible(&pup->port.state->port.delta_msr_wait);
}

static irqreturn_t phytium_uart_interrupt(int irq, void *dev_id)
{
	struct phytium_uart_port *pup = dev_id;
	unsigned long flags;
	unsigned int status, pass_counter = 256;
	int handled = 0;

	spin_lock_irqsave(&pup->port.lock, flags);
	status = phytium_uart_read(pup, REG_RIS) & pup->im;
	if (status) {
		do {
			phytium_uart_write(status & ~(REG_ICR_TXIS|REG_ICR_RTIS|REG_ICR_RXIS),
					   pup, REG_ICR);

			if (status & (REG_ICR_RTIS|REG_ICR_RXIS))
				phytium_rx_chars(pup);

			if (status & (REG_ICR_DSRMIS|REG_ICR_DCDMIS|
				      REG_ICR_CTSMIS|REG_ICR_RIMIS))
				phytium_modem_status(pup);
			if (status & REG_ICR_TXIS)
				phytium_tx_chars(pup, true);

			if (pass_counter-- == 0)
				break;

			status = phytium_uart_read(pup, REG_RIS) & pup->im;
		} while (status != 0);
		handled = 1;
	}
	spin_unlock_irqrestore(&pup->port.lock, flags);

	return IRQ_RETVAL(handled);
}

static unsigned int phytium_tx_empty(struct uart_port *port)
{
	unsigned int status;
	struct phytium_uart_port *pup =
		container_of(port, struct phytium_uart_port, port);

	status = phytium_uart_read(pup, REG_FR) & (REG_FR_BUSY | REG_FR_TXFF);

	return status ? 0 : TIOCSER_TEMT;
}

static void phytium_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	struct phytium_uart_port *pup =
		container_of(port, struct phytium_uart_port, port);
	unsigned int cr;

	cr = phytium_uart_read(pup, REG_CR);

#define TIOCMBIT(tiocmbit, uartbit)	\
	do {				\
		if (mctrl & tiocmbit)	\
			cr |= uartbit;	\
		else			\
			cr &= ~uartbit;	\
	} while (0)

	TIOCMBIT(TIOCM_RTS, REG_CR_RTS);
	TIOCMBIT(TIOCM_DTR, REG_CR_DTR);
	TIOCMBIT(TIOCM_OUT1, REG_CR_OUT1);
	TIOCMBIT(TIOCM_OUT2, REG_CR_OUT2);
	TIOCMBIT(TIOCM_LOOP, REG_CR_LBE);

	if (port->status & UPSTAT_AUTORTS) {
		/* We need to disable auto-RTS if we want to turn RTS off */
		TIOCMBIT(TIOCM_RTS, REG_CR_RTSEN);
	}
#undef TIOCMBIT

	phytium_uart_write(cr, pup, REG_CR);
}

static unsigned int phytium_get_mctrl(struct uart_port *port)
{
	struct phytium_uart_port *pup =
	    container_of(port, struct phytium_uart_port, port);
	unsigned int cr = 0;
	unsigned int status = phytium_uart_read(pup, REG_FR);

#define TIOCMBIT(uartbit, tiocmbit)	\
	do {				\
		if (status & uartbit)	\
			cr |= tiocmbit;	\
	} while (0)

	TIOCMBIT(REG_FR_DCD, TIOCM_CAR);
	TIOCMBIT(REG_FR_DSR, TIOCM_DSR);
	TIOCMBIT(REG_FR_CTS, TIOCM_CTS);
	TIOCMBIT(REG_FR_RI, TIOCM_RNG);
#undef TIOCMBIT
	return cr;
}

static void phytium_start_tx(struct uart_port *port)
{
	struct phytium_uart_port *pup =
		container_of(port, struct phytium_uart_port, port);

	if (phytium_tx_chars(pup, false)) {
		pup->im |= REG_IMSC_TXIM;
		phytium_uart_write(pup->im, pup, REG_IMSC);
	}
}

static void phytium_stop_rx(struct uart_port *port)
{
	struct phytium_uart_port *pup =
		container_of(port, struct phytium_uart_port, port);

	pup->im &= ~(REG_IMSC_RXIM|REG_IMSC_RTIM|REG_IMSC_FEIM|
		      REG_IMSC_PEIM|REG_IMSC_BEIM|REG_IMSC_OEIM);
	phytium_uart_write(pup->im, pup, REG_IMSC);
}

static void phytium_enable_ms(struct uart_port *port)
{
	struct phytium_uart_port *pup =
		container_of(port, struct phytium_uart_port, port);

	pup->im |= REG_IMSC_RIMIM|REG_IMSC_CTSMIM|REG_IMSC_DCDMIM|REG_IMSC_DSRMIM;
	phytium_uart_write(pup->im, pup, REG_IMSC);
}

static void phytium_break_ctl(struct uart_port *port, int break_state)
{
	struct phytium_uart_port *pup =
		container_of(port, struct phytium_uart_port, port);
	unsigned long flags;
	unsigned int lcr_h;

	spin_lock_irqsave(&pup->port.lock, flags);
	lcr_h = phytium_uart_read(pup, REG_LCRH_TX);
	if (break_state == -1)
		lcr_h |= REG_LCRH_BRK;
	else
		lcr_h &= ~REG_LCRH_BRK;
	phytium_uart_write(lcr_h, pup, REG_LCRH_TX);
	spin_unlock_irqrestore(&pup->port.lock, flags);
}

static int phytium_hwinit(struct uart_port *port)
{
	struct phytium_uart_port *pup =
		container_of(port, struct phytium_uart_port, port);

	/* XXX: more configurable setup method in future */
	pup->port.uartclk = DEFAULT_UARTCLK;

	/* Clear pending error and receive interrupts */
	phytium_uart_write(REG_ICR_OEIS | REG_ICR_BEIS | REG_ICR_PEIS |
			   REG_ICR_FEIS | REG_ICR_RTIS | REG_ICR_RXIS,
			   pup,  REG_ICR);

	/*
	 * Save interrupts enable mask, and enable RX interrupts in case if
	 * the interrupt is used for NMI entry.
	 */
	pup->im = phytium_uart_read(pup, REG_IMSC);
	phytium_uart_write(REG_IMSC_RTIM | REG_IMSC_RXIM, pup, REG_IMSC);

	return 0;
}

static int phytium_uart_allocate_irq(struct phytium_uart_port *pup)
{
	phytium_uart_write(pup->im, pup, REG_IMSC);

	return request_irq(pup->port.irq, phytium_uart_interrupt, IRQF_SHARED, DRV_NAME, pup);
}

static void phytium_enable_interrtups(struct phytium_uart_port *pup)
{
	unsigned int i;

	spin_lock_irq(&pup->port.lock);

	/* Clear out any spuriously appearing RX interrupts */
	phytium_uart_write(REG_ICR_RTIS | REG_ICR_RXIS, pup, REG_ICR);

	/*
	 * RXIS is asserted only when the RX FIFO transitions from below
	 * to above the trigger threshold. If the RX FIFO is already
	 * full to the threashold this can't happen and RXIS will now be
	 * stuck off. Drain the RX FIFO explicitly to fix this:
	 */
	for (i = 0; i < pup->port.fifosize * 2; i++) {
		if (phytium_uart_read(pup, REG_FR) & REG_FR_RXFE)
			break;

		phytium_uart_read(pup, REG_DR);
	}

	pup->im = REG_IMSC_RTIM | REG_IMSC_RXIM;
	phytium_uart_write(pup->im, pup, REG_IMSC);
	spin_unlock_irq(&pup->port.lock);
}

static int phytium_startup(struct uart_port *port)
{
	struct phytium_uart_port *pup =
		container_of(port, struct phytium_uart_port, port);
	unsigned int cr;
	int ret = 0;

	ret = phytium_hwinit(port);
	if (ret)
		goto out;

	ret = phytium_uart_allocate_irq(pup);
	if (ret)
		goto out;

	phytium_uart_write(REG_IFLS_RX4_8|REG_IFLS_TX4_8, pup, REG_IFLS);

	spin_lock_irq(&pup->port.lock);

	/* restore RTS and DTR */
	cr = pup->old_cr & (REG_CR_RTS | REG_CR_DTR);
	cr |= REG_CR_UARTEN | REG_CR_RXE | REG_CR_TXE;
	phytium_uart_write(cr, pup, REG_CR);

	spin_unlock_irq(&pup->port.lock);

	/* initialise the old status of the modem signals */
	pup->old_status = phytium_uart_read(pup, REG_FR) & (REG_FR_DCD|REG_FR_DSR|REG_FR_CTS);

	phytium_enable_interrtups(pup);

out:
	return ret;
}

static void phytium_shutdown_channel(struct phytium_uart_port *pup,
				     unsigned int lcrh)
{
	unsigned long val;

	val = phytium_uart_read(pup, lcrh);
	val &= ~(REG_LCRH_BRK | REG_LCRH_FEN);
	phytium_uart_write(val, pup, lcrh);
}

static void phytium_disable_uart(struct phytium_uart_port *pup)
{
	unsigned int cr;

	pup->port.status &= ~(UPSTAT_AUTOCTS | UPSTAT_AUTORTS);
	spin_lock_irq(&pup->port.lock);
	cr = phytium_uart_read(pup, REG_CR);
	pup->old_cr = cr;
	cr &= REG_CR_RTS | REG_CR_DTR;
	cr |= REG_CR_UARTEN | REG_CR_TXE;
	phytium_uart_write(cr, pup, REG_CR);
	spin_unlock_irq(&pup->port.lock);

	/*
	 * disable break condition and fifos
	 */
	phytium_shutdown_channel(pup, REG_LCRH_RX);
}

static void phytium_disable_interrupts(struct phytium_uart_port *pup)
{
	spin_lock_irq(&pup->port.lock);

	/* mask all interrupts and clear all pending ones */
	pup->im = 0;
	phytium_uart_write(pup->im, pup, REG_IMSC);
	phytium_uart_write(0xffff, pup, REG_ICR);

	spin_unlock_irq(&pup->port.lock);
}

static void phytium_shutdown(struct uart_port *port)
{
	struct phytium_uart_port *pup =
		container_of(port, struct phytium_uart_port, port);

	phytium_disable_interrupts(pup);

	free_irq(pup->port.irq, pup);

	phytium_disable_uart(pup);

	if (pup->port.ops->flush_buffer)
		pup->port.ops->flush_buffer(port);
}

static void
phytium_setup_status_masks(struct uart_port *port, struct ktermios *termios)
{
	port->read_status_mask = REG_DR_OE | 255;
	if (termios->c_iflag & INPCK)
		port->read_status_mask |= REG_DR_FE | REG_DR_PE;
	if (termios->c_iflag & (IGNBRK | BRKINT | PARMRK))
		port->read_status_mask |= REG_DR_BE;

	/*
	 * Characters to ignore
	 */
	port->ignore_status_mask = 0;
	if (termios->c_iflag & IGNPAR)
		port->ignore_status_mask |= REG_DR_FE | REG_DR_PE;
	if (termios->c_iflag & IGNBRK) {
		port->ignore_status_mask |= REG_DR_BE;
		/*
		 * If we're ignoring parity and break indicators,
		 * ignore overruns too (for real raw support).
		 */
		if (termios->c_iflag & IGNPAR)
			port->ignore_status_mask |= REG_DR_OE;
	}

	/*
	 * Ignore all characters if CREAD is not set.
	 */
	if ((termios->c_cflag & CREAD) == 0)
		port->ignore_status_mask |= UART_DUMMY_DR_RX;
}

static void
phytium_set_termios(struct uart_port *port, struct ktermios *termios, struct ktermios *old)
{
	struct phytium_uart_port *pup =
		container_of(port, struct phytium_uart_port, port);
	unsigned int lcr_h, old_cr;
	unsigned long flags;
	unsigned int baud, quot;

	/* Ask the core to calculate the divisor for us. */
	baud = uart_get_baud_rate(port, termios, old, 0, port->uartclk/16);

	if (baud > port->uartclk/16)
		quot = DIV_ROUND_CLOSEST(port->uartclk * 8, baud);
	else
		quot = DIV_ROUND_CLOSEST(port->uartclk * 4, baud);

	switch (termios->c_cflag & CSIZE) {
	case CS5:
		lcr_h = REG_LCRH_WLEN_5;
		break;
	case CS6:
		lcr_h = REG_LCRH_WLEN_6;
		break;
	case CS7:
		lcr_h = REG_LCRH_WLEN_7;
		break;
	default: /* CS8 */
		lcr_h = REG_LCRH_WLEN_8;
		break;
	}
	if (termios->c_cflag & CSTOPB)
		lcr_h |= REG_LCRH_STP2;
	if (termios->c_cflag & PARENB) {
		lcr_h |= REG_LCRH_PEN;
		if (!(termios->c_cflag & PARODD))
			lcr_h |= REG_LCRH_EPS;
		if (termios->c_cflag & CMSPAR)
			lcr_h |= REG_LCRH_SPS;
	}
	if (pup->port.fifosize > 1)
		lcr_h |= REG_LCRH_FEN;

	spin_lock_irqsave(&port->lock, flags);

	/*
	 * Update the per-port timeout.
	 */
	uart_update_timeout(port, termios->c_cflag, baud);

	phytium_setup_status_masks(port, termios);

	if (UART_ENABLE_MS(port, termios->c_cflag))
		phytium_enable_ms(port);

	/* first, disable everything */
	old_cr = phytium_uart_read(pup, REG_CR);
	phytium_uart_write(0, pup, REG_CR);

	if (termios->c_cflag & CRTSCTS) {
		if (old_cr & REG_CR_RTS)
			old_cr |= REG_CR_RTSEN;

		old_cr |= REG_CR_CTSEN;
		port->status |= UPSTAT_AUTOCTS | UPSTAT_AUTORTS;
	} else {
		old_cr &= ~(REG_CR_CTSEN | REG_CR_RTSEN);
		port->status &= ~(UPSTAT_AUTOCTS | UPSTAT_AUTORTS);
	}

	/* Set baud rate */
	phytium_uart_write(quot & 0x3f, pup, REG_FBRD);
	phytium_uart_write(quot >> 6, pup, REG_IBRD);

	phytium_uart_write(lcr_h, pup, REG_LCRH_RX);
	phytium_uart_write(old_cr, pup, REG_CR);

	spin_unlock_irqrestore(&port->lock, flags);
}

static const char *phytium_type(struct uart_port *port)
{
	struct phytium_uart_port *pup =
		container_of(port, struct phytium_uart_port, port);
	return pup->port.type == PORT_PHYTIUM ? pup->type : NULL;
}

static void phytium_release_port(struct uart_port *port)
{
	/* Nothing to release ... */
}

static int phytium_request_port(struct uart_port *port)
{
	/* UARTs always present */
	return 0;
}

static void phytium_config_port(struct uart_port *port, int flags)
{
	if (flags & UART_CONFIG_TYPE) {
		port->type = PORT_PHYTIUM;
		phytium_request_port(port);
	}
}

static int phytium_verify_port(struct uart_port *port, struct serial_struct *ser)
{
	int ret = 0;

	if (ser->type != PORT_UNKNOWN && ser->type != PORT_PHYTIUM)
		ret = -EINVAL;
	if (ser->irq < 0 || ser->irq >= nr_irqs)
		ret = -EINVAL;
	if (ser->baud_base < 9600)
		ret = -EINVAL;

	return ret;
}

static const struct uart_ops phytium_uart_ops = {
	.tx_empty	= phytium_tx_empty,
	.set_mctrl	= phytium_set_mctrl,
	.get_mctrl	= phytium_get_mctrl,
	.stop_tx	= phytium_stop_tx,
	.start_tx	= phytium_start_tx,
	.stop_rx	= phytium_stop_rx,
	.enable_ms	= phytium_enable_ms,
	.break_ctl	= phytium_break_ctl,
	.startup	= phytium_startup,
	.shutdown	= phytium_shutdown,
	.set_termios	= phytium_set_termios,
	.type		= phytium_type,
	.release_port	= phytium_release_port,
	.request_port	= phytium_request_port,
	.config_port	= phytium_config_port,
	.verify_port	= phytium_verify_port,
};

static struct phytium_uart_port *uart_ports[UART_NR];

static struct uart_driver phytium_uart = {
	.owner		= THIS_MODULE,
	.driver_name	= DRV_NAME,
	.dev_name	= "ttyFTX",
	.nr		= UART_NR,
};

void phytium_unregister_port(struct phytium_uart_port *pup)
{
	int i;
	bool busy = false;

	for (i = 0; i < ARRAY_SIZE(uart_ports); i++) {
		if (uart_ports[i] == pup)
			uart_ports[i] = NULL;
		else if (uart_ports[i])
			busy = true;
	}

	if (!busy)
		uart_unregister_driver(&phytium_uart);
}

static int phytium_find_free_port(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(uart_ports); i++)
		if (uart_ports[i] == NULL)
			return i;

	return -EBUSY;
}

static int phytium_register_port(struct phytium_uart_port *pup)
{
	int rc;

	/* Ensure interrupts from this UART are masked and cleared */
	phytium_uart_write(0, pup, REG_IMSC);
	phytium_uart_write(0xffff, pup, REG_ICR);

	if (!phytium_uart.state) {
		rc = uart_register_driver(&phytium_uart);
		if (rc < 0) {
			dev_err(pup->port.dev,
				"Failed to register Phytium PCI UART driver\n");
			return rc;
		}
	}

	rc = uart_add_one_port(&phytium_uart, &pup->port);
	if (rc)
		phytium_unregister_port(pup);

	return rc;
}

static int phytium_uart_probe(struct pci_dev *pdev,
			      const struct pci_device_id *id)
{
	struct phytium_uart_port *pup;
	int portnr, rc;

	portnr = phytium_find_free_port();
	if (portnr < 0)
		return portnr;

	pup = devm_kzalloc(&pdev->dev, sizeof(struct phytium_uart_port),
			   GFP_KERNEL);
	if (!pup)
		return -ENOMEM;

	rc = pcim_enable_device(pdev);
	if (rc)
		return rc;

	rc = pcim_iomap_regions_request_all(pdev, 0x01, pci_name(pdev));
	if (rc)
		return rc;

	pup->port.iotype = UPIO_MEM32;
	pup->port.irq = pdev->irq;
	pup->port.mapbase = pci_resource_start(pdev, 0);
	pup->port.membase = pcim_iomap_table(pdev)[0];
	pup->port.ops = &phytium_uart_ops;
	pup->port.dev = &pdev->dev;
	pup->port.fifosize = 32;
	pup->port.flags = UPF_BOOT_AUTOCONF;
	pup->port.line = portnr;

	uart_ports[portnr] = pup;

	pup->old_cr = 0;
	snprintf(pup->type, sizeof(pup->type), "pci-uart");

	pci_set_drvdata(pdev, pup);

	return phytium_register_port(pup);
}

static void phytium_uart_remove(struct pci_dev *pdev)
{
	struct phytium_uart_port *pup = pci_get_drvdata(pdev);

	uart_remove_one_port(&phytium_uart, &pup->port);
	phytium_unregister_port(pup);
}

#ifdef CONFIG_PM_SLEEP
static int phytium_uart_suspend(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct phytium_uart_port *pup = pci_get_drvdata(pdev);

	if (pup)
		uart_suspend_port(&phytium_uart, &pup->port);

	return 0;
}

static int phytium_uart_resume(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct phytium_uart_port *pup = pci_get_drvdata(pdev);

	if (pup)
		uart_resume_port(&phytium_uart, &pup->port);

	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(phytium_dev_pm_ops, phytium_uart_suspend, phytium_uart_resume);

static const struct pci_device_id pci_ids[] = {
	{ PCI_VDEVICE(PHYTIUM, 0xdc2e) },
	{ 0 }
};
MODULE_DEVICE_TABLE(pci, pci_ids);

static struct pci_driver phytium_uart_pci_driver = {
	.name		= DRV_NAME,
	.probe		= phytium_uart_probe,
	.remove		= phytium_uart_remove,
	.driver	= {
		.pm	= &phytium_dev_pm_ops,
	},
	.id_table	= pci_ids,
};

static int __init phytium_uart_init(void)
{
	pr_info("Serial: Phytium PCI UART driver\n");

	return pci_register_driver(&phytium_uart_pci_driver);
}

static void __exit phytium_uart_exit(void)
{
	pci_unregister_driver(&phytium_uart_pci_driver);
}

module_init(phytium_uart_init);
module_exit(phytium_uart_exit);

MODULE_AUTHOR("Chen Baozi <chenbaozi@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium PCI serial port driver");
MODULE_LICENSE("GPL");
