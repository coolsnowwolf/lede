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
	hw_amsdu.h
	Abstract:
	HW AMSDU from 7663 CHIP
	Revision History:
	Who                     When              What
	--------        ----------        ----------------------------------------------
*/


#ifndef __HW_AMSDU_H__
#define __HW_AMSDU_H__

#define AMSDU_BASE      0x9000

#define AMSDU_GC	      (AMSDU_BASE + 0x00)
#define AMSDU_TXD_COMP_MAP_0  (AMSDU_BASE + 0x04)
#define AMSDU_TXD_COMP_MAP_1  (AMSDU_BASE + 0x08)
#define AMSDU_CFG_CTRL	      (AMSDU_BASE + 0x0C)
#define AMSDU_PEEK_CR_00      (AMSDU_BASE + 0xD0)
#define AMSDU_PEEK_CR_01      (AMSDU_BASE + 0xD4)
#define AMSDU_PACK_1_MSDU_CNT (AMSDU_BASE + 0xE0)
#define AMSDU_PACK_2_MSDU_CNT (AMSDU_BASE + 0xE4)
#define AMSDU_PACK_3_MSDU_CNT (AMSDU_BASE + 0xE8)
#define AMSDU_PACK_4_MSDU_CNT (AMSDU_BASE + 0xEC)
#define AMSDU_PACK_5_MSDU_CNT (AMSDU_BASE + 0xF0)
#define AMSDU_PACK_6_MSDU_CNT (AMSDU_BASE + 0xF4)
#define AMSDU_PACK_7_MSDU_CNT (AMSDU_BASE + 0xF8)
#define AMSDU_PACK_8_MSDU_CNT (AMSDU_BASE + 0xFC)

#define AMSDU_AC0_QUEUE_EMPTY0 (AMSDU_BASE + 0x100)
#define AMSDU_AC0_QUEUE_EMPTY1 (AMSDU_BASE + 0x104)
#define AMSDU_AC0_QUEUE_EMPTY2 (AMSDU_BASE + 0x108)
#define AMSDU_AC0_QUEUE_EMPTY3 (AMSDU_BASE + 0x10C)

#define AMSDU_AC1_QUEUE_EMPTY0 (AMSDU_BASE + 0x110)
#define AMSDU_AC1_QUEUE_EMPTY1 (AMSDU_BASE + 0x114)
#define AMSDU_AC1_QUEUE_EMPTY2 (AMSDU_BASE + 0x118)
#define AMSDU_AC1_QUEUE_EMPTY3 (AMSDU_BASE + 0x11C)

#define AMSDU_AC2_QUEUE_EMPTY0 (AMSDU_BASE + 0x120)
#define AMSDU_AC2_QUEUE_EMPTY1 (AMSDU_BASE + 0x124)
#define AMSDU_AC2_QUEUE_EMPTY2 (AMSDU_BASE + 0x128)
#define AMSDU_AC2_QUEUE_EMPTY3 (AMSDU_BASE + 0x12C)

#define AMSDU_AC3_QUEUE_EMPTY0 (AMSDU_BASE + 0x130)
#define AMSDU_AC3_QUEUE_EMPTY1 (AMSDU_BASE + 0x134)
#define AMSDU_AC3_QUEUE_EMPTY2 (AMSDU_BASE + 0x138)
#define AMSDU_AC3_QUEUE_EMPTY3 (AMSDU_BASE + 0x13C)

#define AMSDU_AC0_QUEUE_EMPTY4 (AMSDU_BASE + 0x140)
#define AMSDU_AC1_QUEUE_EMPTY4 (AMSDU_BASE + 0x150)
#define AMSDU_AC2_QUEUE_EMPTY4 (AMSDU_BASE + 0x160)
#define AMSDU_AC3_QUEUE_EMPTY4 (AMSDU_BASE + 0x170)

#endif /*__HW_AMSDU_H__ */


