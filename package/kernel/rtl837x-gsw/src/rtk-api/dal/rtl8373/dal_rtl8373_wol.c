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
 * Feature : Here is a list of all functions and variables in TRUNK module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal_rtl8373_wol.h>
#include <rtl8373_asicdrv.h>
#include <string.h>



/* Function Name:
 *      dal_rtl8373_wolState_set
 * Description:
 *      Set wol function enable
 * Input:
 *      enable : 1 enable wol;   0: disable wol
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
rtk_api_ret_t dal_rtl8373_wolState_set(rtk_uint32 enable)
{
    rtk_api_ret_t retVal;
   
    if ((retVal = rtl8373_setAsicRegBit(RTL8373_WOL_CTRL_ADDR, RTL8373_WOL_CTRL_WOL_EN_OFFSET, enable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_wolState_get
 * Description:
 *      Set wol function enable
 * Input:
 *      pEnable : 1 enable wol;   0: disable wol
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
rtk_api_ret_t dal_rtl8373_wolState_get(rtk_uint32* pEnable)
{
    rtk_api_ret_t retVal;
   
    if ((retVal = rtl8373_getAsicRegBit(RTL8373_WOL_CTRL_ADDR, RTL8373_WOL_CTRL_WOL_EN_OFFSET, pEnable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


#if 0

/* Function Name:
 *      dal_rtl8373_wolPortmsk_set
 * Description:
 *      Set wol port mask
 * Input:
 *      portmask: port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
rtk_api_ret_t dal_rtl8373_wolPortmsk_set(rtk_uint32 portmask)
{
    rtk_api_ret_t retVal;
   
    if ((retVal = rtl8373_setAsicRegBits(RTL8373_WOL_CTRL_ADDR, RTL8373_WOL_CTRL_WOL_PMSK_MASK, portmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

#endif


/* Function Name:
 *      dal_rtl8373_wolPortmsk_get
 * Description:
 *      Set wol port mask
 * Input:
 *      pMask: port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
rtk_api_ret_t dal_rtl8373_wolPortmsk_get(rtk_uint32* pMask)
{
    rtk_api_ret_t retVal;
   
    if ((retVal = rtl8373_getAsicRegBits(RTL8373_WOL_CTRL_ADDR, RTL8373_WOL_CTRL_WOL_PMSK_MASK, pMask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8373_wolMac_set
 * Description:
 *      Set wol port mask
 * Input:
 *      pMac: wol mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
rtk_api_ret_t dal_rtl8373_wolMac_set(rtk_mac_t *pMac)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    if(pMac == NULL)
        return RT_ERR_NULL_POINTER;

    regData = (pMac->octet[2] << 24) | (pMac->octet[3] << 16) | (pMac->octet[4] << 8) | pMac->octet[5];
    if ((retVal = rtl8373_setAsicReg(RTL8373_WOL_MAC0_ADDR, regData)) != RT_ERR_OK)
        return retVal;

    
    regData = (pMac->octet[0] << 8) | pMac->octet[1];
    if ((retVal = rtl8373_setAsicReg(RTL8373_WOL_MAC1_ADDR, regData)) != RT_ERR_OK)
        return retVal;
    

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_wolMac_get
 * Description:
 *      Set wol port mask
 * Input:
 *      pMac: wol mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
rtk_api_ret_t dal_rtl8373_wolMac_get(rtk_mac_t *pMac)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    if(pMac == NULL)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicReg(RTL8373_WOL_MAC0_ADDR, &regData)) != RT_ERR_OK)
        return retVal;
    pMac->octet[2] = (regData >> 24) & 0xff;
    pMac->octet[3] = (regData >> 16) & 0xff;
    pMac->octet[4] = (regData >> 8) & 0xff;
    pMac->octet[5] = regData & 0xff;

    
    if ((retVal = rtl8373_getAsicReg(RTL8373_WOL_MAC1_ADDR, &regData)) != RT_ERR_OK)
        return retVal;

    pMac->octet[0] = (regData >> 8) & 0xff;
    pMac->octet[1] = regData & 0xff;
    

    return RT_ERR_OK;
}






