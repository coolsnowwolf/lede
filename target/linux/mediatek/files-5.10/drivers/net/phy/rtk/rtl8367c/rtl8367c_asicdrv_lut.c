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
 * $Date: 2017-03-08 15:13:58 +0800 (閫变笁, 08 涓夋湀 2017) $
 *
 * Purpose : RTL8367C switch high-level API for RTL8367C
 * Feature : LUT related functions
 *
 */

#include <rtl8367c_asicdrv_lut.h>

#include <string.h>

static void _rtl8367c_fdbStUser2Smi( rtl8367c_luttb *pLutSt, rtk_uint16 *pFdbSmi)
{
    /* L3 lookup */
    if(pLutSt->l3lookup)
    {
        if(pLutSt->l3vidlookup)
        {
            pFdbSmi[0] = (pLutSt->sip & 0x0000FFFF);
            pFdbSmi[1] = (pLutSt->sip & 0xFFFF0000) >> 16;

            pFdbSmi[2] = (pLutSt->dip & 0x0000FFFF);
            pFdbSmi[3] = (pLutSt->dip & 0x0FFF0000) >> 16;

            pFdbSmi[3] |= (pLutSt->l3lookup & 0x0001) << 12;
            pFdbSmi[3] |= (pLutSt->l3vidlookup & 0x0001) << 13;
            pFdbSmi[3] |= ((pLutSt->mbr & 0x0300) >> 8) << 14;

            pFdbSmi[4] |= (pLutSt->mbr & 0x00FF);
            pFdbSmi[4] |= (pLutSt->l3_vid & 0x00FF) << 8;

            pFdbSmi[5] |= ((pLutSt->l3_vid & 0x0F00) >> 8);
            pFdbSmi[5] |= (pLutSt->nosalearn & 0x0001) << 5;
            pFdbSmi[5] |= ((pLutSt->mbr & 0x0400) >> 10) << 7;
        }
        else
        {
            pFdbSmi[0] = (pLutSt->sip & 0x0000FFFF);
            pFdbSmi[1] = (pLutSt->sip & 0xFFFF0000) >> 16;

            pFdbSmi[2] = (pLutSt->dip & 0x0000FFFF);
            pFdbSmi[3] = (pLutSt->dip & 0x0FFF0000) >> 16;

            pFdbSmi[3] |= (pLutSt->l3lookup & 0x0001) << 12;
            pFdbSmi[3] |= (pLutSt->l3vidlookup & 0x0001) << 13;
            pFdbSmi[3] |= ((pLutSt->mbr & 0x0300) >> 8) << 14;

            pFdbSmi[4] |= (pLutSt->mbr & 0x00FF);
            pFdbSmi[4] |= (pLutSt->igmpidx & 0x00FF) << 8;

            pFdbSmi[5] |= (pLutSt->igmp_asic & 0x0001);
            pFdbSmi[5] |= (pLutSt->lut_pri & 0x0007) << 1;
            pFdbSmi[5] |= (pLutSt->fwd_en & 0x0001) << 4;
            pFdbSmi[5] |= (pLutSt->nosalearn & 0x0001) << 5;
            pFdbSmi[5] |= ((pLutSt->mbr & 0x0400) >> 10) << 7;
        }
    }
    else if(pLutSt->mac.octet[0] & 0x01) /*Multicast L2 Lookup*/
    {
        pFdbSmi[0] |= pLutSt->mac.octet[5];
        pFdbSmi[0] |= pLutSt->mac.octet[4] << 8;

        pFdbSmi[1] |= pLutSt->mac.octet[3];
        pFdbSmi[1] |= pLutSt->mac.octet[2] << 8;

        pFdbSmi[2] |= pLutSt->mac.octet[1];
        pFdbSmi[2] |= pLutSt->mac.octet[0] << 8;

        pFdbSmi[3] |= pLutSt->cvid_fid;
        pFdbSmi[3] |= (pLutSt->l3lookup & 0x0001) << 12;
        pFdbSmi[3] |= (pLutSt->ivl_svl & 0x0001) << 13;
        pFdbSmi[3] |= ((pLutSt->mbr & 0x0300) >> 8) << 14;

        pFdbSmi[4] |= (pLutSt->mbr & 0x00FF);
        pFdbSmi[4] |= (pLutSt->igmpidx & 0x00FF) << 8;

        pFdbSmi[5] |= pLutSt->igmp_asic;
        pFdbSmi[5] |= (pLutSt->lut_pri & 0x0007) << 1;
        pFdbSmi[5] |= (pLutSt->fwd_en & 0x0001) << 4;
        pFdbSmi[5] |= (pLutSt->nosalearn & 0x0001) << 5;
        pFdbSmi[5] |= ((pLutSt->mbr & 0x0400) >> 10) << 7;
    }
    else /*Asic auto-learning*/
    {
        pFdbSmi[0] |= pLutSt->mac.octet[5];
        pFdbSmi[0] |= pLutSt->mac.octet[4] << 8;

        pFdbSmi[1] |= pLutSt->mac.octet[3];
        pFdbSmi[1] |= pLutSt->mac.octet[2] << 8;

        pFdbSmi[2] |= pLutSt->mac.octet[1];
        pFdbSmi[2] |= pLutSt->mac.octet[0] << 8;

        pFdbSmi[3] |= pLutSt->cvid_fid;
        pFdbSmi[3] |= (pLutSt->l3lookup & 0x0001) << 12;
        pFdbSmi[3] |= (pLutSt->ivl_svl & 0x0001) << 13;
        pFdbSmi[3] |= ((pLutSt->spa & 0x0008) >> 3) << 15;

        pFdbSmi[4] |= pLutSt->efid;
        pFdbSmi[4] |= (pLutSt->fid & 0x000F) << 3;
        pFdbSmi[4] |= (pLutSt->sa_en & 0x0001) << 7;
        pFdbSmi[4] |= (pLutSt->spa & 0x0007) << 8;
        pFdbSmi[4] |= (pLutSt->age & 0x0007) << 11;
        pFdbSmi[4] |= (pLutSt->auth & 0x0001) << 14;
        pFdbSmi[4] |= (pLutSt->sa_block & 0x0001) << 15;

        pFdbSmi[5] |= pLutSt->da_block;
        pFdbSmi[5] |= (pLutSt->lut_pri & 0x0007) << 1;
        pFdbSmi[5] |= (pLutSt->fwd_en & 0x0001) << 4;
        pFdbSmi[5] |= (pLutSt->nosalearn & 0x0001) << 5;
    }
}


static void _rtl8367c_fdbStSmi2User( rtl8367c_luttb *pLutSt, rtk_uint16 *pFdbSmi)
{
    /*L3 lookup*/
    if(pFdbSmi[3] & 0x1000)
    {
        if(pFdbSmi[3] & 0x2000)
        {
            pLutSt->sip             = pFdbSmi[0] | (pFdbSmi[1] << 16);
            pLutSt->dip             = pFdbSmi[2] | ((pFdbSmi[3] & 0x0FFF) << 16);

            pLutSt->mbr             = (pFdbSmi[4] & 0x00FF) | (((pFdbSmi[3] & 0xC000) >> 14) << 8) | (((pFdbSmi[5] & 0x0080) >> 7) << 10);
            pLutSt->l3_vid          = ((pFdbSmi[4] & 0xFF00) >> 8) | (pFdbSmi[5] & 0x000F);

            pLutSt->l3lookup        = (pFdbSmi[3] & 0x1000) >> 12;
            pLutSt->l3vidlookup     = (pFdbSmi[3] & 0x2000) >> 13;
            pLutSt->nosalearn       = (pFdbSmi[5] & 0x0020) >> 5;
        }
        else
        {
            pLutSt->sip             = pFdbSmi[0] | (pFdbSmi[1] << 16);
            pLutSt->dip             = pFdbSmi[2] | ((pFdbSmi[3] & 0x0FFF) << 16);

            pLutSt->lut_pri         = (pFdbSmi[5] & 0x000E) >> 1;
            pLutSt->fwd_en          = (pFdbSmi[5] & 0x0010) >> 4;

            pLutSt->mbr             = (pFdbSmi[4] & 0x00FF) | (((pFdbSmi[3] & 0xC000) >> 14) << 8) | (((pFdbSmi[5] & 0x0080) >> 7) << 10);
            pLutSt->igmpidx         = (pFdbSmi[4] & 0xFF00) >> 8;

            pLutSt->igmp_asic       = (pFdbSmi[5] & 0x0001);
            pLutSt->l3lookup        = (pFdbSmi[3] & 0x1000) >> 12;
            pLutSt->nosalearn       = (pFdbSmi[5] & 0x0020) >> 5;
        }
    }
    else if(pFdbSmi[2] & 0x0100) /*Multicast L2 Lookup*/
    {
        pLutSt->mac.octet[0]    = (pFdbSmi[2] & 0xFF00) >> 8;
        pLutSt->mac.octet[1]    = (pFdbSmi[2] & 0x00FF);
        pLutSt->mac.octet[2]    = (pFdbSmi[1] & 0xFF00) >> 8;
        pLutSt->mac.octet[3]    = (pFdbSmi[1] & 0x00FF);
        pLutSt->mac.octet[4]    = (pFdbSmi[0] & 0xFF00) >> 8;
        pLutSt->mac.octet[5]    = (pFdbSmi[0] & 0x00FF);

        pLutSt->cvid_fid        = pFdbSmi[3] & 0x0FFF;
        pLutSt->lut_pri         = (pFdbSmi[5] & 0x000E) >> 1;
        pLutSt->fwd_en          = (pFdbSmi[5] & 0x0010) >> 4;

        pLutSt->mbr             = (pFdbSmi[4] & 0x00FF) | (((pFdbSmi[3] & 0xC000) >> 14) << 8) | (((pFdbSmi[5] & 0x0080) >> 7) << 10);
        pLutSt->igmpidx         = (pFdbSmi[4] & 0xFF00) >> 8;

        pLutSt->igmp_asic       = (pFdbSmi[5] & 0x0001);
        pLutSt->l3lookup        = (pFdbSmi[3] & 0x1000) >> 12;
        pLutSt->ivl_svl         = (pFdbSmi[3] & 0x2000) >> 13;
        pLutSt->nosalearn       = (pFdbSmi[5] & 0x0020) >> 5;
    }
    else /*Asic auto-learning*/
    {
        pLutSt->mac.octet[0]    = (pFdbSmi[2] & 0xFF00) >> 8;
        pLutSt->mac.octet[1]    = (pFdbSmi[2] & 0x00FF);
        pLutSt->mac.octet[2]    = (pFdbSmi[1] & 0xFF00) >> 8;
        pLutSt->mac.octet[3]    = (pFdbSmi[1] & 0x00FF);
        pLutSt->mac.octet[4]    = (pFdbSmi[0] & 0xFF00) >> 8;
        pLutSt->mac.octet[5]    = (pFdbSmi[0] & 0x00FF);

        pLutSt->cvid_fid        = pFdbSmi[3] & 0x0FFF;
        pLutSt->lut_pri         = (pFdbSmi[5] & 0x000E) >> 1;
        pLutSt->fwd_en          = (pFdbSmi[5] & 0x0010) >> 4;

        pLutSt->sa_en           = (pFdbSmi[4] & 0x0080) >> 7;
        pLutSt->auth            = (pFdbSmi[4] & 0x4000) >> 14;
        pLutSt->spa             = ((pFdbSmi[4] & 0x0700) >> 8) | (((pFdbSmi[3] & 0x8000) >> 15) << 3);
        pLutSt->age             = (pFdbSmi[4] & 0x3800) >> 11;
        pLutSt->fid             = (pFdbSmi[4] & 0x0078) >> 3;
        pLutSt->efid            = (pFdbSmi[4] & 0x0007);
        pLutSt->sa_block        = (pFdbSmi[4] & 0x8000) >> 15;

        pLutSt->da_block        = (pFdbSmi[5] & 0x0001);
        pLutSt->l3lookup        = (pFdbSmi[3] & 0x1000) >> 12;
        pLutSt->ivl_svl         = (pFdbSmi[3] & 0x2000) >> 13;
        pLutSt->nosalearn       = (pFdbSmi[3] & 0x0020) >> 5;
    }
}

/* Function Name:
 *      rtl8367c_setAsicLutIpMulticastLookup
 * Description:
 *      Set Lut IP multicast lookup function
 * Input:
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicLutIpMulticastLookup(rtk_uint32 enabled)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_LUT_CFG, RTL8367C_LUT_IPMC_HASH_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicLutIpMulticastLookup
 * Description:
 *      Get Lut IP multicast lookup function
 * Input:
 *      pEnabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicLutIpMulticastLookup(rtk_uint32* pEnabled)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_LUT_CFG, RTL8367C_LUT_IPMC_HASH_OFFSET, pEnabled);
}

/* Function Name:
 *      rtl8367c_setAsicLutIpMulticastLookup
 * Description:
 *      Set Lut IP multicast + VID lookup function
 * Input:
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicLutIpMulticastVidLookup(rtk_uint32 enabled)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_LUT_CFG2, RTL8367C_LUT_IPMC_VID_HASH_OFFSET, enabled);
}

/* Function Name:
 *      rtl8367c_getAsicLutIpMulticastVidLookup
 * Description:
 *      Get Lut IP multicast lookup function
 * Input:
 *      pEnabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicLutIpMulticastVidLookup(rtk_uint32* pEnabled)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_LUT_CFG2, RTL8367C_LUT_IPMC_VID_HASH_OFFSET, pEnabled);
}

/* Function Name:
 *      rtl8367c_setAsicLutIpLookupMethod
 * Description:
 *      Set Lut IP lookup hash with DIP or {DIP,SIP} pair
 * Input:
 *      type - 1: When DIP can be found in IPMC_GROUP_TABLE, use DIP+SIP Hash, otherwise, use DIP+(SIP=0.0.0.0) Hash.
 *             0: When DIP can be found in IPMC_GROUP_TABLE, use DIP+(SIP=0.0.0.0) Hash, otherwise use DIP+SIP Hash.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicLutIpLookupMethod(rtk_uint32 type)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_LUT_CFG, RTL8367C_LUT_IPMC_LOOKUP_OP_OFFSET, type);
}
/* Function Name:
 *      rtl8367c_getAsicLutIpLookupMethod
 * Description:
 *      Get Lut IP lookup hash with DIP or {DIP,SIP} pair
 * Input:
 *      pType - 1: When DIP can be found in IPMC_GROUP_TABLE, use DIP+SIP Hash, otherwise, use DIP+(SIP=0.0.0.0) Hash.
 *              0: When DIP can be found in IPMC_GROUP_TABLE, use DIP+(SIP=0.0.0.0) Hash, otherwise use DIP+SIP Hash.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicLutIpLookupMethod(rtk_uint32* pType)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_LUT_CFG, RTL8367C_LUT_IPMC_LOOKUP_OP_OFFSET, pType);
}
/* Function Name:
 *      rtl8367c_setAsicLutAgeTimerSpeed
 * Description:
 *      Set LUT agging out speed
 * Input:
 *      timer - Agging out timer 0:Has been aged out
 *      speed - Agging out speed 0-fastest 3-slowest
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicLutAgeTimerSpeed(rtk_uint32 timer, rtk_uint32 speed)
{
    if(timer>RTL8367C_LUT_AGETIMERMAX)
        return RT_ERR_OUT_OF_RANGE;

    if(speed >RTL8367C_LUT_AGESPEEDMAX)
        return RT_ERR_OUT_OF_RANGE;

    return rtl8367c_setAsicRegBits(RTL8367C_REG_LUT_CFG, RTL8367C_AGE_TIMER_MASK | RTL8367C_AGE_SPEED_MASK, (timer << RTL8367C_AGE_TIMER_OFFSET) | (speed << RTL8367C_AGE_SPEED_OFFSET));
}
/* Function Name:
 *      rtl8367c_getAsicLutAgeTimerSpeed
 * Description:
 *      Get LUT agging out speed
 * Input:
 *      pTimer - Agging out timer 0:Has been aged out
 *      pSpeed - Agging out speed 0-fastest 3-slowest
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicLutAgeTimerSpeed(rtk_uint32* pTimer, rtk_uint32* pSpeed)
{
    rtk_uint32 regData;
    ret_t retVal;

    retVal = rtl8367c_getAsicReg(RTL8367C_REG_LUT_CFG, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pTimer =  (regData & RTL8367C_AGE_TIMER_MASK) >> RTL8367C_AGE_TIMER_OFFSET;

    *pSpeed =  (regData & RTL8367C_AGE_SPEED_MASK) >> RTL8367C_AGE_SPEED_OFFSET;

    return RT_ERR_OK;

}
/* Function Name:
 *      rtl8367c_setAsicLutCamTbUsage
 * Description:
 *      Configure Lut CAM table usage
 * Input:
 *      enabled - L2 CAM table usage 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicLutCamTbUsage(rtk_uint32 enabled)
{
    ret_t retVal;

    retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_LUT_CFG, RTL8367C_BCAM_DISABLE_OFFSET, enabled ? 0 : 1);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicLutCamTbUsage
 * Description:
 *      Get Lut CAM table usage
 * Input:
 *      pEnabled - L2 CAM table usage 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicLutCamTbUsage(rtk_uint32* pEnabled)
{
    ret_t       retVal;
    rtk_uint32  regData;

    if ((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_LUT_CFG, RTL8367C_BCAM_DISABLE_OFFSET, &regData)) != RT_ERR_OK)
        return retVal;

    *pEnabled = regData ? 0 : 1;
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_setAsicLutLearnLimitNo
 * Description:
 *      Set per-Port auto learning limit number
 * Input:
 *      port    - Physical port number (0~7)
 *      number  - ASIC auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - Success
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number
 *      RT_ERR_LIMITED_L2ENTRY_NUM  - Invalid auto learning limit number
 * Note:
 *      None
 */
   /*修改: RTL8367C_PORTIDMAX, RTL8367C_LUT_LEARNLIMITMAX, RTL8367C_LUT_PORT_LEARN_LIMITNO_REG*/
ret_t rtl8367c_setAsicLutLearnLimitNo(rtk_uint32 port, rtk_uint32 number)
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(number > RTL8367C_LUT_LEARNLIMITMAX)
        return RT_ERR_LIMITED_L2ENTRY_NUM;

    if(port < 8)
     return rtl8367c_setAsicReg(RTL8367C_LUT_PORT_LEARN_LIMITNO_REG(port), number);
    else
        return rtl8367c_setAsicReg(RTL8367C_REG_LUT_PORT8_LEARN_LIMITNO+port-8, number);

}
/* Function Name:
 *      rtl8367c_getAsicLutLearnLimitNo
 * Description:
 *      Get per-Port auto learning limit number
 * Input:
 *      port    - Physical port number (0~7)
 *      pNumber     - ASIC auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
  /*修改: RTL8367C_PORTIDMAX, RTL8367C_LUT_PORT_LEARN_LIMITNO_REG*/
ret_t rtl8367c_getAsicLutLearnLimitNo(rtk_uint32 port, rtk_uint32* pNumber)
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(port < 8)
     return rtl8367c_getAsicReg(RTL8367C_LUT_PORT_LEARN_LIMITNO_REG(port), pNumber);
    else
        return rtl8367c_getAsicReg(RTL8367C_REG_LUT_PORT8_LEARN_LIMITNO+port-8, pNumber);
}

/* Function Name:
 *      rtl8367c_setAsicSystemLutLearnLimitNo
 * Description:
 *      Set system auto learning limit number
 * Input:
 *      number  - ASIC auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - Success
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number
 *      RT_ERR_LIMITED_L2ENTRY_NUM  - Invalid auto learning limit number
 * Note:
 *      None
 */
  /*修改: RTL8367C_LUT_LEARNLIMITMAX*/
ret_t rtl8367c_setAsicSystemLutLearnLimitNo(rtk_uint32 number)
{
    if(number > RTL8367C_LUT_LEARNLIMITMAX)
        return RT_ERR_LIMITED_L2ENTRY_NUM;

    return rtl8367c_setAsicReg(RTL8367C_REG_LUT_SYS_LEARN_LIMITNO, number);
}

/* Function Name:
 *      rtl8367c_getAsicSystemLutLearnLimitNo
 * Description:
 *      Get system auto learning limit number
 * Input:
 *      port    - Physical port number (0~7)
 *      pNumber     - ASIC auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicSystemLutLearnLimitNo(rtk_uint32 *pNumber)
{
    if(NULL == pNumber)
        return RT_ERR_NULL_POINTER;

    return rtl8367c_getAsicReg(RTL8367C_REG_LUT_SYS_LEARN_LIMITNO, pNumber);
}

/* Function Name:
 *      rtl8367c_setAsicLutLearnOverAct
 * Description:
 *      Set auto learn over limit number action
 * Input:
 *      action  - Learn over action 0:normal, 1:drop 2:trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NOT_ALLOWED  - Invalid learn over action
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicLutLearnOverAct(rtk_uint32 action)
{
    if(action >= LRNOVERACT_END)
        return RT_ERR_NOT_ALLOWED;

    return rtl8367c_setAsicRegBits(RTL8367C_REG_PORT_SECURITY_CTRL, RTL8367C_LUT_LEARN_OVER_ACT_MASK, action);
}
/* Function Name:
 *      rtl8367c_getAsicLutLearnOverAct
 * Description:
 *      Get auto learn over limit number action
 * Input:
 *      pAction     - Learn over action 0:normal, 1:drop 2:trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicLutLearnOverAct(rtk_uint32* pAction)
{
    return rtl8367c_getAsicRegBits(RTL8367C_REG_PORT_SECURITY_CTRL, RTL8367C_LUT_LEARN_OVER_ACT_MASK, pAction);
}

/* Function Name:
 *      rtl8367c_setAsicSystemLutLearnOverAct
 * Description:
 *      Set system auto learn over limit number action
 * Input:
 *      action  - Learn over action 0:normal, 1:drop, 2:trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NOT_ALLOWED  - Invalid learn over action
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicSystemLutLearnOverAct(rtk_uint32 action)
{
    if(action >= LRNOVERACT_END)
        return RT_ERR_NOT_ALLOWED;

    return rtl8367c_setAsicRegBits(RTL8367C_REG_LUT_LRN_SYS_LMT_CTRL, RTL8367C_LUT_SYSTEM_LEARN_OVER_ACT_MASK, action);
}

/* Function Name:
 *      rtl8367c_getAsicSystemLutLearnOverAct
 * Description:
 *      Get system auto learn over limit number action
 * Input:
 *      pAction     - Learn over action 0:normal, 1:drop 2:trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicSystemLutLearnOverAct(rtk_uint32 *pAction)
{
    if(NULL == pAction)
        return RT_ERR_NULL_POINTER;

    return rtl8367c_getAsicRegBits(RTL8367C_REG_LUT_LRN_SYS_LMT_CTRL, RTL8367C_LUT_SYSTEM_LEARN_OVER_ACT_MASK, pAction);
}

/* Function Name:
 *      rtl8367c_setAsicSystemLutLearnPortMask
 * Description:
 *      Set system auto learn limit port mask
 * Input:
 *      portmask    - port mask of system learning limit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK    - Error port mask
 * Note:
 *      None
 */
  /*修改: RTL8367C_LUT_SYSTEM_LEARN_PMASK_MASK*/
ret_t rtl8367c_setAsicSystemLutLearnPortMask(rtk_uint32 portmask)
{
    ret_t retVal;

    if(portmask > RTL8367C_PORTMASK)
        return RT_ERR_PORT_MASK;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_LUT_LRN_SYS_LMT_CTRL, RTL8367C_LUT_SYSTEM_LEARN_PMASK_MASK, portmask & 0xff);
    if(retVal != RT_ERR_OK)
        return retVal;
    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_LUT_LRN_SYS_LMT_CTRL, RTL8367C_LUT_SYSTEM_LEARN_PMASK1_MASK, (portmask>>8) & 0x7);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;

}

/* Function Name:
 *      rtl8367c_getAsicSystemLutLearnPortMask
 * Description:
 *      Get system auto learn limit port mask
 * Input:
 *      None
 * Output:
 *      pPortmask   - port mask of system learning limit
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      None
 */
 /*修改: RTL8367C_LUT_SYSTEM_LEARN_PMASK_MASK*/
ret_t rtl8367c_getAsicSystemLutLearnPortMask(rtk_uint32 *pPortmask)
{
    rtk_uint32 tmpmask;
    ret_t retVal;

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_LUT_LRN_SYS_LMT_CTRL, RTL8367C_LUT_SYSTEM_LEARN_PMASK_MASK, &tmpmask);
    if(retVal != RT_ERR_OK)
        return retVal;
    *pPortmask = tmpmask & 0xff;
    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_LUT_LRN_SYS_LMT_CTRL, RTL8367C_LUT_SYSTEM_LEARN_PMASK1_MASK, &tmpmask);
    if(retVal != RT_ERR_OK)
        return retVal;
    *pPortmask |= (tmpmask & 0x7) << 8;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicL2LookupTb
 * Description:
 *      Set filtering database entry
 * Input:
 *      pL2Table    - L2 table entry writing to 8K+64 filtering database
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicL2LookupTb(rtl8367c_luttb *pL2Table)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint16 *accessPtr;
    rtk_uint32 i;
    rtk_uint16 smil2Table[RTL8367C_LUT_TABLE_SIZE];
    rtk_uint32 tblCmd;
    rtk_uint32 busyCounter;

    memset(smil2Table, 0x00, sizeof(rtk_uint16) * RTL8367C_LUT_TABLE_SIZE);
    _rtl8367c_fdbStUser2Smi(pL2Table, smil2Table);

    if(pL2Table->wait_time == 0)
        busyCounter = RTL8367C_LUT_BUSY_CHECK_NO;
    else
        busyCounter = pL2Table->wait_time;

    while(busyCounter)
    {
        retVal = rtl8367c_getAsicRegBit(RTL8367C_TABLE_ACCESS_STATUS_REG, RTL8367C_TABLE_LUT_ADDR_BUSY_FLAG_OFFSET,&regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        pL2Table->lookup_busy = regData;
        if(!regData)
            break;

        busyCounter --;
        if(busyCounter == 0)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }

    accessPtr = smil2Table;
    regData = *accessPtr;
    for(i = 0; i < RTL8367C_LUT_ENTRY_SIZE; i++)
    {
        retVal = rtl8367c_setAsicReg(RTL8367C_TABLE_ACCESS_WRDATA_BASE + i, regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        accessPtr ++;
        regData = *accessPtr;

    }

    tblCmd = (RTL8367C_TABLE_ACCESS_REG_DATA(TB_OP_WRITE,TB_TARGET_L2)) & (RTL8367C_TABLE_TYPE_MASK  | RTL8367C_COMMAND_TYPE_MASK);
    /* Write Command */
    retVal = rtl8367c_setAsicReg(RTL8367C_TABLE_ACCESS_CTRL_REG, tblCmd);
    if(retVal != RT_ERR_OK)
        return retVal;

    if(pL2Table->wait_time == 0)
        busyCounter = RTL8367C_LUT_BUSY_CHECK_NO;
    else
        busyCounter = pL2Table->wait_time;

    while(busyCounter)
    {
        retVal = rtl8367c_getAsicRegBit(RTL8367C_TABLE_ACCESS_STATUS_REG, RTL8367C_TABLE_LUT_ADDR_BUSY_FLAG_OFFSET,&regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        pL2Table->lookup_busy = regData;
        if(!regData)
            break;

        busyCounter --;
        if(busyCounter == 0)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }

    /*Read access status*/
    retVal = rtl8367c_getAsicRegBit(RTL8367C_TABLE_ACCESS_STATUS_REG, RTL8367C_HIT_STATUS_OFFSET, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    pL2Table->lookup_hit = regData;
    if(!pL2Table->lookup_hit)
        return RT_ERR_FAILED;

    /*Read access address*/
    /*
    retVal = rtl8367c_getAsicRegBits(RTL8367C_TABLE_ACCESS_STATUS_REG, RTL8367C_TABLE_LUT_ADDR_TYPE_MASK | RTL8367C_TABLE_LUT_ADDR_ADDRESS_MASK,&regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    pL2Table->address = regData;*/

    retVal = rtl8367c_getAsicReg(RTL8367C_TABLE_ACCESS_STATUS_REG, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    pL2Table->address = (regData & 0x7ff) | ((regData & 0x4000) >> 3) | ((regData & 0x800) << 1);
    pL2Table->lookup_busy = 0;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_getAsicL2LookupTb
 * Description:
 *      Get filtering database entry
 * Input:
 *      pL2Table    - L2 table entry writing to 2K+64 filtering database
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameter
 *      RT_ERR_BUSYWAIT_TIMEOUT - LUT is busy at retrieving
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicL2LookupTb(rtk_uint32 method, rtl8367c_luttb *pL2Table)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint16* accessPtr;
    rtk_uint32 i;
    rtk_uint16 smil2Table[RTL8367C_LUT_TABLE_SIZE];
    rtk_uint32 busyCounter;
    rtk_uint32 tblCmd;

    if(pL2Table->wait_time == 0)
        busyCounter = RTL8367C_LUT_BUSY_CHECK_NO;
    else
        busyCounter = pL2Table->wait_time;

    while(busyCounter)
    {
        retVal = rtl8367c_getAsicRegBit(RTL8367C_TABLE_ACCESS_STATUS_REG, RTL8367C_TABLE_LUT_ADDR_BUSY_FLAG_OFFSET,&regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        pL2Table->lookup_busy = regData;
        if(!pL2Table->lookup_busy)
            break;

        busyCounter --;
        if(busyCounter == 0)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }


    tblCmd = (method << RTL8367C_ACCESS_METHOD_OFFSET) & RTL8367C_ACCESS_METHOD_MASK;

    switch(method)
    {
        case LUTREADMETHOD_ADDRESS:
        case LUTREADMETHOD_NEXT_ADDRESS:
        case LUTREADMETHOD_NEXT_L2UC:
        case LUTREADMETHOD_NEXT_L2MC:
        case LUTREADMETHOD_NEXT_L3MC:
        case LUTREADMETHOD_NEXT_L2L3MC:
            retVal = rtl8367c_setAsicReg(RTL8367C_TABLE_ACCESS_ADDR_REG, pL2Table->address);
            if(retVal != RT_ERR_OK)
                return retVal;
            break;
        case LUTREADMETHOD_MAC:
            memset(smil2Table, 0x00, sizeof(rtk_uint16) * RTL8367C_LUT_TABLE_SIZE);
            _rtl8367c_fdbStUser2Smi(pL2Table, smil2Table);

            accessPtr = smil2Table;
            regData = *accessPtr;
            for(i=0; i<RTL8367C_LUT_ENTRY_SIZE; i++)
            {
                retVal = rtl8367c_setAsicReg(RTL8367C_TABLE_ACCESS_WRDATA_BASE + i, regData);
                if(retVal != RT_ERR_OK)
                    return retVal;

                accessPtr ++;
                regData = *accessPtr;

            }
            break;
        case LUTREADMETHOD_NEXT_L2UCSPA:
            retVal = rtl8367c_setAsicReg(RTL8367C_TABLE_ACCESS_ADDR_REG, pL2Table->address);
            if(retVal != RT_ERR_OK)
                return retVal;

            tblCmd = tblCmd | ((pL2Table->spa << RTL8367C_TABLE_ACCESS_CTRL_SPA_OFFSET) & RTL8367C_TABLE_ACCESS_CTRL_SPA_MASK);

            break;
        default:
            return RT_ERR_INPUT;
    }

    tblCmd = tblCmd | ((RTL8367C_TABLE_ACCESS_REG_DATA(TB_OP_READ,TB_TARGET_L2)) & (RTL8367C_TABLE_TYPE_MASK  | RTL8367C_COMMAND_TYPE_MASK));
    /* Read Command */
    retVal = rtl8367c_setAsicReg(RTL8367C_TABLE_ACCESS_CTRL_REG, tblCmd);
    if(retVal != RT_ERR_OK)
        return retVal;

    if(pL2Table->wait_time == 0)
        busyCounter = RTL8367C_LUT_BUSY_CHECK_NO;
    else
        busyCounter = pL2Table->wait_time;

    while(busyCounter)
    {
        retVal = rtl8367c_getAsicRegBit(RTL8367C_TABLE_ACCESS_STATUS_REG, RTL8367C_TABLE_LUT_ADDR_BUSY_FLAG_OFFSET,&regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        pL2Table->lookup_busy = regData;
        if(!pL2Table->lookup_busy)
            break;

        busyCounter --;
        if(busyCounter == 0)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }

    retVal = rtl8367c_getAsicRegBit(RTL8367C_TABLE_ACCESS_STATUS_REG, RTL8367C_HIT_STATUS_OFFSET,&regData);
    if(retVal != RT_ERR_OK)
            return retVal;
    pL2Table->lookup_hit = regData;
    if(!pL2Table->lookup_hit)
        return RT_ERR_L2_ENTRY_NOTFOUND;

    /*Read access address*/
    //retVal = rtl8367c_getAsicRegBits(RTL8367C_TABLE_ACCESS_STATUS_REG, RTL8367C_TABLE_LUT_ADDR_TYPE_MASK | RTL8367C_TABLE_LUT_ADDR_ADDRESS_MASK,&regData);
    retVal = rtl8367c_getAsicReg(RTL8367C_TABLE_ACCESS_STATUS_REG, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    pL2Table->address = (regData & 0x7ff) | ((regData & 0x4000) >> 3) | ((regData & 0x800) << 1);

    /*read L2 entry */
    memset(smil2Table, 0x00, sizeof(rtk_uint16) * RTL8367C_LUT_TABLE_SIZE);

    accessPtr = smil2Table;

    for(i = 0; i < RTL8367C_LUT_ENTRY_SIZE; i++)
    {
        retVal = rtl8367c_getAsicReg(RTL8367C_TABLE_ACCESS_RDDATA_BASE + i, &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        *accessPtr = regData;

        accessPtr ++;
    }

    _rtl8367c_fdbStSmi2User(pL2Table, smil2Table);

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_getAsicLutLearnNo
 * Description:
 *      Get per-Port auto learning number
 * Input:
 *      port    - Physical port number (0~7)
 *      pNumber     - ASIC auto learning entries number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
 /*修改RTL8367C_PORTIDMAX, RTL8367C_REG_L2_LRN_CNT_REG, port10 reg is not contnious, wait for updating of base.h*/
ret_t rtl8367c_getAsicLutLearnNo(rtk_uint32 port, rtk_uint32* pNumber)
{
    ret_t retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(port < 10)
    {
     retVal = rtl8367c_getAsicReg(RTL8367C_REG_L2_LRN_CNT_REG(port), pNumber);
        if (retVal != RT_ERR_OK)
         return retVal;
    }
    else
    {
        retVal = rtl8367c_getAsicReg(RTL8367C_REG_L2_LRN_CNT_CTRL10, pNumber);
        if (retVal != RT_ERR_OK)
         return retVal;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicLutFlushAll
 * Description:
 *      Flush all entries in LUT. Includes static & dynamic entries
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicLutFlushAll(void)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_L2_FLUSH_CTRL3, RTL8367C_L2_FLUSH_CTRL3_OFFSET, 1);
}

/* Function Name:
 *      rtl8367c_getAsicLutFlushAllStatus
 * Description:
 *      Get Flush all status, 1:Busy, 0 normal
 * Input:
 *      None
 * Output:
 *      pBusyStatus - Busy state
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NULL_POINTER - Null pointer
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicLutFlushAllStatus(rtk_uint32 *pBusyStatus)
{
    if(NULL == pBusyStatus)
        return RT_ERR_NULL_POINTER;

    return rtl8367c_getAsicRegBit(RTL8367C_REG_L2_FLUSH_CTRL3, RTL8367C_L2_FLUSH_CTRL3_OFFSET, pBusyStatus);
}

/* Function Name:
 *      rtl8367c_setAsicLutForceFlush
 * Description:
 *      Set per port force flush setting
 * Input:
 *      portmask    - portmask(0~0xFF)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK    - Invalid portmask
 * Note:
 *      None
 */
 /*port8~port10的设置在另外一个register, wait for updating of base.h, reg.h*/
ret_t rtl8367c_setAsicLutForceFlush(rtk_uint32 portmask)
{
    ret_t retVal;

    if(portmask > RTL8367C_PORTMASK)
        return RT_ERR_PORT_MASK;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_FORCE_FLUSH_REG, RTL8367C_FORCE_FLUSH_PORTMASK_MASK, portmask & 0xff);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FORCE_FLUSH1, RTL8367C_PORTMASK1_MASK, (portmask >> 8) & 0x7);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_getAsicLutForceFlushStatus
 * Description:
 *      Get per port force flush status
 * Input:
 *      pPortmask   - portmask(0~0xFF)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
 /*port8~port10的设置在另外一个register, wait for updating of base.h, reg.h*/
ret_t rtl8367c_getAsicLutForceFlushStatus(rtk_uint32 *pPortmask)
{
    rtk_uint32 tmpMask;
    ret_t retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_FORCE_FLUSH_REG, RTL8367C_BUSY_STATUS_MASK,&tmpMask);
    if(retVal != RT_ERR_OK)
        return retVal;
    *pPortmask = tmpMask & 0xff;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FORCE_FLUSH1, RTL8367C_BUSY_STATUS1_MASK,&tmpMask);
    if(retVal != RT_ERR_OK)
        return retVal;
    *pPortmask |= (tmpMask & 7) << 8;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_setAsicLutFlushMode
 * Description:
 *      Set user force L2 pLutSt table flush mode
 * Input:
 *      mode    - 0:Port based 1: Port + VLAN based 2:Port + FID/MSTI based
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NOT_ALLOWED  - Actions not allowed by the function
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicLutFlushMode(rtk_uint32 mode)
{
    if( mode >= FLUSHMDOE_END )
        return RT_ERR_NOT_ALLOWED;

    return rtl8367c_setAsicRegBits(RTL8367C_REG_L2_FLUSH_CTRL2, RTL8367C_LUT_FLUSH_MODE_MASK, mode);
}
/* Function Name:
 *      rtl8367c_getAsicLutFlushMode
 * Description:
 *      Get user force L2 pLutSt table flush mode
 * Input:
 *      pMode   - 0:Port based 1: Port + VLAN based 2:Port + FID/MSTI based
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicLutFlushMode(rtk_uint32* pMode)
{
    return rtl8367c_getAsicRegBits(RTL8367C_REG_L2_FLUSH_CTRL2, RTL8367C_LUT_FLUSH_MODE_MASK, pMode);
}
/* Function Name:
 *      rtl8367c_setAsicLutFlushType
 * Description:
 *      Get L2 LUT flush type
 * Input:
 *      type    - 0: dynamice unicast; 1: both dynamic and static unicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicLutFlushType(rtk_uint32 type)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_L2_FLUSH_CTRL2, RTL8367C_LUT_FLUSH_TYPE_OFFSET,type);
}
/* Function Name:
 *      rtl8367c_getAsicLutFlushType
 * Description:
 *      Set L2 LUT flush type
 * Input:
 *      pType   - 0: dynamice unicast; 1: both dynamic and static unicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicLutFlushType(rtk_uint32* pType)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_L2_FLUSH_CTRL2, RTL8367C_LUT_FLUSH_TYPE_OFFSET,pType);
}


/* Function Name:
 *      rtl8367c_setAsicLutFlushVid
 * Description:
 *      Set VID of Port + VID pLutSt flush mode
 * Input:
 *      vid     - Vid (0~4095)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_VLAN_VID - Invalid VID parameter (0~4095)
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicLutFlushVid(rtk_uint32 vid)
{
    if( vid > RTL8367C_VIDMAX )
        return RT_ERR_VLAN_VID;

    return rtl8367c_setAsicRegBits(RTL8367C_REG_L2_FLUSH_CTRL1, RTL8367C_LUT_FLUSH_VID_MASK, vid);
}
/* Function Name:
 *      rtl8367c_getAsicLutFlushVid
 * Description:
 *      Get VID of Port + VID pLutSt flush mode
 * Input:
 *      pVid    - Vid (0~4095)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicLutFlushVid(rtk_uint32* pVid)
{
    return rtl8367c_getAsicRegBits(RTL8367C_REG_L2_FLUSH_CTRL1, RTL8367C_LUT_FLUSH_VID_MASK, pVid);
}
/* Function Name:
 *      rtl8367c_setAsicPortFlusdFid
 * Description:
 *      Set FID of Port + FID pLutSt flush mode
 * Input:
 *      fid     - FID/MSTI for force flush
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_L2_FID   - Invalid FID (0~15)
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicLutFlushFid(rtk_uint32 fid)
{
    if( fid > RTL8367C_FIDMAX )
        return RT_ERR_L2_FID;

    return rtl8367c_setAsicRegBits(RTL8367C_REG_L2_FLUSH_CTRL1, RTL8367C_LUT_FLUSH_FID_MASK, fid);
}
/* Function Name:
 *      rtl8367c_getAsicLutFlushFid
 * Description:
 *      Get FID of Port + FID pLutSt flush mode
 * Input:
 *      pFid    - FID/MSTI for force flush
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicLutFlushFid(rtk_uint32* pFid)
{
    return rtl8367c_getAsicRegBits(RTL8367C_REG_L2_FLUSH_CTRL1, RTL8367C_LUT_FLUSH_FID_MASK, pFid);
}
/* Function Name:
 *      rtl8367c_setAsicLutDisableAging
 * Description:
 *      Set L2 LUT aging per port setting
 * Input:
 *      port    - Physical port number (0~7)
 *      disabled    - 0: enable aging; 1: disabling aging
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
 /*修改RTL8367C_PORTIDMAX*/
ret_t rtl8367c_setAsicLutDisableAging(rtk_uint32 port, rtk_uint32 disabled)
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367c_setAsicRegBit(RTL8367C_LUT_AGEOUT_CTRL_REG, port, disabled);
}
/* Function Name:
 *      rtl8367c_getAsicLutDisableAging
 * Description:
 *      Get L2 LUT aging per port setting
 * Input:
 *      port    - Physical port number (0~7)
 *      pDisabled - 0: enable aging; 1: disabling aging
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
 /*修改RTL8367C_PORTIDMAX*/
ret_t rtl8367c_getAsicLutDisableAging(rtk_uint32 port, rtk_uint32 *pDisabled)
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367c_getAsicRegBit(RTL8367C_LUT_AGEOUT_CTRL_REG, port, pDisabled);
}

/* Function Name:
 *      rtl8367c_setAsicLutIPMCGroup
 * Description:
 *      Set IPMC Group Table
 * Input:
 *      index       - the entry index in table (0 ~ 63)
 *      group_addr  - the multicast group address (224.0.0.0 ~ 239.255.255.255)
 *      vid         - VLAN ID
 *      pmask       - portmask
 *      valid       - valid bit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_INPUT    - Invalid parameter
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicLutIPMCGroup(rtk_uint32 index, ipaddr_t group_addr, rtk_uint32 vid, rtk_uint32 pmask, rtk_uint32 valid)
{
    rtk_uint32  regAddr, regData, bitoffset;
    ipaddr_t    ipData;
    ret_t       retVal;

    if(index > RTL8367C_LUT_IPMCGRP_TABLE_MAX)
        return RT_ERR_INPUT;

    if (vid > RTL8367C_VIDMAX)
        return RT_ERR_VLAN_VID;

    ipData = group_addr;

    if( (ipData & 0xF0000000) != 0xE0000000)    /* not in 224.0.0.0 ~ 239.255.255.255 */
        return RT_ERR_INPUT;

    /* Group Address */
    regAddr = RTL8367C_REG_IPMC_GROUP_ENTRY0_H + (index * 2);
    regData = ((ipData & 0x0FFFFFFF) >> 16);

    if( (retVal = rtl8367c_setAsicReg(regAddr, regData)) != RT_ERR_OK)
        return retVal;

    regAddr++;
    regData = (ipData & 0x0000FFFF);

    if( (retVal = rtl8367c_setAsicReg(regAddr, regData)) != RT_ERR_OK)
        return retVal;

    /* VID */
    regAddr = RTL8367C_REG_IPMC_GROUP_VID_00 + index;
    regData = vid;

    if( (retVal = rtl8367c_setAsicReg(regAddr, regData)) != RT_ERR_OK)
        return retVal;

    /* portmask */
    regAddr = RTL8367C_REG_IPMC_GROUP_PMSK_00 + index;
    regData = pmask;

    if( (retVal = rtl8367c_setAsicReg(regAddr, regData)) != RT_ERR_OK)
        return retVal;

    /* valid */
    regAddr = RTL8367C_REG_IPMC_GROUP_VALID_15_0 + (index / 16);
    bitoffset = index % 16;
    if( (retVal = rtl8367c_setAsicRegBit(regAddr, bitoffset, valid)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicLutIPMCGroup
 * Description:
 *      Set IPMC Group Table
 * Input:
 *      index       - the entry index in table (0 ~ 63)
 * Output:
 *      pGroup_addr - the multicast group address (224.0.0.0 ~ 239.255.255.255)
 *      pVid        - VLAN ID
 *      pPmask      - portmask
 *      pValid      - Valid bit
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_INPUT    - Invalid parameter
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicLutIPMCGroup(rtk_uint32 index, ipaddr_t *pGroup_addr, rtk_uint32 *pVid, rtk_uint32 *pPmask, rtk_uint32 *pValid)
{
    rtk_uint32      regAddr, regData, bitoffset;
    ipaddr_t    ipData;
    ret_t       retVal;

    if(index > RTL8367C_LUT_IPMCGRP_TABLE_MAX)
        return RT_ERR_INPUT;

    if (NULL == pGroup_addr)
        return RT_ERR_NULL_POINTER;

    if (NULL == pVid)
        return RT_ERR_NULL_POINTER;

    if (NULL == pPmask)
        return RT_ERR_NULL_POINTER;

    /* Group address */
    regAddr = RTL8367C_REG_IPMC_GROUP_ENTRY0_H + (index * 2);
    if( (retVal = rtl8367c_getAsicReg(regAddr, &regData)) != RT_ERR_OK)
        return retVal;

    *pGroup_addr = (((regData & 0x00000FFF) << 16) | 0xE0000000);

    regAddr++;
    if( (retVal = rtl8367c_getAsicReg(regAddr, &regData)) != RT_ERR_OK)
        return retVal;

    ipData = (*pGroup_addr | (regData & 0x0000FFFF));
    *pGroup_addr = ipData;

    /* VID */
    regAddr = RTL8367C_REG_IPMC_GROUP_VID_00 + index;
    if( (retVal = rtl8367c_getAsicReg(regAddr, &regData)) != RT_ERR_OK)
        return retVal;

    *pVid = regData;

    /* portmask */
    regAddr = RTL8367C_REG_IPMC_GROUP_PMSK_00 + index;
    if( (retVal = rtl8367c_getAsicReg(regAddr, &regData)) != RT_ERR_OK)
        return retVal;

    *pPmask = regData;

    /* valid */
    regAddr = RTL8367C_REG_IPMC_GROUP_VALID_15_0 + (index / 16);
    bitoffset = index % 16;
    if( (retVal = rtl8367c_getAsicRegBit(regAddr, bitoffset, &regData)) != RT_ERR_OK)
        return retVal;

    *pValid = regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicLutLinkDownForceAging
 * Description:
 *       Set LUT link down aging setting.
 * Input:
 *      enable      - link down aging setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_ENABLE    - Invalid parameter
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicLutLinkDownForceAging(rtk_uint32 enable)
{
    if(enable > 1)
        return RT_ERR_ENABLE;

    return rtl8367c_setAsicRegBit(RTL8367C_REG_LUT_CFG, RTL8367C_LINKDOWN_AGEOUT_OFFSET, enable ? 0 : 1);
}

/* Function Name:
 *      rtl8367c_getAsicLutLinkDownForceAging
 * Description:
 *       Get LUT link down aging setting.
 * Input:
 *      pEnable         - link down aging setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_ENABLE    - Invalid parameter
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicLutLinkDownForceAging(rtk_uint32 *pEnable)
{
    rtk_uint32  value;
    ret_t   retVal;

    if ((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_LUT_CFG, RTL8367C_LINKDOWN_AGEOUT_OFFSET, &value)) != RT_ERR_OK)
        return retVal;

    *pEnable = value ? 0 : 1;
    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicLutIpmcFwdRouterPort
 * Description:
 *       Set IPMC packet forward to rounter port also or not
 * Input:
 *      enable      - 1: Inlcude router port, 0, exclude router port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_ENABLE     Invalid parameter
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicLutIpmcFwdRouterPort(rtk_uint32 enable)
{
    if(enable > 1)
        return RT_ERR_ENABLE;

    return rtl8367c_setAsicRegBit(RTL8367C_REG_LUT_CFG2, RTL8367C_LUT_IPMC_FWD_RPORT_OFFSET, enable);
}

/* Function Name:
 *      rtl8367c_getAsicLutIpmcFwdRouterPort
 * Description:
 *       Get IPMC packet forward to rounter port also or not
 * Input:
 *      None
 * Output:
 *      pEnable         - 1: Inlcude router port, 0, exclude router port
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_NULL_POINTER     - Null pointer
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicLutIpmcFwdRouterPort(rtk_uint32 *pEnable)
{
    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    return rtl8367c_getAsicRegBit(RTL8367C_REG_LUT_CFG2, RTL8367C_LUT_IPMC_FWD_RPORT_OFFSET, pEnable);
}

