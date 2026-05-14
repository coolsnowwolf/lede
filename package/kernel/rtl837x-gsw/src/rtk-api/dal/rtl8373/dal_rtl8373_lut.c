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
 * Purpose : RTK switch high-level API for RTL8367/RTL8373
 * Feature : Here is a list of all functions and variables in LUT module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal_rtl8373_lut.h>
#include <rtl8373_asicdrv.h>
#include <string.h>
#include "l2.h"

static void _rtl8373_fdbStUser2Smi( rtl8373_luttb *pLutSt, rtk_uint32 *pFdbSmi)
{
    /* L3 lookup */
    if(pLutSt->l3lookup)
    {
        pFdbSmi[0] = pLutSt->sip;

        pFdbSmi[1] = pLutSt->dip & 0xFFFFFFF;
          pFdbSmi[1] |= (pLutSt->l3lookup & 0x0001) << 28;
        pFdbSmi[1] |= (pLutSt->mbr & 0x0003) << 30;

        pFdbSmi[2] = (pLutSt->mbr >> 2) & 0xFF;
        pFdbSmi[2] |= (pLutSt->igmp_idx & 0xFF) << 8;
        pFdbSmi[2] |= (pLutSt->igmp_asic & 1) << 16;

    }
    else if(pLutSt->mac.octet[0] & 0x01) /*Multicast L2 Lookup*/
    {
        pFdbSmi[0] = pLutSt->mac.octet[5]|(pLutSt->mac.octet[4] << 8)|(pLutSt->mac.octet[3] << 16)|(pLutSt->mac.octet[2] << 24);

        pFdbSmi[1] = pLutSt->mac.octet[1]| (pLutSt->mac.octet[0] << 8);
        pFdbSmi[1] |= (pLutSt->cvid_fid & 0xFFF) << 16;
        pFdbSmi[1] |= (pLutSt->l3lookup & 1) << 28;
        pFdbSmi[1] |= (pLutSt->ivl_svl & 1) << 29;
        pFdbSmi[1] |= (pLutSt->mbr & 0x3) << 30;

        pFdbSmi[2] = (pLutSt->mbr >> 2) & 0xFF;
        pFdbSmi[2] |= (pLutSt->igmp_idx & 0xFF) << 8;
        pFdbSmi[2] |= (pLutSt->igmp_asic & 1) << 16;
    }
    else /*Asic auto-learning*/
    {
        pFdbSmi[0] = pLutSt->mac.octet[5]|(pLutSt->mac.octet[4] << 8)|(pLutSt->mac.octet[3] << 16)|(pLutSt->mac.octet[2] << 24);

        pFdbSmi[1] = pLutSt->mac.octet[1]| (pLutSt->mac.octet[0] << 8);
        pFdbSmi[1] |= (pLutSt->cvid_fid & 0xFFF) << 16;
        pFdbSmi[1] |= (pLutSt->l3lookup & 1) << 28;
        pFdbSmi[1] |= (pLutSt->ivl_svl & 1) << 29;
        pFdbSmi[1] |= (pLutSt->spa & 0x3) << 30;

        pFdbSmi[2] = pLutSt->spa >> 2;
        pFdbSmi[2] |= (pLutSt->age & 7) << 2;
        pFdbSmi[2] |= (pLutSt->auth & 1) << 5;
        pFdbSmi[2] |= (pLutSt->nosalearn & 1) << 16;
        
    }
}


static void _rtl8373_fdbStSmi2User( rtl8373_luttb *pLutSt, rtk_uint32 *pFdbSmi)
{
    //rtlglue_printf("0x%x-0x%x-0x%x\n", pFdbSmi[0],pFdbSmi[1],pFdbSmi[2]);
    /*L3 lookup*/
    if(pFdbSmi[1] & 0x10000000)
    {
        pLutSt->sip             = pFdbSmi[0];
        pLutSt->dip             = pFdbSmi[1] & 0xFFFFFFF;
        pLutSt->mbr             = ((pFdbSmi[2] & 0x00FF) << 2) | ((pFdbSmi[1] >> 30) & 3);
        pLutSt->l3lookup        = (pFdbSmi[1] >> 28) & 1;
        pLutSt->igmp_asic        = (pFdbSmi[2] >> 16) & 1;
        pLutSt->igmp_idx        = (pFdbSmi[2] >> 8) & 0xff;
    }
    else if((pFdbSmi[1] >> 8) & 0x01) /*Multicast L2 Lookup*/
    {
        rtlglue_printf("l2 multicast\n");
        pLutSt->mac.octet[5]    = pFdbSmi[0] & 0xFF;
        pLutSt->mac.octet[4]    = (pFdbSmi[0] & 0xFF00) >> 8;
        pLutSt->mac.octet[3]    = (pFdbSmi[0] & 0xFF0000) >> 16;
        pLutSt->mac.octet[2]    = (pFdbSmi[0] & 0xFF000000) >> 24;
        pLutSt->mac.octet[1]    = (pFdbSmi[1] & 0xFF);
        pLutSt->mac.octet[0]    = (pFdbSmi[1] & 0xFF00) >> 8;

        pLutSt->cvid_fid        = (pFdbSmi[1] >> 16) & 0x0FFF;
        pLutSt->l3lookup        = (pFdbSmi[1] >> 28) & 1;
        pLutSt->ivl_svl            = (pFdbSmi[1] >> 29) & 1;
        pLutSt->mbr             = ((pFdbSmi[2] & 0x00FF) << 2) | ((pFdbSmi[1] >> 30) & 3);
        pLutSt->igmp_asic        = (pFdbSmi[2] >> 16) & 1;
        pLutSt->igmp_idx        = (pFdbSmi[2] >> 8) & 0xff;

    }
    else /*Asic auto-learning*/
    {
        pLutSt->mac.octet[5]    = pFdbSmi[0] & 0xFF;
        pLutSt->mac.octet[4]    = (pFdbSmi[0] & 0xFF00) >> 8;
        pLutSt->mac.octet[3]    = (pFdbSmi[0] & 0xFF0000) >> 16;
        pLutSt->mac.octet[2]    = (pFdbSmi[0] & 0xFF000000) >> 24;
        pLutSt->mac.octet[1]    = (pFdbSmi[1] & 0xFF);
        pLutSt->mac.octet[0]    = (pFdbSmi[1] & 0xFF00) >> 8;

        pLutSt->cvid_fid        = (pFdbSmi[1] >> 16) & 0x0FFF;
        pLutSt->l3lookup        = (pFdbSmi[1] >> 28) & 1;
        pLutSt->ivl_svl            = (pFdbSmi[1] >> 29) & 1;
        pLutSt->spa                = ((pFdbSmi[2] & 0x3) << 2) | ((pFdbSmi[1] >> 30) & 3);
        pLutSt->age                = ((pFdbSmi[2] >> 2) & 0x7);
        pLutSt->auth            = ((pFdbSmi[2] >> 5) & 0x1);

        pLutSt->nosalearn       = (pFdbSmi[2] >> 16) & 1;
    }
}



static rtk_api_ret_t _rtl8373_getL2LookupTb(rtk_uint32 method, rtl8373_luttb *pL2Table)
{
#if 1
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint32* accessPtr;
    rtk_uint32 i;
    rtk_uint32 smil2Table[RTL8373_LUT_TABLE_SIZE];
    rtk_uint32 busyCounter;
    rtk_uint32 tblCmd;

    if(pL2Table->wait_time == 0)
        busyCounter = RTL8373_LUT_BUSY_CHECK_NO;
    else
        busyCounter = pL2Table->wait_time;

    while(busyCounter)
    {
        retVal = rtl8373_getAsicRegBit(RTL8373_ITA_CTRL0_ADDR, RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET,&regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        pL2Table->lookup_busy = regData;
        if(!pL2Table->lookup_busy)
            break;

        busyCounter --;
        if(busyCounter == 0)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }

    retVal = rtl8373_setAsicRegBits(RTL8373_ITA_L2_CTRL_ADDR, RTL8373_ITA_L2_CTRL_READ_MTHD_MASK, method);
    if(retVal != RT_ERR_OK)
        return retVal;
#if 0
    retVal = rtl8373_setAsicRegBit(RTL8373_ITA_L2_CTRL_ADDR, RTL8373_ITA_L2_CTRL_ENTRY_CLR_OFFSET, 0);
    if(retVal != RT_ERR_OK)
        return retVal;
#endif
    switch(method)
    {
        case RTL8373_LUTREADMETHOD_ADDRESS:
        case RTL8373_LUTREADMETHOD_NEXT_ADDRESS:
        case RTL8373_LUTREADMETHOD_NEXT_L2UC:
        case RTL8373_LUTREADMETHOD_NEXT_L2MC:
        case RTL8373_LUTREADMETHOD_NEXT_L3MC:
        case RTL8373_LUTREADMETHOD_NEXT_L2L3MC:

            tblCmd = (1|(0 << 1)|(4 << 8) | (pL2Table->address << RTL8373_ITA_CTRL0_TBL_ADDR_OFFSET));
            retVal = rtl8373_setAsicReg(RTL8373_ITA_CTRL0_ADDR, tblCmd);
            if(retVal != RT_ERR_OK)
                return retVal;
            break;
        case RTL8373_LUTREADMETHOD_MAC:
            memset(smil2Table, 0x00, sizeof(rtk_uint32) * RTL8373_LUT_TABLE_SIZE);
            _rtl8373_fdbStUser2Smi(pL2Table, smil2Table);

            accessPtr = smil2Table;
            regData = *accessPtr;

            for(i=0; i<RTL8373_LUT_TABLE_SIZE; i++)
            {
                retVal = rtl8373_setAsicReg(RTL8373_ITA_WRITE_DATA0_ADDR(i), regData);

                if(retVal != RT_ERR_OK)
                    return retVal;

                accessPtr ++;
                regData = *accessPtr;

            }

            tblCmd = (1|(0 << 1)|(4 << 8));
            retVal = rtl8373_setAsicReg(RTL8373_ITA_CTRL0_ADDR, tblCmd);
            
            break;
        case RTL8373_LUTREADMETHOD_NEXT_L2UCSPA:
            retVal = rtl8373_setAsicRegBits(RTL8373_ITA_CTRL0_ADDR, RTL8373_ITA_CTRL0_TBL_ADDR_MASK, pL2Table->address);
            if(retVal != RT_ERR_OK)
                return retVal;
            rtlglue_printf("spa is %d\n",pL2Table->spa);
            retVal = rtl8373_setAsicRegBits(RTL8373_ITA_L2_CTRL_ADDR, RTL8373_ITA_L2_CTRL_PORT_NUM_MASK, pL2Table->spa);
            if(retVal != RT_ERR_OK)
                return retVal;

            tblCmd = (1|(0 << 1)|(4 << 8) | (pL2Table->address << RTL8373_ITA_CTRL0_TBL_ADDR_OFFSET));
            retVal = rtl8373_setAsicReg(RTL8373_ITA_CTRL0_ADDR, tblCmd);
            if(retVal != RT_ERR_OK)
                return retVal;
            rtlglue_printf("tblcmd is 0x%x\n",tblCmd);
            break;
        default:
            return RT_ERR_INPUT;
    }



    if(pL2Table->wait_time == 0)
        busyCounter = RTL8373_LUT_BUSY_CHECK_NO;
    else
        busyCounter = pL2Table->wait_time;

    while(busyCounter)
    {
        retVal = rtl8373_getAsicRegBit(RTL8373_ITA_CTRL0_ADDR, RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET,&regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        pL2Table->lookup_busy = regData;
        if(!pL2Table->lookup_busy)
            break;

        busyCounter --;
        if(busyCounter == 0)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }

    retVal = rtl8373_getAsicRegBit(RTL8373_ITA_L2_CTRL_ADDR, RTL8373_ITA_L2_CTRL_ACT_STS_OFFSET,&regData);
    if(retVal != RT_ERR_OK)
            return retVal;
    pL2Table->lookup_hit = regData;
    if(!pL2Table->lookup_hit)
        return RT_ERR_L2_ENTRY_NOTFOUND;

    /*Read access address*/
    retVal = rtl8373_getAsicRegBits(RTL8373_ITA_L2_CTRL_ADDR, RTL8373_ITA_L2_CTRL_TBL_ADDR_MASK, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    pL2Table->address = (regData & 0xffff);

    retVal = rtl8373_getAsicReg(RTL8373_ITA_L2_CTRL_ADDR, &regData);

    /*read L2 entry */
    memset(smil2Table, 0x00, sizeof(rtk_uint32) * RTL8373_LUT_TABLE_SIZE);

    accessPtr = smil2Table;

    for(i = 0; i < RTL8373_LUT_TABLE_SIZE; i++)
    {
        retVal = rtl8373_getAsicReg(RTL8373_ITA_READ_DATA0_ADDR(i), &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        *accessPtr = regData;

        accessPtr ++;
    }

    _rtl8373_fdbStSmi2User(pL2Table, smil2Table);
#endif 
    return RT_ERR_OK;
}


static rtk_api_ret_t _rtl8373_setL2LookupTb(rtl8373_luttb *pL2Table)
{
#if 1

    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint32 *accessPtr;
    rtk_uint32 i;
    rtk_uint32 smil2Table[RTL8373_LUT_TABLE_SIZE];
    rtk_uint32 tblCmd;
    rtk_uint32 busyCounter;
    memset(smil2Table, 0x00, sizeof(rtk_uint16) * RTL8373_LUT_TABLE_SIZE);
    _rtl8373_fdbStUser2Smi(pL2Table, smil2Table);

    if(pL2Table->wait_time == 0)
        busyCounter = RTL8373_LUT_BUSY_CHECK_NO;
    else
        busyCounter = pL2Table->wait_time;

    while(busyCounter)
    {
        retVal = rtl8373_getAsicRegBit(RTL8373_ITA_CTRL0_ADDR, RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET,&regData);
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

    for(i = 0; i < RTL8373_LUT_TABLE_SIZE; i++)
    {
        regData = *(accessPtr + i);
        retVal = rtl8373_setAsicReg(RTL8373_ITA_WRITE_DATA0_ADDR(i), regData);

        if(retVal != RT_ERR_OK)
            return retVal;
    }
#if 0
    retVal = rtl8373_setAsicRegBit(RTL8373_ITA_L2_CTRL_ADDR, RTL8373_ITA_L2_CTRL_ENTRY_CLR_OFFSET, 0);
    if(retVal != RT_ERR_OK)
        return retVal;
#endif
    tblCmd = (1<<1) | (4 << 8)  | 1;
    /* Write Command */

    retVal = rtl8373_setAsicReg(RTL8373_ITA_CTRL0_ADDR, tblCmd);
    if(retVal != RT_ERR_OK)
        return retVal;

    if(pL2Table->wait_time == 0)
        busyCounter = RTL8373_LUT_BUSY_CHECK_NO;
    else
        busyCounter = pL2Table->wait_time;

    while(busyCounter)
    {
        retVal = rtl8373_getAsicRegBit(RTL8373_ITA_CTRL0_ADDR, RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET,&regData);
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
    retVal = rtl8373_getAsicRegBit(RTL8373_ITA_L2_CTRL_ADDR, RTL8373_ITA_L2_CTRL_ACT_STS_OFFSET, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    pL2Table->lookup_hit = regData;
    if(!pL2Table->lookup_hit)
        return RT_ERR_FAILED;

    /*Read access address*/
    retVal = rtl8373_getAsicRegBits(RTL8373_ITA_L2_CTRL_ADDR, RTL8373_ITA_L2_CTRL_TBL_ADDR_MASK, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    pL2Table->address = (regData & 0xffff);
    pL2Table->lookup_busy = 0;
#endif
    return RT_ERR_OK;
}


static rtk_api_ret_t _rtl8373_clearL2LookupTb(rtk_uint32 index)
{
#if 1

    ret_t retVal;
    rtk_uint32 regData;
    //rtk_uint32 *accessPtr;
    //rtk_uint32 smil2Table[RTL8373_LUT_TABLE_SIZE];
    rtk_uint32 tblCmd;
    rtk_uint32 busyCounter;
    //memset(smil2Table, 0x00, sizeof(rtk_uint16) * RTL8373_LUT_TABLE_SIZE);

    busyCounter = RTL8373_LUT_BUSY_CHECK_NO;

    while(busyCounter)
    {
        retVal = rtl8373_getAsicRegBit(RTL8373_ITA_CTRL0_ADDR, RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET,&regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        if(!regData)
            break;

        busyCounter --;
        if(busyCounter == 0)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }

    //accessPtr = smil2Table;
#if 0
    retVal = rtl8373_setAsicRegBits(RTL8373_ITA_CTRL0_ADDR, RTL8373_ITA_CTRL0_TBL_ADDR_MASK, index);
    if(retVal != RT_ERR_OK)
        return retVal;
#endif
    
    retVal = rtl8373_setAsicRegBit(RTL8373_ITA_L2_CTRL_ADDR, RTL8373_ITA_L2_CTRL_ENTRY_CLR_OFFSET, 1);
    if(retVal != RT_ERR_OK)
        return retVal;


    tblCmd = (index << 16)|(1<<1) | (4 << 8)  | 1;
    /* Write Command */
    retVal = rtl8373_setAsicReg(RTL8373_ITA_CTRL0_ADDR, tblCmd);
    if(retVal != RT_ERR_OK)
        return retVal;
    //rtlglue_printf("cmd data 0x%x\n", tblCmd);

    busyCounter = RTL8373_LUT_BUSY_CHECK_NO;

    while(busyCounter)
    {
        retVal = rtl8373_getAsicRegBit(RTL8373_ITA_CTRL0_ADDR, RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET,&regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        if(!regData)
            break;

        busyCounter --;
        if(busyCounter == 0)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }

#if 1
    retVal = rtl8373_setAsicRegBit(RTL8373_ITA_L2_CTRL_ADDR, RTL8373_ITA_L2_CTRL_ENTRY_CLR_OFFSET, 0);
    if(retVal != RT_ERR_OK)
        return retVal;
#endif 
    /*Read access status*/
    retVal = rtl8373_getAsicRegBit(RTL8373_ITA_L2_CTRL_ADDR, RTL8373_ITA_L2_CTRL_ACT_STS_OFFSET, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

#endif
    return RT_ERR_OK;
}







static rtk_api_ret_t _rtl8373_getLutIPMCGroup(rtk_uint32 index, ipaddr_t *pGroup_addr, rtk_uint32 *pPmask, rtk_uint32 *pValid)
{
    rtk_uint32      regAddr, regData;
    ret_t       retVal;

    if(index > RTL8373_LUT_IPMCGRP_TABLE_MAX)
        return RT_ERR_INPUT;

    if (NULL == pGroup_addr)
        return RT_ERR_NULL_POINTER;

    if (NULL == pPmask)
        return RT_ERR_NULL_POINTER;

    /* Group address */
    regAddr = RTL8373_IPMC_GROUP_DIP_ADDR(index);
    if( (retVal = rtl8373_getAsicReg(regAddr, &regData)) != RT_ERR_OK)
        return retVal;

    *pGroup_addr = regData | 0xE0000000;


    /* portmask */
    regAddr = RTL8373_IPMC_GROUP_PMSK_ADDR(index);
    if( (retVal = rtl8373_getAsicReg(regAddr, &regData)) != RT_ERR_OK)
        return retVal;

    *pPmask = regData;

    /* valid */
    regAddr = RTL8373_IPMC_GROUP_VALID_ADDR(index);
    if( (retVal = rtl8373_getAsicRegBit(regAddr, RTL8373_IPMC_GROUP_VALID_VALID_OFFSET(index), &regData)) != RT_ERR_OK)
        return retVal;

    *pValid = regData;

    return RT_ERR_OK;
}

static rtk_api_ret_t _rtl8373_setLutIPMCGroup(rtk_uint32 index, ipaddr_t group_addr, rtk_uint32 pmask, rtk_uint32 valid)
{
    rtk_uint32  regAddr, regData;
    ipaddr_t    ipData;
    ret_t       retVal;

    if(index > RTL8373_LUT_IPMCGRP_TABLE_MAX)
        return RT_ERR_INPUT;

    ipData = group_addr;

    if( (ipData & 0xF0000000) != 0xE0000000)    /* not in 224.0.0.0 ~ 239.255.255.255 */
        return RT_ERR_INPUT;

    /* Group Address */
    regAddr = RTL8373_IPMC_GROUP_DIP_ADDR(index);

    if( (retVal = rtl8373_setAsicReg(regAddr, ipData)) != RT_ERR_OK)
        return retVal;

    /* portmask */
    regAddr = RTL8373_IPMC_GROUP_PMSK_ADDR(index);
    regData = pmask;

    if( (retVal = rtl8373_setAsicReg(regAddr, regData)) != RT_ERR_OK)
        return retVal;

    /* valid */
    regAddr = RTL8373_IPMC_GROUP_VALID_ADDR(index);
    if( (retVal = rtl8373_setAsicRegBit(regAddr, RTL8373_IPMC_GROUP_VALID_VALID_OFFSET(index), valid)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8373_l2_init
 * Description:
 *      Initialize lut function.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *
 */
rtk_api_ret_t dal_rtl8373_l2_init(void)
{


    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_addr_add
 * Description:
 *      Add LUT unicast entry.
 * Input:
 *      pMac - 6 bytes unicast(I/G bit is 0) mac address to be written into LUT.
 *      pL2_data - Unicast entry parameter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_MAC              - Invalid MAC address.
 *      RT_ERR_L2_FID           - Invalid FID .
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      If the unicast mac address already existed in LUT, it will udpate the status of the entry.
 *      Otherwise, it will find an empty or asic auto learned entry to write. If all the entries
 *      with the same hash value can't be replaced, ASIC will return a RT_ERR_L2_INDEXTBL_FULL error.
 */

rtk_api_ret_t dal_rtl8373_l2_addr_add(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data)
{

    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8373_luttb l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* must be unicast address */
    if ((pMac == NULL) || (pMac->octet[0] & 0x1))
        return RT_ERR_MAC;

    if(pL2_data == NULL)
        return RT_ERR_MAC;

    RTK_CHK_PORT_VALID(pL2_data->port);

    if (pL2_data->ivl >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if (pL2_data->is_static>= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    memset(&l2Table, 0, sizeof(rtl8373_luttb));

    /* fill key (MAC,FID) to get L2 entry */
    memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);
    l2Table.ivl_svl     = pL2_data->ivl;
    l2Table.cvid_fid    = pL2_data->vid_fid;
    method = RTL8373_LUTREADMETHOD_MAC;
    retVal = _rtl8373_getL2LookupTb(method, &l2Table);
    if (RT_ERR_OK == retVal )
    {
        memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);
        l2Table.ivl_svl     = pL2_data->ivl;
        l2Table.cvid_fid    = pL2_data->vid_fid;
        l2Table.spa         = rtk_switch_port_L2P_get(pL2_data->port);
        l2Table.nosalearn   = pL2_data->is_static;
        l2Table.l3lookup    = 0;
        l2Table.age         = 6;
        l2Table.auth        = pL2_data->auth;
        if((retVal = _rtl8373_setL2LookupTb(&l2Table)) != RT_ERR_OK)
            return retVal;

        pL2_data->address = l2Table.address;
        return RT_ERR_OK;
    }
    else if (RT_ERR_L2_ENTRY_NOTFOUND == retVal )
    {
        memset(&l2Table, 0, sizeof(rtl8373_luttb));
        memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);
        l2Table.ivl_svl     = pL2_data->ivl;
        l2Table.cvid_fid    = pL2_data->vid_fid;
        l2Table.spa         = rtk_switch_port_L2P_get(pL2_data->port);
        l2Table.nosalearn   = pL2_data->is_static;
        l2Table.l3lookup    = 0;
        l2Table.age         = 6;
        l2Table.auth        = pL2_data->auth;

        if ((retVal = _rtl8373_setL2LookupTb(&l2Table)) != RT_ERR_OK)
            return retVal;

        pL2_data->address = l2Table.address;

        method = RTL8373_LUTREADMETHOD_MAC;
        retVal = _rtl8373_getL2LookupTb(method, &l2Table);
        if (RT_ERR_L2_ENTRY_NOTFOUND == retVal )
            return RT_ERR_L2_INDEXTBL_FULL;
        else
            return retVal;
    }
    else
        return retVal;

 
}


/* Function Name:
 *      dal_rtl8373_l2_addr_get
 * Description:
 *      Get LUT unicast entry.
 * Input:
 *      pMac    - 6 bytes unicast(I/G bit is 0) mac address to be written into LUT.
 * Output:
 *      pL2_data - Unicast entry parameter
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_MAC                  - Invalid MAC address.
 *      RT_ERR_L2_FID               - Invalid FID .
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      If the unicast mac address existed in LUT, it will return the port and fid where
 *      the mac is learned. Otherwise, it will return a RT_ERR_L2_ENTRY_NOTFOUND error.
 */
rtk_api_ret_t dal_rtl8373_l2_addr_get(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8373_luttb l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* must be unicast address */
    if ((pMac == NULL) || (pMac->octet[0] & 0x1))
        return RT_ERR_MAC;

    if (pL2_data->ivl >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if (pL2_data->ivl == 1)
    {
        if (pL2_data->vid_fid> RTL8373_VIDMAX)
            return RT_ERR_L2_VID;
    }
    else
    {
        if (pL2_data->vid_fid > RTL8373_FIDMAX)
            return RT_ERR_L2_FID;
    }

    memset(&l2Table, 0, sizeof(rtl8373_luttb));

    memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);
    l2Table.ivl_svl     = pL2_data->ivl;
    l2Table.cvid_fid    = pL2_data->vid_fid;
    method = RTL8373_LUTREADMETHOD_MAC;
    //rtlglue_printf("====vid is %d\n", l2Table.cvid_fid );
    if ((retVal = _rtl8373_getL2LookupTb(method, &l2Table)) != RT_ERR_OK)
        return retVal;

    memcpy(pL2_data->mac.octet, pMac->octet,ETHER_ADDR_LEN);
    pL2_data->port      = rtk_switch_port_P2L_get(l2Table.spa);
    pL2_data->ivl       = l2Table.ivl_svl;

    pL2_data->vid_fid      = l2Table.cvid_fid;


    pL2_data->is_static = l2Table.nosalearn;
    pL2_data->address   = l2Table.address;
    pL2_data->auth      = l2Table.auth;
    pL2_data->age       = l2Table.age;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_l2_addr_next_get
 * Description:
 *      Get Next LUT unicast entry.
 * Input:
 *      read_method     - The reading method.
 *      port            - The port number if the read_metohd is READMETHOD_NEXT_L2UCSPA
 *      pAddress        - The Address ID
 * Output:
 *      pL2_data - Unicast entry parameter
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_MAC                  - Invalid MAC address.
 *      RT_ERR_L2_FID               - Invalid FID .
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      Get the next unicast entry after the current entry pointed by pAddress.
 *      The address of next entry is returned by pAddress. User can use (address + 1)
 *      as pAddress to call this API again for dumping all entries is LUT.
 */
 //not use, use dal_rtl8373_l2_entry_getNext
rtk_api_ret_t dal_rtl8373_l2_addr_next_get(rtk_l2_read_method_t read_method, rtk_port_t port, rtk_uint32 *pAddress, rtk_l2_ucastAddr_t *pL2_data)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      method;
    rtl8373_luttb  l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Error Checking */
    if ((pL2_data == NULL) || (pAddress == NULL))
        return RT_ERR_MAC;

    if(read_method == READMETHOD_NEXT_L2UC)
        method = RTL8373_LUTREADMETHOD_NEXT_L2UC;
    else if(read_method == READMETHOD_NEXT_L2UCSPA)
        method = RTL8373_LUTREADMETHOD_NEXT_L2UCSPA;
    else
        return RT_ERR_INPUT;

    if(read_method == READMETHOD_NEXT_L2UCSPA)
    {
        /* Check Port Valid */
        RTK_CHK_PORT_VALID(port);
    }

    if(*pAddress > RTK_MAX_LUT_ADDR_ID )
        return RT_ERR_L2_L2UNI_PARAM;

    memset(pL2_data, 0, sizeof(rtk_l2_ucastAddr_t));
    memset(&l2Table, 0, sizeof(rtl8373_luttb));
    l2Table.address = *pAddress;

    if(read_method == READMETHOD_NEXT_L2UCSPA)
        l2Table.spa = rtk_switch_port_L2P_get(port);

    if ((retVal = _rtl8373_getL2LookupTb(method, &l2Table)) != RT_ERR_OK)
        return retVal;

    if(l2Table.address < *pAddress)
        return RT_ERR_L2_ENTRY_NOTFOUND;

    memcpy(pL2_data->mac.octet, l2Table.mac.octet, ETHER_ADDR_LEN);
    pL2_data->port      = rtk_switch_port_P2L_get(l2Table.spa);
    pL2_data->ivl       = l2Table.ivl_svl;

    pL2_data->vid_fid      = l2Table.cvid_fid;


    pL2_data->is_static = l2Table.nosalearn;
    pL2_data->address   = l2Table.address;
    pL2_data->auth      = l2Table.auth;
    pL2_data->age       = l2Table.age;

    *pAddress = l2Table.address;

    return RT_ERR_OK;

}


/* Function Name:
 *      dal_rtl8373_l2_addr_del
 * Description:
 *      Delete LUT unicast entry.
 * Input:
 *      pMac - 6 bytes unicast(I/G bit is 0) mac address to be written into LUT.
 *      fid - Filtering database
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_MAC                  - Invalid MAC address.
 *      RT_ERR_L2_FID               - Invalid FID .
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      If the mac has existed in the LUT, it will be deleted. Otherwise, it will return RT_ERR_L2_ENTRY_NOTFOUND.
 */
rtk_api_ret_t dal_rtl8373_l2_addr_del(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8373_luttb l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* must be unicast address */
    if ((pMac == NULL) || (pMac->octet[0] & 0x1))
        return RT_ERR_MAC;

    if (pL2_data->ivl >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if (pL2_data->ivl == 1)
    {
        if (pL2_data->vid_fid > RTL8373_VIDMAX)
            return RT_ERR_L2_VID;
    }
    else
    {
        if (pL2_data->vid_fid > RTL8373_FIDMAX)
            return RT_ERR_L2_FID;
    }

    memset(&l2Table, 0, sizeof(rtl8373_luttb));

    /* fill key (MAC,FID) to get L2 entry */
    memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);
    l2Table.ivl_svl     = pL2_data->ivl;
    l2Table.cvid_fid    = pL2_data->vid_fid;
    method = RTL8373_LUTREADMETHOD_MAC;
    retVal = _rtl8373_getL2LookupTb(method, &l2Table);
    if (RT_ERR_OK ==  retVal)
    {
        memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);
        l2Table.ivl_svl     = pL2_data->ivl;
        l2Table.cvid_fid    = pL2_data->vid_fid;
        l2Table.spa         = 0;
        l2Table.nosalearn   = 0;
        l2Table.age         = 0;
        l2Table.auth        = 0;
        if((retVal = _rtl8373_setL2LookupTb(&l2Table)) != RT_ERR_OK)
            return retVal;

        pL2_data->address = l2Table.address;
        return RT_ERR_OK;
    }
    else
        return retVal;
}



/* Function Name:
 *      dal_rtl8373_l2_mcastAddr_add
 * Description:
 *      Add LUT multicast entry.
 * Input:
 *      pMcastAddr  - L2 multicast entry structure
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_MAC              - Invalid MAC address.
 *      RT_ERR_L2_FID           - Invalid FID .
 *      RT_ERR_L2_VID           - Invalid VID .
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_PORT_MASK        - Invalid portmask.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      If the multicast mac address already existed in the LUT, it will udpate the
 *      port mask of the entry. Otherwise, it will find an empty or asic auto learned
 *      entry to write. If all the entries with the same hash value can't be replaced,
 *      ASIC will return a RT_ERR_L2_INDEXTBL_FULL error.
 */
rtk_api_ret_t dal_rtl8373_l2_mcastAddr_add(rtk_l2_mcastAddr_t *pMcastAddr)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      method;
    rtl8373_luttb  l2Table;
    rtk_uint32      pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pMcastAddr)
        return RT_ERR_NULL_POINTER;

    /* must be L2 multicast address */
    if( (pMcastAddr->mac.octet[0] & 0x01) != 0x01)
        return RT_ERR_MAC;

    RTK_CHK_PORTMASK_VALID(&pMcastAddr->portmask);

    if(pMcastAddr->ivl == 1)
    {
        if (pMcastAddr->vid_fid > RTL8373_VIDMAX)
            return RT_ERR_L2_VID;
    }
    else if(pMcastAddr->ivl == 0)
    {
        if (pMcastAddr->vid_fid > RTL8373_FIDMAX)
            return RT_ERR_L2_FID;
    }
    else
        return RT_ERR_INPUT;

    /* Get physical port mask */
    if ((retVal = rtk_switch_portmask_L2P_get(&pMcastAddr->portmask, &pmask)) != RT_ERR_OK)
        return retVal;

    memset(&l2Table, 0, sizeof(rtl8373_luttb));

    /* fill key (MAC,FID) to get L2 entry */
    memcpy(l2Table.mac.octet, pMcastAddr->mac.octet, ETHER_ADDR_LEN);
    l2Table.ivl_svl     = pMcastAddr->ivl;

    l2Table.cvid_fid    = pMcastAddr->vid_fid;


    method = RTL8373_LUTREADMETHOD_MAC;
    retVal = _rtl8373_getL2LookupTb(method, &l2Table);
    if (RT_ERR_OK == retVal)
    {
        memcpy(l2Table.mac.octet, pMcastAddr->mac.octet, ETHER_ADDR_LEN);
        l2Table.ivl_svl     = pMcastAddr->ivl;

        l2Table.cvid_fid    = pMcastAddr->vid_fid;

        l2Table.mbr         = pmask;
        l2Table.igmp_asic   = pMcastAddr->igmp_asic;
        l2Table.igmp_idx    = pMcastAddr->igmp_index;
        l2Table.l3lookup    = 0;
        if((retVal = _rtl8373_setL2LookupTb(&l2Table)) != RT_ERR_OK)
            return retVal;

        pMcastAddr->address = l2Table.address;
        return RT_ERR_OK;
    }
    else if (RT_ERR_L2_ENTRY_NOTFOUND == retVal)
    {
        memset(&l2Table, 0, sizeof(rtl8373_luttb));
        memcpy(l2Table.mac.octet, pMcastAddr->mac.octet, ETHER_ADDR_LEN);
        l2Table.ivl_svl     = pMcastAddr->ivl;
        l2Table.cvid_fid    = pMcastAddr->vid_fid;

        l2Table.mbr         = pmask;
        l2Table.igmp_asic   = pMcastAddr->igmp_asic;
        l2Table.igmp_idx    = pMcastAddr->igmp_index;
        l2Table.l3lookup    = 0;
        if ((retVal = _rtl8373_setL2LookupTb(&l2Table)) != RT_ERR_OK)
            return retVal;

        pMcastAddr->address = l2Table.address;

        method = RTL8373_LUTREADMETHOD_MAC;
        retVal = _rtl8373_getL2LookupTb(method, &l2Table);
        if (RT_ERR_L2_ENTRY_NOTFOUND == retVal)
            return     RT_ERR_L2_INDEXTBL_FULL;
        else
            return retVal;
    }
    else
        return retVal;

}


/* Function Name:
 *      dal_rtl8373_l2_mcastAddr_get
 * Description:
 *      Get LUT multicast entry.
 * Input:
 *      pMcastAddr  - L2 multicast entry structure
 * Output:
 *      pMcastAddr  - L2 multicast entry structure
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_MAC                  - Invalid MAC address.
 *      RT_ERR_L2_FID               - Invalid FID .
 *      RT_ERR_L2_VID               - Invalid VID .
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      If the multicast mac address existed in the LUT, it will return the port where
 *      the mac is learned. Otherwise, it will return a RT_ERR_L2_ENTRY_NOTFOUND error.
 */
rtk_api_ret_t dal_rtl8373_l2_mcastAddr_get(rtk_l2_mcastAddr_t *pMcastAddr)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8373_luttb l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pMcastAddr)
        return RT_ERR_NULL_POINTER;

    /* must be L2 multicast address */
    if( (pMcastAddr->mac.octet[0] & 0x01) != 0x01)
        return RT_ERR_MAC;

    if(pMcastAddr->ivl == 1)
    {
        if (pMcastAddr->vid_fid> RTL8373_VIDMAX)
            return RT_ERR_L2_VID;
    }
    else if(pMcastAddr->ivl == 0)
    {
        if (pMcastAddr->vid_fid > RTL8373_FIDMAX)
            return RT_ERR_L2_FID;
    }
    else
        return RT_ERR_INPUT;

    memset(&l2Table, 0, sizeof(rtl8373_luttb));
    memcpy(l2Table.mac.octet, pMcastAddr->mac.octet, ETHER_ADDR_LEN);
    l2Table.ivl_svl     = pMcastAddr->ivl;

    l2Table.cvid_fid    = pMcastAddr->vid_fid;

    method = RTL8373_LUTREADMETHOD_MAC;

    if ((retVal = _rtl8373_getL2LookupTb(method, &l2Table)) != RT_ERR_OK)
        return retVal;

    pMcastAddr->address     = l2Table.address;
    pMcastAddr->igmp_asic   = l2Table.igmp_asic;
    pMcastAddr->igmp_index  = l2Table.igmp_idx;

    /* Get Logical port mask */
    if ((retVal = rtk_switch_portmask_P2L_get(l2Table.mbr, &pMcastAddr->portmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_mcastAddr_next_get
 * Description:
 *      Get Next L2 Multicast entry.
 * Input:
 *      pAddress        - The Address ID
 * Output:
 *      pMcastAddr  - L2 multicast entry structure
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      Get the next L2 multicast entry after the current entry pointed by pAddress.
 *      The address of next entry is returned by pAddress. User can use (address + 1)
 *      as pAddress to call this API again for dumping all multicast entries is LUT.
 */
rtk_api_ret_t dal_rtl8373_l2_mcastAddr_next_get(rtk_uint32 *pAddress, rtk_l2_mcastAddr_t *pMcastAddr)
{
    rtk_api_ret_t   retVal;
    rtl8373_luttb  l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Error Checking */
    if ((pAddress == NULL) || (pMcastAddr == NULL))
        return RT_ERR_INPUT;

    if(*pAddress > RTK_MAX_LUT_ADDR_ID )
        return RT_ERR_L2_L2UNI_PARAM;

    memset(pMcastAddr, 0, sizeof(rtk_l2_mcastAddr_t));
    memset(&l2Table, 0, sizeof(rtl8373_luttb));
    l2Table.address = *pAddress;

    if ((retVal = _rtl8373_getL2LookupTb(RTL8373_LUTREADMETHOD_NEXT_L2MC, &l2Table)) != RT_ERR_OK)
        return retVal;

    if(l2Table.address < *pAddress)
        return RT_ERR_L2_ENTRY_NOTFOUND;

    memcpy(pMcastAddr->mac.octet, l2Table.mac.octet, ETHER_ADDR_LEN);
    pMcastAddr->ivl     = l2Table.ivl_svl;

    pMcastAddr->vid_fid = l2Table.cvid_fid;

    pMcastAddr->address     = l2Table.address;
    pMcastAddr->igmp_asic   = l2Table.igmp_asic;
    pMcastAddr->igmp_index  = l2Table.igmp_idx;

    /* Get Logical port mask */
    if ((retVal = rtk_switch_portmask_P2L_get(l2Table.mbr, &pMcastAddr->portmask)) != RT_ERR_OK)
        return retVal;

    *pAddress = l2Table.address;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_mcastAddr_del
 * Description:
 *      Delete LUT multicast entry.
 * Input:
 *      pMcastAddr  - L2 multicast entry structure
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_MAC                  - Invalid MAC address.
 *      RT_ERR_L2_FID               - Invalid FID .
 *      RT_ERR_L2_VID               - Invalid VID .
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      If the mac has existed in the LUT, it will be deleted. Otherwise, it will return RT_ERR_L2_ENTRY_NOTFOUND.
 */
rtk_api_ret_t dal_rtl8373_l2_mcastAddr_del(rtk_l2_mcastAddr_t *pMcastAddr)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8373_luttb l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pMcastAddr)
        return RT_ERR_NULL_POINTER;

    /* must be L2 multicast address */
    if( (pMcastAddr->mac.octet[0] & 0x01) != 0x01)
        return RT_ERR_MAC;

    if(pMcastAddr->ivl == 1)
    {
        if (pMcastAddr->vid_fid > RTL8373_VIDMAX)
            return RT_ERR_L2_VID;
    }
    else if(pMcastAddr->ivl == 0)
    {
        if (pMcastAddr->vid_fid > RTL8373_FIDMAX)
            return RT_ERR_L2_FID;
    }
    else
        return RT_ERR_INPUT;

    memset(&l2Table, 0, sizeof(rtl8373_luttb));

    /* fill key (MAC,FID) to get L2 entry */
    memcpy(l2Table.mac.octet, pMcastAddr->mac.octet, ETHER_ADDR_LEN);
    l2Table.ivl_svl     = pMcastAddr->ivl;

    l2Table.cvid_fid    = pMcastAddr->vid_fid;

    method = RTL8373_LUTREADMETHOD_MAC;
    retVal = _rtl8373_getL2LookupTb(method, &l2Table);
    if (RT_ERR_OK == retVal)
    {
#if 0    
        memcpy(l2Table.mac.octet, pMcastAddr->mac.octet, ETHER_ADDR_LEN);
        l2Table.ivl_svl     = pMcastAddr->ivl;

        l2Table.cvid_fid    = pMcastAddr->vid_fid;

        l2Table.mbr         = 0;
        l2Table.igmp_asic   = 0;
        l2Table.igmp_idx    = 0;
        l2Table.l3lookup    = 0;
        if((retVal = _rtl8373_setL2LookupTb(&l2Table)) != RT_ERR_OK)
            return retVal;

        pMcastAddr->address = l2Table.address;
        return RT_ERR_OK;
#endif
        pMcastAddr->address = l2Table.address;
        return _rtl8373_clearL2LookupTb(l2Table.address);

    }
    else
        return retVal;
}


/* Function Name:
 *      dal_rtl8373_l2_ipMcastAddr_add
 * Description:
 *      Add Lut IP multicast entry
 * Input:
 *      pIpMcastAddr    - IP Multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_PORT_MASK        - Invalid portmask.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      System supports L2 entry with IP multicast DIP/SIP to forward IP multicasting frame as user
 *      desired. If this function is enabled, then system will be looked up L2 IP multicast entry to
 *      forward IP multicast frame directly without flooding.
 */
rtk_api_ret_t dal_rtl8373_l2_ipMcastAddr_add(rtk_l2_ipMcastAddr_t *pIpMcastAddr)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8373_luttb l2Table;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pIpMcastAddr)
        return RT_ERR_NULL_POINTER;

    /* check port mask */
    RTK_CHK_PORTMASK_VALID(&pIpMcastAddr->portmask);

    if( (pIpMcastAddr->dip & 0xF0000000) != 0xE0000000)
        return RT_ERR_INPUT;

    /* Get Physical port mask */
    if ((retVal = rtk_switch_portmask_L2P_get(&pIpMcastAddr->portmask, &pmask)) != RT_ERR_OK)
        return retVal;

    memset(&l2Table, 0x00, sizeof(rtl8373_luttb));
    l2Table.sip = pIpMcastAddr->sip;
    l2Table.dip = pIpMcastAddr->dip;
    l2Table.l3lookup = 1;
    l2Table.igmp_asic = pIpMcastAddr->igmp_asic;
    l2Table.igmp_idx = pIpMcastAddr->igmp_index;
    method = RTL8373_LUTREADMETHOD_MAC;
    retVal = _rtl8373_getL2LookupTb(method, &l2Table);
    if (RT_ERR_OK == retVal)
    {
        l2Table.sip = pIpMcastAddr->sip;
        l2Table.dip = pIpMcastAddr->dip;
        l2Table.mbr = pmask;
        l2Table.igmp_asic = pIpMcastAddr->igmp_asic;
        l2Table.igmp_idx = pIpMcastAddr->igmp_index;
        l2Table.l3lookup = 1;
        if((retVal = _rtl8373_setL2LookupTb(&l2Table)) != RT_ERR_OK)
            return retVal;

        pIpMcastAddr->address = l2Table.address;
        return RT_ERR_OK;
    }
    else if (RT_ERR_L2_ENTRY_NOTFOUND == retVal)
    {
        memset(&l2Table, 0, sizeof(rtl8373_luttb));
        l2Table.sip = pIpMcastAddr->sip;
        l2Table.dip = pIpMcastAddr->dip;
        l2Table.mbr = pmask;
        l2Table.igmp_asic = pIpMcastAddr->igmp_asic;
        l2Table.igmp_idx = pIpMcastAddr->igmp_index;
        l2Table.l3lookup = 1;
        if ((retVal = _rtl8373_setL2LookupTb(&l2Table)) != RT_ERR_OK)
            return retVal;

        pIpMcastAddr->address = l2Table.address;

        method = RTL8373_LUTREADMETHOD_MAC;
        retVal = _rtl8373_getL2LookupTb(method, &l2Table);
        if (RT_ERR_L2_ENTRY_NOTFOUND == retVal)
            return     RT_ERR_L2_INDEXTBL_FULL;
        else
            return retVal;

    }
    else
        return retVal;

}


/* Function Name:
 *      dal_rtl8373_l2_ipMcastAddr_get
 * Description:
 *      Get LUT IP multicast entry.
 * Input:
 *      pIpMcastAddr    - IP Multicast entry
 * Output:
 *      pIpMcastAddr    - IP Multicast entry
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      The API can get Lut table of IP multicast entry.
 */
rtk_api_ret_t dal_rtl8373_l2_ipMcastAddr_get(rtk_l2_ipMcastAddr_t *pIpMcastAddr)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8373_luttb l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pIpMcastAddr)
        return RT_ERR_NULL_POINTER;

    if( (pIpMcastAddr->dip & 0xF0000000) != 0xE0000000)
        return RT_ERR_INPUT;

    memset(&l2Table, 0x00, sizeof(rtl8373_luttb));
    l2Table.sip = pIpMcastAddr->sip;
    l2Table.dip = pIpMcastAddr->dip;
    l2Table.l3lookup = 1;
    method = RTL8373_LUTREADMETHOD_MAC;
    if ((retVal = _rtl8373_getL2LookupTb(method, &l2Table)) != RT_ERR_OK)
        return retVal;

    /* Get Logical port mask */
    if ((retVal = rtk_switch_portmask_P2L_get(l2Table.mbr, &pIpMcastAddr->portmask)) != RT_ERR_OK)
        return retVal;

    pIpMcastAddr->address       = l2Table.address;
    pIpMcastAddr->igmp_asic     = l2Table.igmp_asic;
    pIpMcastAddr->igmp_index    = l2Table.igmp_idx;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_ipMcastAddr_next_get
 * Description:
 *      Get Next IP Multicast entry.
 * Input:
 *      pAddress        - The Address ID
 * Output:
 *      pIpMcastAddr    - IP Multicast entry
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      Get the next IP multicast entry after the current entry pointed by pAddress.
 *      The address of next entry is returned by pAddress. User can use (address + 1)
 *      as pAddress to call this API again for dumping all IP multicast entries is LUT.
 */
rtk_api_ret_t dal_rtl8373_l2_ipMcastAddr_next_get(rtk_uint32 *pAddress, rtk_l2_ipMcastAddr_t *pIpMcastAddr)
{
    rtk_api_ret_t   retVal;
    rtl8373_luttb  l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Error Checking */
    if ((pAddress == NULL) || (pIpMcastAddr == NULL) )
        return RT_ERR_INPUT;

    if(*pAddress > RTK_MAX_LUT_ADDR_ID )
        return RT_ERR_L2_L2UNI_PARAM;

    memset(pIpMcastAddr, 0, sizeof(rtk_l2_ipMcastAddr_t));
    memset(&l2Table, 0, sizeof(rtl8373_luttb));
    l2Table.address = *pAddress;

    if ((retVal = _rtl8373_getL2LookupTb(RTL8373_LUTREADMETHOD_NEXT_L3MC, &l2Table)) != RT_ERR_OK)
        return retVal;

    if(l2Table.address < *pAddress)
        return RT_ERR_L2_ENTRY_NOTFOUND;

    pIpMcastAddr->sip = l2Table.sip;
    pIpMcastAddr->dip = l2Table.dip;

    /* Get Logical port mask */
    if ((retVal = rtk_switch_portmask_P2L_get(l2Table.mbr, &pIpMcastAddr->portmask)) != RT_ERR_OK)
        return retVal;

    pIpMcastAddr->address       = l2Table.address;
    pIpMcastAddr->igmp_asic     = l2Table.igmp_asic;
    pIpMcastAddr->igmp_index    = l2Table.igmp_idx;
    *pAddress = l2Table.address;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_l2_ipMcastAddr_del
 * Description:
 *      Delete a ip multicast address entry from the specified device.
 * Input:
 *      pIpMcastAddr    - IP Multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      The API can delete a IP multicast address entry from the specified device.
 */
rtk_api_ret_t dal_rtl8373_l2_ipMcastAddr_del(rtk_l2_ipMcastAddr_t *pIpMcastAddr)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8373_luttb l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Error Checking */
    if (pIpMcastAddr == NULL)
        return RT_ERR_INPUT;

    if( (pIpMcastAddr->dip & 0xF0000000) != 0xE0000000)
        return RT_ERR_INPUT;

    memset(&l2Table, 0x00, sizeof(rtl8373_luttb));
    l2Table.sip = pIpMcastAddr->sip;
    l2Table.dip = pIpMcastAddr->dip;
    l2Table.l3lookup = 1;
    method = RTL8373_LUTREADMETHOD_MAC;
    retVal = _rtl8373_getL2LookupTb(method, &l2Table);
    if (RT_ERR_OK == retVal)
    {
#if 0
        l2Table.sip = pIpMcastAddr->sip;
        l2Table.dip = pIpMcastAddr->dip;
        l2Table.mbr = 0;
        l2Table.igmp_asic = 0;
        l2Table.igmp_idx= 0;
        l2Table.l3lookup = 0;
        if((retVal = _rtl8373_setL2LookupTb(&l2Table)) != RT_ERR_OK)
            return retVal;

        pIpMcastAddr->address = l2Table.address;
        return RT_ERR_OK;
#endif
        pIpMcastAddr->address = l2Table.address;
        return _rtl8373_clearL2LookupTb(l2Table.address);

    }
    else
        return retVal;
}



/* Function Name:
 *      dal_rtl8373_l2_ucastAddr_flush
 * Description:
 *      Flush L2 mac address by type in the specified device (both dynamic and static).
 * Input:
 *      pConfig - flush configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      flushByVid          - 1: Flush by VID, 0: Don't flush by VID
 *      vid                 - VID (0 ~ 4095)
 *      flushByFid          - 1: Flush by FID, 0: Don't flush by FID
 *      fid                 - FID (0 ~ 15)
 *      flushByPort         - 1: Flush by Port, 0: Don't flush by Port
 *      port                - Port ID
 *      flushByMac          - Not Supported
 *      ucastAddr           - Not Supported
 *      flushStaticAddr     - 1: Flush both Static and Dynamic entries, 0: Flush only Dynamic entries
 *      flushAddrOnAllPorts - 1: Flush VID-matched entries at all ports, 0: Flush VID-matched entries per port.
 */
rtk_api_ret_t dal_rtl8373_l2_ucastAddr_flush(rtk_l2_flushCfg_t *pConfig)
{
    rtk_api_ret_t   retVal;
    rtk_uint32 regData, i;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(pConfig == NULL)
        return RT_ERR_NULL_POINTER;

    if(pConfig->flushByVid >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if(pConfig->flushByFid >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if(pConfig->flushByPort >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if(pConfig->flushStaticAddr >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if(pConfig->vid > RTL8373_VIDMAX)
        return RT_ERR_VLAN_VID;

    if(pConfig->fid > RTL8373_FIDMAX)
        return RT_ERR_INPUT;


    

    if(pConfig->flushByVid == ENABLED)
    {
        if ((retVal = rtl8373_setAsicRegBits(RTL8373_L2_TBL_FLUSH_MODE_ADDR, RTL8373_L2_TBL_FLUSH_MODE_FLUSH_MODE_MASK, FLUSHMDOE_VID)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8373_setAsicRegBits(RTL8373_L2_TBL_FLUSH_XID_ADDR, RTL8373_L2_TBL_FLUSH_XID_FLUSH_VID_MASK, pConfig->vid)) != RT_ERR_OK)
                return retVal;

        if ((retVal = rtl8373_setAsicRegBits(RTL8373_L2_TBL_FLUSH_CMD_ADDR, RTL8373_L2_TBL_FLUSH_CMD_FLUSH_PMSK_MASK, pConfig->portmask)) != RT_ERR_OK)
            return retVal;

    }
    else if(pConfig->flushByFid == ENABLED)
    {
        if ((retVal = rtl8373_setAsicRegBits(RTL8373_L2_TBL_FLUSH_MODE_ADDR, RTL8373_L2_TBL_FLUSH_MODE_FLUSH_MODE_MASK, FLUSHMDOE_FID)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8373_setAsicRegBits(RTL8373_L2_TBL_FLUSH_XID_ADDR, RTL8373_L2_TBL_FLUSH_XID_FLUSH_FID_MASK, pConfig->fid)) != RT_ERR_OK)
                return retVal;

        if ((retVal = rtl8373_setAsicRegBits(RTL8373_L2_TBL_FLUSH_CMD_ADDR, RTL8373_L2_TBL_FLUSH_CMD_FLUSH_PMSK_MASK, pConfig->portmask)) != RT_ERR_OK)
            return retVal;

    }
    else if(pConfig->flushByPort == ENABLED)
    {
        if ((retVal = rtl8373_setAsicRegBits(RTL8373_L2_TBL_FLUSH_MODE_ADDR, RTL8373_L2_TBL_FLUSH_MODE_FLUSH_MODE_MASK,FLUSHMDOE_PORT))!= RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8373_setAsicRegBits(RTL8373_L2_TBL_FLUSH_CMD_ADDR, RTL8373_L2_TBL_FLUSH_CMD_FLUSH_PMSK_MASK, pConfig->portmask)) != RT_ERR_OK)
            return retVal;
    }


    if(pConfig->flushStaticAddr)
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_L2_TBL_FLUSH_MODE_ADDR, RTL8373_L2_TBL_FLUSH_MODE_FLUSH_TYPE_OFFSET,1)) != RT_ERR_OK)
            return retVal;
    }
    else
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_L2_TBL_FLUSH_MODE_ADDR, RTL8373_L2_TBL_FLUSH_MODE_FLUSH_TYPE_OFFSET,0)) != RT_ERR_OK)
            return retVal;
    }


    retVal = rtl8373_getAsicRegBit(RTL8373_L2_TBL_FLUSH_CMD_ADDR, RTL8373_L2_TBL_FLUSH_CMD_FLUSH_BUSY_OFFSET, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;
    
    if(regData & 1)
    {
        return RT_ERR_BUSYWAIT_TIMEOUT;
    }

    retVal = rtl8373_setAsicRegBit(RTL8373_L2_TBL_FLUSH_CMD_ADDR, RTL8373_L2_TBL_FLUSH_CMD_FLUSH_ACT_OFFSET, 1);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_L2_TBL_FLUSH_CMD_ADDR, RTL8373_L2_TBL_FLUSH_CMD_FLUSH_BUSY_OFFSET, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    i = 0;
    while(regData & 1)
    {
        retVal = rtl8373_getAsicRegBit(RTL8373_L2_TBL_FLUSH_CMD_ADDR, RTL8373_L2_TBL_FLUSH_CMD_FLUSH_BUSY_OFFSET, &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        i++;

        if(i > 0xfff)
        {
            return RT_ERR_BUSYWAIT_TIMEOUT;
        }
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_table_clear
 * Description:
 *      Flush all static & dynamic entries in LUT.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *
 */
rtk_api_ret_t dal_rtl8373_l2_table_clear(void)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_L2_TBL_FLUSH_ALL_ADDR, RTL8373_L2_TBL_FLUSH_ALL_FLUSH_ALL_OFFSET, 1)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_l2_table_clearStatus_get
 * Description:
 *      Get table clear status
 * Input:
 *      None
 * Output:
 *      pStatus - Clear status, 1:Busy, 0:finish
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *
 */
rtk_api_ret_t dal_rtl8373_l2_table_clearStatus_get(rtk_l2_clearStatus_t *pStatus)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pStatus)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_L2_TBL_FLUSH_ALL_ADDR, RTL8373_L2_TBL_FLUSH_ALL_FLUSH_ALL_OFFSET, (rtk_uint32 *)pStatus)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_flushLinkDownPortAddrEnable_set
 * Description:
 *      Set HW flush linkdown port mac configuration of the specified device.
 * Input:
 *      enable - link down flush status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      The status of flush linkdown port address is as following:
 *      - DISABLED
 *      - ENABLED
 */
rtk_api_ret_t dal_rtl8373_l2_flushLinkDownPortAddrEnable_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();


    if (enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_L2_CTRL_ADDR, RTL8373_L2_CTRL_LINKDOWN_AGEOUT_OFFSET, enable ? 0 : 1)) != RT_ERR_OK)
        return retVal;


    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_flushLinkDownPortAddrEnable_get
 * Description:
 *      Get HW flush linkdown port mac configuration of the specified device.
 * Input:
 * Output:
 *      pEnable - link down flush status
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The status of flush linkdown port address is as following:
 *      - DISABLED
 *      - ENABLED
 */
rtk_api_ret_t dal_rtl8373_l2_flushLinkDownPortAddrEnable_get(rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    rtk_uint32  value;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_L2_CTRL_ADDR, RTL8373_L2_CTRL_LINKDOWN_AGEOUT_OFFSET, &value)) != RT_ERR_OK)
        return retVal;

    *pEnable = value ? 0 : 1;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_agingEnable_set
 * Description:
 *      Set L2 LUT aging status per port setting.
 * Input:
 *      port    - Port id.
 *      enable  - Aging status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can be used to set L2 LUT aging status per port.
 */
rtk_api_ret_t dal_rtl8373_l2_agingEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* check port valid */
    RTK_CHK_PORT_VALID(port);

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if(enable == 1)
        enable = 0;
    else
        enable = 1;

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_L2_PORT_AGE_CTRL_ADDR(port), RTL8373_L2_PORT_AGE_CTRL_DIS_AGE_OFFSET(port), enable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8373_l2_agingEnable_get
 * Description:
 *      Get L2 LUT aging status per port setting.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - Aging status
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can be used to get L2 LUT aging function per port.
 */
rtk_api_ret_t dal_rtl8373_l2_agingEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* check port valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_L2_PORT_AGE_CTRL_ADDR(port), RTL8373_L2_PORT_AGE_CTRL_DIS_AGE_OFFSET(port), pEnable)) != RT_ERR_OK)
        return retVal;

    if(*pEnable == 1)
        *pEnable = 0;
    else
        *pEnable = 1;

    return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8373_l2_ageout_timer_set
 * Description:
 *      Set L2 LUT entry age out timer.
 * Input:
 *      timer  - the time of an lut entry ageout without update.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can be used to set L2 LUT aging status per port.
 */
rtk_api_ret_t dal_rtl8373_l2_ageout_timer_set(rtk_uint32 timer)
{
    rtk_api_ret_t retVal;
    rtk_uint32 age_uint;

    age_uint = timer * 5;

    if((retVal = rtl8373_setAsicRegBits(RTL8373_L2_AGE_CTRL_ADDR, RTL8373_L2_AGE_CTRL_AGE_UNIT_MASK, age_uint))!= RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_l2_ageout_timer_get
 * Description:
 *      Get L2 LUT entry age out timer.
 * Input:
 *      pTimer  - the time of an lut entry ageout without update.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can be used to set L2 LUT aging status per port.
 */
rtk_api_ret_t dal_rtl8373_l2_ageout_timer_get(rtk_uint32 *pTimer)
{
    rtk_api_ret_t retVal;
    rtk_uint32 age_uint;


    if((retVal = rtl8373_getAsicRegBits(RTL8373_L2_AGE_CTRL_ADDR, RTL8373_L2_AGE_CTRL_AGE_UNIT_MASK, &age_uint))!= RT_ERR_OK)
        return retVal;

    *pTimer = age_uint/5;

    return RT_ERR_OK;


}



/* Function Name:
 *      dal_rtl8373_l2_ageout_timer_set
 * Description:
 *      Set L2 LUT entry age field value when address learned in lut table.
 * Input:
 *      timer  - the time of an lut entry ageout without update.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can be used to set L2 LUT aging status per port.
 */
rtk_api_ret_t dal_rtl8373_l2_agefield_value_set(rtk_uint32 value)
{
    rtk_api_ret_t retVal;

  
    if((retVal = rtl8373_setAsicRegBits(RTL8373_L2_CTRL_ADDR, RTL8373_L2_CTRL_AGE_TIMER_MASK, value))!= RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_l2_agefield_value_get
 * Description:
 *      Get L2 LUT entry age field value when address learned in lut table.
 * Input:
 *      pTimer  - the time of an lut entry ageout without update.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can be used to set L2 LUT aging status per port.
 */
rtk_api_ret_t dal_rtl8373_l2_agefield_value_get(rtk_uint32 *pValue)
{
    rtk_api_ret_t retVal;
    
    if((retVal = rtl8373_getAsicRegBits(RTL8373_L2_CTRL_ADDR, RTL8373_L2_CTRL_AGE_TIMER_MASK, pValue))!= RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_l2_limitLearningCnt_set
 * Description:
 *      Set per-Port auto learning limit number
 * Input:
 *      port    - Port id.
 *      mac_cnt - Auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_LIMITED_L2ENTRY_NUM  - Invalid auto learning limit number
 * Note:
 *      The API can set per-port ASIC auto learning limit number from 0(disable learning)
 *      to 2112.
 */
rtk_api_ret_t dal_rtl8373_l2_limitLearningCnt_set(rtk_port_t port, rtk_mac_cnt_t mac_cnt)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* check port valid */
    RTK_CHK_PORT_VALID(port);

    if (mac_cnt > rtk_switch_maxLutAddrNumber_get())
        return RT_ERR_LIMITED_L2ENTRY_NUM;

    if ((retVal = rtl8373_setAsicReg(RTL8373_L2_LRN_PORT_CONSTRT_CTRL_ADDR(port), mac_cnt)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_limitLearningCnt_get
 * Description:
 *      Get per-Port auto learning limit number
 * Input:
 *      port - Port id.
 * Output:
 *      pMac_cnt - Auto learning entries limit number
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get per-port ASIC auto learning limit number.
 */
rtk_api_ret_t dal_rtl8373_l2_limitLearningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* check port valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pMac_cnt)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicReg(RTL8373_L2_LRN_PORT_CONSTRT_CTRL_ADDR(port), pMac_cnt)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_limitSystemLearningCnt_set
 * Description:
 *      Set System auto learning limit number
 * Input:
 *      mac_cnt - Auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_LIMITED_L2ENTRY_NUM  - Invalid auto learning limit number
 * Note:
 *      The API can set system ASIC auto learning limit number from 0(disable learning)
 *      to 2112.
 */
rtk_api_ret_t dal_rtl8373_l2_limitSystemLearningCnt_set(rtk_mac_cnt_t mac_cnt)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (mac_cnt > (rtk_switch_maxLutAddrNumber_get() + 64))
        return RT_ERR_LIMITED_L2ENTRY_NUM;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_L2_LRN_CONSTRT_CTRL_ADDR, RTL8373_L2_LRN_CONSTRT_CTRL_CONSTRT_NUM_MASK, mac_cnt)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_limitSystemLearningCnt_get
 * Description:
 *      Get System auto learning limit number
 * Input:
 *      None
 * Output:
 *      pMac_cnt - Auto learning entries limit number
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get system ASIC auto learning limit number.
 */
rtk_api_ret_t dal_rtl8373_l2_limitSystemLearningCnt_get(rtk_mac_cnt_t *pMac_cnt)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pMac_cnt)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_L2_LRN_CONSTRT_CTRL_ADDR, RTL8373_L2_LRN_CONSTRT_CTRL_CONSTRT_NUM_MASK, pMac_cnt)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_limitLearningCntAction_set
 * Description:
 *      Configure auto learn over limit number action.
 * Input:
 *      port - Port id.
 *      action - Auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_NOT_ALLOWED  - Invalid learn over action
 * Note:
 *      The API can set SA unknown packet action while auto learn limit number is over
 *      The action symbol as following:
 *      - LIMIT_LEARN_CNT_ACTION_DROP,
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD,
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU,
 */
rtk_api_ret_t dal_rtl8373_l2_limitLearningCntAction_set(rtk_port_t port, rtk_l2_limitLearnCntAction_t action)
{
    rtk_api_ret_t retVal;
    rtk_uint32 data;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

	if(port > RTL8373_PORTIDMAX)
		return RT_ERR_INPUT;


    if ( LIMIT_LEARN_CNT_ACTION_DROP == action )
        data = 1;
    else if ( LIMIT_LEARN_CNT_ACTION_FORWARD == action )
        data = 0;
    else if ( LIMIT_LEARN_CNT_ACTION_TO_CPU == action )
        data = 2;
    else
        return RT_ERR_NOT_ALLOWED;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_L2_LRN_PORT_CONSTRT_ACT_ADDR, RTL8373_L2_LRN_PORT_CONSTRT_ACT_LRN_ACT_MASK, data)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_limitLearningCntAction_get
 * Description:
 *      Get auto learn over limit number action.
 * Input:
 *      port - Port id.
 * Output:
 *      pAction - Learn over action
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get SA unknown packet action while auto learn limit number is over
 *      The action symbol as following:
 *      - LIMIT_LEARN_CNT_ACTION_DROP,
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD,
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU,
 */
rtk_api_ret_t dal_rtl8373_l2_limitLearningCntAction_get(rtk_port_t port, rtk_l2_limitLearnCntAction_t *pAction)
{
    rtk_api_ret_t retVal;
    rtk_uint32 action;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

	if(port > RTL8373_PORTIDMAX)
		return RT_ERR_INPUT;

    if(NULL == pAction)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_L2_LRN_PORT_CONSTRT_ACT_ADDR, RTL8373_L2_LRN_PORT_CONSTRT_ACT_LRN_ACT_MASK, &action)) != RT_ERR_OK)
        return retVal;

    if ( 1 == action )
        *pAction = LIMIT_LEARN_CNT_ACTION_DROP;
    else if ( 0 == action )
        *pAction = LIMIT_LEARN_CNT_ACTION_FORWARD;
    else if ( 2 == action )
        *pAction = LIMIT_LEARN_CNT_ACTION_TO_CPU;
    else
    *pAction = action;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_l2_limitSystemLearningCntAction_set
 * Description:
 *      Configure system auto learn over limit number action.
 * Input:
 *      port - Port id.
 *      action - Auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_NOT_ALLOWED  - Invalid learn over action
 * Note:
 *      The API can set SA unknown packet action while auto learn limit number is over
 *      The action symbol as following:
 *      - LIMIT_LEARN_CNT_ACTION_DROP,
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD,
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU,
 */
rtk_api_ret_t dal_rtl8373_l2_limitSystemLearningCntAction_set(rtk_l2_limitLearnCntAction_t action)
{
    rtk_api_ret_t retVal;
    rtk_uint32 data;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ( LIMIT_LEARN_CNT_ACTION_DROP == action )
        data = 1;
    else if ( LIMIT_LEARN_CNT_ACTION_FORWARD == action )
        data = 0;
    else if ( LIMIT_LEARN_CNT_ACTION_TO_CPU == action )
        data = 2;
    else
        return RT_ERR_NOT_ALLOWED;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_L2_LRN_CONSTRT_ACT_ADDR, RTL8373_L2_LRN_CONSTRT_ACT_LRN_ACT_MASK, data)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_limitSystemLearningCntAction_get
 * Description:
 *      Get system auto learn over limit number action.
 * Input:
 *      None.
 * Output:
 *      pAction - Learn over action
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get SA unknown packet action while auto learn limit number is over
 *      The action symbol as following:
 *      - LIMIT_LEARN_CNT_ACTION_DROP,
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD,
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU,
 */
rtk_api_ret_t dal_rtl8373_l2_limitSystemLearningCntAction_get(rtk_l2_limitLearnCntAction_t *pAction)
{
    rtk_api_ret_t retVal;
    rtk_uint32 action;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pAction)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_L2_LRN_CONSTRT_ACT_ADDR, RTL8373_L2_LRN_CONSTRT_ACT_LRN_ACT_MASK, &action)) != RT_ERR_OK)
        return retVal;

    if ( 1 == action )
        *pAction = LIMIT_LEARN_CNT_ACTION_DROP;
    else if ( 0 == action )
        *pAction = LIMIT_LEARN_CNT_ACTION_FORWARD;
    else if ( 2 == action )
        *pAction = LIMIT_LEARN_CNT_ACTION_TO_CPU;
    else
    *pAction = action;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_limitSystemLearningCntPortMask_set
 * Description:
 *      Configure system auto learn portmask
 * Input:
 *      pPortmask - Port Mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK    - Invalid port mask.
 * Note:
 *
 */
rtk_api_ret_t dal_rtl8373_l2_limitSystemLearningCntPortMask_set(rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    /* Check port mask */
    RTK_CHK_PORTMASK_VALID(pPortmask);

    if ((retVal = rtk_switch_portmask_L2P_get(pPortmask, &pmask)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_L2_LRN_CONSTRT_CTRL_ADDR, RTL8373_L2_LRN_CONSTRT_CTRL_PORT_MASK_MASK, pmask & 0x3ff)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_limitSystemLearningCntPortMask_get
 * Description:
 *      get system auto learn portmask
 * Input:
 *      None
 * Output:
 *      pPortmask - Port Mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NULL_POINTER - Null pointer.
 * Note:
 *
 */
rtk_api_ret_t dal_rtl8373_l2_limitSystemLearningCntPortMask_get(rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_L2_LRN_CONSTRT_CTRL_ADDR, RTL8373_L2_LRN_CONSTRT_CTRL_PORT_MASK_MASK, &pmask)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtk_switch_portmask_P2L_get(pmask, pPortmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8367d_l2_learningCnt_get
 * Description:
 *      Get per-Port current auto learning number
 * Input:
 *      port - Port id.
 * Output:
 *      pMac_cnt - ASIC auto learning entries number
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get per-port ASIC auto learning number
 */
rtk_api_ret_t dal_rtl8373_l2_learningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* check port valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pMac_cnt)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicReg(RTL8373_L2_LRN_PORT_CONSTRT_CNT_ADDR(port), pMac_cnt)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_ipMcastAddrLookup_set
 * Description:
 *      Set Lut IP multicast lookup function
 * Input:
 *      type - Lookup type for IPMC packet.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 * Note:
 *      LOOKUP_MAC      - Lookup by MAC address
 *      LOOKUP_IP       - Lookup by IP address
 */
rtk_api_ret_t dal_rtl8373_l2_ipMcastAddrLookup_set(rtk_l2_ipmc_lookup_type_t type)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(type == LOOKUP_MAC)
    {
        if((retVal = rtl8373_setAsicRegBit(RTL8373_L2_CTRL_ADDR, RTL8373_L2_CTRL_LUT_IPMC_HASH_OFFSET, DISABLED)) != RT_ERR_OK)
            return retVal;
    }
    else if(type == LOOKUP_IP)
    {
        if((retVal = rtl8373_setAsicRegBit(RTL8373_L2_CTRL_ADDR, RTL8373_L2_CTRL_LUT_IPMC_HASH_OFFSET, ENABLED)) != RT_ERR_OK)
            return retVal;
    }
    else
        return RT_ERR_INPUT;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_ipMcastAddrLookup_get
 * Description:
 *      Get Lut IP multicast lookup function
 * Input:
 *      None.
 * Output:
 *      pType - Lookup type for IPMC packet.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 * Note:
 *      None.
 */
rtk_api_ret_t dal_rtl8373_l2_ipMcastAddrLookup_get(rtk_l2_ipmc_lookup_type_t *pType)
{
    rtk_api_ret_t       retVal;
    rtk_uint32          enabled;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pType)
        return RT_ERR_NULL_POINTER;

    if((retVal = rtl8373_getAsicRegBit(RTL8373_L2_CTRL_ADDR, RTL8373_L2_CTRL_LUT_IPMC_HASH_OFFSET, &enabled)) != RT_ERR_OK)
        return retVal;

    if(enabled == ENABLED)
        *pType = LOOKUP_IP;
    else
        *pType = LOOKUP_MAC;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_l2_ipMcastGroupEntry_add
 * Description:
 *      Add an IP Multicast entry to group table
 * Input:
 *      ip_addr     - IP address
 *      vid         - VLAN ID
 *      pPortmask   - portmask
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 *      RT_ERR_TBL_FULL    - Table Full
 * Note:
 *      Add an entry to IP Multicast Group table.
 */
rtk_api_ret_t dal_rtl8373_l2_ipMcastGroupEntry_add(ipaddr_t ip_addr, rtk_portmask_t * portmask)
{
    rtk_uint32      empty_idx = 0xFFFF;
    rtk_int32       index;
    ipaddr_t        group_addr;
    rtk_uint32      pmask;
    rtk_uint32      valid;
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();


    if((ip_addr & 0xF0000000) != 0xE0000000)
        return RT_ERR_INPUT;


    for(index = 0; index <= RTL8373_LUT_IPMCGRP_TABLE_MAX; index++)
    {
        if ((retVal = _rtl8373_getLutIPMCGroup((rtk_uint32)index, &group_addr, &pmask, &valid))!=RT_ERR_OK)
            return retVal;

        if( (valid == ENABLED) && (group_addr == ip_addr))
        {
            if(pmask != portmask->bits[0])
            {
                pmask = portmask->bits[0];
                if ((retVal = _rtl8373_setLutIPMCGroup(index, ip_addr, pmask, valid))!=RT_ERR_OK)
                    return retVal;
            }

            return RT_ERR_OK;
        }

        if( (valid == DISABLED) && (empty_idx == 0xFFFF) ) /* Unused */
            empty_idx = (rtk_uint32)index;
    }

    if(empty_idx == 0xFFFF)
        return RT_ERR_TBL_FULL;

    pmask = portmask->bits[0];
    if ((retVal = _rtl8373_setLutIPMCGroup(empty_idx, ip_addr, pmask, ENABLED))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_ipMcastGroupEntry_del
 * Description:
 *      Delete an entry from IP Multicast group table
 * Input:
 *      ip_addr     - IP address
 *      vid         - VLAN ID
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 *      RT_ERR_TBL_FULL    - Table Full
 * Note:
 *      Delete an entry from IP Multicast group table.
 */
rtk_api_ret_t dal_rtl8373_l2_ipMcastGroupEntry_del(ipaddr_t ip_addr)
{
    rtk_int32       index;
    ipaddr_t        group_addr;
    rtk_uint32      pmask;
    rtk_uint32      valid;
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();


    if((ip_addr & 0xF0000000) != 0xE0000000)
        return RT_ERR_INPUT;

    for(index = 0; index <= RTL8373_LUT_IPMCGRP_TABLE_MAX; index++)
    {
        if ((retVal = _rtl8373_getLutIPMCGroup((rtk_uint32)index, &group_addr, &pmask, &valid))!=RT_ERR_OK)
            return retVal;

        if( (valid == ENABLED) && (group_addr == ip_addr) )
        {
            group_addr = 0xE0000000;
            pmask = 0;
            if ((retVal = _rtl8373_setLutIPMCGroup(index, group_addr, pmask, DISABLED))!=RT_ERR_OK)
                return retVal;

            return RT_ERR_OK;
        }
    }

    return RT_ERR_FAILED;
}


/* Function Name:
 *      dal_rtl8373_l2_ipMcastGroupEntry_get
 * Description:
 *      get an entry from IP Multicast group table
 * Input:
 *      ip_addr     - IP address
 *      vid         - VLAN ID
 * Output:
 *      pPortmask   - member port mask
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 *      RT_ERR_TBL_FULL    - Table Full
 * Note:
 *      Delete an entry from IP Multicast group table.
 */
rtk_api_ret_t dal_rtl8373_l2_ipMcastGroupEntry_get(ipaddr_t ip_addr, rtk_portmask_t *pPortmask)
{
    rtk_int32       index;
    ipaddr_t        group_addr;
    rtk_uint32      valid;
    rtk_uint32      pmask;
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();


    if((ip_addr & 0xF0000000) != 0xE0000000)
        return RT_ERR_INPUT;

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    for(index = 0; index <= RTL8373_LUT_IPMCGRP_TABLE_MAX; index++)
    {
        if ((retVal = _rtl8373_getLutIPMCGroup((rtk_uint32)index, &group_addr, &pmask, &valid))!=RT_ERR_OK)
            return retVal;

        if( (valid == ENABLED) && (group_addr == ip_addr) )
        {
            pPortmask->bits[0]= pmask;

            return RT_ERR_OK;
        }
    }

    return RT_ERR_FAILED;
}


/* Function Name:
 *      dal_rtl8373_l2_entry_get
 * Description:
 *      Get LUT entry.
 * Input:
 *      pL2_entry - Index field in the structure.
 * Output:
 *      pL2_entry - other fields such as MAC, port, age...
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_L2_EMPTY_ENTRY   - Empty LUT entry.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      This API is used to get address by index from 0~2111.
 */
rtk_api_ret_t dal_rtl8373_l2_entry_get(rtk_l2_addr_table_t *pL2_entry)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8373_luttb l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (pL2_entry->index >= rtk_switch_maxLutAddrNumber_get())
        return RT_ERR_INPUT;

    memset(&l2Table, 0x00, sizeof(rtl8373_luttb));
    l2Table.address= pL2_entry->index;
    method = RTL8373_LUTREADMETHOD_ADDRESS;
    if ((retVal = _rtl8373_getL2LookupTb(method, &l2Table)) != RT_ERR_OK)
        return retVal;


    if ((pL2_entry->index>4160)&&(l2Table.lookup_hit==0))
         return RT_ERR_L2_EMPTY_ENTRY;

    if(l2Table.l3lookup)
    {
        memset(&pL2_entry->mac, 0, sizeof(rtk_mac_t));
        pL2_entry->is_ipmul  = l2Table.l3lookup;
        pL2_entry->sip       = l2Table.sip;
        pL2_entry->dip       = l2Table.dip;
        pL2_entry->igmp_asic = l2Table.igmp_asic;
        pL2_entry->igmp_idx  = l2Table.igmp_idx;

        /* Get Logical port mask */
        if ((retVal = rtk_switch_portmask_P2L_get(l2Table.mbr, &(pL2_entry->portmask)))!=RT_ERR_OK)
            return retVal;

        pL2_entry->vid_fid       = 0;
        pL2_entry->age       = 0;
    }
    else if(l2Table.mac.octet[0]&0x01)
    {
        memset(&pL2_entry->sip, 0, sizeof(ipaddr_t));
        memset(&pL2_entry->dip, 0, sizeof(ipaddr_t));
        pL2_entry->mac.octet[0] = l2Table.mac.octet[0];
        pL2_entry->mac.octet[1] = l2Table.mac.octet[1];
        pL2_entry->mac.octet[2] = l2Table.mac.octet[2];
        pL2_entry->mac.octet[3] = l2Table.mac.octet[3];
        pL2_entry->mac.octet[4] = l2Table.mac.octet[4];
        pL2_entry->mac.octet[5] = l2Table.mac.octet[5];
        pL2_entry->is_ipmul  = l2Table.l3lookup;
        pL2_entry->igmp_asic = l2Table.igmp_asic;
        pL2_entry->igmp_idx  = l2Table.igmp_idx;

        /* Get Logical port mask */
        if ((retVal = rtk_switch_portmask_P2L_get(l2Table.mbr, &(pL2_entry->portmask)))!=RT_ERR_OK)
            return retVal;

        pL2_entry->ivl       = l2Table.ivl_svl;
        if(l2Table.ivl_svl == 1) /* IVL */
        {
            pL2_entry->vid_fid= l2Table.cvid_fid;
        }
        else /* SVL*/
        {
            pL2_entry->vid_fid       = l2Table.cvid_fid;
        }
        pL2_entry->age       = 0;
    }
    else if((l2Table.age != 0)||(l2Table.nosalearn == 1)||(l2Table.auth== 1))
    {
        memset(&pL2_entry->sip, 0, sizeof(ipaddr_t));
        memset(&pL2_entry->dip, 0, sizeof(ipaddr_t));
        pL2_entry->mac.octet[0] = l2Table.mac.octet[0];
        pL2_entry->mac.octet[1] = l2Table.mac.octet[1];
        pL2_entry->mac.octet[2] = l2Table.mac.octet[2];
        pL2_entry->mac.octet[3] = l2Table.mac.octet[3];
        pL2_entry->mac.octet[4] = l2Table.mac.octet[4];
        pL2_entry->mac.octet[5] = l2Table.mac.octet[5];
        pL2_entry->is_ipmul  = l2Table.l3lookup;
        pL2_entry->is_static = l2Table.nosalearn;

        /* Get Logical port mask */
        if ((retVal = rtk_switch_portmask_P2L_get(1<<(l2Table.spa), &(pL2_entry->portmask)))!=RT_ERR_OK)
            return retVal;

        pL2_entry->ivl       = l2Table.ivl_svl;
        pL2_entry->vid_fid     = l2Table.cvid_fid;
#if 0
        if(l2Table.ivl_svl == 1) /* IVL */
        {
            pL2_entry->cvid      = l2Table.cvid_fid;
            pL2_entry->fid       = 0;
        }
        else /* SVL*/
        {
            pL2_entry->cvid      = 0;
            pL2_entry->fid       = l2Table.cvid_fid;
        }
#endif
        pL2_entry->age       = l2Table.age;
        pL2_entry->auth      = l2Table.auth;
        pL2_entry->srcport   = l2Table.spa;

    }
    else
       return RT_ERR_L2_EMPTY_ENTRY;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_entry_getNext
 * Description:
 *      Get LUT entry.
 * Input:
 *      pL2_entry - Index field in the structure.
 * Output:
 *      pL2_entry - other fields such as MAC, port, age...
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_L2_EMPTY_ENTRY   - Empty LUT entry.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      This API is used to get address by index from 0~2111.
 */
rtk_api_ret_t dal_rtl8373_l2_entry_getNext(rtk_l2_addr_table_t *pL2_entry, rtk_uint32 method)
{
    rtk_api_ret_t retVal;
    rtl8373_luttb l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (pL2_entry->index >= rtk_switch_maxLutAddrNumber_get())
        return RT_ERR_INPUT;

    memset(&l2Table, 0x00, sizeof(rtl8373_luttb));
    l2Table.address= pL2_entry->index;
    l2Table.spa = pL2_entry->srcport;
    if ((retVal = _rtl8373_getL2LookupTb(method, &l2Table)) != RT_ERR_OK)
        return retVal;

#if 0
    if ((l2Table.address>4160)&&(l2Table.lookup_hit==0))
         return RT_ERR_L2_EMPTY_ENTRY;
#endif
    if ((pL2_entry->index>4160)&&(l2Table.lookup_hit==0))
         return RT_ERR_L2_EMPTY_ENTRY;
    
    
    pL2_entry->index = l2Table.address;

    if(l2Table.l3lookup)
    {
        memset(&pL2_entry->mac, 0, sizeof(rtk_mac_t));
        pL2_entry->is_ipmul  = l2Table.l3lookup;
        pL2_entry->sip       = l2Table.sip;
        pL2_entry->dip       = l2Table.dip;
        pL2_entry->igmp_asic = l2Table.igmp_asic;
        pL2_entry->igmp_idx  = l2Table.igmp_idx;

        /* Get Logical port mask */
        if ((retVal = rtk_switch_portmask_P2L_get(l2Table.mbr, &(pL2_entry->portmask)))!=RT_ERR_OK)
            return retVal;

        pL2_entry->vid_fid       = 0;
        pL2_entry->age       = 0;
        
    }
    else if(l2Table.mac.octet[0]&0x01)
    {
        memset(&pL2_entry->sip, 0, sizeof(ipaddr_t));
        memset(&pL2_entry->dip, 0, sizeof(ipaddr_t));
        pL2_entry->mac.octet[0] = l2Table.mac.octet[0];
        pL2_entry->mac.octet[1] = l2Table.mac.octet[1];
        pL2_entry->mac.octet[2] = l2Table.mac.octet[2];
        pL2_entry->mac.octet[3] = l2Table.mac.octet[3];
        pL2_entry->mac.octet[4] = l2Table.mac.octet[4];
        pL2_entry->mac.octet[5] = l2Table.mac.octet[5];
        pL2_entry->is_ipmul  = l2Table.l3lookup;
        pL2_entry->igmp_asic = l2Table.igmp_asic;
        pL2_entry->igmp_idx  = l2Table.igmp_idx;

        /* Get Logical port mask */
        if ((retVal = rtk_switch_portmask_P2L_get(l2Table.mbr, &(pL2_entry->portmask)))!=RT_ERR_OK)
            return retVal;

        pL2_entry->ivl       = l2Table.ivl_svl;
        if(l2Table.ivl_svl == 1) /* IVL */
        {
            pL2_entry->vid_fid= l2Table.cvid_fid;
        }
        else /* SVL*/
        {
            pL2_entry->vid_fid       = l2Table.cvid_fid;
        }
        pL2_entry->age       = 0;
    }
    else if((l2Table.age != 0)||(l2Table.nosalearn == 1)||(l2Table.auth== 1))
    {
        memset(&pL2_entry->sip, 0, sizeof(ipaddr_t));
        memset(&pL2_entry->dip, 0, sizeof(ipaddr_t));
        pL2_entry->mac.octet[0] = l2Table.mac.octet[0];
        pL2_entry->mac.octet[1] = l2Table.mac.octet[1];
        pL2_entry->mac.octet[2] = l2Table.mac.octet[2];
        pL2_entry->mac.octet[3] = l2Table.mac.octet[3];
        pL2_entry->mac.octet[4] = l2Table.mac.octet[4];
        pL2_entry->mac.octet[5] = l2Table.mac.octet[5];
        pL2_entry->is_ipmul  = l2Table.l3lookup;
        pL2_entry->is_static = l2Table.nosalearn;

        /* Get Logical port mask */
        if ((retVal = rtk_switch_portmask_P2L_get(1<<(l2Table.spa), &(pL2_entry->portmask)))!=RT_ERR_OK)
            return retVal;

        pL2_entry->ivl       = l2Table.ivl_svl;
        pL2_entry->vid_fid     = l2Table.cvid_fid;
#if 0
        if(l2Table.ivl_svl == 1) /* IVL */
        {
            pL2_entry->cvid      = l2Table.cvid_fid;
            pL2_entry->fid       = 0;
        }
        else /* SVL*/
        {
            pL2_entry->cvid      = 0;
            pL2_entry->fid       = l2Table.cvid_fid;
        }
#endif
        pL2_entry->age       = l2Table.age;
        pL2_entry->auth      = l2Table.auth;
        pL2_entry->srcport   = l2Table.spa;

    }
    else
       return RT_ERR_L2_EMPTY_ENTRY;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_l2_entry_del
 * Description:
 *      Del LUT  entry.
 * Input:
 *      pL2_entry - Index field in the structure.
 * Output:
 *      pL2_entry - other fields such as MAC, port, age...
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_L2_EMPTY_ENTRY   - Empty LUT entry.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      This API is used to get address by index from 0~4096.
 */
rtk_api_ret_t dal_rtl8373_l2_entry_del(rtk_l2_addr_table_t *pL2_entry)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8373_luttb l2Table;
    rtk_l2_ipMcastAddr_t ipMul;
    rtk_l2_mcastAddr_t l2Mul;
    rtk_l2_ucastAddr_t l2Uni;
    rtk_mac_t macAddr;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (pL2_entry->index >= rtk_switch_maxLutAddrNumber_get())
        return RT_ERR_INPUT;

    memset(&l2Table, 0x00, sizeof(rtl8373_luttb));
    l2Table.address= pL2_entry->index;
    method = RTL8373_LUTREADMETHOD_ADDRESS;
    if ((retVal = _rtl8373_getL2LookupTb(method, &l2Table)) != RT_ERR_OK)
        return retVal;


    if ((pL2_entry->index>4160)&&(l2Table.lookup_hit==0))
         return RT_ERR_L2_EMPTY_ENTRY;

    if(l2Table.l3lookup)
    {
        memset(&pL2_entry->mac, 0, sizeof(rtk_mac_t));
        pL2_entry->is_ipmul  = l2Table.l3lookup;
        pL2_entry->sip       = l2Table.sip;
        pL2_entry->dip       = l2Table.dip;
        pL2_entry->igmp_asic = l2Table.igmp_asic;
        pL2_entry->igmp_idx  = l2Table.igmp_idx;

        /* Get Logical port mask */
        if ((retVal = rtk_switch_portmask_P2L_get(l2Table.mbr, &(pL2_entry->portmask)))!=RT_ERR_OK)
            return retVal;

        pL2_entry->vid_fid       = 0;
        pL2_entry->age       = 0;

        memset(&ipMul, 0, sizeof(rtk_l2_ipMcastAddr_t));
        ipMul.dip = pL2_entry->dip;
        ipMul.sip = pL2_entry->sip;
//        retVal = dal_rtl8373_l2_ipMcastAddr_del(&ipMul);

//        return retVal;
        
    }
    else if(l2Table.mac.octet[0]&0x01)
    {
        memset(&pL2_entry->sip, 0, sizeof(ipaddr_t));
        memset(&pL2_entry->dip, 0, sizeof(ipaddr_t));
        pL2_entry->mac.octet[0] = l2Table.mac.octet[0];
        pL2_entry->mac.octet[1] = l2Table.mac.octet[1];
        pL2_entry->mac.octet[2] = l2Table.mac.octet[2];
        pL2_entry->mac.octet[3] = l2Table.mac.octet[3];
        pL2_entry->mac.octet[4] = l2Table.mac.octet[4];
        pL2_entry->mac.octet[5] = l2Table.mac.octet[5];
        pL2_entry->is_ipmul  = l2Table.l3lookup;
        pL2_entry->igmp_asic = l2Table.igmp_asic;
        pL2_entry->igmp_idx  = l2Table.igmp_idx;

        /* Get Logical port mask */
        if ((retVal = rtk_switch_portmask_P2L_get(l2Table.mbr, &(pL2_entry->portmask)))!=RT_ERR_OK)
            return retVal;

        pL2_entry->ivl       = l2Table.ivl_svl;
        if(l2Table.ivl_svl == 1) /* IVL */
        {
            pL2_entry->vid_fid= l2Table.cvid_fid;
        }
        else /* SVL*/
        {
            pL2_entry->vid_fid       = l2Table.cvid_fid;
        }
        pL2_entry->age       = 0;

        memset(&l2Mul, 0, sizeof(rtk_l2_mcastAddr_t));
        l2Mul.mac.octet[0] = l2Table.mac.octet[0];
        l2Mul.mac.octet[1] = l2Table.mac.octet[1];
        l2Mul.mac.octet[2] = l2Table.mac.octet[2];
        l2Mul.mac.octet[3] = l2Table.mac.octet[3];
        l2Mul.mac.octet[4] = l2Table.mac.octet[4];
        l2Mul.mac.octet[5] = l2Table.mac.octet[5];
        l2Mul.ivl = l2Table.ivl_svl;
        l2Mul.vid_fid = l2Table.cvid_fid;
//        retVal = dal_rtl8373_l2_mcastAddr_del(&l2Mul);

//        return retVal;        
    }
    else if((l2Table.age != 0)||(l2Table.nosalearn == 1)||(l2Table.auth== 1))
    {
        memset(&pL2_entry->sip, 0, sizeof(ipaddr_t));
        memset(&pL2_entry->dip, 0, sizeof(ipaddr_t));
        pL2_entry->mac.octet[0] = l2Table.mac.octet[0];
        pL2_entry->mac.octet[1] = l2Table.mac.octet[1];
        pL2_entry->mac.octet[2] = l2Table.mac.octet[2];
        pL2_entry->mac.octet[3] = l2Table.mac.octet[3];
        pL2_entry->mac.octet[4] = l2Table.mac.octet[4];
        pL2_entry->mac.octet[5] = l2Table.mac.octet[5];
        pL2_entry->is_ipmul  = l2Table.l3lookup;
        pL2_entry->is_static = l2Table.nosalearn;

        /* Get Logical port mask */
        if ((retVal = rtk_switch_portmask_P2L_get(1<<(l2Table.spa), &(pL2_entry->portmask)))!=RT_ERR_OK)
            return retVal;

        pL2_entry->ivl       = l2Table.ivl_svl;
        pL2_entry->vid_fid     = l2Table.cvid_fid;
        pL2_entry->age       = l2Table.age;
        pL2_entry->auth      = l2Table.auth;
        pL2_entry->srcport   = l2Table.spa;

        memset(&l2Uni, 0, sizeof(l2Uni));
        memset(&macAddr, 0, sizeof(macAddr));

        macAddr.octet[0] = l2Table.mac.octet[0];
        macAddr.octet[1] = l2Table.mac.octet[1];
        macAddr.octet[2] = l2Table.mac.octet[2];
        macAddr.octet[3] = l2Table.mac.octet[3];
        macAddr.octet[4] = l2Table.mac.octet[4];
        macAddr.octet[5] = l2Table.mac.octet[5];

        l2Uni.ivl = l2Table.ivl_svl;
        l2Uni.vid_fid = l2Table.cvid_fid;

//        retVal = dal_rtl8373_l2_addr_del(&macAddr, &l2Uni);

//        return retVal;

    }

    return _rtl8373_clearL2LookupTb(pL2_entry->index);
    
}



/* Function Name:
 *      dal_rtl8373_l2_lookupHitIsolationAction_set
 * Description:
 *      Set action of lookup hit & isolation.
 * Input:
 *      action          - The action
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      This API is used to configure the action of packet which is lookup hit
 *      in L2 table but the destination port/portmask are not in the port isolation
 *      group.
 */
rtk_api_ret_t dal_rtl8373_l2_lookupHitIsolationAction_set(rtk_l2_lookupHitIsolationAction_t action)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    switch (action)
    {
        case L2_LOOKUPHIT_ISOACTION_NOP:
            regData = 0;
            break;
        case L2_LOOKUPHIT_ISOACTION_UNKNOWN:
            regData = 1;
            break;
        default:
            return RT_ERR_INPUT;
    }

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_L2_CTRL_ADDR, RTL8373_L2_CTRL_CFG_LOOKUP_HIT_ISO_ACT_OFFSET, regData)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}





/* Function Name:
 *      dal_rtl8373_l2_lookupHitIsolationAction_get
 * Description:
 *      Get action of lookup hit & isolation.
 * Input:
 *      None.
 * Output:
 *      pAction         - The action
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_NULL_POINTER         - Null pointer
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      This API is used to get the action of packet which is lookup hit
 *      in L2 table but the destination port/portmask are not in the port isolation
 *      group.
 */
rtk_api_ret_t dal_rtl8373_l2_lookupHitIsolationAction_get(rtk_l2_lookupHitIsolationAction_t *pAction)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (pAction == NULL)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_L2_CTRL_ADDR, RTL8373_L2_CTRL_CFG_LOOKUP_HIT_ISO_ACT_OFFSET, &regData)) != RT_ERR_OK)
        return retVal;

    switch (regData)
    {
        case 0:
            *pAction = L2_LOOKUPHIT_ISOACTION_NOP;
            break;
        case 1:
            *pAction = L2_LOOKUPHIT_ISOACTION_UNKNOWN;
            break;
        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_portNewSaBehavior_set
 * Description:
 *      Set UNSA behavior
 * Input:
 *      port           - port 0~9
 *      behavior    - 0: flooding; 1: drop; 2:trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NOT_ALLOWED  - Invalid behavior
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_portNewSaBehavior_set(rtk_uint32 port, rtk_uint32 behavior)
{
    if(behavior >= L2_BEHAVE_SA_END)
        return RT_ERR_NOT_ALLOWED;

    return rtl8373_setAsicRegBits(RTL8373_L2_NEWSA_CTRL_ADDR(port), RTL8373_L2_NEWSA_CTRL_NEW_SA_MASK(port), behavior);
}


/* Function Name:
 *      dal_rtl8373_l2_portNewSaBehavior_get
 * Description:
 *      Get UNSA behavior
 * Input:
 *      port           - port 0~9
 *      pBehavior   - 0: flooding; 1: drop; 2:trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_portNewSaBehavior_get(rtk_uint32 port, rtk_uint32 *pBehavior)
{
    return rtl8373_getAsicRegBits(RTL8373_L2_NEWSA_CTRL_ADDR(port), RTL8373_L2_NEWSA_CTRL_NEW_SA_MASK(port), pBehavior);
}


/* Function Name:
 *      dal_rtl8373_l2_portUnmatchedSaBehavior_set
 * Description:
 *      Set Unmatched SA behavior
 * Input:
 *      port           - port 0~9
 *      behavior    - 0: flooding; 1: drop; 2:trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NOT_ALLOWED  - Invalid behavior
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_portUnmatchedSaBehavior_set(rtk_uint32 port, rtk_uint32 behavior)
{
    if(behavior >= L2_BEHAVE_SA_END)
        return RT_ERR_NOT_ALLOWED;

    return rtl8373_setAsicRegBits(RTL8373_L2_UNMATCH_SA_CTRL_ADDR(port), RTL8373_L2_UNMATCH_SA_CTRL_UNMATCH_SA_MASK(port), behavior);
}
/* Function Name:
 *      dal_rtl8373_l2_portUnmatchedSaBehavior_get
 * Description:
 *      Get Unmatched SA behavior
 * Input:
 *      port           - port 0~9
 *      pBehavior   - 0: flooding; 1: drop; 2:trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_portUnmatchedSaBehavior_get(rtk_uint32 port, rtk_uint32 *pBehavior)
{
    return rtl8373_getAsicRegBits(RTL8373_L2_UNMATCH_SA_CTRL_ADDR(port), RTL8373_L2_UNMATCH_SA_CTRL_UNMATCH_SA_MASK(port), pBehavior);
}



/* Function Name:
 *      dal_rtl8373_l2_portSaMovingForbid_set
 * Description:
 *      Set Unmatched SA moving state
 * Input:
 *      port        - Port ID
 *      enabled     - 0: can't move to new port; 1: can move to new port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_portSaMovingForbid_set(rtk_uint32 port, rtk_uint32 forbid)
{
    if(port >= RTL8373_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8373_setAsicRegBit(RTL8373_L2_SA_MOVING_FORBID_ADDR(port), RTL8373_L2_SA_MOVING_FORBID_FORBID_OFFSET(port), forbid);
}


/* Function Name:
 *      dal_rtl8373_getAsicPortUnmatchedSaMoving
 * Description:
 *      Get Unmatched SA moving state
 * Input:
 *      port        - Port ID
 * Output:
 *      pEnabled    - 0: can't move to new port; 1: can move to new port
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_portSaMovingForbid_get(rtk_uint32 port, rtk_uint32 *pForbid)
{
    ret_t retVal;

    if(port >= RTL8373_PORTNO)
        return RT_ERR_PORT_ID;

    if((retVal = rtl8373_getAsicRegBit(RTL8373_L2_SA_MOVING_FORBID_ADDR(port), RTL8373_L2_SA_MOVING_FORBID_FORBID_OFFSET(port), pForbid)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_srcPortPermit_set
 * Description:
 *      Set Unmatched SA moving state
 * Input:
 *      port        - Port ID
 *      enabled     - 0: pkt cannot tx from rx port; 1: pkt can tx from rx port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_srcPortPermit_set(rtk_uint32 port, rtk_uint32 enable)
{
    if(port >= RTL8373_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8373_setAsicRegBit(RTL8373_SOURCE_PORT_PERMIT_ADDR(port), RTL8373_SOURCE_PORT_PERMIT_SRC_PERMIT_EN_OFFSET(port), enable);
}


/* Function Name:
 *      dal_rtl8373_srcPortPermit_get
 * Description:
 *      Get Unmatched SA moving state
 * Input:
 *      port        - Port ID
 * Output:
 *      pEnabled    -0: pkt cannot tx from rx port; 1: pkt can tx from rx port
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_srcPortPermit_get(rtk_uint32 port, rtk_uint32 *pEnable)
{
    ret_t retVal;

    if(port >= RTL8373_PORTNO)
        return RT_ERR_PORT_ID;

    if((retVal = rtl8373_getAsicRegBit(RTL8373_SOURCE_PORT_PERMIT_ADDR(port), RTL8373_SOURCE_PORT_PERMIT_SRC_PERMIT_EN_OFFSET(port), pEnable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_unknUc_fldMsk_set
 * Description:
 *      Set unicast pkt lookup miss action
 * Input:
 *      portmask        - l2 unicast pkt lookup miss flood portmask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_unknUc_fldMsk_set(rtk_uint32 portMask)
{
    ret_t retVal;


    if((retVal = rtl8373_setAsicReg(RTL8373_L2_UNKN_UC_FLD_PMSK_ADDR, portMask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_unknUc_fldMsk_get
 * Description:
 *      get unicast pkt lookup miss action
 * Input:
 *      portmask        - l2 unicast pkt lookup miss flood portmask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_unknUc_fldMsk_get(rtk_uint32* pMask)
{
    ret_t retVal;


    if((retVal = rtl8373_getAsicReg(RTL8373_L2_UNKN_UC_FLD_PMSK_ADDR, pMask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}





/* Function Name:
 *      dal_rtl8373_l2_unknUc_action_set
 * Description:
 *      Set unicast pkt lookup miss action
 * Input:
 *      port        - Port ID
 *      pAction  - 0b00:fwd in L2_UNKNOW_UC_FLD_PMSK  0b01 drop  0b10 trap  0b11 flood
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_unknUc_action_set(rtk_uint32 port, rtk_uint32 action)
{
    ret_t retVal;

    if(port >= RTL8373_PORTNO)
        return RT_ERR_PORT_ID;

    if((retVal = rtl8373_setAsicRegBits(RTL8373_L2_PORT_UC_LM_ACT_ADDR(port), RTL8373_L2_PORT_UC_LM_ACT_ACT_MASK(port), action)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_l2_unknUc_action_get
 * Description:
 *      Get unicast pkt lookup miss action
 * Input:
 *      port        - Port ID
 *      pAction  - 0b00:fwd in L2_UNKNOW_UC_FLD_PMSK  0b01 drop  0b10 trap  0b11 flood
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_unknUc_action_get(rtk_uint32 port, rtk_uint32 *pAction)
{
    ret_t retVal;

    if(port >= RTL8373_PORTNO)
        return RT_ERR_PORT_ID;

    if((retVal = rtl8373_getAsicRegBits(RTL8373_L2_PORT_UC_LM_ACT_ADDR(port), RTL8373_L2_PORT_UC_LM_ACT_ACT_MASK(port), pAction)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_unknMc_fldMsk_set
 * Description:
 *      Set multicast pkt lookup miss action
 * Input:
 *      portmask        - l2 multicast pkt lookup miss flood portmask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_unknMc_fldMsk_set(rtk_uint32 portMask)
{
    ret_t retVal;


    if((retVal = rtl8373_setAsicReg(RTL8373_L2_UNKN_MC_FLD_PMSK_ADDR, portMask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_unknMc_fldMsk_get
 * Description:
 *      get multicast pkt lookup miss action
 * Input:
 *      portmask        - l2 multicast pkt lookup miss flood portmask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_unknMc_fldMsk_get(rtk_uint32* pMask)
{
    ret_t retVal;


    if((retVal = rtl8373_getAsicReg(RTL8373_L2_UNKN_MC_FLD_PMSK_ADDR, pMask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}





/* Function Name:
 *      dal_rtl8373_l2_unknMc_action_set
 * Description:
 *      Set multicast pkt lookup miss action
 * Input:
 *      port        - Port ID
 *      pAction  - 0b00:fwd in L2_UNKNOW_MC_FLD_PMSK  0b01 drop  0b10 trap  0b11 drop exclude RMA
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_unknMc_action_set(rtk_uint32 port, rtk_uint32 action)
{
    ret_t retVal;

    if(port >= RTL8373_PORTNO)
        return RT_ERR_PORT_ID;

    if((retVal = rtl8373_setAsicRegBits(RTL8373_L2_PORT_MC_LM_ACT_ADDR(port), RTL8373_L2_PORT_MC_LM_ACT_ACT_MASK(port), action)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_l2_unknMc_action_get
 * Description:
 *      Get multicast pkt lookup miss action
 * Input:
 *      port        - Port ID
 *      pAction  - 0b00:fwd in L2_UNKNOW_MC_FLD_PMSK  0b01 drop  0b10 trap  0b11 drop exclude RMA
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_unknMc_action_get(rtk_uint32 port, rtk_uint32 *pAction)
{
    ret_t retVal;

    if(port >= RTL8373_PORTNO)
        return RT_ERR_PORT_ID;

    if((retVal = rtl8373_getAsicRegBits(RTL8373_L2_PORT_MC_LM_ACT_ADDR(port), RTL8373_L2_PORT_MC_LM_ACT_ACT_MASK(port), pAction)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_l2_unknV4Mc_fldMsk_set
 * Description:
 *      Set ipv4 multicast pkt lookup miss action
 * Input:
 *      portmask        - ipv4 multicast pkt lookup miss flood portmask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_unknV4Mc_fldMsk_set(rtk_uint32 portMask)
{
    ret_t retVal;


    if((retVal = rtl8373_setAsicReg(RTL8373_IPV4_UNKN_MC_FLD_PMSK_ADDR, portMask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_unknV4Mc_fldMsk_get
 * Description:
 *      get ipv4 multicast pkt lookup miss action
 * Input:
 *      portmask        - ipv4 multicast pkt lookup miss flood portmask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_unknV4Mc_fldMsk_get(rtk_uint32* pMask)
{
    ret_t retVal;


    if((retVal = rtl8373_getAsicReg(RTL8373_IPV4_UNKN_MC_FLD_PMSK_ADDR, pMask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}





/* Function Name:
 *      dal_rtl8373_l2_unknV4Mc_action_set
 * Description:
 *      Set ipv4 multicast pkt lookup miss action
 * Input:
 *      port        - Port ID
 *      action  - 0b00:fwd in IPV4_UNKNOW_MC_FLD_PMSK  0b01 drop  0b10 trap  0b11 to router port
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_unknV4Mc_action_set(rtk_uint32 port, rtk_uint32 action)
{
    ret_t retVal;

    if(port >= RTL8373_PORTNO)
        return RT_ERR_PORT_ID;

    if((retVal = rtl8373_setAsicRegBits(RTL8373_IPV4_PORT_MC_LM_ACT_ADDR(port), RTL8373_IPV4_PORT_MC_LM_ACT_ACT_MASK(port), action)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_l2_unknV4Mc_action_get
 * Description:
 *      Get ipv4 multicast pkt lookup miss action
 * Input:
 *      port        - Port ID
 *      pAction  - 0b00:fwd in IPV4_UNKNOW_MC_FLD_PMSK  0b01 drop  0b10 trap  0b11 to router port
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_unknV4Mc_action_get(rtk_uint32 port, rtk_uint32 *pAction)
{
    ret_t retVal;

    if(port >= RTL8373_PORTNO)
        return RT_ERR_PORT_ID;

    if((retVal = rtl8373_getAsicRegBits(RTL8373_IPV4_PORT_MC_LM_ACT_ADDR(port), RTL8373_IPV4_PORT_MC_LM_ACT_ACT_MASK(port), pAction)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8373_l2_unknV6Mc_fldMsk_set
 * Description:
 *      Set ipv6 multicast pkt lookup miss action
 * Input:
 *      portmask        - ipv6 multicast pkt lookup miss flood portmask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_unknV6Mc_fldMsk_set(rtk_uint32 portMask)
{
    ret_t retVal;


    if((retVal = rtl8373_setAsicReg(RTL8373_IPV6_UNKN_MC_FLD_PMSK_ADDR, portMask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_unknV6Mc_fldMsk_get
 * Description:
 *      get ipv6 multicast pkt lookup miss action
 * Input:
 *      portmask        - ipv6 multicast pkt lookup miss flood portmask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_unknV6Mc_fldMsk_get(rtk_uint32* pMask)
{
    ret_t retVal;


    if((retVal = rtl8373_getAsicReg(RTL8373_IPV6_UNKN_MC_FLD_PMSK_ADDR, pMask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}





/* Function Name:
 *      dal_rtl8373_l2_unknV6Mc_action_set
 * Description:
 *      Set ipv6 multicast pkt lookup miss action
 * Input:
 *      port        - Port ID
 *      action  - 0b00:fwd in IPV6_UNKNOW_MC_FLD_PMSK  0b01 drop  0b10 trap  0b11 to router port
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_unknV6Mc_action_set(rtk_uint32 port, rtk_uint32 action)
{
    ret_t retVal;

    if(port >= RTL8373_PORTNO)
        return RT_ERR_PORT_ID;

    if((retVal = rtl8373_setAsicRegBits(RTL8373_IPV6_PORT_MC_LM_ACT_ADDR(port), RTL8373_IPV6_PORT_MC_LM_ACT_ACT_MASK(port), action)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_l2_unknV6Mc_action_get
 * Description:
 *      Get ipv6 multicast pkt lookup miss action
 * Input:
 *      port        - Port ID
 *      pAction  - 0b00:fwd in IPV6_UNKNOW_MC_FLD_PMSK  0b01 drop  0b10 trap  0b11 to router port
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_unknV6Mc_action_get(rtk_uint32 port, rtk_uint32 *pAction)
{
    ret_t retVal;

    if(port >= RTL8373_PORTNO)
        return RT_ERR_PORT_ID;

    if((retVal = rtl8373_getAsicRegBits(RTL8373_IPV6_PORT_MC_LM_ACT_ADDR(port), RTL8373_IPV6_PORT_MC_LM_ACT_ACT_MASK(port), pAction)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_brdcast_fldMsk_set
 * Description:
 *      set brdcast pkt flood port mask
 * Input:
 *      portMask        - brdcast pkt flood port mask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_brdcast_fldMsk_set(rtk_uint32 portMask)
{
    ret_t retVal;


    if((retVal = rtl8373_setAsicReg(RTL8373_L2_BC_FLD_PMSK_ADDR, portMask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_l2_brdcast_fldMsk_get
 * Description:
 *      get brdcast pkt flood port mask
 * Input:
 *      pMask        - brdcast pkt flood port mask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
ret_t dal_rtl8373_l2_brdcast_fldMsk_get(rtk_uint32* pMask)
{
    ret_t retVal;


    if((retVal = rtl8373_getAsicReg(RTL8373_L2_BC_FLD_PMSK_ADDR, pMask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_l2_trapPort_set
 * Description:
 *      set l2 trap cpu port mask
 * Input:
 *      trapport        - 0b00 none    0b01:8051  0b10:external cpu    0b11: 8051&external
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

rtk_api_ret_t dal_rtl8373_l2_trapPort_set(rtk_uint32 trapport)
{
    ret_t retVal;

    if(trapport > TRAP_BOTH)
        return RT_ERR_INPUT;

    if((retVal = rtl8373_setAsicRegBits(RTL8373_L2_CTRL_ADDR, RTL8373_L2_CTRL_CPU_PMSK_MASK, trapport)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_trapPort_Get
 * Description:
 *      get l2 trap cpu port mask
 * Input:
 *      trapport        - 0b00 none    0b01:8051  0b10:external cpu    0b11: 8051&external
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

rtk_api_ret_t dal_rtl8373_l2_trapPort_get(rtk_uint32 * pTrapport)
{
    ret_t retVal;

    if(pTrapport == NULL)
        return RT_ERR_NULL_POINTER;

    if((retVal = rtl8373_getAsicRegBits(RTL8373_L2_CTRL_ADDR, RTL8373_L2_CTRL_CPU_PMSK_MASK, pTrapport)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_trapPri_set
 * Description:
 *      set l2 trap cpu port mask
 * Input:
 *      type           - 0: normal pkt   1: multicast
 *      trappri        - trap priority
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

rtk_api_ret_t dal_rtl8373_l2_trapPri_set(rtk_uint32 type, rtk_uint32 trappri)
{

    if (type == 0)
        return rtl8373_setAsicRegBits(RTL8373_L2_CTRL_ADDR, RTL8373_L2_CTRL_TRAP_PRI_MASK, trappri);

    else if (type == 1)
        return rtl8373_setAsicRegBits(RTL8373_L2_CTRL_ADDR, RTL8373_L2_CTRL_MUL_TRAP_PRI_MASK, trappri);

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_l2_trapPri_get
 * Description:
 *      set l2 trap cpu port mask
 * Input:
 *      type           - 0: normal pkt   1: multicast
 *      pTrappri        - trap priority
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

rtk_api_ret_t dal_rtl8373_l2_trapPri_get(rtk_uint32 type, rtk_uint32* pTrappri)
{

    if (type == 0)
        return rtl8373_getAsicRegBits(RTL8373_L2_CTRL_ADDR, RTL8373_L2_CTRL_TRAP_PRI_MASK, pTrappri);

    else if (type == 1)
        return rtl8373_getAsicRegBits(RTL8373_L2_CTRL_ADDR, RTL8373_L2_CTRL_MUL_TRAP_PRI_MASK, pTrappri);

    return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8373_l2_hashFull_set
 * Description:
 *      set l2 hsah full acti
 * Input:
 *      act        - 0b00 fwd    0b01:drop  0b10:trap  
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

rtk_api_ret_t dal_rtl8373_l2_hashFull_set(rtk_uint32 act)
{
    ret_t retVal;

    if((retVal = rtl8373_setAsicRegBits(RTL8373_L2_CTRL_ADDR, RTL8373_L2_CTRL_HASH_FULL_ACT_MASK, act)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_hashFull_get
 * Description:
 *      get l2 hsah full acti
 * Input:
 *      pAct        - 0b00 fwd    0b01:drop  0b10:trap  
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

rtk_api_ret_t dal_rtl8373_l2_hashFull_get(rtk_uint32* pAct)
{
    ret_t retVal;

    if((retVal = rtl8373_getAsicRegBits(RTL8373_L2_CTRL_ADDR, RTL8373_L2_CTRL_HASH_FULL_ACT_MASK, pAct)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_l2_ipMul_noVlanEgr_set
 * Description:
 *      set ipmulticast bypass vlanegress filter
 * Input:
 *      port        - port number
 *      enable    - 0 disable vlan egress filter     1 enable vlan egress filter
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

rtk_api_ret_t dal_rtl8373_l2_ipMul_noVlanEgr_set(rtk_uint32 port, rtk_uint32 enable)
{
    ret_t retVal;

    if((retVal = rtl8373_setAsicRegBit(RTL8373_IPMUL_NO_VLAN_EGRESS_ADDR(port), RTL8373_IPMUL_NO_VLAN_EGRESS_IPMUL_VLAN_LEAKY_OFFSET(port), enable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_ipMul_noVlanEgr_get
 * Description:
 *      get ipmulticast bypass vlanegress filter
 * Input:
 *      port        - port number
 *      pEnable    - 0 disable vlan egress filter     1 enable vlan egress filter
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

rtk_api_ret_t dal_rtl8373_l2_ipMul_noVlanEgr_get(rtk_uint32 port, rtk_uint32* pEnable)
{
    ret_t retVal;

    if((retVal = rtl8373_getAsicRegBit(RTL8373_IPMUL_NO_VLAN_EGRESS_ADDR(port), RTL8373_IPMUL_NO_VLAN_EGRESS_IPMUL_VLAN_LEAKY_OFFSET(port), pEnable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_ipMul_noPortIso_set
 * Description:
 *      set ipmulticast bypass port isolation filter
 * Input:
 *      port        - port number
 *      enable    - 0 disable port isolation filter     1 enable port isolation filter
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

rtk_api_ret_t dal_rtl8373_l2_ipMul_noPortIso_set(rtk_uint32 port, rtk_uint32 enable)
{
    ret_t retVal;

    if((retVal = rtl8373_setAsicRegBit(RTL8373_IPMUL_NO_PORTISO_ADDR(port), RTL8373_IPMUL_NO_PORTISO_IPMUL_PORTISO_LEAKY_OFFSET(port), enable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_ipMul_noPortIso_get
 * Description:
 *      get ipmulticast bypass port isolation filter
 * Input:
 *      port        - port number
 *      pEnable    - 0 disable port isolation filter     1 enable port isolation filter
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

rtk_api_ret_t dal_rtl8373_l2_ipMul_noPortIso_get(rtk_uint32 port, rtk_uint32* pEnable)
{
    ret_t retVal;

    if((retVal = rtl8373_getAsicRegBit(RTL8373_IPMUL_NO_PORTISO_ADDR(port), RTL8373_IPMUL_NO_PORTISO_IPMUL_PORTISO_LEAKY_OFFSET(port), pEnable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_forceMode_set
 * Description:
 *      set l2 force mode
 * Input:
 *      port        - port number
 *      enable    - 0 disable port force mode    1 enable port force mode
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

rtk_api_ret_t dal_rtl8373_l2_forceMode_set(rtk_uint32 port, rtk_uint32 enable)
{
    ret_t retVal;

    if((retVal = rtl8373_setAsicRegBit(RTL8373_L2_FORCE_MODE_ADDR(port), RTL8373_L2_FORCE_MODE_FORCE_CTRL_OFFSET(port), enable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_forceMode_get
 * Description:
 *      get l2 force mode
 * Input:
 *      port        - port number
 *      enable    - 0 disable port force mode    1 enable port force mode
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

rtk_api_ret_t dal_rtl8373_l2_forceMode_get(rtk_uint32 port, rtk_uint32* pEnable)
{
    ret_t retVal;

    if((retVal = rtl8373_getAsicRegBit(RTL8373_L2_FORCE_MODE_ADDR(port), RTL8373_L2_FORCE_MODE_FORCE_CTRL_OFFSET(port), pEnable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_l2_forceMode_portMsk_set
 * Description:
 *      set l2 force mode
 * Input:
 *      port        - port number
 *      portmask    - port mask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

rtk_api_ret_t dal_rtl8373_l2_forceMode_portMsk_set(rtk_uint32 port, rtk_uint32 portmask)
{
    ret_t retVal;

    if((retVal = rtl8373_setAsicRegBits(RTL8373_L2_FORCE_DPM_PORT_ADDR(port), RTL8373_L2_FORCE_DPM_PORT_FORCE_PORT_MASK_MASK, portmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_l2_forceMode_portMsk_get
 * Description:
 *      get l2 force mode
 * Input:
 *      port        - port number
 *      portmask    - port mask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

rtk_api_ret_t dal_rtl8373_l2_forceMode_portMsk_get(rtk_uint32 port, rtk_uint32* pMask)
{
    ret_t retVal;

    if((retVal = rtl8373_getAsicRegBits(RTL8373_L2_FORCE_DPM_PORT_ADDR(port), RTL8373_L2_FORCE_DPM_PORT_FORCE_PORT_MASK_MASK, pMask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_l2_floodPortMsk_set
 * Description:
 *      set flood portmask
 * Input:
 *      flood_type       - unknown unicast, unknown l2 multicast, unknown IPV4 multicast, unknown IPV6 multicast, broadcast
 *      pFlood_portmask    - port mask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

rtk_api_ret_t dal_rtl8373_l2_floodPortMsk_set(rtk_l2_flood_type_t flood_type, rtk_portmask_t *pFlood_portmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pFlood_portmask)
        return RT_ERR_NULL_POINTER;

    /* Check port mask */
    RTK_CHK_PORTMASK_VALID(pFlood_portmask);

    if ((retVal = rtk_switch_portmask_L2P_get(pFlood_portmask, &pmask)) != RT_ERR_OK)
        return retVal;

    if(flood_type == FLOOD_UNKNOWNDA)
        return dal_rtl8373_l2_unknUc_fldMsk_set(pmask);
    else if(flood_type ==  FLOOD_UNKNOWNL2MC)
        return dal_rtl8373_l2_unknMc_fldMsk_set(pmask);
    else if(flood_type ==  FLOOD_UNKNOWNV4MC)
        return dal_rtl8373_l2_unknV4Mc_fldMsk_set(pmask);
    else if(flood_type ==  FLOOD_UNKNOWNV6MC)
        return dal_rtl8373_l2_unknV6Mc_fldMsk_set(pmask);
    else if(flood_type ==  FLOOD_BC)
        return dal_rtl8373_l2_brdcast_fldMsk_set(pmask);
    else
        return RT_ERR_INPUT;

    return RT_ERR_OK;
}





/* Function Name:
 *      dal_rtl8373_l2_floodPortMsk_get
 * Description:
 *      Get flood portmask
 * Input:
 *      flood_type       - unknown unicast, unknown l2 multicast, unknown IPV4 multicast, unknown IPV6 multicast, broadcast
 *      pFlood_portmask    - port mask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

rtk_api_ret_t dal_rtl8373_l2_floodPortMsk_get(rtk_l2_flood_type_t flood_type, rtk_portmask_t *pFlood_portmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pFlood_portmask)
        return RT_ERR_NULL_POINTER;

    
    if(flood_type == FLOOD_UNKNOWNDA)
    {
        retVal = dal_rtl8373_l2_unknUc_fldMsk_get(&pmask);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else if(flood_type ==  FLOOD_UNKNOWNL2MC)
    {
        retVal = dal_rtl8373_l2_unknMc_fldMsk_get(&pmask);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else if(flood_type ==  FLOOD_UNKNOWNV4MC)
    {
        retVal = dal_rtl8373_l2_unknV4Mc_fldMsk_get(&pmask);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else if(flood_type ==  FLOOD_UNKNOWNV6MC)
    {
        retVal = dal_rtl8373_l2_unknV6Mc_fldMsk_get(&pmask);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else if(flood_type ==  FLOOD_BC)
    {
        retVal = dal_rtl8373_l2_brdcast_fldMsk_get(&pmask);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
        return RT_ERR_INPUT;

    if ((retVal = rtk_switch_portmask_P2L_get(pmask, pFlood_portmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}














 

