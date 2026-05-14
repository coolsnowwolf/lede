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
#include <mirror.h>
#include <string.h>
#include <dal/dal_mgmt.h>


/* Function Name:
 *      rtk_mirror_set_en
 * Description:
 *      enable/disable port mirror set function.
 * Input:
 *      enable          -
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 * Note:
 *      The API is to enable/disable mirror function
 */
rtk_api_ret_t rtk_mirror_set_en(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->mirror_set_en)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->mirror_set_en(enable);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_mirror_setStatus_get
 * Description:
 *      get port mirror function state: enable/disable.
 * Input:
 *      None          -
 * Output:
 *      pEnable      - mirror set enable or not
 * Return:
 *      RT_ERR_OK           - OK
 * Note:
 *      The API is to enable/disable mirror function
 */
rtk_api_ret_t rtk_mirror_setStatus_get(rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->mirror_setStatus_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->mirror_setStatus_get(pEnable);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_mirror_portBased_set
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
rtk_api_ret_t rtk_mirror_portBased_set(rtk_port_mir_set_t *pMirSet)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->mirror_entry_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->mirror_entry_set(pMirSet);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_mirror_portBased_get
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
rtk_api_ret_t rtk_mirror_portBased_get(rtk_port_mir_set_t *pMirSet)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->mirror_entry_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->mirror_entry_get(pMirSet);
    RTK_API_UNLOCK();
    return retVal;
}

/* Function Name:
 *      rtk_mirror_portIso_set
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
rtk_api_ret_t rtk_mirror_portIso_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->mirror_portIso_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->mirror_portIso_set(enable);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_mirror_portIso_get
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
rtk_api_ret_t rtk_mirror_portIso_get(rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->mirror_portIso_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->mirror_portIso_get(pEnable);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_mirror_vlanLeaky_set
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
rtk_api_ret_t rtk_mirror_vlanLeaky_set(rtk_enable_t txenable, rtk_enable_t rxenable)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->mirror_vlanLeaky_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->mirror_vlanLeaky_set(txenable, rxenable);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_mirror_vlanLeaky_get
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
rtk_api_ret_t rtk_mirror_vlanLeaky_get(rtk_enable_t *pTxenable, rtk_enable_t *pRxenable)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->mirror_vlanLeaky_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->mirror_vlanLeaky_get(pTxenable, pRxenable);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_mirror_isolationLeaky_set
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
 *      The API is to set mirror VLAN leaky function forwarding packets to miror port.
 */
rtk_api_ret_t rtk_mirror_isolationLeaky_set(rtk_enable_t txenable, rtk_enable_t rxenable)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->mirror_isolationLeaky_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->mirror_isolationLeaky_set(txenable, rxenable);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_mirror_isolationLeaky_get
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
rtk_api_ret_t rtk_mirror_isolationLeaky_get(rtk_enable_t *pTxenable, rtk_enable_t *pRxenable)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->mirror_isolationLeaky_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->mirror_isolationLeaky_get(pTxenable, pRxenable);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_mirror_keep_set
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
rtk_api_ret_t rtk_mirror_keep_set(rtk_mirror_keep_t mode)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->mirror_keep_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->mirror_keep_set(mode);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_mirror_keep_get
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
rtk_api_ret_t rtk_mirror_keep_get(rtk_mirror_keep_t *pMode)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->mirror_keep_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->mirror_keep_get(pMode);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_mirror_override_set
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
rtk_api_ret_t rtk_mirror_override_set(rtk_enable_t rxMirror, rtk_enable_t txMirror, rtk_enable_t aclMirror)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->mirror_override_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->mirror_override_set(rxMirror, txMirror, aclMirror);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_mirror_override_get
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
rtk_api_ret_t rtk_mirror_override_get(rtk_enable_t *pRxMirror, rtk_enable_t *pTxMirror, rtk_enable_t *pAclMirror)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->mirror_override_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->mirror_override_get(pRxMirror, pTxMirror, pAclMirror);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_mirror_sampleRate_set
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
rtk_api_ret_t rtk_mirror_sampleRate_set(rtk_uint32 rateVal)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->mirror_sampleRate_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->mirror_sampleRate_set(rateVal);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_mirror_sampleRate_get
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
rtk_api_ret_t rtk_mirror_sampleRate_get(rtk_uint32 *pRateVal)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->mirror_sampleRate_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->mirror_sampleRate_get(pRateVal);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_mirror_pktCnt_get
 * Description:
 *      get Total counter for mirror condition satisfied packets
 * Input:
 *      None -
 * Output:
 *      pTotalPktCntr   -Total counter for mirror condition satisfied packets
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NULL_POINTER - Null Pointer
 * Note:
 *      The API is to get Total counter for mirror condition satisfied packets
 */
rtk_api_ret_t rtk_mirror_pktCnt_get(rtk_uint32 *pTotalPktCntr)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->mirror_pktCnt_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->mirror_pktCnt_get(pTotalPktCntr);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_mirror_samplePktCnt_get
 * Description:
 *      get Total sample counter for traffic mirror
 * Input:
 *      None -
 * Output:
 *      pSamplePktCntr   - Total sample counter for traffic mirror
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NULL_POINTER - Null Pointer
 * Note:
 *      The API is to get Total sample counter for traffic mirror
 */
rtk_api_ret_t rtk_mirror_samplePktCnt_get(rtk_uint32 *pSamplePktCntr)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->mirror_samplePktCnt_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->mirror_samplePktCnt_get(pSamplePktCntr);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_rspan_rxTag_en
 * Description:
 *      set rspan rx tag parser function enable/disable.
 * Input:
 *      enable
 * Output:
 *      None   - 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 */
rtk_api_ret_t rtk_rspan_rxTag_en(rtk_enable_t enable )
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->rspan_rxTag_en)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rspan_rxTag_en(enable);
    RTK_API_UNLOCK();

    return retVal;
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
rtk_api_ret_t rtk_rspan_rxTagEnStatus_get(rtk_enable_t *pEnable )
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->rspan_rxTagEnSts_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rspan_rxTagEnSts_get(pEnable);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_rspan_tagCtxt_set
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
rtk_api_ret_t rtk_rspan_tagCtxt_set(rtk_rspan_tag_t *pRspanTag)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->rspan_tagCtxt_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rspan_tagCtxt_set(pRspanTag);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_rspan_tagCtxt_get
 * Description:
 *      set rspan tag context:TPID PRI CFI  VID .
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

rtk_api_ret_t rtk_rspan_tagCtxt_get(rtk_rspan_tag_t *pRspanTag)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->rspan_tagCtxt_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rspan_tagCtxt_get(pRspanTag);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_rspan_tagAdd_set
 * Description:
 *      per tx port set rspan tag  added  function.
 * Input:
 *      egrPmsk   - a portmask that want add rspantag while tx pkt
 * Output:
 *      None   
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 */
rtk_api_ret_t rtk_rspan_tagAdd_set(rtk_portmask_t egrPmsk)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->rspan_tagAdd_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rspan_tagAdd_set(egrPmsk);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_rspan_tagAdd_get
 * Description:
 *      get a portmask which ports has enable add rspan tag function.
 * Input:
 *      None
 * Output:
 *      pPmsk   -
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 */
rtk_api_ret_t rtk_rspan_tagAdd_get(rtk_portmask_t *pPmskStatus )
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->rspan_tagAdd_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rspan_tagAdd_get(pPmskStatus);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_rspan_tagRemove_set
 * Description:
 *      set rspan  rx tag remove enable/disable function.
 * Input:
 *      enable  -
 * Output:
 *      None   
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 */
rtk_api_ret_t rtk_rspan_tagRemove_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->rspan_tagRemove_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rspan_tagRemove_set(enable);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_rspan_tagRemove_get
 * Description:
 *      get rspan rx tag remove state.
 * Input:
 *      None
 * Output:
 *      pEnable   
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 */
rtk_api_ret_t rtk_rspan_tagRemove_get(rtk_enable_t *pEnable )
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == RT_MAPPER->rspan_tagRemove_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rspan_tagRemove_get(pEnable);
    RTK_API_UNLOCK();

    return retVal;
}

