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
 * Feature : Proprietary CPU-tag related function drivers
 *
 */

#ifndef _RTL8367C_ASICDRV_CPUTAG_H_
#define _RTL8367C_ASICDRV_CPUTAG_H_

#include <rtl8367c_asicdrv.h>

enum CPUTAG_INSERT_MODE
{
    CPUTAG_INSERT_TO_ALL = 0,
    CPUTAG_INSERT_TO_TRAPPING,
    CPUTAG_INSERT_TO_NO,
    CPUTAG_INSERT_END
};

extern ret_t rtl8367c_setAsicCputagEnable(rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicCputagEnable(rtk_uint32 *pEnabled);
extern ret_t rtl8367c_setAsicCputagTrapPort(rtk_uint32 port);
extern ret_t rtl8367c_getAsicCputagTrapPort(rtk_uint32 *pPort);
extern ret_t rtl8367c_setAsicCputagPortmask(rtk_uint32 portmask);
extern ret_t rtl8367c_getAsicCputagPortmask(rtk_uint32 *pPmsk);
extern ret_t rtl8367c_setAsicCputagInsertMode(rtk_uint32 mode);
extern ret_t rtl8367c_getAsicCputagInsertMode(rtk_uint32 *pMode);
extern ret_t rtl8367c_setAsicCputagPriorityRemapping(rtk_uint32 srcPri, rtk_uint32 newPri);
extern ret_t rtl8367c_getAsicCputagPriorityRemapping(rtk_uint32 srcPri, rtk_uint32 *pNewPri);
extern ret_t rtl8367c_setAsicCputagPosition(rtk_uint32 position);
extern ret_t rtl8367c_getAsicCputagPosition(rtk_uint32* pPostion);
extern ret_t rtl8367c_setAsicCputagMode(rtk_uint32 mode);
extern ret_t rtl8367c_getAsicCputagMode(rtk_uint32 *pMode);
extern ret_t rtl8367c_setAsicCputagRxMinLength(rtk_uint32 mode);
extern ret_t rtl8367c_getAsicCputagRxMinLength(rtk_uint32 *pMode);

#endif /*#ifndef _RTL8367C_ASICDRV_CPUTAG_H_*/

