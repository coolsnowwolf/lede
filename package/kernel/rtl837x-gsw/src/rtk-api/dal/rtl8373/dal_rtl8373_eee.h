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
 * Feature : The file includes EEE module high-layer API defination
 *
 */

#ifndef  __DAL_RTL8373_EEE_H__
#define  __DAL_RTL8373_EEE_H__
#include <eee.h>


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

extern rtk_api_ret_t dal_rtl8373_eee_init(void);

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
extern rtk_api_ret_t dal_rtl8373_eee_macForceSpeedEn_set(rtk_port_t port, rtk_eee_speedInMacForceMode_t speed, rtk_enable_t enable);

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
extern rtk_api_ret_t dal_rtl8373_eee_macForceSpeedEn_get(rtk_port_t port, rtk_eee_speedInMacForceMode_t speed, rtk_enable_t *pEnable);

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
extern rtk_api_ret_t dal_rtl8373_eee_macForceAllSpeedEn_get(rtk_port_t port, rtk_uint32 *pState);

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
extern rtk_api_ret_t dal_rtl8373_eee_portTxRxEn_set(rtk_port_t port, rtk_enable_t rxEn, rtk_enable_t txEn);

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
extern rtk_api_ret_t dal_rtl8373_eee_portTxRxEn_get(rtk_port_t port, rtk_enable_t *pRxEn, rtk_enable_t *pTxEn);

#endif /* __DAL_RTL8373_EEE_H__  */
