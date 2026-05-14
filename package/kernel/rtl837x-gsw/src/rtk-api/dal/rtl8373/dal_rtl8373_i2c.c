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
#include <dal/rtl8373/dal_rtl8373_i2c.h>
#include <string.h>
#include <dal/rtl8373/rtl8373_asicdrv.h>

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
rtk_api_ret_t dal_rtl8373_i2c_init(rtk_i2c_sclClockRate_t clkRate, rtk_uint32 deviceAddr)
{
    rtk_uint32 retVal = 0;

	if((clkRate >= I2C_SCL_CLK_END) || (deviceAddr >= 0x80))
		return RT_ERR_INPUT;	

	/* set i2c clk rate */
	if((retVal = rtl8373_setAsicRegBits(RTL8373_I2C_MST1_CTRL2_ADDR, RTL8373_I2C_MST1_CTRL2_SCL_FREQ_MASK,  (rtk_uint32)clkRate)) != RT_ERR_OK)
        return retVal;  

	/* set device addr */
    if((retVal = rtl8373_setAsicRegBits(RTL8373_I2C_MST1_CTRL1_ADDR, RTL8373_I2C_MST1_CTRL1_DEV_ADDR_MASK,  deviceAddr)) != RT_ERR_OK)
        return retVal;  


    return RT_ERR_OK;
}


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
rtk_api_ret_t dal_rtl8373_i2c_readMode_set(rtk_i2c_readMode_t mode)
{
    rtk_uint32 retVal = 0;
	if(mode >= I2C_READ_MODE_END)
        return RT_ERR_RANGE;
	
    if((retVal = rtl8373_setAsicRegBit(RTL8373_I2C_MST1_CTRL1_ADDR, RTL8373_I2C_MST1_CTRL1_READ_MODE_OFFSET, mode)) != RT_ERR_OK)
        return retVal;   

    return RT_ERR_OK;	

}

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
rtk_api_ret_t dal_rtl8373_i2c_readMode_get(rtk_i2c_readMode_t *pMode)
{
    rtk_uint32 retVal = 0, regVal = 0;
	if(pMode == NULL)
        return RT_ERR_NULL_POINTER;
	
    if((retVal = rtl8373_getAsicRegBit(RTL8373_I2C_MST1_CTRL1_ADDR, RTL8373_I2C_MST1_CTRL1_READ_MODE_OFFSET, &regVal)) != RT_ERR_OK)
		return retVal;   

	*pMode = (rtk_i2c_readMode_t)(regVal & 0x1);

    return RT_ERR_OK;	

}


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
rtk_api_ret_t dal_rtl8373_i2c_gpioPinGroup_set(rtk_uint32 sclNum, rtk_uint32 sdaNum )
{
    rtk_uint32 retVal = 0, mask = 0;

	if((sclNum > I2C_GROUP_NUM) || (sdaNum > I2C_GROUP_NUM) || (sclNum == 4))
		return RT_ERR_RANGE;

	if((retVal = rtl8373_setAsicRegBits(RTL8373_I2C_MST1_CTRL1_ADDR, RTL8373_I2C_MST1_CTRL1_SCL_OUT_SEL_MASK, sclNum)) != RT_ERR_OK)
		return retVal;	

	if((retVal = rtl8373_setAsicRegBits(RTL8373_I2C_MST1_CTRL1_ADDR, RTL8373_I2C_MST1_CTRL1_SDA_OUT_SEL_MASK, sdaNum)) != RT_ERR_OK)
		return retVal;	

	/*config gpio pin to scl function*/
	if(sclNum < 3)
	{
	    mask = (0x180 << (sclNum *4));
        if((retVal = rtl8373_setAsicRegBits(RTL8373_IO_MUX_SEL_1_ADDR, mask, 1)) != RT_ERR_OK)
            return retVal;		
	}
	else if(sclNum == 3)
	{
		if((retVal = rtl8373_setAsicRegBits(RTL8373_IO_MUX_SEL_1_ADDR, RTL8373_IO_MUX_SEL_1_GPIO_MDX1_SEL_1_MASK|RTL8373_IO_MUX_SEL_1_GPIO_MDX1_SEL_0_MASK, 2)) != RT_ERR_OK)
			return retVal;		
	}
	else //sclNum=5
	{
		if((retVal = rtl8373_setAsicRegBit(RTL8373_MAC_IF_CTRL_ADDR, RTL8373_MAC_IF_CTRL_DW8051_INDIRECT_EE_EN_OFFSET, 1)) != RT_ERR_OK)
			return retVal;	
	}

	/*config gpio pin to sda function*/
	if(sdaNum < 3)
	{
	    mask = (0x600 << (sdaNum *4));
        if((retVal = rtl8373_setAsicRegBits(RTL8373_IO_MUX_SEL_1_ADDR, mask , 1)) != RT_ERR_OK)
            return retVal;		
	}
	else if(sdaNum == 3)
	{
		if((retVal = rtl8373_setAsicRegBits(RTL8373_IO_MUX_SEL_1_ADDR, RTL8373_IO_MUX_SEL_1_GPIO_MDX1_SEL_1_MASK|RTL8373_IO_MUX_SEL_1_GPIO_MDX1_SEL_0_MASK, 2)) != RT_ERR_OK)
			return retVal;
	}
	else if(sdaNum == 4)
	{
		if((retVal = rtl8373_setAsicRegBit(RTL8373_IO_MUX_SEL_1_ADDR, RTL8373_IO_MUX_SEL_1_GPIO_SDA4_SEL_OFFSET, 0)) != RT_ERR_OK)
			return retVal;		
	}
	else //sdaNum=5
	{
		if((retVal = rtl8373_setAsicRegBit(RTL8373_MAC_IF_CTRL_ADDR, RTL8373_MAC_IF_CTRL_DW8051_INDIRECT_EE_EN_OFFSET, 1)) != RT_ERR_OK)
			return retVal;	
	}

	return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_i2c_gpioPinGroup_get(rtk_uint32 *pSclNum, rtk_uint32 *pSdaNum )
{
    rtk_uint32 retVal = 0;

	if((pSdaNum == NULL) || (pSclNum == NULL))
		return RT_ERR_NULL_POINTER;

	if((retVal = rtl8373_getAsicRegBits(RTL8373_I2C_MST1_CTRL1_ADDR, RTL8373_I2C_MST1_CTRL1_SCL_OUT_SEL_MASK, pSclNum)) != RT_ERR_OK)
		return retVal;	

	if((retVal = rtl8373_getAsicRegBits(RTL8373_I2C_MST1_CTRL1_ADDR, RTL8373_I2C_MST1_CTRL1_SDA_OUT_SEL_MASK, pSdaNum)) != RT_ERR_OK)
		return retVal;

	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_i2c_data_read
 * Description:
 *      i2c master read slave device data
 * Input: 
 *      memAddr 	- slave device memory address 
 *      dataWidth 	- want to read data width(1~16)
 *		memAddrWidth - slave device memory address width(0~3)
 * Output:
 *      pReadData 		- slave device data that has read
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  	- out of range
 * Note:
 *     
 */
rtk_api_ret_t dal_rtl8373_i2c_data_read(rtk_uint32 memAddr, rtk_uint32 dataWidth, rtk_uint32 memAddrWidth, rtk_uint32 *pReadData)
{
    rtk_uint32 retVal = 0, mask = 0, mem_Addr = 0, tmpVal = 0, regVal = 0, i2cFail = 0;
	rtk_uint32 tmpData = 0, baseAddr = 0;
	
	if(pReadData == NULL)
		return RT_ERR_NULL_POINTER;
	
	if((memAddrWidth > I2C_MEM_ADDR_WIDTH_MAX) || (dataWidth > I2C_DATA_WIDTH_MAX))
		return RT_ERR_INPUT;	

	/*set memory address*/
	mask = 1 << (8 * memAddrWidth);
	mask = mask - 1;

	mem_Addr = memAddr & mask;
	if((retVal = rtl8373_setAsicRegBits(RTL8373_I2C_MST1_MEMADDR_CTRL_ADDR, RTL8373_I2C_MST1_MEMADDR_CTRL_MEM_ADDR_MASK, mem_Addr)) != RT_ERR_OK)
		return retVal;	

	/*set dataWidth + memAddrWidth*/
	tmpVal = (memAddrWidth << 4) | (dataWidth-1);
	
	if((retVal = rtl8373_setAsicRegBits(RTL8373_I2C_MST1_CTRL1_ADDR, RTL8373_I2C_MST1_CTRL1_DATA_WIDTH_MASK|RTL8373_I2C_MST1_CTRL1_MEM_ADDR_WIDTH_MASK, tmpVal)) != RT_ERR_OK)
		return retVal;	

	/*set read operation*/	
	if((retVal = rtl8373_setAsicRegBit(RTL8373_I2C_MST1_CTRL1_ADDR, RTL8373_I2C_MST1_CTRL1_RWOP_OFFSET, I2C_RW_OP_READ)) != RT_ERR_OK)
		return retVal;	
	
	
	/*trigger read, and wait finished*/
	if((retVal = rtl8373_setAsicRegBit(RTL8373_I2C_MST1_CTRL1_ADDR, RTL8373_I2C_MST1_CTRL1_I2C_TRIG_OFFSET, I2C_RW_ACT_TRIGGER)) != RT_ERR_OK)
		return retVal;	
	do{
		if((retVal = rtl8373_getAsicRegBit(RTL8373_I2C_MST1_CTRL1_ADDR, RTL8373_I2C_MST1_CTRL1_I2C_TRIG_OFFSET, &regVal)) != RT_ERR_OK)
			return retVal;		
	}while(regVal);


	/*check i2c react fail or not*/
	if((retVal = rtl8373_getAsicRegBit(RTL8373_I2C_MST1_CTRL1_ADDR, RTL8373_I2C_MST1_CTRL1_I2C_FAIL_OFFSET, &i2cFail)) != RT_ERR_OK)
		return retVal;	

	if(i2cFail == 0)
	{
		baseAddr = RTL8373_I2C_MST1_DATA_CTRL_ADDR(0);
		if((retVal = rtl8373_getAsicReg(baseAddr,  &tmpData)) != RT_ERR_OK)
			return retVal;
		if(dataWidth <4)
		{
			mask = (1 << (8 * (dataWidth))) -1;
			*pReadData = mask & tmpData;			
		}
		else
			*pReadData = tmpData;
		
		if(dataWidth >= 5)
		{
			if((retVal = rtl8373_getAsicReg(baseAddr + 4,  &tmpData)) != RT_ERR_OK)
				return retVal;
			if(dataWidth <8) 
			{
				mask = (1 << (8 * (dataWidth-4))) -1;
				*pReadData = mask & tmpData;		

			}
			else
				*(pReadData+1) = tmpData;			
		}

		if(dataWidth >= 9)
		{
			if((retVal = rtl8373_getAsicReg(baseAddr + 8,  &tmpData)) != RT_ERR_OK)
				return retVal;
			if(dataWidth <12) 
			{
				mask = (1 << (8 * (dataWidth-8))) -1;
				*pReadData = mask & tmpData;		

			}
			else
				*(pReadData+2) = tmpData;			
		}
		if(dataWidth >= 13)
		{
			if((retVal = rtl8373_getAsicReg(baseAddr + 12,  &tmpData)) != RT_ERR_OK)
				return retVal;
			if(dataWidth <16) 
			{
				mask = (1 << (8 * (dataWidth-12))) -1;
				*pReadData = mask & tmpData;		

			}
			else
				*(pReadData+3) = tmpData;			
		}
			
	}
	else
		return RT_ERR_I2C_COMMAND_FAIL;	

	return RT_ERR_OK;	
}

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
rtk_api_ret_t dal_rtl8373_i2c_data_write(rtk_uint32 memAddr, rtk_uint32 dataWidth, rtk_uint32 memAddrWidth, rtk_uint32 *pWriteData)
{
    rtk_uint32 retVal = 0, mask = 0, mem_Addr = 0, tmpVal = 0, regVal = 0, i2cFail = 0;
	rtk_uint32 tmpData = 0, baseAddr = 0;
	
	if(pWriteData == NULL)
		return RT_ERR_NULL_POINTER;
	
	if((memAddrWidth > I2C_MEM_ADDR_WIDTH_MAX) || (dataWidth > I2C_DATA_WIDTH_MAX))
		return RT_ERR_INPUT;	

	/*set memory address*/
	mask = 1 << (8 * memAddrWidth);
	mask = mask - 1;

	mem_Addr = memAddr & mask;
	if((retVal = rtl8373_setAsicRegBits(RTL8373_I2C_MST1_MEMADDR_CTRL_ADDR, RTL8373_I2C_MST1_MEMADDR_CTRL_MEM_ADDR_MASK, mem_Addr)) != RT_ERR_OK)
		return retVal;	

	/*set dataWidth + memAddrWidth*/
	tmpVal = (memAddrWidth << 4) | (dataWidth-1);
	
	if((retVal = rtl8373_setAsicRegBits(RTL8373_I2C_MST1_CTRL1_ADDR, RTL8373_I2C_MST1_CTRL1_DATA_WIDTH_MASK|RTL8373_I2C_MST1_CTRL1_MEM_ADDR_WIDTH_MASK, tmpVal)) != RT_ERR_OK)
		return retVal;	

	/*set write operation*/	
	if((retVal = rtl8373_setAsicRegBit(RTL8373_I2C_MST1_CTRL1_ADDR, RTL8373_I2C_MST1_CTRL1_RWOP_OFFSET, I2C_RW_OP_WRITE)) != RT_ERR_OK)
		return retVal;	

	/*set data to indirect reg*/
	baseAddr = RTL8373_I2C_MST1_DATA_CTRL_ADDR(0);
	tmpData = *pWriteData;
	if((retVal = rtl8373_setAsicReg(baseAddr,  tmpData)) != RT_ERR_OK)
		return retVal;
	
	if(dataWidth > 4)
	{
		tmpData = *(pWriteData + 1);
		if((retVal = rtl8373_setAsicReg(baseAddr + 4,  tmpData)) != RT_ERR_OK)
			return retVal;
	}

	if(dataWidth > 8)
	{
		tmpData = *(pWriteData + 2);
		if((retVal = rtl8373_setAsicReg(baseAddr + 8,  tmpData)) != RT_ERR_OK)
			return retVal;
	}
	
	if(dataWidth > 12)
	{
		tmpData = *(pWriteData + 3);
		if((retVal = rtl8373_setAsicReg(baseAddr + 12,  tmpData)) != RT_ERR_OK)
			return retVal;
	}
			
	/*trigger WRITE	, and wait finished*/
	if((retVal = rtl8373_setAsicRegBit(RTL8373_I2C_MST1_CTRL1_ADDR, RTL8373_I2C_MST1_CTRL1_I2C_TRIG_OFFSET, I2C_RW_ACT_TRIGGER)) != RT_ERR_OK)
		return retVal;	
	do{
		if((retVal = rtl8373_getAsicRegBit(RTL8373_I2C_MST1_CTRL1_ADDR, RTL8373_I2C_MST1_CTRL1_I2C_TRIG_OFFSET, &regVal)) != RT_ERR_OK)
			return retVal;		
	}while(regVal);


	/*check i2c react fail or not*/
	if((retVal = rtl8373_getAsicRegBit(RTL8373_I2C_MST1_CTRL1_ADDR, RTL8373_I2C_MST1_CTRL1_I2C_FAIL_OFFSET, &i2cFail)) != RT_ERR_OK)
		return retVal;	
	if(i2cFail)
		return RT_ERR_I2C_COMMAND_FAIL;	

	return RT_ERR_OK;	
}


