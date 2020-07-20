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
 * Feature : Green ethernet related functions
 *
 */

#ifndef _RTL8367C_ASICDRV_GREEN_H_
#define _RTL8367C_ASICDRV_GREEN_H_

#include <rtl8367c_asicdrv.h>
#include <rtl8367c_asicdrv_phy.h>

#define PHY_POWERSAVING_REG                         24

extern ret_t rtl8367c_setAsicGreenTrafficType(rtk_uint32 priority, rtk_uint32 traffictype);
extern ret_t rtl8367c_getAsicGreenTrafficType(rtk_uint32 priority, rtk_uint32* pTraffictype);
extern ret_t rtl8367c_getAsicGreenPortPage(rtk_uint32 port, rtk_uint32* pPage);
extern ret_t rtl8367c_getAsicGreenHighPriorityTraffic(rtk_uint32 port, rtk_uint32* pIndicator);
extern ret_t rtl8367c_setAsicGreenHighPriorityTraffic(rtk_uint32 port);
extern ret_t rtl8367c_setAsicGreenEthernet(rtk_uint32 port, rtk_uint32 green);
extern ret_t rtl8367c_getAsicGreenEthernet(rtk_uint32 port, rtk_uint32* green);
extern ret_t rtl8367c_setAsicPowerSaving(rtk_uint32 phy, rtk_uint32 enable);
extern ret_t rtl8367c_getAsicPowerSaving(rtk_uint32 phy, rtk_uint32* enable);
#endif /*#ifndef _RTL8367C_ASICDRV_GREEN_H_*/

