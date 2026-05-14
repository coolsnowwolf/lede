/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: $
 * $Date: $
 *
 * Purpose : Declaration of Parser
 *
 * Feature : The file have include the following module and sub-modules
 *           1) HSB, HSA get
 *
 */


/*
 * Include Files
 */
#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal_rtl8373_parser.h>
#include <rtl8373_asicdrv.h>

/* Function Name:
 *      dal_rtl8373_ignrOUI_set
 * Description:
 *      Get ignore OUI
 * Input:
 *      None
 * Output:
 *      pEnabled     - ignore OUI enable/disable
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
ret_t dal_rtl8373_ignrOUI_set(rtk_uint32 enabled)
{
    ret_t retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_PARSER_CTRL_ADDR, RTL8373_PARSER_CTRL_RFC1042_OUI_IGNORE_OFFSET,enabled);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_ignrOUI_get
 * Description:
 *      Get ignore OUI
 * Input:
 *      None
 * Output:
 *      pEnabled     - ignore OUI enable/disable
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
ret_t dal_rtl8373_ignrOUI_get(rtk_uint32 *pEnabled)
{
    ret_t retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_PARSER_CTRL_ADDR, RTL8373_PARSER_CTRL_RFC1042_OUI_IGNORE_OFFSET,pEnabled);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;

}



/* Function Name:
 *      dal_rtl8373_rxReason_get
 * Description:
 *      Get ignore OUI
 * Input:
 *      None
 * Output:
 *      pReason     - rx drop reason
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
ret_t dal_rtl8373_rxReason_get(rtk_uint32 port, rtk_uint32 *pReason)
{
    ret_t retVal;

    retVal = rtl8373_getAsicRegBits(RTL8373_PARSER_DROP_REASON_ADDR(port), RTL8373_PARSER_DROP_REASON_REASON_MASK,pReason);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;

}



/* Function Name:
 *      dal_rtl8373_fieldSelector_set
 * Description:
 *      Get field selector setting
 * Input:
 *      None
 * Output:
 *      index    - 0~15
 *      format   0 ~ 7
 *      offset    0 ~ 172
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
ret_t dal_rtl8373_fieldSelector_set(rtk_uint32 index, rtk_uint32 format, rtk_uint32 offset)
{
    ret_t retVal;

    retVal = rtl8373_setAsicRegBits(RTL8373_PARSER_FIELD_SELTOR_CTRL_ADDR(index), RTL8373_PARSER_FIELD_SELTOR_CTRL_FMT_MASK,format);
    if(retVal != RT_ERR_OK)
        return retVal;

    
    retVal = rtl8373_setAsicRegBits(RTL8373_PARSER_FIELD_SELTOR_CTRL_ADDR(index), RTL8373_PARSER_FIELD_SELTOR_CTRL_OFFSET_MASK,offset);
        if(retVal != RT_ERR_OK)
            return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_fieldSelector_get
 * Description:
 *      Get field selector setting
 * Input:
 *      None
 * Output:
 *      index    - 0~15
 *      pFormat   0 ~ 7
 *      pOffset    0 ~ 172
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
ret_t dal_rtl8373_fieldSelector_get(rtk_uint32 index, rtk_uint32 *pFormat, rtk_uint32 *pOffset)
{
    ret_t retVal;

    retVal = rtl8373_getAsicRegBits(RTL8373_PARSER_FIELD_SELTOR_CTRL_ADDR(index), RTL8373_PARSER_FIELD_SELTOR_CTRL_FMT_MASK,pFormat);
    if(retVal != RT_ERR_OK)
        return retVal;

    
    retVal = rtl8373_getAsicRegBits(RTL8373_PARSER_FIELD_SELTOR_CTRL_ADDR(index), RTL8373_PARSER_FIELD_SELTOR_CTRL_OFFSET_MASK,pOffset);
        if(retVal != RT_ERR_OK)
            return retVal;

    return RT_ERR_OK;
}







