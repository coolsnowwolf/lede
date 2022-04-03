/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 38651 $
 * $Date: 2016-02-27 14:32:56 +0800 (йPдT, 17 е|ды 2016) $
 *
 * Purpose : RTL8367C switch high-level API for RTL8367C
 * Feature : I2C related functions
 *
 */


#ifndef _RTL8367C_ASICDRV_I2C_H_
#define _RTL8367C_ASICDRV_I2C_H_
#include <rtk_types.h>
#include <rtl8367c_asicdrv.h>


#define TIMEROUT_FOR_MICROSEMI (0x400)

#define GPIO_INPUT 1
#define GPIO_OUTPUT 2

extern ret_t rtl8367c_setAsicI2C_checkBusIdle(void);
extern ret_t rtl8367c_setAsicI2CStartCmd(void);
extern ret_t rtl8367c_setAsicI2CStopCmd(void);
extern ret_t rtl8367c_setAsicI2CTxOneCharCmd(rtk_uint8 oneChar);
extern ret_t rtl8367c_setAsicI2CcheckRxAck(void);
extern ret_t rtl8367c_setAsicI2CRxOneCharCmd(rtk_uint8 *pValue);
extern ret_t rtl8367c_setAsicI2CTxAckCmd(void);
extern ret_t rtl8367c_setAsicI2CTxNoAckCmd(void);
extern ret_t rtl8367c_setAsicI2CSoftRSTseqCmd(void);
extern ret_t rtl8367c_setAsicI2CGpioPinGroup(rtk_uint32 pinGroup_ID);
extern ret_t rtl8367c_getAsicI2CGpioPinGroup(rtk_uint32 * pPinGroup_ID);





#endif /*#ifndef _RTL8367C_ASICDRV_I2C_H_*/

