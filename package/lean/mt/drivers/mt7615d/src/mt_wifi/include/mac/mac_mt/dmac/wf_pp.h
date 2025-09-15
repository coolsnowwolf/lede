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
	wf_pp.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __WF_PP_H__
#define __WF_PP_H__

#define PP_BASE		0xe000


/* CT path control register */
#define PP_RXCUTDISP					(PP_BASE + 0x54)
#define PP_RXCUTDISP_CT_EN_MASK		(1<<0)
#define PP_RXCUTDISP_CR4_EN_MASK		(1<<1)

/* */
#define PP_PAGECTL_0					(PP_BASE + 0x58)
#define PAGECTL_0_PSE_PG_CNT_MASK	(0xfff)

/* */
#define PP_PAGECTL_1					(PP_BASE + 0x5c)
#define PAGECTL_1_PLE_PG_CNT_MASK	(0xfff)

/* */
#define PP_PAGECTL_2					(PP_BASE + 0x60)
#define PAGECTL_2_CUT_PG_CNT_MASK	(0xfff)

/* PP spare dummy CR */
#define PP_SPARE_DUMMY_CR5				(PP_BASE + 0x64)
#define PP_SPARE_DUMMY_CR6				(PP_BASE + 0x68)
#define PP_SPARE_DUMMY_CR7				(PP_BASE + 0x6c)
#define PP_SPARE_DUMMY_CR8				(PP_BASE + 0x70)

#endif /* __WF_PP_H__ */

