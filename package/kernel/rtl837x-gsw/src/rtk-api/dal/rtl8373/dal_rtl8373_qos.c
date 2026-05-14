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
 * Feature : Here is a list of all functions and variables in QoS module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal/rtl8373/dal_rtl8373_qos.h>
#include <string.h>

#include <dal/rtl8373/rtl8373_asicdrv.h>

/* Function Name:
 *      dal_rtl8373_qos_init
 * Description:
 *      Configure Qos default settings.
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
 *      This API will initialize related Qos setting.
 */
rtk_api_ret_t dal_rtl8373_qos_init(void)
{
    CONST_T rtk_uint16 g_prioritytToQid[8]= {0, 1,2,3,4,5,6,7};
    CONST_T rtk_uint32 g_priorityDecision[6] = {0x01, 0x10,0x04,0x02,0x08,0x20};
    CONST_T rtk_uint32 g_prioritytRemap[8] = {0,1,2,3,4,5,6,7};

    rtk_api_ret_t retVal;
    rtk_uint32 priority;
    rtk_uint32 priDec;
    rtk_uint32 priIdx;
    rtk_uint32 port;
    rtk_uint32 dscp;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /*Set Priority to Qid per port*/
    for (port = 0; port < RTL8373_PORTNO; port++)
    {
       for(priority = 0; priority <=RTK_PRIMAX; priority++)
        {
        if ((retVal = rtl8373_setAsicRegBits(RTL8373_QID_TO_PRI_ADDR(port), 0x7 << (priority * 4), g_prioritytToQid[priority])) != RT_ERR_OK)
            return retVal;
       }
    }

    /*Priority Decision Order*/
    for(priIdx = 0; priIdx < RTL8373_PRIIDX_END; priIdx++)
    	{
	    for (priDec = 0;priDec < RTL8373_PRIDEC_END;priDec++)
	    {
	        if ((retVal = rtl8373_setAsicRegBits(RTL8373_PRI_WEIGHT_ADDR(priIdx), 0x1f << (priDec * 5), g_priorityDecision[priDec])) != RT_ERR_OK)
	            return retVal;
	    }
    	}

    /*Set per port weight select*/
    for(port = 0; port < RTL8373_PORTNO; port++)
    	{
           if ((retVal = rtl8373_setAsicRegBit(RTL8373_PORT_WEIGHT_SEL_ADDR (port),RTL8373_PORT_WEIGHT_SEL_WEIGHT_SEL_OFFSET(port), 0)) != RT_ERR_OK)
              return retVal;
	}

    /*Set Port-based Priority to 0*/
    RTK_SCAN_ALL_PHY_PORTMASK(port)
    {
        if ((retVal = rtl8373_setAsicRegBits(RTL8373_PORT_PRI_ADDR (port),RTL8373_PORT_PRI_PORT_BASE_PRI_MASK(port), 0)) != RT_ERR_OK)
            return retVal;
    }

    RTK_SCAN_ALL_PHY_PORTMASK(port)
    {
        /*Disable 1p Remarking*/
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_RMK_PORT_CTRL_ADDR(port), RTL8373_RMK_PORT_CTRL_IPRI_RMK_EN_OFFSET, DISABLED)) != RT_ERR_OK)
            return retVal;
        /*Disable DSCP Remarking*/
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_RMK_PORT_CTRL_ADDR(port), RTL8373_RMK_PORT_CTRL_DSCP_RMK_EN_OFFSET, DISABLED)) != RT_ERR_OK)
            return retVal;
    }

    /*Set 1p & DSCP  Priority Remapping & Remarking*/
    for (priority = 0; priority <= RTK_PRIMAX; priority++)
    {
        if ((retVal = rtl8373_setAsicRegBits(RTL8373_DOT1Q_PRI_REMAP_ADDR, 0x7<< (priority * 4), g_prioritytRemap[priority])) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8373_setAsicRegBit(RTL8373_RMK_CTRL_ADDR ,RTL8373_RMK_CTRL_IPRI_RMK_SRC_OFFSET, 0))!= RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8373_setAsicRegBit(RTL8373_RMK_CTRL_ADDR,RTL8373_RMK_CTRL_DSCP_RMK_SRC_OFFSET, 0)) != RT_ERR_OK)
            return retVal;

	 /*set RSPAN Priority Remapping*/
	 if ((retVal = rtl8373_setAsicRegBits(RTL8373_RSPAN_PRI_REMAP_ADDR, 0x7<< (priority * 4), g_prioritytRemap[priority])) != RT_ERR_OK)
            return retVal;
    }

    /*Set DSCP Priority*/
    for (dscp = 0; dscp <= 63; dscp++)
    {
        if ((retVal = rtl8373_setAsicRegBits(RTL8373_PRI_SEL_REMAP_DSCP_ADDR(dscp),RTL8373_PRI_SEL_REMAP_DSCP_INTPRI_DSCP_MASK(dscp), 0)) != RT_ERR_OK)
            return retVal;
    }

    /* Finetune B/T value */
  //  if((retVal = rtl8373_setAsicReg(0x1722, 0x1158)) != RT_ERR_OK)
   //     return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_priSel_set
 * Description:
 *      Configure the priority order among different priority mechanism.
 * Input:
 *      index - Priority decision table index (0~1)
 *      pPriDec - Priority assign for port, dscp, 802.1p,  svlan, acl based priority decision.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_QOS_SEL_PRI_SOURCE   - Invalid priority decision source parameter.
 * Note:
 *      ASIC will follow user priority setting of mechanisms to select mapped queue priority for receiving frame.
 *      If two priority mechanisms are the same, the ASIC will chose the highest priority from mechanisms to
 *      assign queue priority to receiving frame.
 *      The priority sources are:
 *      - RTL8373_PRIDEC_PORT
 *      - RTL8373_PRIDEC_ACL
 *      - RTL8373_PRIDEC_DSCP
 *      - RTL8373_PRIDEC_1Q
 *      - RTL8373_PRIDEC_SVLAN
 */
rtk_api_ret_t dal_rtl8373_qos_priSel_set(rtk_qos_priDecTbl_t index, rtk_priority_select_t *pPriDec)
{
    rtk_api_ret_t retVal;
    rtk_uint32 port_pow;
    rtk_uint32 dot1q_pow;
    rtk_uint32 dscp_pow;
    rtk_uint32 acl_pow;
    rtk_uint32 svlan_pow;
    rtk_uint32 i;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (index < 0 || index >= PRIDECTBL_END)
        return RT_ERR_ENTRY_INDEX;

    if (pPriDec->port_pri >= 5 || pPriDec->dot1q_pri >= 5 || pPriDec->acl_pri >= 5 || pPriDec->dscp_pri >= 5 ||
       pPriDec->svlan_pri >= 5)
        return RT_ERR_QOS_SEL_PRI_SOURCE;

    port_pow = 1;
    for (i = pPriDec->port_pri; i > 0; i--)
        port_pow = (port_pow)*2;

    dot1q_pow = 1;
    for (i = pPriDec->dot1q_pri; i > 0; i--)
        dot1q_pow = (dot1q_pow)*2;

    acl_pow = 1;
    for (i = pPriDec->acl_pri; i > 0; i--)
        acl_pow = (acl_pow)*2;

    dscp_pow = 1;
    for (i = pPriDec->dscp_pri; i > 0; i--)
        dscp_pow = (dscp_pow)*2;

    svlan_pow = 1;
    for (i = pPriDec->svlan_pri; i > 0; i--)
        svlan_pow = (svlan_pow)*2;

    
    if ((retVal = rtl8373_setAsicRegBits(RTL8373_PRI_WEIGHT_ADDR(index), RTL8373_PRI_WEIGHT_PORT_WEIGHT_MASK, port_pow)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_PRI_WEIGHT_ADDR(index), RTL8373_PRI_WEIGHT_DOT1Q_WEIGHT_MASK, dot1q_pow)) != RT_ERR_OK)
        return retVal;

   if ((retVal = rtl8373_setAsicRegBits(RTL8373_PRI_WEIGHT_ADDR(index), RTL8373_PRI_WEIGHT_DSCP_WEIGHT_MASK, dscp_pow)) != RT_ERR_OK)
        return retVal;

   if ((retVal = rtl8373_setAsicRegBits(RTL8373_PRI_WEIGHT_ADDR(index), RTL8373_PRI_WEIGHT_ACL_WEIGHT_MASK, acl_pow)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_PRI_WEIGHT_ADDR(index), RTL8373_PRI_WEIGHT_SVLAN_WEIGHT_MASK, svlan_pow)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_priSel_get
 * Description:
 *      Get the priority order configuration among different priority mechanism.
 * Input:
 *      index - Priority decision table index (0~1)
 * Output:
 *      pPriDec - Priority assign for port, dscp, 802.1p, svlan, acl based priority decision .
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      ASIC will follow user priority setting of mechanisms to select mapped queue priority for receiving frame.
 *      If two priority mechanisms are the same, the ASIC will chose the highest priority from mechanisms to
 *      assign queue priority to receiving frame.
 *      The priority sources are:
 *      - RTL8373_PRIDEC_PORT
 *      - RTL8373_PRIDEC_ACL
 *      - RTL8373_PRIDEC_DSCP
 *      - RTL8373_PRIDEC_1Q
 *      - RTL8373_PRIDEC_SVLAN
 */
rtk_api_ret_t dal_rtl8373_qos_priSel_get(rtk_qos_priDecTbl_t index, rtk_priority_select_t *pPriDec)
{

    rtk_api_ret_t retVal;
    rtk_int32 i;
    rtk_uint32 port_pow;
    rtk_uint32 dot1q_pow;
    rtk_uint32 dscp_pow;
    rtk_uint32 acl_pow;
    rtk_uint32 svlan_pow;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (index < 0 || index >= PRIDECTBL_END)
        return RT_ERR_ENTRY_INDEX;

    memset(pPriDec, 0x00, sizeof(rtk_priority_select_t));

   if ((retVal = rtl8373_getAsicRegBits(RTL8373_PRI_WEIGHT_ADDR(index), RTL8373_PRI_WEIGHT_PORT_WEIGHT_MASK, &port_pow)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_PRI_WEIGHT_ADDR(index), RTL8373_PRI_WEIGHT_DOT1Q_WEIGHT_MASK, &dot1q_pow)) != RT_ERR_OK)
        return retVal;

   if ((retVal = rtl8373_getAsicRegBits(RTL8373_PRI_WEIGHT_ADDR(index), RTL8373_PRI_WEIGHT_DSCP_WEIGHT_MASK, &dscp_pow)) != RT_ERR_OK)
        return retVal;

   if ((retVal = rtl8373_getAsicRegBits(RTL8373_PRI_WEIGHT_ADDR(index), RTL8373_PRI_WEIGHT_ACL_WEIGHT_MASK, &acl_pow)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_PRI_WEIGHT_ADDR(index), RTL8373_PRI_WEIGHT_SVLAN_WEIGHT_MASK, &svlan_pow)) != RT_ERR_OK)
        return retVal;
	
    for (i = 31; i >= 0; i--)
    {
        if (port_pow & (1 << i))
        {
            pPriDec->port_pri = i;
            break;
        }
    }

    for (i = 31; i >= 0; i--)
    {
        if (dot1q_pow & (1 << i))
        {
            pPriDec->dot1q_pri = i;
            break;
        }
    }

    for (i = 31; i >= 0; i--)
    {
        if (acl_pow & (1 << i))
        {
            pPriDec->acl_pri = i;
            break;
        }
    }

    for (i = 31; i >= 0; i--)
    {
        if (dscp_pow & (1 << i))
        {
            pPriDec->dscp_pri = i;
            break;
        }
    }

    for (i = 31; i >= 0; i--)
    {
        if (svlan_pow & (1 << i))
        {
            pPriDec->svlan_pri = i;
            break;
        }
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_1pPriRemap_set
 * Description:
 *      Configure 1Q priorities mapping to internal absolute priority.
 * Input:
 *      dot1p_pri   - 802.1p priority value.
 *      int_pri     - internal priority value.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_VLAN_PRIORITY    - Invalid 1p priority.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      Priority of 802.1Q assignment for internal asic priority, and it is used for queue usage and packet scheduling.
 */
rtk_api_ret_t dal_rtl8373_qos_1pPriRemap_set(rtk_pri_t dot1p_pri, rtk_pri_t int_pri)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (dot1p_pri > RTL8373_PRIMAX || int_pri > RTL8373_PRIMAX)
        return  RT_ERR_VLAN_PRIORITY;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_DOT1Q_PRI_REMAP_ADDR, 0x7<< (dot1p_pri * 4), int_pri)) != RT_ERR_OK)
            return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_1pPriRemap_get
 * Description:
 *      Get 1Q priorities mapping to internal absolute priority.
 * Input:
 *      dot1p_pri - 802.1p priority value .
 * Output:
 *      pInt_pri - internal priority value.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_VLAN_PRIORITY    - Invalid priority.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      Priority of 802.1Q assigment for internal asic priority, and it is uesed for queue usage and packet scheduling.
 */
rtk_api_ret_t dal_rtl8373_qos_1pPriRemap_get(rtk_pri_t dot1p_pri, rtk_pri_t *pInt_pri)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (dot1p_pri > RTL8373_PRIMAX)
        return  RT_ERR_QOS_INT_PRIORITY;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_DOT1Q_PRI_REMAP_ADDR, 0x7<< (dot1p_pri * 4), pInt_pri)) != RT_ERR_OK)
            return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_dscpPriRemap_set
 * Description:
 *      Map dscp value to internal priority.
 * Input:
 *      dscp    - Dscp value of receiving frame
 *      int_pri - internal priority value .
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_QOS_DSCP_VALUE   - Invalid DSCP value.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      The Differentiated Service Code Point is a selector for router's per-hop behaviors. As a selector, there is no implication that a numerically
 *      greater DSCP implies a better network service. As can be seen, the DSCP totally overlaps the old precedence field of TOS. So if values of
 *      DSCP are carefully chosen then backward compatibility can be achieved.
 */
rtk_api_ret_t dal_rtl8373_qos_dscpPriRemap_set(rtk_dscp_t dscp, rtk_pri_t int_pri)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (int_pri > RTL8373_PRIMAX )
        return RT_ERR_QOS_INT_PRIORITY;

    if (dscp > RTL8373_DSCPMAX)
        return RT_ERR_QOS_DSCP_VALUE;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_PRI_SEL_REMAP_DSCP_ADDR(dscp), RTL8373_PRI_SEL_REMAP_DSCP_INTPRI_DSCP_MASK(dscp),int_pri)) != RT_ERR_OK)
            return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_dscpPriRemap_get
 * Description:
 *      Get dscp value to internal priority.
 * Input:
 *      dscp - Dscp value of receiving frame
 * Output:
 *      pInt_pri - internal priority value.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_QOS_DSCP_VALUE   - Invalid DSCP value.
 * Note:
 *      The Differentiated Service Code Point is a selector for router's per-hop behaviors. As a selector, there is no implication that a numerically
 *      greater DSCP implies a better network service. As can be seen, the DSCP totally overlaps the old precedence field of TOS. So if values of
 *      DSCP are carefully chosen then backward compatibility can be achieved.
 */
rtk_api_ret_t dal_rtl8373_qos_dscpPriRemap_get(rtk_dscp_t dscp, rtk_pri_t *pInt_pri)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (dscp > RTL8373_DSCPMAX)
        return RT_ERR_QOS_DSCP_VALUE;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_PRI_SEL_REMAP_DSCP_ADDR(dscp),RTL8373_PRI_SEL_REMAP_DSCP_INTPRI_DSCP_MASK(dscp), pInt_pri)) != RT_ERR_OK)
            return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_RspanPriRemap_set
 * Description:
 *      Configure RSPAN priorities mapping to internal absolute priority.
 * Input:
 *      rspan_pri   - rspan priority value.
 *      int_pri     - internal priority value.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_VLAN_PRIORITY    - Invalid 1p priority.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      Priority of RSPAN assignment for internal asic priority, and it is used for queue usage and packet scheduling.
 */
rtk_api_ret_t dal_rtl8373_qos_RspanPriRemap_set(rtk_pri_t rspan_pri, rtk_pri_t int_pri)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (rspan_pri > RTL8373_PRIMAX || int_pri > RTL8373_PRIMAX)
        return  RT_ERR_VLAN_PRIORITY;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_RSPAN_PRI_REMAP_ADDR, 0x7<< (rspan_pri * 4), int_pri)) != RT_ERR_OK)
            return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_RspanPriRemap_get
 * Description:
 *      Get RSPAN priorities mapping to internal absolute priority.
 * Input:
 *      rspan_pri   - rspan priority value.
 * Output:
 *      pInt_pri - internal priority value.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_VLAN_PRIORITY    - Invalid priority.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      Priority of RSPAN assigment for internal asic priority, and it is uesed for queue usage and packet scheduling.
 */
rtk_api_ret_t dal_rtl8373_qos_RspanPriRemap_get(rtk_pri_t rspan_pri, rtk_pri_t *pInt_pri)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (rspan_pri > RTL8373_PRIMAX)
        return  RT_ERR_QOS_INT_PRIORITY;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_RSPAN_PRI_REMAP_ADDR, 0x7<< (rspan_pri * 4), pInt_pri)) != RT_ERR_OK)
            return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_portPri_set
 * Description:
 *      Configure priority usage to each port.
 * Input:
 *      port - Port id.
 *      int_pri - internal priority value.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_QOS_SEL_PORT_PRI - Invalid port priority.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      The API can set priority of port assignments for queue usage and packet scheduling.
 */
rtk_api_ret_t dal_rtl8373_qos_portPri_set(rtk_port_t port, rtk_pri_t int_pri)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (int_pri > RTL8373_PRIMAX )
        return RT_ERR_QOS_INT_PRIORITY;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_PORT_PRI_ADDR (port), RTL8373_PORT_PRI_PORT_BASE_PRI_MASK(port),int_pri)) != RT_ERR_OK)
            return retVal;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_PORT_PRI_DUP_ADDR (port), RTL8373_PORT_PRI_DUP_PORT_BASE_PRI_DUP_MASK(port),int_pri)) != RT_ERR_OK)
            return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_portPri_get
 * Description:
 *      Get priority usage to each port.
 * Input:
 *      port - Port id.
 * Output:
 *      pInt_pri - internal priority value.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can get priority of port assignments for queue usage and packet scheduling.
 */
rtk_api_ret_t dal_rtl8373_qos_portPri_get(rtk_port_t port, rtk_pri_t *pInt_pri)
{
    rtk_api_ret_t retVal;
    rtk_pri_t encap_pri;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

   if ((retVal = rtl8373_getAsicRegBits(RTL8373_PORT_PRI_ADDR (port), RTL8373_PORT_PRI_PORT_BASE_PRI_MASK(port), pInt_pri)) != RT_ERR_OK)
            return retVal;

   if ((retVal = rtl8373_getAsicRegBits(RTL8373_PORT_PRI_DUP_ADDR (port), RTL8373_PORT_PRI_DUP_PORT_BASE_PRI_DUP_MASK(port), &encap_pri)) != RT_ERR_OK)
            return retVal;

   if(*pInt_pri != encap_pri)
   	  return RT_ERR_FAILED;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_priMap_set
 * Description:
 *      Set output queue number for each port.
 * Input:
 *      port   - Port ID.
 *      pPri2qid    - Priority mapping to queue ID.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_QUEUE_NUM        - Invalid queue number.
 *      RT_ERR_QUEUE_ID         - Invalid queue id.
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      ASIC supports priority mapping to queue with different internal available queue IDs.
 */
rtk_api_ret_t dal_rtl8373_qos_priMap_set(rtk_port_t port, rtk_qos_pri2queue_t *pPri2qid)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pri;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    for (pri = 0; pri <= RTK_PRIMAX; pri++)
    {
        if (pPri2qid->pri2queue[pri] > RTK_QIDMAX)
            return RT_ERR_QUEUE_ID;

        if ((retVal = rtl8373_setAsicRegBits(RTL8373_QID_TO_PRI_ADDR(port), 0x7 << (pri * 4), pPri2qid->pri2queue[pri])) != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_priMap_get
 * Description:
 *      Get priority to queue ID mapping table parameters.
 * Input:
 *      port   - Port ID.
 * Output:
 *      pPri2qid - Priority mapping to queue ID.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 * Note:
 *      The API can return the mapping queue id of the specified priority.
 */
rtk_api_ret_t dal_rtl8373_qos_priMap_get(rtk_port_t port, rtk_qos_pri2queue_t *pPri2qid)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pri;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    for (pri = 0; pri <= RTK_PRIMAX; pri++)
    {
        if ((retVal = rtl8373_getAsicRegBits(RTL8373_QID_TO_PRI_ADDR(port), 0x7 << (pri * 4), &pPri2qid->pri2queue[pri])) != RT_ERR_OK)
            return retVal;
    }
	
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_qos_schedulingQueue_set
 * Description:
 *      Set weight and type of queues in dedicated port.
 * Input:
 *      port        - Port id.
 *      pQweights   - The array of weights for WRR/WFQ queue (0 for STRICT_PRIORITY queue).
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_QOS_QUEUE_WEIGHT - Invalid queue weight.
 * Note:
 *      The API can set weight and type, strict priority or weight fair queue (WFQ) for
 *      dedicated port for using queues. If queue id is not included in queue usage,
 *      then its type and weight setting in dummy for setting. There are priorities
 *      as queue id in strict queues. It means strict queue id 5 carrying higher priority
 *      than strict queue id 4. The WFQ queue weight is from 1 to 127, and weight 0 is
 *      for strict priority queue type.
 */
rtk_api_ret_t dal_rtl8373_qos_schedulingQueue_set(rtk_port_t port, rtk_qos_queue_weights_t *pQweights)
{
    rtk_api_ret_t retVal;
    rtk_uint32 qid;
    //rtk_uint32 phy_port;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    for (qid = 0; qid < RTL8373_QUEUENO; qid ++)
    {

        if (pQweights->weights[qid] > QOS_WEIGHT_MAX)
            return RT_ERR_QOS_QUEUE_WEIGHT;

        //phy_port = rtk_switch_port_L2P_get(port);

        if (0 == pQweights->weights[qid])
        {
            if ((retVal = rtl8373_setAsicRegBit(RTL8373_SCHED_PORT_Q_CTRL_SET_ADDR(port,qid), RTL8373_SCHED_PORT_Q_CTRL_SET_STRICT_EN_OFFSET,RTL8373_QTYPE_STRICT)) != RT_ERR_OK)
                return retVal;
        }
        else
        {
            if ((retVal = rtl8373_setAsicRegBit(RTL8373_SCHED_PORT_Q_CTRL_SET_ADDR(port,qid), RTL8373_SCHED_PORT_Q_CTRL_SET_STRICT_EN_OFFSET,RTL8373_QTYPE_WFQ)) != RT_ERR_OK)
                return retVal;
            if ((retVal = rtl8373_setAsicRegBits(RTL8373_SCHED_PORT_Q_CTRL_SET_ADDR(port,qid), RTL8373_SCHED_PORT_Q_CTRL_SET_WEIGHT_MASK,pQweights->weights[qid])) != RT_ERR_OK)
                return retVal;
        }
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_schedulingQueue_get
 * Description:
 *      Get weight and type of queues in dedicated port.
 * Input:
 *      port - Port id.
 * Output:
 *      pQweights - The array of weights for WRR/WFQ queue (0 for STRICT_PRIORITY queue).
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get weight and type, strict priority or weight fair queue (WFQ) for dedicated port for using queues.
 *      The WFQ queue weight is from 1 to 127, and weight 0 is for strict priority queue type.
 */
rtk_api_ret_t dal_rtl8373_qos_schedulingQueue_get(rtk_port_t port, rtk_qos_queue_weights_t *pQweights)
{
    rtk_api_ret_t retVal;
    rtk_uint32 qid,qtype,qweight;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    //phy_port = rtk_switch_port_L2P_get(port);

    for (qid = 0; qid < RTL8373_QUEUENO; qid++)
    {
        if ((retVal = rtl8373_getAsicRegBit(RTL8373_SCHED_PORT_Q_CTRL_SET_ADDR(port,qid), RTL8373_SCHED_PORT_Q_CTRL_SET_STRICT_EN_OFFSET,&qtype)) != RT_ERR_OK)
            return retVal;

        if (RTL8373_QTYPE_STRICT == qtype)
        {
            pQweights->weights[qid] = 0;
        }
        else 
        {
            if ((retVal = rtl8373_getAsicRegBits(RTL8373_SCHED_PORT_Q_CTRL_SET_ADDR(port,qid), RTL8373_SCHED_PORT_Q_CTRL_SET_WEIGHT_MASK, &qweight)) != RT_ERR_OK)
                return retVal;
            pQweights->weights[qid] = qweight;
        }
    }
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_1pRemarkEnable_set
 * Description:
 *      Set 1p Remarking state
 * Input:
 *      port        - Port id.
 *      enable      - State of per-port 1p Remarking
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable parameter.
 * Note:
 *      The API can enable or disable 802.1p remarking ability for whole system.
 *      The status of 802.1p remark:
 *      - DISABLED
 *      - ENABLED
 */
rtk_api_ret_t dal_rtl8373_qos_1pRemarkEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_RMK_PORT_CTRL_ADDR(port), RTL8373_RMK_PORT_CTRL_IPRI_RMK_EN_OFFSET, enable)) != RT_ERR_OK)
            return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_1pRemarkEnable_get
 * Description:
 *      Get 802.1p remarking ability.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - Status of 802.1p remark.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get 802.1p remarking ability.
 *      The status of 802.1p remark:
 *      - DISABLED
 *      - ENABLED
 */
rtk_api_ret_t dal_rtl8373_qos_1pRemarkEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_RMK_PORT_CTRL_ADDR(port), RTL8373_RMK_PORT_CTRL_IPRI_RMK_EN_OFFSET, pEnable)) != RT_ERR_OK)
            return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_1pRemark_set
 * Description:
 *      Set 802.1p remarking parameter.
 * Input:
 *      int_pri     - Internal priority value.
 *      dot1p_pri   - 802.1p priority value.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_VLAN_PRIORITY    - Invalid 1p priority.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      The API can set 802.1p parameters source priority and new priority.
 */
rtk_api_ret_t dal_rtl8373_qos_1pRemark_set(rtk_pri_t int_pri, rtk_pri_t dot1p_pri)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (int_pri > RTL8373_PRIMAX )
        return RT_ERR_QOS_INT_PRIORITY;

    if (dot1p_pri > RTL8373_PRIMAX)
        return RT_ERR_VLAN_PRIORITY;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_RMK_INTPRI2IPRI_CTRL_ADDR,0x7<<(int_pri * 4), dot1p_pri)) != RT_ERR_OK)
             return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_1pRemark_get
 * Description:
 *      Get 802.1p remarking parameter.
 * Input:
 *      int_pri - Internal priority value.
 * Output:
 *      pDot1p_pri - 802.1p priority value.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      The API can get 802.1p remarking parameters. It would return new priority of ingress priority.
 */
rtk_api_ret_t dal_rtl8373_qos_1pRemark_get(rtk_pri_t int_pri, rtk_pri_t *pDot1p_pri)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (int_pri > RTL8373_PRIMAX )
        return RT_ERR_QOS_INT_PRIORITY;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_RMK_INTPRI2IPRI_CTRL_ADDR,0x7<<(int_pri * 4), pDot1p_pri)) != RT_ERR_OK)
             return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_1pRemarkSrcSel_set
 * Description:
 *      Set remarking source of 802.1p remarking.
 * Input:
 *      type      - remarking source
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter

 * Note:
 *      The API can configure 802.1p remark functionality to map original 802.1p value or internal
 *      priority to TX DSCP value.
 */
rtk_api_ret_t dal_rtl8373_qos_1pRemarkSrcSel_set(rtk_qos_1pRmkSrc_t type)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (type >= DOT1P_RMK_SRC_END )
        return RT_ERR_QOS_INT_PRIORITY;

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_RMK_CTRL_ADDR ,RTL8373_RMK_CTRL_IPRI_RMK_SRC_OFFSET, type)) != RT_ERR_OK)
            return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_1pRemarkSrcSel_get
 * Description:
 *      Get remarking source of 802.1p remarking.
 * Input:
 *      none
 * Output:
 *      pType      - remarking source
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer

 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_qos_1pRemarkSrcSel_get(rtk_qos_1pRmkSrc_t *pType)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_RMK_CTRL_ADDR ,RTL8373_RMK_CTRL_IPRI_RMK_SRC_OFFSET, pType)) != RT_ERR_OK)
            return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_dscpRemarkEnable_set
 * Description:
 *      Set DSCP remarking ability.
 * Input:
 *      port    - Port id.
 *      enable  - status of DSCP remark.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 *      RT_ERR_ENABLE           - Invalid enable parameter.
 * Note:
 *      The API can enable or disable DSCP remarking ability for whole system.
 *      The status of DSCP remark:
 *      - DISABLED
 *      - ENABLED
 */
rtk_api_ret_t dal_rtl8373_qos_dscpRemarkEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_RMK_PORT_CTRL_ADDR(port), RTL8373_RMK_PORT_CTRL_DSCP_RMK_EN_OFFSET, enable)) != RT_ERR_OK)
            return retVal;
	
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_dscpRemarkEnable_get
 * Description:
 *      Get DSCP remarking ability.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - status of DSCP remarking.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get DSCP remarking ability.
 *      The status of DSCP remark:
 *      - DISABLED
 *      - ENABLED
 */
rtk_api_ret_t dal_rtl8373_qos_dscpRemarkEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_RMK_PORT_CTRL_ADDR(port), RTL8373_RMK_PORT_CTRL_DSCP_RMK_EN_OFFSET, pEnable)) != RT_ERR_OK)
            return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_intpri2dscp_Remark_set
 * Description:
 *      Set DSCP remarking parameter.
 * Input:
 *      int_pri - Internal priority value.
 *      dscp    - remark DSCP value.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 *      RT_ERR_QOS_DSCP_VALUE   - Invalid DSCP value.
 * Note:
 *      The API can set internal priority to DSCP value.
 */
rtk_api_ret_t dal_rtl8373_qos_intpri2dscp_Remark_set(rtk_pri_t int_pri, rtk_dscp_t dscp)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (int_pri > RTK_PRIMAX )
        return RT_ERR_QOS_INT_PRIORITY;

    if (dscp > RTK_DSCPMAX)
        return RT_ERR_QOS_DSCP_VALUE;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_RMK_INTPRI2DSCP_CTRL_ADDR(int_pri), RTL8373_RMK_INTPRI2DSCP_CTRL_DSCP_MASK(int_pri), dscp)) != RT_ERR_OK)
        return retVal;
	
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_intpri2dscp_Remark_get
 * Description:
 *      Get DSCP remarking parameter.
 * Input:
 *      int_pri - Internal priority value.
 * Output:
 *      pDscp - remark DSCP value.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      The API can get DSCP parameters. It would return DSCP value for mapping priority.
 */
rtk_api_ret_t dal_rtl8373_qos_intpri2dscp_Remark_get(rtk_pri_t int_pri, rtk_dscp_t *pDscp)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (int_pri > RTK_PRIMAX )
        return RT_ERR_QOS_INT_PRIORITY;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_RMK_INTPRI2DSCP_CTRL_ADDR(int_pri), RTL8373_RMK_INTPRI2DSCP_CTRL_DSCP_MASK(int_pri), pDscp)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_dscp2dscp_Remark_set
 * Description:
 *      Set original DSCP remarking parameter.
 * Input:
 *      ori_dscp - original dscp value.
 *      RmkDscp    - remark DSCP value.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 *      RT_ERR_QOS_DSCP_VALUE   - Invalid DSCP value.
 * Note:
 *      The API can set original DSCP value to dscp value.
 */
rtk_api_ret_t dal_rtl8373_qos_dscp2dscp_Remark_set(rtk_pri_t ori_dscp, rtk_dscp_t RmkDscp)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (ori_dscp > RTK_DSCPMAX )
        return RT_ERR_QOS_DSCP_VALUE;

    if (RmkDscp > RTK_DSCPMAX)
        return RT_ERR_QOS_DSCP_VALUE;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_RMK_DSCP2DSCP_CTRL_ADDR(ori_dscp), RTL8373_RMK_DSCP2DSCP_CTRL_DSCP_MASK(ori_dscp), RmkDscp)) != RT_ERR_OK)
        return retVal;
	
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_dscp2dscp_Remark_get
 * Description:
 *      Get DSCP remarking parameter.
 * Input:
 *      int_pri - Internal priority value.
 * Output:
 *      pRmkDscp - remark DSCP value.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      The API can get DSCP parameters. It would return DSCP value for mapping priority.
 */
rtk_api_ret_t dal_rtl8373_qos_dscp2dscp_Remark_get(rtk_pri_t ori_dscp, rtk_dscp_t *pRmkDscp)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (ori_dscp > RTK_DSCPMAX )
        return RT_ERR_QOS_DSCP_VALUE;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_RMK_DSCP2DSCP_CTRL_ADDR(ori_dscp), RTL8373_RMK_DSCP2DSCP_CTRL_DSCP_MASK(ori_dscp), pRmkDscp)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_qos_dscpRemarkSrcSel_set
 * Description:
 *      Set remarking source of DSCP remarking.
 * Input:
 *      type      - remarking source
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter

 * Note:
 *      The API can configure DSCP remark functionality to map original DSCP value or internal
 *      priority to TX DSCP value.
 */
rtk_api_ret_t dal_rtl8373_qos_dscpRemarkSrcSel_set(rtk_qos_dscpRmkSrc_t type)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (type >= DSCP_RMK_SRC_END )
        return RT_ERR_QOS_INT_PRIORITY;

    if (type == DSCP_RMK_SRC_INT_PRI )
        return RT_ERR_QOS_INT_PRIORITY;

    switch (type)
    {
        case DSCP_RMK_SRC_DSCP:
            regData = 1;
            break;
        case DSCP_RMK_SRC_USER_PRI:
            regData = 0;
            break;
        default:
            return RT_ERR_QOS_INT_PRIORITY;
    }

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_RMK_CTRL_ADDR,RTL8373_RMK_CTRL_DSCP_RMK_SRC_OFFSET, regData)) != RT_ERR_OK)
            return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_dscpRemarkSrcSel_get
 * Description:
 *      Get remarking source of DSCP remarking.
 * Input:
 *      none
 * Output:
 *      pType      - remarking source
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer

 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_qos_dscpRemarkSrcSel_get(rtk_qos_dscpRmkSrc_t *pType)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_RMK_CTRL_ADDR,RTL8373_RMK_CTRL_DSCP_RMK_SRC_OFFSET, &regData)) != RT_ERR_OK)
            return retVal;

    switch (regData)
    {
        case 0:
            *pType = DSCP_RMK_SRC_USER_PRI;
            break;
        case 1:
            *pType = DSCP_RMK_SRC_DSCP;
            break;
        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_schedulingType_set
 * Description:
 *      Set scheduling type.
 * Input:
 *      port - port id
 *      type      - scheduling type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_INPUT            - invalid input parameter

 * Note:
 *      The API can configure QoS scheduling type.
 */
rtk_api_ret_t dal_rtl8373_qos_schedulingType_set(rtk_port_t port, rtk_qos_scheduling_type_t type)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (type >= SCHEDULING_TYPE_END )
        return RT_ERR_QOS_SCHE_TYPE;

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_SCHED_PORT_ALGO_CTRL_ADDR(port), RTL8373_SCHED_PORT_ALGO_CTRL_SCHED_TYPE_OFFSET(port), type)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_schedulingType_get
 * Description:
 *      Get type of scheduling.
 * Input:
 *      port - port id
 * Output:
 *      pType      - scheduling type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer

 * Note:
 *      The API can get QoS scheduling type
 */
rtk_api_ret_t dal_rtl8373_qos_schedulingType_get(rtk_port_t port, rtk_qos_scheduling_type_t *pType)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_SCHED_PORT_ALGO_CTRL_ADDR(port), RTL8373_SCHED_PORT_ALGO_CTRL_SCHED_TYPE_OFFSET(port), pType)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_portPriSelIndex_set
 * Description:
 *      Configure priority decision index to each port.
 * Input:
 *      port - Port id.
 *      index - priority decision index.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_ENTRY_INDEX - Invalid entry index.
 * Note:
 *      The API can set priority of port assignments for queue usage and packet scheduling.
 */
rtk_api_ret_t dal_rtl8373_qos_portPriSelIndex_set(rtk_port_t port, rtk_qos_priDecTbl_t index)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (index >= PRIDECTBL_END )
        return RT_ERR_ENTRY_INDEX;

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_PORT_WEIGHT_SEL_ADDR (port), RTL8373_PORT_WEIGHT_SEL_WEIGHT_SEL_OFFSET(port), index)) != RT_ERR_OK)
              return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_qos_portPriSelIndex_get
 * Description:
 *      Get priority decision index from each port.
 * Input:
 *      port - Port id.
 * Output:
 *      pIndex - priority decision index.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get priority of port assignments for queue usage and packet scheduling.
 */
rtk_api_ret_t dal_rtl8373_qos_portPriSelIndex_get(rtk_port_t port, rtk_qos_priDecTbl_t *pIndex)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_PORT_WEIGHT_SEL_ADDR (port), RTL8373_PORT_WEIGHT_SEL_WEIGHT_SEL_OFFSET(port), pIndex)) != RT_ERR_OK)
              return retVal;

    return RT_ERR_OK;
}

