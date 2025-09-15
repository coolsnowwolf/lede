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
	wf_arb.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __WF_ARB_H__
#define __WF_ARB_H__

#define WF_ARB_BASE		0x21400



/*********************************************************
	Tx Control Registers
**********************************************************/

#define ARB_TQCR0		WF_ARB_BASE + 0x100	/* 0x21500 */
#define ARB_TQCR1		WF_ARB_BASE + 0x104	/* 0x21504 */
#define ARB_TQCR2		WF_ARB_BASE + 0x108	/* 0x21508 */
#define ARB_TQCR3		WF_ARB_BASE + 0x10c	/* 0x2150c */
#define ARB_TQCR4		WF_ARB_BASE + 0x110	/* 0x21510 */
#define ARB_TQCR5		WF_ARB_BASE + 0x114	/* 0x21514 */
#define ARB_BCNQCR0		(WF_ARB_BASE + 0x118)	/* 0x21518 */
#define ARB_BCNQCR1		(WF_ARB_BASE + 0x11c)	/* 0x2151c */
#define ARB_BMCQCR0		(WF_ARB_BASE + 0x120)	/* 0x21520 */
#define ARB_BMCQCR1		(WF_ARB_BASE + 0x124)	/* 0x21524 */
#define ARB_BMCQCR2		(WF_ARB_BASE + 0x128)	/* 0x21528 */
#define ARB_BMCQCR3		(WF_ARB_BASE + 0x12c)	/* 0x2152c */
#define ARB_BMCQCR4		(WF_ARB_BASE + 0x130)	/* 0x21530 */
#define ARB_TXENTRYR		(WF_ARB_BASE + 0x134)	/* 0x21534 */
#define ARB_BTIMCR0		(WF_ARB_BASE + 0x138)	/* 0x21538 */
#define ARB_BTIMCR1		(WF_ARB_BASE + 0x13c)	/* 0x2153c */
#define ARB_GTQR0		(WF_ARB_BASE + 0x010)	/* 0x21410 */
#define ARB_GTQR1		(WF_ARB_BASE + 0x014)	/* 0x21414 */
#define ARB_GTQR2		(WF_ARB_BASE + 0x018)	/* 0x21418 */
#define ARB_GTQR3		(WF_ARB_BASE + 0x01c)	/* 0x2141c */
#define ARB_AIFSR0		(WF_ARB_BASE + 0x020)	/* 0x21420 */
#define ARB_AIFSR1		(WF_ARB_BASE + 0x024)	/* 0x21424 */
#define ARB_ACCWXR0		(WF_ARB_BASE + 0x028)	/* 0x21428 */
#define ARB_ACCWXR1		(WF_ARB_BASE + 0x02c)	/* 0x2142c */
#define ARB_ACCWXR2		(WF_ARB_BASE + 0x030)	/* 0x21430 */
#define ARB_ACCWXR3		(WF_ARB_BASE + 0x034)	/* 0x21434 */
#define ARB_ACCWXR4		(WF_ARB_BASE + 0x038)	/* 0x21438 */
#define ARB_ACCWXR5		(WF_ARB_BASE + 0x03c)	/* 0x2143c */
#define ARB_ACCWIR0		(WF_ARB_BASE + 0x040)	/* 0x21440 */
#define ARB_ACCWIR1		(WF_ARB_BASE + 0x044)	/* 0x21444 */
#define ARB_ACCWIR2		(WF_ARB_BASE + 0x048)	/* 0x21448 */
#define ARB_ACCWIR3		(WF_ARB_BASE + 0x04c)	/* 0x2144c */
#define ARB_ACCCWR0		(WF_ARB_BASE + 0x050)	/* 0x21450 */
#define ARB_ACCCWR1		(WF_ARB_BASE + 0x054)	/* 0x21454 */
#define ARB_ACCCWR2		(WF_ARB_BASE + 0x058)	/* 0x21458 */
#define ARB_ACCCWR3		(WF_ARB_BASE + 0x05c)	/* 0x2145c */
#define ARB_ACCCWR4		(WF_ARB_BASE + 0x060)	/* 0x21460 */
#define ARB_ACCCWR5		(WF_ARB_BASE + 0x064)	/* 0x21464 */

/*********************************************************
	Rx Cotnrol Registers
**********************************************************/
#define ARB_RQCR		(WF_ARB_BASE + 0x070)	/* 0x21470 */
#define ARB_RQCR_RX_START		0x01
#define ARB_RQCR_RXV_START		0x10
#define ARB_RQCR_RXV_R_EN		0x80
#define ARB_RQCR_RXV_T_EN		0x100


/*********************************************************
	System Cotnrol Registers
**********************************************************/
#define ARB_SCR		(WF_ARB_BASE + 0x080)  /* 0x21480 */
#define MT_ARB_SCR_TXDIS        BIT8
#define MT_ARB_SCR_RXDIS        BIT9
#define ARB_SCR_BCNQ_EMPTY_CTRL BIT28
#define ARB_SCR_TBTT_BTIM_CTRL  BIT29
#define ARB_SCR_TBTT_BCN_CTRL   BIT30
#define ARB_SCR_TBTT_BM_CTRL    BIT31

//ARB_SCR_BCNQ_OP_MODE
#define BCNQ_OP_MODE_STA	0x0
#define BCNQ_OP_MODE_AP     0x1
#define BCNQ_OP_MODE_ADHOC  0x2
#define ARB_SXCR0		(WF_ARB_BASE + 0x084)	/* 0x21484 */
#define ARB_SXCR1		(WF_ARB_BASE + 0x088)	/* 0x21488 */
#define ARB_SXCR2		(WF_ARB_BASE + 0x08c)	/* 0x2148c */



/*********************************************************
	PTA Control Registers
**********************************************************/



/*********************************************************
	PTA Test Registers
**********************************************************/



#endif /* __WF_ARB_H__ */

