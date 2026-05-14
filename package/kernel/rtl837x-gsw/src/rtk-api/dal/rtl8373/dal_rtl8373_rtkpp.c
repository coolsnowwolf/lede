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
 * Purpose : Declaration of RLDP and RLPP API
 *
 * Feature : The file have include the following module and sub-modules
 *           1) RLDP and RLPP configuration and status
 *
 */


/*
 * Include Files
 */
#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal_rtl8373_rtkpp.h>
#include <rtl8373_asicdrv.h>


/* Function Name:
 *      dal_rtl8373_rldp_config_set
 * Description:
 *      Set RLDP module configuration
 * Input:
 *      pConfig - configuration structure of RLDP
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_rldp_config_set(rtk_rldp_config_t *pConfig)
{
    rtk_api_ret_t retVal;
    rtk_uint32 *magic;
	rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (pConfig->rldp_enable >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if (pConfig->trigger_mode >= RTK_RLDP_TRIGGER_END)
        return RT_ERR_INPUT;

    if (pConfig->compare_type >= RTK_RLDP_CMPTYPE_END)
        return RT_ERR_INPUT;

    if (pConfig->num_check >= RTK_RLDP_NUM_MAX)
        return RT_ERR_INPUT;

    if (pConfig->interval_check >= RTK_RLDP_INTERVAL_MAX)
        return RT_ERR_INPUT;

    if (pConfig->num_loop >= RTK_RLDP_NUM_MAX)
        return RT_ERR_INPUT;

    if (pConfig->interval_loop >= RTK_RLDP_INTERVAL_MAX)
        return RT_ERR_INPUT;


    if ((retVal = rtl8373_setAsicRegBit(RTL8373_RLDP_RLPP_CTRL_ADDR, RTL8373_RLDP_RLPP_CTRL_RLDP_EN_OFFSET, pConfig->rldp_enable))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_RLDP_RLPP_CTRL_ADDR, RTL8373_RLDP_RLPP_CTRL_RLDP_MODE_OFFSET, pConfig->trigger_mode))!=RT_ERR_OK)
        return retVal;


    magic = (rtk_uint32*)&pConfig->magic;
    regData = *magic;
    if ((retVal = rtl8373_setAsicReg(RTL8373_MAGIC_NUM0_ADDR, regData))!=RT_ERR_OK)
            return retVal;

    magic++;
    regData = *magic & 0xffff;
    if ((retVal = rtl8373_setAsicReg(RTL8373_MAGIC_NUM1_ADDR, regData))!=RT_ERR_OK)
            return retVal;


    if ((retVal = rtl8373_setAsicRegBit(RTL8373_RLDP_RLPP_CTRL_ADDR, RTL8373_RLDP_RLPP_CTRL_COMP_ID_OFFSET, pConfig->compare_type))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_RETRY_CTRL_ADDR, RTL8373_RETRY_CTRL_RETRY_CHK_MASK, pConfig->num_check))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_PERIOD_CTRL_ADDR, RTL8373_PERIOD_CTRL_PERIOD_CHK_MASK, pConfig->interval_check))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_RETRY_CTRL_ADDR, RTL8373_RETRY_CTRL_RETRY_LOOP_MASK, pConfig->num_loop))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_PERIOD_CTRL_ADDR, RTL8373_PERIOD_CTRL_PERIOD_LOOP_MASK, pConfig->interval_loop))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_rldp_config_get
 * Description:
 *      Get RLDP module configuration
 * Input:
 *      None
 * Output:
 *      pConfig - configuration structure of RLDP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_rldp_config_get(rtk_rldp_config_t *pConfig)
{
    rtk_api_ret_t retVal;
    rtk_uint32 *magic;
	rtk_uint32 regData;	

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_RLDP_RLPP_CTRL_ADDR, RTL8373_RLDP_RLPP_CTRL_RLDP_EN_OFFSET, &pConfig->rldp_enable))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_RLDP_RLPP_CTRL_ADDR, RTL8373_RLDP_RLPP_CTRL_RLDP_MODE_OFFSET, &pConfig->trigger_mode))!=RT_ERR_OK)
        return retVal;

    magic = (rtk_uint32*)&pConfig->magic;
    retVal = rtl8373_getAsicReg(RTL8373_MAGIC_NUM0_ADDR, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *magic = regData;
    magic++;

    retVal = rtl8373_getAsicReg(RTL8373_MAGIC_NUM1_ADDR, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *magic = regData;

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_RLDP_RLPP_CTRL_ADDR, RTL8373_RLDP_RLPP_CTRL_COMP_ID_OFFSET, &pConfig->compare_type))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_RETRY_CTRL_ADDR, RTL8373_RETRY_CTRL_RETRY_CHK_MASK, &pConfig->num_check))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_PERIOD_CTRL_ADDR, RTL8373_PERIOD_CTRL_PERIOD_CHK_MASK, &pConfig->interval_check))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_RETRY_CTRL_ADDR, RTL8373_RETRY_CTRL_RETRY_LOOP_MASK, &pConfig->num_loop))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_PERIOD_CTRL_ADDR, RTL8373_PERIOD_CTRL_PERIOD_LOOP_MASK, &pConfig->interval_loop))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_rldp_portConfig_set
 * Description:
 *      Set per port RLDP module configuration
 * Input:
 *      port   - port number to be configured
 *      pPortConfig - per port configuration structure of RLDP
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_rldp_portConfig_set(rtk_port_t port, rtk_rldp_portConfig_t *pPortConfig)
{
    rtk_api_ret_t retVal;
    rtk_uint32 portmask;
    rtk_uint32 phy_port;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (pPortConfig->tx_enable>= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    phy_port = rtk_switch_port_L2P_get(port);

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_RLDP_TX_PMSK_ADDR, RTL8373_RLDP_TX_PMSK_PMSK_MASK, &portmask))!=RT_ERR_OK)
        return retVal;

    if (pPortConfig->tx_enable)
    {
         portmask |=(1<<phy_port);
    }
    else
    {
         portmask &= ~(1<<phy_port);
    }

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_RLDP_TX_PMSK_ADDR, RTL8373_RLDP_TX_PMSK_PMSK_MASK, portmask))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;

}


/* Function Name:
 *      dal_rtl8373_rldp_portConfig_get
 * Description:
 *      Get per port RLDP module configuration
 * Input:
 *      port    - port number to be get
 * Output:
 *      pPortConfig - per port configuration structure of RLDP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_rldp_portConfig_get(rtk_port_t port, rtk_rldp_portConfig_t *pPortConfig)
{
    rtk_api_ret_t retVal;
    rtk_uint32 portmask;
    rtk_portmask_t logicalPmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_RLDP_TX_PMSK_ADDR, RTL8373_RLDP_TX_PMSK_PMSK_MASK, &portmask))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtk_switch_portmask_P2L_get(portmask, &logicalPmask)) != RT_ERR_OK)
        return retVal;


    if (logicalPmask.bits[0] & (1<<port))
    {
         pPortConfig->tx_enable = ENABLED;
    }
    else
    {
         pPortConfig->tx_enable = DISABLED;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_rldp_randomNum_get
 * Description:
 *      Get RLDP random number
 * Output:
 *      pRandom    - the pointer to random number
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_rldp_randomNum_get(rtk_mac_t * pRandom)
{
    rtk_api_ret_t retVal;
    rtk_uint32 * tmp;
    rtk_uint32 regData;

    
    tmp = (rtk_uint32*)pRandom;
    retVal = rtl8373_getAsicReg(RTL8373_RAND_NUM0_ADDR, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;
    
    *tmp = regData;
    tmp++;
    
    retVal = rtl8373_getAsicReg(RTL8373_RAND_NUM1_ADDR, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;
    
    *tmp = regData;

    return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8373_rldp_portStatus_get
 * Description:
 *      Get RLDP module status
 * Input:
 *      port    - port number to be get
 * Output:
 *      pPortStatus - per port status structure of RLDP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_rldp_portStatus_get(rtk_port_t port, rtk_rldp_portStatus_t *pPortStatus)
{
    rtk_api_ret_t retVal;
    rtk_uint32 status;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_LOOP_STATE_ADDR(port), RTL8373_LOOP_STATE_LOOP_PMSK_OFFSET(port), &status))!=RT_ERR_OK)
        return retVal;

    if (status & 1)
    {
         pPortStatus->loop_status = RTK_RLDP_LOOPSTS_LOOPING;
    }
    else
    {
         pPortStatus->loop_status  = RTK_RLDP_LOOPSTS_NONE;
    }
	
    if ((retVal = rtl8373_getAsicRegBit(RTL8373_LOOPED_STATE_ADDR(port), RTL8373_LOOPED_STATE_LOOPED_PMSK_OFFSET(port), &status))!=RT_ERR_OK)
        return retVal;

    if (status & 1)
    {
         pPortStatus->loop_enter = RTK_RLDP_LOOPSTS_LOOPING;
    }
    else
    {
         pPortStatus->loop_enter  = RTK_RLDP_LOOPSTS_NONE;
    }

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_LEAVE_LOOP_STATE_ADDR(port), RTL8373_LEAVE_LOOP_STATE_LEAVE_LOOP_PMSK_OFFSET(port), &status))!=RT_ERR_OK)
        return retVal;

    if (status & 1)
    {
         pPortStatus->loop_leave = RTK_RLDP_LOOPSTS_LOOPING;
    }
    else
    {
         pPortStatus->loop_leave  = RTK_RLDP_LOOPSTS_NONE;
    }

    return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8373_rldp_portLoopPair_get
 * Description:
 *      Get RLDP port loop pairs
 * Input:
 *      port    - port number to be get
 * Output:
 *      pPortmask - per port related loop ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_rldp_portLoopPair_get(rtk_port_t port, rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmsk;
    //rtk_uint32 phy_port;
	rtk_uint32 loopedPair;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

	//phy_port = rtk_switch_port_L2P_get(port);

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_LOOPPAIR_ADDR(port), RTL8373_LOOPPAIR_LOOP_PAIR_MASK(port), &loopedPair))!=RT_ERR_OK)
        return retVal;

    if(loopedPair == 0)
        pmsk = 0;
    else
        pmsk = 1 << (loopedPair - 1);
    if ((retVal = rtk_switch_portmask_P2L_get(pmsk, pPortmask)) != RT_ERR_OK)
        return retVal;
	
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_rldp_genRandom
 * Description:
 *      set asic gen random number
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_rldp_genRandom(void)
{
    rtk_api_ret_t retVal;



    if ((retVal = rtl8373_setAsicRegBit(RTL8373_RLDP_RLPP_CTRL_ADDR, RTL8373_RLDP_RLPP_CTRL_GEN_RANDOM_OFFSET, 1))!=RT_ERR_OK)
        return retVal;

    
    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_rlpp_config_set
 * Description:
 *      Set trap RLPP pkt to 8051
 * Input:
 *      enable - enable trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_rlpp_trap_set(rtk_uint32 enable)
{
    rtk_api_ret_t retVal;


    if ((retVal = rtl8373_setAsicRegBit(RTL8373_RLDP_RLPP_CTRL_ADDR, RTL8373_RLDP_RLPP_CTRL_RLPP_TRAP_OFFSET, enable))!=RT_ERR_OK)
        return retVal;

    
    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_rlpp_config_get
 * Description:
 *      Get trap RLPP pkt to 8051
 * Input:
 *      pEnable - enable trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_rlpp_trap_get(rtk_uint32* pEnable)
{
    rtk_api_ret_t retVal;


    if ((retVal = rtl8373_getAsicRegBit(RTL8373_RLDP_RLPP_CTRL_ADDR, RTL8373_RLDP_RLPP_CTRL_RLPP_TRAP_OFFSET, pEnable))!=RT_ERR_OK)
        return retVal;

    
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rrcp_config_set
 * Description:
 *      Set trap RRCP status
 * Input:
 *      status - 0b00:fwd, 0b01:trap to 8051, 0b10: trap external cpu
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_rrcp_trap_set(rtk_uint32 status)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_RRCP_CTRL_ADDR, RTL8373_RRCP_CTRL_RRCP_TRAP_MASK, status))!=RT_ERR_OK)
        return retVal;

    
    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_rrcp_config_get
 * Description:
 *      Get trap RRCP status
 * Input:
 *      pStatus - 0b00:fwd, 0b01:trap to 8051, 0b10: trap external cpu
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_rrcp_trap_get(rtk_uint32* pStatus)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_RRCP_CTRL_ADDR, RTL8373_RRCP_CTRL_RRCP_TRAP_MASK, pStatus))!=RT_ERR_OK)
        return retVal;

    
    return RT_ERR_OK;
}






