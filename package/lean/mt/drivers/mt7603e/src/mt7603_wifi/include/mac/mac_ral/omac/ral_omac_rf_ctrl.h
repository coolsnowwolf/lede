/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering	the source code	is stricitly prohibited, unless	the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	ral_omac_rf_ctrl.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __RAL_OMAC_RF_CTRL_H__
#define __RAL_OMAC_RF_CTRL_H__



/* ================================================================================= */
/* Register format  for RFCTRL                                                       */
/* ================================================================================= */

#define OSC_CTRL		0x5a4
#define PCIE_PHY_TX_ATTENUATION_CTRL		0x05C8
#define INTERNAL_1		0x05C8

#ifdef RT_BIG_ENDIAN
typedef union _INTERNAL_1_STRUCT
{
	struct
	{
		UINT32 Reserve1:10;
		UINT32 CSO_RX_IPV6_CHKSUM_EN:1;
		UINT32 CSO_TX_IPV6_CHKSUM_EN:1;
		UINT32 CSO_HW_PARSE_TCP:1;
		UINT32 CSO_HW_PARSE_IP:1;
		UINT32 CSO_RX_CHKSUM_EN:1;
		UINT32 CSO_TX_CHKSUM_EN:1;
		UINT32 CSO_TIMEOUT_VALUE:4;
		UINT32 PCIE_PHY_TX_ATTEN_EN:1;
		UINT32 PCIE_PHY_TX_ATTEN_VALUE:3;
		UINT32 Reserve2:7;
		UINT32 RF_ISOLATION_ENABLE:1;
	} field;

	UINT32 word;
} INTERNAL_1_STRUCT;
#else
typedef union _INTERNAL_1_STRUCT {
	struct
	{
		UINT32 RF_ISOLATION_ENABLE:1;
		UINT32 Reserve2:7;
		UINT32 PCIE_PHY_TX_ATTEN_VALUE:3;
		UINT32 PCIE_PHY_TX_ATTEN_EN:1;
		UINT32 CSO_TIMEOUT_VALUE:4;
		UINT32 CSO_TX_CHKSUM_EN:1;
		UINT32 CSO_RX_CHKSUM_EN:1;
		UINT32 CSO_HW_PARSE_IP:1;
		UINT32 CSO_HW_PARSE_TCP:1;
		UINT32 CSO_TX_IPV6_CHKSUM_EN:1;
		UINT32 CSO_RX_IPV6_CHKSUM_EN:1;
		UINT32 Reserve1:10;		
	} field;
	
	UINT32 word;
} INTERNAL_1_STRUCT;
#endif

#define RF_DBG1					0x050C
#define RF_CONTROL0				0x0518
#define RTMP_RF_BYPASS0				0x051C
#define RF_CONTROL1				0x0520
#define RF_BYPASS1				0x0524
#define RF_CONTROL2				0x0528
#define RF_BYPASS2				0x052C
#define RF_CONTROL3				0x0530
#define RF_BYPASS3				0x0534

#define LDO_CFG0 				0x05d4
#define GPIO_SWITCH				0x05dc

#define DEBUG_INDEX				0x05e8	



#endif /* __RAL_OMAC_RF_CTRL_H__ */

