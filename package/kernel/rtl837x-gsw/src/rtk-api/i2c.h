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

#ifndef __RTK_API_I2C_H__
#define __RTK_API_I2C_H__

#define I2C_GROUP_NUM            (5)
#define I2C_MEM_ADDR_WIDTH_MAX   (3)
#define I2C_DATA_WIDTH_MAX		 (16)


typedef enum rtk_i2c_readMode_e 
{
    I2C_READ_STANDARD_MODE = 0,
    I2C_READ_OLD_MODE,
    I2C_READ_MODE_END 
}rtk_i2c_readMode_t;

typedef enum rtk_i2c_sclClockRate_e 
{
    I2C_SCL_CLK_50KHZ = 0,
    I2C_SCL_CLK_100KHZ,
    I2C_SCL_CLK_400KHZ, 
    I2C_SCL_CLK_2500KHZ,
    I2C_SCL_CLK_END
}rtk_i2c_sclClockRate_t;

typedef enum rtk_i2c_rwActTrigger_e 
{
    I2C_RW_ACT_IDLE = 0,
    I2C_RW_ACT_TRIGGER,
    I2C_RW_ACT_END
}rtk_i2c_rwActTrigger_t;

typedef enum rtk_i2c_rwOperation_e 
{
    I2C_RW_OP_READ = 0,
    I2C_RW_OP_WRITE,
    I2C_RW_OP_END
}rtk_i2c_rwOperation_t;


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
extern rtk_api_ret_t rtk_i2c_init(rtk_i2c_sclClockRate_t clkRate, rtk_uint32 deviceAddr);

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
extern rtk_api_ret_t rtk_i2c_readMode_set(rtk_i2c_readMode_t mode);

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
extern rtk_api_ret_t rtk_i2c_readMode_get(rtk_i2c_readMode_t *pMode);

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
extern rtk_api_ret_t rtk_i2c_gpioPinGroup_set(rtk_uint32 sclNum, rtk_uint32 sdaNum);

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
extern rtk_api_ret_t rtk_i2c_gpioPinGroup_get(rtk_uint32 *pSclNum, rtk_uint32 *pSdaNum);

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
extern rtk_api_ret_t rtk_i2c_data_read(rtk_uint32 memAddr, rtk_uint32 dataWidth, rtk_uint32 memAddrWidth, rtk_uint32 *pReadData);

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
extern rtk_api_ret_t rtk_i2c_data_write(rtk_uint32 memAddr, rtk_uint32 dataWidth, rtk_uint32 memAddrWidth, rtk_uint32 *pWriteData);


#endif /* __RTK_API_I2C_H__ */


