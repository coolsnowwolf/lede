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
 * Feature : Unknown multicast related functions
 *
 */

#ifndef _RTL8367C_ASICDRV_UNKNOWNMULTICAST_H_
#define _RTL8367C_ASICDRV_UNKNOWNMULTICAST_H_

#include <rtl8367c_asicdrv.h>

enum L2_UNKOWN_MULTICAST_BEHAVE
{
    L2_UNKOWN_MULTICAST_FLOODING = 0,
    L2_UNKOWN_MULTICAST_DROP,
    L2_UNKOWN_MULTICAST_TRAP,
    L2_UNKOWN_MULTICAST_DROP_EXCLUDE_RMA,
    L2_UNKOWN_MULTICAST_END
};

enum L3_UNKOWN_MULTICAST_BEHAVE
{
    L3_UNKOWN_MULTICAST_FLOODING = 0,
    L3_UNKOWN_MULTICAST_DROP,
    L3_UNKOWN_MULTICAST_TRAP,
    L3_UNKOWN_MULTICAST_ROUTER,
    L3_UNKOWN_MULTICAST_END
};

enum MULTICASTTYPE{
    MULTICAST_TYPE_IPV4 = 0,
    MULTICAST_TYPE_IPV6,
    MULTICAST_TYPE_L2,
    MULTICAST_TYPE_END
};

extern ret_t rtl8367c_setAsicUnknownL2MulticastBehavior(rtk_uint32 port, rtk_uint32 behave);
extern ret_t rtl8367c_getAsicUnknownL2MulticastBehavior(rtk_uint32 port, rtk_uint32 *pBehave);
extern ret_t rtl8367c_setAsicUnknownIPv4MulticastBehavior(rtk_uint32 port, rtk_uint32 behave);
extern ret_t rtl8367c_getAsicUnknownIPv4MulticastBehavior(rtk_uint32 port, rtk_uint32 *pBehave);
extern ret_t rtl8367c_setAsicUnknownIPv6MulticastBehavior(rtk_uint32 port, rtk_uint32 behave);
extern ret_t rtl8367c_getAsicUnknownIPv6MulticastBehavior(rtk_uint32 port, rtk_uint32 *pBehave);
extern ret_t rtl8367c_setAsicUnknownMulticastTrapPriority(rtk_uint32 priority);
extern ret_t rtl8367c_getAsicUnknownMulticastTrapPriority(rtk_uint32 *pPriority);

#endif /*_RTL8367C_ASICDRV_UNKNOWNMULTICAST_H_*/


