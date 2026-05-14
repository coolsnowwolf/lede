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
 * Feature : The file includes time module high-layer API defination
 *
 */

#ifndef __DAL_RTL8373_PTP_H__
#define __DAL_RTL8373_PTP_H__

#include "ptp.h"

/*
 * Symbol Definition
 */
#define DAL_RTL8373_MAX_NUM_OF_NANO_SECOND                     0x3B9AC9FF
#define DAL_RTL8373_PTP_INTR_MASK                                          0xFF
#define DAL_RTL8373_MAX_NUM_OF_TPID                                    0xFFFF
#define RTL8373_MAX_PPS_WIDTH                 630




typedef enum RTL8373_PTP_TIME_CMD_E
{
    PTP_TIME_READ = 0,
    PTP_TIME_WRITE,
     PTP_TIME_ADJUST,
    PTP_TIME_CMD_END
}RTL8373_PTP_TIME_CMD;


typedef enum RTL8373_PTP_TIME_ADJ_E
{
    PTP_TIME_ADJ_INC = 0,
    PTP_TIME_ADJ_DEC,
    PTP_TIME_ADJ_END
}RTL8373_PTP_TIME_ADJ;



typedef struct  rtl8373_ptp_tx_time_stamp_s{
    rtk_uint32 valid;
    rtk_uint32 portid;
    rtk_uint32 msgtype;
    rtk_ptp_info_t timestamp;
}rtl8373_ptp_tx_time_stamp_t;

typedef struct  rtl8373_ptp_imr_s{
    rtk_uint32 imr_pps_1;
    rtk_uint32 imr_tod_done;
    rtk_uint32 imr_txtime_empty;
}rtl8373_ptp_imr_t;

typedef struct  rtl8373_ptp_isr_s{
    rtk_uint32 isr_pps_1;
    rtk_uint32 isr_tod_done;
    rtk_uint32 isr_txtime_empty;
}rtl8373_ptp_isr_t;


typedef struct  rtl8373_ptp_todframe_s{
    rtk_uint16 TodDate[16];
}rtl8373_ptp_todframe_t;
#define RTL8373_PTP_INTR_MASK        0xFF

#define RTL8373_PTP_PORT_MASK        0x3FF
/*
 * Data Declaration
 */

/*
 * Function Declaration
 */
/* Function Name:
 *      rtk_switch_port_L2Ptpport_get
 * Description:
 *      Get ptp port ID
 * Input:
 *      logicalPort       - logical port ID
 * Output:
 *      None
 * Return:
 *      ptp port ID
 * Note:
 *
 */
extern rtk_uint32 rtk_switch_port_L2Ptpport_get(rtk_port_t logicalPort);
/* Function Name:
 *      dal_rtl8373_ptp_init
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
ret_t dal_rtl8373_ptp_internalport(rtk_portmask_t portmask);



/* Function Name:
 *      dal_rtl8373_ptp_init
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
extern ret_t dal_rtl8373_ptp_init(  rtk_portmask_t ptpinternalpmask);
/* Function Name:
 *      dal_rtl8373_bypassptpEn_get
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
extern ret_t dal_rtl8373_bypassptpEn_get( rtk_port_t port, rtk_enable_t *pEnable);
/* Function Name:
 *      dal_rtl8373_bypassptpEn_set
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
extern ret_t dal_rtl8373_bypassptpEn_set( rtk_port_t port, rtk_enable_t Enable);

/* Function Name:
 *      dal_rtl8373_ptpEn_get
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
extern ret_t dal_rtl8373_ptpEn_get( rtk_port_t port, rtk_ptp_header_t type,rtk_enable_t *pEnable);


/* Function Name:
 *      dal_rtl8373_ptpEn_get
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
extern ret_t dal_rtl8373_ptpEn_set( rtk_port_t port, rtk_ptp_header_t type,rtk_enable_t Enable);

/* Function Name:
 *      dal_rtl8373_ptp_TxTimestampFifo_get
 * Description:
 *      Get the top entry from PTP Tx timstamp FIFO on the dedicated port from the specified device.
 * Input:
 *      port        - port id
 * Output:
 *      pTimeEntry  - pointer buffer of TIME timestamp entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ptp_TxTimestampFifo_get( rtk_time_txTimeEntry_t *pTimeEntry);
/* Function Name:
 *      dal_rtl8373_ptp_Oper_triger
 * Description:
 *      Set the PTP time operation configuration of specific port.
 * Input:
 *      triType     
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ptp_Oper_triger(void);
/* Function Name:
 *      dal_rtl8373_ptp_Oper_get
 * Description:
 *      Get the PTP time operation configuration of specific port.
 * Input:
 *      port        - port ID
 * Output:
 *      pOperCfg    - pointer to PTP time operation configuraton
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ptp_Oper_get(rtk_time_operCfg_t *pOperCfg);
/* Function Name:
 *      dal_rtl8373_ptp_Oper_set
 * Description:
 *      Set the PTP time operation configuration of specific port.
 * Input:
 *      port        - port ID
 *      pOperCfg    - pointer to PTP time operation configuraton
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ptp_Oper_set( rtk_time_operCfg_t pOperCfg);
/* Function Name:
 *      dal_rtl8373_ptp_LatchTime_get
 * Description:
 *      Get the PTP latched time of specific port.
 * Input:
 * Output:
 *      pLatchTime    - pointer to PTP time operation configuraton
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ptp_LatchTime_get( rtk_time_timeStamp_t *pLatchTime);

/* Function Name:
 *      dal_rtl8373_ptp_1PPSOutput_get
 * Description:
 *      Get 1 PPS output configuration of the specified port.
 * Input:
 * Output:
 *      pPulseWidth - pointer to 1 PPS pulse width, unit: 10 ms
 *      pEnable     - pointer to 1 PPS output enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ptp_1PPSOutput_get(rtk_uint32 *pPulseWidth, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_rtl8373_ptp_1PPSOutput_set
 * Description:
 *      Set 1 PPS output configuration of the specified port.
 * Input:
 *      pulseWidth  - pointer to 1 PPS pulse width, unit: 10 ms
 *      enable      - enable 1 PPS output
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ptp_1PPSOutput_set( rtk_uint32 pulseWidth, rtk_enable_t enable);

/* Function Name:
 *      dal_rtl8373_ptp_ClockOutput_get
 * Description:
 *      Get clock output configuration of the specified port.
 * Input:
 *      pClkOutput   -pClkOutput
 * Output:
 *      pClkOutput  - pointer to clock output configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ptp_ClockOutput_get( rtk_time_clkOutput_t *pClkOutput);
/* Function Name:
 *      dal_rtl8373_ptp_ClockOutput_set
 * Description:
 *      Set 1 PPS output configuration of the specified port.
 * Input:
 *      pClkOutput  - pointer to clock output configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ptp_ClockOutput_set( rtk_time_clkOutput_t pClkOutput);
#if 0
/* Function Name:
 *      dal_rtl8373_ptp_mac_set
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
extern ret_t dal_rtl8373_ptp_mac_set(rtk_mac_t mac);

/* Function Name:
 *      dal_rtl8373_ptp_mac_get
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
extern ret_t dal_rtl8373_ptp_mac_get(rtk_mac_t *pMac);
#endif
/* Function Name:
 *      dal_rtl8373_ptp_portctrl_set
 * Description:
 *      Get enable status for PTP transparent clock of the specified port.
 * Input:
 *      port    - port id
 *      portcfg   -port role/udp_en/eth_en/always_ts
 * Output:
 *      
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ptp_portctrl_set( rtk_port_t port, rtk_ptp_port_ctrl_t portcfg);
/* Function Name:
 *      dal_rtl8373_ptp_TransEnable_get
 * Description:
 *      Get enable status for PTP transparent clock of the specified port.
 * Input:
 *      port    - port id
 * Output:
 *      portcfg   -port role/udp_en/eth_en/always_ts
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ptp_portctrl_get( rtk_port_t port, rtk_ptp_port_ctrl_t *pportcfg);
/* Function Name:
 *      dal_rtl8373_ptp_tpid_set
 * Description:
 *      Set PTP accepted outer or inner tag TPID.
 * Input:
 *      type
 *      idx
 * Output:
 *      Tpid - Ether type of tag frame parsing in PTP ports.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ptp_tpid_set(rtk_vlanType_t type, rtk_uint32 idx, rtk_uint32 Tpid);

/* Function Name:
 *      dal_rtl8373_ptp_tpid_get
 * Description:
 *      Get PTP accepted outer or inner tag TPID.
 * Input:
 *      type
 *      idx
 * Output:
 *      pTpid - Ether type of tag frame parsing in PTP ports.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ptp_tpid_get(rtk_vlanType_t type, rtk_uint32 idx, rtk_uint32 *pTpid);

/* Function Name:
 *      dal_rtl8373_ptp_refTime_set
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
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ptp_refTime_set(rtk_time_timeStamp_t timeStamp, rtk_enable_t apply);

/* Function Name:
 *      dal_rtl8373_ptp_refTime_get
 * Description:
 *      Get the reference time of the specified device by software.
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
 *      8371
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ptp_refTime_get(rtk_time_timeStamp_t *pTimeStamp);

/* Function Name:
 *      dal_rtl8373_ptp_refTimeAdjust_set
 * Description:
 *      Adjust the reference time.
 * Input:
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
extern ret_t dal_rtl8373_ptp_refTimeAdjust_set(rtk_ptp_sys_adjust_t sign, rtk_time_timeStamp_t timeStamp, rtk_enable_t apply);

/* Function Name:
 *      dal_rtl8373_ptp_refTimeEnable_set
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
extern ret_t dal_rtl8373_ptp_refTimeEnable_set(rtk_enable_t enable);

/* Function Name:
 *      dal_rtl8373_ptp_refTimeEnable_get
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
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ptp_refTimeEnable_get(rtk_enable_t *pEnable);

/* Function Name:
 *      dal_rtl8373_ptp_portEnable_set
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
extern ret_t dal_rtl8373_ptp_portEnable_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      dal_rtl8373_ptp_portEnable_get
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
extern ret_t dal_rtl8373_ptp_portEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      dal_rtl8373_ptp_portTimestamp_get
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
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ptp_portTimestamp_get( rtk_port_t port, rtk_ptp_msgType_t type, rtk_ptp_info_t *pInfo);

/* Function Name:
 *      dal_rtl8373_ptp_intControl_set
 * Description:
 *      Set PTP interrupt trigger status configuration.
 * Input:
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
 *      PTP_INT_TYPE_1PPS = 0,
 *      PTP_INT_TYPE_TOD_DONE,
 *      PTP_INT_TYPE_TXTIME_EMPTY
 */
extern  ret_t dal_rtl8373_ptp_intControl_set(rtk_ptp_intType_t type, rtk_enable_t enable);
;

/* Function Name:
 *      dal_rtl8373_ptp_intControl_get
 * Description:
 *      Get PTP interrupt trigger status configuration.
 * Input:
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
 *      PTP_INT_TYPE_1PPS = 0,
 *      PTP_INT_TYPE_TOD_DONE,
 *      PTP_INT_TYPE_TXTIME_EMPTY
 */
extern  ret_t dal_rtl8373_ptp_intControl_get(rtk_ptp_intType_t type,rtk_enable_t *pEnable);

/* Function Name:
 *      dal_rtl8373_ptp_intStatus_get
 * Description:
 *      Get PTP interrupt trigger status configuration.
 * Input:
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
 *      PTP_INT_TYPE_1PPS = 0,
 *      PTP_INT_TYPE_TOD_DONE,
 *      PTP_INT_TYPE_TXTIME_EMPTY,
 */
 extern ret_t dal_rtl8373_ptp_intStatus_get(rtk_uint32 *mask);
/* Function Name:
 *      dal_rtl8373_ptp_toddelay_set
 * Description:
 *      Set toddelay.
 * Input:
 *      toddelay  - toddelay
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ptp_toddelay_set( rtk_uint32 toddelay);

/* Function Name:
 *      dal_rtl8373_ptp_toddelay_get
 * Description:
 *      Set toddelay.
 * Input:
 *      toddelay  - toddelay
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ptp_toddelay_get(rtk_uint32 *ptoddelay);

/* Function Name:
 *      dal_rtl8373_ptp_RefTimeFreqCfg_set
 * Description:
 *      Set the frequency of reference time of PHY of the specified port.
 * Input:
 *      freq    - reference time frequency
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The frequency configuration decides the reference time tick frequency.
 *      The default value is 0x10000000.
 *      If it is configured to 0x8000000, the tick frequency would be half of default.
 *      If it is configured to 0x20000000, the tick frequency would be one and half times of default.
 */
extern ret_t dal_rtl8373_ptp_RefTimeFreqCfg_set(rtk_uint32 freq,rtk_enable_t apply);

/* Function Name:
 *      dal_rtl8373_ptp_RefTimeFreqCfg_get
 * Description:
 *      Set  ptp_RefTimeFreqCfg_get.
 * Input:
 *      None    
 * Output:
 *      cfgFreq
 *      curFreq
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 */
extern ret_t dal_rtl8373_ptp_RefTimeFreqCfg_get(rtk_uint32 *cfgFreq,rtk_uint32 *curFreq);


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
extern ret_t dal_rtl8373_ptp_portIntStatus_set(rtk_port_t port, rtk_ptp_intStatus_t statusMask);

/* Function Name:
 *      dal_rtl8373_ptp_portIntStatus_get
 * Description:
 *      Get PTP port interrupt trigger status.
 * Input:
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
extern ret_t dal_rtl8373_ptp_portIntStatus_get(rtk_ptp_intStatus_t *pStatusMask);

/* Function Name:
 *      dal_rtl8373_ptp_portTrap_set
 * Description:
 *      Set PTP packet trap of the specified port.
 * Input:
 *      port   - port id
 *      *trapctrl -trapen mask
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
extern ret_t dal_rtl8373_ptp_portTrap_set(rtk_port_t port, rtk_ptp_porttrap_ctrl_t *trapctrl );

/* Function Name:
 *      dal_rtl8373_ptp_portTrap_get
 * Description:
 *      Get PTP packet trap of the specified port.
 * Input:
 *      port    - port id
 * Output:
 *      *trapctrl - trapen mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT         - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ptp_portTrap_get(rtk_port_t port,  rtk_ptp_porttrap_ctrl_t *trapctrl);


/* Function Name:
 *      dal_rtl8373_ptp_TransEnable_set
 * Description:
 *      Set TX/RX timer value compensation..
 * Input:
 *      port    - port id
 *      TxImbal  - TX timer value compensation
 *      RxImbal  - RX timer value compensation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      unit: 1 ns
 */
ret_t dal_rtl8373_ptp_TxImbal_set(rtk_port_t port, rtk_uint32 TxImbal,rtk_uint32 RxImbal);

/* Function Name:
 *      dal_rtl8373_ptp_TxImbal_get
 * Description:
 *      Get TX/RX timer value compensation..
 * Input:
 *      port    - port id
 *      PTxImbal  - TX timer value compensation
 *      pRxImbal  - RX timer value compensation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      unit: 1 ns
 */
ret_t dal_rtl8373_ptp_TxImbal_get(rtk_port_t port, rtk_uint32 *pTxImbal,rtk_uint32 *pRxImbal);
/* Function Name:
 *      dal_rtl8373_ptp_toddelay_set
 * Description:
 *      Set toddelay.
 * Input:
 *      type    - accessType
 *      toddelay  - toddelay
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
ret_t dal_rtl8373_ptp_toddelay_set(  rtk_uint32 toddelay);

/* Function Name:
 *      dal_rtl8373_ptp_toddelay_get
 * Description:
 *      Set toddelay.
 * Input:
 *      type    - accessType
 *      toddelay  - toddelay
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
ret_t dal_rtl8373_ptp_toddelay_get( rtk_uint32 *ptoddelay);
/* Function Name:
 *      dal_rtl8373_ptp_phyidtoptpid_set
 * Description:
 *      Set Packet TX port ID..
 * Input:
 *      port    - port id
 *      ptp_portino  - TX timer value compensation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      unit: 1 ns
 */
ret_t dal_rtl8373_ptp_phyidtoportid_set(rtk_port_t port, rtk_port_t ptp_portino);

/* Function Name:
 *      dal_rtl8373_ptp_phyidtoportid_get
 * Description:
 *      Get Packet TX port ID..
 * Input:
 *      port    - port id
 *      TxImbal  - TX timer value compensation
 *      RxImbal  - RX timer value compensation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      unit: 1 ns
 */
ret_t dal_rtl8373_ptp_phyidtoportid_get(rtk_port_t port, rtk_port_t *pptp_portino);
/* Function Name:
 *      dal_rtl8373_ptp_PPSLatchTime_get
 * Description:
 *      Set toddelay.
 * Input:
 *      type    - accessType
 * Output:
 *      pLatchTime  - latch time
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      mac mode only
 */
ret_t dal_rtl8373_ptp_PPSLatchTime_get( rtk_time_timeStamp_t *pLatchTime);
/* Function Name:
 *      dal_rtl8373_ptp_RefTimeFreqCfg_set
 * Description:
 *      Set the frequency of reference time of PHY of the specified port.
 * Input:
 *      freq    - reference time frequency
 *      apply
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The frequency configuration decides the reference time tick frequency.
 *      The default value is 0x10000000.
 *      If it is configured to 0x8000000, the tick frequency would be half of default.
 *      If it is configured to 0x20000000, the tick frequency would be one and half times of default.
 */
ret_t dal_rtl8373_ptp_RefTimeFreqCfg_set(rtk_uint32 freq, rtk_enable_t apply);

/* Function Name:
 *      dal_rtl8373_ptp_RefTimeFreqCfg_get
 * Description:
 *      Set  ptp_RefTimeFreqCfg_get.
 * Input:
 *      None    
 * Output:
 *      cfgFreq
 *      curFreq
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 */
ret_t dal_rtl8373_ptp_RefTimeFreqCfg_get(rtk_uint32 *cfgFreq,rtk_uint32 *curFreq);


/* Function Name:
 *      dal_rtl8373_ptp_ClkSrcCtrl_set
 * Description:
 *      Set PTP time Clock source selection
 * Input:
 *      clksrc
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 * 0: internal clock (Internal PLL, 1GMHz)
 * 1: external clock, refer to cfg_ext_clk_src)
 */
ret_t dal_rtl8373_ptp_ClkSrcCtrl_set(rtk_enable_t clksrc);

/* Function Name:
 *      dal_rtl8373_ptp_ClkSrcCtrl_get
 * Description:
 *      Get PTP time Clock source selection
 * Input:
 *      clksrc
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 * 0: internal clock (Internal PLL, 1GMHz)
 * 1: external clock, refer to cfg_ext_clk_src)
 */
ret_t dal_rtl8373_ptp_ClkSrcCtrl_get(rtk_enable_t *clksrc);



#endif /* __DAL_RTL8373_PTP_H__ */

