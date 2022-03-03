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
	wf_int_wakeup_top.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------
----------------------------------------------
*/
#ifndef __WF_INT_WAKEUP_TOP_H__
#define __WF_INT_WAKEUP_TOP_H__

/* In range of 0x24000 ~ 0x24400 */
#define WF_INT_WAKEUP_TOP_BASE		0x24600

#define WISR0		(WF_INT_WAKEUP_TOP_BASE + 0x00)		/* 0x24600 */
#define WISR1		(WF_INT_WAKEUP_TOP_BASE + 0x04)
#define WISR2		(WF_INT_WAKEUP_TOP_BASE + 0x24)
#define WISR3		(WF_INT_WAKEUP_TOP_BASE + 0x2c)
#define WISR4		(WF_INT_WAKEUP_TOP_BASE + 0x34)

#define WIER0		(WF_INT_WAKEUP_TOP_BASE + 0x08)		/* 0x24608 */
#define WIER1		(WF_INT_WAKEUP_TOP_BASE + 0x0c)
#define WIER2		(WF_INT_WAKEUP_TOP_BASE + 0x28)
#define WIER3		(WF_INT_WAKEUP_TOP_BASE + 0x30)		/* 0x24630 */
#define WIER4		(WF_INT_WAKEUP_TOP_BASE + 0x38)

#define AHB_CLK_CTL	(WF_INT_WAKEUP_TOP_BASE + 0x20)		/* 0x24620 */

#define HWISR0		(WF_INT_WAKEUP_TOP_BASE + 0x3c)		/* 0x2463c */
#define HWISR1		(WF_INT_WAKEUP_TOP_BASE + 0x44)		/* 0x24644 */
#define HWISR2		(WF_INT_WAKEUP_TOP_BASE + 0x4c)		/* 0x2464c */
#define HWISR3		(WF_INT_WAKEUP_TOP_BASE + 0x54)		/* 0x24654 */
#define HWISR4		(WF_INT_WAKEUP_TOP_BASE + 0x5c)		/* 0x2465C */


#define HWIER0_TBTT1    BIT20
#define HWIER0_PRETBTT1 BIT23
#define HWIER0_TBTT2    BIT21
#define HWIER0_PRETBTT2 BIT24
#define HWIER0_TBTT3    BIT22
#define HWIER0_PRETBTT3 BIT25

#define HWIER0_TTTT1    BIT26
#define HWIER0_PRETTTT1 BIT29
#define HWIER0_TTTT2    BIT27
#define HWIER0_PRETTTT2 BIT30
#define HWIER0_TTTT3    BIT28
#define HWIER0_PRETTTT3 BIT31

#define HWIER0		(WF_INT_WAKEUP_TOP_BASE + 0x40)		/* 0x24640 */
#define HWIER1		(WF_INT_WAKEUP_TOP_BASE + 0x48)		/* 0x24648 */
#define HWIER2		(WF_INT_WAKEUP_TOP_BASE + 0x50)		/* 0x24650 */

#define HWIER3_TBTT0    BIT15
#define HWIER3_PRETBTT0 BIT31
#define HWIER3		(WF_INT_WAKEUP_TOP_BASE + 0x58)		/* 0x24658 */

#define HWIER4_TTTT0    BIT15
#define HWIER4_PRETTTT0 BIT31
#define HWIER4		(WF_INT_WAKEUP_TOP_BASE + 0x60)		/* 0x24660 */

#endif /* __WF_INT_WAKEUP_TOP_H__ */

