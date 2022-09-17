/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 42321 $
 * $Date: 2013-08-26 13:51:29 +0800 (¶g¤@, 26 ¤K¤ë 2013) $
 *
 * Purpose : RTL8367C switch high-level API for RTL8367C
 * Feature : RLDP related functions
 *
 */

#ifndef _RTL8367C_ASICDRV_RLDP_H_
#define _RTL8367C_ASICDRV_RLDP_H_

#include <rtl8367c_asicdrv.h>
#include <string.h>

extern ret_t rtl8367c_setAsicRldp(rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicRldp(rtk_uint32 *pEnabled);
extern ret_t rtl8367c_setAsicRldpEnable8051(rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicRldpEnable8051(rtk_uint32 *pEnabled);
extern ret_t rtl8367c_setAsicRldpCompareRandomNumber(rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicRldpCompareRandomNumber(rtk_uint32 *pEnabled);
extern ret_t rtl8367c_setAsicRldpIndicatorSource(rtk_uint32 src);
extern ret_t rtl8367c_getAsicRldpIndicatorSource(rtk_uint32 *pSrc);
extern ret_t rtl8367c_setAsicRldpCheckingStatePara(rtk_uint32 retryCount, rtk_uint32 retryPeriod);
extern ret_t rtl8367c_getAsicRldpCheckingStatePara(rtk_uint32 *pRetryCount, rtk_uint32 *pRetryPeriod);
extern ret_t rtl8367c_setAsicRldpLoopStatePara(rtk_uint32 retryCount, rtk_uint32 retryPeriod);
extern ret_t rtl8367c_getAsicRldpLoopStatePara(rtk_uint32 *pRetryCount, rtk_uint32 *pRetryPeriod);
extern ret_t rtl8367c_setAsicRldpTxPortmask(rtk_uint32 portmask);
extern ret_t rtl8367c_getAsicRldpTxPortmask(rtk_uint32 *pPortmask);
extern ret_t rtl8367c_setAsicRldpMagicNum(ether_addr_t seed);
extern ret_t rtl8367c_getAsicRldpMagicNum(ether_addr_t *pSeed);
extern ret_t rtl8367c_getAsicRldpLoopedPortmask(rtk_uint32 *pPortmask);
extern ret_t rtl8367c_setAsicRldp8051Portmask(rtk_uint32 portmask);
extern ret_t rtl8367c_getAsicRldp8051Portmask(rtk_uint32 *pPortmask);


extern ret_t rtl8367c_getAsicRldpRandomNumber(ether_addr_t *pRandNumber);
extern ret_t rtl8367c_getAsicRldpLoopedPortPair(rtk_uint32 port, rtk_uint32 *pLoopedPair);
extern ret_t rtl8367c_setAsicRlppTrap8051(rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicRlppTrap8051(rtk_uint32 *pEnabled);

extern ret_t rtl8367c_setAsicRldpLeaveLoopedPortmask(rtk_uint32 portmask);
extern ret_t rtl8367c_getAsicRldpLeaveLoopedPortmask(rtk_uint32 *pPortmask);

extern ret_t rtl8367c_setAsicRldpEnterLoopedPortmask(rtk_uint32 portmask);
extern ret_t rtl8367c_getAsicRldpEnterLoopedPortmask(rtk_uint32 *pPortmask);

extern ret_t rtl8367c_setAsicRldpTriggerMode(rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicRldpTriggerMode(rtk_uint32 *pEnabled);

#endif /*_RTL8367C_ASICDRV_RLDP_H_*/

