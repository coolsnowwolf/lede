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
 * Feature : Miscellaneous functions
 *
 */

#ifndef _RTL8367C_ASICDRV_MISC_H_
#define _RTL8367C_ASICDRV_MISC_H_

#include <rtl8367c_asicdrv.h>

extern ret_t rtl8367c_setAsicMacAddress(ether_addr_t mac);
extern ret_t rtl8367c_getAsicMacAddress(ether_addr_t *pMac);
extern ret_t rtl8367c_getAsicDebugInfo(rtk_uint32 port, rtk_uint32 *pDebugifo);
extern ret_t rtl8367c_setAsicPortJamMode(rtk_uint32 mode);
extern ret_t rtl8367c_getAsicPortJamMode(rtk_uint32* pMode);
extern ret_t rtl8367c_setAsicMaxLengthCfg(rtk_uint32 cfgId, rtk_uint32 maxLength);
extern ret_t rtl8367c_getAsicMaxLengthCfg(rtk_uint32 cfgId, rtk_uint32 *pMaxLength);
extern ret_t rtl8367c_setAsicMaxLength(rtk_uint32 port, rtk_uint32 type, rtk_uint32 cfgId);
extern ret_t rtl8367c_getAsicMaxLength(rtk_uint32 port, rtk_uint32 type, rtk_uint32 *pCfgId);

#endif /*_RTL8367C_ASICDRV_MISC_H_*/

