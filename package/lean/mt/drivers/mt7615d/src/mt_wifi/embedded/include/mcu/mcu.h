/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mcu.h

	Abstract:
	MCU related information

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __MCU_H__
#define __MCU_H__

#define CONFG_BASE_ADDR                  0x2000
#define CONFG_COM1_REG3                  (CONFG_BASE_ADDR + 0x0000020C)
#define CONFG_COM1_REG3_FWOPMODE          BIT(4)
#define CONFG_COM2_REG3                  (CONFG_BASE_ADDR + 0x0000060C)
#define CONFG_COM2_REG3_FWOPMODE          BIT(4)


#define ENABLE_RXD_LOG 0

enum MCU_TYPE {
	SWMCU = (1 << 0),
	M8051 = (1 << 1),
	ANDES = (1 << 2),
	CR4 = (1 << 3),
};

/*
 * Power opration
 */
enum PWR_OP {
	RADIO_OFF = 0x30,
	RADIO_ON,
	RADIO_OFF_AUTO_WAKEUP,
	RADIO_OFF_ADVANCE,
	RADIO_ON_ADVANCE,
};

struct _RTMP_ADAPTER;

VOID ChipOpsMCUHook(struct _RTMP_ADAPTER *pAd, enum MCU_TYPE MCUType);
VOID MCUCtrlInit(struct _RTMP_ADAPTER *pAd);
VOID MCUCtrlExit(struct _RTMP_ADAPTER *pAd);

INT32 MCUSysPrepare(struct _RTMP_ADAPTER *pAd);


INT32 MCUSysInit(struct _RTMP_ADAPTER *pAd);
INT32 MCUSysExit(struct _RTMP_ADAPTER *pAd);

#endif
