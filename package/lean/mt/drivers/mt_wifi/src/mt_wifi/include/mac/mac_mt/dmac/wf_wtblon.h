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
	wf_wtblon.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __WF_WTBL_ON_H__
#define __WF_WTBL_ON_H__

#define WF_WTBL_ON_BASE		0x23000

#define WTBL_ON_WTBLOR		(WF_WTBL_ON_BASE + 0x0)
#define WTBL_OR_PSM_W_FLAG	(1<<31)

#define WTBL_ON_RICR0		(WF_WTBL_ON_BASE + 0x10)	/* DW0 */
#define WTBL_ON_RICR1		(WF_WTBL_ON_BASE + 0x14)	/* DW1 */
#define WTBL_ON_RIUCR0		(WF_WTBL_ON_BASE + 0x20)	/* DW5 */
#define WTBL_ON_RIUCR1		(WF_WTBL_ON_BASE + 0x24)	/* DW6 */
#define WTBL_ON_RIUCR2		(WF_WTBL_ON_BASE + 0x28)	/* DW7 */
#define WTBL_ON_RIUCR3		(WF_WTBL_ON_BASE + 0x2C)	/* DW8 */

#define WTBL_ON_TCGSBR		(WF_WTBL_ON_BASE + 0x40)
#define WTBL_ON_ACGSBR		(WF_WTBL_ON_BASE + 0x44)
#define WTBL_ON_RVCDARx		(WF_WTBL_ON_BASE + 0x80)	/* RV bit for wtbl */

#define WTBL_BTCRn		(WF_WTBL_ON_BASE + 0x100)
#define WTBL_BTBCRn		(WF_WTBL_ON_BASE + 0x110)
#define WTBL_BRCRn		(WF_WTBL_ON_BASE + 0x120)
#define WTBL_BRBCRn		(WF_WTBL_ON_BASE + 0x130)
#define WTBL_BTDCRn		(WF_WTBL_ON_BASE + 0x140)
#define WTBL_BRDCRn		(WF_WTBL_ON_BASE + 0x150)

#define WTBL_MBTCRn		(WF_WTBL_ON_BASE + 0x200)
#define WTBL_MBTBCRn		(WF_WTBL_ON_BASE + 0x240)
#define WTBL_MBRCRn		(WF_WTBL_ON_BASE + 0x280)
#define WTBL_MBRBCRn		(WF_WTBL_ON_BASE + 0x2C0)


#define PSM_W_FLAG			(1 << 31)
#endif /* __WF_WTBL_ON_H__ */

