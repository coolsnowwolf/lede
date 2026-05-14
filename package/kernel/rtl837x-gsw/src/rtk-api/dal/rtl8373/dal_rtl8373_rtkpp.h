#ifndef __DAL_RTL8373_RTKPP_H__
#define __DAL_RTL8373_RTKPP_H__


#include "rldp.h"



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
extern rtk_api_ret_t dal_rtl8373_rldp_config_set(rtk_rldp_config_t *pConfig);

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
extern rtk_api_ret_t dal_rtl8373_rldp_config_get(rtk_rldp_config_t *pConfig);



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
extern rtk_api_ret_t dal_rtl8373_rldp_portConfig_set(rtk_port_t port, rtk_rldp_portConfig_t *pPortConfig);

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
extern rtk_api_ret_t dal_rtl8373_rldp_portConfig_get(rtk_port_t port, rtk_rldp_portConfig_t *pPortConfig);

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
extern rtk_api_ret_t dal_rtl8373_rldp_portStatus_get(rtk_port_t port, rtk_rldp_portStatus_t *pPortStatus);


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
extern rtk_api_ret_t dal_rtl8373_rldp_portLoopPair_get(rtk_port_t port, rtk_portmask_t *pPortmask);

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
extern rtk_api_ret_t dal_rtl8373_rldp_genRandom(void);


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
extern rtk_api_ret_t dal_rtl8373_rlpp_trap_set(rtk_uint32 enable);

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
extern rtk_api_ret_t dal_rtl8373_rlpp_trap_get(rtk_uint32* pEnable);

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
extern rtk_api_ret_t dal_rtl8373_rrcp_trap_set(rtk_uint32 status);


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
extern rtk_api_ret_t dal_rtl8373_rrcp_trap_get(rtk_uint32* pStatus);


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
extern rtk_api_ret_t dal_rtl8373_rldp_randomNum_get(rtk_mac_t * pRandom);


#endif