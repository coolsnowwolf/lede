/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 76333 $
 * $Date: 2017-03-09 09:33:15 +0800 (¶g¥|, 09 ¤T¤ë 2017) $
 *
 * Purpose : RTL8367C switch high-level API for RTL8367C
 * Feature : Port security related functions
 *
 */

#ifndef _RTL8367C_ASICDRV_PORTSECURITY_H_
#define _RTL8367C_ASICDRV_PORTSECURITY_H_

#include <rtl8367c_asicdrv.h>
#include <rtl8367c_asicdrv_unknownMulticast.h>
#include <rtl8367c_asicdrv_phy.h>

/****************************************************************/
/* Type Definition                                              */
/****************************************************************/

#define RTL8367C_MAC7       7
#define RTL8367C_EXTNO       3

#define RTL8367C_RTCT_PAGE          (11)
#define RTL8367C_RTCT_RESULT_A_REG  (27)
#define RTL8367C_RTCT_RESULT_B_REG  (28)
#define RTL8367C_RTCT_RESULT_C_REG  (29)
#define RTL8367C_RTCT_RESULT_D_REG  (30)
#define RTL8367C_RTCT_STATUS_REG    (26)

enum L2_SECURITY_BEHAVE
{
    L2_BEHAVE_FLOODING = 0,
    L2_BEHAVE_DROP,
    L2_BEHAVE_TRAP,
    L2_BEHAVE_END
};

enum L2_UNDA_BEHAVE
{
    L2_UNDA_BEHAVE_FLOODING_PMASK = 0,
    L2_UNDA_BEHAVE_DROP,
    L2_UNDA_BEHAVE_TRAP,
    L2_UNDA_BEHAVE_FLOODING,
    L2_UNDA_BEHAVE_END
};

enum L2_SECURITY_SA_BEHAVE
{
    L2_BEHAVE_SA_FLOODING = 0,
    L2_BEHAVE_SA_DROP,
    L2_BEHAVE_SA_TRAP,
    L2_BEHAVE_SA_COPY28051,
    L2_BEHAVE_SA_END
};

/* enum for port current link speed */
enum SPEEDMODE
{
    SPD_10M = 0,
    SPD_100M,
    SPD_1000M,
    SPD_2500M
};

/* enum for mac link mode */
enum LINKMODE
{
    MAC_NORMAL = 0,
    MAC_FORCE,
};

/* enum for port current link duplex mode */
enum DUPLEXMODE
{
    HALF_DUPLEX = 0,
    FULL_DUPLEX
};

/* enum for port current MST mode */
enum MSTMODE
{
    SLAVE_MODE= 0,
    MASTER_MODE
};


enum EXTMODE
{
    EXT_DISABLE = 0,
    EXT_RGMII,
    EXT_MII_MAC,
    EXT_MII_PHY,
    EXT_TMII_MAC,
    EXT_TMII_PHY,
    EXT_GMII,
    EXT_RMII_MAC,
    EXT_RMII_PHY,
    EXT_SGMII,
    EXT_HSGMII,
    EXT_1000X_100FX,
    EXT_1000X,
    EXT_100FX,
    EXT_RGMII_2,
    EXT_MII_MAC_2,
    EXT_MII_PHY_2,
    EXT_TMII_MAC_2,
    EXT_TMII_PHY_2,
    EXT_RMII_MAC_2,
    EXT_RMII_PHY_2,
    EXT_END
};

enum DOSTYPE
{
    DOS_DAEQSA = 0,
    DOS_LANDATTACKS,
    DOS_BLATATTACKS,
    DOS_SYNFINSCAN,
    DOS_XMASCAN,
    DOS_NULLSCAN,
    DOS_SYN1024,
    DOS_TCPSHORTHDR,
    DOS_TCPFRAGERROR,
    DOS_ICMPFRAGMENT,
    DOS_END,

};

typedef struct  rtl8367c_port_ability_s{
    rtk_uint16 forcemode;
    rtk_uint16 mstfault;
    rtk_uint16 mstmode;
    rtk_uint16 nway;
    rtk_uint16 txpause;
    rtk_uint16 rxpause;
    rtk_uint16 link;
    rtk_uint16 duplex;
    rtk_uint16 speed;
}rtl8367c_port_ability_t;

typedef struct  rtl8367c_port_status_s{

    rtk_uint16 lpi1000;
    rtk_uint16 lpi100;
    rtk_uint16 mstfault;
    rtk_uint16 mstmode;
    rtk_uint16 nway;
    rtk_uint16 txpause;
    rtk_uint16 rxpause;
    rtk_uint16 link;
    rtk_uint16 duplex;
    rtk_uint16 speed;

}rtl8367c_port_status_t;

typedef struct rtct_result_s
{
    rtk_uint32      channelAShort;
    rtk_uint32      channelBShort;
    rtk_uint32      channelCShort;
    rtk_uint32      channelDShort;

    rtk_uint32      channelAOpen;
    rtk_uint32      channelBOpen;
    rtk_uint32      channelCOpen;
    rtk_uint32      channelDOpen;

    rtk_uint32      channelAMismatch;
    rtk_uint32      channelBMismatch;
    rtk_uint32      channelCMismatch;
    rtk_uint32      channelDMismatch;

    rtk_uint32      channelALinedriver;
    rtk_uint32      channelBLinedriver;
    rtk_uint32      channelCLinedriver;
    rtk_uint32      channelDLinedriver;

    rtk_uint32      channelALen;
    rtk_uint32      channelBLen;
    rtk_uint32      channelCLen;
    rtk_uint32      channelDLen;
} rtl8367c_port_rtct_result_t;


/****************************************************************/
/* Driver Proto Type Definition                                 */
/****************************************************************/
extern ret_t rtl8367c_setAsicPortUnknownDaBehavior(rtk_uint32 port, rtk_uint32 behavior);
extern ret_t rtl8367c_getAsicPortUnknownDaBehavior(rtk_uint32 port, rtk_uint32 *pBehavior);
extern ret_t rtl8367c_setAsicPortUnknownSaBehavior(rtk_uint32 behavior);
extern ret_t rtl8367c_getAsicPortUnknownSaBehavior(rtk_uint32 *pBehavior);
extern ret_t rtl8367c_setAsicPortUnmatchedSaBehavior(rtk_uint32 behavior);
extern ret_t rtl8367c_getAsicPortUnmatchedSaBehavior(rtk_uint32 *pBehavior);
extern ret_t rtl8367c_setAsicPortUnmatchedSaMoving(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicPortUnmatchedSaMoving(rtk_uint32 port, rtk_uint32 *pEnabled);
extern ret_t rtl8367c_setAsicPortUnknownDaFloodingPortmask(rtk_uint32 portmask);
extern ret_t rtl8367c_getAsicPortUnknownDaFloodingPortmask(rtk_uint32 *pPortmask);
extern ret_t rtl8367c_setAsicPortUnknownMulticastFloodingPortmask(rtk_uint32 portmask);
extern ret_t rtl8367c_getAsicPortUnknownMulticastFloodingPortmask(rtk_uint32 *pPortmask);
extern ret_t rtl8367c_setAsicPortBcastFloodingPortmask(rtk_uint32 portmask);
extern ret_t rtl8367c_getAsicPortBcastFloodingPortmask(rtk_uint32 *pPortmask);
extern ret_t rtl8367c_setAsicPortBlockSpa(rtk_uint32 port, rtk_uint32 block);
extern ret_t rtl8367c_getAsicPortBlockSpa(rtk_uint32 port, rtk_uint32 *pBlock);
extern ret_t rtl8367c_setAsicPortForceLink(rtk_uint32 port, rtl8367c_port_ability_t *pPortAbility);
extern ret_t rtl8367c_getAsicPortForceLink(rtk_uint32 port, rtl8367c_port_ability_t *pPortAbility);
extern ret_t rtl8367c_getAsicPortStatus(rtk_uint32 port, rtl8367c_port_status_t *pPortStatus);
extern ret_t rtl8367c_setAsicPortForceLinkExt(rtk_uint32 id, rtl8367c_port_ability_t *pPortAbility);
extern ret_t rtl8367c_getAsicPortForceLinkExt(rtk_uint32 id, rtl8367c_port_ability_t *pPortAbility);
extern ret_t rtl8367c_setAsicPortExtMode(rtk_uint32 id, rtk_uint32 mode);
extern ret_t rtl8367c_getAsicPortExtMode(rtk_uint32 id, rtk_uint32 *pMode);
extern ret_t rtl8367c_setAsicPortDos(rtk_uint32 type, rtk_uint32 drop);
extern ret_t rtl8367c_getAsicPortDos(rtk_uint32 type, rtk_uint32* pDrop);
extern ret_t rtl8367c_setAsicPortEnableAll(rtk_uint32 enable);
extern ret_t rtl8367c_getAsicPortEnableAll(rtk_uint32 *pEnable);
extern ret_t rtl8367c_setAsicPortSmallIpg(rtk_uint32 port, rtk_uint32 enable);
extern ret_t rtl8367c_getAsicPortSmallIpg(rtk_uint32 port, rtk_uint32* pEnable);
extern ret_t rtl8367c_setAsicPortLoopback(rtk_uint32 port, rtk_uint32 enable);
extern ret_t rtl8367c_getAsicPortLoopback(rtk_uint32 port, rtk_uint32 *pEnable);
extern ret_t rtl8367c_setAsicPortRTCTEnable(rtk_uint32 portmask);
extern ret_t rtl8367c_setAsicPortRTCTDisable(rtk_uint32 portmask);
extern ret_t rtl8367c_getAsicPortRTCTResult(rtk_uint32 port, rtl8367c_port_rtct_result_t *pResult);
extern ret_t rtl8367c_sdsReset(rtk_uint32 id);
extern ret_t rtl8367c_getSdsLinkStatus(rtk_uint32 ext_id, rtk_uint32 *pSignalDetect, rtk_uint32 *pSync, rtk_uint32 *pLink);
extern ret_t rtl8367c_setSgmiiNway(rtk_uint32 ext_id, rtk_uint32 state);
extern ret_t rtl8367c_getSgmiiNway(rtk_uint32 ext_id, rtk_uint32 *pState);

#endif /*_RTL8367C_ASICDRV_PORTSECURITY_H_*/

