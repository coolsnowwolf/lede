/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 39583 $
 * $Date: 2013-05-20 16:59:23 +0800 (星期一, 20 五月 2013) $
 *
 * Purpose : RTK switch high-level API for RTL8367/RTL8367C
 * Feature : Here is a list of all functions and variables in time module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <ptp.h>
#include <string.h>

#include <rtl8367c_asicdrv.h>
#include <rtl8367c_asicdrv_eav.h>

/* Function Name:
 *      rtk_ptp_init
 * Description:
 *      PTP function initialization.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API is used to initialize PTP status.
 */
rtk_api_ret_t rtk_ptp_init(void)
{
    /* Check initialization state */
    RTK_CHK_INIT_STATE();


    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_ptp_mac_set
 * Description:
 *      Configure PTP mac address.
 * Input:
 *      mac - mac address to parser PTP packets.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 * Note:
 *      None
 */
rtk_api_ret_t rtk_ptp_mac_set(rtk_mac_t mac)
{
    rtk_api_ret_t retVal;
    ether_addr_t sw_mac;

    memcpy(sw_mac.octet, mac.octet, ETHER_ADDR_LEN);

    if((retVal=rtl8367c_setAsicEavMacAddress(sw_mac))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_ptp_mac_get
 * Description:
 *      Get PTP mac address.
 * Input:
 *      None
 * Output:
 *      pMac - mac address to parser PTP packets.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 * Note:
 *      None
 */
rtk_api_ret_t rtk_ptp_mac_get(rtk_mac_t *pMac)
{
    rtk_api_ret_t retVal;
    ether_addr_t sw_mac;

    if((retVal=rtl8367c_getAsicEavMacAddress(&sw_mac))!=RT_ERR_OK)
        return retVal;

    memcpy(pMac->octet, sw_mac.octet, ETHER_ADDR_LEN);

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_ptp_tpid_set
 * Description:
 *      Configure PTP accepted outer & inner tag TPID.
 * Input:
 *      outerId - Ether type of S-tag frame parsing in PTP ports.
 *      innerId - Ether type of C-tag frame parsing in PTP ports.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 * Note:
 *      None
 */
rtk_api_ret_t rtk_ptp_tpid_set(rtk_ptp_tpid_t outerId, rtk_ptp_tpid_t innerId)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ((outerId>RTK_MAX_NUM_OF_TPID) ||(innerId>RTK_MAX_NUM_OF_TPID))
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicEavTpid(outerId, innerId)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_ptp_tpid_get
 * Description:
 *      Get PTP accepted outer & inner tag TPID.
 * Input:
 *      None
 * Output:
 *      pOuterId - Ether type of S-tag frame parsing in PTP ports.
 *      pInnerId - Ether type of C-tag frame parsing in PTP ports.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */
rtk_api_ret_t rtk_ptp_tpid_get(rtk_ptp_tpid_t *pOuterId, rtk_ptp_tpid_t *pInnerId)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ((retVal = rtl8367c_getAsicEavTpid(pOuterId, pInnerId)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_ptp_refTime_set
 * Description:
 *      Set the reference time of the specified device.
 * Input:
 *      timeStamp - reference timestamp value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
rtk_api_ret_t rtk_ptp_refTime_set(rtk_ptp_timeStamp_t timeStamp)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (timeStamp.nsec > RTK_MAX_NUM_OF_NANO_SECOND)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicEavSysTime(timeStamp.sec, timeStamp.nsec))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_ptp_refTime_get
 * Description:
 *      Get the reference time of the specified device.
 * Input:
 * Output:
 *      pTimeStamp - pointer buffer of the reference time
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
rtk_api_ret_t rtk_ptp_refTime_get(rtk_ptp_timeStamp_t *pTimeStamp)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ((retVal = rtl8367c_getAsicEavSysTime(&pTimeStamp->sec, &pTimeStamp->nsec))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_ptp_refTimeAdjust_set
 * Description:
 *      Adjust the reference time.
 * Input:
 *      unit      - unit id
 *      sign      - significant
 *      timeStamp - reference timestamp value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      sign=0 for positive adjustment, sign=1 for negative adjustment.
 */
rtk_api_ret_t rtk_ptp_refTimeAdjust_set(rtk_ptp_sys_adjust_t sign, rtk_ptp_timeStamp_t timeStamp)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (timeStamp.nsec > RTK_MAX_NUM_OF_NANO_SECOND)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicEavSysTimeAdjust(sign, timeStamp.sec, timeStamp.nsec))!=RT_ERR_OK)
        return retVal;


    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_ptp_refTimeEnable_set
 * Description:
 *      Set the enable state of reference time of the specified device.
 * Input:
 *      enable - status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
rtk_api_ret_t rtk_ptp_refTimeEnable_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8367c_setAsicEavSysTimeCtrl(enable))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_ptp_refTimeEnable_get
 * Description:
 *      Get the enable state of reference time of the specified device.
 * Input:
 * Output:
 *      pEnable - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
rtk_api_ret_t rtk_ptp_refTimeEnable_get(rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ((retVal = rtl8367c_getAsicEavSysTimeCtrl(pEnable))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_ptp_portEnable_set
 * Description:
 *      Set PTP status of the specified port.
 * Input:
 *      port   - port id
 *      enable - status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT     - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
rtk_api_ret_t rtk_ptp_portEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port is PTP port */
    RTK_CHK_PORT_IS_PTP(port);

    if (enable>=RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicEavPortEnable(rtk_switch_port_L2P_get(port), enable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_ptp_portEnable_get
 * Description:
 *      Get PTP status of the specified port.
 * Input:
 *      port    - port id
 * Output:
 *      pEnable - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT         - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
rtk_api_ret_t rtk_ptp_portEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port is PTP port */
    RTK_CHK_PORT_IS_PTP(port);

    if ((retVal = rtl8367c_getAsicEavPortEnable(rtk_switch_port_L2P_get(port), pEnable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_ptp_portTimestamp_get
 * Description:
 *      Get PTP timstamp according to the PTP identifier on the dedicated port from the specified device.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      type       - PTP message type
 * Output:
 *      pInfo      - pointer buffer of sequence ID and timestamp
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
rtk_api_ret_t rtk_ptp_portTimestamp_get( rtk_port_t port, rtk_ptp_msgType_t type, rtk_ptp_info_t *pInfo)
{
    rtk_api_ret_t retVal;
    rtl8367c_ptp_time_stamp_t time;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port is PTP port */
    RTK_CHK_PORT_IS_PTP(port);

    if ((retVal = rtl8367c_getAsicEavPortTimeStamp(rtk_switch_port_L2P_get(port), type, &time)) != RT_ERR_OK)
        return retVal;

    pInfo->sequenceId = time.sequence_id;
    pInfo->timeStamp.sec = time.second;
    pInfo->timeStamp.nsec = time.nano_second;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_ptp_intControl_set
 * Description:
 *      Set PTP interrupt trigger status configuration.
 * Input:
 *      type - Interrupt type.
 *      enable - Interrupt status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      The API can set PTP interrupt status configuration.
 *      The interrupt trigger status is shown in the following:
 *          PTP_INT_TYPE_TX_SYNC = 0,
 *          PTP_INT_TYPE_TX_DELAY_REQ,
 *          PTP_INT_TYPE_TX_PDELAY_REQ,
 *          PTP_INT_TYPE_TX_PDELAY_RESP,
 *          PTP_INT_TYPE_RX_SYNC,
 *          PTP_INT_TYPE_RX_DELAY_REQ,
 *          PTP_INT_TYPE_RX_PDELAY_REQ,
 *          PTP_INT_TYPE_RX_PDELAY_RESP,
 *          PTP_INT_TYPE_ALL,
 */
rtk_api_ret_t rtk_ptp_intControl_set(rtk_ptp_intType_t type, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 mask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (type>=PTP_INT_TYPE_END)
        return RT_ERR_INPUT;

    if (PTP_INT_TYPE_ALL!=type)
    {
        if ((retVal = rtl8367c_getAsicEavInterruptMask(&mask)) != RT_ERR_OK)
            return retVal;

        if (ENABLED == enable)
            mask = mask | (1<<type);
        else if (DISABLED == enable)
            mask = mask & ~(1<<type);
        else
            return RT_ERR_INPUT;

        if ((retVal = rtl8367c_setAsicEavInterruptMask(mask)) != RT_ERR_OK)
            return retVal;
    }
    else
    {
        if (ENABLED == enable)
            mask = RTK_PTP_INTR_MASK;
        else if (DISABLED == enable)
            mask = 0;
        else
            return RT_ERR_INPUT;

        if ((retVal = rtl8367c_setAsicEavInterruptMask(mask)) != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_ptp_intControl_get
 * Description:
 *      Get PTP interrupt trigger status configuration.
 * Input:
 *      type - Interrupt type.
 * Output:
 *      pEnable - Interrupt status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can get interrupt status configuration.
 *      The interrupt trigger status is shown in the following:
 *          PTP_INT_TYPE_TX_SYNC = 0,
 *          PTP_INT_TYPE_TX_DELAY_REQ,
 *          PTP_INT_TYPE_TX_PDELAY_REQ,
 *          PTP_INT_TYPE_TX_PDELAY_RESP,
 *          PTP_INT_TYPE_RX_SYNC,
 *          PTP_INT_TYPE_RX_DELAY_REQ,
 *          PTP_INT_TYPE_RX_PDELAY_REQ,
 *          PTP_INT_TYPE_RX_PDELAY_RESP,
 */
rtk_api_ret_t rtk_ptp_intControl_get(rtk_ptp_intType_t type, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 mask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (type>=PTP_INT_TYPE_ALL)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_getAsicEavInterruptMask(&mask)) != RT_ERR_OK)
        return retVal;

    if (0 == (mask&(1<<type)))
        *pEnable=DISABLED;
    else
        *pEnable=ENABLED;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_ptp_intStatus_get
 * Description:
 *      Get PTP port interrupt trigger status.
 * Input:
 *      port           - physical port
 * Output:
 *      pStatusMask - Interrupt status bit mask.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can get interrupt trigger status when interrupt happened.
 *      The interrupt trigger status is shown in the following:
 *      - PORT 0  INT    (value[0] (Bit0))
 *      - PORT 1  INT    (value[0] (Bit1))
 *      - PORT 2  INT    (value[0] (Bit2))
 *      - PORT 3  INT    (value[0] (Bit3))
 *      - PORT 4  INT   (value[0] (Bit4))

 *
 */
rtk_api_ret_t rtk_ptp_intStatus_get(rtk_ptp_intStatus_t *pStatusMask)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pStatusMask)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicEavInterruptStatus(pStatusMask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      rtk_ptp_portIntStatus_set
 * Description:
 *      Set PTP port interrupt trigger status to clean.
 * Input:
 *      port           - physical port
 *      statusMask - Interrupt status bit mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can clean interrupt trigger status when interrupt happened.
 *      The interrupt trigger status is shown in the following:
 *      - PTP_INT_TYPE_TX_SYNC              (value[0] (Bit0))
 *      - PTP_INT_TYPE_TX_DELAY_REQ      (value[0] (Bit1))
 *      - PTP_INT_TYPE_TX_PDELAY_REQ    (value[0] (Bit2))
 *      - PTP_INT_TYPE_TX_PDELAY_RESP   (value[0] (Bit3))
 *      - PTP_INT_TYPE_RX_SYNC              (value[0] (Bit4))
 *      - PTP_INT_TYPE_RX_DELAY_REQ      (value[0] (Bit5))
 *      - PTP_INT_TYPE_RX_PDELAY_REQ    (value[0] (Bit6))
 *      - PTP_INT_TYPE_RX_PDELAY_RESP   (value[0] (Bit7))
 *      The status will be cleared after execute this API.
 */
rtk_api_ret_t rtk_ptp_portIntStatus_set(rtk_port_t port, rtk_ptp_intStatus_t statusMask)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port is PTP port */
    RTK_CHK_PORT_IS_PTP(port);

    if ((retVal = rtl8367c_setAsicEavPortInterruptStatus(rtk_switch_port_L2P_get(port), statusMask))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_ptp_portIntStatus_get
 * Description:
 *      Get PTP port interrupt trigger status.
 * Input:
 *      port           - physical port
 * Output:
 *      pStatusMask - Interrupt status bit mask.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can get interrupt trigger status when interrupt happened.
 *      The interrupt trigger status is shown in the following:
 *      - PTP_INT_TYPE_TX_SYNC              (value[0] (Bit0))
 *      - PTP_INT_TYPE_TX_DELAY_REQ      (value[0] (Bit1))
 *      - PTP_INT_TYPE_TX_PDELAY_REQ    (value[0] (Bit2))
 *      - PTP_INT_TYPE_TX_PDELAY_RESP   (value[0] (Bit3))
 *      - PTP_INT_TYPE_RX_SYNC              (value[0] (Bit4))
 *      - PTP_INT_TYPE_RX_DELAY_REQ      (value[0] (Bit5))
 *      - PTP_INT_TYPE_RX_PDELAY_REQ    (value[0] (Bit6))
 *      - PTP_INT_TYPE_RX_PDELAY_RESP   (value[0] (Bit7))
 *
 */
rtk_api_ret_t rtk_ptp_portIntStatus_get(rtk_port_t port, rtk_ptp_intStatus_t *pStatusMask)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port is PTP port */
    RTK_CHK_PORT_IS_PTP(port);

    if(NULL == pStatusMask)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicEavPortInterruptStatus(rtk_switch_port_L2P_get(port), pStatusMask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_ptp_portPtpTrap_set
 * Description:
 *      Set PTP packet trap of the specified port.
 * Input:
 *      port   - port id
 *      enable - status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT     - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
rtk_api_ret_t rtk_ptp_portTrap_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (enable>=RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicEavTrap(rtk_switch_port_L2P_get(port), enable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_ptp_portPtpEnable_get
 * Description:
 *      Get PTP packet trap of the specified port.
 * Input:
 *      port    - port id
 * Output:
 *      pEnable - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT         - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
rtk_api_ret_t rtk_ptp_portTrap_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if ((retVal = rtl8367c_getAsicEavTrap(rtk_switch_port_L2P_get(port), pEnable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



