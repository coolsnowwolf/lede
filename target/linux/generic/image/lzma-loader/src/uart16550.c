/*
 * Copyright (C) 2001 MontaVista Software Inc.
 * Author: Jun Sun, jsun@mvista.com or jsun@junsun.net
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */


#include "uart16550.h"

/* === CONFIG === */

#define         BASE                    0xb8058000
#define         MAX_BAUD                1152000
#define         REG_OFFSET              4

/* === END OF CONFIG === */

/* register offset */
#define         OFS_RCV_BUFFER          (0*REG_OFFSET)
#define         OFS_TRANS_HOLD          (0*REG_OFFSET)
#define         OFS_SEND_BUFFER         (0*REG_OFFSET)
#define         OFS_INTR_ENABLE         (1*REG_OFFSET)
#define         OFS_INTR_ID             (2*REG_OFFSET)
#define         OFS_DATA_FORMAT         (3*REG_OFFSET)
#define         OFS_LINE_CONTROL        (3*REG_OFFSET)
#define         OFS_MODEM_CONTROL       (4*REG_OFFSET)
#define         OFS_RS232_OUTPUT        (4*REG_OFFSET)
#define         OFS_LINE_STATUS         (5*REG_OFFSET)
#define         OFS_MODEM_STATUS        (6*REG_OFFSET)
#define         OFS_RS232_INPUT         (6*REG_OFFSET)
#define         OFS_SCRATCH_PAD         (7*REG_OFFSET)

#define         OFS_DIVISOR_LSB         (0*REG_OFFSET)
#define         OFS_DIVISOR_MSB         (1*REG_OFFSET)


/* memory-mapped read/write of the port */
#define         UART16550_READ(y)    (*((volatile uint32*)(BASE + y)))
#define         UART16550_WRITE(y, z)  ((*((volatile uint32*)(BASE + y))) = z)

#define DEBUG_LED (*(unsigned short*)0xb7ffffc0)
#define OutputLED(x)  (DEBUG_LED = x)

void Uart16550Init(uint32 baud, uint8 data, uint8 parity, uint8 stop)
{
    /* disable interrupts */
    UART16550_WRITE(OFS_INTR_ENABLE, 0);

    /* set up buad rate */
    { 
        uint32 divisor;
       
        /* set DIAB bit */
        UART16550_WRITE(OFS_LINE_CONTROL, 0x80);
        
        /* set divisor */
        divisor = MAX_BAUD / baud;
        UART16550_WRITE(OFS_DIVISOR_LSB, divisor & 0xff);
        UART16550_WRITE(OFS_DIVISOR_MSB, (divisor & 0xff00)>>8);

        /* clear DIAB bit */
        UART16550_WRITE(OFS_LINE_CONTROL, 0x0);
    }

    /* set data format */
    UART16550_WRITE(OFS_DATA_FORMAT, data | parity | stop);
}

uint8 Uart16550GetPoll()
{
    while((UART16550_READ(OFS_LINE_STATUS) & 0x1) == 0);
    return UART16550_READ(OFS_RCV_BUFFER);
}


void Uart16550Put(uint8 byte)
{
    while ((UART16550_READ(OFS_LINE_STATUS) &0x20) == 0);
    UART16550_WRITE(OFS_SEND_BUFFER, byte);
}

