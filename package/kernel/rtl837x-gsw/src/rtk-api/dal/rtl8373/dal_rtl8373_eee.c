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
 * Feature : Here is a list of all functions and variables in EEE module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal/rtl8373/dal_rtl8373_eee.h>
#include <string.h>
#include <dal/rtl8373/rtl8373_asicdrv.h>

/* Function Name:
 *      dal_rtl8373_eee_init
 * Description:
 *      Initial EEE function.
 * Input:
 *      None
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can set EEE function to the specific port.
 *      The configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */

rtk_api_ret_t dal_rtl8373_eee_init(void)
{
	return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_eee_macForceSpeedEn_set
 * Description:
 *      Set enable status of EEE for a Specified Speed,
 * Input:
 *      port - port id.
 *		speed	- a specified
 *      enable  - enable EEE status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can set EEE function to the specific port.
 *      The configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtk_api_ret_t dal_rtl8373_eee_macForceSpeedEn_set(rtk_port_t port, rtk_eee_speedInMacForceMode_t speed, rtk_enable_t enable)
{
    rtk_uint32 retVal = 0;

    /* Check port valid */
    RTK_CHK_PORT_VALID(port); 

	if((speed >= EEE_MAC_FORCE_SPEED_END) || (enable >=RTK_ENABLE_END))
		return RT_ERR_RANGE;

	if((retVal = rtl8373_setAsicRegBit(RTL8373_MAC_FORCE_MODE_CTRL1_ADDR(port), speed,  (rtk_uint32)enable)) != RT_ERR_OK)
        return retVal;  

    return RT_ERR_OK;	
}

/* Function Name:
 *      dal_rtl8373_eee_macForceSpeedEn_get
 * Description:
 *      Get port_n status of EEE for a Specified Speed,
 * Input:
 *      port - port id.
 *		speed	- a specified
 * Output:
 *      pEnable - EEE status.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can get EEE function to the specific port.
 *      The configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtk_api_ret_t dal_rtl8373_eee_macForceSpeedEn_get(rtk_port_t port, rtk_eee_speedInMacForceMode_t speed, rtk_enable_t *pEnable)
{
    rtk_uint32 regVal = 0, retVal = 0;

    /* Check port valid */
    RTK_CHK_PORT_VALID(port); 

	if(speed >= EEE_MAC_FORCE_SPEED_END)
		return RT_ERR_RANGE;
	if(pEnable == NULL)
		return RT_ERR_NULL_POINTER;

	if((retVal = rtl8373_getAsicRegBit(RTL8373_MAC_FORCE_MODE_CTRL1_ADDR(port), speed, &regVal)) != RT_ERR_OK)
        return retVal;  

	*pEnable = (rtk_enable_t)regVal;

    return RT_ERR_OK;	
}

/* Function Name:
 *      dal_rtl8373_eee_macForceAllSpeedEn_get
 * Description:
 *      Get port_n status of EEE for all Speed.
 *      port - port id.
 * Output:
 *      pState - EEE status.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can get EEE function to the specific port.
 *      The configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtk_api_ret_t dal_rtl8373_eee_macForceAllSpeedEn_get(rtk_port_t port, rtk_uint32 *pState)
{
    rtk_uint32 retVal = 0;

    /* Check port valid */
    RTK_CHK_PORT_VALID(port); 

	if(pState == NULL)
		return RT_ERR_NULL_POINTER;

	if((retVal = rtl8373_getAsicRegBits(RTL8373_MAC_FORCE_MODE_CTRL1_ADDR(port), 0x1FF, pState)) != RT_ERR_OK)
        return retVal;  

    return RT_ERR_OK;	
}

/* Function Name:
 *      dal_rtl8373_eee_portTxRxEn_set
 * Description:
 *      Set port_n Tx & Rx EEE capability.
 * Input:
 *      port - port id.
 *      rxEn - Enable or Disable Rx EEE capability.
 *      txEn - Enable or Disable Tx EEE capability.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can config tx EEE and rx EEE capability to the specific port.
 *      The configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtk_api_ret_t dal_rtl8373_eee_portTxRxEn_set(rtk_port_t port, rtk_enable_t rxEn, rtk_enable_t txEn)
{
    rtk_uint32 retVal = 0;

    /* Check port valid */
    RTK_CHK_PORT_VALID(port); 

	if((rxEn >=RTK_ENABLE_END) || (txEn >=RTK_ENABLE_END))
		return RT_ERR_RANGE;

	if((retVal = rtl8373_setAsicRegBit(RTL8373_EEE_CTRL_ADDR(port), RTL8373_EEE_CTRL_EEE_PORT_RX_EN_OFFSET, (rtk_uint32)rxEn)) != RT_ERR_OK)
        return retVal;  
	if((retVal = rtl8373_setAsicRegBit(RTL8373_EEE_CTRL_ADDR(port), RTL8373_EEE_CTRL_EEE_PORT_TX_EN_OFFSET, (rtk_uint32)txEn)) != RT_ERR_OK)
        return retVal;  

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_eee_portTxRxEn_set
 * Description:
 *      Set port_n Tx & Rx EEE capability.
 * Input:
 *      port - port id.
 *      rxEn - Enable or Disable Rx EEE capability.
 *      txEn - Enable or Disable Tx EEE capability.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can config tx EEE and rx EEE capability to the specific port.
 *      The configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtk_api_ret_t dal_rtl8373_eee_portTxRxEn_get(rtk_port_t port, rtk_enable_t *pRxEn, rtk_enable_t *pTxEn)
{
    rtk_uint32 regVal = 0, retVal = 0;

    /* Check port valid */
    RTK_CHK_PORT_VALID(port); 

	if((pRxEn == NULL) || (pTxEn == NULL))
		return RT_ERR_NULL_POINTER;

	if((retVal = rtl8373_getAsicRegBit(RTL8373_EEE_CTRL_ADDR(port), RTL8373_EEE_CTRL_EEE_PORT_RX_EN_OFFSET, &regVal)) != RT_ERR_OK)
        return retVal;  
	*pRxEn = (rtk_enable_t)regVal;


	if((retVal = rtl8373_getAsicRegBit(RTL8373_EEE_CTRL_ADDR(port),  RTL8373_EEE_CTRL_EEE_PORT_TX_EN_OFFSET, &regVal)) != RT_ERR_OK)
        return retVal;  
	*pTxEn = (rtk_enable_t)regVal;

    return RT_ERR_OK;
}







