/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	efuse.h
*/
#ifndef __EFUSE_H__
#define __EFUSE_H__

/* eFuse registers */
#define EFUSE_CTRL				0x0580
#define EFUSE_DATA0				0x0590
#define EFUSE_DATA1				0x0594
#define EFUSE_DATA2				0x0598
#define EFUSE_DATA3				0x059c

#define EFUSE_CTRL_3290			0x24
#define EFUSE_DATA0_3290		0x28
#define EFUSE_DATA1_3290		0x2c
#define EFUSE_DATA2_3290		0x30
#define EFUSE_DATA3_3290		0x34

#define EFUSE_TAG				0x2fe


#define MT_EEF_BASE 0x81070000
#define MT_EE_CTRL (MT_EEF_BASE)
#define MT_EFUSE_CTRL (MT_EEF_BASE + 0x8)
#define MT_EFUSE_WDATA0 (MT_EEF_BASE + 0x10)
#define MT_EFUSE_WDATA1 (MT_EEF_BASE + 0x14)
#define MT_EFUSE_WDATA2 (MT_EEF_BASE + 0x18)
#define MT_EFUSE_WDATA3 (MT_EEF_BASE + 0x1C)
#define MT_EFUSE_RDATA0 (MT_EEF_BASE + 0x30)
#define MT_EFUSE_RDATA1 (MT_EEF_BASE + 0x34)
#define MT_EFUSE_RDATA2 (MT_EEF_BASE + 0x38)
#define MT_EFUSE_RDATA3 (MT_EEF_BASE + 0x3C)

#ifdef RT_BIG_ENDIAN
typedef	union	_EFUSE_CTRL_STRUC {
	struct	{
		UINT32            SEL_EFUSE:1;
		UINT32            EFSROM_KICK:1;
		UINT32            EFSROM_DOUT_VLD:1;
		UINT32            RESERVED:3;
		UINT32            EFSROM_AIN:10;
		UINT32            EFSROM_LDO_ON_TIME:2;
		UINT32            EFSROM_LDO_OFF_TIME:6;
		UINT32            EFSROM_MODE:2;
		UINT32            EFSROM_AOUT:6;
	}	field;
	UINT32			word;
}	EFUSE_CTRL_STRUC, *PEFUSE_CTRL_STRUC;
#else
typedef	union	_EFUSE_CTRL_STRUC {
	struct	{
		UINT32            EFSROM_AOUT:6;
		UINT32            EFSROM_MODE:2;
		UINT32            EFSROM_LDO_OFF_TIME:6;
		UINT32            EFSROM_LDO_ON_TIME:2;
		UINT32            EFSROM_AIN:10;
		UINT32            RESERVED:3;
		UINT32            EFSROM_DOUT_VLD:1;
		UINT32            EFSROM_KICK:1;
		UINT32            SEL_EFUSE:1;
	}	field;
	UINT32			word;
}	EFUSE_CTRL_STRUC, *PEFUSE_CTRL_STRUC;
#endif /* RT_BIG_ENDIAN */

VOID eFuseReadPhysical(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUSHORT lpInBuffer,
	IN	ULONG nInBufferSize,
	OUT	PUSHORT lpOutBuffer,
	IN	ULONG nOutBufferSize);

#endif /* __EFUSE_H__ */
