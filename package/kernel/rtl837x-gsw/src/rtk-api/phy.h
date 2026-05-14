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
 * Feature : The file includes QoS module high-layer API defination
 *
 */

#ifndef __RTK_API_PHY_H__
#define __RTK_API_PHY_H__

#include <port.h>

#define PHY_MMD_PMAPMD              1
#define PHY_MMD_PCS                 3
#define PHY_MMD_AN                  7
#define PHY_MMD_VEND1               30   /* Vendor specific 1 */
#define PHY_MMD_VEND2               31   /* Vendor specific 2 */

#define BIT_0       0x0001
#define BIT_1       0x0002
#define BIT_2       0x0004
#define BIT_3       0x0008
#define BIT_4       0x0010
#define BIT_5       0x0020
#define BIT_6       0x0040
#define BIT_7       0x0080
#define BIT_8       0x0100
#define BIT_9       0x0200
#define BIT_10      0x0400
#define BIT_11      0x0800
#define BIT_12      0x1000
#define BIT_13      0x2000
#define BIT_14      0x4000
#define BIT_15      0x8000
/* PHY Page Definition */
#define PHY_PAGE_0                      (0)
#define PHY_PAGE_1                      (1)
#define PHY_PAGE_8                      (8)
#define PHY_PAGE_31                     (31)

/* PHY register definition */
#define PHY_CONTROL_REG                         0
#define PHY_STATUS_REG                          1
#define PHY_IDENTIFIER_1_REG                    2
#define PHY_IDENTIFIER_2_REG                    3
#define PHY_AN_ADVERTISEMENT_REG                4
#define PHY_AN_LINKPARTNER_REG                  5
#define PHY_1000_BASET_CONTROL_REG              9
#define PHY_1000_BASET_STATUS_REG               10
#define PHY_EXTENDED_STATUS_REG                 15
#define PHY_PAGE_SELECTION_REG                  31

/* Register 0: Control Register */
#define Reset_OFFSET                     (15)
#define Reset_MASK                       (0x1UL<<Reset_OFFSET)
#define Loopback_OFFSET                  (14)
#define Loopback_MASK                    (0x1UL<<Loopback_OFFSET)
#define SpeedSelection0_OFFSET           (13)
#define SpeedSelection0_MASK             (0x1UL<<SpeedSelection0_OFFSET)
#define AutoNegotiationEnable_OFFSET     (12)
#define AutoNegotiationEnable_MASK       (0x1UL<<AutoNegotiationEnable_OFFSET)
#define PowerDown_OFFSET                 (11)
#define PowerDown_MASK                   (0x1UL<<PowerDown_OFFSET)
#define Isolate_OFFSET                   (10)
#define Isolate_MASK                     (0x1UL<<Isolate_OFFSET)
#define RestartAutoNegotiation_OFFSET    (9)
#define RestartAutoNegotiation_MASK      (0x1UL<<RestartAutoNegotiation_OFFSET)
#define DuplexMode_OFFSET                (8)
#define DuplexMode_MASK                  (0x1UL<<DuplexMode_OFFSET)
#define CollisionTest_OFFSET             (7)
#define CollisionTest_MASK               (0x1UL<<CollisionTest_OFFSET)
#define SpeedSelection1_OFFSET           (6)
#define SpeedSelection1_MASK             (0x1UL<<SpeedSelection1_OFFSET)


typedef struct rtk_port_phy_ability_s
{
    rtk_uint16 Half_10:1;
    rtk_uint16 Full_10:1;
    rtk_uint16 Half_100:1;
    rtk_uint16 Full_100:1;
    rtk_uint16 Half_1000:1;
    rtk_uint16 Full_1000:1;
    rtk_uint16 adv_2_5G:1;
    rtk_uint16 adv_5G:1;
    rtk_uint16 adv_10GBase_T:1;
    rtk_uint16 FC:1;
    rtk_uint16 AsyFC:1;
} rtk_port_phy_ability_t;



/* Function Name:
*      rtk_phy_common_c45_an_restart
* Description:
*      Get ability advertisement for auto-negotiation of the specific port
* Input:
*      port - port id
* Output:
*      pAbility - pointer to PHY auto-negotiation abWility
* Return:
*      RT_ERR_OK     - OK
*      RT_ERR_FAILED - invalid parameter
* Note:
*      1000 is vendor specific in C45.
*/
extern rtk_api_ret_t rtk_phy_common_c45_an_restart(rtk_port_t port);

/* Function Name:
 *      rtk_phy_common_c45_autoNegoEnable_get
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
extern rtk_api_ret_t rtk_phy_common_c45_autoNegoEnable_get( rtk_port_t port, rtk_enable_t *pEnable);


/* Function Name:
 *      rtk_phy_common_c45_autoNegoEnable_set
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
extern rtk_api_ret_t rtk_phy_common_c45_autoNegoEnable_set(rtk_port_t port, rtk_enable_t enable);


/* Function Name:
 *      rtk_phy_autoNegoAbility_set
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
extern rtk_api_ret_t rtk_phy_autoNegoAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *      rtk_phy_conmmon_c45_autoSpeed_set
 * Description:
 *      Set phy ability for the specific port
 * Input:
 *      port - port id
 *      pAbility  - phy ability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */

extern rtk_api_ret_t rtk_phy_common_c45_autoSpeed_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *      rtk_phy_common_c45_speed_get
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
rtk_api_ret_t rtk_phy_common_c45_speed_get(rtk_port_t port, rtk_port_speed_t *pSpeed);

/* Function Name:
 *      rtk_phy_common_c45_speed_set
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
rtk_api_ret_t rtk_phy_common_c45_speed_set(rtk_port_t port, rtk_port_speed_t speed);

/* Function Name:
 *      rtk_phy_common_c45_enable_set
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
rtk_api_ret_t rtk_phy_common_c45_enable_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_phy_common_c45_enable_get
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
rtk_api_ret_t rtk_phy_common_c45_enable_get( rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_phy_common_c45_duplex_get
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
rtk_api_ret_t rtk_phy_common_c45_duplex_get(rtk_port_t port, rtk_port_duplex_t *pDuplex);

/* Function Name:
 *      rtk_phy_common_c45_duplex_set
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
rtk_api_ret_t rtk_phy_common_c45_duplex_set(rtk_port_t port, rtk_port_duplex_t duplex);

/* Function Name:
 *      rtk_phy_common_c45_speedDuplexStatusResReg_get
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
rtk_int32 rtk_phy_common_c45_speedDuplexStatusResReg_get(rtk_port_t port, rtk_port_speed_t *pSpeed, rtk_port_duplex_t *pDuplex);

#endif