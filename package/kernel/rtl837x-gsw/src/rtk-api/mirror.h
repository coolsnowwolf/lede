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
 * Purpose : RTL8373 switch high-level API
 *
 * Feature : The file includes Mirror module high-layer API defination
 *
 */

#ifndef __RTK_API_MIRROR_H__
#define __RTK_API_MIRROR_H__

typedef enum rtk_mirror_keep_e
{
    MIRROR_FOLLOW_VLAN = 0,
    MIRROR_KEEP_ORIGINAL,
    MIRROR_KEEP_END
}rtk_mirror_keep_t;

typedef enum rtk_mirror_direction_e
{
    TX_DIR = 0,
    RX_DIR,
    MIRROR_DIR_END
}rtk_mirror_direction_t;

typedef struct rtk_port_mir_set_s{
rtk_uint32 mtp_port;
rtk_portmask_t tx_pmsk;
rtk_portmask_t rx_pmsk;
rtk_mirror_direction_t rx_tx_sel;
}rtk_port_mir_set_t;


typedef struct rtk_rspan_tag_s{
rtk_uint32 tpid;
rtk_uint32 pri;
rtk_uint32 cfi;
rtk_uint32 vid;
}rtk_rspan_tag_t;

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
extern rtk_api_ret_t rtk_mirror_set_en(rtk_enable_t enable);

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
extern rtk_api_ret_t rtk_mirror_setStatus_get(rtk_enable_t *pEnable);

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
extern rtk_api_ret_t rtk_mirror_portBased_set(rtk_port_mir_set_t *pMirSet);

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
extern rtk_api_ret_t rtk_mirror_portBased_get(rtk_port_mir_set_t *pMirSet);

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
extern rtk_api_ret_t rtk_mirror_portIso_set(rtk_enable_t enable);

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
extern rtk_api_ret_t rtk_mirror_portIso_get(rtk_enable_t *pEnable);

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
extern rtk_api_ret_t rtk_mirror_vlanLeaky_set(rtk_enable_t txenable, rtk_enable_t rxenable);


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
extern rtk_api_ret_t rtk_mirror_vlanLeaky_get(rtk_enable_t *pTxenable, rtk_enable_t *pRxenable);

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
extern rtk_api_ret_t rtk_mirror_isolationLeaky_set(rtk_enable_t txenable, rtk_enable_t rxenable);

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
extern rtk_api_ret_t rtk_mirror_isolationLeaky_get(rtk_enable_t *pTxenable, rtk_enable_t *pRxenable);

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
extern rtk_api_ret_t rtk_mirror_keep_set(rtk_mirror_keep_t mode);


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
extern rtk_api_ret_t rtk_mirror_keep_get(rtk_mirror_keep_t *pMode);

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
extern rtk_api_ret_t rtk_mirror_override_set(rtk_enable_t rxMirror, rtk_enable_t txMirror, rtk_enable_t aclMirror);

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
extern rtk_api_ret_t rtk_mirror_override_get(rtk_enable_t *pRxMirror, rtk_enable_t *pTxMirror, rtk_enable_t *pAclMirror);

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
extern rtk_api_ret_t rtk_mirror_sampleRate_set(rtk_uint32 rateVal);

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
extern rtk_api_ret_t rtk_mirror_sampleRate_get(rtk_uint32 *pRateVal);

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
extern rtk_api_ret_t rtk_mirror_pktCnt_get(rtk_uint32 *pTotalPktCntr);

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
extern rtk_api_ret_t rtk_mirror_samplePktCnt_get(rtk_uint32 *pSamplePktCntr);

/* Function Name:
 *      rtk_rspan_rxTag_en
 * Description:
 *      set rspan rx tag parser function enable/disable.
 * Input:
 *      None
 * Output:
 *      enable   - .
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 */
extern rtk_api_ret_t rtk_rspan_rxTag_en(rtk_enable_t enable );

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
extern rtk_api_ret_t rtk_rspan_rxTagEnStatus_get(rtk_enable_t *pEnable );

/* Function Name:
 *      rtk_rspan_tagCtxt_set
 * Description:
 *      set rspan tag context:TPID PRI CFI  VID .
 * Input:
 *      pRspanTag  - rspan tag context:TPID PRI CFI  VID .
 * Output:
 *      None   -mirror keep mode.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 */
extern rtk_api_ret_t rtk_rspan_tagCtxt_set(rtk_rspan_tag_t *pRspanTag );

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

extern rtk_api_ret_t rtk_rspan_tagCtxt_get(rtk_rspan_tag_t *pRspanTag );

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
extern rtk_api_ret_t rtk_rspan_tagAdd_set(rtk_portmask_t egrPmsk);

/* Function Name:
 *      rtk_rspan_tagAdd_get
 * Description:
 *      get a portmask which ports has enable add rspan tag function.
 * Input:
 *      None
 * Output:
 *      pPmskStatus   -
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 */
extern rtk_api_ret_t rtk_rspan_tagAdd_get(rtk_portmask_t *pPmskStatus);

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
extern rtk_api_ret_t rtk_rspan_tagRemove_set(rtk_enable_t enable);

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
extern rtk_api_ret_t rtk_rspan_tagRemove_get(rtk_enable_t *pEnable );

#endif /* __RTK_API_MIRROR_H__ */

