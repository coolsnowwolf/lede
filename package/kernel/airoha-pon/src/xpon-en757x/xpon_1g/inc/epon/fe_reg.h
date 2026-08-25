/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2012, MTK.
 *
 * All rights reserved.	MediaTeK's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************

	Module Name:
	fe_reg.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	jq.zhu		2012/9/26		Create
*/



#ifndef __FE_REG
#define __FE_REG






#define FE_BASE     		0xBFB50000


#define GDMA2_BASE     		(FE_BASE + 0x1500)
#define GDMA2_MIB_CLR     		(GDMA2_BASE + 0x20)



#define GDMA2_COUNT_BASE 	(FE_BASE + 0x1600)
#define GDMA2_TX_GETCNT     (GDMA2_COUNT_BASE + 0x00)
#define GDMA2_TX_OKCNT     	(GDMA2_COUNT_BASE + 0x4)
#define GDMA2_TX_DROPCNT   	(GDMA2_COUNT_BASE + 0x8)

#define GDMA2_TX_ETHCNT   	(GDMA2_COUNT_BASE + 0x10)
#define GDMA2_TX_ETHLENCNT   	(GDMA2_COUNT_BASE + 0x14)
#define GDMA2_TX_ETHDROPCNT   	(GDMA2_COUNT_BASE + 0x18)
#define GDMA2_TX_ETHBCDCNT   	(GDMA2_COUNT_BASE + 0x1C)
#define GDMA2_TX_ETHMULTICASTCNT   	(GDMA2_COUNT_BASE + 0x20)
#define GDMA2_TX_ETH_LESS64_CNT   	(GDMA2_COUNT_BASE + 0x24)
#define GDMA2_TX_ETH_MORE1518_CNT   	(GDMA2_COUNT_BASE + 0x28)
#define GDMA2_TX_ETH_64_CNT   			(GDMA2_COUNT_BASE + 0x2C)
#define GDMA2_TX_ETH_65_TO_127_CNT   	(GDMA2_COUNT_BASE + 0x30)
#define GDMA2_TX_ETH_128_TO_255_CNT   	(GDMA2_COUNT_BASE + 0x34)
#define GDMA2_TX_ETH_256_TO_511_CNT   	(GDMA2_COUNT_BASE + 0x38)
#define GDMA2_TX_ETH_512_TO_1023_CNT   	(GDMA2_COUNT_BASE + 0x3C)
#define GDMA2_TX_ETH_1024_TO_1518_CNT   	(GDMA2_COUNT_BASE + 0x40)


#define GDMA2_RX_OKCNT     	(GDMA2_COUNT_BASE + 0x50)
#define GDMA2_RX_OVDROPCNT    (GDMA2_COUNT_BASE + 0x54)
#define GDMA2_RX_ERRDROPCNT    (GDMA2_COUNT_BASE + 0x58)
#define GDMA2_RX_ETHERPCNT  (GDMA2_COUNT_BASE + 0x60)
#define GDMA2_RX_ETHERPLEN  (GDMA2_COUNT_BASE + 0x64)
#define GDMA2_RX_ETHDROPCNT (GDMA2_COUNT_BASE + 0x68)
#define GDMA2_RX_ETHBCCNT   (GDMA2_COUNT_BASE + 0x6c)
#define GDMA2_RX_ETHMCCNT   (GDMA2_COUNT_BASE + 0x70)
#define GDMA2_RX_ETHCRCCNT  (GDMA2_COUNT_BASE + 0x74)
#define GDMA2_RX_ETHFRACCNT (GDMA2_COUNT_BASE + 0x78)
#define GDMA2_RX_ETHJABCNT  (GDMA2_COUNT_BASE + 0x7c)
#define GDMA2_RX_ETHRUNTCNT (GDMA2_COUNT_BASE + 0x80)
#define GDMA2_RX_ETHLONGCNT (GDMA2_COUNT_BASE + 0x84)
#define GDMA2_RX_ETH_64_CNT (GDMA2_COUNT_BASE + 0x88)
#define GDMA2_RX_ETH_65_TO_127_CNT (GDMA2_COUNT_BASE + 0x8C)
#define GDMA2_RX_ETH_128_TO_255_CNT (GDMA2_COUNT_BASE + 0x90)
#define GDMA2_RX_ETH_256_TO_511_CNT (GDMA2_COUNT_BASE + 0x94)
#define GDMA2_RX_ETH_512_TO_1023_CNT (GDMA2_COUNT_BASE + 0x98)
#define GDMA2_RX_ETH_1024_TO_1518_CNT (GDMA2_COUNT_BASE + 0x9C)



#endif //__FE_REG

