/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 76336 $
 * $Date: 2017-03-09 10:41:21 +0800 (週四, 09 三月 2017) $
 *
 * Purpose : RTK switch high-level API
 * Feature : Here is a list of all functions and variables in this module.
 *
 */
#include <rtk_switch.h>
#include <rtk_error.h>
#include <string.h>

#include <rate.h>
#include <rtl8367c_asicdrv.h>
#include <rtl8367c_asicdrv_misc.h>
#include <rtl8367c_asicdrv_green.h>
#include <rtl8367c_asicdrv_lut.h>
#include <rtl8367c_asicdrv_rma.h>
#include <rtl8367c_asicdrv_mirror.h>

#if defined(FORCE_PROBE_RTL8367C)
static init_state_t    init_state = INIT_COMPLETED;
#elif defined(FORCE_PROBE_RTL8370B)
static init_state_t    init_state = INIT_COMPLETED;
#elif defined(FORCE_PROBE_RTL8364B)
static init_state_t    init_state = INIT_COMPLETED;
#elif defined(FORCE_PROBE_RTL8363SC_VB)
static init_state_t    init_state = INIT_COMPLETED;
#else
static init_state_t    init_state = INIT_NOT_COMPLETED;
#endif

#define AUTO_PROBE (!defined(FORCE_PROBE_RTL8367C) && !defined(FORCE_PROBE_RTL8370B) && !defined(FORCE_PROBE_RTL8364B) && !defined(FORCE_PROBE_RTL8363SC_VB))

#if (AUTO_PROBE || defined(FORCE_PROBE_RTL8367C))
static rtk_switch_halCtrl_t rtl8367c_hal_Ctrl =
{
    /* Switch Chip */
    CHIP_RTL8367C,

    /* Logical to Physical */
    {0, 1, 2, 3, 4, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     6, 7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },

    /* Physical to Logical */
    {UTP_PORT0, UTP_PORT1, UTP_PORT2, UTP_PORT3, UTP_PORT4, UNDEFINE_PORT, EXT_PORT0, EXT_PORT1,
     UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT,
     UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT,
     UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT},

    /* Port Type */
    {UTP_PORT, UTP_PORT, UTP_PORT, UTP_PORT, UTP_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT,
     UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT,
     EXT_PORT, EXT_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT,
     UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT},

    /* PTP port */
    {1, 1, 1, 1, 1, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0 },

    /* Valid port mask */
    ( (0x1 << UTP_PORT0) | (0x1 << UTP_PORT1) | (0x1 << UTP_PORT2) | (0x1 << UTP_PORT3) | (0x1 << UTP_PORT4) | (0x1 << EXT_PORT0) | (0x1 << EXT_PORT1) ),

    /* Valid UTP port mask */
    ( (0x1 << UTP_PORT0) | (0x1 << UTP_PORT1) | (0x1 << UTP_PORT2) | (0x1 << UTP_PORT3) | (0x1 << UTP_PORT4) ),

    /* Valid EXT port mask */
    ( (0x1 << EXT_PORT0) | (0x1 << EXT_PORT1) ),

    /* Valid CPU port mask */
    0x00,

    /* Minimum physical port number */
    0,

    /* Maxmum physical port number */
    7,

    /* Physical port mask */
    0xDF,

    /* Combo Logical port ID */
    4,

    /* HSG Logical port ID */
    EXT_PORT0,

    /* SGMII Logical portmask */
    (0x1 << EXT_PORT0),

    /* Max Meter ID */
    31,

    /* MAX LUT Address Number */
    2112,

    /* Trunk Group Mask */
    0x03
};
#endif

#if (AUTO_PROBE || defined(FORCE_PROBE_RTL8370B))
static rtk_switch_halCtrl_t rtl8370b_hal_Ctrl =
{
    /* Switch Chip */
    CHIP_RTL8370B,

    /* Logical to Physical */
    {0, 1, 2, 3, 4, 5, 6, 7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     8, 9, 10, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },

    /* Physical to Logical */
    {UTP_PORT0, UTP_PORT1, UTP_PORT2, UTP_PORT3, UTP_PORT4, UTP_PORT5, UTP_PORT6, UTP_PORT7,
     EXT_PORT0, EXT_PORT1, EXT_PORT2, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT,
     UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT,
     UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT},

    /* Port Type */
    {UTP_PORT, UTP_PORT, UTP_PORT, UTP_PORT, UTP_PORT, UTP_PORT, UTP_PORT, UTP_PORT,
     UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT,
     EXT_PORT, EXT_PORT, EXT_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT,
     UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT},

    /* PTP port */
    {1, 1, 1, 1, 1, 1, 1, 1,
     0, 0, 0, 0, 0, 0, 0, 0,
     1, 1, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0 },

    /* Valid port mask */
    ( (0x1 << UTP_PORT0) | (0x1 << UTP_PORT1) | (0x1 << UTP_PORT2) | (0x1 << UTP_PORT3) | (0x1 << UTP_PORT4) | (0x1 << UTP_PORT5) | (0x1 << UTP_PORT6) | (0x1 << UTP_PORT7) | (0x1 << EXT_PORT0) | (0x1 << EXT_PORT1) | (0x1 << EXT_PORT2) ),

    /* Valid UTP port mask */
    ( (0x1 << UTP_PORT0) | (0x1 << UTP_PORT1) | (0x1 << UTP_PORT2) | (0x1 << UTP_PORT3) | (0x1 << UTP_PORT4) | (0x1 << UTP_PORT5) | (0x1 << UTP_PORT6) | (0x1 << UTP_PORT7) ),

    /* Valid EXT port mask */
    ( (0x1 << EXT_PORT0) | (0x1 << EXT_PORT1) | (0x1 << EXT_PORT2) ),

    /* Valid CPU port mask */
    (0x1 << EXT_PORT2),

    /* Minimum physical port number */
    0,

    /* Maxmum physical port number */
    10,

    /* Physical port mask */
    0x7FF,

    /* Combo Logical port ID */
    7,

    /* HSG Logical port ID */
    EXT_PORT1,

    /* SGMII Logical portmask */
    ( (0x1 << EXT_PORT0) | (0x1 << EXT_PORT1) ),

    /* Max Meter ID */
    63,

    /* MAX LUT Address Number 4096 + 64*/
    4160,

    /* Trunk Group Mask */
    0x07
};
#endif

#if (AUTO_PROBE || defined(FORCE_PROBE_RTL8364B))
static rtk_switch_halCtrl_t rtl8364b_hal_Ctrl =
{
    /* Switch Chip */
    CHIP_RTL8364B,

    /* Logical to Physical */
    {0xFF, 1, 0xFF, 3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     6, 7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },

    /* Physical to Logical */
    {UNDEFINE_PORT, UTP_PORT1, UNDEFINE_PORT, UTP_PORT3, UNDEFINE_PORT, UNDEFINE_PORT, EXT_PORT0, EXT_PORT1,
     UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT,
     UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT,
     UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT},

    /* Port Type */
    {UNKNOWN_PORT, UTP_PORT, UNKNOWN_PORT, UTP_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT,
     UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT,
     EXT_PORT, EXT_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT,
     UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT},

    /* PTP port */
    {0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0 },

    /* Valid port mask */
    ( (0x1 << UTP_PORT1) | (0x1 << UTP_PORT3) | (0x1 << EXT_PORT0) | (0x1 << EXT_PORT1) ),

    /* Valid UTP port mask */
    ( (0x1 << UTP_PORT1) | (0x1 << UTP_PORT3) ),

    /* Valid EXT port mask */
    ( (0x1 << EXT_PORT0) | (0x1 << EXT_PORT1) ),

    /* Valid CPU port mask */
    0x00,

    /* Minimum physical port number */
    0,

    /* Maxmum physical port number */
    7,

    /* Physical port mask */
    0xCA,

    /* Combo Logical port ID */
    4,

    /* HSG Logical port ID */
    EXT_PORT0,

    /* SGMII Logical portmask */
    ( (0x1 << EXT_PORT0) | (0x1 << EXT_PORT1) ),

    /* Max Meter ID */
    32,

    /* MAX LUT Address Number */
    2112,

    /* Trunk Group Mask */
    0x01
};
#endif

#if (AUTO_PROBE || defined(FORCE_PROBE_RTL8363SC_VB))
static rtk_switch_halCtrl_t rtl8363sc_vb_hal_Ctrl =
{
    /* Switch Chip */
    CHIP_RTL8363SC_VB,

    /* Logical to Physical */
    {0xFF, 0xFF, 1, 3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     6, 7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },

    /* Physical to Logical */
    {UNDEFINE_PORT, UTP_PORT2, UNDEFINE_PORT, UTP_PORT3, UNDEFINE_PORT, UNDEFINE_PORT, EXT_PORT0, EXT_PORT1,
     UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT,
     UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT,
     UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT, UNDEFINE_PORT},

    /* Port Type */
    {UNKNOWN_PORT, UNKNOWN_PORT, UTP_PORT, UTP_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT,
     UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT,
     EXT_PORT, EXT_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT,
     UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT, UNKNOWN_PORT},

    /* PTP port */
    {0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0 },

    /* Valid port mask */
    ( (0x1 << UTP_PORT2) | (0x1 << UTP_PORT3) | (0x1 << EXT_PORT0) | (0x1 << EXT_PORT1) ),

    /* Valid UTP port mask */
    ( (0x1 << UTP_PORT2) | (0x1 << UTP_PORT3) ),

    /* Valid EXT port mask */
    ( (0x1 << EXT_PORT0) | (0x1 << EXT_PORT1) ),

    /* Valid CPU port mask */
    0x00,

    /* Minimum physical port number */
    0,

    /* Maxmum physical port number */
    7,

    /* Physical port mask */
    0xCA,

    /* Combo Logical port ID */
    4,

    /* HSG Logical port ID */
    EXT_PORT0,

    /* SGMII Logical portmask */
    ( (0x1 << EXT_PORT0) | (0x1 << EXT_PORT1) ),

    /* Max Meter ID */
    32,

    /* MAX LUT Address Number */
    2112,

    /* Trunk Group Mask */
    0x01
};
#endif

#if defined(FORCE_PROBE_RTL8367C)
static rtk_switch_halCtrl_t *halCtrl = &rtl8367c_hal_Ctrl;
#elif defined(FORCE_PROBE_RTL8370B)
static rtk_switch_halCtrl_t *halCtrl = &rtl8370b_hal_Ctrl;
#elif defined(FORCE_PROBE_RTL8364B)
static rtk_switch_halCtrl_t *halCtrl = &rtl8364b_hal_Ctrl;
#elif defined(FORCE_PROBE_RTL8363SC_VB)
static rtk_switch_halCtrl_t *halCtrl = &rtl8363sc_vb_hal_Ctrl;
#else
static rtk_switch_halCtrl_t *halCtrl = NULL;
#endif

static rtk_uint32 PatchChipData[210][2] =
{
        {0xa436, 0x8028}, {0xa438, 0x6800}, {0xb82e, 0x0001}, {0xa436, 0xb820}, {0xa438, 0x0090}, {0xa436, 0xa012}, {0xa438, 0x0000}, {0xa436, 0xa014}, {0xa438, 0x2c04}, {0xa438, 0x2c6c},
        {0xa438, 0x2c75}, {0xa438, 0x2c77}, {0xa438, 0x1414}, {0xa438, 0x1579}, {0xa438, 0x1536}, {0xa438, 0xc432}, {0xa438, 0x32c0}, {0xa438, 0x42d6}, {0xa438, 0x32b5}, {0xa438, 0x003e},
        {0xa438, 0x614c}, {0xa438, 0x1569}, {0xa438, 0xd705}, {0xa438, 0x318c}, {0xa438, 0x42d6}, {0xa438, 0xd702}, {0xa438, 0x31ef}, {0xa438, 0x42d6}, {0xa438, 0x629c}, {0xa438, 0x2c04},
        {0xa438, 0x653c}, {0xa438, 0x422a}, {0xa438, 0x5d83}, {0xa438, 0xd06a}, {0xa438, 0xd1b0}, {0xa438, 0x1536}, {0xa438, 0xc43a}, {0xa438, 0x32c0}, {0xa438, 0x42d6}, {0xa438, 0x32b5},
        {0xa438, 0x003e}, {0xa438, 0x314a}, {0xa438, 0x42fe}, {0xa438, 0x337b}, {0xa438, 0x02d6}, {0xa438, 0x3063}, {0xa438, 0x0c1b}, {0xa438, 0x22fe}, {0xa438, 0xc435}, {0xa438, 0xd0be},
        {0xa438, 0xd1f7}, {0xa438, 0xe0f0}, {0xa438, 0x1a40}, {0xa438, 0xa320}, {0xa438, 0xd702}, {0xa438, 0x154a}, {0xa438, 0xc434}, {0xa438, 0x32c0}, {0xa438, 0x42d6}, {0xa438, 0x32b5},
        {0xa438, 0x003e}, {0xa438, 0x60ec}, {0xa438, 0x1569}, {0xa438, 0xd705}, {0xa438, 0x619f}, {0xa438, 0xd702}, {0xa438, 0x414f}, {0xa438, 0x2c2e}, {0xa438, 0x610a}, {0xa438, 0xd705},
        {0xa438, 0x5e1f}, {0xa438, 0xc43f}, {0xa438, 0xc88b}, {0xa438, 0xd702}, {0xa438, 0x7fe0}, {0xa438, 0x22f3}, {0xa438, 0xd0a0}, {0xa438, 0xd1b2}, {0xa438, 0xd0c3}, {0xa438, 0xd1c3},
        {0xa438, 0x8d01}, {0xa438, 0x1536}, {0xa438, 0xc438}, {0xa438, 0xe0f0}, {0xa438, 0x1a80}, {0xa438, 0xd706}, {0xa438, 0x60c0}, {0xa438, 0xd710}, {0xa438, 0x409e}, {0xa438, 0xa804},
        {0xa438, 0xad01}, {0xa438, 0x8804}, {0xa438, 0xd702}, {0xa438, 0x32c0}, {0xa438, 0x42d6}, {0xa438, 0x32b5}, {0xa438, 0x003e}, {0xa438, 0x405b}, {0xa438, 0x1576}, {0xa438, 0x7c9c},
        {0xa438, 0x60ec}, {0xa438, 0x1569}, {0xa438, 0xd702}, {0xa438, 0x5d43}, {0xa438, 0x31ef}, {0xa438, 0x02fe}, {0xa438, 0x22d6}, {0xa438, 0x590a}, {0xa438, 0xd706}, {0xa438, 0x5c80},
        {0xa438, 0xd702}, {0xa438, 0x5c44}, {0xa438, 0x3063}, {0xa438, 0x02d6}, {0xa438, 0x5be2}, {0xa438, 0x22fb}, {0xa438, 0xa240}, {0xa438, 0xa104}, {0xa438, 0x8c03}, {0xa438, 0x8178},
        {0xa438, 0xd701}, {0xa438, 0x31ad}, {0xa438, 0x4917}, {0xa438, 0x8102}, {0xa438, 0x2917}, {0xa438, 0xc302}, {0xa438, 0x268a}, {0xa436, 0xA01A}, {0xa438, 0x0000}, {0xa436, 0xA006},
        {0xa438, 0x0fff}, {0xa436, 0xA004}, {0xa438, 0x0689}, {0xa436, 0xA002}, {0xa438, 0x0911}, {0xa436, 0xA000}, {0xa438, 0x7302}, {0xa436, 0xB820}, {0xa438, 0x0010}, {0xa436, 0x8412},
        {0xa438, 0xaf84}, {0xa438, 0x1eaf}, {0xa438, 0x8427}, {0xa438, 0xaf84}, {0xa438, 0x27af}, {0xa438, 0x8427}, {0xa438, 0x0251}, {0xa438, 0x6802}, {0xa438, 0x8427}, {0xa438, 0xaf04},
        {0xa438, 0x0af8}, {0xa438, 0xf9bf}, {0xa438, 0x5581}, {0xa438, 0x0255}, {0xa438, 0x27ef}, {0xa438, 0x310d}, {0xa438, 0x345b}, {0xa438, 0x0fa3}, {0xa438, 0x032a}, {0xa438, 0xe087},
        {0xa438, 0xffac}, {0xa438, 0x2040}, {0xa438, 0xbf56}, {0xa438, 0x7402}, {0xa438, 0x5527}, {0xa438, 0xef31}, {0xa438, 0xef20}, {0xa438, 0xe787}, {0xa438, 0xfee6}, {0xa438, 0x87fd},
        {0xa438, 0xd488}, {0xa438, 0x88bf}, {0xa438, 0x5674}, {0xa438, 0x0254}, {0xa438, 0xe3e0}, {0xa438, 0x87ff}, {0xa438, 0xf720}, {0xa438, 0xe487}, {0xa438, 0xffaf}, {0xa438, 0x847e},
        {0xa438, 0xe087}, {0xa438, 0xffad}, {0xa438, 0x2016}, {0xa438, 0xe387}, {0xa438, 0xfee2}, {0xa438, 0x87fd}, {0xa438, 0xef45}, {0xa438, 0xbf56}, {0xa438, 0x7402}, {0xa438, 0x54e3},
        {0xa438, 0xe087}, {0xa438, 0xfff6}, {0xa438, 0x20e4}, {0xa438, 0x87ff}, {0xa438, 0xfdfc}, {0xa438, 0x0400}, {0xa436, 0xb818}, {0xa438, 0x0407}, {0xa436, 0xb81a}, {0xa438, 0xfffd},
        {0xa436, 0xb81c}, {0xa438, 0xfffd}, {0xa436, 0xb81e}, {0xa438, 0xfffd}, {0xa436, 0xb832}, {0xa438, 0x0001}, {0xb820, 0x0000}, {0xb82e, 0x0000}, {0xa436, 0x8028}, {0xa438, 0x0000}
};

static rtk_api_ret_t _rtk_switch_init_8367c(void)
{
    rtk_port_t port;
    rtk_uint32 retVal;
    rtk_uint32 regData;
    rtk_uint32 regValue;

    if( (retVal = rtl8367c_setAsicReg(0x13c2, 0x0249)) != RT_ERR_OK)
        return retVal;

    if( (retVal = rtl8367c_getAsicReg(0x1301, &regValue)) != RT_ERR_OK)
        return retVal;

    if( (retVal = rtl8367c_setAsicReg(0x13c2, 0x0000)) != RT_ERR_OK)
        return retVal;

    RTK_SCAN_ALL_LOG_PORT(port)
    {
         if(rtk_switch_isUtpPort(port) == RT_ERR_OK)
         {
             if((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_PORT0_EEECFG + (0x20 * port), RTL8367C_PORT0_EEECFG_EEE_100M_OFFSET, 1)) != RT_ERR_OK)
                 return retVal;

             if((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_PORT0_EEECFG + (0x20 * port), RTL8367C_PORT0_EEECFG_EEE_GIGA_500M_OFFSET, 1)) != RT_ERR_OK)
                 return retVal;

             if((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_PORT0_EEECFG + (0x20 * port), RTL8367C_PORT0_EEECFG_EEE_TX_OFFSET, 1)) != RT_ERR_OK)
                 return retVal;

             if((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_PORT0_EEECFG + (0x20 * port), RTL8367C_PORT0_EEECFG_EEE_RX_OFFSET, 1)) != RT_ERR_OK)
                 return retVal;

             if((retVal = rtl8367c_getAsicPHYOCPReg(port, 0xA428, &regData)) != RT_ERR_OK)
                return retVal;

             regData &= ~(0x0200);
             if((retVal = rtl8367c_setAsicPHYOCPReg(port, 0xA428, regData)) != RT_ERR_OK)
                 return retVal;

             if((regValue & 0x00F0) == 0x00A0)
             {
                 if((retVal = rtl8367c_getAsicPHYOCPReg(port, 0xA5D0, &regData)) != RT_ERR_OK)
                     return retVal;

                 regData |= 0x0006;
                 if((retVal = rtl8367c_setAsicPHYOCPReg(port, 0xA5D0, regData)) != RT_ERR_OK)
                     return retVal;
             }
         }
    }

    if((retVal = rtl8367c_setAsicReg(RTL8367C_REG_UTP_FIB_DET, 0x15BB)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicReg(0x1303, 0x06D6)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicReg(0x1304, 0x0700)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicReg(0x13E2, 0x003F)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicReg(0x13F9, 0x0090)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicReg(0x121e, 0x03CA)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicReg(0x1233, 0x0352)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicReg(0x1237, 0x00a0)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicReg(0x123a, 0x0030)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicReg(0x1239, 0x0084)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicReg(0x0301, 0x1000)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicReg(0x1349, 0x001F)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicRegBit(0x18e0, 0, 0)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicRegBit(0x122b, 14, 1)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicRegBits(0x1305, 0xC000, 3)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

static rtk_api_ret_t _rtk_switch_init_8370b(void)
{
    ret_t retVal;
    rtk_uint32 regData, tmp = 0;
    rtk_uint32 i, prf, counter;
    rtk_uint32 long_link[8] = {0x0210, 0x03e8, 0x0218, 0x03f0, 0x0220, 0x03f8, 0x0208, 0x03e0 };

    if((retVal = rtl8367c_setAsicRegBits(0x1205, 0x0300, 3)) != RT_ERR_OK)
        return retVal;


    for(i=0; i<8; i++)
    {
      if ((retVal = rtl8367c_getAsicPHYOCPReg(i, 0xa420, &regData)) != RT_ERR_OK)
          return retVal;
        tmp = regData & 0x7 ;
       if(tmp == 0x3)
       {
           prf = 1;
           if((retVal = rtl8367c_setAsicPHYOCPReg(i, 0xb83e, 0x6fa9)) != RT_ERR_OK)
              return retVal;
           if((retVal = rtl8367c_setAsicPHYOCPReg(i, 0xb840, 0xa9)) != RT_ERR_OK)
               return retVal;
           for(counter = 0; counter < 10000; counter++); //delay

           if ((retVal = rtl8367c_getAsicPHYOCPReg(i, 0xb820, &regData)) != RT_ERR_OK)
               return retVal;
           tmp = regData | 0x10;
           if ((retVal = rtl8367c_setAsicPHYOCPReg(i, 0xb820, tmp)) != RT_ERR_OK)
               return retVal;
           for(counter = 0; counter < 10000; counter++); //delay
           counter = 0;
           do{
              counter = counter + 1;
              if ((retVal = rtl8367c_getAsicPHYOCPReg(i, 0xb800, &regData)) != RT_ERR_OK)
                   return retVal;
              tmp = regData & 0x40;
              if(tmp != 0)
                break;
           } while (counter < 20);   //Wait for patch ready = 1...
       }
   }
    if ((retVal = rtl8367c_getAsicReg(0x1d01, &regData)) != RT_ERR_OK)
        return retVal;
    tmp = regData;
    tmp = tmp | 0x3BE0; /*Broadcast port enable*/
    tmp = tmp & 0xFFE0; /*Phy_id = 0 */
    if((retVal = rtl8367c_setAsicReg(0x1d01, tmp)) != RT_ERR_OK)
        return retVal;

    for(i=0;i < 210; i++)
    {
        if((retVal = rtl8367c_setAsicPHYOCPReg(0, PatchChipData[i][0], PatchChipData[i][1])) != RT_ERR_OK)
             return retVal;
    }

   if((retVal = rtl8367c_setAsicReg(0x1d01, regData)) != RT_ERR_OK)
        return retVal;

    for(i=0; i < 8; i++)
    {
        if((retVal = rtl8367c_setAsicPHYOCPReg(i, 0xa4b4, long_link[i])) != RT_ERR_OK)
             return retVal;
    }

  if (prf == 0x1)
     {
        for(i=0; i<8; i++)
        {
         if ((retVal = rtl8367c_getAsicPHYOCPReg(i, 0xb820, &regData)) != RT_ERR_OK)
             return retVal;
       tmp = regData & 0xFFEF;
       if ((retVal = rtl8367c_setAsicPHYOCPReg(i, 0xb820, tmp)) != RT_ERR_OK)
             return retVal;

       for(counter = 0; counter < 10000; counter++); //delay

       counter = 0;
       do{
            counter = counter + 1;
            if ((retVal = rtl8367c_getAsicPHYOCPReg(i, 0xb800, &regData)) != RT_ERR_OK)
              return retVal;
            tmp = regData & 0x40;
            if( tmp == 0 )
               break;
       } while (counter < 20);   //Wait for patch ready = 1...
      if ((retVal = rtl8367c_setAsicPHYOCPReg(i, 0xb83e, 0x6f48)) != RT_ERR_OK)
          return retVal;
      if ((retVal = rtl8367c_setAsicPHYOCPReg(i, 0xb840, 0xfa)) != RT_ERR_OK)
          return retVal;
          }
   }

    /*Check phy link status*/
    for(i=0; i<8; i++)
    {
      if ((retVal = rtl8367c_getAsicPHYOCPReg(i, 0xa400, &regData)) != RT_ERR_OK)
          return retVal;
      tmp = regData & 0x800;
        if(tmp == 0x0)
            {
              tmp = regData | 0x200;
          if ((retVal = rtl8367c_setAsicPHYOCPReg(i, 0xa400, tmp)) != RT_ERR_OK)
             return retVal;
            }
    }

  for(counter = 0; counter < 10000; counter++); //delay

  return RT_ERR_OK;
}

static rtk_api_ret_t _rtk_switch_init_8364b(void)
{
    ret_t retVal;
    rtk_uint32 regData;

    /*enable EEE, include mac & phy*/

    if ((retVal = rtl8367c_setAsicRegBits(0x38, 0x300, 3)) != RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8367c_setAsicRegBits(0x78, 0x300, 3)) != RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8367c_setAsicRegBits(0xd8, 0x300, 0)) != RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8367c_setAsicRegBits(0xf8, 0x300, 0)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicPHYOCPReg(1, 0xa5d0, 6)) != RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8367c_setAsicPHYOCPReg(3, 0xa5d0, 6)) != RT_ERR_OK)
        return retVal;

    /*PAD para*/

    /*EXT1 PAD Para*/
    if ((retVal = rtl8367c_getAsicReg(0x1303, &regData)) != RT_ERR_OK)
        return retVal;
    regData &= 0xFFFFFFFE;
    regData |= 0x250;
    if((retVal = rtl8367c_setAsicReg(0x1303, regData)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicRegBits(0x1304, 0x7000, 0)) != RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8367c_setAsicRegBits(0x1304, 0x700, 7)) != RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8367c_setAsicRegBits(0x13f9, 0x38, 0)) != RT_ERR_OK)
        return retVal;

    /*EXT2 PAD Para*/
    if ((retVal = rtl8367c_setAsicRegBit(0x1303, 10, 1)) != RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8367c_setAsicRegBits(0x13E2, 0x1ff, 0x26)) != RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8367c_setAsicRegBits(0x13f9, 0x1c0, 0)) != RT_ERR_OK)
        return retVal;


    /*SDS PATCH*/
    /*SP_CFG_EN_LINK_FIB1G*/
    if((retVal = rtl8367c_getAsicSdsReg(0, 4, 0, &regData)) != RT_ERR_OK)
        return retVal;
    regData |= 0x4;
    if((retVal = rtl8367c_setAsicSdsReg(0,4,0, regData)) != RT_ERR_OK)
        return retVal;

    /*FIB100 Down-speed*/
    if((retVal = rtl8367c_getAsicSdsReg(0, 1, 0, &regData)) != RT_ERR_OK)
        return retVal;
    regData |= 0x20;
    if((retVal = rtl8367c_setAsicSdsReg(0,1,0, regData)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

static rtk_api_ret_t _rtk_switch_init_8363sc_vb(void)
{

    ret_t retVal;
    rtk_uint32 regData;

    /*enable EEE, include mac & phy*/

    if ((retVal = rtl8367c_setAsicRegBits(0x38, 0x300, 3)) != RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8367c_setAsicRegBits(0x78, 0x300, 3)) != RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8367c_setAsicRegBits(0xd8, 0x300, 0)) != RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8367c_setAsicRegBits(0xf8, 0x300, 0)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicPHYOCPReg(1, 0xa5d0, 6)) != RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8367c_setAsicPHYOCPReg(3, 0xa5d0, 6)) != RT_ERR_OK)
        return retVal;

    /*PAD para*/

    /*EXT1 PAD Para*/
    if ((retVal = rtl8367c_getAsicReg(0x1303, &regData)) != RT_ERR_OK)
        return retVal;
    regData &= 0xFFFFFFFE;
    regData |= 0x250;
    if((retVal = rtl8367c_setAsicReg(0x1303, regData)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicRegBits(0x1304, 0x7000, 0)) != RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8367c_setAsicRegBits(0x1304, 0x700, 7)) != RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8367c_setAsicRegBits(0x13f9, 0x38, 0)) != RT_ERR_OK)
        return retVal;

    /*EXT2 PAD Para*/
    if ((retVal = rtl8367c_setAsicRegBit(0x1303, 10, 1)) != RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8367c_setAsicRegBits(0x13E2, 0x1ff, 0x26)) != RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8367c_setAsicRegBits(0x13f9, 0x1c0, 0)) != RT_ERR_OK)
        return retVal;


    /*SDS PATCH*/
    /*SP_CFG_EN_LINK_FIB1G*/
    if((retVal = rtl8367c_getAsicSdsReg(0, 4, 0, &regData)) != RT_ERR_OK)
        return retVal;
    regData |= 0x4;
    if((retVal = rtl8367c_setAsicSdsReg(0,4,0, regData)) != RT_ERR_OK)
        return retVal;

    /*FIB100 Down-speed*/
    if((retVal = rtl8367c_getAsicSdsReg(0, 1, 0, &regData)) != RT_ERR_OK)
        return retVal;
    regData |= 0x20;
    if((retVal = rtl8367c_setAsicSdsReg(0,1,0, regData)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_switch_probe
 * Description:
 *      Probe switch
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Switch probed
 *      RT_ERR_FAILED   - Switch Unprobed.
 * Note:
 *
 */
rtk_api_ret_t rtk_switch_probe(switch_chip_t *pSwitchChip)
{
#if defined(FORCE_PROBE_RTL8367C)

    *pSwitchChip = CHIP_RTL8367C;
    halCtrl = &rtl8367c_hal_Ctrl;

#elif defined(FORCE_PROBE_RTL8370B)

    *pSwitchChip = CHIP_RTL8370B;
    halCtrl = &rtl8370b_hal_Ctrl;

#elif defined(FORCE_PROBE_RTL8364B)

    *pSwitchChip = CHIP_RTL8364B;
    halCtrl = &rtl8364b_hal_Ctrl;

#elif defined(FORCE_PROBE_RTL8363SC_VB)

    *pSwitchChip = CHIP_RTL8363SC_VB;
    halCtrl = &rtl8363sc_vb_hal_Ctrl;

#else
    rtk_uint32 retVal;
    rtk_uint32 data, regValue;

    if((retVal = rtl8367c_setAsicReg(0x13C2, 0x0249)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_getAsicReg(0x1300, &data)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_getAsicReg(0x1301, &regValue)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicReg(0x13C2, 0x0000)) != RT_ERR_OK)
        return retVal;

    switch (data)
    {
        case 0x0276:
        case 0x0597:
        case 0x6367:
            *pSwitchChip = CHIP_RTL8367C;
            halCtrl = &rtl8367c_hal_Ctrl;
            break;
        case 0x0652:
        case 0x6368:
            *pSwitchChip = CHIP_RTL8370B;
            halCtrl = &rtl8370b_hal_Ctrl;
            break;
        case 0x0801:
        case 0x6511:
            if( (regValue & 0x00F0) == 0x0080)
            {
                *pSwitchChip = CHIP_RTL8363SC_VB;
                halCtrl = &rtl8363sc_vb_hal_Ctrl;
            }
            else
            {
                *pSwitchChip = CHIP_RTL8364B;
                halCtrl = &rtl8364b_hal_Ctrl;
            }
            break;
        default:
            return RT_ERR_FAILED;
    }
#endif

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_switch_initialState_set
 * Description:
 *      Set initial status
 * Input:
 *      state   - Initial state;
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Initialized
 *      RT_ERR_FAILED   - Uninitialized
 * Note:
 *
 */
rtk_api_ret_t rtk_switch_initialState_set(init_state_t state)
{
    if(state >= INIT_STATE_END)
        return RT_ERR_FAILED;

    init_state = state;
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_switch_initialState_get
 * Description:
 *      Get initial status
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      INIT_COMPLETED     - Initialized
 *      INIT_NOT_COMPLETED - Uninitialized
 * Note:
 *
 */
init_state_t rtk_switch_initialState_get(void)
{
    return init_state;
}

/* Function Name:
 *      rtk_switch_logicalPortCheck
 * Description:
 *      Check logical port ID.
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is correct
 *      RT_ERR_FAILED   - Port ID is not correct
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
rtk_api_ret_t rtk_switch_logicalPortCheck(rtk_port_t logicalPort)
{
    if(init_state != INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(logicalPort >= RTK_SWITCH_PORT_NUM)
        return RT_ERR_FAILED;

    if(halCtrl->l2p_port[logicalPort] == 0xFF)
        return RT_ERR_FAILED;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_switch_isUtpPort
 * Description:
 *      Check is logical port a UTP port
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is a UTP port
 *      RT_ERR_FAILED   - Port ID is not a UTP port
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
rtk_api_ret_t rtk_switch_isUtpPort(rtk_port_t logicalPort)
{
    if(init_state != INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(logicalPort >= RTK_SWITCH_PORT_NUM)
        return RT_ERR_FAILED;

    if(halCtrl->log_port_type[logicalPort] == UTP_PORT)
        return RT_ERR_OK;
    else
        return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_switch_isExtPort
 * Description:
 *      Check is logical port a Extension port
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is a EXT port
 *      RT_ERR_FAILED   - Port ID is not a EXT port
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
rtk_api_ret_t rtk_switch_isExtPort(rtk_port_t logicalPort)
{
    if(init_state != INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(logicalPort >= RTK_SWITCH_PORT_NUM)
        return RT_ERR_FAILED;

    if(halCtrl->log_port_type[logicalPort] == EXT_PORT)
        return RT_ERR_OK;
    else
        return RT_ERR_FAILED;
}


/* Function Name:
 *      rtk_switch_isHsgPort
 * Description:
 *      Check is logical port a HSG port
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is a HSG port
 *      RT_ERR_FAILED   - Port ID is not a HSG port
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
rtk_api_ret_t rtk_switch_isHsgPort(rtk_port_t logicalPort)
{
    if(init_state != INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(logicalPort >= RTK_SWITCH_PORT_NUM)
        return RT_ERR_FAILED;

    if(logicalPort == halCtrl->hsg_logical_port)
        return RT_ERR_OK;
    else
        return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_switch_isSgmiiPort
 * Description:
 *      Check is logical port a SGMII port
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is a SGMII port
 *      RT_ERR_FAILED   - Port ID is not a SGMII port
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
rtk_api_ret_t rtk_switch_isSgmiiPort(rtk_port_t logicalPort)
{
    if(init_state != INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(logicalPort >= RTK_SWITCH_PORT_NUM)
        return RT_ERR_FAILED;

    if( ((0x01 << logicalPort) & halCtrl->sg_logical_portmask) != 0)
        return RT_ERR_OK;
    else
        return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_switch_isCPUPort
 * Description:
 *      Check is logical port a CPU port
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is a CPU port
 *      RT_ERR_FAILED   - Port ID is not a CPU port
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
rtk_api_ret_t rtk_switch_isCPUPort(rtk_port_t logicalPort)
{
    if(init_state != INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(logicalPort >= RTK_SWITCH_PORT_NUM)
        return RT_ERR_FAILED;

    if( ((0x01 << logicalPort) & halCtrl->valid_cpu_portmask) != 0)
        return RT_ERR_OK;
    else
        return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_switch_isComboPort
 * Description:
 *      Check is logical port a Combo port
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is a combo port
 *      RT_ERR_FAILED   - Port ID is not a combo port
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
rtk_api_ret_t rtk_switch_isComboPort(rtk_port_t logicalPort)
{
    if(init_state != INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(logicalPort >= RTK_SWITCH_PORT_NUM)
        return RT_ERR_FAILED;

    if(halCtrl->combo_logical_port == logicalPort)
        return RT_ERR_OK;
    else
        return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_switch_ComboPort_get
 * Description:
 *      Get Combo port ID
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      Port ID of combo port
 * Note:
 *
 */
rtk_uint32 rtk_switch_ComboPort_get(void)
{
    return halCtrl->combo_logical_port;
}

/* Function Name:
 *      rtk_switch_isPtpPort
 * Description:
 *      Check is logical port a PTP port
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is a PTP port
 *      RT_ERR_FAILED   - Port ID is not a PTP port
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
rtk_api_ret_t rtk_switch_isPtpPort(rtk_port_t logicalPort)
{
    if(init_state != INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(logicalPort >= RTK_SWITCH_PORT_NUM)
        return RT_ERR_FAILED;

    if(halCtrl->ptp_port[logicalPort] == 1)
        return RT_ERR_OK;
    else
        return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_switch_port_L2P_get
 * Description:
 *      Get physical port ID
 * Input:
 *      logicalPort       - logical port ID
 * Output:
 *      None
 * Return:
 *      Physical port ID
 * Note:
 *
 */
rtk_uint32 rtk_switch_port_L2P_get(rtk_port_t logicalPort)
{
    if(init_state != INIT_COMPLETED)
        return UNDEFINE_PHY_PORT;

    if(logicalPort >= RTK_SWITCH_PORT_NUM)
        return UNDEFINE_PHY_PORT;

    return (halCtrl->l2p_port[logicalPort]);
}

/* Function Name:
 *      rtk_switch_port_P2L_get
 * Description:
 *      Get logical port ID
 * Input:
 *      physicalPort       - physical port ID
 * Output:
 *      None
 * Return:
 *      logical port ID
 * Note:
 *
 */
rtk_port_t rtk_switch_port_P2L_get(rtk_uint32 physicalPort)
{
    if(init_state != INIT_COMPLETED)
        return UNDEFINE_PORT;

    if(physicalPort >= RTK_SWITCH_PORT_NUM)
        return UNDEFINE_PORT;

    return (halCtrl->p2l_port[physicalPort]);
}

/* Function Name:
 *      rtk_switch_isPortMaskValid
 * Description:
 *      Check portmask is valid or not
 * Input:
 *      pPmask       - logical port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - port mask is valid
 *      RT_ERR_FAILED       - port mask is not valid
 *      RT_ERR_NOT_INIT     - Not Initialize
 *      RT_ERR_NULL_POINTER - Null pointer
 * Note:
 *
 */
rtk_api_ret_t rtk_switch_isPortMaskValid(rtk_portmask_t *pPmask)
{
    if(init_state != INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(NULL == pPmask)
        return RT_ERR_NULL_POINTER;

    if( (pPmask->bits[0] | halCtrl->valid_portmask) != halCtrl->valid_portmask )
        return RT_ERR_FAILED;
    else
        return RT_ERR_OK;
}

/* Function Name:
 *      rtk_switch_isPortMaskUtp
 * Description:
 *      Check all ports in portmask are only UTP port
 * Input:
 *      pPmask       - logical port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Only UTP port in port mask
 *      RT_ERR_FAILED       - Not only UTP port in port mask
 *      RT_ERR_NOT_INIT     - Not Initialize
 *      RT_ERR_NULL_POINTER - Null pointer
 * Note:
 *
 */
rtk_api_ret_t rtk_switch_isPortMaskUtp(rtk_portmask_t *pPmask)
{
    if(init_state != INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(NULL == pPmask)
        return RT_ERR_NULL_POINTER;

    if( (pPmask->bits[0] | halCtrl->valid_utp_portmask) != halCtrl->valid_utp_portmask )
        return RT_ERR_FAILED;
    else
        return RT_ERR_OK;
}

/* Function Name:
 *      rtk_switch_isPortMaskExt
 * Description:
 *      Check all ports in portmask are only EXT port
 * Input:
 *      pPmask       - logical port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Only EXT port in port mask
 *      RT_ERR_FAILED       - Not only EXT port in port mask
 *      RT_ERR_NOT_INIT     - Not Initialize
 *      RT_ERR_NULL_POINTER - Null pointer
 * Note:
 *
 */
rtk_api_ret_t rtk_switch_isPortMaskExt(rtk_portmask_t *pPmask)
{
    if(init_state != INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(NULL == pPmask)
        return RT_ERR_NULL_POINTER;

    if( (pPmask->bits[0] | halCtrl->valid_ext_portmask) != halCtrl->valid_ext_portmask )
        return RT_ERR_FAILED;
    else
        return RT_ERR_OK;
}

/* Function Name:
 *      rtk_switch_portmask_L2P_get
 * Description:
 *      Get physicl portmask from logical portmask
 * Input:
 *      pLogicalPmask       - logical port mask
 * Output:
 *      pPhysicalPortmask   - physical port mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_NOT_INIT     - Not Initialize
 *      RT_ERR_NULL_POINTER - Null pointer
 *      RT_ERR_PORT_MASK    - Error port mask
 * Note:
 *
 */
rtk_api_ret_t rtk_switch_portmask_L2P_get(rtk_portmask_t *pLogicalPmask, rtk_uint32 *pPhysicalPortmask)
{
    rtk_uint32 log_port, phy_port;

    if(init_state != INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(NULL == pLogicalPmask)
        return RT_ERR_NULL_POINTER;

    if(NULL == pPhysicalPortmask)
        return RT_ERR_NULL_POINTER;

    if(rtk_switch_isPortMaskValid(pLogicalPmask) != RT_ERR_OK)
        return RT_ERR_PORT_MASK;

    /* reset physical port mask */
    *pPhysicalPortmask = 0;

    RTK_PORTMASK_SCAN((*pLogicalPmask), log_port)
    {
        phy_port = rtk_switch_port_L2P_get((rtk_port_t)log_port);
        *pPhysicalPortmask |= (0x0001 << phy_port);
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_switch_portmask_P2L_get
 * Description:
 *      Get logical portmask from physical portmask
 * Input:
 *      physicalPortmask    - physical port mask
 * Output:
 *      pLogicalPmask       - logical port mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_NOT_INIT     - Not Initialize
 *      RT_ERR_NULL_POINTER - Null pointer
 *      RT_ERR_PORT_MASK    - Error port mask
 * Note:
 *
 */
rtk_api_ret_t rtk_switch_portmask_P2L_get(rtk_uint32 physicalPortmask, rtk_portmask_t *pLogicalPmask)
{
    rtk_uint32 log_port, phy_port;

    if(init_state != INIT_COMPLETED)
        return RT_ERR_NOT_INIT;

    if(NULL == pLogicalPmask)
        return RT_ERR_NULL_POINTER;

    RTK_PORTMASK_CLEAR(*pLogicalPmask);

    for(phy_port = halCtrl->min_phy_port; phy_port <= halCtrl->max_phy_port; phy_port++)
    {
        if(physicalPortmask & (0x0001 << phy_port))
        {
            log_port = rtk_switch_port_P2L_get(phy_port);
            if(log_port != UNDEFINE_PORT)
            {
                RTK_PORTMASK_PORT_SET(*pLogicalPmask, log_port);
            }
        }
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_switch_phyPortMask_get
 * Description:
 *      Get physical portmask
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      0x00                - Not Initialize
 *      Other value         - Physical port mask
 * Note:
 *
 */
rtk_uint32 rtk_switch_phyPortMask_get(void)
{
    if(init_state != INIT_COMPLETED)
        return 0x00; /* No port in portmask */

    return (halCtrl->phy_portmask);
}

/* Function Name:
 *      rtk_switch_logPortMask_get
 * Description:
 *      Get Logical portmask
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_NOT_INIT     - Not Initialize
 *      RT_ERR_NULL_POINTER - Null pointer
 * Note:
 *
 */
rtk_api_ret_t rtk_switch_logPortMask_get(rtk_portmask_t *pPortmask)
{
    if(init_state != INIT_COMPLETED)
        return RT_ERR_FAILED;

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    pPortmask->bits[0] = halCtrl->valid_portmask;
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_switch_init
 * Description:
 *      Set chip to default configuration enviroment
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      The API can set chip registers to default configuration for different release chip model.
 */
rtk_api_ret_t rtk_switch_init(void)
{
    rtk_uint32  retVal;
    rtl8367c_rma_t rmaCfg;
    switch_chip_t   switchChip;

    /* probe switch */
    if((retVal = rtk_switch_probe(&switchChip)) != RT_ERR_OK)
        return retVal;

    /* Set initial state */

    if((retVal = rtk_switch_initialState_set(INIT_COMPLETED)) != RT_ERR_OK)
        return retVal;

    /* Initial */
    switch(switchChip)
    {
        case CHIP_RTL8367C:
            if((retVal = _rtk_switch_init_8367c()) != RT_ERR_OK)
                return retVal;
            break;
        case CHIP_RTL8370B:
            if((retVal = _rtk_switch_init_8370b()) != RT_ERR_OK)
                return retVal;
            break;
        case CHIP_RTL8364B:
            if((retVal = _rtk_switch_init_8364b()) != RT_ERR_OK)
                return retVal;
            break;
        case CHIP_RTL8363SC_VB:
            if((retVal = _rtk_switch_init_8363sc_vb()) != RT_ERR_OK)
                return retVal;
            break;
        default:
            return RT_ERR_CHIP_NOT_FOUND;
    }

    /* Set Old max packet length to 16K */
    if((retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_MAX_LENGTH_LIMINT_IPG, RTL8367C_MAX_LENTH_CTRL_MASK, 3)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_MAX_LEN_RX_TX, RTL8367C_MAX_LEN_RX_TX_MASK, 3)) != RT_ERR_OK)
        return retVal;

    /* ACL Mode */
    if((retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_ACL_ACCESS_MODE, RTL8367C_ACL_ACCESS_MODE_MASK, 1)) != RT_ERR_OK)
        return retVal;

    /* Max rate */
    if((retVal = rtk_rate_igrBandwidthCtrlRate_set(halCtrl->hsg_logical_port, RTL8367C_QOS_RATE_INPUT_MAX_HSG, DISABLED, ENABLED)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtk_rate_egrBandwidthCtrlRate_set(halCtrl->hsg_logical_port, RTL8367C_QOS_RATE_INPUT_MAX_HSG, ENABLED)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicReg(0x03fa, 0x0007)) != RT_ERR_OK)
        return retVal;

    /* Change unknown DA to per port setting */
    if((retVal = rtl8367c_setAsicRegBits(RTL8367C_PORT_SECURIT_CTRL_REG, RTL8367C_UNKNOWN_UNICAST_DA_BEHAVE_MASK, 3)) != RT_ERR_OK)
        return retVal;

    /* LUT lookup OP = 1 */
    if ((retVal = rtl8367c_setAsicLutIpLookupMethod(1))!=RT_ERR_OK)
        return retVal;

    /* Set RMA */
    rmaCfg.portiso_leaky = 0;
    rmaCfg.vlan_leaky = 0;
    rmaCfg.keep_format = 0;
    rmaCfg.trap_priority = 0;
    rmaCfg.discard_storm_filter = 0;
    rmaCfg.operation = 0;
    if ((retVal = rtl8367c_setAsicRma(2, &rmaCfg))!=RT_ERR_OK)
        return retVal;

    /* Enable TX Mirror isolation leaky */
    if ((retVal = rtl8367c_setAsicPortMirrorIsolationTxLeaky(ENABLED)) != RT_ERR_OK)
        return retVal;

    /* INT EN */
    if((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_IO_MISC_FUNC, RTL8367C_INT_EN_OFFSET, 1)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_switch_portMaxPktLen_set
 * Description:
 *      Set Max packet length
 * Input:
 *      port    - Port ID
 *      speed   - Speed
 *      cfgId   - Configuration ID
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 */
rtk_api_ret_t rtk_switch_portMaxPktLen_set(rtk_port_t port, rtk_switch_maxPktLen_linkSpeed_t speed, rtk_uint32 cfgId)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(speed >= MAXPKTLEN_LINK_SPEED_END)
        return RT_ERR_INPUT;

    if(cfgId > MAXPKTLEN_CFG_ID_MAX)
        return RT_ERR_INPUT;

    if((retVal = rtl8367c_setAsicMaxLength(rtk_switch_port_L2P_get(port), (rtk_uint32)speed, cfgId)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_switch_portMaxPktLen_get
 * Description:
 *      Get Max packet length
 * Input:
 *      port    - Port ID
 *      speed   - Speed
 * Output:
 *      pCfgId  - Configuration ID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 */
rtk_api_ret_t rtk_switch_portMaxPktLen_get(rtk_port_t port, rtk_switch_maxPktLen_linkSpeed_t speed, rtk_uint32 *pCfgId)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(speed >= MAXPKTLEN_LINK_SPEED_END)
        return RT_ERR_INPUT;

    if(NULL == pCfgId)
        return RT_ERR_NULL_POINTER;

    if((retVal = rtl8367c_getAsicMaxLength(rtk_switch_port_L2P_get(port), (rtk_uint32)speed, pCfgId)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_switch_maxPktLenCfg_set
 * Description:
 *      Set Max packet length configuration
 * Input:
 *      cfgId   - Configuration ID
 *      pktLen  - Max packet length
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 */
rtk_api_ret_t rtk_switch_maxPktLenCfg_set(rtk_uint32 cfgId, rtk_uint32 pktLen)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(cfgId > MAXPKTLEN_CFG_ID_MAX)
        return RT_ERR_INPUT;

    if(pktLen > RTK_SWITCH_MAX_PKTLEN)
        return RT_ERR_INPUT;

    if((retVal = rtl8367c_setAsicMaxLengthCfg(cfgId, pktLen)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_switch_maxPktLenCfg_get
 * Description:
 *      Get Max packet length configuration
 * Input:
 *      cfgId   - Configuration ID
 *      pPktLen - Max packet length
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 */
rtk_api_ret_t rtk_switch_maxPktLenCfg_get(rtk_uint32 cfgId, rtk_uint32 *pPktLen)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(cfgId > MAXPKTLEN_CFG_ID_MAX)
        return RT_ERR_INPUT;

    if(NULL == pPktLen)
        return RT_ERR_NULL_POINTER;

    if((retVal = rtl8367c_getAsicMaxLengthCfg(cfgId, pPktLen)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_switch_greenEthernet_set
 * Description:
 *      Set all Ports Green Ethernet state.
 * Input:
 *      enable - Green Ethernet state.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - OK
 *      RT_ERR_FAILED   - Failed
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_ENABLE   - Invalid enable input.
 * Note:
 *      This API can set all Ports Green Ethernet state.
 *      The configuration is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtk_api_ret_t rtk_switch_greenEthernet_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 port;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    RTK_SCAN_ALL_LOG_PORT(port)
    {
        if(rtk_switch_isUtpPort(port) == RT_ERR_OK)
        {
            if ((retVal = rtl8367c_setAsicPowerSaving(rtk_switch_port_L2P_get(port),enable))!=RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367c_setAsicGreenEthernet(rtk_switch_port_L2P_get(port), enable))!=RT_ERR_OK)
                return retVal;
        }
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_switch_greenEthernet_get
 * Description:
 *      Get all Ports Green Ethernet state.
 * Input:
 *      None
 * Output:
 *      pEnable - Green Ethernet state.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API can get Green Ethernet state.
 */
rtk_api_ret_t rtk_switch_greenEthernet_get(rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 port;
    rtk_uint32 state;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    RTK_SCAN_ALL_LOG_PORT(port)
    {
        if(rtk_switch_isUtpPort(port) == RT_ERR_OK)
        {
            if ((retVal = rtl8367c_getAsicPowerSaving(rtk_switch_port_L2P_get(port), &state))!=RT_ERR_OK)
                return retVal;

            if(state == DISABLED)
            {
                *pEnable = DISABLED;
                return RT_ERR_OK;
            }

            if ((retVal = rtl8367c_getAsicGreenEthernet(rtk_switch_port_L2P_get(port), &state))!=RT_ERR_OK)
                return retVal;

            if(state == DISABLED)
            {
                *pEnable = DISABLED;
                return RT_ERR_OK;
            }
        }
    }

    *pEnable = ENABLED;
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_switch_maxLogicalPort_get
 * Description:
 *      Get Max logical port ID
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      Max logical port
 * Note:
 *      This API can get max logical port
 */
rtk_port_t rtk_switch_maxLogicalPort_get(void)
{
    rtk_port_t port, maxLogicalPort = 0;

    /* Check initialization state */
    if(rtk_switch_initialState_get() != INIT_COMPLETED)
    {
        return UNDEFINE_PORT;
    }

    for(port = 0; port < RTK_SWITCH_PORT_NUM; port++)
    {
        if( (halCtrl->log_port_type[port] == UTP_PORT) || (halCtrl->log_port_type[port] == EXT_PORT) )
            maxLogicalPort = port;
    }

    return maxLogicalPort;
}

/* Function Name:
 *      rtk_switch_maxMeterId_get
 * Description:
 *      Get Max Meter ID
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      0x00                - Not Initialize
 *      Other value         - Max Meter ID
 * Note:
 *
 */
rtk_uint32 rtk_switch_maxMeterId_get(void)
{
    if(init_state != INIT_COMPLETED)
        return 0x00;

    return (halCtrl->max_meter_id);
}

/* Function Name:
 *      rtk_switch_maxLutAddrNumber_get
 * Description:
 *      Get Max LUT Address number
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      0x00                - Not Initialize
 *      Other value         - Max LUT Address number
 * Note:
 *
 */
rtk_uint32 rtk_switch_maxLutAddrNumber_get(void)
{
    if(init_state != INIT_COMPLETED)
        return 0x00;

    return (halCtrl->max_lut_addr_num);
}

/* Function Name:
 *      rtk_switch_isValidTrunkGrpId
 * Description:
 *      Check if trunk group is valid or not
 * Input:
 *      grpId       - Group ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Trunk Group ID is valid
 *      RT_ERR_LA_TRUNK_ID  - Trunk Group ID is not valid
 * Note:
 *
 */
rtk_uint32 rtk_switch_isValidTrunkGrpId(rtk_uint32 grpId)
{
    if(init_state != INIT_COMPLETED)
        return 0x00;

    if( (halCtrl->trunk_group_mask & (0x01 << grpId) ) != 0)
        return RT_ERR_OK;
    else
        return RT_ERR_LA_TRUNK_ID;

}

