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
 * Purpose : RTL8367C switch high-level API for RTL8367C
 * Feature : Shared meter related functions
 *
 */
#include <rtl8367c_asicdrv_meter.h>
/* Function Name:
 *      rtl8367c_setAsicShareMeter
 * Description:
 *      Set meter configuration
 * Input:
 *      index   - hared meter index (0-31)
 *      rate    - 17-bits rate of share meter, unit is 8Kpbs
 *      ifg     - Including IFG in rate calculation, 1:include 0:exclude
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicShareMeter(rtk_uint32 index, rtk_uint32 rate, rtk_uint32 ifg)
{
    ret_t retVal;

    if(index > RTL8367C_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    if(index < 32)
    {
    /*19-bits Rate*/
        retVal = rtl8367c_setAsicReg(RTL8367C_METER_RATE_REG(index), rate&0xFFFF);
        if(retVal != RT_ERR_OK)
            return retVal;

        retVal = rtl8367c_setAsicReg(RTL8367C_METER_RATE_REG(index) + 1, (rate &0x70000) >> 16);
        if(retVal != RT_ERR_OK)
            return retVal;

        retVal = rtl8367c_setAsicRegBit(RTL8367C_METER_IFG_CTRL_REG(index), RTL8367C_METER_IFG_OFFSET(index), ifg);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
    /*19-bits Rate*/
        retVal = rtl8367c_setAsicReg(RTL8367C_REG_METER32_RATE_CTRL0 + ((index-32) << 1), rate&0xFFFF);
        if(retVal != RT_ERR_OK)
            return retVal;

        retVal = rtl8367c_setAsicReg(RTL8367C_REG_METER32_RATE_CTRL0 + ((index-32) << 1) + 1, (rate &0x70000) >> 16);
        if(retVal != RT_ERR_OK)
            return retVal;

        retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_METER_IFG_CTRL2 + ((index-32) >> 4), RTL8367C_METER_IFG_OFFSET(index), ifg);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_getAsicShareMeter
 * Description:
 *      Get meter configuration
 * Input:
 *      index   - hared meter index (0-31)
 *      pRate   - 17-bits rate of share meter, unit is 8Kpbs
 *      pIfg    - Including IFG in rate calculation, 1:include 0:exclude
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicShareMeter(rtk_uint32 index, rtk_uint32 *pRate, rtk_uint32 *pIfg)
{
    rtk_uint32 regData;
    rtk_uint32 regData2;
    ret_t retVal;

    if(index > RTL8367C_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    if(index < 32)
    {
    /*17-bits Rate*/
     retVal = rtl8367c_getAsicReg(RTL8367C_METER_RATE_REG(index), &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

     retVal = rtl8367c_getAsicReg(RTL8367C_METER_RATE_REG(index) + 1, &regData2);
        if(retVal != RT_ERR_OK)
            return retVal;

    *pRate = ((regData2 << 16) & 0x70000) | regData;
    /*IFG*/
    retVal = rtl8367c_getAsicRegBit(RTL8367C_METER_IFG_CTRL_REG(index), RTL8367C_METER_IFG_OFFSET(index), pIfg);

    return retVal;
    }
    else
    {
    /*17-bits Rate*/
     retVal = rtl8367c_getAsicReg(RTL8367C_REG_METER32_RATE_CTRL0 + ((index-32) << 1), &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

     retVal = rtl8367c_getAsicReg(RTL8367C_REG_METER32_RATE_CTRL0 + ((index-32) << 1) + 1, &regData2);
        if(retVal != RT_ERR_OK)
            return retVal;

    *pRate = ((regData2 << 16) & 0x70000) | regData;
    /*IFG*/
    retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_METER_IFG_CTRL2 + ((index-32) >> 4), RTL8367C_METER_IFG_OFFSET(index), pIfg);

    return retVal;
    }
}
/* Function Name:
 *      rtl8367c_setAsicShareMeterBucketSize
 * Description:
 *      Set meter related leaky bucket threshold
 * Input:
 *      index       - hared meter index (0-31)
 *      lbthreshold - Leaky bucket threshold of meter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicShareMeterBucketSize(rtk_uint32 index, rtk_uint32 lbthreshold)
{

    if(index > RTL8367C_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    if(index < 32)
    return rtl8367c_setAsicReg(RTL8367C_METER_BUCKET_SIZE_REG(index), lbthreshold);
    else
       return rtl8367c_setAsicReg(RTL8367C_REG_METER32_BUCKET_SIZE + index - 32, lbthreshold);
}
/* Function Name:
 *      rtl8367c_getAsicShareMeterBucketSize
 * Description:
 *      Get meter related leaky bucket threshold
 * Input:
 *      index       - hared meter index (0-31)
 *      pLbthreshold - Leaky bucket threshold of meter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicShareMeterBucketSize(rtk_uint32 index, rtk_uint32 *pLbthreshold)
{
    if(index > RTL8367C_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    if(index < 32)
    return rtl8367c_getAsicReg(RTL8367C_METER_BUCKET_SIZE_REG(index), pLbthreshold);
    else
       return rtl8367c_getAsicReg(RTL8367C_REG_METER32_BUCKET_SIZE + index - 32, pLbthreshold);
}

/* Function Name:
 *      rtl8367c_setAsicShareMeterType
 * Description:
 *      Set meter Type
 * Input:
 *      index       - shared meter index (0-31)
 *      Type        - 0: kbps, 1: pps
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicShareMeterType(rtk_uint32 index, rtk_uint32 type)
{
    rtk_uint32 reg;

    if(index > RTL8367C_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    if(index < 32)
        reg = RTL8367C_REG_METER_MODE_SETTING0 + (index / 16);
    else
        reg = RTL8367C_REG_METER_MODE_SETTING2 + ((index - 32) / 16);
    return rtl8367c_setAsicRegBit(reg, index % 16, type);
}

/* Function Name:
 *      rtl8367c_getAsicShareMeterType
 * Description:
 *      Get meter Type
 * Input:
 *      index       - shared meter index (0-31)
 * Output:
 *      pType       - 0: kbps, 1: pps
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicShareMeterType(rtk_uint32 index, rtk_uint32 *pType)
{
    rtk_uint32 reg;

    if(index > RTL8367C_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    if(NULL == pType)
        return RT_ERR_NULL_POINTER;

    if(index < 32)
        reg = RTL8367C_REG_METER_MODE_SETTING0 + (index / 16);
    else
        reg = RTL8367C_REG_METER_MODE_SETTING2 + ((index - 32) / 16);
    return rtl8367c_getAsicRegBit(reg, index % 16, pType);
}


/* Function Name:
 *      rtl8367c_setAsicMeterExceedStatus
 * Description:
 *      Clear shared meter status
 * Input:
 *      index       - hared meter index (0-31)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicMeterExceedStatus(rtk_uint32 index)
{
    if(index > RTL8367C_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    if(index < 32)
        return rtl8367c_setAsicRegBit(RTL8367C_METER_OVERRATE_INDICATOR_REG(index), RTL8367C_METER_EXCEED_OFFSET(index), 1);
    else
        return rtl8367c_setAsicRegBit(RTL8367C_REG_METER_OVERRATE_INDICATOR2 + ((index - 32) >> 4), RTL8367C_METER_EXCEED_OFFSET(index), 1);

}
/* Function Name:
 *      rtl8367c_getAsicMeterExceedStatus
 * Description:
 *      Get shared meter status
 * Input:
 *      index   - hared meter index (0-31)
 *      pStatus     - 0: rate doesn't exceed    1: rate exceeds
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      If rate is over rate*8Kbps of a meter, the state bit of this meter is set to 1.
 */
ret_t rtl8367c_getAsicMeterExceedStatus(rtk_uint32 index, rtk_uint32* pStatus)
{
    if(index > RTL8367C_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    if(index < 32)
        return rtl8367c_getAsicRegBit(RTL8367C_METER_OVERRATE_INDICATOR_REG(index), RTL8367C_METER_EXCEED_OFFSET(index), pStatus);
    else
        return rtl8367c_getAsicRegBit(RTL8367C_REG_METER_OVERRATE_INDICATOR2 + ((index - 32) >> 4), RTL8367C_METER_EXCEED_OFFSET(index), pStatus);
}

