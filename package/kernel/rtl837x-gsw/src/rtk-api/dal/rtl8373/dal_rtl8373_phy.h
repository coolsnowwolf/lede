#ifndef __DAL_RTL8373_PHY_H__
#define __DAL_RTL8373_PHY_H__
#include <rtk_switch.h>
#include <rtk_types.h>
#include <phy.h>


/* Function Name:
*      dal_rlt8373_phy_common_c45_an_restart
* Description:
*      Get ability advertisement for auto-negotiation of the specific port
* Input:
*      port - port id
* Output:
*      pAbility - pointer to PHY auto-negotiation ability
* Return:
*      RT_ERR_OK     - OK
*      RT_ERR_FAILED - invalid parameter
* Note:
*      1000 is vendor specific in C45.
*/
extern rtk_api_ret_t dal_rlt8373_phy_common_c45_an_restart( rtk_port_t port);


/* Function Name:
 *      dal_rlt8373_phy_common_c45_autoNegoEnable_get
 * Description:
 *      Get the auto-negotiation state of the specific port
 * Input:
 *      port - port id
 * Output:
 *      pEnable - pointer to output the auto-negotiation state
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern rtk_api_ret_t dal_rlt8373_phy_common_c45_autoNegoEnable_get( rtk_port_t port, rtk_enable_t *pEnable);


/* Function Name:
 *      dal_rlt8373_phy_common_c45_autoNegoEnable_set
 * Description:
 *      Set the auto-negotiation state of the specific port and restart auto-negotiation for enabled state.
 * Input:
 *      port - port id
 *      enable - auto-negotiation state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern rtk_api_ret_t dal_rlt8373_phy_common_c45_autoNegoEnable_set(rtk_port_t port, rtk_enable_t enable);


/* Function Name:
 *      dal_rlt8373_phy_autoNegoAbility_set
 * Description:
 *      Set ability advertisement for auto-negotiation of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 *      pAbility  - auto-negotiation ability that is going to set to PHY
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern rtk_api_ret_t dal_rlt8373_phy_autoNegoAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility);



/* Function Name:
 *      dal_rlt8373_phy_conmmon_c45_autoSpeed_set
 * Description:
 *      Set ability advertisement for auto-negotiation of the specific port
 * Input:
 *      port - port id
 *      pAbility  - auto-negotiation ability that is going to set to PHY
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */

extern rtk_api_ret_t dal_rlt8373_phy_common_c45_autoSpeed_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *      dal_rlt8373_phy_common_c45_speed_get
 * Description:
 *      Get speed of the specific port
 * Input:
 *      port - port id
 * Output:
 *      pSpeed - pointer to PHY link speed
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
rtk_api_ret_t dal_rlt8373_phy_common_c45_speed_get(rtk_port_t port, rtk_port_speed_t *pSpeed);

/* Function Name:
 *      dal_rlt8373_phy_common_c45_speed_set
 * Description:
 *      Set speed of the specific port
 * Input:
 *      port          - port id
 *      speed         - link speed rtk_port_speed_t
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - invalid parameter
 * Note:
 *      None
 */
rtk_api_ret_t dal_rlt8373_phy_common_c45_speed_set(rtk_port_t port, rtk_port_speed_t speed);

/* Function Name:
 *      dal_rlt8373_phy_common_c45_enable_set
 * Description:
 *      Set PHY interface state enable/disable of the specific port
 * Input:
 *      port          - port id
 *      enable        - admin configuration of PHY interface
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
rtk_api_ret_t dal_rlt8373_phy_common_c45_enable_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_rlt8373_phy_common_c45_enable_get
 * Description:
 *      Get PHY interface state enable/disable of the specific port
 * Input:
 *      port          - port id
 * Output:
 *      pEnable       - pointer to admin configuration of PHY interface
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
rtk_api_ret_t dal_rlt8373_phy_common_c45_enable_get( rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_rlt8373_phy_common_c45_duplex_get
 * Description:
 *      Get duplex of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pDuplex - pointer to PHY duplex mode status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
rtk_api_ret_t dal_rlt8373_phy_common_c45_duplex_get(rtk_port_t port, rtk_port_duplex_t *pDuplex);

/* Function Name:
 *      dal_rlt8373_phy_common_c45_duplex_set
 * Description:
 *      Set duplex of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      duplex        - duplex mode of the port, full or half
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
rtk_api_ret_t dal_rlt8373_phy_common_c45_duplex_set(rtk_port_t port, rtk_port_duplex_t duplex);

/* Function Name:
 *      dal_rlt8373_phy_common_c45_speedDuplexStatusResReg_get
 * Description:
 *      Get operational link speed-duplex status from proprietary register
 * Input:
 *      port    - port id
 * Output:
 *      pSpeed - pointer to PHY operational link speed
 *      pDuplex - pointer to PHY operational Duplex
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      Only PHY that supports proprietary resolution register can use this driver!
 */
rtk_int32 dal_rlt8373_phy_common_c45_speedDuplexStatusResReg_get(rtk_port_t port, rtk_port_speed_t *pSpeed, rtk_port_duplex_t *pDuplex);

extern rtk_api_ret_t phy_conmmon_c45_autoSpeed_set_test(rtk_uint32 port, rtk_uint32 speed);
extern void rtk_speedtest_1g(void);
#endif
