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
 * $Date: 2016-02-27 14:32:56 +0800 (周三, 17 四月 2016) $
 *
 * Purpose : RTL8367C switch high-level API for RTL8367C
 * Feature : I2C related functions
 *
 */


#include <rtl8367c_asicdrv_i2c.h>
#include <rtk_error.h>
#include <rtk_types.h>



/* Function Name:
 *      rtl8367c_setAsicI2C_checkBusIdle
 * Description:
 *      Check i2c bus status idle or not
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                - Success
 *      RT_ERR_BUSYWAIT_TIMEOUT  - i2c bus is busy
 * Note:
 *      This API can check i2c bus status.
 */
ret_t rtl8367c_setAsicI2C_checkBusIdle(void)
{
    rtk_uint32 regData;
    ret_t retVal;

    if ((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_M_I2C_CTL_STA_REG, RTL8367C_M_I2C_BUS_IDLE_OFFSET, &regData)) != RT_ERR_OK)
        return retVal;

    if(regData == 0x0001)
        return RT_ERR_OK; /*i2c is idle*/
    else
        return RT_ERR_BUSYWAIT_TIMEOUT; /*i2c is busy*/
}


/* Function Name:
 *      rtl8367c_setAsicI2CStartCmd
 * Description:
 *      Set I2C start command
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                - Success
 * Note:
 *      This API can set i2c start command ,start a i2c traffic  .
 */
ret_t rtl8367c_setAsicI2CStartCmd(void)
{
    rtk_uint32 regData;
    ret_t retVal;

    /* Bits [4-1] = 0b0000, Start Command; Bit [0] = 1, Trigger the Command */
    if ((retVal = rtl8367c_getAsicReg(RTL8367C_REG_M_I2C_CTL_STA_REG, &regData)) != RT_ERR_OK)
        return retVal;
    regData &= 0xFFE0;
    regData |= 0x0001;

    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_M_I2C_CTL_STA_REG, regData)) != RT_ERR_OK)
        return retVal;

    /* wait for command finished */
    do{
       if ((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_M_I2C_CTL_STA_REG, RTL8367C_I2C_CMD_EXEC_OFFSET, &regData)) != RT_ERR_OK)
            return retVal;
    }while( regData != 0x0);

    return RT_ERR_OK ;
}

/* Function Name:
 *      rtl8367c_setAsicI2CStopCmd
 * Description:
 *      Set I2C stop command
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                - Success
 * Note:
 *      This API can set i2c stop command ,stop a i2c traffic.
 */
ret_t rtl8367c_setAsicI2CStopCmd(void)
{

    rtk_uint32 regData;
    ret_t retVal;

    /* Bits [4-1] = 0b0001, Stop Command; Bit [0] = 1, Trigger the Command */
    if ((retVal = rtl8367c_getAsicReg(RTL8367C_REG_M_I2C_CTL_STA_REG, &regData)) != RT_ERR_OK)
        return retVal;
    regData &= 0xFFE0;
    regData |= 0x0003;

    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_M_I2C_CTL_STA_REG, regData)) != RT_ERR_OK)
        return retVal;


    /* wait for command finished */
    do{
       if ((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_M_I2C_CTL_STA_REG, RTL8367C_I2C_CMD_EXEC_OFFSET, &regData)) != RT_ERR_OK)
            return retVal;
    }while( regData != 0x0);

    return RT_ERR_OK ;
}

/* Function Name:
 *      rtl8367c_setAsicI2CTxOneCharCmd
 * Description:
 *      Set I2C Tx a char command, with a 8-bit data
 * Input:
 *      oneChar - 8-bit data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                - Success
 * Note:
 *      This API can set i2c Tx command and with a 8-bit data.
 */
ret_t rtl8367c_setAsicI2CTxOneCharCmd(rtk_uint8 oneChar)
{
    rtk_uint32 regData;
    ret_t retVal;

    /* Bits [4-1] = 0b0010, tx one char; Bit [0] = 1, Trigger the Command */
    if ((retVal = rtl8367c_getAsicReg(RTL8367C_REG_M_I2C_CTL_STA_REG, &regData)) != RT_ERR_OK)
        return retVal;

    regData &= 0xFFE0;
    regData |= 0x0005;
    regData &= 0x00FF;
    regData |= (rtk_uint16) (oneChar << 8);

    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_M_I2C_CTL_STA_REG, regData)) != RT_ERR_OK)
        return retVal;


   /* wait for command finished */
    do{
       if ((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_M_I2C_CTL_STA_REG, RTL8367C_I2C_CMD_EXEC_OFFSET, &regData)) != RT_ERR_OK)
            return retVal;
    }while( regData != 0x0);

    return RT_ERR_OK ;
}


/* Function Name:
 *      rtl8367c_setAsicI2CcheckRxAck
 * Description:
 *      Check if rx an Ack
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                - Success
 * Note:
 *      This API can check if rx an ack from i2c slave.
 */
ret_t rtl8367c_setAsicI2CcheckRxAck(void)
{
    rtk_uint32 regData;
    ret_t retVal;
    rtk_uint32 count = 0;

    do{
         count++;
         if ((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_M_I2C_CTL_STA_REG, RTL8367C_SLV_ACK_FLAG_OFFSET, &regData)) != RT_ERR_OK)
            return retVal;
    }while( (regData != 0x1) && (count < TIMEROUT_FOR_MICROSEMI) );

    if(regData != 0x1)
        return RT_ERR_FAILED;
    else
        return RT_ERR_OK;
}


/* Function Name:
 *      rtl8367c_setAsicI2CRxOneCharCmd
 * Description:
 *      Set I2C Rx command and get 8-bit data
 * Input:
 *      None
 * Output:
 *      pValue - 8bit-data
 * Return:
 *      RT_ERR_OK                - Success
 * Note:
 *      This API can set I2C Rx command and get 8-bit data.
 */
ret_t rtl8367c_setAsicI2CRxOneCharCmd(rtk_uint8 *pValue)
{
    rtk_uint32 regData;
    ret_t retVal;

    /* Bits [4-1] = 0b0011, Rx one char; Bit [0] = 1, Trigger the Command */
    if ((retVal = rtl8367c_getAsicReg(RTL8367C_REG_M_I2C_CTL_STA_REG, &regData)) != RT_ERR_OK)
        return retVal;
    regData &= 0xFFE0;
    regData |= 0x0007;
    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_M_I2C_CTL_STA_REG, regData)) != RT_ERR_OK)
        return retVal;

    /* wait for command finished */
     do{
        if ((retVal = rtl8367c_getAsicReg(RTL8367C_REG_M_I2C_CTL_STA_REG, &regData)) != RT_ERR_OK)
             return retVal;
     }while( (regData & 0x1) != 0x0);

    *pValue = (rtk_uint8)(regData >> 8);
     return RT_ERR_OK ;

}

/* Function Name:
 *      rtl8367c_setAsicI2CTxAckCmd
 * Description:
 *      Set I2C Tx ACK command
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                - Success
 * Note:
 *      This API can set I2C Tx ack command.
 */
ret_t rtl8367c_setAsicI2CTxAckCmd(void)
{
    rtk_uint32 regData;
    ret_t retVal;

    /* Bits [4-1] = 0b0100, tx ACK Command; Bit [0] = 1, Trigger the Command */
    if ((retVal = rtl8367c_getAsicReg(RTL8367C_REG_M_I2C_CTL_STA_REG, &regData)) != RT_ERR_OK)
        return retVal;
    regData &= 0xFFE0;
    regData |= 0x0009;
    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_M_I2C_CTL_STA_REG, regData)) != RT_ERR_OK)
        return retVal;

     /* wait for command finished */
    do{
       if ((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_M_I2C_CTL_STA_REG, RTL8367C_I2C_CMD_EXEC_OFFSET, &regData)) != RT_ERR_OK)
            return retVal;
    }while( regData != 0x0);

    return RT_ERR_OK ;

}


/* Function Name:
 *      rtl8367c_setAsicI2CTxNoAckCmd
 * Description:
 *      Set I2C master Tx noACK command
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                - Success
 * Note:
 *      This API can set I2C master Tx noACK command.
 */
ret_t rtl8367c_setAsicI2CTxNoAckCmd(void)
{
    rtk_uint32 regData;
    ret_t retVal;

    /* Bits [4-1] = 0b0101, tx noACK Command; Bit [0] = 1, Trigger the Command */
    if ((retVal = rtl8367c_getAsicReg(RTL8367C_REG_M_I2C_CTL_STA_REG, &regData)) != RT_ERR_OK)
        return retVal;
    regData &= 0xFFE0;
    regData |= 0x000b;
    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_M_I2C_CTL_STA_REG, regData)) != RT_ERR_OK)
        return retVal;

     /* wait for command finished */
    do{
       if ((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_M_I2C_CTL_STA_REG, RTL8367C_I2C_CMD_EXEC_OFFSET, &regData)) != RT_ERR_OK)
            return retVal;
    }while( regData != 0x0);

    return RT_ERR_OK ;

}

/* Function Name:
 *      rtl8367c_setAsicI2CSoftRSTseqCmd
 * Description:
 *      set I2C master tx soft reset command
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                - Success
 * Note:
 *      This API can set I2C master tx soft reset command.
 */
ret_t rtl8367c_setAsicI2CSoftRSTseqCmd(void)
{

    rtk_uint32 regData;
    ret_t retVal;

    /*Bits [4-1] = 0b0110, tx soft reset Command;  Bit [0] = 1, Trigger the Command */
    if ((retVal = rtl8367c_getAsicReg(RTL8367C_REG_M_I2C_CTL_STA_REG, &regData)) != RT_ERR_OK)
        return retVal;
    regData &= 0xFFE0;
    regData |= 0x000d;

    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_M_I2C_CTL_STA_REG, regData)) != RT_ERR_OK)
        return retVal;


    /* wait for command finished */
    do{
       if ((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_M_I2C_CTL_STA_REG, RTL8367C_I2C_CMD_EXEC_OFFSET, &regData)) != RT_ERR_OK)
            return retVal;
    }while( regData != 0x0);

    return RT_ERR_OK ;
}


/* Function Name:
 *      rtl8367c_setAsicI2CGpioPinGroup
 * Description:
 *      set I2C function used gpio pins
 * Input:
 *      pinGroup_ID - gpio pins group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                - Success
 *      RT_ERR_INPUT             _ Invalid input parameter
 * Note:
 *      This API can set I2C function used gpio pins.
 *      There are three group gpio pins
 */
ret_t rtl8367c_setAsicI2CGpioPinGroup(rtk_uint32 pinGroup_ID)
{
    rtk_uint32 regData;
    ret_t retVal;

    if ((retVal = rtl8367c_getAsicReg(RTL8367C_REG_M_I2C_SYS_CTL, &regData)) != RT_ERR_OK)
         return retVal;
    if( pinGroup_ID==0 )
    {
        regData &= 0x0FFF;
        regData |= 0x5000;

        if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_M_I2C_SYS_CTL, regData)) != RT_ERR_OK)
             return retVal;
    }

    else if( pinGroup_ID==1 )
    {
        regData &= 0x0FFF;
        regData |= 0xA000;

        if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_M_I2C_SYS_CTL, regData)) != RT_ERR_OK)
             return retVal;
    }

    else if( pinGroup_ID==2 )
    {
        regData &= 0x0FFF;
        regData |= 0xF000;

        if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_M_I2C_SYS_CTL, regData)) != RT_ERR_OK)
             return retVal;
    }
    else
        return RT_ERR_INPUT;

    return RT_ERR_OK ;

}

/* Function Name:
 *      rtl8367c_setAsicI2CGpioPinGroup
 * Description:
 *      set I2C function used gpio pins
 * Input:
 *      pinGroup_ID - gpio pins group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                - Success
 *      RT_ERR_INPUT             _ Invalid input parameter
 * Note:
 *      This API can set I2C function used gpio pins.
 *      There are three group gpio pins
 */
ret_t rtl8367c_getAsicI2CGpioPinGroup(rtk_uint32 * pPinGroup_ID)
{

    rtk_uint32 regData;
    ret_t retVal;
    if( (retVal = rtl8367c_getAsicReg(RTL8367C_REG_M_I2C_SYS_CTL, &regData)) != RT_ERR_OK)
        return retVal;
    regData &= 0xF000 ;
    regData = (regData >> 12);

    if( regData == 0x5 )
        *pPinGroup_ID = 0;
    else if(regData == 0xA)
        *pPinGroup_ID = 1;
    else if(regData == 0xF)
        *pPinGroup_ID = 2;
    else
       return RT_ERR_FAILED;
    return RT_ERR_OK ;
}



































