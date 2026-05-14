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
 * Feature : Here is a list of all functions and variables in Mirror module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal/rtl8373/dal_rtl8373_mirror.h>
#include <dal/rtl8373/rtl8373_asicdrv.h>

/* Function Name:
 *      dal_rtl8373_mirror_set_en
 * Description:
 *      enable/disable port mirror function.
 * Input:
 *      enable          -
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 * Note:
 *      The API is to enable/disable mirror function
 */
rtk_api_ret_t dal_rtl8373_mirror_set_en(rtk_enable_t enable)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();
    
    if(enable > RTK_ENABLE_END)
        return RT_ERR_INPUT;

    retVal = rtl8373_setAsicRegBit(RTL8373_MIR_SET_CTRL_ADDR, RTL8373_MIR_SET_CTRL_MIR_EN_OFFSET, (rtk_uint32)enable);
    if( retVal !=  RT_ERR_OK )
        return retVal;

    return RT_ERR_OK ;
}

/* Function Name:
 *      dal_rtl8373_mirror_setStatus_get
 * Description:
 *     get port mirror function state.
 * Input:
 *      enable          -
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_NULL_POINTER -
 * Note:
 *      The API is to get mirror function state
 */
rtk_api_ret_t dal_rtl8373_mirror_setStatus_get(rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 regData = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pEnable)
           return RT_ERR_NULL_POINTER;

    retVal = rtl8373_getAsicRegBit(RTL8373_MIR_SET_CTRL_ADDR, RTL8373_MIR_SET_CTRL_MIR_EN_OFFSET, &regData);
    if( retVal !=  RT_ERR_OK )
        return retVal;
    *pEnable = (rtk_enable_t)regData;

    return RT_ERR_OK ;
}

/* Function Name:
 *      dal_rtl8373_mirror_portBased_set
 * Description:
 *      Set port mirror function.
 * Input:
 *      mirroring_port          - Monitor port.
 *      pMirrored_rx_portmask   - Rx mirror port mask.
 *      pMirrored_tx_portmask   - Tx mirror port mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_PORT_MASK    - Invalid portmask.
 * Note:
 *      The API is to set mirror function of source port and mirror port.
 *      The mirror port can only be set to one port and the TX and RX mirror ports
 *      should be identical.
 */
rtk_api_ret_t dal_rtl8373_mirror_entry_set(rtk_port_mir_set_t *pMirSet)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint16 tmp = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pMirSet)
        return RT_ERR_NULL_POINTER;

    if( (pMirSet->rx_tx_sel) >=  MIRROR_DIR_END )
        return RT_ERR_INPUT;
    
    /* Check port valid */
    RTK_CHK_PORTMASK_VALID(&pMirSet->tx_pmsk);
    RTK_CHK_PORTMASK_VALID(&pMirSet->rx_pmsk);
    RTK_CHK_PORT_VALID(pMirSet->mtp_port); 
    
    /*Check mirrored port not include mtp port*/
    tmp = 1<<(pMirSet->mtp_port);
    
    if( (tmp & pMirSet->rx_pmsk.bits[0]) || (tmp & pMirSet->tx_pmsk.bits[0]) )
        return RT_ERR_INPUT;

    retVal = rtl8373_setAsicRegBits(RTL8373_MIR_SET_CTRL_ADDR, RTL8373_MIR_SET_CTRL_MTP_PORT_MASK, pMirSet->mtp_port);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBits(RTL8373_MIR_SET_PMSK_ADDR, RTL8373_MIR_SET_PMSK_RX_PMSK_MASK, pMirSet->rx_pmsk.bits[0]);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBits(RTL8373_MIR_SET_PMSK_ADDR, RTL8373_MIR_SET_PMSK_TX_PMSK_MASK, pMirSet->tx_pmsk.bits[0]);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_MIR_SET_CTRL_ADDR, RTL8373_MIR_SET_CTRL_MIR_RX_TX_SEL_OFFSET, (rtk_uint32)pMirSet->rx_tx_sel);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_mirror_portBased_get
 * Description:
 *      Get port mirror function.
 * Input:
 *      None
 * Output:
 *      pMirroring_port         - Monitor port.
 *      pMirrored_rx_portmask   - Rx mirror port mask.
 *      pMirrored_tx_portmask   - Tx mirror port mask.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API is to get mirror function of source port and mirror port.
 */
rtk_api_ret_t dal_rtl8373_mirror_entry_get(rtk_port_mir_set_t *pMirSet)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 txRxSel = 0;
    rtk_uint32 regData = 0;
    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pMirSet)
        return RT_ERR_NULL_POINTER;
    /* Get source portmask */
    if((retVal = rtl8373_getAsicRegBits(RTL8373_MIR_SET_PMSK_ADDR, RTL8373_MIR_SET_PMSK_TX_PMSK_MASK , &regData)) != RT_ERR_OK)
        return retVal;
    pMirSet->tx_pmsk.bits[0] = regData;

    if((retVal = rtl8373_getAsicRegBits(RTL8373_MIR_SET_PMSK_ADDR, RTL8373_MIR_SET_PMSK_RX_PMSK_MASK, &regData)) != RT_ERR_OK)
        return retVal;
    pMirSet->rx_pmsk.bits[0] = regData;

    if((retVal = rtl8373_getAsicRegBit(RTL8373_MIR_SET_CTRL_ADDR, RTL8373_MIR_SET_CTRL_MIR_RX_TX_SEL_OFFSET,  &txRxSel)) != RT_ERR_OK)
        return retVal;

    /* Get monitor(destination) port */
    if((retVal = rtl8373_getAsicRegBits(RTL8373_MIR_SET_CTRL_ADDR, RTL8373_MIR_SET_CTRL_MTP_PORT_MASK,  &regData)) != RT_ERR_OK)
        return retVal;

    pMirSet->mtp_port = regData;

    pMirSet->rx_tx_sel = (rtk_mirror_direction_t)(txRxSel) ;
    return RT_ERR_OK;

}

/* Function Name:
 *      dal_rtl8373_mirror_portIso_set
 * Description:
 *      Set mirror port isolation.
 * Input:
 *      enable |Mirror isolation status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid enable input
 * Note:
 *      The API is to set mirror isolation function that prevent normal forwarding packets to miror port.
 */
rtk_api_ret_t dal_rtl8373_mirror_portIso_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 isoEn = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    isoEn = (enable == ENABLED) ? 1 : 0;
    if ((retVal = rtl8373_setAsicRegBit(RTL8373_MIR_SET_CTRL_ADDR, RTL8373_MIR_SET_CTRL_MIR_ISO_OFFSET, isoEn)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_mirror_portIso_get
 * Description:
 *      Get mirror port isolation.
 * Input:
 *      None
 * Output:
 *      pEnable |Mirror isolation status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API is to get mirror isolation status.
 */
rtk_api_ret_t dal_rtl8373_mirror_portIso_get(rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 isoEn = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_MIR_SET_CTRL_ADDR, RTL8373_MIR_SET_CTRL_MIR_ISO_OFFSET, &isoEn)) != RT_ERR_OK)
        return retVal;

    *pEnable = (isoEn == 1) ? ENABLED : DISABLED;
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_mirror_vlanLeaky_set
 * Description:
 *      Set mirror VLAN leaky.
 * Input:
 *      txenable -TX leaky enable.
 *      rxenable - RX leaky enable.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid enable input
 * Note:
 *      The API is to set mirror VLAN leaky function forwarding packets to miror port.
 */
rtk_api_ret_t dal_rtl8373_mirror_vlanLeaky_set(rtk_enable_t txenable, rtk_enable_t rxenable)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 txEn = 0, rxEn = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ((txenable >= RTK_ENABLE_END) ||(rxenable >= RTK_ENABLE_END))
        return RT_ERR_ENABLE;

    txEn = (txenable == ENABLED) ? 1 : 0;
    rxEn = (rxenable == ENABLED) ? 1 : 0;
    if ((retVal = rtl8373_setAsicRegBit(RTL8373_MIR_CTRL_ADDR, RTL8373_MIR_CTRL_MIR_TX_VLAN_LKY_OFFSET, txEn)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_MIR_CTRL_ADDR, RTL8373_MIR_CTRL_MIR_RX_VLAN_LKY_OFFSET, rxEn)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_mirror_vlanLeaky_get
 * Description:
 *      Get mirror VLAN leaky.
 * Input:
 *      None
 * Output:
 *      pTxenable - TX leaky enable.
 *      pRxenable - RX leaky enable.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API is to get mirror VLAN leaky status.
 */
rtk_api_ret_t dal_rtl8373_mirror_vlanLeaky_get(rtk_enable_t *pTxenable, rtk_enable_t *pRxenable)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 txEn = 0, rxEn = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if( (NULL == pTxenable) || (NULL == pRxenable) )
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_MIR_CTRL_ADDR, RTL8373_MIR_CTRL_MIR_TX_VLAN_LKY_OFFSET, &txEn)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_MIR_CTRL_ADDR, RTL8373_MIR_CTRL_MIR_RX_VLAN_LKY_OFFSET, &rxEn)) != RT_ERR_OK)
        return retVal;

    *pTxenable = (txEn == 1) ? ENABLED : DISABLED;
    *pRxenable = (rxEn == 1) ? ENABLED : DISABLED;
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_mirror_isolationLeaky_set
 * Description:
 *      Set mirror Isolation leaky.
 * Input:
 *      txenable -TX leaky enable.
 *      rxenable - RX leaky enable.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid enable input
 * Note:
 *      The API is to set mirror PORT ISOLATION leaky function forwarding packets to miror port.
 */
rtk_api_ret_t dal_rtl8373_mirror_isolationLeaky_set(rtk_enable_t txenable, rtk_enable_t rxenable)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 txEn = 0, rxEn = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ((txenable >= RTK_ENABLE_END) ||(rxenable >= RTK_ENABLE_END))
        return RT_ERR_ENABLE;

    txEn = (txenable == ENABLED) ? 1 : 0;
    rxEn = (rxenable == ENABLED) ? 1 : 0;
    if ((retVal = rtl8373_setAsicRegBit(RTL8373_MIR_CTRL_ADDR, RTL8373_MIR_CTRL_MIR_TX_ISOLATE_LKY_OFFSET, txEn)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_MIR_CTRL_ADDR, RTL8373_MIR_CTRL_MIR_RX_ISOLATE_LKY_OFFSET, rxEn)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_mirror_isolationLeaky_get
 * Description:
 *      Get mirror isolation leaky.
 * Input:
 *      None
 * Output:
 *      pTxenable - TX leaky enable.
 *      pRxenable - RX leaky enable.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API is to get mirror isolation leaky status.
 */
rtk_api_ret_t dal_rtl8373_mirror_isolationLeaky_get(rtk_enable_t *pTxenable, rtk_enable_t *pRxenable)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 txEn = 0, rxEn = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if( (NULL == pTxenable) || (NULL == pRxenable) )
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_MIR_CTRL_ADDR, RTL8373_MIR_CTRL_MIR_TX_ISOLATE_LKY_OFFSET, &txEn)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_MIR_CTRL_ADDR, RTL8373_MIR_CTRL_MIR_RX_ISOLATE_LKY_OFFSET, &rxEn)) != RT_ERR_OK)
        return retVal;

    *pTxenable = (txEn == 1) ? ENABLED : DISABLED;
    *pRxenable = (rxEn == 1) ? ENABLED : DISABLED;
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_mirror_keep_set
 * Description:
 *      Set mirror packet format keep.
 * Input:
 *      mode - -mirror keep mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid enable input
 * Note:
 *      The API is to set  -mirror keep mode.
 *      The mirror keep mode is as following:
 *      - MIRROR_FOLLOW_VLAN
 *      - MIRROR_KEEP_ORIGINAL
 *      - MIRROR_KEEP_END
 */
rtk_api_ret_t dal_rtl8373_mirror_keep_set(rtk_mirror_keep_t mode)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 keepMode = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (mode >= MIRROR_KEEP_END)
        return RT_ERR_ENABLE;

    keepMode = (mode == MIRROR_FOLLOW_VLAN) ? 0 : 1;
    if ((retVal = rtl8373_setAsicRegBit(RTL8373_MIR_CTRL_ADDR, RTL8373_MIR_CTRL_MIR_REALKEEP_EN_OFFSET, keepMode)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_mirror_keep_get
 * Description:
 *      Get mirror packet format keep.
 * Input:
 *      None
 * Output:
 *      pMode -mirror keep mode.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API is to get mirror keep mode.
 *      The mirror keep mode is as following:
 *      - MIRROR_FOLLOW_VLAN
 *      - MIRROR_KEEP_ORIGINAL
 *      - MIRROR_KEEP_END
 */
rtk_api_ret_t dal_rtl8373_mirror_keep_get(rtk_mirror_keep_t *pMode)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 keepMode = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pMode)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_MIR_CTRL_ADDR, RTL8373_MIR_CTRL_MIR_REALKEEP_EN_OFFSET, &keepMode)) != RT_ERR_OK)
        return retVal;

    *pMode = (keepMode == 0) ? MIRROR_FOLLOW_VLAN : MIRROR_KEEP_ORIGINAL;
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_mirror_override_set
 * Description:
 *      Set port mirror override function.
 * Input:
 *      rxMirror        - 1: output mirrored packet, 0: output normal forward packet
 *      txMirror        - 1: output mirrored packet, 0: output normal forward packet
 *      aclMirror       - 1: output mirrored packet, 0: output normal forward packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      The API is to set mirror override function.
 *      This function control the output format when a port output
 *      normal forward & mirrored packet at the same time.
 */
rtk_api_ret_t dal_rtl8373_mirror_override_set(rtk_enable_t rxMirror, rtk_enable_t txMirror, rtk_enable_t aclMirror)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if( (rxMirror >= RTK_ENABLE_END) || (txMirror >= RTK_ENABLE_END) || (aclMirror >= RTK_ENABLE_END))
        return RT_ERR_NULL_POINTER;

    if((retVal = rtl8373_setAsicRegBit(RTL8373_MIR_CTRL_ADDR, RTL8373_MIR_CTRL_MIRROR_RX_OVERRIDE_EN_OFFSET, (rxMirror == ENABLED) ? 1 : 0)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8373_setAsicRegBit(RTL8373_MIR_CTRL_ADDR, RTL8373_MIR_CTRL_MIRROR_TX_OVERRIDE_EN_OFFSET, (txMirror == ENABLED) ? 1 : 0)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8373_setAsicRegBit(RTL8373_MIR_CTRL_ADDR, RTL8373_MIR_CTRL_MIRROR_ACL_OVERRIDE_EN_OFFSET, (aclMirror == ENABLED) ? 1 : 0)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_mirror_override_get
 * Description:
 *      Get port mirror override function.
 * Input:
 *      None
 * Output:
 *      pRxMirror       - 1: output mirrored packet, 0: output normal forward packet
 *      pTxMirror       - 1: output mirrored packet, 0: output normal forward packet
 *      pAclMirror      - 1: output mirrored packet, 0: output normal forward packet
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NULL_POINTER - Null Pointer
 * Note:
 *      The API is to Get mirror override function.
 *      This function control the output format when a port output
 *      normal forward & mirrored packet at the same time.
 */
rtk_api_ret_t dal_rtl8373_mirror_override_get(rtk_enable_t *pRxMirror, rtk_enable_t *pTxMirror, rtk_enable_t *pAclMirror)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 txEn = 0, rxEn = 0, aclEn = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if( (pRxMirror == NULL) || (pTxMirror == NULL) || (pAclMirror == NULL))
        return RT_ERR_NULL_POINTER;

    if((retVal = rtl8373_getAsicRegBit(RTL8373_MIR_CTRL_ADDR, RTL8373_MIR_CTRL_MIRROR_RX_OVERRIDE_EN_OFFSET, &rxEn)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8373_getAsicRegBit(RTL8373_MIR_CTRL_ADDR, RTL8373_MIR_CTRL_MIRROR_TX_OVERRIDE_EN_OFFSET, &txEn)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8373_getAsicRegBit(RTL8373_MIR_CTRL_ADDR, RTL8373_MIR_CTRL_MIRROR_ACL_OVERRIDE_EN_OFFSET, &aclEn)) != RT_ERR_OK)
        return retVal;

    *pRxMirror = (rxEn == 1) ? ENABLED : DISABLED;
    *pTxMirror = (txEn == 1) ? ENABLED : DISABLED;
    *pAclMirror = (aclEn == 1) ? ENABLED : DISABLED;
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_mirror_sampeRate_set
 * Description:
 *      set port mirror sample rate.
 * Input:
 *      rateVal -
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NULL_POINTER - Null Pointer
 * Note:
 *      The API is to Set mirror sample rate.
 */
rtk_api_ret_t dal_rtl8373_mirror_sampeRate_set(rtk_uint32 rateVal)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(rateVal > MIR_SAMPLE_RATE_MAX)
        return RT_ERR_INPUT;
    
    if((retVal = rtl8373_setAsicRegBits(RTL8373_MIR_SAMPLE_CRTL_ADDR, RTL8373_MIR_SAMPLE_CRTL_RATE_MASK, rateVal) != RT_ERR_OK))
           return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_mirror_sampeRate_get
 * Description:
 *      get port mirror sample rate.
 * Input:
 *      None -
 * Output:
 *      pRateVal   - sample rate value
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NULL_POINTER - Null Pointer
 * Note:
 *      The API is to Set mirror sample rate.
 */
rtk_api_ret_t dal_rtl8373_mirror_sampeRate_get(rtk_uint32 *pRateVal)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();
        
    if( pRateVal == NULL)
        return RT_ERR_NULL_POINTER;
    
    if((retVal = rtl8373_getAsicRegBits(RTL8373_MIR_SAMPLE_CRTL_ADDR, RTL8373_MIR_SAMPLE_CRTL_RATE_MASK, pRateVal) != RT_ERR_OK))
           return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_mirror_samplePktCnt_get
 * Description:
 *      get Total counter for mirror condition satisfied packets
 * Input:
 *      None -
 * Output:
 *      pCntr   -Total counter for mirror condition satisfied packets
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NULL_POINTER - Null Pointer
 * Note:
 *      The API is to get Total counter for mirror condition satisfied packets
 */
rtk_api_ret_t dal_rtl8373_mirror_pktCnt_get(rtk_uint32 *pCntr)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();
        
    if( pCntr == NULL)
        return RT_ERR_NULL_POINTER;
    
    if((retVal = rtl8373_getAsicRegBits(RTL8373_MIR_MATCHED_ADDR, RTL8373_MIR_MATCHED_PKT_CNT_MASK, pCntr) != RT_ERR_OK))
           return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_mirror_samplePktCnt_get
 * Description:
 *      get Total sample counter for traffic mirror
 * Input:
 *      None -
 * Output:
 *      pCntr   - Total sample counter for traffic mirror
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NULL_POINTER - Null Pointer
 * Note:
 *      The API is to get Total sample counter for traffic mirror
 */
rtk_api_ret_t dal_rtl8373_mirror_samplePktCnt_get(rtk_uint32 *pCntr)
{
    rtk_api_ret_t retVal = 0;
    
    /* Check initialization state */
    RTK_CHK_INIT_STATE();
        
    if( pCntr == NULL)
        return RT_ERR_NULL_POINTER;
    
    if((retVal = rtl8373_getAsicRegBits(RTL8373_MIR_MATCHED_ADDR, RTL8373_MIR_MATCHED_SAMPLE_PKT_CNT_MASK, pCntr) != RT_ERR_OK))
           return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rspan_rxTag_en
 * Description:
 *      set rspan rx tag parser function enable/disable.
 * Input:
 *      enable
 * Output:
 *      None   - .
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 */
rtk_api_ret_t dal_rtl8373_rspan_rxTag_en(rtk_enable_t enable)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(enable > RTK_ENABLE_END)
        return RT_ERR_INPUT;
    
    retVal = rtl8373_setAsicRegBit(RTL8373_MIR_RSPAN_CTRL_ADDR, RTL8373_MIR_RSPAN_CTRL_RX_TAG_EN_OFFSET, (rtk_uint32)enable);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rspan_rxTagEnStatus_get
 * Description:
 *      get rspan rx tag enable/disable status.
 * Input:
 *      None
 * Output:
 *      pEnable   -
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 */
rtk_api_ret_t dal_rtl8373_rspan_rxTagEnSts_get(rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 regData = 0;
    
    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;
    
    retVal = rtl8373_getAsicRegBit(RTL8373_MIR_RSPAN_CTRL_ADDR, RTL8373_MIR_RSPAN_CTRL_RX_TAG_EN_OFFSET, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pEnable = (rtk_enable_t )regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rspan_tagContext_set
 * Description:
 *      set rspan tag context:TPID PRI CFI  VID .
 * Input:
 *      pRspanTag  - rspan tag context:TPID PRI CFI  VID .
 * Output:
 *      None  
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 */
rtk_api_ret_t dal_rtl8373_rspan_tagContext_set(rtk_rspan_tag_t *pRspanTag)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 regData = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pRspanTag)
        return RT_ERR_NULL_POINTER;

    if (pRspanTag->tpid > RTK_MAX_NUM_OF_PROTO_TYPE)
        return RT_ERR_INPUT;

    if (pRspanTag->vid > RTL8373_VIDMAX)
        return RT_ERR_VLAN_VID;

    if (pRspanTag->pri > RTL8373_PRIMAX)
        return RT_ERR_VLAN_PRIORITY;

    if (pRspanTag->cfi > RTK_ENABLE_END)
        return RT_ERR_INPUT;

    regData = ((pRspanTag->tpid  & 0xFFFF)<< RTL8373_MIR_RSPAN_TAG_CTRL_TPID_OFFSET) | \
                    ((pRspanTag->pri & 0x7) << RTL8373_MIR_RSPAN_TAG_CTRL_PRI_OFFSET) | \
                    ((pRspanTag->cfi & 0x1) << RTL8373_MIR_RSPAN_TAG_CTRL_CFI_OFFSET) | (pRspanTag->vid & 0xFFF);                    

    retVal = rtl8373_setAsicReg(RTL8373_MIR_RSPAN_TAG_CTRL_ADDR, regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
 }

/* Function Name:
 *      dal_rtl8373_rspan_tagContext_get
 * Description:
 *      get rspan tag context:TPID PRI CFI  VID .
 * Input:
 *      None
 * Output:
 *      pRspanTag  - rspan tag context:TPID PRI CFI  VID .
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 */
rtk_api_ret_t dal_rtl8373_rspan_tagContext_get(rtk_rspan_tag_t *pRspanTag)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 regData = 0;

    retVal = rtl8373_getAsicReg(RTL8373_MIR_RSPAN_TAG_CTRL_ADDR, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    pRspanTag->tpid = (regData >> RTL8373_MIR_RSPAN_TAG_CTRL_TPID_OFFSET) & RTK_MAX_NUM_OF_PROTO_TYPE ;
    pRspanTag->pri = (regData >> RTL8373_MIR_RSPAN_TAG_CTRL_PRI_OFFSET) & RTL8373_PRIMAX ;
    pRspanTag->cfi = (regData >> RTL8373_MIR_RSPAN_TAG_CTRL_CFI_OFFSET) & 0x1 ;
    pRspanTag->vid = (regData >> RTL8373_MIR_RSPAN_TAG_CTRL_VID_OFFSET) & RTL8373_VIDMAX ;

    return RT_ERR_OK;
}

/*
* Function Name:
*       dal_rtl8373_rspan_tagAdd_set
* Description:
*       enable port add rspan tag  function
*Input:
*       pmsk: port mask that tx pkt want add rspan tag
*Output:
*       None
*Return:
*       RT_ERR_OK           - OK
*       RT_ERR_FAILED       - Failed
*Note:  None
*/
rtk_api_ret_t dal_rtl8373_rspan_tagAdd_set(rtk_portmask_t pmsk)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();
    RTK_CHK_PORTMASK_VALID(&pmsk);

    retVal = rtl8373_setAsicRegBits(RTL8373_MIR_RSPAN_TX_PORT_CTRL_ADDR, RTL8373_MIR_RSPAN_TX_PORT_CTRL_TAG_ADD_MASK, pmsk.bits[0]);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/*
* Function Name:
*    dal_rtl8373_rspan_tagAdd_get
* Description:
*    enable port add rspan tag  function
*Input:
*       None
*Output:
*       pPmskSts: per port add rspan tag status
*Return:
*       RT_ERR_OK           - OK
*       RT_ERR_FAILED       - Failed
*Note:  None
*/
rtk_api_ret_t dal_rtl8373_rspan_tagAdd_get(rtk_portmask_t *pPmskSts)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 regData = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();
    
    if(NULL == pPmskSts)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8373_getAsicRegBits(RTL8373_MIR_RSPAN_TX_PORT_CTRL_ADDR, RTL8373_MIR_RSPAN_TX_PORT_CTRL_TAG_ADD_MASK, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    pPmskSts->bits[0] = regData;

    return RT_ERR_OK;
}

/*
* Function Name:
*   dal_rtl8373_rspan_tagRemove_set
* Description:
*   set  mtp port remove rspan tag function
*Input:
*       enable: mtp port remove rspan tag function enable/disable
*Output:
*       None
*Return:
*       RT_ERR_OK           - OK
*       RT_ERR_FAILED       - Failed
*Note:  None
*/
rtk_api_ret_t dal_rtl8373_rspan_tagRemove_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(enable > RTK_ENABLE_END)
        return RT_ERR_INPUT;
    
    retVal = rtl8373_setAsicRegBit(RTL8373_MIR_RSPAN_RX_ACT_ADDR, RTL8373_MIR_RSPAN_RX_ACT_TAG_RM_OFFSET, enable);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/*
* Function Name:
*   dal_rtl8373_rspan_tagRemove_get
* Description:
*   get  mtp port remove rspan tag status
*Input:
*       None
*Output:
*       pEnable - 
*Return:
*       RT_ERR_OK           - OK
*       RT_ERR_FAILED       - Failed
*Note:  None
*/
rtk_api_ret_t dal_rtl8373_rspan_tagRemove_get(rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 regData = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;
    
    retVal = rtl8373_getAsicRegBit(RTL8373_MIR_RSPAN_RX_ACT_ADDR, RTL8373_MIR_RSPAN_RX_ACT_TAG_RM_OFFSET, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pEnable = (rtk_enable_t )regData;
    
    return RT_ERR_OK;
}

