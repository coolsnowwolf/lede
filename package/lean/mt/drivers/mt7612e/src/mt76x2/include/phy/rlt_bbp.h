/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:

	Abstract:

	Revision History:
	Who 		When			What
	--------	----------		----------------------------------------------
*/


#ifndef __RLT_BBP_H__
#define __RLT_BBP_H__


#define BBP_CORE	0x2000
#define BBP_IBI		0x2100
#define BBP_AGC1	0x2300
#define BBP_TXC		0x2400
#define BBP_RXC		0x2500
#define BBP_TXO		0x2600
#define BBP_TXBE	0x2700
#define BBP_RXFE	0x2800
#define BBP_RXO		0x2900
#define BBP_DFS		0x2a00
#define BBP_TR		0x2b00
#define BBP_CAL		0x2c00
#define BBP_DSC		0x2e00
#define BBP_PFMU	0x2f00


/* 0x2000 ~ */
#define CORE_R0		(BBP_CORE + 0x00)
#define CORE_R1		(BBP_CORE + 0x04)
#define CORE_R4		(BBP_CORE + 0x10)
#define CORE_R24	(BBP_CORE + 0x60)
#define CORE_R32	(BBP_CORE + 0x80)
#define CORE_R33	(BBP_CORE + 0x84)
#define CORE_R34	(BBP_CORE + 0x88)
#define CORE_R35	(BBP_CORE + 0x8c)
#define CORE_R42	(BBP_CORE + 0xa8)
#define CORE_R44	(BBP_CORE + 0xb0)

/* 0x2100 ~ */
#define IBI_R0		(BBP_IBI + 0x00)
#define IBI_R1		(BBP_IBI + 0x04)
#define IBI_R2		(BBP_IBI + 0x08)
#define IBI_R3		(BBP_IBI + 0x0c)
#define IBI_R4		(BBP_IBI + 0x10)
#define IBI_R5		(BBP_IBI + 0x14)
#define IBI_R6		(BBP_IBI + 0x18)
#define IBI_R7		(BBP_IBI + 0x1c)
#define IBI_R9		(BBP_IBI + 0x24)
#define IBI_R11		(BBP_IBI + 0x2c)
#define IBI_R15     (BBP_IBI + 0x3c)

/* 0x2300 ~ */
#define AGC1_R0		(BBP_AGC1 + 0x00)
#define AGC1_R0_PRIMARY_CHL_INDEX_MASK (0x3 << 8)
#define AGC1_R0_RPIMARY_CHL_INDEX(p) (((p) & 0x3) << 8)
#define GET_PRIMARY_CHL_INDEX(p) (((p) & AGC1_R0_PRIMARY_CHL_INDEX_MASK) >> 8)

#define AGC1_R1		(BBP_AGC1 + 0x04)
#define AGC1_R2		(BBP_AGC1 + 0x08)
#define AGC1_R4		(BBP_AGC1 + 0x10)
#define AGC1_R5		(BBP_AGC1 + 0x14)
#define AGC1_R6		(BBP_AGC1 + 0x18)
#define AGC1_R7		(BBP_AGC1 + 0x1C)
#define AGC1_R8		(BBP_AGC1 + 0x20)
#define AGC1_R9		(BBP_AGC1 + 0x24)
#define AGC1_R11	(BBP_AGC1 + 0x2C)
#define AGC1_R12	(BBP_AGC1 + 0x30)
#define AGC1_R13	(BBP_AGC1 + 0x34)
#define AGC1_R14	(BBP_AGC1 + 0x38)
#define AGC1_R15	(BBP_AGC1 + 0x3C)
#define AGC1_R16	(BBP_AGC1 + 0x40)
#define AGC1_R18	(BBP_AGC1 + 0x48)
#define AGC1_R19	(BBP_AGC1 + 0x4c)
#define AGC1_R20	(BBP_AGC1 + 0x50)
#define AGC1_R21	(BBP_AGC1 + 0x54)
#define AGC1_R22	(BBP_AGC1 + 0x58)
#define AGC1_R23	(BBP_AGC1 + 0x5c)
#define AGC1_R24	(BBP_AGC1 + 0x60)
#define AGC1_R25	(BBP_AGC1 + 0x64)
#define AGC1_R26	(BBP_AGC1 + 0x68)
#define AGC1_R27	(BBP_AGC1 + 0x6c)
#define AGC1_R28	(BBP_AGC1 + 0x70)
#define AGC1_R30	(BBP_AGC1 + 0x78)
#define AGC1_R31	(BBP_AGC1 + 0x7c)
#define AGC1_R32	(BBP_AGC1 + 0x80)
#define AGC1_R33	(BBP_AGC1 + 0x84)
#define AGC1_R34	(BBP_AGC1 + 0x88)
#define AGC1_R35	(BBP_AGC1 + 0x8c)
#define AGC1_R37	(BBP_AGC1 + 0x94)
#define AGC1_R39	(BBP_AGC1 + 0x9c)
#define AGC1_R41	(BBP_AGC1 + 0xa4)
#define AGC1_R43	(BBP_AGC1 + 0xac)
#define AGC1_R45	(BBP_AGC1 + 0xb4)
#define AGC1_R47	(BBP_AGC1 + 0xbc)
#define AGC1_R49	(BBP_AGC1 + 0xc4)
#define AGC1_R51	(BBP_AGC1 + 0xcc)
#define AGC1_R53	(BBP_AGC1 + 0xd4)
#define AGC1_R55	(BBP_AGC1 + 0xdc)
#define AGC1_R57	(BBP_AGC1 + 0xe4)
#define AGC1_R58	(BBP_AGC1 + 0xe8)
#define AGC1_R59	(BBP_AGC1 + 0xec)
#define AGC1_R60	(BBP_AGC1 + 0xf0)
#define AGC1_R61	(BBP_AGC1 + 0xf4)
#define AGC1_R62	(BBP_AGC1 + 0xf8)
#define AGC1_R63	(BBP_AGC1 + 0xfc)

/* 0x2400 ~ */
#define TXC_R0		(BBP_TXC + 0x00)
#define TXC_R1		(BBP_TXC + 0x04)
#define TXC_R2		(BBP_TXC + 0x08)

/* 0x2500 ~ */
#define RXC_R0		(BBP_RXC + 0x00)
#define RXC_R1		(BBP_RXC + 0x04)
#define RXC_R2		(BBP_RXC + 0x08)
#define RXC_R3		(BBP_RXC + 0x0c)
#define RXC_R4		(BBP_RXC + 0x10)
#define RXC_R5		(BBP_RXC + 0x14)
#define RXC_R7		(BBP_RXC + 0x1C)
#define RXC_R9      (BBP_RXC + 0x28)

/* 0x2600 ~ */
#define TXO_R0		(BBP_TXO + 0x00)
#define TXO_R1		(BBP_TXO + 0x04)
#define TXO_R2		(BBP_TXO + 0x08)
#define TXO_R3		(BBP_TXO + 0x0c)
#define TXO_R4		(BBP_TXO + 0x10)
#define TXO_R5		(BBP_TXO + 0x14)
#define TXO_R6		(BBP_TXO + 0x18)
#define TXO_R7		(BBP_TXO + 0x1c)
#define TXO_R8		(BBP_TXO + 0x20)
#define TXO_R13     (BBP_TXO + 0x34)

/* 0x2700 ~ */
#define TXBE_R0		(BBP_TXBE + 0x00)
#define TXBE_R1		(BBP_TXBE + 0x04)
#define TXBE_R2		(BBP_TXBE + 0x08)
#define TXBE_R3		(BBP_TXBE + 0x0c)
#define TXBE_R4		(BBP_TXBE + 0x10)
#define TXBE_R5		(BBP_TXBE + 0x14)
#define TXBE_R6		(BBP_TXBE + 0x18)
#define TXBE_R8		(BBP_TXBE + 0x20)
#define TXBE_R9		(BBP_TXBE + 0x24)
#define TXBE_R10	(BBP_TXBE + 0x28)
#define TXBE_R12	(BBP_TXBE + 0x30)
#define TXBE_R13	(BBP_TXBE + 0x34)
#define TXBE_R14	(BBP_TXBE + 0x38)
#define TXBE_R15	(BBP_TXBE + 0x3c)
#define TXBE_R16	(BBP_TXBE + 0x40)
#define TXBE_R17	(BBP_TXBE + 0x44)
#define TXBE_R52    (BBP_TXBE + 0xd0)

/* 0x2800 ~ */
#define RXFE_R0		(BBP_RXFE + 0x00)
#define RXFE_R1		(BBP_RXFE + 0x04)
#define RXFE_R2		(BBP_RXFE + 0x08)
#define RXFE_R3		(BBP_RXFE + 0x0c)
#define RXFE_R4		(BBP_RXFE + 0x10)
#define RXFE_R17    (BBP_RXFE + 0x44)

/* 0x2900 ~ */
#define RXO_R0      (BBP_RXO + 0x00)
#define RXO_R9		(BBP_RXO + 0x24)
#define RXO_R13		(BBP_RXO + 0x34)
#define RXO_R14		(BBP_RXO + 0x38)
#define RXO_R15		(BBP_RXO + 0x3c)
#define RXO_R16		(BBP_RXO + 0x40)
#define RXO_R17		(BBP_RXO + 0x44)
#define RXO_R18		(BBP_RXO + 0x48)
#define RXO_R19		(BBP_RXO + 0x4C)
#define RXO_R20		(BBP_RXO + 0x50)
#define RXO_R21		(BBP_RXO + 0x54)
#define RXO_R24		(BBP_RXO + 0x60)
#define RXO_R28		(BBP_RXO + 0x70)
#define RXO_R29		(BBP_RXO + 0x74)
#define RXO_R63     (BBP_RXO + 0xfc)

/* 0x2a00 ~ */
#define DFS_R0		(BBP_DFS + 0x00)
#define DFS_R1		(BBP_DFS + 0x04)
#define DFS_R2		(BBP_DFS + 0x08)
#define DFS_R3		(BBP_DFS + 0x0c)
#define DFS_R4		(BBP_DFS + 0x10)
#define DFS_R5		(BBP_DFS + 0x14)
#define DFS_R7		(BBP_DFS + 0x1c)
#define DFS_R9		(BBP_DFS + 0x24)
#define DFS_R11		(BBP_DFS + 0x2c)
#define DFS_R13		(BBP_DFS + 0x34)
#define DFS_R14		(BBP_DFS + 0x38)
#define DFS_R15		(BBP_DFS + 0x3c)
#define DFS_R17		(BBP_DFS + 0x44)
#define DFS_R19		(BBP_DFS + 0x4c)
#define DFS_R20		(BBP_DFS + 0x50)
#define DFS_R22		(BBP_DFS + 0x58)
#define DFS_R23		(BBP_DFS + 0x5c)
#define DFS_R25		(BBP_DFS + 0x64)
#define DFS_R26		(BBP_DFS + 0x68)
#define DFS_R28		(BBP_DFS + 0x70)
#define DFS_R30		(BBP_DFS + 0x78)
#define DFS_R31		(BBP_DFS + 0x7c)
#define DFS_R32		(BBP_DFS + 0x80)
#define DFS_R36		(BBP_DFS + 0x90)
#define DFS_R37		(BBP_DFS + 0x94)

/* 0x2b00 ~ */
#define TR_R0		(BBP_TR + 0x00)
#define TR_R1		(BBP_TR + 0x04)
#define TR_R2		(BBP_TR + 0x08)
#define TR_R3		(BBP_TR + 0x0c)
#define TR_R4		(BBP_TR + 0x10)
#define TR_R5		(BBP_TR + 0x14)
#define TR_R6		(BBP_TR + 0x18)
#define TR_R9       (BBP_TR + 0x24)

/* 0x2c00 ~ */
#define CAL_R0      (BBP_CAL + 0x00)
#define CAL_R1      (BBP_CAL + 0x04)
#define CAL_R2		(BBP_CAL + 0x08)
#define CAL_R3		(BBP_CAL + 0x0c)
#define CAL_R4		(BBP_CAL + 0x10)
#define CAL_R5		(BBP_CAL + 0x14)
#define CAL_R6		(BBP_CAL + 0x18)
#define CAL_R7		(BBP_CAL + 0x1C)
#define CAL_R8		(BBP_CAL + 0x20)
#define CAL_R9		(BBP_CAL + 0x24)
#define CAL_R10		(BBP_CAL + 0x28)
#define CAL_R11		(BBP_CAL + 0x2C)
#define CAL_R12		(BBP_CAL + 0x30)
#define CAL_R13		(BBP_CAL + 0x34)
#define CAL_R14		(BBP_CAL + 0x38)
#define CAL_R15		(BBP_CAL + 0x3C)
#define CAL_R16		(BBP_CAL + 0x40)
#define CAL_R17		(BBP_CAL + 0x44)
#define CAL_R18		(BBP_CAL + 0x48)
#define CAL_R19		(BBP_CAL + 0x4C)
#define CAL_R20		(BBP_CAL + 0x50)
#define CAL_R21		(BBP_CAL + 0x54)
#define CAL_R22		(BBP_CAL + 0x58)
#define CAL_R23		(BBP_CAL + 0x5C)
#define CAL_R24		(BBP_CAL + 0x60)
#define CAL_R25		(BBP_CAL + 0x64)
#define CAL_R26		(BBP_CAL + 0x68)
#define CAL_R27		(BBP_CAL + 0x6C)
#define CAL_R28		(BBP_CAL + 0x70)
#define CAL_R29		(BBP_CAL + 0x74)
#define CAL_R30		(BBP_CAL + 0x78)
#define CAL_R31		(BBP_CAL + 0x7C)
#define CAL_R32		(BBP_CAL + 0x80)
#define CAL_R33		(BBP_CAL + 0x84)
#define CAL_R34		(BBP_CAL + 0x88)
#define CAL_R35		(BBP_CAL + 0x8C)
#define CAL_R36		(BBP_CAL + 0x90)
#define CAL_R37		(BBP_CAL + 0x94)
#define CAL_R38		(BBP_CAL + 0x98)
#define CAL_R39		(BBP_CAL + 0x9C)
#define CAL_R40		(BBP_CAL + 0xA0)
#define CAL_R41		(BBP_CAL + 0xA4)
#define CAL_R42		(BBP_CAL + 0xA8)
#define CAL_R43		(BBP_CAL + 0xAC)
#define CAL_R44		(BBP_CAL + 0xB0)
#define CAL_R45		(BBP_CAL + 0xB4)
#define CAL_R46		(BBP_CAL + 0xB8)
#define CAL_R47		(BBP_CAL + 0xBC)
#define CAL_R48		(BBP_CAL + 0xC0)
#define CAL_R49		(BBP_CAL + 0xC4)
#define CAL_R50		(BBP_CAL + 0xC8)
#define CAL_R51		(BBP_CAL + 0xCC)
#define CAL_R52		(BBP_CAL + 0xD0)
#define CAL_R53		(BBP_CAL + 0xD4)
#define CAL_R54		(BBP_CAL + 0xD8)
#define CAL_R55		(BBP_CAL + 0xDC)
#define CAL_R56		(BBP_CAL + 0xE0)
#define CAL_R57		(BBP_CAL + 0xE4)
#define CAL_R58		(BBP_CAL + 0xE8)
#define CAL_R59		(BBP_CAL + 0xEC)
#define CAL_R60		(BBP_CAL + 0xF0)
#define CAL_R61		(BBP_CAL + 0xF4)
#define CAL_R62		(BBP_CAL + 0xF8)
#define CAL_R63		(BBP_CAL + 0xFC)
#define CAL_R64		(BBP_CAL + 0x100)
#define CAL_R65		(BBP_CAL + 0x104)
#define CAL_R66		(BBP_CAL + 0x108)
#define CAL_R67		(BBP_CAL + 0x10C)
#define CAL_R68		(BBP_CAL + 0x110)
#define CAL_R69		(BBP_CAL + 0x114)
#define CAL_R70		(BBP_CAL + 0x118)

/* 0x2e00 ~ */
#define DSC_R0		(BBP_DSC + 0x00)
#define DSC_R8		(BBP_DSC + 0x20)
#define DSC_R10     (BBP_DSC + 0x28)

/* 0x2f00 ~ */
#define PFMU_R0     (BBP_PFMU + 0x00)
#define PFMU_R1     (BBP_PFMU + 0x04)
#define PFMU_R8     (BBP_PFMU + 0x20)
#define PFMU_R9     (BBP_PFMU + 0x24)
#define PFMU_R10    (BBP_PFMU + 0x28)
#define PFMU_R11    (BBP_PFMU + 0x2C)
#define PFMU_R12    (BBP_PFMU + 0x30)
#define PFMU_R13    (BBP_PFMU + 0x34)
#define PFMU_R14    (BBP_PFMU + 0x38)
#define PFMU_R15    (BBP_PFMU + 0x3C)
#define PFMU_R19	(BBP_PFMU + 0x4C)
#define PFMU_R20	(BBP_PFMU + 0x50)
#define PFMU_R21	(BBP_PFMU + 0x54)
#define PFMU_R22	(BBP_PFMU + 0x58)
#define PFMU_R23	(BBP_PFMU + 0x5C)
#define PFMU_R51	(BBP_PFMU + 0xCC)
#define PFMU_R54	(BBP_PFMU + 0xD8)
#define PFMU_R57    (BBP_PFMU + 0xE4)


#define RTMP_BBP_IO_READ32(_p, _i, _pV)	RTMP_IO_READ32(_p, _i, _pV)
#define RTMP_BBP_IO_WRITE32(_p, _i, _v)	RTMP_IO_WRITE32(_p, _i, _v)


struct _RTMP_ADAPTER;
INT rlt_phy_probe(struct _RTMP_ADAPTER *pAd);

#endif /* __RLT_BBP_H__ */

