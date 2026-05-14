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
 * Feature : Here is a list of all functions and variables in ingress bandwitdh control and egress queue bandwidth control module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal/rtl8373/dal_rtl8373_rate.h>
#include <vlan.h>
#include <string.h>

#include <dal/rtl8373/rtl8373_asicdrv.h>


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
rtk_api_ret_t dal_rtl8373_rate_igrBwCtrlPortEn_set(rtk_port_t port, rtk_enable_t bwEn)
{
	rtk_uint32 retVal = 0;
	
	/* Check initialization state */
	RTK_CHK_INIT_STATE();

	/* Check Port Valid */
	RTK_CHK_PORT_VALID(port);
	if(port > INBW_CTRL_MAX_PORT_ID)
		return RT_ERR_INBW_PORT_ID;

	if(bwEn >= RTK_ENABLE_END)
		return RT_ERR_INPUT;

	if((retVal = rtl8373_setAsicRegBit(RTL8373_IGBW_PORT_CTRL_ADDR(port), RTL8373_IGBW_PORT_CTRL_BW_EN_OFFSET, bwEn)) != RT_ERR_OK)
		return retVal;
	
	return RT_ERR_OK;

}

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
rtk_api_ret_t dal_rtl8373_rate_igrBwCtrlPortEn_get(rtk_port_t port, rtk_enable_t *pBwEn)
{
	rtk_uint32 retVal = 0, regVal = 0;
	
	/* Check initialization state */
	RTK_CHK_INIT_STATE();

	/* Check Port Valid */
	RTK_CHK_PORT_VALID(port);
	if(port > INBW_CTRL_MAX_PORT_ID)
		return RT_ERR_INBW_PORT_ID;

	if(pBwEn == NULL)
		return RT_ERR_NULL_POINTER;

	if((retVal = rtl8373_getAsicRegBit(RTL8373_IGBW_PORT_CTRL_ADDR(port), RTL8373_IGBW_PORT_CTRL_BW_EN_OFFSET, &regVal)) != RT_ERR_OK)
		return retVal;
	*pBwEn = (rtk_enable_t)regVal;
	
	return RT_ERR_OK;

}

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
rtk_api_ret_t dal_rtl8373_rate_igrBwCtrlRate_set(rtk_port_t port, rtk_rate_t rate, rtk_enable_t fcEn)
{
	rtk_uint32 retVal = 0, regData = 0;
	/* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);
	
	if (port > INBW_CTRL_MAX_PORT_ID)
		return RT_ERR_INBW_PORT_ID;

    if (fcEn >= RTK_ENABLE_END)
		return RT_ERR_INPUT;

	if (rate > INBW_CTRL_RATE_MAX)
		return RT_ERR_INBW_RATE;
	
	regData = (rate >> 4);
    if ((retVal = rtl8373_setAsicRegBits(RTL8373_IGBW_PORT_CTRL_ADDR(port), RTL8373_IGBW_PORT_CTRL_RATE_MASK, regData)) != RT_ERR_OK)
		return retVal;

	if ((retVal = rtl8373_setAsicRegBit(RTL8373_IGBW_PORT_FC_CTRL_ADDR(port), RTL8373_IGBW_PORT_FC_CTRL_EN_OFFSET(port), fcEn)) != RT_ERR_OK)
		return retVal;
	
	return RT_ERR_OK;	
}

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
rtk_api_ret_t dal_rtl8373_rate_igrBwCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pFcEn)
{
	rtk_uint32 retVal = 0, regData = 0;
	/* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);
	if(port > INBW_CTRL_MAX_PORT_ID)
		return RT_ERR_INBW_PORT_ID;

    if((pRate == NULL) || (pFcEn == NULL))
        return RT_ERR_NULL_POINTER;
	
    if((retVal = rtl8373_getAsicRegBits(RTL8373_IGBW_PORT_CTRL_ADDR(port), RTL8373_IGBW_PORT_CTRL_RATE_MASK, &regData)) != RT_ERR_OK)
		return retVal;
	*pRate = (regData << 4);

	if((retVal = rtl8373_getAsicRegBit(RTL8373_IGBW_PORT_FC_CTRL_ADDR(port), RTL8373_IGBW_PORT_FC_CTRL_EN_OFFSET(port), &regData)) != RT_ERR_OK)
		return retVal;
	*pFcEn = (rtk_enable_t)regData;
	
	return RT_ERR_OK;	
}

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
rtk_api_ret_t dal_rtl8373_rate_igrBwCtrlIfg_set(rtk_enable_t ifgInclude)
{
	rtk_uint32 retVal = 0;
	
	if(ifgInclude >= RTK_ENABLE_END)
		return RT_ERR_INPUT;
	
    if((retVal = rtl8373_setAsicRegBit(RTL8373_IGBW_CTRL_ADDR, RTL8373_IGBW_CTRL_INC_IFG_OFFSET, ifgInclude)) != RT_ERR_OK)
		return retVal;
	
	return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_rate_igrBwCtrlIfg_get(rtk_enable_t *pIfgInclude)
{
	rtk_uint32 retVal = 0, regVal = 0;
	
	if(pIfgInclude == NULL)
        return RT_ERR_INPUT;
	
    if((retVal = rtl8373_getAsicRegBit(RTL8373_IGBW_CTRL_ADDR, RTL8373_IGBW_CTRL_INC_IFG_OFFSET, &regVal)) != RT_ERR_OK)
		return retVal;
	*pIfgInclude = (rtk_enable_t)regVal;
	
	return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_rate_igrBwCtrlCongestSts_get(rtk_port_t port, rtk_rate_igrBwCongestSts_t *pCongestSts)
{
	rtk_uint32 retVal = 0, regVal = 0;
	
	if(pCongestSts == NULL)
        return RT_ERR_INPUT;
	
    if((retVal = rtl8373_getAsicRegBit(RTL8373_IGBW_PORT_CNGST_FLAG_ADDR(port), RTL8373_IGBW_PORT_CNGST_FLAG_FLAG_OFFSET(port), &regVal)) != RT_ERR_OK)
		return retVal;
	*pCongestSts = (rtk_rate_igrBwCongestSts_t)regVal;
	
	return RT_ERR_OK;
}


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
rtk_api_ret_t dal_rtl8373_rate_egrBwCtrlPortEn_set(rtk_port_t port, rtk_enable_t bwEn)
{
	rtk_uint32 retVal = 0;
	
	/* Check initialization state */
	RTK_CHK_INIT_STATE();

	/* Check Port Valid */
	RTK_CHK_PORT_VALID(port);
	if(port > EBW_CTRL_MAX_PORT_ID)
		return RT_ERR_QOS_EBW_PORT_ID;

	if(bwEn >= RTK_ENABLE_END)
		return RT_ERR_INPUT;

	if((retVal = rtl8373_setAsicRegBit(RTL8373_EGBW_PORT_CTRL_ADDR(port), RTL8373_EGBW_PORT_CTRL_EN_OFFSET, bwEn)) != RT_ERR_OK)
		return retVal;
	
	return RT_ERR_OK;

}

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
rtk_api_ret_t dal_rtl8373_rate_egrBwCtrlPortEn_get(rtk_port_t port, rtk_enable_t *pBwEn)
{
	rtk_uint32 retVal = 0, regVal = 0;
	
	/* Check initialization state */
	RTK_CHK_INIT_STATE();

	/* Check Port Valid */
	RTK_CHK_PORT_VALID(port);
	if(port > EBW_CTRL_MAX_PORT_ID)
		return RT_ERR_QOS_EBW_PORT_ID;

	if(pBwEn == NULL)
		return RT_ERR_NULL_POINTER;

	if((retVal = rtl8373_getAsicRegBit(RTL8373_EGBW_PORT_CTRL_ADDR(port), RTL8373_EGBW_PORT_CTRL_EN_OFFSET, &regVal)) != RT_ERR_OK)
		return retVal;
	*pBwEn = (rtk_enable_t)regVal;
	
	return RT_ERR_OK;

}


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
rtk_api_ret_t dal_rtl8373_rate_egrBwCtrlRate_set(rtk_port_t port, rtk_rate_t rate, rtk_enable_t ipg)
{
	rtk_uint32 retVal = 0, regData = 0;
	/* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);
	if(port > EBW_CTRL_MAX_PORT_ID)
		return RT_ERR_QOS_EBW_PORT_ID;

    if(ipg >= RTK_ENABLE_END)
		return RT_ERR_INPUT;


	if(rate > EBW_CTRL_RATE_MAX)
		return RT_ERR_QOS_EBW_RATE;
	
	regData = (rate >> 4);
    if((retVal = rtl8373_setAsicRegBits(RTL8373_EGBW_PORT_CTRL_ADDR(port), RTL8373_EGBW_PORT_CTRL_RATE_MASK, regData)) != RT_ERR_OK)
		return retVal;

	if((retVal = rtl8373_setAsicRegBit(RTL8373_EGBW_CTRL_ADDR, RTL8373_EGBW_CTRL_INC_IFG_OFFSET, ipg)) != RT_ERR_OK)
		return retVal;
	
	return RT_ERR_OK;	
}

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
rtk_api_ret_t dal_rtl8373_rate_egrBwCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIpg)
{
	rtk_uint32 retVal = 0, regData = 0;
	/* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);
	if(port > EBW_CTRL_MAX_PORT_ID)
		return RT_ERR_QOS_EBW_PORT_ID;

    if((pRate == NULL) || (pIpg == NULL))
        return RT_ERR_NULL_POINTER;
	
    if((retVal = rtl8373_getAsicRegBits(RTL8373_EGBW_PORT_CTRL_ADDR(port), RTL8373_EGBW_PORT_CTRL_RATE_MASK, &regData)) != RT_ERR_OK)
		return retVal;
	*pRate = (regData << 4);

	if((retVal = rtl8373_getAsicRegBit(RTL8373_EGBW_CTRL_ADDR, RTL8373_EGBW_CTRL_INC_IFG_OFFSET, &regData)) != RT_ERR_OK)
    	return retVal;
	*pIpg = (rtk_enable_t)regData;
	
	return RT_ERR_OK;	
}



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
rtk_api_ret_t dal_rtl8373_rate_egrQueueMaxBwEn_set(rtk_port_t port, rtk_qid_t qid, rtk_enable_t enable)
{
	rtk_uint32 retVal = 0;
	/* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);
	if(port > EBW_CTRL_MAX_PORT_ID)
		return RT_ERR_QOS_EBW_PORT_ID;

    if(enable >= RTK_ENABLE_END)
		return RT_ERR_INPUT;


    if((retVal = rtl8373_setAsicRegBit(RTL8373_EGBW_PORT_Q_MAX_LB_CTRL_SET_ADDR(port, qid), RTL8373_EGBW_PORT_Q_MAX_LB_CTRL_SET_EN_OFFSET, enable)) != RT_ERR_OK)
		return retVal;

	
	return RT_ERR_OK;	
}


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
rtk_api_ret_t dal_rtl8373_rate_egrQueueMaxBwEn_get(rtk_port_t port, rtk_qid_t qid, rtk_enable_t * pEnable)
{
	rtk_uint32 retVal = 0;
	/* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);
	if(port > EBW_CTRL_MAX_PORT_ID)
		return RT_ERR_QOS_EBW_PORT_ID;


    if((retVal = rtl8373_getAsicRegBit(RTL8373_EGBW_PORT_Q_MAX_LB_CTRL_SET_ADDR(port, qid), RTL8373_EGBW_PORT_Q_MAX_LB_CTRL_SET_EN_OFFSET, pEnable)) != RT_ERR_OK)
		return retVal;

	
	return RT_ERR_OK;	
}


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
rtk_api_ret_t dal_rtl8373_rate_egrQueueMaxBwRate_set(rtk_port_t port, rtk_qid_t qid, rtk_rate_t rate)
{
	rtk_uint32 retVal = 0;
	/* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);
	if(port > EBW_CTRL_MAX_PORT_ID)
		return RT_ERR_QOS_EBW_PORT_ID;

    if(rate >= EBW_CTRL_RATE_MAX)
        return RT_ERR_INPUT;


    if((retVal = rtl8373_setAsicRegBits(RTL8373_EGBW_PORT_Q_MAX_LB_CTRL_SET_ADDR(port, qid), RTL8373_EGBW_PORT_Q_MAX_LB_CTRL_SET_RATE_MASK, rate)) != RT_ERR_OK)
		return retVal;

	
	return RT_ERR_OK;	
}


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
rtk_api_ret_t dal_rtl8373_rate_egrQueueMaxBwRate_get(rtk_port_t port, rtk_qid_t qid, rtk_rate_t * pRate)
{
	rtk_uint32 retVal = 0;
	/* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);
	if(port > EBW_CTRL_MAX_PORT_ID)
		return RT_ERR_QOS_EBW_PORT_ID;

    if(pRate == NULL)
        return RT_ERR_NULL_POINTER;


    if((retVal = rtl8373_getAsicRegBits(RTL8373_EGBW_PORT_Q_MAX_LB_CTRL_SET_ADDR(port, qid), RTL8373_EGBW_PORT_Q_MAX_LB_CTRL_SET_RATE_MASK, pRate)) != RT_ERR_OK)
		return retVal;

	
	return RT_ERR_OK;	
}




