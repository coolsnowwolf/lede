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
	wf_trb.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __WF_TRB_H__
#define __WF_TRB_H__

#define WF_TRB_BASE		0x20800

#define TRB_GCR			(WF_TRB_BASE + 0x000)

#define TRB_TRBTHR		(WF_TRB_BASE + 0x014)
#define TRB_RXBTHR		(WF_TRB_BASE + 0x01C)

#ifdef DBG
#define TRB_TXBSR		(WF_TRB_BASE + 0x020)
#define TRB_TXPSR		(WF_TRB_BASE + 0x024)

#define TRB_RXBSR		(WF_TRB_BASE + 0x040)
#define TRB_RXPSR0		(WF_TRB_BASE + 0x044)
#define TRB_RXPSR1		(WF_TRB_BASE + 0x048)
#define TRB_RXPSR2		(WF_TRB_BASE + 0x04c)
#endif /* DBG */
#endif /* __WF_TRB_H__ */

