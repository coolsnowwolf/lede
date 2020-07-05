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
	wf_wtbloff.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __WF_WTBL_OFF_H__
#define __WF_WTBL_OFF_H__

#define WF_WTBL_OFF_BASE		0x23400

#define WTBL_OFF_WIUCR			(WF_WTBL_OFF_BASE + 0x30)	/* 0x23430 */
#define WLAN_IDX_MASK			(0xff)
#define WLAN_IDX(p)				(((p) & 0xff))
#define WTBL_RXINFO_UPDATE	(1 << 11)
#define WTBL_ADM_CNT_CLEAR			(1 << 12)
#define WTBL_RATE_UPDATE				(1 << 13)
#define WTBL_TX_CNT_CLEAR			(1 << 14)
#define WTBL_RX_CNT_CLEAR			(1 << 15)
#define WTBL_IU_BUSY					(1 << 31)

#define WTBL_OFF_BCR			(WF_WTBL_OFF_BASE + 0x4)	/* 0x23404 */
#define BIP_EN				(1 << 0)

#define WTBL_OFF_RMVTCR		(WF_WTBL_OFF_BASE + 0x8)	/* 0x23408 */
#define RX_MV_MODE				(1 << 23)
#define RCPI_AVG_PARAM_MASK	(0x3 << 20)
#define RCPI_AVG_PARAM(p)		(((p) & 0x3) << 20)

#define WTBL_OFF_RCR	(WF_WTBL_OFF_BASE + 0x0)	/* 0x23400 */
#define CHECK_CTRL(p) ((p & 0x1))
#define CHECK_CTRL_MASK (0x1)

#define WTBL_OFF_ACR	(WF_WTBL_OFF_BASE + 0xC)	/* 0x2340C */
#define WTBL_ADM_RX_EN                           (1 << 31)

#endif /* __WF_WTBL_OFF_H__ */

