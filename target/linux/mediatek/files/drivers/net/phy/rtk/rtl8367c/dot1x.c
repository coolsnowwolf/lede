/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 75783 $
 * $Date: 2017-02-13 14:54:53 +0800 (週一, 13 二月 2017) $
 *
 * Purpose : RTK switch high-level API for RTL8367/RTL8367C
 * Feature : Here is a list of all functions and variables in 1X module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dot1x.h>
#include <string.h>
#include <vlan.h>
#include <rtl8367c_asicdrv.h>
#include <rtl8367c_asicdrv_dot1x.h>
#include <rtl8367c_asicdrv_rma.h>
#include <rtl8367c_asicdrv_lut.h>
#include <rtl8367c_asicdrv_vlan.h>

/* Function Name:
 *      rtk_dot1x_unauthPacketOper_set
 * Description:
 *      Set 802.1x unauth action configuration.
 * Input:
 *      port            - Port id.
 *      unauth_action   - 802.1X unauth action.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_INPUT        - Invalid input parameter.
 * Note:
 *      This API can set 802.1x unauth action configuration.
 *      The unauth action is as following:
 *      - DOT1X_ACTION_DROP
 *      - DOT1X_ACTION_TRAP2CPU
 *      - DOT1X_ACTION_GUESTVLAN
 */
rtk_api_ret_t rtk_dot1x_unauthPacketOper_set(rtk_port_t port, rtk_dot1x_unauth_action_t unauth_action)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port Valid */
    RTK_CHK_PORT_VALID(port);

    if (unauth_action >= DOT1X_ACTION_END)
        return RT_ERR_DOT1X_PROC;

    if ((retVal = rtl8367c_setAsic1xProcConfig(rtk_switch_port_L2P_get(port), unauth_action)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_unauthPacketOper_get
 * Description:
 *      Get 802.1x unauth action configuration.
 * Input:
 *      port - Port id.
 * Output:
 *      pUnauth_action - 802.1X unauth action.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can get 802.1x unauth action configuration.
 *      The unauth action is as following:
 *      - DOT1X_ACTION_DROP
 *      - DOT1X_ACTION_TRAP2CPU
 *      - DOT1X_ACTION_GUESTVLAN
 */
rtk_api_ret_t rtk_dot1x_unauthPacketOper_get(rtk_port_t port, rtk_dot1x_unauth_action_t *pUnauth_action)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pUnauth_action)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsic1xProcConfig(rtk_switch_port_L2P_get(port), pUnauth_action)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_eapolFrame2CpuEnable_set
 * Description:
 *      Set 802.1x EAPOL packet trap to CPU configuration
 * Input:
 *      enable - The status of 802.1x EAPOL packet.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      To support 802.1x authentication functionality, EAPOL frame (ether type = 0x888E) has to
 *      be trapped to CPU.
 *      The status of EAPOL frame trap to CPU is as following:
 *      - DISABLED
 *      - ENABLED
 */
rtk_api_ret_t rtk_dot1x_eapolFrame2CpuEnable_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    rtl8367c_rma_t rmacfg;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8367c_getAsicRma(3, &rmacfg)) != RT_ERR_OK)
        return retVal;

    if (ENABLED == enable)
        rmacfg.operation = RMAOP_TRAP_TO_CPU;
    else if (DISABLED == enable)
        rmacfg.operation = RMAOP_FORWARD;

    if ((retVal = rtl8367c_setAsicRma(3, &rmacfg)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_eapolFrame2CpuEnable_get
 * Description:
 *      Get 802.1x EAPOL packet trap to CPU configuration
 * Input:
 *      None
 * Output:
 *      pEnable - The status of 802.1x EAPOL packet.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      To support 802.1x authentication functionality, EAPOL frame (ether type = 0x888E) has to
 *      be trapped to CPU.
 *      The status of EAPOL frame trap to CPU is as following:
 *      - DISABLED
 *      - ENABLED
 */
rtk_api_ret_t rtk_dot1x_eapolFrame2CpuEnable_get(rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    rtl8367c_rma_t rmacfg;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicRma(3, &rmacfg)) != RT_ERR_OK)
        return retVal;

    if (RMAOP_TRAP_TO_CPU == rmacfg.operation)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_portBasedEnable_set
 * Description:
 *      Set 802.1x port-based enable configuration
 * Input:
 *      port - Port id.
 *      enable - The status of 802.1x port.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_ENABLE               - Invalid enable input.
 *      RT_ERR_DOT1X_PORTBASEDPNEN  - 802.1X port-based enable error
 * Note:
 *      The API can update the port-based port enable register content. If a port is 802.1x
 *      port based network access control "enabled", it should be authenticated so packets
 *      from that port won't be dropped or trapped to CPU.
 *      The status of 802.1x port-based network access control is as following:
 *      - DISABLED
 *      - ENABLED
 */
rtk_api_ret_t rtk_dot1x_portBasedEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port Valid */
    RTK_CHK_PORT_VALID(port);

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8367c_setAsic1xPBEnConfig(rtk_switch_port_L2P_get(port),enable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_portBasedEnable_get
 * Description:
 *      Get 802.1x port-based enable configuration
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - The status of 802.1x port.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get the 802.1x port-based port status.
 */
rtk_api_ret_t rtk_dot1x_portBasedEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsic1xPBEnConfig(rtk_switch_port_L2P_get(port), pEnable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_portBasedAuthStatus_set
 * Description:
 *      Set 802.1x port-based auth. port configuration
 * Input:
 *      port - Port id.
 *      port_auth - The status of 802.1x port.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *     RT_ERR_DOT1X_PORTBASEDAUTH   - 802.1X port-based auth error
 * Note:
 *      The authenticated status of 802.1x port-based network access control is as following:
 *      - UNAUTH
 *      - AUTH
 */
rtk_api_ret_t rtk_dot1x_portBasedAuthStatus_set(rtk_port_t port, rtk_dot1x_auth_status_t port_auth)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port Valid */
    RTK_CHK_PORT_VALID(port);

     if (port_auth >= AUTH_STATUS_END)
        return RT_ERR_DOT1X_PORTBASEDAUTH;

    if ((retVal = rtl8367c_setAsic1xPBAuthConfig(rtk_switch_port_L2P_get(port), port_auth)) != RT_ERR_OK)
        return retVal;


    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_portBasedAuthStatus_get
 * Description:
 *      Get 802.1x port-based auth. port configuration
 * Input:
 *      port - Port id.
 * Output:
 *      pPort_auth - The status of 802.1x port.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get 802.1x port-based port auth.information.
 */
rtk_api_ret_t rtk_dot1x_portBasedAuthStatus_get(rtk_port_t port, rtk_dot1x_auth_status_t *pPort_auth)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pPort_auth)
        return RT_ERR_NULL_POINTER;

    /* Check port Valid */
    RTK_CHK_PORT_VALID(port);

    if ((retVal = rtl8367c_getAsic1xPBAuthConfig(rtk_switch_port_L2P_get(port), pPort_auth)) != RT_ERR_OK)
        return retVal;
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_portBasedDirection_set
 * Description:
 *      Set 802.1x port-based operational direction configuration
 * Input:
 *      port            - Port id.
 *      port_direction  - Operation direction
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_DOT1X_PORTBASEDOPDIR - 802.1X port-based operation direction error
 * Note:
 *      The operate controlled direction of 802.1x port-based network access control is as following:
 *      - BOTH
 *      - IN
 */
rtk_api_ret_t rtk_dot1x_portBasedDirection_set(rtk_port_t port, rtk_dot1x_direction_t port_direction)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port Valid */
    RTK_CHK_PORT_VALID(port);

    if (port_direction >= DIRECTION_END)
        return RT_ERR_DOT1X_PORTBASEDOPDIR;

    if ((retVal = rtl8367c_setAsic1xPBOpdirConfig(rtk_switch_port_L2P_get(port), port_direction)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_portBasedDirection_get
 * Description:
 *      Get 802.1X port-based operational direction configuration
 * Input:
 *      port - Port id.
 * Output:
 *      pPort_direction - Operation direction
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get 802.1x port-based operational direction information.
 */
rtk_api_ret_t rtk_dot1x_portBasedDirection_get(rtk_port_t port, rtk_dot1x_direction_t *pPort_direction)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pPort_direction)
        return RT_ERR_NULL_POINTER;

    /* Check port Valid */
    RTK_CHK_PORT_VALID(port);

    if ((retVal = rtl8367c_getAsic1xPBOpdirConfig(rtk_switch_port_L2P_get(port), pPort_direction)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_macBasedEnable_set
 * Description:
 *      Set 802.1x mac-based port enable configuration
 * Input:
 *      port - Port id.
 *      enable - The status of 802.1x port.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_ENABLE               - Invalid enable input.
 *      RT_ERR_DOT1X_MACBASEDPNEN   - 802.1X mac-based enable error
 * Note:
 *      If a port is 802.1x MAC based network access control "enabled", the incoming packets should
 *       be authenticated so packets from that port won't be dropped or trapped to CPU.
 *      The status of 802.1x MAC-based network access control is as following:
 *      - DISABLED
 *      - ENABLED
 */
rtk_api_ret_t rtk_dot1x_macBasedEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port Valid */
    RTK_CHK_PORT_VALID(port);

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8367c_setAsic1xMBEnConfig(rtk_switch_port_L2P_get(port),enable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_macBasedEnable_get
 * Description:
 *      Get 802.1x mac-based port enable configuration
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - The status of 802.1x port.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      If a port is 802.1x MAC based network access control "enabled", the incoming packets should
 *      be authenticated so packets from that port wont be dropped or trapped to CPU.
 *      The status of 802.1x MAC-based network access control is as following:
 *      - DISABLED
 *      - ENABLED
 */
rtk_api_ret_t rtk_dot1x_macBasedEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    /* Check port Valid */
    RTK_CHK_PORT_VALID(port);

    if ((retVal = rtl8367c_getAsic1xMBEnConfig(rtk_switch_port_L2P_get(port), pEnable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_macBasedAuthMac_add
 * Description:
 *      Add an authenticated MAC to ASIC
 * Input:
 *      port        - Port id.
 *      pAuth_mac   - The authenticated MAC.
 *      fid         - filtering database.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_ENABLE               - Invalid enable input.
 *      RT_ERR_DOT1X_MACBASEDPNEN   - 802.1X mac-based enable error
 * Note:
 *      The API can add a 802.1x authenticated MAC address to port. If the MAC does not exist in LUT,
 *      user can't add this MAC to auth status.
 */
rtk_api_ret_t rtk_dot1x_macBasedAuthMac_add(rtk_port_t port, rtk_mac_t *pAuth_mac, rtk_fid_t fid)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8367c_luttb l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* must be unicast address */
    if ((pAuth_mac == NULL) || (pAuth_mac->octet[0] & 0x1))
        return RT_ERR_MAC;

    /* Check port Valid */
    RTK_CHK_PORT_VALID(port);

    if (fid > RTL8367C_FIDMAX)
        return RT_ERR_L2_FID;

    memset(&l2Table, 0, sizeof(rtl8367c_luttb));

    /* fill key (MAC,FID) to get L2 entry */
    memcpy(l2Table.mac.octet, pAuth_mac->octet, ETHER_ADDR_LEN);
    l2Table.fid = fid;
    method = LUTREADMETHOD_MAC;
    retVal = rtl8367c_getAsicL2LookupTb(method, &l2Table);
    if ( RT_ERR_OK == retVal)
    {
        if (l2Table.spa != rtk_switch_port_L2P_get(port))
            return RT_ERR_DOT1X_MAC_PORT_MISMATCH;

        memcpy(l2Table.mac.octet, pAuth_mac->octet, ETHER_ADDR_LEN);
        l2Table.fid = fid;
        l2Table.efid = 0;
        l2Table.auth = 1;
        retVal = rtl8367c_setAsicL2LookupTb(&l2Table);
        return retVal;
    }
    else
        return retVal;

}

/* Function Name:
 *      rtk_dot1x_macBasedAuthMac_del
 * Description:
 *      Delete an authenticated MAC to ASIC
 * Input:
 *      port - Port id.
 *      pAuth_mac - The authenticated MAC.
 *      fid - filtering database.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_MAC          - Invalid MAC address.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can delete a 802.1x authenticated MAC address to port. It only change the auth status of
 *      the MAC and won't delete it from LUT.
 */
rtk_api_ret_t rtk_dot1x_macBasedAuthMac_del(rtk_port_t port, rtk_mac_t *pAuth_mac, rtk_fid_t fid)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8367c_luttb l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* must be unicast address */
    if ((pAuth_mac == NULL) || (pAuth_mac->octet[0] & 0x1))
        return RT_ERR_MAC;

    /* Check port Valid */
    RTK_CHK_PORT_VALID(port);

    if (fid > RTL8367C_FIDMAX)
        return RT_ERR_L2_FID;

    memset(&l2Table, 0, sizeof(rtl8367c_luttb));

    /* fill key (MAC,FID) to get L2 entry */
    memcpy(l2Table.mac.octet, pAuth_mac->octet, ETHER_ADDR_LEN);
    l2Table.fid = fid;
    method = LUTREADMETHOD_MAC;
    retVal = rtl8367c_getAsicL2LookupTb(method, &l2Table);
    if (RT_ERR_OK == retVal)
    {
        if (l2Table.spa != rtk_switch_port_L2P_get(port))
            return RT_ERR_DOT1X_MAC_PORT_MISMATCH;

        memcpy(l2Table.mac.octet, pAuth_mac->octet, ETHER_ADDR_LEN);
        l2Table.fid = fid;
        l2Table.auth = 0;
        retVal = rtl8367c_setAsicL2LookupTb(&l2Table);
        return retVal;
    }
    else
        return retVal;

}

/* Function Name:
 *      rtk_dot1x_macBasedDirection_set
 * Description:
 *      Set 802.1x mac-based operational direction configuration
 * Input:
 *      mac_direction - Operation direction
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_INPUT                - Invalid input parameter.
 *      RT_ERR_DOT1X_MACBASEDOPDIR  - 802.1X mac-based operation direction error
 * Note:
 *      The operate controlled direction of 802.1x mac-based network access control is as following:
 *      - BOTH
 *      - IN
 */
rtk_api_ret_t rtk_dot1x_macBasedDirection_set(rtk_dot1x_direction_t mac_direction)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (mac_direction >= DIRECTION_END)
        return RT_ERR_DOT1X_MACBASEDOPDIR;

    if ((retVal = rtl8367c_setAsic1xMBOpdirConfig(mac_direction)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_macBasedDirection_get
 * Description:
 *      Get 802.1x mac-based operational direction configuration
 * Input:
 *      port - Port id.
 * Output:
 *      pMac_direction - Operation direction
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can get 802.1x mac-based operational direction information.
 */
rtk_api_ret_t rtk_dot1x_macBasedDirection_get(rtk_dot1x_direction_t *pMac_direction)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pMac_direction)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsic1xMBOpdirConfig(pMac_direction)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      Set 802.1x guest VLAN configuration
 * Description:
 *      Set 802.1x mac-based operational direction configuration
 * Input:
 *      vid - 802.1x guest VLAN ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 * Note:
 *      The operate controlled 802.1x guest VLAN
 */
rtk_api_ret_t rtk_dot1x_guestVlan_set(rtk_vlan_t vid)
{
    rtk_api_ret_t retVal;
    rtk_uint32 index;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* vid must be 0~4095 */
    if (vid > RTL8367C_VIDMAX)
        return RT_ERR_VLAN_VID;

    if((retVal = rtk_vlan_checkAndCreateMbr(vid, &index)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsic1xGuestVidx(index)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_guestVlan_get
 * Description:
 *      Get 802.1x guest VLAN configuration
 * Input:
 *      None
 * Output:
 *      pVid - 802.1x guest VLAN ID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can get 802.1x guest VLAN information.
 */
rtk_api_ret_t rtk_dot1x_guestVlan_get(rtk_vlan_t *pVid)
{
    rtk_api_ret_t retVal;
    rtk_uint32 gvidx;
    rtl8367c_vlanconfiguser vlanMC;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pVid)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsic1xGuestVidx(&gvidx)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_getAsicVlanMemberConfig(gvidx, &vlanMC)) != RT_ERR_OK)
        return retVal;

    *pVid = vlanMC.evid;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_guestVlan2Auth_set
 * Description:
 *      Set 802.1x guest VLAN to auth host configuration
 * Input:
 *      enable - The status of guest VLAN to auth host.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 * Note:
 *      The operational direction of 802.1x guest VLAN to auth host control is as following:
 *      - ENABLED
 *      - DISABLED
 */
rtk_api_ret_t rtk_dot1x_guestVlan2Auth_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8367c_setAsic1xGVOpdir(enable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_guestVlan2Auth_get
 * Description:
 *      Get 802.1x guest VLAN to auth host configuration
 * Input:
 *      None
 * Output:
 *      pEnable - The status of guest VLAN to auth host.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can get 802.1x guest VLAN to auth host information.
 */
rtk_api_ret_t rtk_dot1x_guestVlan2Auth_get(rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsic1xGVOpdir(pEnable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


