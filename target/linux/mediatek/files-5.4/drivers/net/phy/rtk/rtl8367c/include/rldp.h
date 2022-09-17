/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 76306 $
 * $Date: 2017-03-08 15:13:58 +0800 (¶g¤T, 08 ¤T¤ë 2017) $
 *
 * Purpose : Declaration of RLDP and RLPP API
 *
 * Feature : The file have include the following module and sub-modules
 *           1) RLDP and RLPP configuration and status
 *
 */


#ifndef __RTK_RLDP_H__
#define __RTK_RLDP_H__


/*
 * Include Files
 */


/*
 * Symbol Definition
 */
typedef enum rtk_rldp_trigger_e
{
    RTK_RLDP_TRIGGER_SAMOVING = 0,
    RTK_RLDP_TRIGGER_PERIOD,
    RTK_RLDP_TRIGGER_END
} rtk_rldp_trigger_t;

typedef enum rtk_rldp_cmpType_e
{
    RTK_RLDP_CMPTYPE_MAGIC = 0,     /* Compare the RLDP with magic only */
    RTK_RLDP_CMPTYPE_MAGIC_ID,      /* Compare the RLDP with both magic + ID */
    RTK_RLDP_CMPTYPE_END
} rtk_rldp_cmpType_t;

typedef enum rtk_rldp_loopStatus_e
{
    RTK_RLDP_LOOPSTS_NONE = 0,
    RTK_RLDP_LOOPSTS_LOOPING,
    RTK_RLDP_LOOPSTS_END
} rtk_rldp_loopStatus_t;

typedef enum rtk_rlpp_trapType_e
{
    RTK_RLPP_TRAPTYPE_NONE = 0,
    RTK_RLPP_TRAPTYPE_CPU,
    RTK_RLPP_TRAPTYPE_END
} rtk_rlpp_trapType_t;

typedef struct rtk_rldp_config_s
{
    rtk_enable_t        rldp_enable;
    rtk_rldp_trigger_t trigger_mode;
    rtk_mac_t           magic;
    rtk_rldp_cmpType_t  compare_type;
    rtk_uint32              interval_check; /* Checking interval for check state */
    rtk_uint32              num_check;      /* Checking number for check state */
    rtk_uint32              interval_loop;  /* Checking interval for loop state */
    rtk_uint32              num_loop;       /* Checking number for loop state */
} rtk_rldp_config_t;

typedef struct rtk_rldp_portConfig_s
{
    rtk_enable_t        tx_enable;
} rtk_rldp_portConfig_t;

typedef struct rtk_rldp_status_s
{
    rtk_mac_t           id;
} rtk_rldp_status_t;

typedef struct rtk_rldp_portStatus_s
{
    rtk_rldp_loopStatus_t   loop_status;
    rtk_rldp_loopStatus_t   loop_enter;
    rtk_rldp_loopStatus_t   loop_leave;
} rtk_rldp_portStatus_t;

/*
 * Data Declaration
 */


/*
 * Macro Declaration
 */

#define RTK_RLDP_INTERVAL_MAX  0xffff
#define RTK_RLDP_NUM_MAX       0xff


/*
 * Function Declaration
 */

/* Module Name : RLDP */


/* Function Name:
 *      rtk_rldp_config_set
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
extern rtk_api_ret_t rtk_rldp_config_set(rtk_rldp_config_t *pConfig);


/* Function Name:
 *      rtk_rldp_config_get
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
extern rtk_api_ret_t rtk_rldp_config_get(rtk_rldp_config_t *pConfig);


/* Function Name:
 *      rtk_rldp_portConfig_set
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
extern rtk_api_ret_t rtk_rldp_portConfig_set(rtk_port_t port, rtk_rldp_portConfig_t *pPortConfig);


/* Function Name:
 *      rtk_rldp_portConfig_get
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
extern rtk_api_ret_t rtk_rldp_portConfig_get(rtk_port_t port, rtk_rldp_portConfig_t *pPortConfig);


/* Function Name:
 *      rtk_rldp_status_get
 * Description:
 *      Get RLDP module status
 * Input:
 *      None
 * Output:
 *      pStatus - status structure of RLDP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
extern rtk_api_ret_t rtk_rldp_status_get(rtk_rldp_status_t *pStatus);


/* Function Name:
 *      rtk_rldp_portStatus_get
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
extern rtk_api_ret_t rtk_rldp_portStatus_get(rtk_port_t port, rtk_rldp_portStatus_t *pPortStatus);


/* Function Name:
 *      rtk_rldp_portStatus_clear
 * Description:
 *      Clear RLDP module status
 * Input:
 *      port    - port number to be clear
 *      pPortStatus - per port status structure of RLDP
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      Clear operation effect loop_enter and loop_leave only, other field in
 *      the structure are don't care
 */
extern rtk_api_ret_t rtk_rldp_portStatus_set(rtk_port_t port, rtk_rldp_portStatus_t *pPortStatus);


/* Function Name:
 *      rtk_rldp_portLoopPair_get
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
extern rtk_api_ret_t rtk_rldp_portLoopPair_get(rtk_port_t port, rtk_portmask_t *pPortmask);

#endif /* __RTK_RLDP_H__ */

