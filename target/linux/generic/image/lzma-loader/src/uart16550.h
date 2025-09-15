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

#ifndef _uart16550_h_
#define _uart16550_h_

typedef         unsigned char uint8;
typedef         unsigned int  uint32;

#define         UART16550_BAUD_2400             2400
#define         UART16550_BAUD_4800             4800
#define         UART16550_BAUD_9600             9600
#define         UART16550_BAUD_19200            19200
#define         UART16550_BAUD_38400            38400
#define         UART16550_BAUD_57600            57600
#define         UART16550_BAUD_115200           115200

#define         UART16550_PARITY_NONE           0
#define         UART16550_PARITY_ODD            0x08
#define         UART16550_PARITY_EVEN           0x18
#define         UART16550_PARITY_MARK           0x28
#define         UART16550_PARITY_SPACE          0x38

#define         UART16550_DATA_5BIT             0x0
#define         UART16550_DATA_6BIT             0x1
#define         UART16550_DATA_7BIT             0x2
#define         UART16550_DATA_8BIT             0x3

#define         UART16550_STOP_1BIT             0x0
#define         UART16550_STOP_2BIT             0x4

void Uart16550Init(uint32 baud, uint8 data, uint8 parity, uint8 stop);

/* blocking call */
uint8 Uart16550GetPoll();

void Uart16550Put(uint8 byte);

#endif
