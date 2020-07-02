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
 * Feature : Port trunking related functions
 *
 */


#ifndef _RTL8367C_ASICDRV_TRUNKING_H_
#define _RTL8367C_ASICDRV_TRUNKING_H_

#include <rtl8367c_asicdrv.h>

#define RTL8367C_MAX_TRUNK_GID              (2)
#define RTL8367C_TRUNKING_PORTNO            (4)
#define RTL8367C_TRUNKING1_PORTN0           (2)
#define RTL8367C_TRUNKING_HASHVALUE_MAX     (15)

extern ret_t rtl8367c_setAsicTrunkingGroup(rtk_uint32 group, rtk_uint32 portmask);
extern ret_t rtl8367c_getAsicTrunkingGroup(rtk_uint32 group, rtk_uint32* pPortmask);
extern ret_t rtl8367c_setAsicTrunkingFlood(rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicTrunkingFlood(rtk_uint32* pEnabled);
extern ret_t rtl8367c_setAsicTrunkingHashSelect(rtk_uint32 hashsel);
extern ret_t rtl8367c_getAsicTrunkingHashSelect(rtk_uint32* pHashsel);

extern ret_t rtl8367c_getAsicQeueuEmptyStatus(rtk_uint32* pPortmask);

extern ret_t rtl8367c_setAsicTrunkingMode(rtk_uint32 mode);
extern ret_t rtl8367c_getAsicTrunkingMode(rtk_uint32* pMode);
extern ret_t rtl8367c_setAsicTrunkingFc(rtk_uint32 group, rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicTrunkingFc(rtk_uint32 group, rtk_uint32* pEnabled);
extern ret_t rtl8367c_setAsicTrunkingHashTable(rtk_uint32 hashval, rtk_uint32 portId);
extern ret_t rtl8367c_getAsicTrunkingHashTable(rtk_uint32 hashval, rtk_uint32* pPortId);
extern ret_t rtl8367c_setAsicTrunkingHashTable1(rtk_uint32 hashval, rtk_uint32 portId);
extern ret_t rtl8367c_getAsicTrunkingHashTable1(rtk_uint32 hashval, rtk_uint32* pPortId);

#endif /*_RTL8367C_ASICDRV_TRUNKING_H_*/

