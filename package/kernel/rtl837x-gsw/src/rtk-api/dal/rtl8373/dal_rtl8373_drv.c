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
 * Feature : Here is a list of all functions and variables in QoS module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal_rtl8373_drv.h>
#include <string.h>
#include <rtl8373_asicdrv.h>
#include <phy_rtl8224.h>
/* Function Name:
 *      dal_rtl8373_mdc_en
 * Description:
 *      Enbale MDC function.
 * Input:
 *      enable  - enable 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will enbale MDC function.
 */

rtk_api_ret_t dal_rtl8373_mdc_en(rtk_uint32 enable)
{
    rtk_api_ret_t retVal;

    
    if ((retVal = rtl8373_setAsicRegBit(RTL8373_SMI_CTRL_ADDR, RTL8373_SMI_CTRL_SMI0_MDC_EN_OFFSET, enable)) != RT_ERR_OK)
                return retVal;

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_SMI_CTRL_ADDR, RTL8373_SMI_CTRL_SMI1_MDC_EN_OFFSET, enable)) != RT_ERR_OK)
                return retVal;

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_SMI_CTRL_ADDR, RTL8373_SMI_CTRL_SMI2_MDC_EN_OFFSET, enable)) != RT_ERR_OK)
                return retVal;
      
	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_phy_write
 * Description:
 *      Configure phy register data.
 * Input:
 *      phy_mask  - phy mask, bit[0:9]
 *      dev_addr   - device address
 *      reg_addr   - register address
 *      indata       - input data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */

rtk_api_ret_t dal_rtl8373_phy_write(rtk_uint32 phy_mask, rtk_uint32 dev_addr, rtk_uint32 reg_addr, rtk_uint32 indata)
{
       rtk_api_ret_t retVal;
       rtk_uint32 tmp;
       rtk_uint32 tmp_cmd,tmp_res;
       rtk_uint32 pollcnt = 0;

       if ((retVal = rtl8373_setAsicReg(RTL8373_SMI_ACCESS_PHY_CTRL_0_ADDR, phy_mask)) != RT_ERR_OK)
                   return retVal;

       if ((retVal = rtl8373_setAsicRegBits(RTL8373_SMI_ACCESS_PHY_CTRL_3_ADDR, RTL8373_SMI_ACCESS_PHY_CTRL_3_INDATA_15_0_MASK, indata)) != RT_ERR_OK)
                   return retVal;

       tmp = (dev_addr << 19) | (reg_addr << 3) | 0x7;

       if ((retVal = rtl8373_setAsicReg(RTL8373_SMI_ACCESS_PHY_CTRL_1_ADDR, tmp)) != RT_ERR_OK)
                   return retVal;

       for(pollcnt = 0; pollcnt < RTL8373_MAX_POLLCNT; pollcnt++)
       {
              if ((retVal = rtl8373_getAsicRegBit(RTL8373_SMI_ACCESS_PHY_CTRL_1_ADDR, RTL8373_SMI_ACCESS_PHY_CTRL_1_CMD_OFFSET, &tmp_cmd)) != RT_ERR_OK)
                          return retVal;

              if ((retVal = rtl8373_getAsicRegBits(RTL8373_SMI_ACCESS_PHY_CTRL_1_ADDR, RTL8373_SMI_ACCESS_PHY_CTRL_1_FAIL_MASK, &tmp_res)) != RT_ERR_OK)
                          return retVal;

              if((tmp_cmd == 0) && (tmp_res == 0))
                     break;
       }

         if(pollcnt == RTL8373_MAX_POLLCNT)
            {
               return RT_ERR_BUSYWAIT_TIMEOUT;
            }

        return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_phy_read
 * Description:
 *      get phy register data.
 * Input:
 *      phy_id  - phy id
 *      dev_addr   - device address
 *      reg_addr   - register address
 * Output:
*      pdata       - phy data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get phy register data.
 */

rtk_api_ret_t dal_rtl8373_phy_read(rtk_uint32 phy_id, rtk_uint32 dev_addr, rtk_uint32 reg_addr, rtk_uint32 *pdata)
{
       rtk_api_ret_t retVal;
       rtk_uint32 tmp=0;
       rtk_uint32 tmp_cmd,tmp_res;
       rtk_uint32 pollcnt = 0;

       if ((retVal = rtl8373_setAsicRegBits(RTL8373_SMI_ACCESS_PHY_CTRL_3_ADDR, RTL8373_SMI_ACCESS_PHY_CTRL_3_INDATA_15_0_MASK, phy_id)) != RT_ERR_OK)
                   return retVal;

       tmp = (dev_addr << 19) | (reg_addr << 3) | (0 << 2) | 0x3;

       if ((retVal = rtl8373_setAsicReg(RTL8373_SMI_ACCESS_PHY_CTRL_1_ADDR, tmp)) != RT_ERR_OK)
                   return retVal;

       for(pollcnt = 0; pollcnt < RTL8373_MAX_POLLCNT; pollcnt++)
       {
              if ((retVal = rtl8373_getAsicRegBit(RTL8373_SMI_ACCESS_PHY_CTRL_1_ADDR, RTL8373_SMI_ACCESS_PHY_CTRL_1_CMD_OFFSET, &tmp_cmd)) != RT_ERR_OK)
                          return retVal;

              if ((retVal = rtl8373_getAsicRegBits(RTL8373_SMI_ACCESS_PHY_CTRL_1_ADDR, RTL8373_SMI_ACCESS_PHY_CTRL_1_FAIL_MASK, &tmp_res)) != RT_ERR_OK)
                          return retVal;

              if((tmp_cmd == 0) && (tmp_res == 0))
                     break;
       }

        if(pollcnt == RTL8373_MAX_POLLCNT)
            {
               return RT_ERR_BUSYWAIT_TIMEOUT;
            }

        if ((retVal = rtl8373_getAsicRegBits(RTL8373_SMI_ACCESS_PHY_CTRL_2_ADDR, RTL8373_SMI_ACCESS_PHY_CTRL_2_DATA_15_0_MASK, pdata)) != RT_ERR_OK)
                          return retVal;

        return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_phy_readBits
 * Description:
 *      Configure phy register data.
 * Input:
 *      phy_id - phy id
 *      dev_addr   - device address
 *      reg_addr   - register address
 *      indata       - input data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */
 
rtk_api_ret_t dal_rtl8373_phy_readBits(rtk_uint32 phy_id, rtk_uint32 dev_addr, rtk_uint32 reg_addr,  rtk_uint32 bitsMask,rtk_uint32 *pdata)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData,regDatal,regDatah;
    rtk_uint32 bitsShift;
    
   if( !bitsMask )
        return RT_ERR_INPUT;

    bitsShift = 0;
    while(!(bitsMask & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8373_PHYBITLENGTH)
            return RT_ERR_INPUT;
    }

     if(dev_addr==RTL8224TOPDEVAD)
    {
          retVal = dal_rtl8373_phy_read(phy_id,dev_addr, reg_addr,&regDatal);
          if(retVal != RT_ERR_OK)
             return RT_ERR_SMI;
          retVal = dal_rtl8373_phy_read(phy_id,dev_addr, reg_addr+1,&regDatah);
          if(retVal != RT_ERR_OK)
             return RT_ERR_SMI;

          regData=regDatal|(regDatah<<16);
    }
    else
    {
        retVal = dal_rtl8373_phy_read(phy_id,dev_addr, reg_addr,&regData);
        if(retVal != RT_ERR_OK)
            return RT_ERR_SMI;
    }
    *pdata = (regData & bitsMask) >> bitsShift;
    return retVal;
}

/* Function Name:
 *      rtk_port_phyReg_setBits
 * Description:
 *      Configure phy register data.
 * Input:
 *      phy_mask  - phy mask, bit[0:9]
 *      dev_addr   - device address
 *      reg_addr   - register address
 *      indata       - input data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */
 
rtk_api_ret_t dal_rtl8373_phy_writeBits(rtk_uint32 phy_mask, rtk_uint32 dev_addr, rtk_uint32 reg_addr,  rtk_uint32 bitsMask,rtk_uint32 indata)
{
   rtk_uint32 regDatah,regDatal,regData;
    rtk_api_ret_t retVal;
    rtk_uint32 bitsShift;
    rtk_uint32 valueShifted;
    rtk_uint32 phy_id;

    if( !bitsMask )
        return RT_ERR_INPUT;

    bitsShift = 0;
    while(!(bitsMask & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8373_REGBITLENGTH)
            return RT_ERR_INPUT;
    }
    valueShifted = indata << bitsShift;

    if(valueShifted > RTL8373_REGDATAMAX)
        return RT_ERR_INPUT;
    
    phy_id =0;
    if(dev_addr==RTL8224TOPDEVAD)
    {
        for(phy_id=0;phy_id<RTL8373_PORTIDMAX;phy_id++)
        {
            if((1<<phy_id)&phy_mask)
            {
                  retVal = dal_rtl8373_phy_read(phy_id,dev_addr, reg_addr+1,&regDatah);
             if(retVal != RT_ERR_OK)
                return RT_ERR_SMI;
                  retVal = dal_rtl8373_phy_read(phy_id,dev_addr, reg_addr,&regDatal);
             if(retVal != RT_ERR_OK)
                return RT_ERR_SMI;

            regData = regDatal|(regDatah<<16);
            regData = regData & (~bitsMask);
            regData = regData | (valueShifted & bitsMask);

            regDatah= regData>>16;
            regDatal = regData&0xffff;
            retVal = dal_rtl8373_phy_write(1<<phy_id,dev_addr, reg_addr+1,regDatah);
            if(retVal != RT_ERR_OK)
                return RT_ERR_SMI;
           retVal = dal_rtl8373_phy_write(1<<phy_id,dev_addr, reg_addr,regDatal);
            if(retVal != RT_ERR_OK)
                return RT_ERR_SMI;
            }
        }
    }
    else
    {
        for(phy_id=0;phy_id<RTL8373_PORTIDMAX;phy_id++)
        {
            if((1<<phy_id)&phy_mask)
            {
                  retVal = dal_rtl8373_phy_read(phy_id,dev_addr, reg_addr,&regData);
             if(retVal != RT_ERR_OK)
                return RT_ERR_SMI;
           
            regData = regData & (~bitsMask);
            regData = regData | (valueShifted & bitsMask);

            retVal = dal_rtl8373_phy_write(1<<phy_id,dev_addr, reg_addr,regData);
            if(retVal != RT_ERR_OK)
                return RT_ERR_SMI;
            }
        }
    }
      
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8224_top_reg_write
 * Description:
 *      Set RTL8224 top register value.
 * Input:
 *      top_reg_addr   - top register address
 *      value               - register value
 * Output:
*      pdata       - None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will set phy register data.
 */

rtk_api_ret_t  dal_rtl8224_top_reg_write(rtk_uint32 top_reg_addr,  rtk_uint32 value)
{
    rtk_uint32 phymask;
    rtk_api_ret_t retVal;
    rtk_uint32 lowdata, highdata;

    phymask=1;

    lowdata = value & 0xffff;
    highdata = (value >> 16) & 0xffff;
    if ((retVal = dal_rtl8373_phy_write(phymask,30,top_reg_addr,lowdata)) != RT_ERR_OK)
        return retVal;

    if ((retVal = dal_rtl8373_phy_write(phymask,30,top_reg_addr+1,highdata)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8224_top_reg_read
 * Description:
 *      Get RTL8224 top register value.
 * Input:
 *      top_reg_addr   - top register address
 *      pvalue               - register value
 * Output:
*      pdata       - None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get phy register data.
 */

rtk_api_ret_t  dal_rtl8224_top_reg_read(rtk_uint32 top_reg_addr,  rtk_uint32* pvalue)
{
    rtk_uint32 phyid;
    rtk_uint32 lowdata, highdata;
    rtk_api_ret_t retVal;

    phyid=0;
    if ((retVal = dal_rtl8373_phy_read(phyid,30,top_reg_addr,&lowdata)) != RT_ERR_OK)
        return retVal;

    if ((retVal = dal_rtl8373_phy_read(phyid,30,top_reg_addr+1,&highdata)) != RT_ERR_OK)
        return retVal;

    *pvalue = (lowdata & 0xffff) | ((highdata & 0xffff) << 16);

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8224_top_regbit_write
 * Description:
 *      Set RTL8224 top register bit value.
 * Input:
 *      top_reg_addr   - top register address
 *      offet                - bit offset
 *      value               - register bit value
 * Output:
*      pdata       - None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will set phy register data.
 */

rtk_api_ret_t  dal_rtl8224_top_regbit_write(rtk_uint32 top_reg_addr,  rtk_uint32 offset, rtk_uint32 value)
{
    rtk_uint32 phyid;
    rtk_uint32 phymask;
    rtk_uint32 regdata;
    rtk_api_ret_t retVal;

    phyid=0;

    if(offset < 16)
    {
        if ((retVal = dal_rtl8373_phy_read(phyid,30,top_reg_addr, &regdata)) != RT_ERR_OK)
            return retVal;
        if(1 == value)
            regdata |= (1 << offset);
        else
            regdata &= ~(1 << offset);

        phymask = 1;
        return dal_rtl8373_phy_write(phymask, 30, top_reg_addr, regdata);

    }
    else
    {
        offset = offset - 16;

        if ((retVal = dal_rtl8373_phy_read(phyid,30,top_reg_addr+1, &regdata)) != RT_ERR_OK)
            return retVal;

        if(1 == value)
            regdata |= (1 << offset);
        else
            regdata &= ~(1 << offset);

        phymask = 1;
        return dal_rtl8373_phy_write(phymask, 30, top_reg_addr+1, regdata);
    }
}


/* Function Name:
 *      dal_rtl8224_top_regbit_read
 * Description:
 *      Get RTL8224 top register bit value.
 * Input:
 *      top_reg_addr   - top register address
 *      offet                - bit offset
 *      value               - register bit value
 * Output:
*      pdata       - None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get phy register data.
 */

rtk_api_ret_t  dal_rtl8224_top_regbit_read(rtk_uint32 top_reg_addr,  rtk_uint32 offset, rtk_uint32* pvalue)
{
    rtk_uint32 phyid;
    rtk_uint32 regdata;
    rtk_api_ret_t retVal;

    phyid=0;

    if(offset < 16)
    {
        if ((retVal = dal_rtl8373_phy_read(phyid,30,top_reg_addr, &regdata)) != RT_ERR_OK)
            return retVal;

        *pvalue = (regdata >> offset) & 1;
    }
    else
    {
        offset =  offset - 16;
        if ((retVal = dal_rtl8373_phy_read(phyid,30,top_reg_addr+1, &regdata)) != RT_ERR_OK)
            return retVal;

        *pvalue = (regdata >> offset) & 1;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8224_top_regbits_write
 * Description:
 *      Set RTL8224 top register bits value.
 * Input:
 *      top_reg_addr   - top register address
 *      bitmask          - bit mask
 *      value               - register bits value
 * Output:
*      pdata       - None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will set phy register data.
 */

rtk_api_ret_t  dal_rtl8224_top_regbits_write(rtk_uint32 top_reg_addr,  rtk_uint32 bitmask, rtk_uint32 value)
{
    rtk_uint32 phymask;
    rtk_uint32 phyid;
    rtk_uint32 regdata;
    rtk_api_ret_t retVal;
    rtk_uint32 bitsShift;
    rtk_uint32 valueShifted;

    if( !bitmask )
        return RT_ERR_INPUT;

    bitsShift = 0;
    while(!(bitmask & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8373_REGBITLENGTH)
            return RT_ERR_INPUT;
    }

    valueShifted = value << bitsShift;
    if(valueShifted > RTL8373_REGDATAMAX)
        return RT_ERR_INPUT;

    if(bitsShift < 16)
    {
        phyid=0;
        if ((retVal = dal_rtl8373_phy_read(phyid,30,top_reg_addr, &regdata)) != RT_ERR_OK)
            return retVal;

        regdata = regdata & (~bitmask);
        regdata = regdata | (valueShifted & bitmask);
        
        phymask = 1;
        return dal_rtl8373_phy_write(phymask, 30, top_reg_addr, regdata);
    }
    else
    {
        phyid=0;
        if ((retVal = dal_rtl8373_phy_read(phyid,30,top_reg_addr+1, &regdata)) != RT_ERR_OK)
            return retVal;

        bitmask = bitmask >> 16;
        valueShifted = valueShifted >> 16;
        regdata = regdata & (~bitmask);
        regdata = regdata | (valueShifted & bitmask);
        
        phymask = 1;
        return dal_rtl8373_phy_write(phymask, 30, top_reg_addr+1, regdata);
    }
    
}



/* Function Name:
 *      dal_rtl8224_top_regbits_read
 * Description:
 *      Get RTL8224 top register bits value.
 * Input:
 *      top_reg_addr   - top register address
 *      bitmask          - bit mask
 *      value               - register bits value
 * Output:
*      pdata       - None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get phy register data.
 */

rtk_api_ret_t  dal_rtl8224_top_regbits_read(rtk_uint32 top_reg_addr,  rtk_uint32 bitmask, rtk_uint32* pvalue)
{
    rtk_uint32 phyid;
    rtk_uint32 regdata;
    rtk_api_ret_t retVal;
    rtk_uint32 bitsShift;

    if( !bitmask )
        return RT_ERR_INPUT;

    bitsShift = 0;
    while(!(bitmask & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8373_REGBITLENGTH)
            return RT_ERR_INPUT;
    }

    phyid=0;

    if(bitsShift < 16)
    {
        if ((retVal = dal_rtl8373_phy_read(phyid,30,top_reg_addr, &regdata)) != RT_ERR_OK)
            return retVal;

        *pvalue = (regdata & bitmask) >> bitsShift;
    }
    else
    {
        if ((retVal = dal_rtl8373_phy_read(phyid,30,top_reg_addr+1, &regdata)) != RT_ERR_OK)
            return retVal;

        bitsShift = bitsShift - 16;
        bitmask = bitmask >> 16;
        *pvalue = (regdata & bitmask) >> bitsShift;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_sds_reg_write
 * Description:
 *      Configure phy register data.
 * Input:
 *      sds_index  - sds index 0 ~ 1
 *      sds_page   - page
 *      sds_geg   - register address
 *      regdata       - input data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */

rtk_api_ret_t dal_rtl8373_sds_reg_write(rtk_uint32 sds_index, rtk_uint32 sds_page, rtk_uint32 sds_reg, rtk_uint32 regdata)
{
    rtk_api_ret_t retVal;
    rtk_uint32 tmp_cmd;
    rtk_uint32 pollcnt = 0;

    for(pollcnt = 0; pollcnt < RTL8373_MAX_POLLCNT; pollcnt++)
    {
           if ((retVal = rtl8373_getAsicRegBit(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_CMD_OFFSET, &tmp_cmd)) != RT_ERR_OK)
                       return retVal;

           if(tmp_cmd == 0) 
                  break;
    }

    if(pollcnt == RTL8373_MAX_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;

    if ((retVal = rtl8373_setAsicReg(RTL8373_SDS_INDACS_WD_ADDR, regdata)) != RT_ERR_OK)
                return retVal;
    

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_INDEX_OFFSET, sds_index)) != RT_ERR_OK)
                return retVal;


    if ((retVal = rtl8373_setAsicRegBits(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_PAGE_MASK, sds_page)) != RT_ERR_OK)
                return retVal;


    if ((retVal = rtl8373_setAsicRegBits(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_REGAD_MASK, sds_reg)) != RT_ERR_OK)
                return retVal;


    if ((retVal = rtl8373_setAsicRegBit(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_RWOP_OFFSET, 1)) != RT_ERR_OK)
                return retVal;

    
    if ((retVal = rtl8373_setAsicRegBit(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_CMD_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;


    for(pollcnt = 0; pollcnt < RTL8373_MAX_POLLCNT; pollcnt++)
    {
           if ((retVal = rtl8373_getAsicRegBit(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_CMD_OFFSET, &tmp_cmd)) != RT_ERR_OK)
                       return retVal;

           if(tmp_cmd == 0) 
                  break;
    }

    if(pollcnt == RTL8373_MAX_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;

     return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_sds_reg_read
 * Description:
 *      Configure phy register data.
 * Input:
 *      sds_index  - sds index 0 ~ 1
 *      sds_page   - page
 *      sds_geg   - register address
 *      pdata       - output data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */

rtk_api_ret_t dal_rtl8373_sds_reg_read(rtk_uint32 sds_index, rtk_uint32 sds_page, rtk_uint32 sds_reg, rtk_uint32 * pdata)
{
    rtk_api_ret_t retVal;
    rtk_uint32 tmp_cmd;
    rtk_uint32 pollcnt = 0;

    for(pollcnt = 0; pollcnt < RTL8373_MAX_POLLCNT; pollcnt++)
    {
           if ((retVal = rtl8373_getAsicRegBit(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_CMD_OFFSET, &tmp_cmd)) != RT_ERR_OK)
                       return retVal;

           if(tmp_cmd == 0) 
                  break;
    }

    if(pollcnt == RTL8373_MAX_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;

    

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_INDEX_OFFSET, sds_index)) != RT_ERR_OK)
                return retVal;


    if ((retVal = rtl8373_setAsicRegBits(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_PAGE_MASK, sds_page)) != RT_ERR_OK)
                return retVal;


    if ((retVal = rtl8373_setAsicRegBits(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_REGAD_MASK, sds_reg)) != RT_ERR_OK)
                return retVal;


    if ((retVal = rtl8373_setAsicRegBit(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_RWOP_OFFSET, 0)) != RT_ERR_OK)
                return retVal;

    
    if ((retVal = rtl8373_setAsicRegBit(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_CMD_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;


    for(pollcnt = 0; pollcnt < RTL8373_MAX_POLLCNT; pollcnt++)
    {
           if ((retVal = rtl8373_getAsicRegBit(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_CMD_OFFSET, &tmp_cmd)) != RT_ERR_OK)
                       return retVal;

           if(tmp_cmd == 0) 
                  break;
    }

    if ((retVal = rtl8373_getAsicReg(RTL8373_SDS_INDACS_RD_ADDR, pdata)) != RT_ERR_OK)
                return retVal;
    

    if(pollcnt == RTL8373_MAX_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;

     return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_sds_regbits_write
 * Description:
 *      Configure phy register data.
 * Input:
 *      sds_index  - sds index 0 ~ 1
 *      sds_page   - page
 *      sds_geg   - register address
 *      bitmask   - bits mask
 *      value       - input data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */

rtk_api_ret_t dal_rtl8373_sds_regbits_write(rtk_uint32 sds_index, rtk_uint32 sds_page, rtk_uint32 sds_reg, rtk_uint32 bitmask, rtk_uint32 value)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regdata;
    rtk_uint32 bitsShift;
    rtk_uint32 valueShifted;


    if( !bitmask )
        return RT_ERR_INPUT;

    bitsShift = 0;
    while(!(bitmask & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8373_REGBITLENGTH)
            return RT_ERR_INPUT;
    }

    valueShifted = value << bitsShift;
    if(valueShifted > RTL8373_REGDATAMAX)
        return RT_ERR_INPUT;
    

    if ((retVal = dal_rtl8373_sds_reg_read(sds_index,sds_page,sds_reg,&regdata)) != RT_ERR_OK)
            return retVal;

    regdata = regdata & (~bitmask);
    regdata = regdata | (valueShifted & bitmask);
   
    if ((retVal = dal_rtl8373_sds_reg_write(sds_index,sds_page,sds_reg,regdata)) != RT_ERR_OK)
            return retVal;
    

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_sds_regbis_read
 * Description:
 *      Configure phy register data.
 * Input:
 *      sds_index  - sds index 0 ~ 1
 *      sds_page   - page
 *      sds_geg   - register address
 *      pvalue       - output data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */

rtk_api_ret_t dal_rtl8373_sds_regbits_read(rtk_uint32 sds_index, rtk_uint32 sds_page, rtk_uint32 sds_reg, rtk_uint32 bitmask, rtk_uint32 * pvalue)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regdata;
    rtk_uint32 bitsShift;

    if( !bitmask )
        return RT_ERR_INPUT;

    bitsShift = 0;
    while(!(bitmask & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8373_REGBITLENGTH)
            return RT_ERR_INPUT;
    }


    if ((retVal = dal_rtl8373_sds_reg_read(sds_index,sds_page,sds_reg, &regdata)) != RT_ERR_OK)
        return retVal;

    *pvalue = (regdata & bitmask) >> bitsShift;


     return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8224_sdsreg_write
 * Description:
 *      Configure phy register data.
 * Input:
 *      sds_index  - sds index 0 ~ 1
 *      sds_page   - page
 *      sds_geg   - register address
 *      regdata       - input data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */

rtk_api_ret_t dal_rtl8224_sds_reg_write(rtk_uint32 sds_index, rtk_uint32 sds_page, rtk_uint32 sds_reg, rtk_uint32 regdata)
{
    rtk_api_ret_t retVal;
    rtk_uint32 tmp_cmd;
    rtk_uint32 pollcnt = 0;

    for(pollcnt = 0; pollcnt < RTL8373_MAX_POLLCNT; pollcnt++)
    {
           if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_CMD_OFFSET, &tmp_cmd)) != RT_ERR_OK)
                       return retVal;

           if(tmp_cmd == 0) 
                  break;
    }

    if(pollcnt == RTL8373_MAX_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;

    if ((retVal = dal_rtl8224_top_reg_write(RTL8373_SDS_INDACS_WD_ADDR, regdata)) != RT_ERR_OK)
                return retVal;
    

    if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_INDEX_OFFSET, sds_index)) != RT_ERR_OK)
                return retVal;


    if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_PAGE_MASK, sds_page)) != RT_ERR_OK)
                return retVal;


    if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_REGAD_MASK, sds_reg)) != RT_ERR_OK)
                return retVal;


    if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_RWOP_OFFSET, 1)) != RT_ERR_OK)
                return retVal;

    
    if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_CMD_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;


    for(pollcnt = 0; pollcnt < RTL8373_MAX_POLLCNT; pollcnt++)
    {
           if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_CMD_OFFSET, &tmp_cmd)) != RT_ERR_OK)
                       return retVal;

           if(tmp_cmd == 0) 
                  break;
    }

    if(pollcnt == RTL8373_MAX_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;

     return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8224_sdsreg_read
 * Description:
 *      Configure phy register data.
 * Input:
 *      sds_index  - sds index 0 ~ 1
 *      sds_page   - page
 *      sds_geg   - register address
 *      pdata       - output data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */

rtk_api_ret_t dal_rtl8224_sds_reg_read(rtk_uint32 sds_index, rtk_uint32 sds_page, rtk_uint32 sds_reg, rtk_uint32 * pdata)
{
    rtk_api_ret_t retVal;
    rtk_uint32 tmp_cmd;
    rtk_uint32 pollcnt = 0;

    for(pollcnt = 0; pollcnt < RTL8373_MAX_POLLCNT; pollcnt++)
    {
           if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_CMD_OFFSET, &tmp_cmd)) != RT_ERR_OK)
                       return retVal;

           if(tmp_cmd == 0) 
                  break;
    }

    if(pollcnt == RTL8373_MAX_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;

    

    if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_INDEX_OFFSET, sds_index)) != RT_ERR_OK)
                return retVal;


    if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_PAGE_MASK, sds_page)) != RT_ERR_OK)
                return retVal;


    if ((retVal = dal_rtl8224_top_regbits_write(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_REGAD_MASK, sds_reg)) != RT_ERR_OK)
                return retVal;


    if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_RWOP_OFFSET, 0)) != RT_ERR_OK)
                return retVal;

    
    if ((retVal = dal_rtl8224_top_regbit_write(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_CMD_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;


    for(pollcnt = 0; pollcnt < RTL8373_MAX_POLLCNT; pollcnt++)
    {
           if ((retVal = dal_rtl8224_top_regbit_read(RTL8373_SDS_INDACS_CMD_ADDR, RTL8373_SDS_INDACS_CMD_SDS_CMD_OFFSET, &tmp_cmd)) != RT_ERR_OK)
                       return retVal;

           if(tmp_cmd == 0) 
                  break;
    }

    if ((retVal = dal_rtl8224_top_reg_read(RTL8373_SDS_INDACS_RD_ADDR, pdata)) != RT_ERR_OK)
                return retVal;
    

    if(pollcnt == RTL8373_MAX_POLLCNT)
            return RT_ERR_BUSYWAIT_TIMEOUT;

     return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8224_sds_regbits_write
 * Description:
 *      Configure phy register data.
 * Input:
 *      sds_index  - sds index 0 ~ 1
 *      sds_page   - page
 *      sds_geg   - register address
 *      bitmask   - bits mask
 *      value       - input data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */

rtk_api_ret_t dal_rtl8224_sds_regbits_write(rtk_uint32 sds_index, rtk_uint32 sds_page, rtk_uint32 sds_reg, rtk_uint32 bitmask, rtk_uint32 value)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regdata;
    rtk_uint32 bitsShift;
    rtk_uint32 valueShifted;


    if( !bitmask )
        return RT_ERR_INPUT;

    bitsShift = 0;
    while(!(bitmask & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8373_REGBITLENGTH)
            return RT_ERR_INPUT;
    }

    valueShifted = value << bitsShift;
    if(valueShifted > RTL8373_REGDATAMAX)
        return RT_ERR_INPUT;
    

    if ((retVal = dal_rtl8224_sds_reg_read(sds_index,sds_page,sds_reg,&regdata)) != RT_ERR_OK)
            return retVal;

    regdata = regdata & (~bitmask);
    regdata = regdata | (valueShifted & bitmask);
   
    if ((retVal = dal_rtl8224_sds_reg_write(sds_index,sds_page,sds_reg,regdata)) != RT_ERR_OK)
            return retVal;
    

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8224_sds_regbis_read
 * Description:
 *      Configure phy register data.
 * Input:
 *      sds_index  - sds index 0 ~ 1
 *      sds_page   - page
 *      sds_geg   - register address
 *      pvalue       - output data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */

rtk_api_ret_t dal_rtl8224_sds_regbits_read(rtk_uint32 sds_index, rtk_uint32 sds_page, rtk_uint32 sds_reg, rtk_uint32 bitmask, rtk_uint32 * pvalue)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regdata;
    rtk_uint32 bitsShift;

    if( !bitmask )
        return RT_ERR_INPUT;

    bitsShift = 0;
    while(!(bitmask & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8373_REGBITLENGTH)
            return RT_ERR_INPUT;
    }


    if ((retVal = dal_rtl8224_sds_reg_read(sds_index,sds_page,sds_reg, &regdata)) != RT_ERR_OK)
        return retVal;

    *pvalue = (regdata & bitmask) >> bitsShift;


     return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_phy_regbits_write
 * Description:
 *      Configure phy register data.
 * Input:
 *      phy_mask  - phy mask, bit[0:9]
 *      dev_addr   - device address
 *      reg_addr   - register address
 *      bitmask    - bits mask
 *      indata       - input data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */

rtk_api_ret_t dal_rtl8373_phy_regbits_write(rtk_uint32 phy_mask, rtk_uint32 dev_addr, rtk_uint32 reg_addr, rtk_uint32 bitmask, rtk_uint32 value)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regdata;
    rtk_uint32 port;
    rtk_uint32 bitsShift;
    rtk_uint32 valueShifted;


    if( !bitmask )
        return RT_ERR_INPUT;

    bitsShift = 0;
    while(!(bitmask & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8373_REGBITLENGTH)
            return RT_ERR_INPUT;
    }

    valueShifted = value << bitsShift;
    if(valueShifted > RTL8373_REGDATAMAX)
        return RT_ERR_INPUT;

    for(port = 0; port < 9; port++)
    {
        if((1 << port) & phy_mask)
        {
            if ((retVal = dal_rtl8373_phy_read(port,dev_addr,reg_addr,&regdata)) != RT_ERR_OK)
                return retVal;

            regdata = regdata & (~bitmask);
            regdata = regdata | (valueShifted & bitmask);

            if ((retVal = dal_rtl8373_phy_write(phy_mask,dev_addr,reg_addr,regdata)) != RT_ERR_OK)
                return retVal;
        }
    }

    return RT_ERR_OK;

}


/* Function Name:
 *      dal_rtl8373_phy_regbits_read
 * Description:
 *      get phy register data.
 * Input:
 *      phy_id  - phy id
 *      dev_addr   - device address
 *      reg_addr   - register address
 *      bitmask    - bits mask
 * Output:
 *      pdata       - phy data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get phy register data.
 */

rtk_api_ret_t dal_rtl8373_phy_regbits_read(rtk_uint32 phy_id, rtk_uint32 dev_addr, rtk_uint32 reg_addr, rtk_uint32 bitmask, rtk_uint32 * pvalue)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regdata;
    rtk_uint32 bitsShift;

    if( !bitmask )
        return RT_ERR_INPUT;

    bitsShift = 0;
    while(!(bitmask & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8373_REGBITLENGTH)
            return RT_ERR_INPUT;
    }


    if ((retVal = dal_rtl8373_phy_read(phy_id,dev_addr,reg_addr, &regdata)) != RT_ERR_OK)
        return retVal;

    *pvalue = (regdata & bitmask) >> bitsShift;


     return RT_ERR_OK;
}




