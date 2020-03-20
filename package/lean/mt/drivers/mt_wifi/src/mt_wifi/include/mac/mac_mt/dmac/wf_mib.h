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
	wf_mib.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __WF_MIB_H__
#define __WF_MIB_H__


#define WF_MIB_BASE		0x24800

#define MIB_M0SCR0	(WF_MIB_BASE + 0x00)
#define MIB_M0PBSCR	(WF_MIB_BASE + 0x04)
#define MIB_M0SCR1	(WF_MIB_BASE + 0x08)
#define M0_MISC_CR	(WF_MIB_BASE + 0x0c)

#define MIB_M0SDR0	(WF_MIB_BASE + 0x10)
#define MIB_M0SDR3	(WF_MIB_BASE + 0x14)
#define MIB_M0SDR4	(WF_MIB_BASE + 0x18)
#define MIB_M0SDR5	(WF_MIB_BASE + 0x1c)
#define MIB_M0SDR6	(WF_MIB_BASE + 0x20)
#define MIB_M0SDR7	(WF_MIB_BASE + 0x24)
#define MIB_M0SDR8	(WF_MIB_BASE + 0x28)
#define MIB_M0SDR9	(WF_MIB_BASE + 0x2c)
#define MIB_M0SDR10	(WF_MIB_BASE + 0x30)
#define MIB_M0SDR11	(WF_MIB_BASE + 0x34)
#define MIB_M0SDR12	(WF_MIB_BASE + 0x38)
#define MIB_M0SDR14	(WF_MIB_BASE + 0x40)
#define MIB_M0SDR15	(WF_MIB_BASE + 0x44)
#define MIB_M0SDR16	(WF_MIB_BASE + 0x48)
#define MIB_M0SDR17	(WF_MIB_BASE + 0x4c)
#define MIB_M0SDR18	(WF_MIB_BASE + 0x50)
#define MIB_M0SDR19	(WF_MIB_BASE + 0x54)
#define MIB_M0SDR20	(WF_MIB_BASE + 0x58)
#define MIB_M0SDR21	(WF_MIB_BASE + 0x5c)
#define MIB_M0SDR22	(WF_MIB_BASE + 0x60)
#define MIB_M0SDR23	(WF_MIB_BASE + 0x64)
#define MIB_M0SDR29	(WF_MIB_BASE + 0x7c)

#define MIB_M0SDR34 (WF_MIB_BASE + 0x90)
#define MIB_M0SDR35 (WF_MIB_BASE + 0x94)

#define MIB_M0SDR36      (WF_MIB_BASE + 0x98)	/* 820FD098 */
#define MIB_M0SDR37      (WF_MIB_BASE + 0x9c)	/* 820FD09c */
#define MIB_M0SDR51      (WF_MIB_BASE + 0x1E0)	/* 820FD1E0 */

#define MIB_M0DR0	(WF_MIB_BASE + 0xa0)
#define MIB_M0DR1	(WF_MIB_BASE + 0xa4)
#define MIB_M0DR2	(WF_MIB_BASE + 0xa8)
#define MIB_M0DR3	(WF_MIB_BASE + 0xac)
#define MIB_M0DR4	(WF_MIB_BASE + 0xb0)
#define MIB_M0DR5	(WF_MIB_BASE + 0xb4)

#define MIB_M0DR6	(WF_MIB_BASE + 0xb8)
#define TX_DDLMT_RNG1_CNT_MASK (0xff)
#define GET_TX_DDLMT_RNG1_CNT(p) (((p) & TX_DDLMT_RNG1_CNT_MASK))
#define TX_DDLMT_RNG2_CNT_MASK (0xff << 16)
#define GET_TX_DDLMT_RNG2_CNT(p) (((p) & TX_DDLMT_RNG2_CNT_MASK) >> 16)

#define MIB_M0DR7	(WF_MIB_BASE + 0xbc)
#define TX_DDLMT_RNG3_CNT_MASK (0xff)
#define GET_TX_DDLMT_RNG3_CNT(p) (((p) & TX_DDLMT_RNG3_CNT_MASK))
#define TX_DDLMT_RNG4_CNT_MASK (0xff << 16)
#define GET_TX_DDLMT_RNG4_CNT(p) (((p) & TX_DDLMT_RNG4_CNT_MASK) >> 16)

#define MIB_M0DR8	(WF_MIB_BASE + 0xc0)
#define MIB_M0DR9	(WF_MIB_BASE + 0xc4)
#define MIB_M0DR10	(WF_MIB_BASE + 0xc8)
#define MIB_M0DR11	(WF_MIB_BASE + 0xcc)

#define MIB_MB0SDR0	(WF_MIB_BASE + 0x100)
#define MIB_MB1SDR0	(WF_MIB_BASE + 0x110)
#define MIB_MB2SDR0	(WF_MIB_BASE + 0x120)
#define MIB_MB3SDR0	(WF_MIB_BASE + 0x130)

#define MIB_MB0SDR1	(WF_MIB_BASE + 0x104)
#define MIB_MB1SDR1	(WF_MIB_BASE + 0x114)
#define MIB_MB2SDR1	(WF_MIB_BASE + 0x124)
#define MIB_MB3SDR1	(WF_MIB_BASE + 0x134)

#define MIB_MB0SDR2	(WF_MIB_BASE + 0x108)
#define MIB_MB1SDR2	(WF_MIB_BASE + 0x118)
#define MIB_MB2SDR2	(WF_MIB_BASE + 0x128)
#define MIB_MB3SDR2	(WF_MIB_BASE + 0x138)

#define MIB_MB0SDR3	(WF_MIB_BASE + 0x10c)
#define MIB_MB1SDR3	(WF_MIB_BASE + 0x11c)
#define MIB_MB2SDR3	(WF_MIB_BASE + 0x12c)
#define MIB_MB3SDR3	(WF_MIB_BASE + 0x13c)

#define MIB_MPDU_SR0	(WF_MIB_BASE + 0x190)
#define MIB_MPDU_SR1	(WF_MIB_BASE + 0x194)

#define MIB_M1SCR	(WF_MIB_BASE + 0x200)
#define MIB_M1PBSCR	(WF_MIB_BASE + 0x204)
#define MIB_M1SCR1	(WF_MIB_BASE + 0x208)
#define MIB_M1SDR3	(WF_MIB_BASE + 0x214)
#define MIB_M1SDR4	(WF_MIB_BASE + 0x218)
#define MIB_M1SDR10	(WF_MIB_BASE + 0x230)
#define MIB_M1SDR11	(WF_MIB_BASE + 0x234)
#define MIB_M1SDR16	(WF_MIB_BASE + 0x248)
#define MIB_M1SDR17	(WF_MIB_BASE + 0x24c)
#define MIB_M1SDR18	(WF_MIB_BASE + 0x250)
#define MIB_M1SDR20 (WF_MIB_BASE + 0x258)
#define MIB_M1SDR36      (WF_MIB_BASE + 0x298) /* 820FD298 */
#define MIB_M1SDR37      (WF_MIB_BASE + 0x29c) /* 820FD29c */
#endif /* __WF_SEC_H__ */

