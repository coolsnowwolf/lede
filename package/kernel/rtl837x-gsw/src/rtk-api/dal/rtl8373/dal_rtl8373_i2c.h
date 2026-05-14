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
 * Purpose : RTL8373 switch high-level API
 *
 * Feature : The file includes I2C module high-layer API defination
 *
 */

#ifndef  __DAL_RTL8373_I2C_H__
#define  __DAL_RTL8373_I2C_H__
#include <i2c.h>
 

/* Function Name:
 *      dal_rtl8373_i2c_init
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
extern rtk_api_ret_t dal_rtl8373_i2c_init(rtk_i2c_sclClockRate_t clkRate, rtk_uint32 deviceAddr);

/* Function Name:
 *      dal_rtl8373_i2c_readMode_set
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
extern rtk_api_ret_t dal_rtl8373_i2c_readMode_set(rtk_i2c_readMode_t mode);

/* Function Name:
 *      dal_rtl8373_i2c_readMode_get
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
extern rtk_api_ret_t dal_rtl8373_i2c_readMode_get(rtk_i2c_readMode_t *pMode);

/* Function Name:
 *      dal_rtl8373_i2c_gpioPinGroup_set
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
extern rtk_api_ret_t dal_rtl8373_i2c_gpioPinGroup_set(rtk_uint32 sclNum, rtk_uint32 sdaNum);

/* Function Name:
 *      dal_rtl8373_i2c_gpioPinGroup_get
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
extern rtk_api_ret_t dal_rtl8373_i2c_gpioPinGroup_get(rtk_uint32 *pSclNum, rtk_uint32 *pSdaNum);

/* Function Name:
 *      dal_rtl8373_i2c_data_read
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
extern rtk_api_ret_t dal_rtl8373_i2c_data_read(rtk_uint32 memAddr, rtk_uint32 dataWidth, rtk_uint32 memAddrWidth, rtk_uint32 *pReadData);

/* Function Name:
 *      dal_rtl8373_i2c_data_write
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
extern rtk_api_ret_t dal_rtl8373_i2c_data_write(rtk_uint32 memAddr, rtk_uint32 dataWidth, rtk_uint32 memAddrWidth, rtk_uint32 *pWriteData);


#endif /*  __DAL_RTL8373_I2C_H__ */

