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

#define WF_ARB_BASE		0x20c00


/*********************************************************
	Tx Control Registers
**********************************************************/

#define ARB_DCR			(WF_ARB_BASE + 0x60)	/* 0x20c60 */

#define ARB_TQSW0		(WF_ARB_BASE + 0x100)	/* 0x20d00 */
#define ARB_TQSW1		(WF_ARB_BASE + 0x104)	/* 0x20d04 */
#define ARB_TQSW2		(WF_ARB_BASE + 0x108)	/* 0x20d08 */
#define ARB_TQSW3		(WF_ARB_BASE + 0x10c)	/* 0x20d0c */
#define ARB_TQSM0		(WF_ARB_BASE + 0x110)	/* 0x20d10 */
#define ARB_TQSM1		(WF_ARB_BASE + 0x114)	/* 0x20d14 */
#define ARB_TQSE0		(WF_ARB_BASE + 0x118)	/* 0x20d18 */
#define ARB_TQSE1		(WF_ARB_BASE + 0x11c)	/* 0x20d1c */
#define ARB_TQFW0		(WF_ARB_BASE + 0x120)	/* 0x20d20 */
#define ARB_TQFW1		(WF_ARB_BASE + 0x124)	/* 0x20d24 */
#define ARB_TQFW2		(WF_ARB_BASE + 0x128)	/* 0x20d28 */
#define ARB_TQFW3		(WF_ARB_BASE + 0x12c)	/* 0x20d2c */
#define ARB_TQFM0		(WF_ARB_BASE + 0x130)	/* 0x20d30 */
#define ARB_TQFM1		(WF_ARB_BASE + 0x134)	/* 0x20d34 */
#define ARB_TQFE0		(WF_ARB_BASE + 0x138)	/* 0x20d38 */
#define ARB_TQFE1		(WF_ARB_BASE + 0x13c)	/* 0x20d3c */
#define ARB_TQPW0		(WF_ARB_BASE + 0x140)	/* 0x20d40 */
#define ARB_TQPW1		(WF_ARB_BASE + 0x144)	/* 0x20d44 */
#define ARB_TQPW2		(WF_ARB_BASE + 0x148)	/* 0x20d48 */
#define ARB_TQPW3		(WF_ARB_BASE + 0x14c)	/* 0x20d4c */
#define ARB_TQPM0		(WF_ARB_BASE + 0x150)	/* 0x20d50 */
#define ARB_TQPM1		(WF_ARB_BASE + 0x154)	/* 0x20d54 */
#define ARB_BTIMCR0		(WF_ARB_BASE + 0x158)	/* 0x20d58 */
#define ARB_BTIMCR1		(WF_ARB_BASE + 0x15c)	/* 0x20d5c */
#define ARB_BMCCR0		(WF_ARB_BASE + 0x160)	/* 0x20d60 */
#define ARB_BMCCR1		(WF_ARB_BASE + 0x164)	/* 0x20d64 */
#define ARB_BMCCR2		(WF_ARB_BASE + 0x168)	/* 0x20d68 */
#define ARB_BMCCR3		(WF_ARB_BASE + 0x16C)	/* 0x20d6C */
#define ARB_BMCCR4		(WF_ARB_BASE + 0x170)	/* 0x20d70 */
#define ARB_BFCR		(WF_ARB_BASE + 0x190)	/* 0x20d90 */
#define ARB_DRNGR0		(WF_ARB_BASE + 0x194)	/* 0x20d94 */
#define ARB_DRNGR1		(WF_ARB_BASE + 0x198)	/* 0x20d98 */


#define ARB_WMMAC00	(WF_ARB_BASE + 0x1a0)	/* 0x20da0 */
#define ARB_WMMAC01	(WF_ARB_BASE + 0x1a4)	/* 0x20da4 */
#define ARB_WMMAC02	(WF_ARB_BASE + 0x1a8)	/* 0x20da8 */
#define ARB_WMMAC03	(WF_ARB_BASE + 0x1ac)	/* 0x20dac */
#define ARB_WMMAC10	(WF_ARB_BASE + 0x1b0)	/* 0x20db0 */
#define ARB_WMMAC11	(WF_ARB_BASE + 0x1b4)	/* 0x20db4 */
#define ARB_WMMAC12	(WF_ARB_BASE + 0x1b8)	/* 0x20db8 */
#define ARB_WMMAC13	(WF_ARB_BASE + 0x1bc)	/* 0x20dbc */
#define ARB_WMMAC20	(WF_ARB_BASE + 0x1c0)	/* 0x20dc0 */
#define ARB_WMMAC21	(WF_ARB_BASE + 0x1c4)	/* 0x20dc4 */
#define ARB_WMMAC22	(WF_ARB_BASE + 0x1c8)	/* 0x20dc8 */
#define ARB_WMMAC23	(WF_ARB_BASE + 0x1cc)	/* 0x20dcc */
#define ARB_WMMAC30	(WF_ARB_BASE + 0x1d0)	/* 0x20dd0 */
#define ARB_WMMAC31	(WF_ARB_BASE + 0x1d4)	/* 0x20dd4 */
#define ARB_WMMAC32	(WF_ARB_BASE + 0x1d8)	/* 0x20dd8 */
#define ARB_WMMAC33	(WF_ARB_BASE + 0x1dc)	/* 0x20ddc */

#define ARB_WMMALTX0	(WF_ARB_BASE + 0x1e0)	/* 0x20de0 */
#define ARB_WMMBMC0	(WF_ARB_BASE + 0x1e4)	/* 0x20de4 */
#define ARB_WMMBCN0	(WF_ARB_BASE + 0x1e8)	/* 0x20de8 */
#define ARB_WMMALTX1	(WF_ARB_BASE + 0x1f0)	/* 0x20df0 */
#define ARB_WMMBMC1	(WF_ARB_BASE + 0x1f4)	/* 0x20df4 */
#define ARB_WMMBCN1	(WF_ARB_BASE + 0x1f8)	/* 0x20df8 */

#define ARB_WMMBCN_AIFS_DEFAULT_VALUE   (BIT0)
#define ARB_WMMBCN_CWMIN_DEFAULT_VALUE  (0x2 << 8)

/*********************************************************
	Rx Cotnrol Registers
**********************************************************/
#define ARB_RQCR		(WF_ARB_BASE + 0x070)	/* 0x20c70 */
#define ARB_RQCR_RX_START		0x01
#define ARB_RQCR_RXV_START		0x10
#define ARB_RQCR_RXV_R_EN		0x80
#define ARB_RQCR_RXV_T_EN		0x100
#define ARB_RQCR_RX1_START		0x010000
#define ARB_RQCR_RXV1_START	0x100000
#define ARB_RQCR_RXV1_R_EN		0x800000
#define ARB_RQCR_RXV1_T_EN		0x1000000


/*********************************************************
	System Cotnrol Registers
**********************************************************/
#define ARB_SCR		(WF_ARB_BASE + 0x080)  /* 0x20c80 */
#define MT_ARB_SCR_BM_CTRL      BIT31
#define MT_ARB_SCR_BCN_CTRL     BIT30
#define MT_ARB_SCR_BTIM_CTRL    BIT30
#define MT_ARB_SCR_BCN_EMPTY    BIT28
#define MT_ARB_SCR_TXDIS	    BIT8
#define MT_ARB_SCR_RXDIS        BIT9
#define MT_ARB_SCR_TX1DIS       BIT10
#define MT_ARB_SCR_RX1DIS       BIT11
#define MT_ARB_SCR_OPMODE_MASK  0x3

/* ARB_SCR_BCNQ_OP_MODE */
#define BCNQ_OP_MODE_STA	0x0
#define BCNQ_OP_MODE_AP     0x1
#define BCNQ_OP_MODE_ADHOC  0x2
#define ARB_SXCR0		(WF_ARB_BASE + 0x084)	/* 0x20c84 */
#define ARB_SXCR1		(WF_ARB_BASE + 0x088)	/* 0x20c88 */
#define ARB_SXCR2		(WF_ARB_BASE + 0x08c)	/* 0x20c8c */



/*********************************************************
	PTA Control Registers
**********************************************************/



/*********************************************************
	PTA Test Registers
**********************************************************/



#endif /* __WF_ARB_H__ */

