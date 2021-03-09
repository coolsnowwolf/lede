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
 * Feature : SVLAN related functions
 *
 */

#ifndef _RTL8367C_ASICDRV_SVLAN_H_
#define _RTL8367C_ASICDRV_SVLAN_H_

#include <rtl8367c_asicdrv.h>

#define RTL8367C_C2SIDXNO               128
#define RTL8367C_C2SIDXMAX              (RTL8367C_C2SIDXNO-1)
#define RTL8367C_MC2SIDXNO              32
#define RTL8367C_MC2SIDXMAX             (RTL8367C_MC2SIDXNO-1)
#define RTL8367C_SP2CIDXNO              128
#define RTL8367C_SP2CMAX                (RTL8367C_SP2CIDXNO-1)

#define RTL8367C_SVLAN_MEMCONF_LEN      4
#define RTL8367C_SVLAN_MC2S_LEN         5
#define RTL8367C_SVLAN_SP2C_LEN         2

enum RTL8367C_SPRISEL
{
    SPRISEL_INTERNALPRI =  0,
    SPRISEL_CTAGPRI,
    SPRISEL_VSPRI,
    SPRISEL_PBPRI,
    SPRISEL_END
};

enum RTL8367C_SUNACCEPT
{
    SUNACCEPT_DROP =  0,
    SUNACCEPT_TRAP,
    SUNACCEPT_SVLAN,
    SUNACCEPT_END
};

enum RTL8367C_SVLAN_MC2S_MODE
{
    SVLAN_MC2S_MODE_MAC =  0,
    SVLAN_MC2S_MODE_IP,
    SVLAN_MC2S_MODE_END
};


typedef struct  rtl8367c_svlan_memconf_s{

    rtk_uint16 vs_member:11;
    rtk_uint16 vs_untag:11;

    rtk_uint16 vs_fid_msti:4;
    rtk_uint16 vs_priority:3;
    rtk_uint16 vs_force_fid:1;
    rtk_uint16 reserved:8;

    rtk_uint16 vs_svid:12;
    rtk_uint16 vs_efiden:1;
    rtk_uint16 vs_efid:3;


}rtl8367c_svlan_memconf_t;


typedef struct  rtl8367c_svlan_mc2s_s{

    rtk_uint16 valid:1;
    rtk_uint16 format:1;
    rtk_uint16 svidx:6;
    rtk_uint32 sdata;
    rtk_uint32 smask;
}rtl8367c_svlan_mc2s_t;


typedef struct  rtl8367c_svlan_s2c_s{

    rtk_uint16 valid:1;
    rtk_uint16 svidx:6;
    rtk_uint16 dstport:4;
    rtk_uint32 vid:12;
}rtl8367c_svlan_s2c_t;

extern ret_t rtl8367c_setAsicSvlanIngressUntag(rtk_uint32 mode);
extern ret_t rtl8367c_getAsicSvlanIngressUntag(rtk_uint32* pMode);
extern ret_t rtl8367c_setAsicSvlanIngressUnmatch(rtk_uint32 mode);
extern ret_t rtl8367c_getAsicSvlanIngressUnmatch(rtk_uint32* pMode);
extern ret_t rtl8367c_setAsicSvlanTrapPriority(rtk_uint32 priority);
extern ret_t rtl8367c_getAsicSvlanTrapPriority(rtk_uint32* pPriority);
extern ret_t rtl8367c_setAsicSvlanDefaultVlan(rtk_uint32 port, rtk_uint32 index);
extern ret_t rtl8367c_getAsicSvlanDefaultVlan(rtk_uint32 port, rtk_uint32* pIndex);

extern ret_t rtl8367c_setAsicSvlanMemberConfiguration(rtk_uint32 index,rtl8367c_svlan_memconf_t* pSvlanMemCfg);
extern ret_t rtl8367c_getAsicSvlanMemberConfiguration(rtk_uint32 index,rtl8367c_svlan_memconf_t* pSvlanMemCfg);

extern ret_t rtl8367c_setAsicSvlanPrioritySel(rtk_uint32 priSel);
extern ret_t rtl8367c_getAsicSvlanPrioritySel(rtk_uint32* pPriSel);
extern ret_t rtl8367c_setAsicSvlanTpid(rtk_uint32 protocolType);
extern ret_t rtl8367c_getAsicSvlanTpid(rtk_uint32* pProtocolType);
extern ret_t rtl8367c_setAsicSvlanUplinkPortMask(rtk_uint32 portMask);
extern ret_t rtl8367c_getAsicSvlanUplinkPortMask(rtk_uint32* pPortmask);
extern ret_t rtl8367c_setAsicSvlanEgressUnassign(rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicSvlanEgressUnassign(rtk_uint32* pEnabled);
extern ret_t rtl8367c_setAsicSvlanC2SConf(rtk_uint32 index, rtk_uint32 evid, rtk_uint32 portmask, rtk_uint32 svidx);
extern ret_t rtl8367c_getAsicSvlanC2SConf(rtk_uint32 index, rtk_uint32* pEvid, rtk_uint32* pPortmask, rtk_uint32* pSvidx);
extern ret_t rtl8367c_setAsicSvlanMC2SConf(rtk_uint32 index,rtl8367c_svlan_mc2s_t* pSvlanMc2sCfg);
extern ret_t rtl8367c_getAsicSvlanMC2SConf(rtk_uint32 index,rtl8367c_svlan_mc2s_t* pSvlanMc2sCfg);
extern ret_t rtl8367c_setAsicSvlanSP2CConf(rtk_uint32 index,rtl8367c_svlan_s2c_t* pSvlanSp2cCfg);
extern ret_t rtl8367c_getAsicSvlanSP2CConf(rtk_uint32 index,rtl8367c_svlan_s2c_t* pSvlanSp2cCfg);
extern ret_t rtl8367c_setAsicSvlanDmacCvidSel(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicSvlanDmacCvidSel(rtk_uint32 port, rtk_uint32* pEnabled);
extern ret_t rtl8367c_setAsicSvlanUntagVlan(rtk_uint32 index);
extern ret_t rtl8367c_getAsicSvlanUntagVlan(rtk_uint32* pIndex);
extern ret_t rtl8367c_setAsicSvlanUnmatchVlan(rtk_uint32 index);
extern ret_t rtl8367c_getAsicSvlanUnmatchVlan(rtk_uint32* pIndex);
extern ret_t rtl8367c_setAsicSvlanLookupType(rtk_uint32 type);
extern ret_t rtl8367c_getAsicSvlanLookupType(rtk_uint32* pType);


#endif /*#ifndef _RTL8367C_ASICDRV_SVLAN_H_*/

