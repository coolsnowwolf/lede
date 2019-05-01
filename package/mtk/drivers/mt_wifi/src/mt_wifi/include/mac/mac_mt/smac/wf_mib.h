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

#define WF_MIB_BASE		0x2C000

#define MIB_MSCR	(WF_MIB_BASE + 0x00)
#define MIB_MPBSCR	(WF_MIB_BASE + 0x04)
#define MIB_MSDR0	(WF_MIB_BASE + 0x08)
#define MIB_MSDR4	(WF_MIB_BASE + 0x18)
#define MIB_MSDR5	(WF_MIB_BASE + 0x1c)
#define MIB_MSDR6	(WF_MIB_BASE + 0x20)
#define MIB_MSDR7	(WF_MIB_BASE + 0x24)
#define MIB_MSDR8	(WF_MIB_BASE + 0x28)
#define MIB_MSDR9	(WF_MIB_BASE + 0x2c)
#define MIB_MSDR10	(WF_MIB_BASE + 0x30)
#define MIB_MSDR11	(WF_MIB_BASE + 0x34)
#define MIB_MSDR12	(WF_MIB_BASE + 0x38)
#define MIB_MSDR14	(WF_MIB_BASE + 0x40)
#define MIB_MSDR15	(WF_MIB_BASE + 0x44)
#define MIB_MSDR16	(WF_MIB_BASE + 0x48)
#define MIB_MSDR17	(WF_MIB_BASE + 0x4c)
#define MIB_MSDR18	(WF_MIB_BASE + 0x50)
#define MIB_MSDR22	(WF_MIB_BASE + 0x60)
#define MIB_MSDR23	(WF_MIB_BASE + 0x64)
#define MIB_MDR0	(WF_MIB_BASE + 0xa0)
#define MIB_MDR1	(WF_MIB_BASE + 0xa4)
#define MIB_MDR2	(WF_MIB_BASE + 0xa8)
#define MIB_MDR3	(WF_MIB_BASE + 0xac)
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

#endif /* __WF_SEC_H__ */

