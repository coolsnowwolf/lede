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
	wf_cfg.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __WF_CFG_H__
#define __WF_CFG_H__


#define WF_CFG_BASE		0x20200

#define CFG_CCR			        (WF_CFG_BASE)

#define CFG_DBG_EN				(WF_CFG_BASE + 0x14)
#define CFG_DBG_L1_MOD_SEL		(WF_CFG_BASE + 0x18)
#define CFG_DBG0				(WF_CFG_BASE + 0x24)
#define CFG_DBG_L1_SEL			(WF_CFG_BASE + 0x4c)

#define CFG_DBDC_CTRL0			(WF_CFG_BASE + 0x50)
#define CFG_DBDC_CTRL1			(WF_CFG_BASE + 0x54)


/*DBDC_CTRL0*/
#define DBDC_EN_MASK (0x1)
#define DBDC_EN(p) (((p) & 0x1) << 31)
#define DBDC_EN_BIT_BASE 31

#define PTA_BAND_SELPTA_MASK (0x1)
#define PTA_BAND_SELPTA(p) (((p) & 0x1) << 30)
#define PTA_BAND_SELPTA_BIT_BASE 30

#define MU_BAND_SEL_MASK (0x1)
#define MU_BAND_SEL(p) (((p) & 0x1) << 29)

#define MU_BAND_SEL_BIT_BASE 29

#define BEFEE_BAND_SEL_MASK (0x1)
#define BEFEE_BAND_SEL(p) (((p) & 0x1) << 28)

#define BEFER_BAND_SEL_MASK (0x1)
#define BEFER_BAND_SEL(p) (((p) & 0x1) << 27)

#define RXBF_BAND_SEL_MASK (0x1)
#define RXBF_BAND_SEL(p) (((p) & 0x1) << 26)

#define BF_BAND_SEL_BIT_BASE 26

#define WMM_0TO3_BAND_SEL_MASK (0xf)
#define WMM_0TO3_BAND_SEL(p) (((p) & 0xf) << 22)

#define WMM_BAND_SEL_BIT_BASE 22

#define MNG_BAND_SEL_MASK (0x3)
#define MNG_BAND_SEL(p) (((p) & 0x3) << 20)

#define MNG_BAND_SEL_BIT_BASE 20

#define OM_11TO1F_BAND_SEL_MASK (0xefff)
#define OM_11TO1F_BAND_SEL(p) (((p) & 0xefff) << 5)

#define MBSS_BAND_SEL_BIT_BASE 5

#define OM_00TO04_BAND_SEL_MASK (0x1f)
#define OM_00TO04_BAND_SEL(p) (((p) & 0x1f))

#define BSS_BAND_SEL_BIT_BASE 0

/*DBDC_CTRL1*/
#define OM_20TO3F_BAND_SEL_MASK (0xffffffff)
#define OM_20TO3F_BAND_SEL(p) (((p)  & 0xffffffff))

#define REPEATER_BAND_SEL_BIT_BASE 0

#endif /* __WF_CFG_H__ */

