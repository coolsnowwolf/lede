/*
 ***************************************************************************
 * MediaTek Inc. 
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mt_hif_sdio.h
*/

#ifndef __MT_HIF_SDIO_H__
#define __MT_HIF_SDIO_H__

#define WCIR 0x0000
#define CHIP_ID_MASK (0xffff)
#define GET_CHIP_ID(p) (((p) & CHIP_ID_MASK))
#define REVISION_ID_MASK (0xf << 16)
#define GET_REVISION_ID(p) (((p) & REVISION_ID_MASK) >> 16)
#define POR_INDICATOR (1 << 20)
#define GET_POR_INDICATOR(p) (((p) & POR_INDICATOR) >> 20)
#define W_FUNC_RDY (1 << 21)
#define GET_W_FUNC_RDY(p) (((p) & W_FUNC_RDY) >> 21)
#define DEVICE_STATUS_MASK (0xff << 24)
#define GET_DEVICE_STATUS(p) (((p) & DEVICE_STATUS_MASK) >> 24)

#define WHLPCR 0x0004
#define W_INT_EN_SET (1 << 0)
#define W_INT_EN_CLR (1 << 1)
#define W_FW_OWN_REQ_SET (1 << 8)
#define GET_W_FW_OWN_REQ_SET(p) (((p) & W_FW_OWN_REQ_SET) >> 8)
#define W_FW_OWN_REQ_CLR (1 << 9)

#define WSDIOCSR 0x0008

#define WHCR 0x000C
#define W_INT_CLR_CTRL (1 << 1)
#define RECV_MAILBOX_RD_CLR_EN (1 << 2)
#define RPT_OWN_RX_PACKET_LEN (1 << 3)
#define MAX_HIF_RX_LEN_NUM_MASK (0x3f << 8)
#define MAX_HIF_RX_LEN_NUM(p) (((p) & 0x3f) << 8)
#define GET_MAX_HIF_RX_LEN_NUM(p) (((p) & MAX_HIF_RX_LEN_NUM_MASK) >> 8)
#define RX_ENHANCE_MODE (1 << 16)

#define WHISR 0x0010
#define TX_DONE_INT (1 << 0)
#define RX0_DONE_INT (1 << 1)
#define RX1_DONE_INT (1 << 2)
#define ABNORMAL_INT (1 << 6)
#define FW_OWN_BACK_INT (1 << 7)
#define D2H_SW_INT (0xffffff << 8)
#define D2H_SW_INT_MASK (0xffffff << 8)
#define GET_D2H_SW_INT(p) (((p) & D2H_SW_INT_MASK) >> 8)

#define WHIER 0x0014
#define TX_DONE_INT_EN (1 << 0)
#define RX0_DONE_INT_EN (1 << 1)
#define RX1_DONE_INT_EN (1 << 2)
#define ABNORMAL_INT_EN (1 << 6)
#define FW_OWN_BACK_INT_EN (1 << 7)
#define D2H_SW_INT_EN_MASK (0xffffff << 8)
#define D2H_SW_INT_EN(p) (((p) & 0xffffff) << 8)
#define GET_D2H_SW_INT_EN(p) (((p) & D2H_SW_INT_EN_MASK) >> 8)

#define WHIER_DEFAULT (TX_DONE_INT_EN | RX0_DONE_INT_EN | RX1_DONE_INT_EN\
						| ABNORMAL_INT_EN\
						| D2H_SW_INT_EN_MASK)


#define WASR 0x0020
#define TX1_OVERFLOW (1 << 1)
#define RX0_UNDERFLOW (1 << 8)
#define RX1_UNDERFLOW (1 << 9)
#define FW_OWN_INVALID_ACCESS (1 << 16)

#define WSICR 0x0024
#define WTDR1 0x0034
#define WRDR0 0x0050
#define WRDR1 0x0054
#define H2DSM0R 0x0070
#define H2DSM1R 0x0074
#define D2HRM0R 0x0078
#define D2HRM1R 0x007c

#define WRPLR 0x0090
#define RX0_PACKET_LENGTH_MASK (0xffff)
#define GET_RX0_PACKET_LENGTH(p) (((p) & RX0_PACKET_LENGTH_MASK))
#define RX1_PACKET_LENGTH_MASK (0xffff << 16)
#define GET_RX1_PACKET_LENGTH(p) (((p) & RX1_PACKET_LENGTH_MASK) >> 16)

#define WTMDR 0x00b0
#define WTMCR 0x00b4
#define WTMDPCR0 0x00b8
#define WTMDPCR1 0x00bc

#define WPLRCR 0x00d4
#define RX0_RPT_PKT_LEN_MASK (0x3f)
#define RX0_RPT_PKT_LEN(p) (((p) & 0x3f))
#define GET_RPT_PKT_LEN(p) (((p) & RX0_RPT_PKT_LEN_MASK))
#define RX1_RPT_PKT_LEN_MASK (0x3f << 8)
#define RX1_RPT_PKT_LEN(p) (((p) & 0x3f) << 8)
#define GET_RX1_RPT_PKT_LEN(p) (((p) & RX1_RPT_PKT_LEN_MASK) >> 8)

#define WSR 0x00D8
#define CLKIOCR 0x0100
#define CMDIOCR 0x0104
#define DAT0IOCR 0x0108
#define DAT1IOCR 0x010C
#define DAT2IOCR 0x0110
#define DAT3IOCR 0x0114
#define CLKDLYCR 0x0118
#define CMDDLYCR 0x011C
#define ODATDLYCR 0x0120
#define IDATDLYCR1 0x0124
#define IDATDLYCR2 0x0128
#define ILCHCR 0x012C
#define WTQCR0 0x0130
#define WTQCR1 0x0134
#define WTQCR2 0x0138
#define WTQCR3 0x013C
#define WTQCR4 0x0140
#define WTQCR5 0x0144
#define WTQCR6 0x0148
#define WTQCR7 0x014C

#endif

