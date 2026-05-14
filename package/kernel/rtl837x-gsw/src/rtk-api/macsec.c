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
 * Purpose : RTL8371c switch high-level API
 *
 * Feature : The file includes MACsec module high-layer API defination
 *
 */

 #include <rtk_switch.h>
#include <rtk_error.h>
#include <macsec.h>
#include <string.h>

#include <dal/dal_mgmt.h>

/* Function Name:
 *      rtk_macsec_enable_set
 * Description:
 *      Configure macsec enable.
 * Input:
 *      port   - port id
 *      ingress_en  - ingress enable
 *      egress_en  -  egress enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec enable.
 */
rtk_api_ret_t rtk_macsec_enable_set(rtk_uint32 port, rtk_uint32 ingress_en, rtk_uint32 egress_en)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->macsec_enable_set)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_enable_set(port, ingress_en, egress_en);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_enable_get
 * Description:
 *      get macsec enable status.
 * Input:
 *      port   - port id
 * Output:
 *      ingress_en  - ingress enable
 *      egress_en  -  egress enable
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get macsec enable status.
 */
rtk_api_ret_t rtk_macsec_enable_get(rtk_uint32 port, rtk_uint32 *ingress_en, rtk_uint32 *egress_en)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->macsec_enable_get)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_enable_get(port, ingress_en, egress_en);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_egress_set
 * Description:
 *      Configure macsec egress rule.
 * Input:
 *      port  - port id
 *      addr - macsec ip core register address
 *      value - data for rule
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec egress rule:SA match rule, flow control register and transform record.
 */
rtk_api_ret_t rtk_macsec_egress_set(rtk_uint32 port, rtk_uint32 addr, rtk_uint32 value)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->macsec_egress_set)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_egress_set(port, addr, value);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_egress_get
 * Description:
 *      get macsec egress rule.
 * Input:
 *      port  -  port id
 *      addr -  macsec ip core register address
 * Output:
 *      value  - data for rule
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get macsec egress rule:SA match rule, flow control register and transform record.
 */
rtk_api_ret_t rtk_macsec_egress_get(rtk_uint32 port, rtk_uint32 addr, rtk_uint32 *value)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->macsec_egress_get)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_egress_get(port, addr, value);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_ingress_set
 * Description:
 *      Configure macsec ingress rule.
 * Input:
 *      port  - port id
 *      addr - macsec ip core register address
 *      value - data for rule
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec ingress rule:SA match rule, flow control register and transform record.
 */
rtk_api_ret_t rtk_macsec_ingress_set(rtk_uint32 port, rtk_uint32 addr, rtk_uint32 value)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->macsec_ingress_set)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_ingress_set(port, addr, value);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_ingress_get
 * Description:
 *      get macsec egress rule.
 * Input:
 *      port  -  port id
 *      addr -  macsec ip core register address
 * Output:
 *      value  - data for rule
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get macsec ingress rule:SA match rule, flow control register and transform record.
 */
rtk_api_ret_t rtk_macsec_ingress_get(rtk_uint32 port, rtk_uint32 addr, rtk_uint32 *value)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->macsec_ingress_get)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_ingress_get(port, addr, value);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_rxgating_set
 * Description:
 *      Configure macsec rx gating value
 * Input:
 *      port  - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec rx gating value,before set the packet flow path called this API.
 */
rtk_api_ret_t rtk_macsec_rxgating_set(rtk_uint32 port)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->macsec_rxgating_set)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_rxgating_set(port);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_rxgating_cancel
 * Description:
 *      Configure macsec rx gating value
 * Input:
 *      port  - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec rx gating value, called after packet flow path changed.
 */
rtk_api_ret_t rtk_macsec_rxgating_cancel(rtk_uint32 port)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->macsec_rxgating_cancel)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_rxgating_cancel(port);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_txgating_set
 * Description:
 *      Configure macsec tx gating value
 * Input:
 *      port  - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec tx gating value, before set the packet flow path called this API.
 */
rtk_api_ret_t rtk_macsec_txgating_set(rtk_uint32 port)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->macsec_txgating_set)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_txgating_set(port);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_txgating_cancel
 * Description:
 *      Configure macsec tx gating value
 * Input:
 *      port  - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec tx gating value, called after packet flow path changed..
 */
rtk_api_ret_t rtk_macsec_txgating_cancel(rtk_uint32 port)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->macsec_txgating_cancel)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_txgating_cancel(port);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_rxIPbypass_set
 * Description:
 *      Configure macsec bypass MACsec IP function.
 * Input:
 *      port  - port id
 *      enable  - enable ip bypass
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec bypass MACsec IP function.
 */
rtk_api_ret_t rtk_macsec_rxIPbypass_set(rtk_uint32 port, rtk_uint32 enable)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->macsec_rxIPbypass_set)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_rxIPbypass_set(port, enable);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_rxIPbypass_get
 * Description:
 *      get macsec bypass MACsec IP function status.
 * Input:
 *      port  - port id
 * Output:
 *      enable  - enable ip bypass
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get macsec bypass MACsec IP function status.
 */
rtk_api_ret_t rtk_macsec_rxIPbypass_get(rtk_uint32 port, rtk_uint32 *enable)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->macsec_rxIPbypass_get)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_rxIPbypass_get(port, enable);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_txIPbypass_set
 * Description:
 *      Configure macsec bypass MACsec IP function.
 * Input:
 *      port  - port id
 *      enable  - enable ip bypass
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec bypass MACsec IP function.
 */
rtk_api_ret_t rtk_macsec_txIPbypass_set(rtk_uint32 port, rtk_uint32 enable)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->macsec_txIPbypass_set)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_txIPbypass_set(port, enable);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_txIPbypass_get
 * Description:
 *      Configure macsec bypass MACsec IP function.
 * Input:
 *      port  - port id
 *      enable  - enable ip bypass
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec bypass MACsec IP function.
 */
rtk_api_ret_t rtk_macsec_txIPbypass_get(rtk_uint32 port, rtk_uint32 *enable)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->macsec_txIPbypass_get)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_txIPbypass_get(port, enable);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_rxbypass_set
 * Description:
 *      Configure macsec bypass in MACsec IP function.
 * Input:
 *      port  - port id
 *      enable  - enable ip bypass
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec bypass in MACsec IP function.
 */
rtk_api_ret_t rtk_macsec_rxbypass_set(rtk_uint32 port, rtk_uint32 enable)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->macsec_rxbypass_set)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_rxbypass_set(port, enable);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_rxbypass_get
 * Description:
 *      get macsec bypass in MACsec IP function status.
 * Input:
 *      port  - port id
 * Output:
 *      enable  - enable ip bypass
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get macsec bypass in MACsec IP function status.
 */
rtk_api_ret_t rtk_macsec_rxbypass_get(rtk_uint32 port, rtk_uint32 *enable)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->macsec_rxbypass_get)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_rxbypass_get(port, enable);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_txbypass_set
 * Description:
 *      Configure macsec bypass in MACsec IP function.
 * Input:
 *      port  - port id
 *      enable  - enable ip bypass
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure macsec bypass in MACsec IP function.
 */
rtk_api_ret_t rtk_macsec_txbypass_set(rtk_uint32 port, rtk_uint32 enable)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->macsec_txbypass_set)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_txbypass_set(port, enable);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_txbypass_get
 * Description:
 *      get macsec bypass in MACsec IP function status.
 * Input:
 *      port  - port id
 * Output:
 *      enable  - enable ip bypass
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get macsec bypass in MACsec IP function status.
 */
rtk_api_ret_t rtk_macsec_txbypass_get(rtk_uint32 port, rtk_uint32 *enable)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->macsec_txbypass_get)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_txbypass_get(port, enable);
	RTK_API_UNLOCK();

	return retVal;
}


/* Function Name:
 *      rtk_wrapper_int_control_set
 * Description:
 *      Configure MACsec interrupt.
 * Input:
 *      port  -  port id
 *      type  -  interrupt type
 *      enable - enable interrupt
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure MACsec interrupt.
 *      The interrupt trigger status is shown in the following:
 *      - INT_TYPE_TX_IPE_GLB
 *      - INT_TYPE_TX_IPESECFAIL
 *      - INT_TYPE_TX_IPELOCK
 *      - INT_TYPE_TX_IPELOCK_XG
 *      - INT_TYPE_RX_IPI_GLB
 *      - INT_TYPE_RX_IPISECFAIL
 *      - INT_TYPE_RX_IPILOCK
 *      - INT_TYPE_RX_IPILOCK_XG
 */
rtk_api_ret_t rtk_wrapper_int_control_set(rtk_uint32 port, rtk_macsec_int_type_t type, rtk_enable_t enable)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->wrapper_int_control_set)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->wrapper_int_control_set(port, type, enable);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_wrapper_int_control_get
 * Description:
 *      gonfigure MACsec interrupt.
 * Input:
 *      port  -  port id
 *      type  -  interrupt type
 * Output:
 *      pEnable - enable interrupt
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get MACsec interrupt.
 *      The interrupt trigger status is shown in the following:
 *      - INT_TYPE_TX_IPE_GLB
 *      - INT_TYPE_TX_IPESECFAIL
 *      - INT_TYPE_TX_IPELOCK
 *      - INT_TYPE_TX_IPELOCK_XG
 *      - INT_TYPE_RX_IPI_GLB
 *      - INT_TYPE_RX_IPISECFAIL
 *      - INT_TYPE_RX_IPILOCK
 *      - INT_TYPE_RX_IPILOCK_XG
 */
rtk_api_ret_t rtk_wrapper_int_control_get(rtk_uint32 port, rtk_macsec_int_type_t type, rtk_enable_t *pEnable)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->wrapper_int_control_get)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->wrapper_int_control_get(port, type, pEnable);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_wrapper_int_status_set
 * Description:
 *      Configure MACsec interrupt status.
 * Input:
 *      port  -  port id
 *      statusMask  -  interrupt status mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will clean MACsec interrupt status when interrupt happened.
 *      The interrupt trigger status is shown in the following:
 *      - INT_TYPE_TX_IPE_GLB [bit[0]]
 *      - INT_TYPE_TX_IPESECFAIL [bit[1]]
 *      - INT_TYPE_TX_IPELOCK [bit[2]]
 *      - INT_TYPE_TX_IPELOCK_XG [bit[3]]
 *      - INT_TYPE_RX_IPI_GLB [bit[8]]
 *      - INT_TYPE_RX_IPISECFAIL [bit[9]]
 *      - INT_TYPE_RX_IPILOCK [bit[10]]
 *      - INT_TYPE_RX_IPILOCK_XG [bit[11]]
 */
rtk_api_ret_t rtk_wrapper_int_status_set(rtk_uint32 port, rtk_uint32 statusMask)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->wrapper_int_status_set)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->wrapper_int_status_set(port, statusMask);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_wrapper_int_status_get
 * Description:
 *      Configure MACsec interrupt status.
 * Input:
 *      port  -  port id
 *      statusMask  -  interrupt status mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will clean MACsec interrupt status when interrupt happened.
 *      The interrupt trigger status is shown in the following:
 *      - INT_TYPE_TX_IPE_GLB [bit[0]]
 *      - INT_TYPE_TX_IPESECFAIL [bit[1]]
 *      - INT_TYPE_TX_IPELOCK [bit[2]]
 *      - INT_TYPE_TX_IPELOCK_XG [bit[3]]
 *      - INT_TYPE_RX_IPI_GLB [bit[8]]
 *      - INT_TYPE_RX_IPISECFAIL [bit[9]]
 *      - INT_TYPE_RX_IPILOCK [bit[10]]
 *      - INT_TYPE_RX_IPILOCK_XG [bit[11]]
 */
rtk_api_ret_t rtk_wrapper_int_status_get(rtk_uint32 port, rtk_uint32 *pStatusMask)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->wrapper_int_status_get)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->wrapper_int_status_get(port, pStatusMask);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_wrapper_mib_reset
 * Description:
 *      Configure wrapper mib reset.
 * Input:
 *      port  - port id
 *      reset  -  reset value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure wrapper mib reset.
 */
rtk_api_ret_t rtk_wrapper_mib_reset(rtk_uint32 port, rtk_uint32 reset)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->wrapper_mib_reset)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->wrapper_mib_reset(port, reset);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_wrapper_mib_counter
 * Description:
 *      get wrapper mib counters.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get wrapper mib counters.
 */
rtk_api_ret_t rtk_wrapper_mib_counter(rtk_uint32 port, RTL8373_WRAPPER_MIBCOUNTER mibIdx, rtk_uint64* pCounter)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->wrapper_mib_counter)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->wrapper_mib_counter(port, mibIdx, pCounter);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_ipg_len_set
 * Description:
 *      mac mode MACsec ipg length set.
 * Input:
 *      port   -  port number
 *      length - ipg length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will set mac mode MACsec ipg length.
 */
rtk_api_ret_t rtk_macsec_ipg_len_set(rtk_uint32 port, rtk_uint32 length)
{
   	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->macsec_ipg_len_set)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_ipg_len_set(port,length);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_ipg_len_get
 * Description:
 *      mac mode MACsec ipg length get.
 * Input:
 *      port   -  port number
 * Output:
 *      plength - ipg length
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get mac mode MACsec ipg length.
 */
rtk_api_ret_t rtk_macsec_ipg_len_get(rtk_uint32 port, rtk_uint32 *plength)
{
   	rtk_api_ret_t retVal;
	if (NULL == RT_MAPPER->macsec_ipg_len_get)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_ipg_len_get(port,plength);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_ipg_mode_set
 * Description:
 *      mac mode MACsec ipg mode set.
 * Input:
 *      port   -  port number
 *      mode -  ipg config mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will set mac mode MACsec ipg mode.
 *	  mode[1:0]:
		0: don't insert ipg for macsec
		1: insert ipg for macsec according to macsec feedback signal
		2: insert ipg for masec according to ethertype & cfg_macsec_ipg_length
		3: always insert ipg for macsec, length according to cfg_macsec_ipg_length
 */
rtk_api_ret_t rtk_macsec_ipg_mode_set(rtk_uint32 port, rtk_uint32 mode)
{
   	rtk_api_ret_t retVal;
	
	if (NULL == RT_MAPPER->macsec_ipg_mode_set)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_ipg_mode_set(port,mode);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_ipg_mode_get
 * Description:
 *      mac mode MACsec ipg mode get.
 * Input:
 *      port   -  port number
 * Output:
 *      pmode - ipg config mode
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get mac mode MACsec ipg mode.
 *	  mode[1:0]:
		0: don't insert ipg for macsec
		1: insert ipg for macsec according to macsec feedback signal
		2: insert ipg for masec according to ethertype & cfg_macsec_ipg_length
		3: always insert ipg for macsec, length according to cfg_macsec_ipg_length
 */
rtk_api_ret_t rtk_macsec_ipg_mode_get(rtk_uint32 port, rtk_uint32 *pmode)
{
	rtk_api_ret_t retVal;
	
	if (NULL == RT_MAPPER->macsec_ipg_mode_get)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_ipg_mode_get(port,pmode);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_eth_set
 * Description:
 *      mac mode MACsec eth set.
 * Input:
 *      port   -  port number
 *      entry - entry number(0-7)
 *      ethertype - ether type value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will set mac mode MACsec eth.
 */
rtk_api_ret_t rtk_macsec_eth_set(rtk_uint32 port, rtk_uint32 entry, rtk_uint32 ethertype)
{
   	rtk_api_ret_t retVal;
	
	if (NULL == RT_MAPPER->macsec_eth_set)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_eth_set(port, entry, ethertype);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      rtk_macsec_eth_get
 * Description:
 *      mac mode MACsec eth get.
 * Input:
 *      port   -  port number
 *      entry - entry number(0-7)
 * Output:
 *      pethertype - ether type value
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get mac mode MACsec eth.
 */
rtk_api_ret_t rtk_macsec_eth_get(rtk_uint32 port, rtk_uint32 entry, rtk_uint32 *pethertype)
{
   	rtk_api_ret_t retVal;
	
	if (NULL == RT_MAPPER->macsec_eth_get)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_eth_get(port, entry, pethertype);
	RTK_API_UNLOCK();

	return retVal;
}

/* Function Name:
 *      dal_rtl8371c_macsec_init
 * Description:
 *      Initialize MACsec information.
 * Input:
 *      port_mask   -  port mask, bit[4:7]
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will initialize MACsec information.
 */
rtk_api_ret_t rtk_macsec_init(rtk_uint32 port_mask)
{
	rtk_api_ret_t retVal;

	if (NULL == RT_MAPPER->macsec_init)
	    return RT_ERR_DRIVER_NOT_FOUND;

	RTK_API_LOCK();
	retVal = RT_MAPPER->macsec_init(port_mask);
	RTK_API_UNLOCK();

	return retVal;
}








