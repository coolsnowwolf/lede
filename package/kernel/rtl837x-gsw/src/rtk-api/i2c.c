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
 * Feature : Here is a list of all functions and variables in I2C module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <i2c.h>
#include <string.h>
#include <dal/dal_mgmt.h>

/* Function Name:
 *      rtk_i2c_init
 * Description:
 *      initial i2c config
 * Input: 
 *      clkRate - I2C SCL clock rate
 *      deviceAddr  - I2C slave device address
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  	- out of range
 * Note:
 *     
 */
rtk_api_ret_t rtk_i2c_init(rtk_i2c_sclClockRate_t clkRate, rtk_uint32 deviceAddr)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->i2c_init)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->i2c_init(clkRate, deviceAddr);
    RTK_API_UNLOCK();

    return retVal;

}

/* Function Name:
 *      rtk_i2c_readMode_set
 * Description:
 *      set i2c read mode
 * Input: 
 *      mode - standard mode or old mode
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  	- out of range
 * Note:
 *     
 */
rtk_api_ret_t rtk_i2c_readMode_set(rtk_i2c_readMode_t mode)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->i2c_readMode_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->i2c_readMode_set(mode);
    RTK_API_UNLOCK();

    return retVal;

}

/* Function Name:
 *      rtk_i2c_readMode_get
 * Description:
 *      get i2c read mode
 * Input: 
 *      none
 * Output:
 *      pMode - standard mode or old mode
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  	- out of range
 * Note:
 *     
 */
rtk_api_ret_t rtk_i2c_readMode_get(rtk_i2c_readMode_t *pMode)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->i2c_readMode_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->i2c_readMode_get(pMode);
    RTK_API_UNLOCK();

    return retVal;

}

/* Function Name:
 *      rtk_i2c_gpioPinGroup_set
 * Description:
 *      config i2c scl and sda used gpio pin
 * Input: 
 *      sclNum - scl pad num
 *      sdaNum - sda pad num
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  	- out of range
 * Note:
 *     
 */
rtk_api_ret_t rtk_i2c_gpioPinGroup_set(rtk_uint32 sclNum, rtk_uint32 sdaNum)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->i2c_gpioPinGroup_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->i2c_gpioPinGroup_set(sclNum, sdaNum);
    RTK_API_UNLOCK();

    return retVal;

}

/* Function Name:
 *      rtk_i2c_gpioPinGroup_get
 * Description:
 *      config i2c scl and sda used gpio pin
 * Input: 
 *      none
 * Output:
 *      pSclNum - scl pad num
 *      pSdaNum - sda pad num
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  	- out of range
 * Note:
 *     
 */
rtk_api_ret_t rtk_i2c_gpioPinGroup_get(rtk_uint32 *pSclNum, rtk_uint32 *pSdaNum)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->i2c_gpioPinGroup_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->i2c_gpioPinGroup_get(pSclNum, pSdaNum);
    RTK_API_UNLOCK();

    return retVal;

}

/* Function Name:
 *      rtk_i2c_data_read
 * Description:
 *      i2c master read slave device data
 * Input: 
 *      memAddr 	- slave device memory address 
 *      dataWidth 	- want to read data width (1~16)
 *		memAddrWidth - slave device memory address width (0~3)
 * Output:
 *      pReadData 		- slave device data that has read
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  	- out of range
 * Note:
 *     
 */
rtk_api_ret_t rtk_i2c_data_read(rtk_uint32 memAddr, rtk_uint32 dataWidth, rtk_uint32 memAddrWidth, rtk_uint32 *pReadData)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->i2c_data_read)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->i2c_data_read(memAddr, dataWidth, memAddrWidth, pReadData);
    RTK_API_UNLOCK();

    return retVal;

}

/* Function Name:
 *      rtk_i2c_data_write
 * Description:
 *      i2c master write data to slave device's memory
 * Input: 
 *      memAddr 	- slave device memory address want to write 
 *      dataWidth 	- want to write data's width (1~16)
 *		memAddrWidth - slave device memory address width (0~3)
 *		pWriteData	 - the data going to wirte to slave device
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  	- out of range
 * Note:
 *     
 */
rtk_api_ret_t rtk_i2c_data_write(rtk_uint32 memAddr, rtk_uint32 dataWidth, rtk_uint32 memAddrWidth, rtk_uint32 *pWriteData)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->i2c_data_write)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->i2c_data_write(memAddr, dataWidth, memAddrWidth, pWriteData);
    RTK_API_UNLOCK();

    return retVal;
}



