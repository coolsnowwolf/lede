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
 * Feature : Here is a list of all functions and variables in Flow Control module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal_rtl8373_fc.h>
#include <rtl8373_asicdrv.h>
#include <string.h>


/* Function Name:
 *      dal_rtl8373_asicFCPubPage_set
 * Description:
 *      Set public page number
 * Input:
 *      number     - public page number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCPubPage_set(rtk_uint32 number)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBits(RTL8373_FC_GLB_DROP_THR_ADDR, RTL8373_FC_GLB_DROP_THR_PUB_PAGE_MASK, number);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_asicFCPubPage_get
 * Description:
 *      Get public page number
 * Input:
 *      number     - public page number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCPubPage_get(rtk_uint32* pNumber)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBits(RTL8373_FC_GLB_DROP_THR_ADDR, RTL8373_FC_GLB_DROP_THR_PUB_PAGE_MASK, pNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_asicFCDropAll_set
 * Description:
 *      Set drop all  page number
 * Input:
 *      number     - drop all page number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCDropAll_set(rtk_uint32 number)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBits(RTL8373_FC_GLB_DROP_THR_ADDR, RTL8373_FC_GLB_DROP_THR_DROP_ALL_MASK, number);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_asicFCDropAll_get
 * Description:
 *      get drop all  page number
 * Input:
 *      number     - drop all page number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCDropAll_get(rtk_uint32* pNumber)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBits(RTL8373_FC_GLB_DROP_THR_ADDR, RTL8373_FC_GLB_DROP_THR_DROP_ALL_MASK, pNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8373_asicFCOnHiThr_set
 * Description:
 *      Set flow control on high threshold  page number
 * Input:
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCOnHiThr_set(rtk_uint32 onNumber, rtk_uint32 offNumber)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBits(RTL8373_FC_GLB_HI_THR_ADDR, RTL8373_FC_GLB_HI_THR_ON_MASK, onNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBits(RTL8373_FC_GLB_HI_THR_ADDR, RTL8373_FC_GLB_HI_THR_OFF_MASK, offNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_asicFCOnHiThr_get
 * Description:
 *      Get flow control on high threshold  page number
 * Input:
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCOnHiThr_get(rtk_uint32* onNumber, rtk_uint32* offNumber)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBits(RTL8373_FC_GLB_HI_THR_ADDR, RTL8373_FC_GLB_HI_THR_ON_MASK, onNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBits(RTL8373_FC_GLB_HI_THR_ADDR, RTL8373_FC_GLB_HI_THR_OFF_MASK, offNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_asicFCOnLoThr_set
 * Description:
 *      Set flow control on low threshold  page number
 * Input:
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCOnLoThr_set(rtk_uint32 onNumber, rtk_uint32 offNumber)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBits(RTL8373_FC_GLB_LO_THR_ADDR, RTL8373_FC_GLB_LO_THR_ON_MASK, onNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBits(RTL8373_FC_GLB_LO_THR_ADDR, RTL8373_FC_GLB_LO_THR_OFF_MASK, offNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_asicFCOnLoThr_get
 * Description:
 *      Get flow control on low threshold  page number
 * Input:
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCOnLoThr_get(rtk_uint32* onNumber, rtk_uint32* offNumber)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBits(RTL8373_FC_GLB_LO_THR_ADDR, RTL8373_FC_GLB_LO_THR_ON_MASK, onNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBits(RTL8373_FC_GLB_LO_THR_ADDR, RTL8373_FC_GLB_LO_THR_OFF_MASK, offNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_asicFCOffHiThr_set
 * Description:
 *      Set flow control off high threshold  page number
 * Input:
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCOffHiThr_set(rtk_uint32 onNumber, rtk_uint32 offNumber)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBits(RTL8373_FC_GLB_FCOFF_HI_THR_ADDR, RTL8373_FC_GLB_FCOFF_HI_THR_ON_MASK, onNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBits(RTL8373_FC_GLB_FCOFF_HI_THR_ADDR, RTL8373_FC_GLB_FCOFF_HI_THR_OFF_MASK, offNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_asicFCOffHiThr_get
 * Description:
 *      Get flow control off high threshold  page number
 * Input:
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCOffHiThr_get(rtk_uint32* onNumber, rtk_uint32* offNumber)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBits(RTL8373_FC_GLB_FCOFF_HI_THR_ADDR, RTL8373_FC_GLB_FCOFF_HI_THR_ON_MASK, onNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBits(RTL8373_FC_GLB_FCOFF_HI_THR_ADDR, RTL8373_FC_GLB_FCOFF_HI_THR_OFF_MASK, offNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_asicFCOffLoThr_set
 * Description:
 *      Set flow control off low threshold  page number
 * Input:
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCOffLoThr_set(rtk_uint32 onNumber, rtk_uint32 offNumber)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBits(RTL8373_FC_GLB_FCOFF_LO_THR_ADDR, RTL8373_FC_GLB_FCOFF_LO_THR_ON_MASK, onNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBits(RTL8373_FC_GLB_FCOFF_LO_THR_ADDR, RTL8373_FC_GLB_FCOFF_LO_THR_OFF_MASK, offNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_asicFCOffLoThr_get
 * Description:
 *      Get flow control off low threshold  page number
 * Input:
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCOffLoThr_get(rtk_uint32* onNumber, rtk_uint32* offNumber)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBits(RTL8373_FC_GLB_FCOFF_LO_THR_ADDR, RTL8373_FC_GLB_FCOFF_LO_THR_ON_MASK, onNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBits(RTL8373_FC_GLB_FCOFF_LO_THR_ADDR, RTL8373_FC_GLB_FCOFF_LO_THR_OFF_MASK, offNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8373_asicFCOnPortHiThr_set
 * Description:
 *      Set flow control on high threshold  page number
 * Input:
 *      index           - 0 ~ 3
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCOnPortHiThr_set(rtk_uint32 index, rtk_uint32 onNumber, rtk_uint32 offNumber)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBits(RTL8373_FC_PORT_HI_THR_ADDR(index), RTL8373_FC_PORT_HI_THR_ON_MASK, onNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBits(RTL8373_FC_PORT_HI_THR_ADDR(index), RTL8373_FC_PORT_HI_THR_OFF_MASK, offNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_asicFCOnPortHiThr_get
 * Description:
 *      Get flow control on high threshold  page number
 * Input:
 *      index           - 0 ~ 3
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCOnPortHiThr_get(rtk_uint32 index, rtk_uint32* onNumber, rtk_uint32* offNumber)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBits(RTL8373_FC_PORT_HI_THR_ADDR(index), RTL8373_FC_PORT_HI_THR_ON_MASK, onNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBits(RTL8373_FC_PORT_HI_THR_ADDR(index), RTL8373_FC_PORT_HI_THR_OFF_MASK, offNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_asicFCOnPortLoThr_set
 * Description:
 *      Set flow control on low threshold  page number
 * Input:
 *      index           - 0 ~ 3
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCOnPortLoThr_set(rtk_uint32 index, rtk_uint32 onNumber, rtk_uint32 offNumber)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBits(RTL8373_FC_PORT_LO_THR_ADDR(index), RTL8373_FC_PORT_LO_THR_ON_MASK, onNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBits(RTL8373_FC_PORT_LO_THR_ADDR(index), RTL8373_FC_PORT_LO_THR_OFF_MASK, offNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_asicFCOnPortLoThr_get
 * Description:
 *      Get flow control on low threshold  page number
 * Input:
 *      index           - 0 ~ 3
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCOnPortLoThr_get(rtk_uint32 index, rtk_uint32* onNumber, rtk_uint32* offNumber)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBits(RTL8373_FC_PORT_LO_THR_ADDR(index), RTL8373_FC_PORT_LO_THR_ON_MASK, onNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBits(RTL8373_FC_PORT_LO_THR_ADDR(index), RTL8373_FC_PORT_LO_THR_OFF_MASK, offNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_asicFCOffPortHiThr_set
 * Description:
 *      Set flow control off high threshold  page number
 * Input:
 *      index           - 0 ~ 3
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCOffPortHiThr_set(rtk_uint32 index, rtk_uint32 onNumber, rtk_uint32 offNumber)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBits(RTL8373_FC_PORT_FCOFF_HI_THR_ADDR(index), RTL8373_FC_PORT_FCOFF_HI_THR_ON_MASK, onNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBits(RTL8373_FC_PORT_FCOFF_HI_THR_ADDR(index), RTL8373_FC_PORT_FCOFF_HI_THR_OFF_MASK, offNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_asicFCOffPortHiThr_get
 * Description:
 *      Get flow control off high threshold  page number
 * Input:
 *      index           - 0 ~ 3
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCOffPortHiThr_get(rtk_uint32 index, rtk_uint32* onNumber, rtk_uint32* offNumber)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBits(RTL8373_FC_PORT_FCOFF_HI_THR_ADDR(index), RTL8373_FC_PORT_FCOFF_HI_THR_ON_MASK, onNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBits(RTL8373_FC_PORT_FCOFF_HI_THR_ADDR(index), RTL8373_FC_PORT_FCOFF_HI_THR_OFF_MASK, offNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_asicFCOffPortLoThr_set
 * Description:
 *      Set flow control off low threshold  page number
 * Input:
 *      index           - 0 ~ 3
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCOffPortLoThr_set(rtk_uint32 index, rtk_uint32 onNumber, rtk_uint32 offNumber)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBits(RTL8373_FC_PORT_FCOFF_LO_THR_ADDR(index), RTL8373_FC_PORT_FCOFF_LO_THR_ON_MASK, onNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBits(RTL8373_FC_PORT_FCOFF_LO_THR_ADDR(index), RTL8373_FC_PORT_FCOFF_LO_THR_OFF_MASK, offNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_asicFCOffPortLoThr_get
 * Description:
 *      Get flow control off low threshold  page number
 * Input:
 *      index           - 0 ~ 3
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCOffPortLoThr_get(rtk_uint32 index, rtk_uint32* onNumber, rtk_uint32* offNumber)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBits(RTL8373_FC_PORT_FCOFF_LO_THR_ADDR(index), RTL8373_FC_PORT_FCOFF_LO_THR_ON_MASK, onNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBits(RTL8373_FC_PORT_FCOFF_LO_THR_ADDR(index), RTL8373_FC_PORT_FCOFF_LO_THR_OFF_MASK, offNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_asicFCPortGua_set
 * Description:
 *      Set port guarantee page
 * Input:
 *      index           - 0 ~ 3
 *      number        - guarantee
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCPortGua_set(rtk_uint32 index, rtk_uint32 number)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBits(RTL8373_FC_PORT_GUAR_THR_ADDR(index), RTL8373_FC_PORT_GUAR_THR_THR_MASK, number);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBits(RTL8373_FC_PORT_GUAR_THR_ADDR(index), RTL8373_FC_PORT_GUAR_THR_THR_MASK, number);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8373_asicFCPortGua_get
 * Description:
 *      Set port guarantee page
 * Input:
 *      index           - 0 ~ 3
 *      number        - guarantee
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCPortGua_get(rtk_uint32 index, rtk_uint32* pNumber)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBits(RTL8373_FC_PORT_GUAR_THR_ADDR(index), RTL8373_FC_PORT_GUAR_THR_THR_MASK, pNumber);
    if(retVal != RT_ERR_OK)
        return retVal;


    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_asicFCPortThrSel_set
 * Description:
 *      Set port threshold select
 * Input:
 *      port             - 0 ~ 9
 *      index           - 0 ~ 3
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCPortThrSel_set(rtk_uint32 port, rtk_uint32 index)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBits(RTL8373_FC_PORT_THR_SET_SEL_ADDR(port), RTL8373_FC_PORT_THR_SET_SEL_IDX_MASK(port) , index);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_asicFCPortThrSel_get
 * Description:
 *      Set port threshold select
 * Input:
 *      port             - 0 ~ 9
 *      index           - 0 ~ 3
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCPortThrSel_get(rtk_uint32 port, rtk_uint32* pIndex)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBits(RTL8373_FC_PORT_THR_SET_SEL_ADDR(port), RTL8373_FC_PORT_THR_SET_SEL_IDX_MASK(port) , pIndex);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_asicFCPortThrSel_set
 * Description:
 *      Set port threshold select
 * Input:
 *      type             - 0:unknown unicast     1: l2 multicast      2: broadcast
 *      enable           - 0:disable hol prevention    1: enable hol prevention
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCHOLPrvnt_set(rtk_uint32 type, rtk_uint32 enable)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBit(RTL8373_FC_HOL_PRVNT_CTRL_ADDR, 1 << type , enable);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_asicFCPortThrSel_get
 * Description:
 *      Set port threshold select
 * Input:
 *      type             - 0:unknown unicast     1: l2 multicast      2: broadcast
 *      enable           - 0:disable hol prevention    1: enable hol prevention
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCHOLPrvnt_get(rtk_uint32 type, rtk_uint32* pEnable)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBit(RTL8373_FC_HOL_PRVNT_CTRL_ADDR, 1 << type , pEnable);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_asicFCPortHOLPrvntEn_set
 * Description:
 *      Set port threshold select
 * Input:
 *      port             - 0 ~ 9
 *      enable           - 0:disable hol prevention    1: enable hol prevention
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCPortHOLPrvntEn_set(rtk_uint32 port, rtk_uint32 enable)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBit(RTL8373_FC_PORT_EGR_DROP_CTRL_ADDR(port), RTL8373_FC_PORT_EGR_DROP_CTRL_HOL_PRVNT_EN_OFFSET , enable);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_asicFCPortHOLPrvntEn_get
 * Description:
 *      Get port threshold select
 * Input:
 *      port             - 0 ~ 9
 *      enable           - 0:disable hol prevention    1: enable hol prevention
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCPortHOLPrvntEn_get(rtk_uint32 port, rtk_uint32* enable)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBit(RTL8373_FC_PORT_EGR_DROP_CTRL_ADDR(port), RTL8373_FC_PORT_EGR_DROP_CTRL_HOL_PRVNT_EN_OFFSET , enable);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_asicFCPortAct_set
 * Description:
 *      Set port flow control action
 * Input:
 *      portnum             - 0 ~ 9
 *      act                    - 0:receive    1: drop
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCPortAct_set(rtk_uint32 portnum, rtk_uint32 act)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBits(RTL8373_FC_PORT_ACT_CTRL_ADDR(portnum), RTL8373_FC_PORT_ACT_CTRL_ACT_MASK, act);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_asicFCPortAct_get
 * Description:
 *      Get port flow control action
 * Input:
 *      portnum             - 0 ~ 9
 *      act                    - 0:receive    1: drop
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCPortAct_get(rtk_uint32 portnum, rtk_uint32* pAct)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBits(RTL8373_FC_PORT_ACT_CTRL_ADDR(portnum), RTL8373_FC_PORT_ACT_CTRL_ACT_MASK, pAct);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_asicFCPortAllowPage_set
 * Description:
 *      Set port allow page number
 * Input:
 *      portnum             - 0 ~ 9
 *      pagenum           - allow page number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCPortAllowPage_set(rtk_uint32 portnum, rtk_uint32 pagenum)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBits(RTL8373_FC_PORT_ACT_CTRL_ADDR(portnum), RTL8373_FC_PORT_ACT_CTRL_ALLOW_PAGE_CNT_MASK, pagenum);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_asicFCPortAllowPage_get
 * Description:
 *      Get port allow page number
 * Input:
 *      portnum             - 0 ~ 9
 *      pagenum           - allow page number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCPortAllowPage_get(rtk_uint32 portnum, rtk_uint32* pPagenum)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBits(RTL8373_FC_PORT_ACT_CTRL_ADDR(portnum), RTL8373_FC_PORT_ACT_CTRL_ALLOW_PAGE_CNT_MASK, pPagenum);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_asicFCQEgrDropThr_set
 * Description:
 *      Set flow control on high threshold  page number
 * Input:
 *      index           - 0 ~ 3
 *      queueid       - 0 ~ 7
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCQEgrDropThr_set(rtk_uint32 index, rtk_uint32 queueid, rtk_uint32 onNumber, rtk_uint32 offNumber)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBits(RTL8373_FC_Q_EGR_DROP_THR_ADDR(queueid, index), RTL8373_FC_Q_EGR_DROP_THR_ON_MASK, onNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBits(RTL8373_FC_Q_EGR_DROP_THR_ADDR(queueid, index), RTL8373_FC_Q_EGR_DROP_THR_OFF_MASK, offNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_asicFCQEgrDropThr_get
 * Description:
 *      Get flow control on high threshold  page number
 * Input:
 *      index           - 0 ~ 3
 *      queueid       - 0 ~ 7
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCQEgrDropThr_get(rtk_uint32 index, rtk_uint32 queueid, rtk_uint32* onNumber, rtk_uint32* offNumber)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBits(RTL8373_FC_Q_EGR_DROP_THR_ADDR(queueid, index), RTL8373_FC_Q_EGR_DROP_THR_ON_MASK, onNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBits(RTL8373_FC_Q_EGR_DROP_THR_ADDR(queueid, index), RTL8373_FC_Q_EGR_DROP_THR_OFF_MASK, offNumber);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_asicFCPortQEgrDropThrSel_set
 * Description:
 *      Select port queue egress drop threshold
 * Input:
 *      port             - 0 ~ 9
 *      index           - 0 ~ 3
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCPortQEgrDropThrSel_set(rtk_uint32 port, rtk_uint32 index)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBits(RTL8373_FC_PORT_EGR_DROP_THR_SET_SEL_ADDR(port), RTL8373_FC_PORT_EGR_DROP_THR_SET_SEL_IDX_MASK(port) , index);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_asicFCPortQEgrDropThrSel_get
 * Description:
 *      Gelect port queue egress drop threshold
 * Input:
 *      port             - 0 ~ 9
 *      index           - 0 ~ 3
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCPortQEgrDropThrSel_get(rtk_uint32 port, rtk_uint32* pIndex)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBits(RTL8373_FC_PORT_EGR_DROP_THR_SET_SEL_ADDR(port), RTL8373_FC_PORT_EGR_DROP_THR_SET_SEL_IDX_MASK(port) , pIndex);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_asicFCPortQEgrDropEn_set
 * Description:
 *      Set port queue egress drop enable
 * Input:
 *      port             - 0 ~ 9
 *      qid              - 0 ~ 7
 *      enable         - 1: enable    0: disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCPortQEgrDropEn_set(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 enable)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBit(RTL8373_FC_PORT_Q_EGR_DROP_CTRL_SET_ADDR(port, qid) , RTL8373_FC_PORT_Q_EGR_DROP_CTRL_SET_EN_OFFSET(qid) , enable);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_asicFCPortQEgrDropEn_get
 * Description:
 *      Get port queue egress drop enable
 * Input:
 *      port             - 0 ~ 9
 *      qid              - 0 ~ 7
 *      enable         - 1: enable    0: disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCPortQEgrDropEn_get(rtk_uint32 port, rtk_uint32 qid, rtk_uint32* enable)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBit(RTL8373_FC_PORT_Q_EGR_DROP_CTRL_SET_ADDR(port, qid) , RTL8373_FC_PORT_Q_EGR_DROP_CTRL_SET_EN_OFFSET(qid) , enable);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_asicFCPortQForceEgrDrop_set
 * Description:
 *      Set port queue force egress drop ability
 * Input:
 *      port             - 0 ~ 9
 *      qid              - 0 ~ 7
 *      enable         - 1: enable    0: disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCPortQForceEgrDrop_set(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 enable)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBit(RTL8373_FC_PORT_Q_EGR_FORCE_DROP_CTRL_SET_ADDR(port, qid) , RTL8373_FC_PORT_Q_EGR_FORCE_DROP_CTRL_SET_EN_OFFSET(qid) , enable);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_asicFCPortQForceEgrDrop_get
 * Description:
 *      Get port queue force egress drop ability
 * Input:
 *      port             - 0 ~ 9
 *      qid              - 0 ~ 7
 *      enable         - 1: enable    0: disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicFCPortQForceEgrDrop_get(rtk_uint32 port, rtk_uint32 qid, rtk_uint32* enable)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBit(RTL8373_FC_PORT_Q_EGR_FORCE_DROP_CTRL_SET_ADDR(port, qid) , RTL8373_FC_PORT_Q_EGR_FORCE_DROP_CTRL_SET_EN_OFFSET(qid) , enable);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}




