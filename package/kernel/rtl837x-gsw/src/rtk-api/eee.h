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
 * Feature : The file includes I2C module high-layer API defination
 *
 */

#ifndef __RTK_API_EEE_H__
#define __RTK_API_EEE_H__

#define EEE_GROUP_NUM            (5)
#define EEE_MEM_ADDR_WIDTH_MAX   (3)
#define EEEE_DATA_WIDTH_MAX		 (16)


typedef enum rtk_eee_speedInMacForceMode_e 
{
    EEE_MAC_FORCE_SPEED_100M = 0,
    EEE_MAC_FORCE_SPEED_500M,
    EEE_MAC_FORCE_SPEED_1000M,
    EEE_MAC_FORCE_SPEED_2P5G_LITE,
    EEE_MAC_FORCE_SPEED_2P5G,
    EEE_MAC_FORCE_SPEED_5G_LITE,
    EEE_MAC_FORCE_SPEED_5G,
    EEE_MAC_FORCE_SPEED_10G_LITE,
    EEE_MAC_FORCE_SPEED_10G,
    EEE_MAC_FORCE_SPEED_END
}rtk_eee_speedInMacForceMode_t;

/* Function Name:
 *      rtk_eee_init
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

extern rtk_api_ret_t rtk_eee_init(void);

/* Function Name:
 *      rtk_eee_macForceSpeedEn_set
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
extern rtk_api_ret_t rtk_eee_macForceSpeedEn_set(rtk_port_t port, rtk_eee_speedInMacForceMode_t speed, rtk_enable_t enable);

/* Function Name:
 *      rtk_eee_macForceSpeedEn_get
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
extern rtk_api_ret_t rtk_eee_macForceSpeedEn_get(rtk_port_t port, rtk_eee_speedInMacForceMode_t speed, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_eee_macForceAllSpeedEn_get
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
extern rtk_api_ret_t rtk_eee_macForceAllSpeedEn_get(rtk_port_t port, rtk_uint32 *pState);

/* Function Name:
 *      rtk_eee_portTxRxEn_set
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
extern rtk_api_ret_t rtk_eee_portTxRxEn_set(rtk_port_t port, rtk_enable_t rxEn, rtk_enable_t txEn);

/* Function Name:
 *      rtk_eee_portTxRxEn_set
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
extern rtk_api_ret_t rtk_eee_portTxRxEn_get(rtk_port_t port, rtk_enable_t *pRxEn, rtk_enable_t *pTxEn);

#endif /* __RTK_API_EEE_H__ */

