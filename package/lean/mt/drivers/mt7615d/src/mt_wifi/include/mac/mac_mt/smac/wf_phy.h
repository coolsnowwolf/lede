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
	wf_phy.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __WF_PHY_H__
#define __WF_PHY_H__

#define WF_PHY_BASE		    0x10000

/* WF_PHY_FRONT */
#define CR_PHYCK_CTRL       (WF_PHY_BASE + 0x0000)
#define CR_FR_CKG_CTRL      (WF_PHY_BASE + 0x0004)
#define CR_FR_RST_CTRL      (WF_PHY_BASE + 0x0008)

#define CR_TXPTN_00         (WF_PHY_BASE + 0x01A0)
#define CR_TXPTN_01         (WF_PHY_BASE + 0x01A4)

#define CR_RFINTF_00		(WF_PHY_BASE + 0x0200)
#define CR_RFINTF_CAL_NSS_MASK (0x3 << 28)
#define CR_RFINTF_CAL_NSS(p) (((p) & 0x3) << 28)
#define CR_RFINTF_01		(WF_PHY_BASE + 0x0204)

#define CR_RFINTF_02		(WF_PHY_BASE + 0x0208)
#define CR_RFINTF_03        (WF_PHY_BASE + 0x020C)

#define RO_AGC_DEBUG_2	    (WF_PHY_BASE + 0x05A4)

#define CR_RXTD_39			(WF_PHY_BASE + 0x06F8)
#define CR_FFT_MANU_CTRL    (WF_PHY_BASE + 0x0704)

#define CR_DPD_CAL_03       (WF_PHY_BASE + 0x090C)

#define CR_TXFE_3           (WF_PHY_BASE + 0x0A08)
#define CR_TXFE_4           (WF_PHY_BASE + 0x0A0C)
#define CR_TSSI_0           (WF_PHY_BASE + 0x0D00)
#define CR_TSSI_1           (WF_PHY_BASE + 0x0D04)
#define CR_TSSI_3			(WF_PHY_BASE + 0x0D14)
#define CR_TSSI_6			(WF_PHY_BASE + 0x0D18)
#define CR_TSSI_9           (WF_PHY_BASE + 0x0D24)
#define CR_TSSI_13			(WF_PHY_BASE + 0x0D34)

#define CR_WF1_RFINTF_01	(WF_PHY_BASE + 0x1204)
#define CR_WF1_RFINTF_02	(WF_PHY_BASE + 0x1208)

#define CR_TXFE1_3          (WF_PHY_BASE + 0x1A08)
#define CR_TXFE1_4          (WF_PHY_BASE + 0x1A0C)

#define CR_WF1_TSSI_0       (WF_PHY_BASE + 0x1D00)
#define CR_WF1_TSSI_1       (WF_PHY_BASE + 0x1D04)
#define CR_WF1_TSSI_3		(WF_PHY_BASE + 0x1D14)
#define CR_WF1_TSSI_6		(WF_PHY_BASE + 0x1D18)
#define CR_WF1_TSSI_9       (WF_PHY_BASE + 0x1D24)
#define CR_WF1_TSSI_13      (WF_PHY_BASE + 0x1D34)

/* WF_PHY_BACK */
#define CR_BK_RST_CTRL      (WF_PHY_BASE + 0x4004)

#define CR_PHYCTRL_0        (WF_PHY_BASE + 0x4100)
#define CR_PHYCTRL_2        (WF_PHY_BASE + 0x4108)
#define RO_PHYCTRL_STS0     (WF_PHY_BASE + 0x410C)
#define RO_PHYCTRL_STS1     (WF_PHY_BASE + 0x4110)
#define RO_PHYCTRL_STS2     (WF_PHY_BASE + 0x4114)
#define RO_PHYCTRL_STS4     (WF_PHY_BASE + 0x411C)
#define RO_PHYCTRL_STS5     (WF_PHY_BASE + 0x4120)

#define CR_PHYCTRL_DBGCTRL  (WF_PHY_BASE + 0x4140)

#define CR_PHYMUX_3         (WF_PHY_BASE + 0x420C)
#define CR_PHYMUX_5         (WF_PHY_BASE + 0x4214)
#define CR_PHYMUX_10        (WF_PHY_BASE + 0x4228)
#define CR_PHYMUX_11        (WF_PHY_BASE + 0x422C)
#define CR_PHYMUX_24        (WF_PHY_BASE + 0x4260)
#define CR_PHYMUX_25		(WF_PHY_BASE + 0x4264)
#define CR_PHYMUX_26		(WF_PHY_BASE + 0x4268)
#define CR_PHYMUX_27		(WF_PHY_BASE + 0x426C)
/* Tx Vector */
#define CR_PHYMUX_12    (WF_PHY_BASE + 0x4230)
#define CR_PHYMUX_13    (WF_PHY_BASE + 0x4234)
#define CR_PHYMUX_14    (WF_PHY_BASE + 0x4238)
#define CR_PHYMUX_15    (WF_PHY_BASE + 0x423c)
#define CR_PHYMUX_16    (WF_PHY_BASE + 0x4240)
#define CR_PHYMUX_17    (WF_PHY_BASE + 0x4244)

#define TXV1    CR_PHYMUX_12
#define TXV2    CR_PHYMUX_13
#define TXV3    CR_PHYMUX_14
#define TXV4    CR_PHYMUX_15
#define TXV5    CR_PHYMUX_16
#define TXV6    CR_PHYMUX_17


/* Rx Vector */
#define CR_PHYMUX_18    (WF_PHY_BASE + 0x4248)
#define CR_PHYMUX_19    (WF_PHY_BASE + 0x425c)
#define CR_PHYMUX_20    (WF_PHY_BASE + 0x4250)
#define CR_PHYMUX_21    (WF_PHY_BASE + 0x4254)
#define CR_PHYMUX_22    (WF_PHY_BASE + 0x4258)
#define CR_PHYMUX_23    (WF_PHY_BASE + 0x425c)

#define RXV1    CR_PHYMUX_18
#define RXV2    CR_PHYMUX_19
#define RXV3    CR_PHYMUX_20
#define RXV4    CR_PHYMUX_21
#define RXV5    CR_PHYMUX_22
#define RXV6    CR_PHYMUX_23


#define CR_TXFD_0           (WF_PHY_BASE + 0x4700)
#define CR_TXFD_1           (WF_PHY_BASE + 0x4704)
#define CR_TXFD_3           (WF_PHY_BASE + 0x470C)
#define CR_BF_CTRL0			(WF_PHY_BASE + 0x4800)
#define CR_BF_TIMEOUT		(WF_PHY_BASE + 0x4804)
#define CR_BF_CTRL1			(WF_PHY_BASE + 0x4808)
#define CR_BF_PFILE_ACCESS	(WF_PHY_BASE + 0x480C)
#define CR_BF_PFILE_WDATA0	(WF_PHY_BASE + 0x4810)
#define CR_BF_PFILE_WDATA1	(WF_PHY_BASE + 0x4814)
#define CR_BF_PFILE_WDATA2	(WF_PHY_BASE + 0x4818)
#define CR_BF_PFILE_WDATA3	(WF_PHY_BASE + 0x481C)
#define CR_BF_PFILE_WDATA4	(WF_PHY_BASE + 0x4820)
#define CR_BF_PFILE_WDATA5	(WF_PHY_BASE + 0x4824)
#define CR_BF_PFILE_RDATA0	(WF_PHY_BASE + 0x4828)
#define CR_BF_PFILE_RDATA1	(WF_PHY_BASE + 0x482C)
#define CR_BF_PFILE_RDATA2	(WF_PHY_BASE + 0x4830)
#define CR_BF_PFILE_RDATA3	(WF_PHY_BASE + 0x4834)
#define CR_BF_PFILE_RDATA4	(WF_PHY_BASE + 0x4838)
#define CR_BF_PFILE_RDATA5	(WF_PHY_BASE + 0x483C)
#define CR_BF_FIL_MSB0		(WF_PHY_BASE + 0x4844)
#define CR_BF_FIL_LSB0		(WF_PHY_BASE + 0x4848)

#define CR_PHYMUX_WF1_3     (WF_PHY_BASE + 0x520C)
#define CR_PHYMUX_WF1_5     (WF_PHY_BASE + 0x5214)



#define PHYBACK_CR_PHYMUX_27   (WF_PHY_BASE + 0x426C)
#define PHYBACK_CR_PHYMUX_27_MSK (0xf << 4)
#define SET_CR_PHYMUX_RXV_SEL_G2(mode) (((mode) & 0xf) << 4)

#endif /* __WF_PHY_H__ */
