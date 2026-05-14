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
 * Purpose : Declaration of MIB
 *
 * Feature : The file have include the following module and sub-modules
 *           1) MIB reset , get
 *
 */


/*
 * Include Files
 */
#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal_rtl8373_mib.h>
#include <rtl8373_asicdrv.h>


/* Function Name:
 *      dal_rtl8373_globalMib_rst
 * Description:
 *      global mib reset
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_globalMib_rst(void)
{
    ret_t retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_STAT_RST_ADDR, RTL8373_STAT_RST_RST_GLB_MIB_OFFSET,1);

    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_portMib_rst
 * Description:
 *      port mib reset
 * Input:
 *      port: the port to reset mib
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_portMib_rst(rtk_uint32 port)
{
    ret_t retVal;
    rtk_uint32 tmp;

    tmp = (1 << 4)| port;

    retVal = rtl8373_setAsicReg(RTL8373_STAT_PORT_RST_ADDR, tmp);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_dbgMib_rst
 * Description:
 *      debug mib reset
 * Input:
 *      port: the port to reset mib
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_dbgMib_rst(rtk_uint32 port)
{
    ret_t retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_DEBUG_MIB_RST_ADDR(port), RTL8373_DEBUG_MIB_RST_WRAP_MIB_RST_OFFSET(port), 1);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8373_dbgMib_rst
 * Description:
 *      debug mib reset
 * Input:
 *      port: the port to reset mib
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_portMib_read(rtk_uint32 portid, rtk_stat_port_type_t mibid, rtk_uint64 * pMibCounter)
{
    ret_t retVal;
    rtk_uint32 tmp, wait, mibReqIdx = 0;
    rtk_uint32 dataH, dataL;

    mibReqIdx = mibid/2;
    tmp = 1 | ((portid & 0xf) << 1) | ((mibReqIdx & 0x3f) << 5);
    //rtlglue_printf("regvalue 0x%x    ", tmp);
    retVal = rtl8373_setAsicReg(RTL8373_INDIRECT_ACCESS_CTRL_ADDR, tmp);
    if(retVal != RT_ERR_OK)
        return retVal;

    wait = 0;
    retVal = rtl8373_getAsicRegBit(RTL8373_INDIRECT_ACCESS_CTRL_ADDR, RTL8373_INDIRECT_ACCESS_CTRL_ACC_CMD_OFFSET, &tmp);
    if(retVal != RT_ERR_OK)
        return retVal;

    while(tmp & 1)
    {
        wait++;
        if(wait > 100)
            return RT_ERR_FAILED;

        retVal = rtl8373_getAsicRegBit(RTL8373_INDIRECT_ACCESS_CTRL_ADDR, RTL8373_INDIRECT_ACCESS_CTRL_ACC_CMD_OFFSET, &tmp);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    retVal = rtl8373_getAsicReg(RTL8373_INDIRECT_ACCESS_CNT_L_ADDR, &dataL);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicReg(RTL8373_INDIRECT_ACCESS_CNT_H_ADDR, &dataH);
    if(retVal != RT_ERR_OK)
        return retVal;
    //rtlglue_printf("dataH 0x%x,  dataL: 0x%x  ", dataH, dataL);
    if((mibid < 16) || ((mibid >91) && (mibid < 96)) || ((mibid >97) && (mibid < 102)))
        *pMibCounter = ((rtk_uint64)dataL << 32) | dataH;
    else if((mibid < 92) || ((mibid > 95) && (mibid < 98)) || ((mibid > 101) && (mibid < 104)))
    {
        if(mibid % 2)
            *pMibCounter = dataH;
        else
            *pMibCounter = dataL;
    }

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_mibLength_set
 * Description:
 *      Set mib counter length include tag or not
 * Input:
 *      txMode: tx include tag or not; rxMode: rx include tag or not
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_mibLength_set(rtk_stat_lengthMode_t txMode, rtk_stat_lengthMode_t rxMode)
{
    ret_t retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_STAT_CTRL_ADDR, RTL8373_STAT_CTRL_TX_CNT_TAG_OFFSET,txMode);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_STAT_CTRL_ADDR, RTL8373_STAT_CTRL_RX_CNT_TAG_OFFSET,rxMode);

    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_mibLength_get
 * Description:
 *      Get mib counter length include tag or not
 * Input:
 *      txMode: tx include tag or not; rxMode: rx include tag or not
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_mibLength_get(rtk_stat_lengthMode_t* pTxMode, rtk_stat_lengthMode_t* pRxMode)
{
    ret_t retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_STAT_CTRL_ADDR, RTL8373_STAT_CTRL_TX_CNT_TAG_OFFSET,pTxMode);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_STAT_CTRL_ADDR, RTL8373_STAT_CTRL_RX_CNT_TAG_OFFSET,pRxMode);

    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}








