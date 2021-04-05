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
	top.h
*/

#ifndef __TOP_H__
#define __TOP_H__

#define TOP_CFG_BASE 0x0000 

#define TOP_HVR (TOP_CFG_BASE + 0x1000)
#define HW_VER_MASK (0xffff)
#define GET_HW_VER(p) (((p) & HW_VER_MASK))
#define RESC_CID_MASK (0xf << 28)
#define GET_RESC_CID(p) (((p) & RESC_CID_MASK) >> 28)

#define TOP_FVR (TOP_CFG_BASE + 0x1004)
#define FW_VER_MASK (0xffff)
#define GET_FW_VER(p) (((p) & FW_VER_MASK))

#define TOP_HCR (TOP_CFG_BASE + 0x1008)
#define HW_CODE_MASK (0xffff)
#define GET_HW_CODE(p) (((p) & HW_CODE_MASK))

#define STRAP_STA (TOP_CFG_BASE + 0x1010)
#define XTAL_SEL_MASK (0x3)
#define GET_XTAL_SEL(p) (((p) & XTAL_SEL_MASK))
#define EEPROM_SEL (1 << 2)
#define GET_EEPROM_SEL(p) (((p) & EEPROM_SEL) >> 2)
#define CO_CLOCK_SEL (1 << 8)
#define GET_CO_CLOCK_SEL(p) (((p) & CO_CLOCK_SEL) >> 8)
#define ONE_ANT (1 << 24)
#define GET_ONE_ANT(p) (((p) & ONE_ANT) >> 24)
#define USB_MODE (1 << 25)
#define GET_USB_MODE(p) (((p) & USB_MODE) >> 25

#endif
