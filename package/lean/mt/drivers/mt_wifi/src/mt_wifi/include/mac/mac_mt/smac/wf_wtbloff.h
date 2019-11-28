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

#define WF_WTBL_OFF_BASE		0x23000
#define WTBL_OFF_WIUCR			(WF_WTBL_OFF_BASE + 0x0)	/* 0x23000 */
#define WLAN_IDX_MASK			(0xff)
#define WLAN_IDX(p)				(((p) & 0xff))
#define WTBL2_UPDATE_FLAG		(1 << 11)
#define ADM_CNT_CLEAR			(1 << 12)
#define RATE_UPDATE				(1 << 13)
#define TX_CNT_CLEAR			(1 << 14)
#define RX_CNT_CLEAR			(1 << 15)
#define IU_BUSY					(1 << 16)

#define WTBL_OFF_BCR			(WF_WTBL_OFF_BASE + 0x4)	/* 0x23204 */
#define BIP_EN				(1 << 0)

#define WTBL_OFF_RMVTCR		(WF_WTBL_OFF_BASE + 0x8)	/* 0x23008 */
#define RX_MV_MODE				(BIT23)
#define RCPI_AVG_PARAM_MASK	(0x3 << 20)
#define RCPI_AVG_PARAM(p)		(((p) & 0x3) << 20)

#endif /* __WF_WTBL_OFF_H__ */

