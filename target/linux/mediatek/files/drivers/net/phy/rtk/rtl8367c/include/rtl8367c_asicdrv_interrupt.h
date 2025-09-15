/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 76306 $
 * $Date: 2017-03-08 15:13:58 +0800 (¶g¤T, 08 ¤T¤ë 2017) $
 *
 * Purpose : RTL8367C switch high-level API for RTL8367C
 * Feature : Interrupt related functions
 *
 */

#ifndef _RTL8367C_ASICDRV_INTERRUPT_H_
#define _RTL8367C_ASICDRV_INTERRUPT_H_

#include <rtl8367c_asicdrv.h>

typedef enum RTL8367C_INTR_IMRS_E
{
    IMRS_LINK_CHANGE,
    IMRS_METER_EXCEED,
    IMRS_L2_LEARN,
    IMRS_SPEED_CHANGE,
    IMRS_SPECIAL_CONGESTION,
    IMRS_GREEN_FEATURE,
    IMRS_LOOP_DETECTION,
    IMRS_8051,
    IMRS_CABLE_DIAG,
    IMRS_ACL,
    IMRS_RESERVED, /* Unused */
    IMRS_SLIENT,
    IMRS_END,
}RTL8367C_INTR_IMRS;

typedef enum RTL8367C_INTR_INDICATOR_E
{
    INTRST_L2_LEARN = 0,
    INTRST_SPEED_CHANGE,
    INTRST_SPECIAL_CONGESTION,
    INTRST_PORT_LINKDOWN,
    INTRST_PORT_LINKUP,
    INTRST_METER0_15,
    INTRST_METER16_31,
    INTRST_RLDP_LOOPED,
    INTRST_RLDP_RELEASED,
    INTRST_SYS_LEARN,
    INTRST_END,
}RTL8367C_INTR_INDICATOR;

extern ret_t rtl8367c_setAsicInterruptPolarity(rtk_uint32 polarity);
extern ret_t rtl8367c_getAsicInterruptPolarity(rtk_uint32* pPolarity);
extern ret_t rtl8367c_setAsicInterruptMask(rtk_uint32 imr);
extern ret_t rtl8367c_getAsicInterruptMask(rtk_uint32* pImr);
extern ret_t rtl8367c_setAsicInterruptStatus(rtk_uint32 ims);
extern ret_t rtl8367c_getAsicInterruptStatus(rtk_uint32* pIms);
extern ret_t rtl8367c_setAsicInterruptRelatedStatus(rtk_uint32 type, rtk_uint32 status);
extern ret_t rtl8367c_getAsicInterruptRelatedStatus(rtk_uint32 type, rtk_uint32* pStatus);


#endif /*#ifndef _RTL8367C_ASICDRV_INTERRUPT_H_*/

