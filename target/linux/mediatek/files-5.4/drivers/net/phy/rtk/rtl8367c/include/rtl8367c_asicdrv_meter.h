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
 * Feature : Shared meter related functions
 *
 */

#ifndef _RTL8367C_ASICDRV_METER_H_
#define _RTL8367C_ASICDRV_METER_H_

#include <rtl8367c_asicdrv.h>


extern ret_t rtl8367c_setAsicShareMeter(rtk_uint32 index, rtk_uint32 rate, rtk_uint32 ifg);
extern ret_t rtl8367c_getAsicShareMeter(rtk_uint32 index, rtk_uint32 *pRate, rtk_uint32 *pIfg);
extern ret_t rtl8367c_setAsicShareMeterBucketSize(rtk_uint32 index, rtk_uint32 lbThreshold);
extern ret_t rtl8367c_getAsicShareMeterBucketSize(rtk_uint32 index, rtk_uint32 *pLbThreshold);
extern ret_t rtl8367c_setAsicShareMeterType(rtk_uint32 index, rtk_uint32 type);
extern ret_t rtl8367c_getAsicShareMeterType(rtk_uint32 index, rtk_uint32 *pType);
extern ret_t rtl8367c_setAsicMeterExceedStatus(rtk_uint32 index);
extern ret_t rtl8367c_getAsicMeterExceedStatus(rtk_uint32 index, rtk_uint32* pStatus);

#endif /*_RTL8367C_ASICDRV_FC_H_*/

