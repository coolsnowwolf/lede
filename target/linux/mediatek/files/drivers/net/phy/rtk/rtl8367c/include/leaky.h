/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * Purpose : RTL8367/RTL8367C switch high-level API
 *
 * Feature : The file includes Leaky module high-layer API definition
 *
 */

#ifndef __RTK_API_LEAKY_H__
#define __RTK_API_LEAKY_H__


typedef enum rtk_leaky_type_e
{
    LEAKY_BRG_GROUP = 0,
    LEAKY_FD_PAUSE,
    LEAKY_SP_MCAST,
    LEAKY_1X_PAE,
    LEAKY_UNDEF_BRG_04,
    LEAKY_UNDEF_BRG_05,
    LEAKY_UNDEF_BRG_06,
    LEAKY_UNDEF_BRG_07,
    LEAKY_PROVIDER_BRIDGE_GROUP_ADDRESS,
    LEAKY_UNDEF_BRG_09,
    LEAKY_UNDEF_BRG_0A,
    LEAKY_UNDEF_BRG_0B,
    LEAKY_UNDEF_BRG_0C,
    LEAKY_PROVIDER_BRIDGE_GVRP_ADDRESS,
    LEAKY_8021AB,
    LEAKY_UNDEF_BRG_0F,
    LEAKY_BRG_MNGEMENT,
    LEAKY_UNDEFINED_11,
    LEAKY_UNDEFINED_12,
    LEAKY_UNDEFINED_13,
    LEAKY_UNDEFINED_14,
    LEAKY_UNDEFINED_15,
    LEAKY_UNDEFINED_16,
    LEAKY_UNDEFINED_17,
    LEAKY_UNDEFINED_18,
    LEAKY_UNDEFINED_19,
    LEAKY_UNDEFINED_1A,
    LEAKY_UNDEFINED_1B,
    LEAKY_UNDEFINED_1C,
    LEAKY_UNDEFINED_1D,
    LEAKY_UNDEFINED_1E,
    LEAKY_UNDEFINED_1F,
    LEAKY_GMRP,
    LEAKY_GVRP,
    LEAKY_UNDEF_GARP_22,
    LEAKY_UNDEF_GARP_23,
    LEAKY_UNDEF_GARP_24,
    LEAKY_UNDEF_GARP_25,
    LEAKY_UNDEF_GARP_26,
    LEAKY_UNDEF_GARP_27,
    LEAKY_UNDEF_GARP_28,
    LEAKY_UNDEF_GARP_29,
    LEAKY_UNDEF_GARP_2A,
    LEAKY_UNDEF_GARP_2B,
    LEAKY_UNDEF_GARP_2C,
    LEAKY_UNDEF_GARP_2D,
    LEAKY_UNDEF_GARP_2E,
    LEAKY_UNDEF_GARP_2F,
    LEAKY_IGMP,
    LEAKY_IPMULTICAST,
    LEAKY_CDP,
    LEAKY_CSSTP,
    LEAKY_LLDP,
    LEAKY_END,
}rtk_leaky_type_t;

/* Function Name:
 *      rtk_leaky_vlan_set
 * Description:
 *      Set VLAN leaky.
 * Input:
 *      type - Packet type for VLAN leaky.
 *      enable - Leaky status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_ENABLE       - Invalid enable input
 * Note:
 *      This API can set VLAN leaky for RMA ,IGMP/MLD, CDP, CSSTP, and LLDP packets.
 *      The leaky frame types are as following:
 *      - LEAKY_BRG_GROUP,
 *      - LEAKY_FD_PAUSE,
 *      - LEAKY_SP_MCAST,
 *      - LEAKY_1X_PAE,
 *      - LEAKY_UNDEF_BRG_04,
 *      - LEAKY_UNDEF_BRG_05,
 *      - LEAKY_UNDEF_BRG_06,
 *      - LEAKY_UNDEF_BRG_07,
 *      - LEAKY_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - LEAKY_UNDEF_BRG_09,
 *      - LEAKY_UNDEF_BRG_0A,
 *      - LEAKY_UNDEF_BRG_0B,
 *      - LEAKY_UNDEF_BRG_0C,
 *      - LEAKY_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - LEAKY_8021AB,
 *      - LEAKY_UNDEF_BRG_0F,
 *      - LEAKY_BRG_MNGEMENT,
 *      - LEAKY_UNDEFINED_11,
 *      - LEAKY_UNDEFINED_12,
 *      - LEAKY_UNDEFINED_13,
 *      - LEAKY_UNDEFINED_14,
 *      - LEAKY_UNDEFINED_15,
 *      - LEAKY_UNDEFINED_16,
 *      - LEAKY_UNDEFINED_17,
 *      - LEAKY_UNDEFINED_18,
 *      - LEAKY_UNDEFINED_19,
 *      - LEAKY_UNDEFINED_1A,
 *      - LEAKY_UNDEFINED_1B,
 *      - LEAKY_UNDEFINED_1C,
 *      - LEAKY_UNDEFINED_1D,
 *      - LEAKY_UNDEFINED_1E,
 *      - LEAKY_UNDEFINED_1F,
 *      - LEAKY_GMRP,
 *      - LEAKY_GVRP,
 *      - LEAKY_UNDEF_GARP_22,
 *      - LEAKY_UNDEF_GARP_23,
 *      - LEAKY_UNDEF_GARP_24,
 *      - LEAKY_UNDEF_GARP_25,
 *      - LEAKY_UNDEF_GARP_26,
 *      - LEAKY_UNDEF_GARP_27,
 *      - LEAKY_UNDEF_GARP_28,
 *      - LEAKY_UNDEF_GARP_29,
 *      - LEAKY_UNDEF_GARP_2A,
 *      - LEAKY_UNDEF_GARP_2B,
 *      - LEAKY_UNDEF_GARP_2C,
 *      - LEAKY_UNDEF_GARP_2D,
 *      - LEAKY_UNDEF_GARP_2E,
 *      - LEAKY_UNDEF_GARP_2F,
 *      - LEAKY_IGMP,
 *      - LEAKY_IPMULTICAST.
 *      - LEAKY_CDP,
 *      - LEAKY_CSSTP,
 *      - LEAKY_LLDP.
 */
extern rtk_api_ret_t rtk_leaky_vlan_set(rtk_leaky_type_t type, rtk_enable_t enable);

/* Function Name:
 *      rtk_leaky_vlan_get
 * Description:
 *      Get VLAN leaky.
 * Input:
 *      type - Packet type for VLAN leaky.
 * Output:
 *      pEnable - Leaky status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API can get VLAN leaky status for RMA ,IGMP/MLD, CDP, CSSTP, and LLDP  packets.
 *      The leaky frame types are as following:
 *      - LEAKY_BRG_GROUP,
 *      - LEAKY_FD_PAUSE,
 *      - LEAKY_SP_MCAST,
 *      - LEAKY_1X_PAE,
 *      - LEAKY_UNDEF_BRG_04,
 *      - LEAKY_UNDEF_BRG_05,
 *      - LEAKY_UNDEF_BRG_06,
 *      - LEAKY_UNDEF_BRG_07,
 *      - LEAKY_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - LEAKY_UNDEF_BRG_09,
 *      - LEAKY_UNDEF_BRG_0A,
 *      - LEAKY_UNDEF_BRG_0B,
 *      - LEAKY_UNDEF_BRG_0C,
 *      - LEAKY_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - LEAKY_8021AB,
 *      - LEAKY_UNDEF_BRG_0F,
 *      - LEAKY_BRG_MNGEMENT,
 *      - LEAKY_UNDEFINED_11,
 *      - LEAKY_UNDEFINED_12,
 *      - LEAKY_UNDEFINED_13,
 *      - LEAKY_UNDEFINED_14,
 *      - LEAKY_UNDEFINED_15,
 *      - LEAKY_UNDEFINED_16,
 *      - LEAKY_UNDEFINED_17,
 *      - LEAKY_UNDEFINED_18,
 *      - LEAKY_UNDEFINED_19,
 *      - LEAKY_UNDEFINED_1A,
 *      - LEAKY_UNDEFINED_1B,
 *      - LEAKY_UNDEFINED_1C,
 *      - LEAKY_UNDEFINED_1D,
 *      - LEAKY_UNDEFINED_1E,
 *      - LEAKY_UNDEFINED_1F,
 *      - LEAKY_GMRP,
 *      - LEAKY_GVRP,
 *      - LEAKY_UNDEF_GARP_22,
 *      - LEAKY_UNDEF_GARP_23,
 *      - LEAKY_UNDEF_GARP_24,
 *      - LEAKY_UNDEF_GARP_25,
 *      - LEAKY_UNDEF_GARP_26,
 *      - LEAKY_UNDEF_GARP_27,
 *      - LEAKY_UNDEF_GARP_28,
 *      - LEAKY_UNDEF_GARP_29,
 *      - LEAKY_UNDEF_GARP_2A,
 *      - LEAKY_UNDEF_GARP_2B,
 *      - LEAKY_UNDEF_GARP_2C,
 *      - LEAKY_UNDEF_GARP_2D,
 *      - LEAKY_UNDEF_GARP_2E,
 *      - LEAKY_UNDEF_GARP_2F,
 *      - LEAKY_IGMP,
 *      - LEAKY_IPMULTICAST.
 *      - LEAKY_CDP,
 *      - LEAKY_CSSTP,
 *      - LEAKY_LLDP.
 */
extern rtk_api_ret_t rtk_leaky_vlan_get(rtk_leaky_type_t type, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_leaky_portIsolation_set
 * Description:
 *      Set port isolation leaky.
 * Input:
 *      type - Packet type for port isolation leaky.
 *      enable - Leaky status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_ENABLE       - Invalid enable input
 * Note:
 *      This API can set port isolation leaky for RMA ,IGMP/MLD, CDP, CSSTP, and LLDP  packets.
 *      The leaky frame types are as following:
 *      - LEAKY_BRG_GROUP,
 *      - LEAKY_FD_PAUSE,
 *      - LEAKY_SP_MCAST,
 *      - LEAKY_1X_PAE,
 *      - LEAKY_UNDEF_BRG_04,
 *      - LEAKY_UNDEF_BRG_05,
 *      - LEAKY_UNDEF_BRG_06,
 *      - LEAKY_UNDEF_BRG_07,
 *      - LEAKY_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - LEAKY_UNDEF_BRG_09,
 *      - LEAKY_UNDEF_BRG_0A,
 *      - LEAKY_UNDEF_BRG_0B,
 *      - LEAKY_UNDEF_BRG_0C,
 *      - LEAKY_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - LEAKY_8021AB,
 *      - LEAKY_UNDEF_BRG_0F,
 *      - LEAKY_BRG_MNGEMENT,
 *      - LEAKY_UNDEFINED_11,
 *      - LEAKY_UNDEFINED_12,
 *      - LEAKY_UNDEFINED_13,
 *      - LEAKY_UNDEFINED_14,
 *      - LEAKY_UNDEFINED_15,
 *      - LEAKY_UNDEFINED_16,
 *      - LEAKY_UNDEFINED_17,
 *      - LEAKY_UNDEFINED_18,
 *      - LEAKY_UNDEFINED_19,
 *      - LEAKY_UNDEFINED_1A,
 *      - LEAKY_UNDEFINED_1B,
 *      - LEAKY_UNDEFINED_1C,
 *      - LEAKY_UNDEFINED_1D,
 *      - LEAKY_UNDEFINED_1E,
 *      - LEAKY_UNDEFINED_1F,
 *      - LEAKY_GMRP,
 *      - LEAKY_GVRP,
 *      - LEAKY_UNDEF_GARP_22,
 *      - LEAKY_UNDEF_GARP_23,
 *      - LEAKY_UNDEF_GARP_24,
 *      - LEAKY_UNDEF_GARP_25,
 *      - LEAKY_UNDEF_GARP_26,
 *      - LEAKY_UNDEF_GARP_27,
 *      - LEAKY_UNDEF_GARP_28,
 *      - LEAKY_UNDEF_GARP_29,
 *      - LEAKY_UNDEF_GARP_2A,
 *      - LEAKY_UNDEF_GARP_2B,
 *      - LEAKY_UNDEF_GARP_2C,
 *      - LEAKY_UNDEF_GARP_2D,
 *      - LEAKY_UNDEF_GARP_2E,
 *      - LEAKY_UNDEF_GARP_2F,
 *      - LEAKY_IGMP,
 *      - LEAKY_IPMULTICAST.
 *      - LEAKY_CDP,
 *      - LEAKY_CSSTP,
 *      - LEAKY_LLDP.
 */
extern rtk_api_ret_t rtk_leaky_portIsolation_set(rtk_leaky_type_t type, rtk_enable_t enable);

/* Function Name:
 *      rtk_leaky_portIsolation_get
 * Description:
 *      Get port isolation leaky.
 * Input:
 *      type - Packet type for port isolation leaky.
 * Output:
 *      pEnable - Leaky status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API can get port isolation leaky status for RMA ,IGMP/MLD, CDP, CSSTP, and LLDP  packets.
 *      The leaky frame types are as following:
 *      - LEAKY_BRG_GROUP,
 *      - LEAKY_FD_PAUSE,
 *      - LEAKY_SP_MCAST,
 *      - LEAKY_1X_PAE,
 *      - LEAKY_UNDEF_BRG_04,
 *      - LEAKY_UNDEF_BRG_05,
 *      - LEAKY_UNDEF_BRG_06,
 *      - LEAKY_UNDEF_BRG_07,
 *      - LEAKY_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - LEAKY_UNDEF_BRG_09,
 *      - LEAKY_UNDEF_BRG_0A,
 *      - LEAKY_UNDEF_BRG_0B,
 *      - LEAKY_UNDEF_BRG_0C,
 *      - LEAKY_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - LEAKY_8021AB,
 *      - LEAKY_UNDEF_BRG_0F,
 *      - LEAKY_BRG_MNGEMENT,
 *      - LEAKY_UNDEFINED_11,
 *      - LEAKY_UNDEFINED_12,
 *      - LEAKY_UNDEFINED_13,
 *      - LEAKY_UNDEFINED_14,
 *      - LEAKY_UNDEFINED_15,
 *      - LEAKY_UNDEFINED_16,
 *      - LEAKY_UNDEFINED_17,
 *      - LEAKY_UNDEFINED_18,
 *      - LEAKY_UNDEFINED_19,
 *      - LEAKY_UNDEFINED_1A,
 *      - LEAKY_UNDEFINED_1B,
 *      - LEAKY_UNDEFINED_1C,
 *      - LEAKY_UNDEFINED_1D,
 *      - LEAKY_UNDEFINED_1E,
 *      - LEAKY_UNDEFINED_1F,
 *      - LEAKY_GMRP,
 *      - LEAKY_GVRP,
 *      - LEAKY_UNDEF_GARP_22,
 *      - LEAKY_UNDEF_GARP_23,
 *      - LEAKY_UNDEF_GARP_24,
 *      - LEAKY_UNDEF_GARP_25,
 *      - LEAKY_UNDEF_GARP_26,
 *      - LEAKY_UNDEF_GARP_27,
 *      - LEAKY_UNDEF_GARP_28,
 *      - LEAKY_UNDEF_GARP_29,
 *      - LEAKY_UNDEF_GARP_2A,
 *      - LEAKY_UNDEF_GARP_2B,
 *      - LEAKY_UNDEF_GARP_2C,
 *      - LEAKY_UNDEF_GARP_2D,
 *      - LEAKY_UNDEF_GARP_2E,
 *      - LEAKY_UNDEF_GARP_2F,
 *      - LEAKY_IGMP,
 *      - LEAKY_IPMULTICAST.
 *      - LEAKY_CDP,
 *      - LEAKY_CSSTP,
 *      - LEAKY_LLDP.
 */
extern rtk_api_ret_t rtk_leaky_portIsolation_get(rtk_leaky_type_t type, rtk_enable_t *pEnable);

#endif /* __RTK_API_LEAKY_H__ */

