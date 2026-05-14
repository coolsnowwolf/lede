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
 * Feature : The file includes Rate module high-layer API defination
 *
 */

#ifndef  __DAL_RTL8373_RATE_H__
#define  __DAL_RTL8373_RATE_H__
#include <rate.h>

/* Function Name:
 *      dal_rtl8373_rate_igrBwCtrlPortEn_set
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
extern rtk_api_ret_t dal_rtl8373_rate_igrBwCtrlPortEn_set(rtk_port_t port, rtk_enable_t bwEn);

/* Function Name:
 *      dal_rtl8373_rate_igrBwCtrlPortEn_get
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
extern rtk_api_ret_t dal_rtl8373_rate_igrBwCtrlPortEn_get(rtk_port_t port, rtk_enable_t *pBwEn);

/* Function Name:
 *      dal_rtl8373_rate_igrBwCtrlRate_set
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
extern rtk_api_ret_t dal_rtl8373_rate_igrBwCtrlRate_set(rtk_port_t port, rtk_rate_t rate, rtk_enable_t fcEn);

/* Function Name:
 *      dal_rtl8373_rate_igrBwCtrlRate_get
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
extern rtk_api_ret_t dal_rtl8373_rate_igrBwCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pFcEn);

/* Function Name:
 *      dal_rtl8373_rate_igrBwCtrlIfg_set
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
extern rtk_api_ret_t dal_rtl8373_rate_igrBwCtrlIfg_set(rtk_enable_t ifgInclude);

/* Function Name:
 *      dal_rtl8373_rate_igrBwCtrlIfg_get
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
extern rtk_api_ret_t dal_rtl8373_rate_igrBwCtrlIfg_get(rtk_enable_t *pIfgInclude);

/* Function Name:
 *      dal_rtl8373_rate_igrBwCtrlCongestSts_get
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
extern rtk_api_ret_t dal_rtl8373_rate_igrBwCtrlCongestSts_get(rtk_port_t port, rtk_rate_igrBwCongestSts_t *pCongestSts);


/* Function Name:
 *      dal_rtl8373_rate_egrBwCtrlPortEn_set
 * Description:
 *      Enable or disable port egress bandwidth control 
 * Input:
 *      port        - Port id
 *      bwEn        - enable egress bandwidth control or not
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
extern rtk_api_ret_t dal_rtl8373_rate_egrBwCtrlPortEn_set(rtk_port_t port, rtk_enable_t bwEn);

/* Function Name:
 *      dal_rtl8373_rate_egrBwCtrlPortEn_get
 * Description:
 *      Enable or disable port egress bandwidth control 
 * Input:
 *      port        - Port id
 *
 * Output:
 *      pBwEn        - Port egress bandwidth control state
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
extern rtk_api_ret_t dal_rtl8373_rate_egrBwCtrlPortEn_get(rtk_port_t port, rtk_enable_t *pBwEn);


/* Function Name:
 *      dal_rtl8373_rate_egrBwCtrlRate_set
 * Description:
 *      Set port egress bandwidth control rate and IPG config
 * Input:
 *      port        - Port id
 *      rate        - Rate of share meter(uint: kpbs)
 *      ipg  	    - include ipg or not, ENABLE:include DISABLE:exclude
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
extern rtk_api_ret_t dal_rtl8373_rate_egrBwCtrlRate_set(rtk_port_t port, rtk_rate_t rate, rtk_enable_t ipg);

/* Function Name:
 *      dal_rtl8373_rate_egrBwCtrlRate_get
 * Description:
 *      Get port egress bandwidth control rate and IPG config
 * Input:
 *      port        - Port id
 *
 * Output:
 *      pRate        - Rate of share meter(uint: kpbs)
 *      pIpg         - include ipg or not, ENABLE:include DISABLE:exclude
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
extern rtk_api_ret_t dal_rtl8373_rate_egrBwCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIpg);



/* Function Name:
 *      dal_rtl8373_rate_egrQueueMaxBwEn_set
 * Description:
 *      Set port egress queue max bandwidth enable
 * Input:
 *      port        - Port id
 *      qid         - Queue ID
 *      enable 	    - 1:enable, 0:disable
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
 */
extern rtk_api_ret_t dal_rtl8373_rate_egrQueueMaxBwEn_set(rtk_port_t port, rtk_qid_t qid, rtk_enable_t enable);


/* Function Name:
 *      dal_rtl8373_rate_egrQueueMaxBwEn_gget
 * Description:
 *      Get port egress queue max bandwidth enable
 * Input:
 *      port        - Port id
 *      qid         - Queue ID
 *      pEnable 	    - 1:enable, 0:disable
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
 */
extern rtk_api_ret_t dal_rtl8373_rate_egrQueueMaxBwEn_get(rtk_port_t port, rtk_qid_t qid, rtk_enable_t * pEnable);


/* Function Name:
 *      dal_rtl8373_rate_egrQueueMaxBwRate_set
 * Description:
 *      Set port egress queue max bandwidth rate
 * Input:
 *      port        - Port id
 *      qid         - Queue ID
 *      rate 	    - Rate of share meter(uint: kpbs)
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
 */
extern rtk_api_ret_t dal_rtl8373_rate_egrQueueMaxBwRate_set(rtk_port_t port, rtk_qid_t qid, rtk_rate_t rate);


/* Function Name:
 *      dal_rtl8373_rate_egrQueueMaxBwRate_get
 * Description:
 *      Get port egress queue max bandwidth rate
 * Input:
 *      port        - Port id
 *      qid         - Queue ID
 *      rate 	    - Rate of share meter(uint: kpbs)
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
 */
extern rtk_api_ret_t dal_rtl8373_rate_egrQueueMaxBwRate_get(rtk_port_t port, rtk_qid_t qid, rtk_rate_t * pRate);








#endif /*  __DAL_RTL8373_RATE_H__ */


