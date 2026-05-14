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
#include <gpio.h>
#include <string.h>
#include <dal/dal_mgmt.h>




/* Function Name:
 *      rtk_gpio_muxSel_set
 * Description:
 *      enable gpio  function
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
rtk_api_ret_t rtk_gpio_muxSel_set(rtk_uint32 gpioNum)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->gpio_muxSel_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->gpio_muxSel_set(gpioNum);
    RTK_API_UNLOCK();

    return retVal;
}


/* Function Name:
 *      dal_rtl8371c_gpio_muxSel_get
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
 rtk_api_ret_t rtk_gpio_muxSel_get(rtk_uint32 gpioNum, rtk_enable_t *pStatus )
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->gpio_muxSel_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->gpio_muxSel_get(gpioNum, pStatus);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_gpio_groupVal_write
 * Description:
 *      write group 32bits gpio pin val
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
rtk_api_ret_t rtk_gpio_groupVal_write(rtk_gpio_groupReg_t idx, rtk_uint32 val )
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->gpio_pinVal_write)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->gpio_pinVal_write(idx, val);
    RTK_API_UNLOCK();

    return retVal;

}

/* Function Name:
 *      rtk_gpio_groupVal_read
 * Description:
 *      Read group 32bits gpio pin val 
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
rtk_api_ret_t rtk_gpio_groupVal_read(rtk_gpio_groupReg_t idx, rtk_uint32 *pVal)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->gpio_pinVal_read)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->gpio_pinVal_read(idx, pVal);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_gpio_pinVal_write
 * Description:
 *      write gpio pin val
 * Input:
 *      gpioNum - GPIO pin num
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
rtk_api_ret_t rtk_gpio_pinVal_write(rtk_uint32 gpioNum, rtk_gpio_level_t val)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->gpio_pinVal_write)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->gpio_pinVal_write(gpioNum, val);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_gpio_pinVal_read
 * Description:
 *      Read gpio pin val 
 * Input:
 *      gpioNum - GPIO pin num
 * Output:
 *      *pVal - value
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  - out of range
 * Note:
 *     
 */
rtk_api_ret_t rtk_gpio_pinVal_read(rtk_uint32 gpioNum, rtk_gpio_level_t *pVal)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->gpio_pinVal_read)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->gpio_pinVal_read(gpioNum, pVal);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_gpio_pinDir_set
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
rtk_api_ret_t rtk_gpio_pinDir_set(rtk_uint32 gpioNum, rtk_gpio_direction_t dir)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->gpio_pinDir_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->gpio_pinDir_set(gpioNum, dir);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_gpio_pinDir_get
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
rtk_api_ret_t rtk_gpio_pinDir_get(rtk_uint32 gpioNum, rtk_gpio_direction_t *pDir)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->gpio_pinDir_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->gpio_pinDir_get(gpioNum, pDir);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_gpio_groupDir_get
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
rtk_api_ret_t rtk_gpio_groupDir_get(rtk_gpio_groupReg_t idx, rtk_uint32 *pDirVal)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->gpio_groupDir_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->gpio_groupDir_get(idx, pDirVal);
    RTK_API_UNLOCK();

    return retVal;
}


