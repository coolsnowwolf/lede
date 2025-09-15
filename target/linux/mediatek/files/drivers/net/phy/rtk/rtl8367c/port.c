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
 * $Date: 2017-03-08 15:13:58 +0800 (週三, 08 三月 2017) $
 *
 * Purpose : RTK switch high-level API for RTL8367/RTL8367C
 * Feature : Here is a list of all functions and variables in Port module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <port.h>
#include <string.h>

#include <rtl8367c_asicdrv.h>
#include <rtl8367c_asicdrv_port.h>
#include <rtl8367c_asicdrv_misc.h>
#include <rtl8367c_asicdrv_portIsolation.h>

#define FIBER_INIT_SIZE 1507
CONST_T rtk_uint8 Fiber[FIBER_INIT_SIZE] = {
0x02,0x04,0x41,0xE4,0xF5,0xA8,0xD2,0xAF,
0x22,0x00,0x00,0x02,0x05,0x2D,0xE4,0x90,
0x06,0x2A,0xF0,0xFD,0x7C,0x01,0x7F,0x3F,
0x7E,0x1D,0x12,0x05,0xAF,0x7D,0x40,0x12,
0x02,0x5F,0xE4,0xFF,0xFE,0xFD,0x80,0x08,
0x12,0x05,0x9E,0x50,0x0C,0x12,0x05,0x8B,
0xFC,0x90,0x06,0x24,0x12,0x03,0x76,0x80,
0xEF,0xE4,0xF5,0xA8,0xD2,0xAF,0x7D,0x1F,
0xFC,0x7F,0x49,0x7E,0x13,0x12,0x05,0xAF,
0x12,0x05,0xD6,0x7D,0xD7,0x12,0x02,0x1E,
0x7D,0x80,0x12,0x01,0xCA,0x7D,0x94,0x7C,
0xF9,0x12,0x02,0x3B,0x7D,0x81,0x12,0x01,
0xCA,0x7D,0xA2,0x7C,0x31,0x12,0x02,0x3B,
0x7D,0x82,0x12,0x01,0xDF,0x7D,0x60,0x7C,
0x69,0x12,0x02,0x43,0x7D,0x83,0x12,0x01,
0xDF,0x7D,0x28,0x7C,0x97,0x12,0x02,0x43,
0x7D,0x84,0x12,0x01,0xF4,0x7D,0x85,0x7C,
0x9D,0x12,0x02,0x57,0x7D,0x23,0x12,0x01,
0xF4,0x7D,0x10,0x7C,0xD8,0x12,0x02,0x57,
0x7D,0x24,0x7C,0x04,0x12,0x02,0x28,0x7D,
0x00,0x12,0x02,0x1E,0x7D,0x2F,0x12,0x02,
0x09,0x7D,0x20,0x7C,0x0F,0x7F,0x02,0x7E,
0x66,0x12,0x05,0xAF,0x7D,0x01,0x12,0x02,
0x09,0x7D,0x04,0x7C,0x00,0x7F,0x01,0x7E,
0x66,0x12,0x05,0xAF,0x7D,0x80,0x7C,0x00,
0x7F,0x00,0x7E,0x66,0x12,0x05,0xAF,0x7F,
0x02,0x7E,0x66,0x12,0x02,0x4B,0x44,0x02,
0xFF,0x90,0x06,0x28,0xEE,0xF0,0xA3,0xEF,
0xF0,0x44,0x04,0xFF,0x90,0x06,0x28,0xEE,
0xF0,0xFC,0xA3,0xEF,0xF0,0xFD,0x7F,0x02,
0x7E,0x66,0x12,0x05,0xAF,0x7D,0x04,0x7C,
0x00,0x12,0x02,0x28,0x7D,0xB9,0x7C,0x15,
0x7F,0xEB,0x7E,0x13,0x12,0x05,0xAF,0x7D,
0x07,0x7C,0x00,0x7F,0xE7,0x7E,0x13,0x12,
0x05,0xAF,0x7D,0x40,0x7C,0x11,0x7F,0x00,
0x7E,0x62,0x12,0x05,0xAF,0x12,0x03,0x82,
0x7D,0x41,0x12,0x02,0x5F,0xE4,0xFF,0xFE,
0xFD,0x80,0x08,0x12,0x05,0x9E,0x50,0x0C,
0x12,0x05,0x8B,0xFC,0x90,0x06,0x24,0x12,
0x03,0x76,0x80,0xEF,0xC2,0x00,0xC2,0x01,
0xD2,0xA9,0xD2,0x8C,0x7F,0x01,0x7E,0x62,
0x12,0x02,0x4B,0x30,0xE2,0x05,0xE4,0xA3,
0xF0,0x80,0xF1,0x90,0x06,0x2A,0xE0,0x70,
0x12,0x12,0x01,0x89,0x90,0x06,0x2A,0x74,
0x01,0xF0,0xE4,0x90,0x06,0x2D,0xF0,0xA3,
0xF0,0x80,0xD9,0xC3,0x90,0x06,0x2E,0xE0,
0x94,0x64,0x90,0x06,0x2D,0xE0,0x94,0x00,
0x40,0xCA,0xE4,0xF0,0xA3,0xF0,0x12,0x01,
0x89,0x90,0x06,0x2A,0x74,0x01,0xF0,0x80,
0xBB,0x7D,0x04,0xFC,0x7F,0x02,0x7E,0x66,
0x12,0x05,0xAF,0x7D,0x00,0x7C,0x04,0x7F,
0x01,0x7E,0x66,0x12,0x05,0xAF,0x7D,0xC0,
0x7C,0x00,0x7F,0x00,0x7E,0x66,0x12,0x05,
0xAF,0xE4,0xFD,0xFC,0x7F,0x02,0x7E,0x66,
0x12,0x05,0xAF,0x7D,0x00,0x7C,0x04,0x7F,
0x01,0x7E,0x66,0x12,0x05,0xAF,0x7D,0xC0,
0x7C,0x00,0x7F,0x00,0x7E,0x66,0x12,0x05,
0xAF,0x22,0x7C,0x04,0x7F,0x01,0x7E,0x66,
0x12,0x05,0xAF,0x7D,0xC0,0x7C,0x00,0x7F,
0x00,0x7E,0x66,0x12,0x05,0xAF,0x22,0x7C,
0x04,0x7F,0x01,0x7E,0x66,0x12,0x05,0xAF,
0x7D,0xC0,0x7C,0x00,0x7F,0x00,0x7E,0x66,
0x12,0x05,0xAF,0x22,0x7C,0x04,0x7F,0x01,
0x7E,0x66,0x12,0x05,0xAF,0x7D,0xC0,0x7C,
0x00,0x7F,0x00,0x7E,0x66,0x12,0x05,0xAF,
0x22,0x7C,0x00,0x7F,0x01,0x7E,0x66,0x12,
0x05,0xAF,0x7D,0xC0,0x7C,0x00,0x7F,0x00,
0x7E,0x66,0x12,0x05,0xAF,0x22,0x7C,0x04,
0x7F,0x02,0x7E,0x66,0x12,0x05,0xAF,0x22,
0x7F,0x01,0x7E,0x66,0x12,0x05,0xAF,0x7D,
0xC0,0x7C,0x00,0x7F,0x00,0x7E,0x66,0x12,
0x05,0xAF,0x22,0x7F,0x02,0x7E,0x66,0x12,
0x05,0xAF,0x22,0x7F,0x02,0x7E,0x66,0x12,
0x05,0xAF,0x22,0x12,0x05,0x67,0x90,0x06,
0x28,0xEE,0xF0,0xA3,0xEF,0xF0,0x22,0x7F,
0x02,0x7E,0x66,0x12,0x05,0xAF,0x22,0x7C,
0x00,0x7F,0x36,0x7E,0x13,0x12,0x05,0xAF,
0x22,0xC5,0xF0,0xF8,0xA3,0xE0,0x28,0xF0,
0xC5,0xF0,0xF8,0xE5,0x82,0x15,0x82,0x70,
0x02,0x15,0x83,0xE0,0x38,0xF0,0x22,0x75,
0xF0,0x08,0x75,0x82,0x00,0xEF,0x2F,0xFF,
0xEE,0x33,0xFE,0xCD,0x33,0xCD,0xCC,0x33,
0xCC,0xC5,0x82,0x33,0xC5,0x82,0x9B,0xED,
0x9A,0xEC,0x99,0xE5,0x82,0x98,0x40,0x0C,
0xF5,0x82,0xEE,0x9B,0xFE,0xED,0x9A,0xFD,
0xEC,0x99,0xFC,0x0F,0xD5,0xF0,0xD6,0xE4,
0xCE,0xFB,0xE4,0xCD,0xFA,0xE4,0xCC,0xF9,
0xA8,0x82,0x22,0xB8,0x00,0xC1,0xB9,0x00,
0x59,0xBA,0x00,0x2D,0xEC,0x8B,0xF0,0x84,
0xCF,0xCE,0xCD,0xFC,0xE5,0xF0,0xCB,0xF9,
0x78,0x18,0xEF,0x2F,0xFF,0xEE,0x33,0xFE,
0xED,0x33,0xFD,0xEC,0x33,0xFC,0xEB,0x33,
0xFB,0x10,0xD7,0x03,0x99,0x40,0x04,0xEB,
0x99,0xFB,0x0F,0xD8,0xE5,0xE4,0xF9,0xFA,
0x22,0x78,0x18,0xEF,0x2F,0xFF,0xEE,0x33,
0xFE,0xED,0x33,0xFD,0xEC,0x33,0xFC,0xC9,
0x33,0xC9,0x10,0xD7,0x05,0x9B,0xE9,0x9A,
0x40,0x07,0xEC,0x9B,0xFC,0xE9,0x9A,0xF9,
0x0F,0xD8,0xE0,0xE4,0xC9,0xFA,0xE4,0xCC,
0xFB,0x22,0x75,0xF0,0x10,0xEF,0x2F,0xFF,
0xEE,0x33,0xFE,0xED,0x33,0xFD,0xCC,0x33,
0xCC,0xC8,0x33,0xC8,0x10,0xD7,0x07,0x9B,
0xEC,0x9A,0xE8,0x99,0x40,0x0A,0xED,0x9B,
0xFD,0xEC,0x9A,0xFC,0xE8,0x99,0xF8,0x0F,
0xD5,0xF0,0xDA,0xE4,0xCD,0xFB,0xE4,0xCC,
0xFA,0xE4,0xC8,0xF9,0x22,0xEB,0x9F,0xF5,
0xF0,0xEA,0x9E,0x42,0xF0,0xE9,0x9D,0x42,
0xF0,0xE8,0x9C,0x45,0xF0,0x22,0xE0,0xFC,
0xA3,0xE0,0xFD,0xA3,0xE0,0xFE,0xA3,0xE0,
0xFF,0x22,0xE0,0xF8,0xA3,0xE0,0xF9,0xA3,
0xE0,0xFA,0xA3,0xE0,0xFB,0x22,0xEC,0xF0,
0xA3,0xED,0xF0,0xA3,0xEE,0xF0,0xA3,0xEF,
0xF0,0x22,0x12,0x03,0xF8,0x12,0x04,0x1A,
0x44,0x40,0x12,0x04,0x0F,0x7D,0x03,0x7C,
0x00,0x12,0x04,0x23,0x12,0x05,0xAF,0x12,
0x03,0xF8,0x12,0x04,0x1A,0x54,0xBF,0x12,
0x04,0x0F,0x7D,0x03,0x7C,0x00,0x12,0x03,
0xD0,0x7F,0x02,0x7E,0x66,0x12,0x05,0x67,
0xEF,0x54,0xFD,0x54,0xFE,0x12,0x04,0x33,
0x12,0x03,0xD0,0x7F,0x02,0x7E,0x66,0x12,
0x05,0x67,0xEF,0x44,0x02,0x44,0x01,0x12,
0x04,0x33,0x12,0x04,0x23,0x02,0x05,0xAF,
0x7F,0x01,0x7E,0x66,0x12,0x05,0xAF,0x7D,
0xC0,0x7C,0x00,0x7F,0x00,0x7E,0x66,0x12,
0x05,0xAF,0xE4,0xFD,0xFC,0x7F,0x01,0x7E,
0x66,0x12,0x05,0xAF,0x7D,0x80,0x7C,0x00,
0x7F,0x00,0x7E,0x66,0x12,0x05,0xAF,0x22,
0x7D,0x03,0x7C,0x00,0x7F,0x01,0x7E,0x66,
0x12,0x05,0xAF,0x7D,0x80,0x7C,0x00,0x7F,
0x00,0x7E,0x66,0x12,0x05,0xAF,0x22,0xFD,
0xAC,0x06,0x7F,0x02,0x7E,0x66,0x12,0x05,
0xAF,0x22,0x7F,0x02,0x7E,0x66,0x12,0x05,
0x67,0xEF,0x22,0x7F,0x01,0x7E,0x66,0x12,
0x05,0xAF,0x7D,0xC0,0x7C,0x00,0x7F,0x00,
0x7E,0x66,0x22,0xFD,0xAC,0x06,0x7F,0x02,
0x7E,0x66,0x12,0x05,0xAF,0xE4,0xFD,0xFC,
0x22,0x78,0x7F,0xE4,0xF6,0xD8,0xFD,0x75,
0x81,0x3C,0x02,0x04,0x88,0x02,0x00,0x0E,
0xE4,0x93,0xA3,0xF8,0xE4,0x93,0xA3,0x40,
0x03,0xF6,0x80,0x01,0xF2,0x08,0xDF,0xF4,
0x80,0x29,0xE4,0x93,0xA3,0xF8,0x54,0x07,
0x24,0x0C,0xC8,0xC3,0x33,0xC4,0x54,0x0F,
0x44,0x20,0xC8,0x83,0x40,0x04,0xF4,0x56,
0x80,0x01,0x46,0xF6,0xDF,0xE4,0x80,0x0B,
0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,
0x90,0x05,0xCB,0xE4,0x7E,0x01,0x93,0x60,
0xBC,0xA3,0xFF,0x54,0x3F,0x30,0xE5,0x09,
0x54,0x1F,0xFE,0xE4,0x93,0xA3,0x60,0x01,
0x0E,0xCF,0x54,0xC0,0x25,0xE0,0x60,0xA8,
0x40,0xB8,0xE4,0x93,0xA3,0xFA,0xE4,0x93,
0xA3,0xF8,0xE4,0x93,0xA3,0xC8,0xC5,0x82,
0xC8,0xCA,0xC5,0x83,0xCA,0xF0,0xA3,0xC8,
0xC5,0x82,0xC8,0xCA,0xC5,0x83,0xCA,0xDF,
0xE9,0xDE,0xE7,0x80,0xBE,0x75,0x0F,0x80,
0x75,0x0E,0x7E,0x75,0x0D,0xAA,0x75,0x0C,
0x83,0xE4,0xF5,0x10,0x75,0x0B,0xA0,0x75,
0x0A,0xAC,0x75,0x09,0xB9,0x75,0x08,0x03,
0x75,0x89,0x11,0x7B,0x60,0x7A,0x09,0xF9,
0xF8,0xAF,0x0B,0xAE,0x0A,0xAD,0x09,0xAC,
0x08,0x12,0x02,0xBB,0xAD,0x07,0xAC,0x06,
0xC3,0xE4,0x9D,0xFD,0xE4,0x9C,0xFC,0x78,
0x17,0xF6,0xAF,0x05,0xEF,0x08,0xF6,0x18,
0xE6,0xF5,0x8C,0x08,0xE6,0xF5,0x8A,0x74,
0x0D,0x2D,0xFD,0xE4,0x3C,0x18,0xF6,0xAF,
0x05,0xEF,0x08,0xF6,0x75,0x88,0x10,0x53,
0x8E,0xC7,0xD2,0xA9,0x22,0xC0,0xE0,0xC0,
0xF0,0xC0,0x83,0xC0,0x82,0xC0,0xD0,0x75,
0xD0,0x00,0xC0,0x00,0x78,0x17,0xE6,0xF5,
0x8C,0x78,0x18,0xE6,0xF5,0x8A,0x90,0x06,
0x2B,0xE4,0x75,0xF0,0x01,0x12,0x02,0x69,
0x90,0x06,0x2D,0xE4,0x75,0xF0,0x01,0x12,
0x02,0x69,0xD0,0x00,0xD0,0xD0,0xD0,0x82,
0xD0,0x83,0xD0,0xF0,0xD0,0xE0,0x32,0xC2,
0xAF,0xAD,0x07,0xAC,0x06,0x8C,0xA2,0x8D,
0xA3,0x75,0xA0,0x01,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xAE,
0xA1,0xBE,0x00,0xF0,0xAE,0xA6,0xAF,0xA7,
0xD2,0xAF,0x22,0x90,0x06,0x24,0x12,0x03,
0x5E,0xEF,0x24,0x01,0xFF,0xE4,0x3E,0xFE,
0xE4,0x3D,0xFD,0xE4,0x3C,0x22,0xE4,0x7F,
0x20,0x7E,0x4E,0xFD,0xFC,0x90,0x06,0x24,
0x12,0x03,0x6A,0xC3,0x02,0x03,0x4D,0xC2,
0xAF,0xAB,0x07,0xAA,0x06,0x8A,0xA2,0x8B,
0xA3,0x8C,0xA4,0x8D,0xA5,0x75,0xA0,0x03,
0x00,0x00,0x00,0xAA,0xA1,0xBA,0x00,0xF8,
0xD2,0xAF,0x22,0x42,0x06,0x2D,0x00,0x00,
0x42,0x06,0x2B,0x00,0x00,0x00,0x12,0x05,
0xDF,0x12,0x04,0xCD,0x02,0x00,0x03,0xE4,
0xF5,0x8E,0x22};

static rtk_api_ret_t _rtk_port_FiberModeAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      regData;

    /* Check Combo port or not */
    RTK_CHK_PORT_IS_COMBO(port);

    /* Flow Control */
    if ((retVal = rtl8367c_getAsicReg(RTL8367C_REG_FIB0_CFG04, &regData)) != RT_ERR_OK)
        return retVal;

    if (pAbility->AsyFC == 1)
        regData |= (0x0001 << 8);
    else
        regData &= ~(0x0001 << 8);

    if (pAbility->FC == 1)
        regData |= (0x0001 << 7);
    else
        regData &= ~(0x0001 << 7);

    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_FIB0_CFG04, regData)) != RT_ERR_OK)
        return retVal;

    /* Speed ability */
    if( (pAbility->Full_1000 == 1) && (pAbility->Full_100 == 1) && (pAbility->AutoNegotiation == 1) )
    {
        if ((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_FIBER_CFG_1, RTL8367C_SDS_FRC_MODE_OFFSET, 0)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FIBER_CFG_1, RTL8367C_SDS_MODE_MASK, 7)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_FIB0_CFG00, 0x1140)) != RT_ERR_OK)
            return retVal;
    }
    else if(pAbility->Full_1000 == 1)
    {
        if ((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_FIBER_CFG_1, RTL8367C_SDS_FRC_MODE_OFFSET, 1)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FIBER_CFG_1, RTL8367C_SDS_MODE_MASK, 4)) != RT_ERR_OK)
            return retVal;

        if(pAbility->AutoNegotiation == 1)
        {
            if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_FIB0_CFG00, 0x1140)) != RT_ERR_OK)
                return retVal;
        }
        else
        {
            if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_FIB0_CFG00, 0x0140)) != RT_ERR_OK)
                return retVal;
        }
    }
    else if(pAbility->Full_100 == 1)
    {
        if ((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_FIBER_CFG_1, RTL8367C_SDS_FRC_MODE_OFFSET, 1)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FIBER_CFG_1, RTL8367C_SDS_MODE_MASK, 5)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_FIB0_CFG00, 0x2100)) != RT_ERR_OK)
            return retVal;
    }

    /* Digital software reset */
    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_SDS_INDACS_ADR, 0x0003)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_SDS_INDACS_CMD, 0x0080)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_getAsicReg(RTL8367C_REG_SDS_INDACS_DATA, &regData)) != RT_ERR_OK)
        return retVal;

    regData |= (0x0001 << 6);

    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_SDS_INDACS_DATA, regData)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_SDS_INDACS_ADR, 0x0003)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_SDS_INDACS_CMD, 0x00C0)) != RT_ERR_OK)
        return retVal;

    regData &= ~(0x0001 << 6);

    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_SDS_INDACS_DATA, regData)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_SDS_INDACS_ADR, 0x0003)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_SDS_INDACS_CMD, 0x00C0)) != RT_ERR_OK)
        return retVal;

    /* CDR reset */
    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_SDS_INDACS_DATA, 0x1401))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_SDS_INDACS_ADR, 0x0000))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_SDS_INDACS_CMD, 0x00C0))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_SDS_INDACS_DATA, 0x1403))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_SDS_INDACS_ADR, 0x0000))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_SDS_INDACS_CMD, 0x00C0))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

static rtk_api_ret_t _rtk_port_FiberModeAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      data, regData;

    /* Check Combo port or not */
    RTK_CHK_PORT_IS_COMBO(port);

    memset(pAbility, 0x00, sizeof(rtk_port_phy_ability_t));

    /* Flow Control */
    if ((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_FIBER_CFG_1, RTL8367C_SDS_FRC_REG4_OFFSET, 1)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_FIBER_CFG_1, RTL8367C_SDS_FRC_REG4_FIB100_OFFSET, 0)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_SDS_INDACS_ADR, 0x0044)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_SDS_INDACS_CMD, 0x0080)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_getAsicReg(RTL8367C_REG_SDS_INDACS_DATA, &regData)) != RT_ERR_OK)
        return retVal;

    if(regData & (0x0001 << 8))
        pAbility->AsyFC = 1;

    if(regData & (0x0001 << 7))
        pAbility->FC = 1;

    /* Speed ability */
    if ((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_FIBER_CFG_1, RTL8367C_SDS_FRC_MODE_OFFSET, &data)) != RT_ERR_OK)
            return retVal;

    if(data == 0)
    {
        pAbility->AutoNegotiation = 1;
        pAbility->Full_1000 = 1;
        pAbility->Full_100 = 1;
    }
    else
    {
        if ((retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FIBER_CFG_1, RTL8367C_SDS_MODE_MASK, &data)) != RT_ERR_OK)
            return retVal;

        if(data == 4)
        {
            pAbility->Full_1000 = 1;

            if ((retVal = rtl8367c_getAsicReg(RTL8367C_REG_FIB0_CFG00, &data)) != RT_ERR_OK)
                return retVal;

            if(data & 0x1000)
                pAbility->AutoNegotiation = 1;
            else
                pAbility->AutoNegotiation = 0;
        }
        else if(data == 5)
            pAbility->Full_100 = 1;
        else
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_phyAutoNegoAbility_set
 * Description:
 *      Set Ethernet PHY auto-negotiation desired ability.
 * Input:
 *      port        - port id.
 *      pAbility    - Ability structure
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_PHY_REG_ID       - Invalid PHY address
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      If Full_1000 bit is set to 1, the AutoNegotiation will be automatic set to 1. While both AutoNegotiation and Full_1000 are set to 0, the PHY speed and duplex selection will
 *      be set as following 100F > 100H > 10F > 10H priority sequence.
 */
rtk_api_ret_t rtk_port_phyAutoNegoAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    rtk_api_ret_t       retVal;
    rtk_uint32          phyData;
    rtk_uint32          phyEnMsk0;
    rtk_uint32          phyEnMsk4;
    rtk_uint32          phyEnMsk9;
    rtk_port_media_t    media_type;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    if(NULL == pAbility)
        return RT_ERR_NULL_POINTER;

    if (pAbility->Half_10 >= RTK_ENABLE_END || pAbility->Full_10 >= RTK_ENABLE_END ||
       pAbility->Half_100 >= RTK_ENABLE_END || pAbility->Full_100 >= RTK_ENABLE_END ||
       pAbility->Full_1000 >= RTK_ENABLE_END || pAbility->AutoNegotiation >= RTK_ENABLE_END ||
       pAbility->AsyFC >= RTK_ENABLE_END || pAbility->FC >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if (rtk_switch_isComboPort(port) == RT_ERR_OK)
    {
        if ((retVal = rtk_port_phyComboPortMedia_get(port, &media_type)) != RT_ERR_OK)
            return retVal;

        if(media_type == PORT_MEDIA_FIBER)
        {
            return _rtk_port_FiberModeAbility_set(port, pAbility);
        }
    }

    /*for PHY auto mode setup*/
    pAbility->AutoNegotiation = 1;

    phyEnMsk0 = 0;
    phyEnMsk4 = 0;
    phyEnMsk9 = 0;

    if (1 == pAbility->Half_10)
    {
        /*10BASE-TX half duplex capable in reg 4.5*/
        phyEnMsk4 = phyEnMsk4 | (1 << 5);

        /*Speed selection [1:0] */
        /* 11=Reserved*/
        /* 10= 1000Mpbs*/
        /* 01= 100Mpbs*/
        /* 00= 10Mpbs*/
        phyEnMsk0 = phyEnMsk0 & (~(1 << 6));
        phyEnMsk0 = phyEnMsk0 & (~(1 << 13));
    }

    if (1 == pAbility->Full_10)
    {
        /*10BASE-TX full duplex capable in reg 4.6*/
        phyEnMsk4 = phyEnMsk4 | (1 << 6);
        /*Speed selection [1:0] */
        /* 11=Reserved*/
        /* 10= 1000Mpbs*/
        /* 01= 100Mpbs*/
        /* 00= 10Mpbs*/
        phyEnMsk0 = phyEnMsk0 & (~(1 << 6));
        phyEnMsk0 = phyEnMsk0 & (~(1 << 13));

        /*Full duplex mode in reg 0.8*/
        phyEnMsk0 = phyEnMsk0 | (1 << 8);

    }

    if (1 == pAbility->Half_100)
    {
        /*100BASE-TX half duplex capable in reg 4.7*/
        phyEnMsk4 = phyEnMsk4 | (1 << 7);
        /*Speed selection [1:0] */
        /* 11=Reserved*/
        /* 10= 1000Mpbs*/
        /* 01= 100Mpbs*/
        /* 00= 10Mpbs*/
        phyEnMsk0 = phyEnMsk0 & (~(1 << 6));
        phyEnMsk0 = phyEnMsk0 | (1 << 13);
    }


    if (1 == pAbility->Full_100)
    {
        /*100BASE-TX full duplex capable in reg 4.8*/
        phyEnMsk4 = phyEnMsk4 | (1 << 8);
        /*Speed selection [1:0] */
        /* 11=Reserved*/
        /* 10= 1000Mpbs*/
        /* 01= 100Mpbs*/
        /* 00= 10Mpbs*/
        phyEnMsk0 = phyEnMsk0 & (~(1 << 6));
        phyEnMsk0 = phyEnMsk0 | (1 << 13);
        /*Full duplex mode in reg 0.8*/
        phyEnMsk0 = phyEnMsk0 | (1 << 8);
    }


    if (1 == pAbility->Full_1000)
    {
        /*1000 BASE-T FULL duplex capable setting in reg 9.9*/
        phyEnMsk9 = phyEnMsk9 | (1 << 9);

        /*Speed selection [1:0] */
        /* 11=Reserved*/
        /* 10= 1000Mpbs*/
        /* 01= 100Mpbs*/
        /* 00= 10Mpbs*/
        phyEnMsk0 = phyEnMsk0 | (1 << 6);
        phyEnMsk0 = phyEnMsk0 & (~(1 << 13));


        /*Auto-Negotiation setting in reg 0.12*/
        phyEnMsk0 = phyEnMsk0 | (1 << 12);

     }

    if (1 == pAbility->AutoNegotiation)
    {
        /*Auto-Negotiation setting in reg 0.12*/
        phyEnMsk0 = phyEnMsk0 | (1 << 12);
    }

    if (1 == pAbility->AsyFC)
    {
        /*Asymetric flow control in reg 4.11*/
        phyEnMsk4 = phyEnMsk4 | (1 << 11);
    }
    if (1 == pAbility->FC)
    {
        /*Flow control in reg 4.10*/
        phyEnMsk4 = phyEnMsk4 | (1 << 10);
    }

    /*1000 BASE-T control register setting*/
    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_1000_BASET_CONTROL_REG, &phyData)) != RT_ERR_OK)
        return retVal;

    phyData = (phyData & (~0x0200)) | phyEnMsk9 ;

    if ((retVal = rtl8367c_setAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_1000_BASET_CONTROL_REG, phyData)) != RT_ERR_OK)
        return retVal;

    /*Auto-Negotiation control register setting*/
    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_AN_ADVERTISEMENT_REG, &phyData)) != RT_ERR_OK)
        return retVal;

    phyData = (phyData & (~0x0DE0)) | phyEnMsk4;
    if ((retVal = rtl8367c_setAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_AN_ADVERTISEMENT_REG, phyData)) != RT_ERR_OK)
        return retVal;

    /*Control register setting and restart auto*/
    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
        return retVal;

    phyData = (phyData & (~0x3140)) | phyEnMsk0;
    /*If have auto-negotiation capable, then restart auto negotiation*/
    if (1 == pAbility->AutoNegotiation)
    {
        phyData = phyData | (1 << 9);
    }

    if ((retVal = rtl8367c_setAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_phyAutoNegoAbility_get
 * Description:
 *      Get PHY ability through PHY registers.
 * Input:
 *      port - Port id.
 * Output:
 *      pAbility - Ability structure
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_PHY_REG_ID       - Invalid PHY address
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      Get the capability of specified PHY.
 */
rtk_api_ret_t rtk_port_phyAutoNegoAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    rtk_api_ret_t       retVal;
    rtk_uint32          phyData0;
    rtk_uint32          phyData4;
    rtk_uint32          phyData9;
    rtk_port_media_t    media_type;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    if(NULL == pAbility)
        return RT_ERR_NULL_POINTER;

    if (rtk_switch_isComboPort(port) == RT_ERR_OK)
    {
        if ((retVal = rtk_port_phyComboPortMedia_get(port, &media_type)) != RT_ERR_OK)
            return retVal;

        if(media_type == PORT_MEDIA_FIBER)
        {
            return _rtk_port_FiberModeAbility_get(port, pAbility);
        }
    }

    /*Control register setting and restart auto*/
    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return retVal;

    /*Auto-Negotiation control register setting*/
    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_AN_ADVERTISEMENT_REG, &phyData4)) != RT_ERR_OK)
        return retVal;

    /*1000 BASE-T control register setting*/
    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_1000_BASET_CONTROL_REG, &phyData9)) != RT_ERR_OK)
        return retVal;

    if (phyData9 & (1 << 9))
        pAbility->Full_1000 = 1;
    else
        pAbility->Full_1000 = 0;

    if (phyData4 & (1 << 11))
        pAbility->AsyFC = 1;
    else
        pAbility->AsyFC = 0;

    if (phyData4 & (1 << 10))
        pAbility->FC = 1;
    else
        pAbility->FC = 0;


    if (phyData4 & (1 << 8))
        pAbility->Full_100 = 1;
    else
        pAbility->Full_100 = 0;

    if (phyData4 & (1 << 7))
        pAbility->Half_100 = 1;
    else
        pAbility->Half_100 = 0;

    if (phyData4 & (1 << 6))
        pAbility->Full_10 = 1;
    else
        pAbility->Full_10 = 0;

    if (phyData4 & (1 << 5))
        pAbility->Half_10 = 1;
    else
        pAbility->Half_10 = 0;


    if (phyData0 & (1 << 12))
        pAbility->AutoNegotiation = 1;
    else
        pAbility->AutoNegotiation = 0;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_phyForceModeAbility_set
 * Description:
 *      Set the port speed/duplex mode/pause/asy_pause in the PHY force mode.
 * Input:
 *      port        - port id.
 *      pAbility    - Ability structure
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_PHY_REG_ID       - Invalid PHY address
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      While both AutoNegotiation and Full_1000 are set to 0, the PHY speed and duplex selection will
 *      be set as following 100F > 100H > 10F > 10H priority sequence.
 *      This API can be used to configure combo port in fiber mode.
 *      The possible parameters in fiber mode are Full_1000 and Full 100.
 *      All the other fields in rtk_port_phy_ability_t will be ignored in fiber port.
 */
rtk_api_ret_t rtk_port_phyForceModeAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
     rtk_api_ret_t      retVal;
     rtk_uint32         phyData;
     rtk_uint32         phyEnMsk0;
     rtk_uint32         phyEnMsk4;
     rtk_uint32         phyEnMsk9;
     rtk_port_media_t   media_type;

     /* Check initialization state */
     RTK_CHK_INIT_STATE();

     /* Check Port Valid */
     RTK_CHK_PORT_IS_UTP(port);

     if(NULL == pAbility)
        return RT_ERR_NULL_POINTER;

     if (pAbility->Half_10 >= RTK_ENABLE_END || pAbility->Full_10 >= RTK_ENABLE_END ||
        pAbility->Half_100 >= RTK_ENABLE_END || pAbility->Full_100 >= RTK_ENABLE_END ||
        pAbility->Full_1000 >= RTK_ENABLE_END || pAbility->AutoNegotiation >= RTK_ENABLE_END ||
        pAbility->AsyFC >= RTK_ENABLE_END || pAbility->FC >= RTK_ENABLE_END)
         return RT_ERR_INPUT;

     if (rtk_switch_isComboPort(port) == RT_ERR_OK)
     {
         if ((retVal = rtk_port_phyComboPortMedia_get(port, &media_type)) != RT_ERR_OK)
             return retVal;

         if(media_type == PORT_MEDIA_FIBER)
         {
             return _rtk_port_FiberModeAbility_set(port, pAbility);
         }
     }

     if (1 == pAbility->Full_1000)
         return RT_ERR_INPUT;

     /*for PHY force mode setup*/
     pAbility->AutoNegotiation = 0;

     phyEnMsk0 = 0;
     phyEnMsk4 = 0;
     phyEnMsk9 = 0;

     if (1 == pAbility->Half_10)
     {
         /*10BASE-TX half duplex capable in reg 4.5*/
         phyEnMsk4 = phyEnMsk4 | (1 << 5);

         /*Speed selection [1:0] */
         /* 11=Reserved*/
         /* 10= 1000Mpbs*/
         /* 01= 100Mpbs*/
         /* 00= 10Mpbs*/
         phyEnMsk0 = phyEnMsk0 & (~(1 << 6));
         phyEnMsk0 = phyEnMsk0 & (~(1 << 13));
     }

     if (1 == pAbility->Full_10)
     {
         /*10BASE-TX full duplex capable in reg 4.6*/
         phyEnMsk4 = phyEnMsk4 | (1 << 6);
         /*Speed selection [1:0] */
         /* 11=Reserved*/
         /* 10= 1000Mpbs*/
         /* 01= 100Mpbs*/
         /* 00= 10Mpbs*/
         phyEnMsk0 = phyEnMsk0 & (~(1 << 6));
         phyEnMsk0 = phyEnMsk0 & (~(1 << 13));

         /*Full duplex mode in reg 0.8*/
         phyEnMsk0 = phyEnMsk0 | (1 << 8);

     }

     if (1 == pAbility->Half_100)
     {
         /*100BASE-TX half duplex capable in reg 4.7*/
         phyEnMsk4 = phyEnMsk4 | (1 << 7);
         /*Speed selection [1:0] */
         /* 11=Reserved*/
         /* 10= 1000Mpbs*/
         /* 01= 100Mpbs*/
         /* 00= 10Mpbs*/
         phyEnMsk0 = phyEnMsk0 & (~(1 << 6));
         phyEnMsk0 = phyEnMsk0 | (1 << 13);
     }


     if (1 == pAbility->Full_100)
     {
         /*100BASE-TX full duplex capable in reg 4.8*/
         phyEnMsk4 = phyEnMsk4 | (1 << 8);
         /*Speed selection [1:0] */
         /* 11=Reserved*/
         /* 10= 1000Mpbs*/
         /* 01= 100Mpbs*/
         /* 00= 10Mpbs*/
         phyEnMsk0 = phyEnMsk0 & (~(1 << 6));
         phyEnMsk0 = phyEnMsk0 | (1 << 13);
         /*Full duplex mode in reg 0.8*/
         phyEnMsk0 = phyEnMsk0 | (1 << 8);
     }

     if (1 == pAbility->AsyFC)
     {
         /*Asymmetric flow control in reg 4.11*/
         phyEnMsk4 = phyEnMsk4 | (1 << 11);
     }
     if (1 == pAbility->FC)
     {
         /*Flow control in reg 4.10*/
         phyEnMsk4 = phyEnMsk4 | ((1 << 10));
     }

     /*1000 BASE-T control register setting*/
     if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_1000_BASET_CONTROL_REG, &phyData)) != RT_ERR_OK)
         return retVal;

     phyData = (phyData & (~0x0200)) | phyEnMsk9 ;

     if ((retVal = rtl8367c_setAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_1000_BASET_CONTROL_REG, phyData)) != RT_ERR_OK)
         return retVal;

     /*Auto-Negotiation control register setting*/
     if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_AN_ADVERTISEMENT_REG, &phyData)) != RT_ERR_OK)
         return retVal;

     phyData = (phyData & (~0x0DE0)) | phyEnMsk4;
     if ((retVal = rtl8367c_setAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_AN_ADVERTISEMENT_REG, phyData)) != RT_ERR_OK)
         return retVal;

     /*Control register setting and power off/on*/
     phyData = phyEnMsk0 & (~(1 << 12));
     phyData |= (1 << 11);   /* power down PHY, bit 11 should be set to 1 */
     if ((retVal = rtl8367c_setAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
         return retVal;

     phyData = phyData & (~(1 << 11));   /* power on PHY, bit 11 should be set to 0*/
     if ((retVal = rtl8367c_setAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
         return retVal;

     return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_phyForceModeAbility_get
 * Description:
 *      Get PHY ability through PHY registers.
 * Input:
 *      port - Port id.
 * Output:
 *      pAbility - Ability structure
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_PHY_REG_ID       - Invalid PHY address
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      Get the capability of specified PHY.
 */
rtk_api_ret_t rtk_port_phyForceModeAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    rtk_api_ret_t       retVal;
    rtk_uint32          phyData0;
    rtk_uint32          phyData4;
    rtk_uint32          phyData9;
    rtk_port_media_t    media_type;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
     RTK_CHK_PORT_IS_UTP(port);

     if(NULL == pAbility)
        return RT_ERR_NULL_POINTER;

     if (rtk_switch_isComboPort(port) == RT_ERR_OK)
     {
         if ((retVal = rtk_port_phyComboPortMedia_get(port, &media_type)) != RT_ERR_OK)
             return retVal;

         if(media_type == PORT_MEDIA_FIBER)
         {
             return _rtk_port_FiberModeAbility_get(port, pAbility);
         }
     }

    /*Control register setting and restart auto*/
    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return retVal;

    /*Auto-Negotiation control register setting*/
    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_AN_ADVERTISEMENT_REG, &phyData4)) != RT_ERR_OK)
        return retVal;

    /*1000 BASE-T control register setting*/
    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_1000_BASET_CONTROL_REG, &phyData9)) != RT_ERR_OK)
        return retVal;

    if (phyData9 & (1 << 9))
        pAbility->Full_1000 = 1;
    else
        pAbility->Full_1000 = 0;

    if (phyData4 & (1 << 11))
        pAbility->AsyFC = 1;
    else
        pAbility->AsyFC = 0;

    if (phyData4 & ((1 << 10)))
        pAbility->FC = 1;
    else
        pAbility->FC = 0;


    if (phyData4 & (1 << 8))
        pAbility->Full_100 = 1;
    else
        pAbility->Full_100 = 0;

    if (phyData4 & (1 << 7))
        pAbility->Half_100 = 1;
    else
        pAbility->Half_100 = 0;

    if (phyData4 & (1 << 6))
        pAbility->Full_10 = 1;
    else
        pAbility->Full_10 = 0;

    if (phyData4 & (1 << 5))
        pAbility->Half_10 = 1;
    else
        pAbility->Half_10 = 0;


    if (phyData0 & (1 << 12))
        pAbility->AutoNegotiation = 1;
    else
        pAbility->AutoNegotiation = 0;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_phyStatus_get
 * Description:
 *      Get Ethernet PHY linking status
 * Input:
 *      port - Port id.
 * Output:
 *      linkStatus  - PHY link status
 *      speed       - PHY link speed
 *      duplex      - PHY duplex mode
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_PHY_REG_ID       - Invalid PHY address
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      API will return auto negotiation status of phy.
 */
rtk_api_ret_t rtk_port_phyStatus_get(rtk_port_t port, rtk_port_linkStatus_t *pLinkStatus, rtk_port_speed_t *pSpeed, rtk_port_duplex_t *pDuplex)
{
    rtk_api_ret_t retVal;
    rtk_uint32 phyData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    if( (NULL == pLinkStatus) || (NULL == pSpeed) || (NULL == pDuplex) )
        return RT_ERR_NULL_POINTER;

    /*Get PHY resolved register*/
    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_RESOLVED_REG, &phyData)) != RT_ERR_OK)
        return retVal;

    /*check link status*/
    if (phyData & (1<<2))
    {
        *pLinkStatus = 1;

        /*check link speed*/
        *pSpeed = (phyData&0x0030) >> 4;

        /*check link duplex*/
        *pDuplex = (phyData&0x0008) >> 3;
    }
    else
    {
        *pLinkStatus = 0;
        *pSpeed = 0;
        *pDuplex = 0;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_macForceLink_set
 * Description:
 *      Set port force linking configuration.
 * Input:
 *      port            - port id.
 *      pPortability    - port ability configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can set Port/MAC force mode properties.
 */
rtk_api_ret_t rtk_port_macForceLink_set(rtk_port_t port, rtk_port_mac_ability_t *pPortability)
{
    rtk_api_ret_t retVal;
    rtl8367c_port_ability_t ability;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    if(NULL == pPortability)
        return RT_ERR_NULL_POINTER;

    if (pPortability->forcemode >1|| pPortability->speed > 2 || pPortability->duplex > 1 ||
       pPortability->link > 1 || pPortability->nway > 1 || pPortability->txpause > 1 || pPortability->rxpause > 1)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_getAsicPortForceLink(rtk_switch_port_L2P_get(port), &ability)) != RT_ERR_OK)
        return retVal;

    ability.forcemode = pPortability->forcemode;
    ability.speed     = pPortability->speed;
    ability.duplex    = pPortability->duplex;
    ability.link      = pPortability->link;
    ability.nway      = pPortability->nway;
    ability.txpause   = pPortability->txpause;
    ability.rxpause   = pPortability->rxpause;

    if ((retVal = rtl8367c_setAsicPortForceLink(rtk_switch_port_L2P_get(port), &ability)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_macForceLink_get
 * Description:
 *      Get port force linking configuration.
 * Input:
 *      port - Port id.
 * Output:
 *      pPortability - port ability configuration
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API can get Port/MAC force mode properties.
 */
rtk_api_ret_t rtk_port_macForceLink_get(rtk_port_t port, rtk_port_mac_ability_t *pPortability)
{
    rtk_api_ret_t retVal;
    rtl8367c_port_ability_t ability;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    if(NULL == pPortability)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicPortForceLink(rtk_switch_port_L2P_get(port), &ability)) != RT_ERR_OK)
        return retVal;

    pPortability->forcemode = ability.forcemode;
    pPortability->speed     = ability.speed;
    pPortability->duplex    = ability.duplex;
    pPortability->link      = ability.link;
    pPortability->nway      = ability.nway;
    pPortability->txpause   = ability.txpause;
    pPortability->rxpause   = ability.rxpause;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_macForceLinkExt_set
 * Description:
 *      Set external interface force linking configuration.
 * Input:
 *      port            - external port ID
 *      mode            - external interface mode
 *      pPortability    - port ability configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API can set external interface force mode properties.
 *      The external interface can be set to:
 *      - MODE_EXT_DISABLE,
 *      - MODE_EXT_RGMII,
 *      - MODE_EXT_MII_MAC,
 *      - MODE_EXT_MII_PHY,
 *      - MODE_EXT_TMII_MAC,
 *      - MODE_EXT_TMII_PHY,
 *      - MODE_EXT_GMII,
 *      - MODE_EXT_RMII_MAC,
 *      - MODE_EXT_RMII_PHY,
 *      - MODE_EXT_SGMII,
 *      - MODE_EXT_HSGMII,
 *      - MODE_EXT_1000X_100FX,
 *      - MODE_EXT_1000X,
 *      - MODE_EXT_100FX,
 */
rtk_api_ret_t rtk_port_macForceLinkExt_set(rtk_port_t port, rtk_mode_ext_t mode, rtk_port_mac_ability_t *pPortability)
{
    rtk_api_ret_t retVal;
    rtl8367c_port_ability_t ability;
    rtk_uint32 ext_id;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_EXT(port);

    if(NULL == pPortability)
        return RT_ERR_NULL_POINTER;

    if (mode >=MODE_EXT_END)
        return RT_ERR_INPUT;

    if(mode == MODE_EXT_HSGMII)
    {
        if (pPortability->forcemode > 1 || pPortability->speed != PORT_SPEED_2500M || pPortability->duplex != PORT_FULL_DUPLEX ||
           pPortability->link >= PORT_LINKSTATUS_END || pPortability->nway > 1 || pPortability->txpause > 1 || pPortability->rxpause > 1)
            return RT_ERR_INPUT;

        if(rtk_switch_isHsgPort(port) != RT_ERR_OK)
            return RT_ERR_PORT_ID;
    }
    else
    {
        if (pPortability->forcemode > 1 || pPortability->speed > PORT_SPEED_1000M || pPortability->duplex >= PORT_DUPLEX_END ||
           pPortability->link >= PORT_LINKSTATUS_END || pPortability->nway > 1 || pPortability->txpause > 1 || pPortability->rxpause > 1)
            return RT_ERR_INPUT;
    }

    ext_id = port - 15;

    if(mode == MODE_EXT_DISABLE)
    {
        memset(&ability, 0x00, sizeof(rtl8367c_port_ability_t));
        if ((retVal = rtl8367c_setAsicPortForceLinkExt(ext_id, &ability)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicPortExtMode(ext_id, mode)) != RT_ERR_OK)
            return retVal;
    }
    else
    {
        if ((retVal = rtl8367c_setAsicPortExtMode(ext_id, mode)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_getAsicPortForceLinkExt(ext_id, &ability)) != RT_ERR_OK)
            return retVal;

        ability.forcemode = pPortability->forcemode;
        ability.speed     = (mode == MODE_EXT_HSGMII) ? PORT_SPEED_1000M : pPortability->speed;
        ability.duplex    = pPortability->duplex;
        ability.link      = pPortability->link;
        ability.nway      = pPortability->nway;
        ability.txpause   = pPortability->txpause;
        ability.rxpause   = pPortability->rxpause;

        if ((retVal = rtl8367c_setAsicPortForceLinkExt(ext_id, &ability)) != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_macForceLinkExt_get
 * Description:
 *      Set external interface force linking configuration.
 * Input:
 *      port            - external port ID
 * Output:
 *      pMode           - external interface mode
 *      pPortability    - port ability configuration
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API can get external interface force mode properties.
 */
rtk_api_ret_t rtk_port_macForceLinkExt_get(rtk_port_t port, rtk_mode_ext_t *pMode, rtk_port_mac_ability_t *pPortability)
{
    rtk_api_ret_t retVal;
    rtl8367c_port_ability_t ability;
    rtk_uint32 ext_id;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_EXT(port);

    if(NULL == pMode)
        return RT_ERR_NULL_POINTER;

    if(NULL == pPortability)
        return RT_ERR_NULL_POINTER;

    ext_id = port - 15;

    if ((retVal = rtl8367c_getAsicPortExtMode(ext_id, (rtk_uint32 *)pMode)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_getAsicPortForceLinkExt(ext_id, &ability)) != RT_ERR_OK)
        return retVal;

    pPortability->forcemode = ability.forcemode;
    pPortability->speed     = (*pMode == MODE_EXT_HSGMII) ? PORT_SPEED_2500M : ability.speed;
    pPortability->duplex    = ability.duplex;
    pPortability->link      = ability.link;
    pPortability->nway      = ability.nway;
    pPortability->txpause   = ability.txpause;
    pPortability->rxpause   = ability.rxpause;

    return RT_ERR_OK;

}

/* Function Name:
 *      rtk_port_macStatus_get
 * Description:
 *      Get port link status.
 * Input:
 *      port - Port id.
 * Output:
 *      pPortstatus - port ability configuration
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can get Port/PHY properties.
 */
rtk_api_ret_t rtk_port_macStatus_get(rtk_port_t port, rtk_port_mac_ability_t *pPortstatus)
{
    rtk_api_ret_t retVal;
    rtl8367c_port_status_t status;
    rtk_uint32 hsgsel;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pPortstatus)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicPortStatus(rtk_switch_port_L2P_get(port), &status)) != RT_ERR_OK)
        return retVal;


    pPortstatus->duplex    = status.duplex;
    pPortstatus->link      = status.link;
    pPortstatus->nway      = status.nway;
    pPortstatus->txpause   = status.txpause;
    pPortstatus->rxpause   = status.rxpause;

    if( (retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_SDS_MISC, RTL8367C_CFG_MAC8_SEL_HSGMII_OFFSET, &hsgsel)) != RT_ERR_OK)
            return retVal;

    if( (rtk_switch_isHsgPort(port) == RT_ERR_OK) && (hsgsel == 1) )
        pPortstatus->speed = PORT_SPEED_2500M;
    else
        pPortstatus->speed = status.speed;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_macLocalLoopbackEnable_set
 * Description:
 *      Set Port Local Loopback. (Redirect TX to RX.)
 * Input:
 *      port    - Port id.
 *      enable  - Loopback state, 0:disable, 1:enable
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can enable/disable Local loopback in MAC.
 *      For UTP port, This API will also enable the digital
 *      loopback bit in PHY register for sync of speed between
 *      PHY and MAC. For EXT port, users need to force the
 *      link state by themselves.
 */
rtk_api_ret_t rtk_port_macLocalLoopbackEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      data;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(enable >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicPortLoopback(rtk_switch_port_L2P_get(port), enable)) != RT_ERR_OK)
        return retVal;

    if(rtk_switch_isUtpPort(port) == RT_ERR_OK)
    {
        if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_CONTROL_REG, &data)) != RT_ERR_OK)
            return retVal;

        if(enable == ENABLED)
            data |= (0x0001 << 14);
        else
            data &= ~(0x0001 << 14);

        if ((retVal = rtl8367c_setAsicPHYReg(rtk_switch_port_L2P_get(port), PHY_CONTROL_REG, data)) != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_macLocalLoopbackEnable_get
 * Description:
 *      Get Port Local Loopback. (Redirect TX to RX.)
 * Input:
 *      port    - Port id.
 * Output:
 *      pEnable  - Loopback state, 0:disable, 1:enable
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      None.
 */
rtk_api_ret_t rtk_port_macLocalLoopbackEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicPortLoopback(rtk_switch_port_L2P_get(port), pEnable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_phyReg_set
 * Description:
 *      Set PHY register data of the specific port.
 * Input:
 *      port    - port id.
 *      reg     - Register id
 *      regData - Register data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_PHY_REG_ID       - Invalid PHY address
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      This API can set PHY register data of the specific port.
 */
rtk_api_ret_t rtk_port_phyReg_set(rtk_port_t port, rtk_port_phy_reg_t reg, rtk_port_phy_data_t regData)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    if ((retVal = rtl8367c_setAsicPHYReg(rtk_switch_port_L2P_get(port), reg, regData)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_phyReg_get
 * Description:
 *      Get PHY register data of the specific port.
 * Input:
 *      port    - Port id.
 *      reg     - Register id
 * Output:
 *      pData   - Register data
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_PHY_REG_ID       - Invalid PHY address
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      This API can get PHY register data of the specific port.
 */
rtk_api_ret_t rtk_port_phyReg_get(rtk_port_t port, rtk_port_phy_reg_t reg, rtk_port_phy_data_t *pData)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(port), reg, pData)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_backpressureEnable_set
 * Description:
 *      Set the half duplex back-pressure enable status of the specific port.
 * Input:
 *      port    - port id.
 *      enable  - Back pressure status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can set the half duplex back-pressure enable status of the specific port.
 *      The half duplex back-pressure enable status of the port is as following:
 *      - DISABLE(Defer)
 *      - ENABLE (Back-pressure)
 */
rtk_api_ret_t rtk_port_backpressureEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (port != RTK_WHOLE_SYSTEM)
        return RT_ERR_PORT_ID;

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicPortJamMode(!enable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_backpressureEnable_get
 * Description:
 *      Get the half duplex back-pressure enable status of the specific port.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - Back pressure status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can get the half duplex back-pressure enable status of the specific port.
 *      The half duplex back-pressure enable status of the port is as following:
 *      - DISABLE(Defer)
 *      - ENABLE (Back-pressure)
 */
rtk_api_ret_t rtk_port_backpressureEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (port != RTK_WHOLE_SYSTEM)
        return RT_ERR_PORT_ID;

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicPortJamMode(&regData)) != RT_ERR_OK)
        return retVal;

    *pEnable = !regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_adminEnable_set
 * Description:
 *      Set port admin configuration of the specific port.
 * Input:
 *      port    - port id.
 *      enable  - Back pressure status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can set port admin configuration of the specific port.
 *      The port admin configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtk_api_ret_t rtk_port_adminEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    rtk_uint32      data;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtk_port_phyReg_get(port, PHY_CONTROL_REG, &data)) != RT_ERR_OK)
        return retVal;

    if (ENABLED == enable)
    {
        data &= 0xF7FF;
        data |= 0x0200;
    }
    else if (DISABLED == enable)
    {
        data |= 0x0800;
    }

    if ((retVal = rtk_port_phyReg_set(port, PHY_CONTROL_REG, data)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_adminEnable_get
 * Description:
 *      Get port admin configuration of the specific port.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - Back pressure status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can get port admin configuration of the specific port.
 *      The port admin configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtk_api_ret_t rtk_port_adminEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    rtk_uint32      data;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtk_port_phyReg_get(port, PHY_CONTROL_REG, &data)) != RT_ERR_OK)
        return retVal;

    if ( (data & 0x0800) == 0x0800)
    {
        *pEnable = DISABLED;
    }
    else
    {
        *pEnable = ENABLED;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_isolation_set
 * Description:
 *      Set permitted port isolation portmask
 * Input:
 *      port         - port id.
 *      pPortmask    - Permit port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_PORT_MASK    - Invalid portmask.
 * Note:
 *      This API set the port mask that a port can transmit packet to of each port
 *      A port can only transmit packet to ports included in permitted portmask
 */
rtk_api_ret_t rtk_port_isolation_set(rtk_port_t port, rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    /* check port mask */
    RTK_CHK_PORTMASK_VALID(pPortmask);

    if ((retVal = rtk_switch_portmask_L2P_get(pPortmask, &pmask)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicPortIsolationPermittedPortmask(rtk_switch_port_L2P_get(port), pmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_isolation_get
 * Description:
 *      Get permitted port isolation portmask
 * Input:
 *      port - Port id.
 * Output:
 *      pPortmask - Permit port mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API get the port mask that a port can transmit packet to of each port
 *      A port can only transmit packet to ports included in permitted portmask
 */
rtk_api_ret_t rtk_port_isolation_get(rtk_port_t port, rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicPortIsolationPermittedPortmask(rtk_switch_port_L2P_get(port), &pmask)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtk_switch_portmask_P2L_get(pmask, pPortmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_rgmiiDelayExt_set
 * Description:
 *      Set RGMII interface delay value for TX and RX.
 * Input:
 *      txDelay - TX delay value, 1 for delay 2ns and 0 for no-delay
 *      rxDelay - RX delay value, 0~7 for delay setup.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API can set external interface 2 RGMII delay.
 *      In TX delay, there are 2 selection: no-delay and 2ns delay.
 *      In RX delay, there are 8 steps for delay tuning. 0 for no-delay, and 7 for maximum delay.
 */
rtk_api_ret_t rtk_port_rgmiiDelayExt_set(rtk_port_t port, rtk_data_t txDelay, rtk_data_t rxDelay)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regAddr, regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_EXT(port);

    if ((txDelay > 1) || (rxDelay > 7))
        return RT_ERR_INPUT;

    if(port == EXT_PORT0)
        regAddr = RTL8367C_REG_EXT1_RGMXF;
    else if(port == EXT_PORT1)
        regAddr = RTL8367C_REG_EXT2_RGMXF;
    else
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_getAsicReg(regAddr, &regData)) != RT_ERR_OK)
        return retVal;

    regData = (regData & 0xFFF0) | ((txDelay << 3) & 0x0008) | (rxDelay & 0x0007);

    if ((retVal = rtl8367c_setAsicReg(regAddr, regData)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_rgmiiDelayExt_get
 * Description:
 *      Get RGMII interface delay value for TX and RX.
 * Input:
 *      None
 * Output:
 *      pTxDelay - TX delay value
 *      pRxDelay - RX delay value
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API can set external interface 2 RGMII delay.
 *      In TX delay, there are 2 selection: no-delay and 2ns delay.
 *      In RX delay, there are 8 steps for delay tuning. 0 for n0-delay, and 7 for maximum delay.
 */
rtk_api_ret_t rtk_port_rgmiiDelayExt_get(rtk_port_t port, rtk_data_t *pTxDelay, rtk_data_t *pRxDelay)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regAddr, regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_EXT(port);

    if( (NULL == pTxDelay) || (NULL == pRxDelay) )
        return RT_ERR_NULL_POINTER;

    if(port == EXT_PORT0)
        regAddr = RTL8367C_REG_EXT1_RGMXF;
    else if(port == EXT_PORT1)
        regAddr = RTL8367C_REG_EXT2_RGMXF;
    else
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_getAsicReg(regAddr, &regData)) != RT_ERR_OK)
        return retVal;

    *pTxDelay = (regData & 0x0008) >> 3;
    *pRxDelay = regData & 0x0007;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_phyEnableAll_set
 * Description:
 *      Set all PHY enable status.
 * Input:
 *      enable - PHY Enable State.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can set all PHY status.
 *      The configuration of all PHY is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtk_api_ret_t rtk_port_phyEnableAll_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 data;
    rtk_uint32 port;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8367c_setAsicPortEnableAll(enable)) != RT_ERR_OK)
        return retVal;

    RTK_SCAN_ALL_LOG_PORT(port)
    {
        if(rtk_switch_isUtpPort(port) == RT_ERR_OK)
        {
            if ((retVal = rtk_port_phyReg_get(port, PHY_CONTROL_REG, &data)) != RT_ERR_OK)
                return retVal;

            if (ENABLED == enable)
            {
                data &= 0xF7FF;
                data |= 0x0200;
            }
            else
            {
                data |= 0x0800;
            }

            if ((retVal = rtk_port_phyReg_set(port, PHY_CONTROL_REG, data)) != RT_ERR_OK)
                return retVal;
        }
    }

    return RT_ERR_OK;

}

/* Function Name:
 *      rtk_port_phyEnableAll_get
 * Description:
 *      Get all PHY enable status.
 * Input:
 *      None
 * Output:
 *      pEnable - PHY Enable State.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      This API can set all PHY status.
 *      The configuration of all PHY is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtk_api_ret_t rtk_port_phyEnableAll_get(rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicPortEnableAll(pEnable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_efid_set
 * Description:
 *      Set port-based enhanced filtering database
 * Input:
 *      port - Port id.
 *      efid - Specified enhanced filtering database.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_L2_FID - Invalid fid.
 *      RT_ERR_INPUT - Invalid input parameter.
 *      RT_ERR_PORT_ID - Invalid port ID.
 * Note:
 *      The API can set port-based enhanced filtering database.
 */
rtk_api_ret_t rtk_port_efid_set(rtk_port_t port, rtk_data_t efid)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    /* efid must be 0~7 */
    if (efid > RTK_EFID_MAX)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicPortIsolationEfid(rtk_switch_port_L2P_get(port), efid))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_efid_get
 * Description:
 *      Get port-based enhanced filtering database
 * Input:
 *      port - Port id.
 * Output:
 *      pEfid - Specified enhanced filtering database.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port ID.
 * Note:
 *      The API can get port-based enhanced filtering database status.
 */
rtk_api_ret_t rtk_port_efid_get(rtk_port_t port, rtk_data_t *pEfid)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pEfid)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicPortIsolationEfid(rtk_switch_port_L2P_get(port), pEfid))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_phyComboPortMedia_set
 * Description:
 *      Set Combo port media type
 * Input:
 *      port    - Port id.
 *      media   - Media (COPPER or FIBER)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_PORT_ID          - Invalid port ID.
 * Note:
 *      The API can Set Combo port media type.
 */
rtk_api_ret_t rtk_port_phyComboPortMedia_set(rtk_port_t port, rtk_port_media_t media)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;
    rtk_uint32 idx;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    /* Check Combo Port ID */
    RTK_CHK_PORT_IS_COMBO(port);

    if (media >= PORT_MEDIA_END)
        return RT_ERR_INPUT;

    if((retVal = rtl8367c_setAsicReg(0x13C2, 0x0249)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_getAsicReg(0x1300, &regData)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicReg(0x13C2, 0x0000)) != RT_ERR_OK)
        return retVal;

    if(regData != 0x6367)
        return RT_ERR_CHIP_NOT_SUPPORTED;

    if(media == PORT_MEDIA_FIBER)
    {
        /* software init */
        if ((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_CHIP_RESET, RTL8367C_DW8051_RST_OFFSET, 1)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_MISCELLANEOUS_CONFIGURE0, RTL8367C_DW8051_EN_OFFSET, 1)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_DW8051_RDY, RTL8367C_ACS_IROM_ENABLE_OFFSET, 1)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_DW8051_RDY, RTL8367C_IROM_MSB_OFFSET, 0)) != RT_ERR_OK)
            return retVal;

        for(idx = 0; idx < FIBER_INIT_SIZE; idx++)
        {
            if ((retVal = rtl8367c_setAsicReg(0xE000 + idx, (rtk_uint32)Fiber[idx])) != RT_ERR_OK)
                return retVal;
        }

        if ((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_DW8051_RDY, RTL8367C_IROM_MSB_OFFSET, 0)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_DW8051_RDY, RTL8367C_ACS_IROM_ENABLE_OFFSET, 0)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_CHIP_RESET, RTL8367C_DW8051_RST_OFFSET, 0)) != RT_ERR_OK)
            return retVal;
    }
    else
    {
        if ((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_UTP_FIB_DET, RTL8367C_UTP_FIRST_OFFSET, 1))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_DW8051_RDY, RTL8367C_DW8051_READY_OFFSET, 0)) != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_phyComboPortMedia_get
 * Description:
 *      Get Combo port media type
 * Input:
 *      port    - Port id.
 * Output:
 *      pMedia  - Media (COPPER or FIBER)
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_PORT_ID          - Invalid port ID.
 * Note:
 *      The API can Set Combo port media type.
 */
rtk_api_ret_t rtk_port_phyComboPortMedia_get(rtk_port_t port, rtk_port_media_t *pMedia)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      regData;
    rtk_uint32      data;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    /* Check Combo Port ID */
    RTK_CHK_PORT_IS_COMBO(port);

    if((retVal = rtl8367c_setAsicReg(0x13C2, 0x0249)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_getAsicReg(0x1300, &regData)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicReg(0x13C2, 0x0000)) != RT_ERR_OK)
        return retVal;

    if(regData != 0x6367)
    {
        *pMedia = PORT_MEDIA_COPPER;
    }
    else
    {
        if ((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_UTP_FIB_DET, RTL8367C_UTP_FIRST_OFFSET, &data))!=RT_ERR_OK)
                return retVal;

        if(data == 1)
            *pMedia = PORT_MEDIA_COPPER;
        else
            *pMedia = PORT_MEDIA_FIBER;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_rtctEnable_set
 * Description:
 *      Enable RTCT test
 * Input:
 *      pPortmask    - Port mask of RTCT enabled port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_MASK        - Invalid port mask.
 * Note:
 *      The API can enable RTCT Test
 */
rtk_api_ret_t rtk_port_rtctEnable_set(rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Mask Valid */
    RTK_CHK_PORTMASK_VALID_ONLY_UTP(pPortmask);

    if ((retVal = rtl8367c_setAsicPortRTCTEnable(pPortmask->bits[0]))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_rtctDisable_set
 * Description:
 *      Disable RTCT test
 * Input:
 *      pPortmask    - Port mask of RTCT disabled port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_MASK        - Invalid port mask.
 * Note:
 *      The API can disable RTCT Test
 */
rtk_api_ret_t rtk_port_rtctDisable_set(rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Mask Valid */
    RTK_CHK_PORTMASK_VALID_ONLY_UTP(pPortmask);

    if ((retVal = rtl8367c_setAsicPortRTCTDisable(pPortmask->bits[0]))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_port_rtctResult_get
 * Description:
 *      Get the result of RTCT test
 * Input:
 *      port        - Port ID
 * Output:
 *      pRtctResult - The result of RTCT result
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port ID.
 *      RT_ERR_PHY_RTCT_NOT_FINISH  - Testing does not finish.
 * Note:
 *      The API can get RTCT test result.
 *      RTCT test may takes 4.8 seconds to finish its test at most.
 *      Thus, if this API return RT_ERR_PHY_RTCT_NOT_FINISH or
 *      other error code, the result can not be referenced and
 *      user should call this API again until this API returns
 *      a RT_ERR_OK.
 *      The result is stored at pRtctResult->ge_result
 *      pRtctResult->linkType is unused.
 *      The unit of channel length is 2.5cm. Ex. 300 means 300 * 2.5 = 750cm = 7.5M
 */
rtk_api_ret_t rtk_port_rtctResult_get(rtk_port_t port, rtk_rtctResult_t *pRtctResult)
{
    rtk_api_ret_t               retVal;
    rtl8367c_port_rtct_result_t result;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_UTP(port);

    memset(pRtctResult, 0x00, sizeof(rtk_rtctResult_t));
    if ((retVal = rtl8367c_getAsicPortRTCTResult(port, &result))!=RT_ERR_OK)
        return retVal;

    pRtctResult->result.ge_result.channelALen = result.channelALen;
    pRtctResult->result.ge_result.channelBLen = result.channelBLen;
    pRtctResult->result.ge_result.channelCLen = result.channelCLen;
    pRtctResult->result.ge_result.channelDLen = result.channelDLen;

    pRtctResult->result.ge_result.channelALinedriver = result.channelALinedriver;
    pRtctResult->result.ge_result.channelBLinedriver = result.channelBLinedriver;
    pRtctResult->result.ge_result.channelCLinedriver = result.channelCLinedriver;
    pRtctResult->result.ge_result.channelDLinedriver = result.channelDLinedriver;

    pRtctResult->result.ge_result.channelAMismatch = result.channelAMismatch;
    pRtctResult->result.ge_result.channelBMismatch = result.channelBMismatch;
    pRtctResult->result.ge_result.channelCMismatch = result.channelCMismatch;
    pRtctResult->result.ge_result.channelDMismatch = result.channelDMismatch;

    pRtctResult->result.ge_result.channelAOpen = result.channelAOpen;
    pRtctResult->result.ge_result.channelBOpen = result.channelBOpen;
    pRtctResult->result.ge_result.channelCOpen = result.channelCOpen;
    pRtctResult->result.ge_result.channelDOpen = result.channelDOpen;

    pRtctResult->result.ge_result.channelAShort = result.channelAShort;
    pRtctResult->result.ge_result.channelBShort = result.channelBShort;
    pRtctResult->result.ge_result.channelCShort = result.channelCShort;
    pRtctResult->result.ge_result.channelDShort = result.channelDShort;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_sds_reset
 * Description:
 *      Reset Serdes
 * Input:
 *      port        - Port ID
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port ID.
 * Note:
 *      The API can reset Serdes
 */
rtk_api_ret_t rtk_port_sds_reset(rtk_port_t port)
{
    rtk_uint32 ext_id;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    if(rtk_switch_isSgmiiPort(port) != RT_ERR_OK)
        return RT_ERR_PORT_ID;

    ext_id = port - 15;
    return rtl8367c_sdsReset(ext_id);
}

/* Function Name:
 *      rtk_port_sgmiiLinkStatus_get
 * Description:
 *      Get SGMII status
 * Input:
 *      port        - Port ID
 * Output:
 *      pSignalDetect   - Signal detect
 *      pSync           - Sync
 *      pLink           - Link
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port ID.
 * Note:
 *      The API can reset Serdes
 */
rtk_api_ret_t rtk_port_sgmiiLinkStatus_get(rtk_port_t port, rtk_data_t *pSignalDetect, rtk_data_t *pSync, rtk_port_linkStatus_t *pLink)
{
    rtk_uint32 ext_id;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    if(rtk_switch_isSgmiiPort(port) != RT_ERR_OK)
        return RT_ERR_PORT_ID;

    if(NULL == pSignalDetect)
        return RT_ERR_NULL_POINTER;

    if(NULL == pSync)
        return RT_ERR_NULL_POINTER;

    if(NULL == pLink)
        return RT_ERR_NULL_POINTER;

    ext_id = port - 15;
    return rtl8367c_getSdsLinkStatus(ext_id, (rtk_uint32 *)pSignalDetect, (rtk_uint32 *)pSync, (rtk_uint32 *)pLink);
}

/* Function Name:
 *      rtk_port_sgmiiNway_set
 * Description:
 *      Configure SGMII/HSGMII port Nway state
 * Input:
 *      port        - Port ID
 *      state       - Nway state
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port ID.
 * Note:
 *      The API configure SGMII/HSGMII port Nway state
 */
rtk_api_ret_t rtk_port_sgmiiNway_set(rtk_port_t port, rtk_enable_t state)
{
    rtk_uint32 ext_id;

     /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    if(rtk_switch_isSgmiiPort(port) != RT_ERR_OK)
        return RT_ERR_PORT_ID;

    if(state >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    ext_id = port - 15;
    return rtl8367c_setSgmiiNway(ext_id, (rtk_uint32)state);
}

/* Function Name:
 *      rtk_port_sgmiiNway_get
 * Description:
 *      Get SGMII/HSGMII port Nway state
 * Input:
 *      port        - Port ID
 * Output:
 *      pState      - Nway state
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port ID.
 * Note:
 *      The API can get SGMII/HSGMII port Nway state
 */
rtk_api_ret_t rtk_port_sgmiiNway_get(rtk_port_t port, rtk_enable_t *pState)
{
    rtk_uint32 ext_id;

     /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    if(rtk_switch_isSgmiiPort(port) != RT_ERR_OK)
        return RT_ERR_PORT_ID;

    if(NULL == pState)
        return RT_ERR_NULL_POINTER;

    ext_id = port - 15;
    return rtl8367c_getSgmiiNway(ext_id, (rtk_uint32 *)pState);
}
