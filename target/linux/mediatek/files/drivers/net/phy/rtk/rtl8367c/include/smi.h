/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * Purpose : RTL8367C switch low-level function for access register
 * Feature : SMI related functions
 *
 */

#ifndef __SMI_H__
#define __SMI_H__

#include <rtk_types.h>
#include "rtk_error.h"

#define MDC_MDIO_CTRL0_REG          31
#define MDC_MDIO_START_REG          29
#define MDC_MDIO_CTRL1_REG          21
#define MDC_MDIO_ADDRESS_REG        23
#define MDC_MDIO_DATA_WRITE_REG     24
#define MDC_MDIO_DATA_READ_REG      25
#define MDC_MDIO_PREAMBLE_LEN       32

#define MDC_MDIO_START_OP          0xFFFF
#define MDC_MDIO_ADDR_OP           0x000E
#define MDC_MDIO_READ_OP           0x0001
#define MDC_MDIO_WRITE_OP          0x0003

#define SPI_READ_OP                 0x3
#define SPI_WRITE_OP                0x2
#define SPI_READ_OP_LEN             0x8
#define SPI_WRITE_OP_LEN            0x8
#define SPI_REG_LEN                 16
#define SPI_DATA_LEN                16

#define GPIO_DIR_IN                 1
#define GPIO_DIR_OUT                0

#define ack_timer                   5

#define DELAY                        10000
#define CLK_DURATION(clk)            { int i; for(i=0; i<clk; i++); }

rtk_int32 smi_read(rtk_uint32 mAddrs, rtk_uint32 *rData);
rtk_int32 smi_write(rtk_uint32 mAddrs, rtk_uint32 rData);

#endif /* __SMI_H__ */


