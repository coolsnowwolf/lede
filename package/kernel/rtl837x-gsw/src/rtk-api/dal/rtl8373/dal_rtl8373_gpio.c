/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision$
 * $Date$
 *
 * Purpose : RTK switch high-level API for RTL8373
 * Feature : Here is a list of all functions and variables in GPIO module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal/rtl8373/dal_rtl8373_gpio.h>
#include <string.h>
#include <dal/rtl8373/rtl8373_asicdrv.h>



/* Function Name:
 *      dal_rtl8373_gpio_muxSel_set
 * Description:
 *      enable gpio pin 
 * Input:
 *      gpioNum - GPIO pin number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  - out of range
 * Note:
 *     
 */
rtk_api_ret_t dal_rtl8373_gpio_muxSel_set(rtk_uint32 gpioNum)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 val = 0, mask  = 0;

    if(gpioNum > GPIO_MAX_PIN_NUM) 
        return RT_ERR_RANGE;

    if(gpioNum < 30)
    {
        val = 1;
        if((retVal = rtl8373_setAsicRegBit(RTL8373_IO_MUX_SEL_0_ADDR, gpioNum, val)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum == 30)
    {
        val = 1;
        if((retVal = rtl8373_setAsicRegBit(RTL8373_IO_MUX_SEL_2_ADDR, RTL8373_IO_MUX_SEL_2_ACL_BIT3_EN_OFFSET, val)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum == 31 || gpioNum == 32)
    {
        // we have to set RTL8373_IO_MUX_SEL_1_PAD_UART0_SEL_0_MASK and RTL8373_IO_MUX_SEL_1_PAD_UART0_SEL_1_MASK
        // only set RTL8373_IO_MUX_SEL_1_PAD_UART0_SEL_0_MASK or RTL8373_IO_MUX_SEL_1_PAD_UART0_SEL_1_MASK is not work
        if((retVal = rtl8373_setAsicRegBits(RTL8373_IO_MUX_SEL_1_ADDR, RTL8373_IO_MUX_SEL_1_PAD_UART0_SEL_0_MASK | RTL8373_IO_MUX_SEL_1_PAD_UART0_SEL_1_MASK, 0x3)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum >= 33 && gpioNum <= 35 )
    {
        val = 1;
        if((retVal = rtl8373_setAsicRegBit(RTL8373_IO_MUX_SEL_1_ADDR, (gpioNum - 31), val)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum == 36)
    {
        if((retVal = rtl8373_setAsicRegBit(RTL8373_IO_MUX_SEL_1_ADDR, RTL8373_IO_MUX_SEL_1_GPIO_PWM_OUT_SEL_OFFSET, 1)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum == 37 || gpioNum == 38)
    {
        ;//do nothing
    }
    else if (gpioNum == 39)
    {
        if((retVal = rtl8373_setAsicRegBit(RTL8373_IO_MUX_SEL_1_ADDR, RTL8373_IO_MUX_SEL_1_GPIO_SDA4_SEL_OFFSET, 1)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum == 40 || gpioNum == 41)
    {
        val = 1;
        mask = gpioNum == 40 ? RTL8373_IO_MUX_SEL_1_GPIO_MDX1_SEL_0_MASK : RTL8373_IO_MUX_SEL_1_GPIO_MDX1_SEL_1_MASK;
        if((retVal = rtl8373_setAsicRegBits(RTL8373_IO_MUX_SEL_1_ADDR, mask, val)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum >= 42 && gpioNum <= 45)
    {
        // what fuck is this?
        // I don't know how does realtek use two bits to represent gpio 42 43 44 45
        // Is this a magic?????
        if((retVal = rtl8373_setAsicRegBits(RTL8373_INI_MODE_ADDR, RTL8373_INI_MODE_INI_MODE_MASK , val)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum >= 46 && gpioNum <= 51)
    {
        mask = (0x180 << ((gpioNum-46)*2));
        // what fuck is this????
        // I don't know how does realtek use two bits to represent gpio 46 47 48 49 50 51
        // Is this a magic?????
        if((retVal = rtl8373_setAsicRegBits(RTL8373_IO_MUX_SEL_1_ADDR, mask , val)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum >= 52 && gpioNum <= 54)
    {
        val = 1;
        if((retVal = rtl8373_setAsicRegBit(RTL8373_IO_MUX_SEL_2_ADDR, gpioNum -52, val)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum >= 55 && gpioNum <= 60)
    {
        val = 1;
        if((retVal = rtl8373_setAsicRegBit(RTL8373_IO_MUX_SEL_1_ADDR, gpioNum -36, val)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum == 61 || gpioNum == 62)
    {
        val = 1;
        if((retVal = rtl8373_setAsicRegBit(RTL8373_IO_MUX_SEL_1_ADDR, gpioNum -34, val)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum == 63) //gpio63
    {
        val = 1;
        if((retVal = rtl8373_setAsicRegBit(RTL8373_IO_MUX_SEL_1_ADDR, RTL8373_IO_MUX_SEL_1_GPIO_MDIO0_SEL_OFFSET, val)) != RT_ERR_OK)
            return retVal;
    }else
    {
        return RT_ERR_RANGE;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_gpio_muxSel_get
 * Description:
 *      Get gpio pin status 
 * Input:
 *      gpioNum - GPIO pin number
 * Output:
 *      *pStatus - status
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  - out of range
 * Note:
 *     
 */
rtk_api_ret_t dal_rtl8373_gpio_muxSel_get(rtk_uint32 gpioNum, rtk_enable_t *pStatus )
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 val = 0, mask  = 0;

    if(gpioNum > GPIO_MAX_PIN_NUM) 
        return RT_ERR_RANGE;
    if(NULL == pStatus)
        return RT_ERR_NULL_POINTER;


    if(gpioNum < 30)
    {
        if((retVal = rtl8373_getAsicRegBit(RTL8373_IO_MUX_SEL_0_ADDR, gpioNum, &val)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum == 30)
    {
        if((retVal = rtl8373_getAsicRegBit(RTL8373_IO_MUX_SEL_2_ADDR, RTL8373_IO_MUX_SEL_2_ACL_BIT3_EN_OFFSET, &val)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum == 31 || gpioNum == 32)
    {
        if((retVal = rtl8373_getAsicRegBits(RTL8373_IO_MUX_SEL_1_ADDR, (RTL8373_IO_MUX_SEL_1_PAD_UART0_SEL_0_MASK | RTL8373_IO_MUX_SEL_1_PAD_UART0_SEL_1_MASK), &val)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum >= 33 && gpioNum <= 35 )
    {
        if((retVal = rtl8373_getAsicRegBit(RTL8373_IO_MUX_SEL_1_ADDR, (gpioNum - 31), &val)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum == 36)
    {
        if((retVal = rtl8373_getAsicRegBit(RTL8373_IO_MUX_SEL_1_ADDR, RTL8373_IO_MUX_SEL_1_GPIO_PWM_OUT_SEL_OFFSET, &val)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum == 37 || gpioNum == 38)
    {
        val = 0;
    }
    else if (gpioNum == 39)
    {
        if((retVal = rtl8373_getAsicRegBit(RTL8373_IO_MUX_SEL_1_ADDR, RTL8373_IO_MUX_SEL_1_GPIO_SDA4_SEL_OFFSET, &val)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum == 40 || gpioNum == 41)
    {
        if((retVal = rtl8373_getAsicRegBits(RTL8373_IO_MUX_SEL_1_ADDR, (RTL8373_IO_MUX_SEL_1_GPIO_MDX1_SEL_0_MASK | RTL8373_IO_MUX_SEL_1_GPIO_MDX1_SEL_1_MASK), &val)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum >= 42 && gpioNum <= 45)
    {
        if((retVal = rtl8373_getAsicRegBits(RTL8373_INI_MODE_ADDR, RTL8373_INI_MODE_INI_MODE_MASK , &val)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum >= 46 && gpioNum <= 51)
    {
        mask = (0x180 << ((gpioNum-46)*2));
        if((retVal = rtl8373_getAsicRegBits(RTL8373_IO_MUX_SEL_1_ADDR, mask , &val)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum >= 52 && gpioNum <= 54)
    {
        if((retVal = rtl8373_getAsicRegBit(RTL8373_IO_MUX_SEL_2_ADDR, gpioNum -52, &val)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum >= 55 && gpioNum <= 60)
    {
        if((retVal = rtl8373_getAsicRegBit(RTL8373_IO_MUX_SEL_1_ADDR, gpioNum -36, &val)) != RT_ERR_OK)
            return retVal;
    }
    else if (gpioNum == 61 || gpioNum == 62)
    {
        if((retVal = rtl8373_getAsicRegBit(RTL8373_IO_MUX_SEL_1_ADDR, gpioNum -34, &val)) != RT_ERR_OK)
            return retVal;
    }
    else//gpio63
    {
        if((retVal = rtl8373_getAsicRegBit(RTL8373_IO_MUX_SEL_1_ADDR, RTL8373_IO_MUX_SEL_1_GPIO_MDIO0_SEL_OFFSET, &val)) != RT_ERR_OK)
            return retVal;
    }
    
    *pStatus = (rtk_enable_t)val ;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_gpio_groupVal_write
 * Description:
 *      write 32bits gpio pin val
 * Input:
 *      idx - GPIO pin LSB 32bits or MSB 31bits
 *      val         - value
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  - out of range
 * Note:
 *     
 */
rtk_api_ret_t dal_rtl8373_gpio_groupVal_write(rtk_gpio_groupReg_t idx, rtk_uint32 val )
{
    rtk_uint32 regAddr = 0 , retVal = 0;

    if(idx >= GPIO_INOUT_REG_END)
        return RT_ERR_RANGE;

    if(idx == GPIO_INOUT_LSB_REG)
        regAddr = RTL8373_GPIO_OUT0_ADDR;
    else
        regAddr = RTL8373_GPIO_OUT1_ADDR;
    
    if((retVal = rtl8373_setAsicReg(regAddr, val)) != RT_ERR_OK)
        return retVal;   

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_gpio_groupVal_read
 * Description:
 *      Read 32bits gpio pin val 
 * Input:
 *      idx - GPIO pin LSB 32bits or MSB 31bits
 * Output:
 *      *pVal - value
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  - out of range
 * Note:
 *     
 */
rtk_api_ret_t dal_rtl8373_gpio_groupVal_read(rtk_gpio_groupReg_t idx, rtk_uint32 *pVal )
{
    rtk_uint32 regAddr = 0 , retVal = 0;

    if(idx >= GPIO_INOUT_REG_END)
        return RT_ERR_RANGE;
    if(NULL == pVal)
        return RT_ERR_NULL_POINTER;

    if(idx == GPIO_INOUT_LSB_REG)
        regAddr = RTL8373_GPIO_IN0_ADDR;
    else
        regAddr = RTL8373_GPIO_IN1_ADDR;
    
    if((retVal = rtl8373_getAsicReg(regAddr, pVal)) != RT_ERR_OK)
        return retVal;   

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_gpio_pinVal_write
 * Description:
 *      write gpio pin val
 * Input:
 *      idx - GPIO pin LSB 32bits or MSB 31bits
 *      val         - value
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  - out of range
 * Note:
 *     
 */
rtk_api_ret_t dal_rtl8373_gpio_pinVal_write(rtk_uint32 gpioNum, rtk_gpio_level_t val )
{
    rtk_uint32 regAddr = 0 , retVal = 0, offset = 0;

    if(gpioNum >= GPIO_MAX_PIN_NUM) 
        return RT_ERR_RANGE;
    if(val >= GPIO_LEVEL_END) 
        return RT_ERR_RANGE;

    if(gpioNum < 32 )
    {
        regAddr = RTL8373_GPIO_OUT0_ADDR;
        offset = gpioNum;
    }
    else
    {
        regAddr = RTL8373_GPIO_OUT1_ADDR;
        offset = gpioNum - 32;
    }
    
    if((retVal = rtl8373_setAsicRegBit(regAddr, offset,  val)) != RT_ERR_OK)
        return retVal;   

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_gpio_pinVal_read
 * Description:
 *      Read gpio pin val 
 * Input:
 *      idx - GPIO pin LSB 32bits or MSB 31bits
 * Output:
 *      *pVal - value
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  - out of range
 * Note:
 *     
 */
rtk_api_ret_t dal_rtl8373_gpio_pinVal_read(rtk_uint32 gpioNum, rtk_gpio_level_t *pVal )
{
    rtk_uint32 regAddr = 0 , retVal = 0, offset = 0, regVal = 0;

    if(gpioNum >= GPIO_MAX_PIN_NUM) 
        return RT_ERR_RANGE;
    if(NULL == pVal)
        return RT_ERR_NULL_POINTER;

    if(gpioNum < 32 )
    {
        regAddr = RTL8373_GPIO_IN0_ADDR;
        offset = gpioNum;
    }
    else
    {
        regAddr = RTL8373_GPIO_IN1_ADDR;
        offset = gpioNum - 32;
    }
    
    if((retVal = rtl8373_getAsicRegBit(regAddr, offset, &regVal)) != RT_ERR_OK)
        return retVal;   

    *pVal = (rtk_gpio_level_t)regVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_gpio_pinDir_set
 * Description:
 *      set gpio pin direction
 * Input:
 *      gpioNum - GPIO pin num
 *      dir  - GPIO pin direction
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  - out of range
 * Note:
 *     
 */
rtk_api_ret_t dal_rtl8373_gpio_pinDir_set(rtk_uint32 gpioNum, rtk_gpio_direction_t dir )
{
    rtk_uint32 regAddr = 0 , retVal = 0, offset = 0;

    if((gpioNum >= GPIO_MAX_PIN_NUM) || (dir>= GPIO_DIR_END))
        return RT_ERR_RANGE;

    if(gpioNum < 32)
    {
        regAddr = RTL8373_GPIO_OE0_ADDR;
        offset = gpioNum;
    }
    else
    {
        regAddr = RTL8373_GPIO_OE1_ADDR;
        offset = gpioNum - 32;
    }
    
    if((retVal = rtl8373_setAsicRegBit(regAddr, offset, dir)) != RT_ERR_OK)
        return retVal;   

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_gpio_pinDir_get
 * Description:
 *      Get gpio pin direction
 * Input:
 *      gpioNum - GPIO pin num
 * Output:
 *      dir  - GPIO pin direction
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  - out of range
 * Note:
 *     
 */
rtk_api_ret_t dal_rtl8373_gpio_pinDir_get(rtk_uint32 gpioNum, rtk_gpio_direction_t *pDir )
{
    rtk_uint32 regAddr = 0 , retVal = 0, offset = 0, regVal = 0;

    if(gpioNum >= GPIO_MAX_PIN_NUM) 
        return RT_ERR_RANGE;
    if(NULL == pDir)
        return RT_ERR_NULL_POINTER;

    if(gpioNum < 32)
    {
        regAddr = RTL8373_GPIO_OE0_ADDR;
        offset = gpioNum;
    }
    else
    {
        regAddr = RTL8373_GPIO_OE1_ADDR;
        offset = gpioNum - 32;
    }
    
    if((retVal = rtl8373_getAsicRegBit(regAddr, offset, &regVal)) != RT_ERR_OK)
        return retVal;   

    *pDir = (regVal & 1);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_gpio_groupDir_get
 * Description:
 *      Get gpio pin direction
 * Input:
 *      idx - GPIO pin LSB 32bits or MSB 31bits
 * Output:
 *      pDirVal  -  Group GPIO pin direction
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  - out of range
 * Note:
 *     
 */
rtk_api_ret_t dal_rtl8373_gpio_groupDir_get(rtk_gpio_groupReg_t idx, rtk_uint32 *pDirVal )
{
    rtk_uint32 regAddr = 0 , retVal = 0;

    if(idx >= GPIO_INOUT_REG_END)
          return RT_ERR_RANGE;
      if(NULL == pDirVal)
          return RT_ERR_NULL_POINTER;

    if(idx == GPIO_INOUT_LSB_REG)
    {
        regAddr = RTL8373_GPIO_OE0_ADDR;
    }
    else
    {
        regAddr = RTL8373_GPIO_OE1_ADDR;
    }
    
    if((retVal = rtl8373_getAsicReg(regAddr, pDirVal)) != RT_ERR_OK)
        return retVal;   

    return RT_ERR_OK;
}

