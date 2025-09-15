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
 * Feature : The file includes time module high-layer API definition
 *
 */

#ifndef __RTK_API_PTP_H__
#define __RTK_API_PTP_H__

/*
 * Symbol Definition
 */
#define RTK_MAX_NUM_OF_NANO_SECOND                     0x3B9AC9FF
#define RTK_PTP_INTR_MASK                                          0xFF
#define RTK_MAX_NUM_OF_TPID                                    0xFFFF

/* Message Type */
typedef enum rtk_ptp_msgType_e
{
    PTP_MSG_TYPE_TX_SYNC = 0,
    PTP_MSG_TYPE_TX_DELAY_REQ,
    PTP_MSG_TYPE_TX_PDELAY_REQ,
    PTP_MSG_TYPE_TX_PDELAY_RESP,
    PTP_MSG_TYPE_RX_SYNC,
    PTP_MSG_TYPE_RX_DELAY_REQ,
    PTP_MSG_TYPE_RX_PDELAY_REQ,
    PTP_MSG_TYPE_RX_PDELAY_RESP,
    PTP_MSG_TYPE_END
} rtk_ptp_msgType_t;

typedef enum rtk_ptp_intType_e
{
    PTP_INT_TYPE_TX_SYNC = 0,
    PTP_INT_TYPE_TX_DELAY_REQ,
    PTP_INT_TYPE_TX_PDELAY_REQ,
    PTP_INT_TYPE_TX_PDELAY_RESP,
    PTP_INT_TYPE_RX_SYNC,
    PTP_INT_TYPE_RX_DELAY_REQ,
    PTP_INT_TYPE_RX_PDELAY_REQ,
    PTP_INT_TYPE_RX_PDELAY_RESP,
    PTP_INT_TYPE_ALL,
    PTP_INT_TYPE_END
}rtk_ptp_intType_t;

typedef enum rtk_ptp_sys_adjust_e
{
    SYS_ADJUST_PLUS = 0,
    SYS_ADJUST_MINUS,
    SYS_ADJUST_END
} rtk_ptp_sys_adjust_t;


/* Reference Time */
typedef struct rtk_ptp_timeStamp_s
{
    rtk_uint32 sec;
    rtk_uint32 nsec;
} rtk_ptp_timeStamp_t;

typedef struct rtk_ptp_info_s
{
    rtk_uint32 sequenceId;
    rtk_ptp_timeStamp_t   timeStamp;
} rtk_ptp_info_t;

typedef rtk_uint32 rtk_ptp_tpid_t;

typedef rtk_uint32  rtk_ptp_intStatus_t;     /* interrupt status mask  */

/*
 * Data Declaration
 */

/*
 * Function Declaration
 */
/* Function Name:
 *      rtk_time_init
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
 *      This API is used to initialize EEE status.
 */
extern rtk_api_ret_t rtk_ptp_init(void);

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
extern rtk_api_ret_t rtk_ptp_mac_set(rtk_mac_t mac);

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
extern rtk_api_ret_t rtk_ptp_mac_get(rtk_mac_t *pMac);

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
extern rtk_api_ret_t rtk_ptp_tpid_set(rtk_ptp_tpid_t outerId, rtk_ptp_tpid_t innerId);

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
extern rtk_api_ret_t rtk_ptp_tpid_get(rtk_ptp_tpid_t *pOuterId, rtk_ptp_tpid_t *pInnerId);

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
extern rtk_api_ret_t rtk_ptp_refTime_set(rtk_ptp_timeStamp_t timeStamp);

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
extern rtk_api_ret_t rtk_ptp_refTime_get(rtk_ptp_timeStamp_t *pTimeStamp);

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
extern rtk_api_ret_t rtk_ptp_refTimeAdjust_set(rtk_ptp_sys_adjust_t sign, rtk_ptp_timeStamp_t timeStamp);

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
extern rtk_api_ret_t rtk_ptp_refTimeEnable_set(rtk_enable_t enable);

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
extern rtk_api_ret_t rtk_ptp_refTimeEnable_get(rtk_enable_t *pEnable);

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
extern rtk_api_ret_t rtk_ptp_portEnable_set(rtk_port_t port, rtk_enable_t enable);

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
extern rtk_api_ret_t rtk_ptp_portEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_ptp_portTimestamp_get
 * Description:
 *      Get PTP timestamp according to the PTP identifier on the dedicated port from the specified device.
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
extern rtk_api_ret_t rtk_ptp_portTimestamp_get( rtk_port_t port, rtk_ptp_msgType_t type, rtk_ptp_info_t *pInfo);

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
extern rtk_api_ret_t rtk_ptp_intControl_set(rtk_ptp_intType_t type, rtk_enable_t enable);

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
extern rtk_api_ret_t rtk_ptp_intControl_get(rtk_ptp_intType_t type, rtk_enable_t *pEnable);


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
extern rtk_api_ret_t rtk_ptp_intStatus_get(rtk_ptp_intStatus_t *pStatusMask);

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
extern rtk_api_ret_t rtk_ptp_portIntStatus_set(rtk_port_t port, rtk_ptp_intStatus_t statusMask);

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
extern rtk_api_ret_t rtk_ptp_portIntStatus_get(rtk_port_t port, rtk_ptp_intStatus_t *pStatusMask);

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
extern rtk_api_ret_t rtk_ptp_portTrap_set(rtk_port_t port, rtk_enable_t enable);

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
extern rtk_api_ret_t rtk_ptp_portTrap_get(rtk_port_t port, rtk_enable_t *pEnable);

#endif /* __RTK_API_PTP_H__ */
