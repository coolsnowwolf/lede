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
 * Feature : The file includes ingress bandwidth control module high-layer API defination
 *
 */

#ifndef __RTK_API_RATE_H__
#define __RTK_API_RATE_H__

#define INBW_CTRL_RATE_MAX     (0xFFFFF * 16)
#define INBW_CTRL_MAX_PORT_ID 	(8)

#define EBW_CTRL_RATE_MAX      (0xFFFFF * 16)
#define EBW_CTRL_MAX_PORT_ID 	(8)


typedef enum rtk_rate_igrBwCongestSts_e
{
    INBW_RATE_NOT_EXCEED = 0,
    INBW_RATE_EXCEED,
    INBW_RATE_STATE_END
} rtk_rate_igrBwCongestSts_t;


/* Function Name:
 *      rtk_rate_igrBwCtrlPortEn_set
 * Description:
 *      Enable or disable port ingress bandwidth control 
 * Input:
 *      port        - Port id
 *      bwEn        - enable ingress bandwidth control or not
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid IFG parameter.
 *      RT_ERR_INBW_RATE    - Invalid ingress rate parameter.
 * Note:
 *     
 */
extern rtk_api_ret_t rtk_rate_igrBwCtrlPortEn_set(rtk_port_t port, rtk_enable_t bwEn);

/* Function Name:
 *      rtk_rate_igrBwCtrlPortEn_get
 * Description:
 *      Enable or disable port ingress bandwidth control 
 * Input:
 *      port        - Port id
 *
 * Output:
 *      pBwEn        - Port ingress bandwidth control state
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid IFG parameter.
 *      RT_ERR_INBW_RATE    - Invalid ingress rate parameter.
 * Note:
 *     
 */
extern rtk_api_ret_t rtk_rate_igrBwCtrlPortEn_get(rtk_port_t port, rtk_enable_t *pBwEn);

/* Function Name:
 *      rtk_rate_igrBwCtrlRate_set
 * Description:
 *      Set port ingress bandwidth control rate and FC config
 * Input:
 *      port        - Port id
 *      rate        - Rate of share meter(uint: kpbs)
 *      fcEn	    - enable flow control or not, ENABLE:use flow control DISABLE:drop
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid IFG parameter.
 *      RT_ERR_INBW_RATE    - Invalid ingress rate parameter.
 * Note:
 *      The rate unit is 16 kbps and the range is from 16k to 10G. The granularity of rate is 16 kbps.
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble.
 */
extern rtk_api_ret_t rtk_rate_igrBwCtrlRate_set(rtk_port_t port, rtk_rate_t rate, rtk_enable_t fcEn);

/* Function Name:
 *      rtk_rate_igrBwCtrlRate_get
 * Description:
 *      Get port ingress bandwidth control rate and FC config
 * Input:
 *      port        - Port id
 *
 * Output:
 *      pRate        - Rate of share meter(uint: kpbs)
 *      pFcEn        - enable flow control or not, ENABLE:use flow control DISABLE:drop
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid IFG parameter.
 *      RT_ERR_INBW_RATE    - Invalid ingress rate parameter.
 * Note:
 *      The rate unit is 16 kbps and the range is from 16k to 10G. The granularity of rate is 16 kbps.
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble.
 */
extern rtk_api_ret_t rtk_rate_igrBwCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pFcEn);

/* Function Name:
 *      rtk_rate_igrBwCtrlIfg_set
 * Description:
 *      Set ingress bandwidth control include Preamble and IFG or not
 * Input:
 *      ifgInclude - include or not, ENABLE:include DISABLE:exclude
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid IFG parameter.
 *      RT_ERR_INBW_RATE    - Invalid ingress rate parameter.
 * Note:
 *
 */
extern rtk_api_ret_t rtk_rate_igrBwCtrlIfg_set(rtk_enable_t ifgInclude);

/* Function Name:
 *      rtk_rate_igrBwCtrlIfg_get
 * Description:
 *      Get ingress bandwidth control include 8B Preamble and 12B IFG or not
 * Input:
 *      None
 * Output:
 *      ifgInclude - include or not, ENABLE:include DISABLE:exclude
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid IFG parameter.
 *      RT_ERR_INBW_RATE    - Invalid ingress rate parameter.
 * Note:
 *
 */
extern rtk_api_ret_t rtk_rate_igrBwCtrlIfg_get(rtk_enable_t *pIfgInclude);

/* Function Name:
 *      rtk_rate_igrBwCtrlCongestSts_get
 * Description:
 *      Get port_n ingress bandwidth exceed leaky bucket high-on or not
 * Input:
 *      port        - port Idx
 * Output:
 *      pCongestSts - Indicate ingress bandwidth exceed Pn_IGR_LB_ HIGH _ON for port n.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid IFG parameter.
 *      RT_ERR_INBW_RATE    - Invalid ingress rate parameter.
 * Note:
 *
 */
extern rtk_api_ret_t rtk_rate_igrBwCtrlCongestSts_get(rtk_port_t port, rtk_rate_igrBwCongestSts_t *pCongestSts);

/* Function Name:
 *      rtk_rate_egrBandwidthCtrlRate_set
 * Description:
 *      Set port egress bandwidth control
 * Input:
 *      port        - Port id
 *      rate        - Rate of egress bandwidth
 *      ifg_include - include IFG or not, ENABLE:include DISABLE:exclude
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_QOS_EBW_RATE - Invalid egress bandwidth/rate
 * Note:
 *     The rate unit is 1 kbps and the range is from 8k to 1048568k. The granularity of rate is 8 kbps.
 *     The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble.
 */
extern rtk_api_ret_t rtk_rate_egrBandwidthCtrlRate_set( rtk_port_t port, rtk_rate_t rate,  rtk_enable_t ifg_include);

/* Function Name:
 *      rtk_rate_egrBandwidthCtrlRate_get
 * Description:
 *      Get port egress bandwidth control
 * Input:
 *      port - Port id
 * Output:
 *      pRate           - Rate of egress bandwidth
 *      pIfg_include    - Rate's calculation including IFG, ENABLE:include DISABLE:exclude
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *     The rate unit is 1 kbps and the range is from 8k to 1048568k. The granularity of rate is 8 kbps.
 *     The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble.
 */
extern rtk_api_ret_t rtk_rate_egrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIfg_include);

/* Function Name:
 *      rtk_rate_egrQueueBwCtrlEnable_get
 * Description:
 *      Get enable status of egress bandwidth control on specified queue.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      queue   - queue id
 * Output:
 *      pEnable - Pointer to enable status of egress queue bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
extern rtk_api_ret_t rtk_rate_egrQueueBwCtrlEnable_get(rtk_port_t port, rtk_qid_t queue, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_rate_egrQueueBwCtrlEnable_set
 * Description:
 *      Set enable status of egress bandwidth control on specified queue.
 * Input:
 *      port   - port id
 *      queue  - queue id
 *      enable - enable status of egress queue bandwidth control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
extern rtk_api_ret_t rtk_rate_egrQueueBwCtrlEnable_set(rtk_port_t port, rtk_qid_t queue, rtk_enable_t enable);

/* Function Name:
 *      rtk_rate_egrQueueBwCtrlRate_get
 * Description:
 *      Get rate of egress bandwidth control on specified queue.
 * Input:
 *      port  - port id
 *      queue - queue id
 *      pIndex - shared meter index
 * Output:
 *      pRate - pointer to rate of egress queue bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_FILTER_METER_ID  - Invalid meter id
 * Note:
 *    The actual rate control is set in shared meters.
 *    The unit of granularity is 8Kbps.
 */
extern rtk_api_ret_t rtk_rate_egrQueueBwCtrlRate_get(rtk_port_t port, rtk_qid_t queue, rtk_meter_id_t *pIndex);

/* Function Name:
 *      rtk_rate_egrQueueBwCtrlRate_set
 * Description:
 *      Set rate of egress bandwidth control on specified queue.
 * Input:
 *      port  - port id
 *      queue - queue id
 *      index - shared meter index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_FILTER_METER_ID  - Invalid meter id
 * Note:
 *    The actual rate control is set in shared meters.
 *    The unit of granularity is 8Kbps.
 */
extern rtk_api_ret_t rtk_rate_egrQueueBwCtrlRate_set(rtk_port_t port, rtk_qid_t queue, rtk_meter_id_t index);



#endif /* __RTK_API_RATE_H__ */


