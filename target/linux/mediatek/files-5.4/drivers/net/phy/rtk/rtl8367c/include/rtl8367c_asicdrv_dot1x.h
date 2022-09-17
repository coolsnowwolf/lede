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
 * Feature : 802.1X related functions
 *
 */

#ifndef _RTL8367C_ASICDRV_DOT1X_H_
#define _RTL8367C_ASICDRV_DOT1X_H_

#include <rtl8367c_asicdrv.h>

enum DOT1X_UNAUTH_BEHAV
{
    DOT1X_UNAUTH_DROP = 0,
    DOT1X_UNAUTH_TRAP,
    DOT1X_UNAUTH_GVLAN,
    DOT1X_UNAUTH_END
};

extern ret_t rtl8367c_setAsic1xPBEnConfig(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367c_getAsic1xPBEnConfig(rtk_uint32 port, rtk_uint32 *pEnabled);
extern ret_t rtl8367c_setAsic1xPBAuthConfig(rtk_uint32 port, rtk_uint32 auth);
extern ret_t rtl8367c_getAsic1xPBAuthConfig(rtk_uint32 port, rtk_uint32 *pAuth);
extern ret_t rtl8367c_setAsic1xPBOpdirConfig(rtk_uint32 port, rtk_uint32 opdir);
extern ret_t rtl8367c_getAsic1xPBOpdirConfig(rtk_uint32 port, rtk_uint32 *pOpdir);
extern ret_t rtl8367c_setAsic1xMBEnConfig(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367c_getAsic1xMBEnConfig(rtk_uint32 port, rtk_uint32 *pEnabled);
extern ret_t rtl8367c_setAsic1xMBOpdirConfig(rtk_uint32 opdir);
extern ret_t rtl8367c_getAsic1xMBOpdirConfig(rtk_uint32 *pOpdir);
extern ret_t rtl8367c_setAsic1xProcConfig(rtk_uint32 port, rtk_uint32 proc);
extern ret_t rtl8367c_getAsic1xProcConfig(rtk_uint32 port, rtk_uint32 *pProc);
extern ret_t rtl8367c_setAsic1xGuestVidx(rtk_uint32 index);
extern ret_t rtl8367c_getAsic1xGuestVidx(rtk_uint32 *pIndex);
extern ret_t rtl8367c_setAsic1xGVOpdir(rtk_uint32 enabled);
extern ret_t rtl8367c_getAsic1xGVOpdir(rtk_uint32 *pEnabled);
extern ret_t rtl8367c_setAsic1xTrapPriority(rtk_uint32 priority);
extern ret_t rtl8367c_getAsic1xTrapPriority(rtk_uint32 *pPriority);


#endif /*_RTL8367C_ASICDRV_DOT1X_H_*/

