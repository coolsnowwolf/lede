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
 * Purpose : RTL8367/RTL8367C switch high-level API
 *
 * Feature : The file includes time module high-layer API defination
 *
 */

#ifndef __RTK_API_PTP_H__
#define __RTK_API_PTP_H__

/*
 * Symbol Definition
 */
#define RTK_MAX_NUM_OF_NANO_SECOND                     0x3B9ACA00
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
    PTP_INT_TYPE_1PPS = 0,
    PTP_INT_TYPE_TOD_DONE,
    PTP_INT_TYPE_TXTIME_EMPTY,
    PTP_INT_TYPE_END
}rtk_ptp_intType_t;

typedef enum rtk_ptp_sys_adjust_e
{
    SYS_ADJUST_PLUS = 0,
    SYS_ADJUST_MINUS,
    SYS_ADJUST_END
} rtk_ptp_sys_adjust_t;

typedef enum rtk_ptp_header_e
{
    PTP_ETH = 0,
    PTP_UDP ,
    PTP_END
} rtk_ptp_header_t;

/* Reference Time */
typedef struct rtk_time_timeStamp_s
{
    rtk_uint64 sec;
    rtk_uint32 nsec;
} rtk_time_timeStamp_t;

typedef struct rtk_ptp_info_s
{
    rtk_uint32 sequenceId;
    rtk_time_timeStamp_t   timeStamp;
} rtk_ptp_info_t;

typedef enum rtk_ptp_port_role_e
{
    PTP_PORT_NONE= 0,
    PTP_PORT_BCOC,
    PTP_PORT_E2ETC,
    PTP_PORT_P2PTC,
    PTP_PORT_END,
}  rtk_ptp_port_role_t;

typedef struct 
{
    rtk_ptp_port_role_t portrole;
    rtk_enable_t udp_en;
    rtk_enable_t eth_en;
    rtk_enable_t always_ts_en;
    rtk_uint32 link_delay;    
} rtk_ptp_port_ctrl_t;

typedef struct  rtk_ptp_porttrap_ctrl_s{
    rtk_uint32 udp_en;
    rtk_uint32 eth_en;
    rtk_uint32 ptp_delay_en;//MSG TYPE 0,1,8,9
    rtk_uint32 ptp_pdelay_en;//MSG TYPE 2,3,A
    rtk_uint32 ptp_pasm_en;//MSG TYPE B,C,D
}rtk_ptp_porttrap_ctrl_t;

typedef rtk_uint32 rtk_ptp_tpid_t;

typedef rtk_uint32  rtk_ptp_intStatus_t;     /* interrupt status mask  */

typedef enum rtk_vlanType_e
{
    INNER_VLAN = 0,
    OUTER_VLAN,
    VLAN_TYPE_END
} rtk_vlanType_t;


/* Message Type */
typedef enum rtk_time_ptpMsgType_e
{
    PTP_MSG_TYPE_SYNC = 0,
    PTP_MSG_TYPE_DELAY_REQ = 1,
    PTP_MSG_TYPE_PDELAY_REQ = 2,
    PTP_MSG_TYPE_PDELAY_RESP = 3,
} rtk_time_ptpMsgType_t;

/* trap packet target */
typedef enum rtk_trapTarget_e
{
    RTK_FORWARD,
    RTK_TRAP,
    RTK_DROP,
    RTK_TRAP_END,
} rtk_trapTarget_t;

/* TIME packet identifier */
typedef struct rtk_time_ptpIdentifier_s
{
    rtk_time_ptpMsgType_t   msgType;
    rtk_uint32                  sequenceId;
} rtk_time_ptpIdentifier_t;

typedef enum rtk_time_oper_e
{
    TIME_OPER_START = 0,
    TIME_OPER_STOP,
    TIME_OPER_LATCH,
    TIME_OPER_CMD_EXEC,
    TIME_OPER_FREQ_APPLY,
    TIME_OPER_END
} rtk_time_oper_t;


typedef enum rtk_time_opertriger_e
{
    TIME_FALL_TRI= 1,
    TIME_RISE_TRI,
    TIME_BOTH_TRI,
    TIME_TRI_END
} rtk_time_opertriger_t;

typedef struct rtk_time_operCfg_s
{
    rtk_time_oper_t oper;
    rtk_enable_t rise_tri;
    rtk_enable_t fall_tri;
    rtk_enable_t tri_apply;
} rtk_time_operCfg_t;
typedef enum rtk_time_clkOutMode_e
{
    PTP_CLK_OUT_REPEAT = 0,
    PTP_CLK_OUT_PULSE = 1,
    PTP_CLK_OUT_END
} rtk_time_clkOutMode_t;

typedef enum rtk_time_outSigSel_e
{
    PTP_OUT_SIG_SEL_CLOCK = 0,
    PTP_OUT_SIG_SEL_1PPS = 1,
    PTP_OUT_SIG_SEL_END
} rtk_time_outSigSel_t;

/* TIME transmission callback function prototype */
typedef rtk_int32 (rtk_time_ptpTime_cb_f)(
    rtk_port_t                  port,
    rtk_time_ptpIdentifier_t    identifier,
    rtk_time_timeStamp_t        time);
/*
 * Data Declaration
 */
typedef struct rtk_time_txTimeEntry_s
{
    rtk_uint8 valid;
    rtk_port_t port;
    rtk_time_ptpMsgType_t msg_type;
    rtk_uint32 seqId;
    rtk_time_timeStamp_t txTime;
} rtk_time_txTimeEntry_t;

typedef struct rtk_time_clkOutput_s
{
    rtk_time_clkOutMode_t mode;
    rtk_time_timeStamp_t startTime;
    rtk_uint32 halfPeriodNsec;
    rtk_enable_t enable;
    rtk_uint8 runing; //Only valid for get API
} rtk_time_clkOutput_t;


/* Function Name:
 *      rtk_time_portPtpEnable_get
 * Description:
 *      Get PTP status of the specified port.
 * Input:
 *      port    - port id
 *      type    - PTP_ETH or UDP
 *      enable - status
 * Output:
 *      pEnable - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT         - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      
 * Note:
 *      None
 * Changes:
 *      None
 */
ret_t rtk_time_portPtpEnable_get( rtk_port_t port, rtk_ptp_header_t type,rtk_enable_t *pEnable);


/* Function Name:
 *      rtk_time_portPtpEnable_set
 * Description:
 *      Get PTP status of the specified port.
 * Input:
 *      port    - port id
 *      type    - PTP_ETH or UDP
 *      enable - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT         - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      
 * Note:
 *      None
 * Changes:
 *      None
 */
ret_t rtk_time_portPtpEnable_set( rtk_port_t port, rtk_ptp_header_t type,rtk_enable_t Enable);



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
rtk_api_ret_t rtk_time_init(rtk_portmask_t ptpinternalpmask);

/* Function Name:
 *      rtk_time_portPtpbypassEnable_get
 * Description:
 *      Get PTP status of the specified port.
 * Input:
 * Output:
 *      pEnable - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT         - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8371
 * Note:
 *      None
 * Changes:
 *      None
 */
extern rtk_api_ret_t rtk_time_portPtpbypassEnable_get(rtk_port_t port,rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_time_portPtpbypassEnable_set
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
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT     - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8371
 * Note:
 *      None
 * Changes:
 *      None
 */
extern rtk_api_ret_t rtk_time_portPtpbypassEnable_set(rtk_port_t port,rtk_enable_t enable);

/* Function Name:
 *      rtk_time_portRefTime_get
 * Description:
 *      Get the reference time of the specified port.
 * Input:
 *      port       - port id, it should be base port of PHY
 * Output:
 *      pTimeStamp - pointer buffer of the reference time
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8371
 * Note:
 *      None
 * Changes:
 *          Add port parameter.
 */
extern rtk_api_ret_t rtk_time_portRefTime_get( rtk_time_timeStamp_t *pTimeStamp);

/* Function Name:
 *      rtk_time_portRefTime_set
 * Description:
 *      Set the reference time of the specified portmask.
 * Input:
 *      timeStamp - reference timestamp value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8371
 * Note:
 *      None
 * Changes:
 *          Change name from rtk_time_refTime_set.
 *          Add portmask parameter.
 */
extern rtk_api_ret_t rtk_time_portRefTime_set( rtk_time_timeStamp_t timeStamp,rtk_enable_t apply);
/* Function Name:
 *      rtk_time_portRefTimeAdjust_set
 * Description:
 *      Adjust the reference time of portmask.
 * Input:
 *      portmask    - portmask, it should be base ports of PHYs
 *      sign      - significant
 *      timeStamp - reference timestamp value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8371
 * Note:
 *      sign=0 for positive adjustment, sign=1 for negative adjustment.
 * Changes:
 *      [3.2.0]
 *          Change name from rtk_time_refTimeAdjust_set.
 *          Add portmask parameter.
 */
extern rtk_api_ret_t rtk_time_portRefTimeAdjust_set(rtk_uint32 sign, rtk_time_timeStamp_t timeStamp,rtk_enable_t apply);

/* Function Name:
 *      rtk_time_portRefTimeEnable_get
 * Description:
 *      Get the enable state of reference time of the specified port.
 * Input:
 *      port    - port id, it should be base port of PHY
 * Output:
 *      pEnable - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *     8371
 * Note:
 *      None
 * Changes:
 *          Change name from rtk_time_refTimeEnable_get.
 *          Add port parameter.
 */
extern rtk_api_ret_t rtk_time_portRefTimeEnable_get(  rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_time_portRefTimeEnable_set
 * Description:
 *      Set the enable state of reference time of the specified portmask.
 * Input:
 *      portmask    - portmask, it should be base ports of PHYs
 *      enable - status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8371
 * Note:
 *      None
 * Changes:
 *          Change name from rtk_time_refTimeEnable_set.
 *          Add portmask parameter.
 */
extern rtk_api_ret_t rtk_time_portRefTimeEnable_set(rtk_enable_t enable);

/* Function Name:
 *      rtk_time_portPtpOper_triger
 * Description:
 *      Set the PTP time operation triger.
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
extern rtk_api_ret_t rtk_time_portPtpOper_triger(void);
#if 0
/* Function Name:
 *      rtk_time_portRefTimeFreq_get
 * Description:
 *      Get the frequency of PTP reference time of the specified port.
 * Input:
 *      port    - port id, it should be base port of PHY
 * Output:
 *      pFreq  - pointer to reference time frequency
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8371
 * Note:
 *      The frequency configuration decides the reference time tick frequency.
 *      The default value is 0x80000000.
 *      If it is configured to 0x40000000, the tick frequency would be half of default.
 *      If it is configured to 0xC0000000, the tick frequency would be one and half times of default.
 * Changes:
 *          New added function.
 *          Change name from rtk_time_refTimeFreq_get.
 *          Add port parameter.
 */
extern rtk_api_ret_t rtk_time_portRefTimeFreq_get( rtk_uint32 *pFreq);
#endif
/* Function Name:
 *      rtk_time_portRefTimeFreq_set
 * Description:
 *      Set the frequency of PTP reference time of the specified portmask.
 * Input:
 *      portmask    - portmask, it should be base ports of PHYs
 *      freq   - reference time frequency
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8371
 * Note:
 *      The frequency configuration decides the reference time tick frequency.
 *      The default value is 0x80000000.
 *      If it is configured to 0x40000000, the tick frequence would be half of default.
 *      If it is configured to 0xC0000000, the tick frequence would be one and half times of default.
 * Changes:
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portRefTimeFreq_set( rtk_uint32 freq);

/* Function Name:
 *      rtk_time_portPtpVlanTpid_get
 * Description:
 *      Get the VLAN TPID of specific port.
 * Input:
 *      port   - port ID
 *      type   - outer or inner VLAN
 *      idx    - TPID entry index
 * Output:
 *      pTpid  - pointer to TPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portPtpVlanTpid_get( rtk_vlanType_t type, rtk_uint32 idx, rtk_uint32 *pTpid);

/* Function Name:
 *      rtk_time_portPtpVlanTpid_set
 * Description:
 *      Set the VLAN TPID of specific port.
 * Input:
 *      port   - port ID
 *      type   - outer or inner VLAN
 *      idx     - TPID entry index
 *      tpid    - VLAN TPID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8371
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portPtpVlanTpid_set( rtk_vlanType_t type, rtk_uint32 idx, rtk_uint32 tpid);

/* Function Name:
 *      rtk_time_portPtpOper_get
 * Description:
 *      Get the PTP time operation configuration of specific port.
 * Input:
 * Output:
 *      pOperCfg  - pointer to PTP time operation configuraton
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8371
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portPtpOper_get(rtk_time_operCfg_t *pOperCfg);


/* Function Name:
 *      rtk_time_portPtpOper_set
 * Description:
 *      Set the PTP time operation configuration of specific port.
 * Input:
 *      pOperCfg  - pointer to PTP time operation configuraton
 * Output:
 *      None
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
 * Changes:
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portPtpOper_set( rtk_time_operCfg_t pOperCfg);

/* Function Name:
 *      rtk_time_portPtpLatchTime_get
 * Description:
 *      Get the PTP latched time of specific port.
 * Input:
 *      port   - port ID
 * Output:
 *      pOperCfg  - pointer to PTP time operation configuraton
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8371
 * Note:
 *      None
 * Changes:
 *      [SDK_3.6.0]
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portPtpLatchTime_get( rtk_time_timeStamp_t *pLatchTime);


/* Function Name:
 *      rtk_time_portPtpRefTimeFreqCfg_get
 * Description:
 *      Get the frequency of reference time of the specified port.
 * Input:
 *      port        - port id
 * Output:
 *      pFreqCfg    - pointer to configured reference time frequency
 *      pFreqCur    - pointer to current reference time frequency
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *  phy
 * Note:
 *      None
 * Changes:
 *      [SDK_NEXT_RELEASE_VERSION]
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portPtpRefTimeFreqCfg_get( rtk_uint32 *pFreqCfg, rtk_uint32 *pFreqCur);

/* Function Name:
 *      rtk_time_portPtpRefTimeFreqCfg_set
 * Description:
 *      Set the frequency of reference time of the specified port.
 * Input:
 *      port        - port id
 *      freq        - reference time frequency
 *      apply       - if the frequency is applied immediately
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Applicable:
 *      phy
 * Note:
 *      None
 * Changes:
 *      [SDK_NEXT_RELEASE_VERSION]
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portPtpRefTimeFreqCfg_set(rtk_uint32 freq, rtk_enable_t apply);

/* Function Name:
 *      rtk_time_portPtpTxInterruptStatus_get
 * Description:
 *      Get the TX timestamp FIFO non-empty interrupt status of the specified port.
 * Input:
 *      port        - port id
 * Output:
 *      pIntrSts    - interrupt status of RX/TX PTP frame types
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      phy
 * Note:
 *      None
 * Changes:
 *      [SDK_NEXT_RELEASE_VERSION]
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portPtpTxInterruptStatus_get( rtk_uint32 *pIntrSts);

/* Function Name:
 *      rtk_time_portPtpTxTimestampFifo_get
 * Description:
 *      Get the top entry from PTP Tx timstamp FIFO on the dedicated port from the specified device. of the specified port.
 * Input:
 *      port        - port id
 * Output:
 *      pTimeEntry  - pointer buffer of TIME timestamp entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      phy
 * Note:
 *      None
 * Changes:
 *      [SDK_NEXT_RELEASE_VERSION]
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portPtpTxTimestampFifo_get(rtk_time_txTimeEntry_t *pTimeEntry);

/* Function Name:
 *      rtk_time_portPtpClkSrcCtrl_get
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
extern rtk_api_ret_t rtk_time_portPtpClkSrcCtrl_get(rtk_enable_t* clksrc);



/* Function Name:
 *      rtk_time_portPtpClkSrcCtrl_set
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
extern rtk_api_ret_t rtk_time_portPtpClkSrcCtrl_set( rtk_enable_t clksrc);
/* Function Name:
 *      rtk_time_portPtptoddelay_get
 * Description:
 *     
 * Input:
 *      port        - port id
 * Output:
 *      toddelay
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portPtptoddelay_get(rtk_uint32* toddelay);



/* Function Name:
 *      rtk_time_portPtptoddelay_set
 * Description:
 *      
 * Input:
 *     toddelay
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Applicable:
 *
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portPtptoddelay_set( rtk_uint32 toddelay);
/* Function Name:
 *      rtk_time_portPtp1PPSOutput_get
 * Description:
 *      Get the 1 PPS output configuration of the specified port.
 * Input:
 *      port        - port id
 * Output:
 *      pPulseWidth - pointer to 1 PPS pulse width
 *      pEnable     - pointer to 1 PPS output enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      phy
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portPtp1PPSOutput_get(rtk_uint32 *pPulseWidth, rtk_enable_t *pEnable);


/* Function Name:
 *      rtk_time_portPtp1PPSOutput_set
 * Description:
 *      Set the 1 PPS output configuration of the specified port.
 * Input:
 *      port        - port id
 *      pulseWidth  - pointer to 1 PPS pulse width
 *      enable      - enable 1 PPS output
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Applicable:
 *
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portPtp1PPSOutput_set(rtk_uint32 pulseWidth, rtk_enable_t enable);

/* Function Name:
 *      rtk_time_portPtpClockOutput_get
 * Description:
 *      Get the clock output configuration of the specified port.
 * Input:
 *      port        - port id
 * Output:
 *      pClkOutput  - pointer to clock output configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portPtpClockOutput_get(rtk_time_clkOutput_t *pClkOutput);

/* Function Name:
 *      rtk_time_portPtpClockOutput_set
 * Description:
 *      Set the clock output configuration of the specified port.
 * Input:
 *      port        - port id
 *      pClkOutput  - pointer to clock output configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Applicable:
 * Note:
 *      None
 * Changes:
 *      [SDK_NEXT_RELEASE_VERSION]
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portPtpClockOutput_set(rtk_time_clkOutput_t pClkOutput);

/* Function Name:
 *      rtk_time_portPtpOutputSigSel_get
 * Description:
 *      Get the output pin signal selection configuration of the specified port.
 * Input:
 *      port        - port id
 * Output:
 *      pOutSigSel  - pointer to output pin signal selection configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      phy
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portPtpOutputSigSel_get(rtk_time_outSigSel_t *pOutSigSel);

/* Function Name:
 *      rtk_time_portPtpOutputSigSel_set
 * Description:
 *      Set the output pin signal selection configuration of the specified port.
 * Input:
 *      port        - port id
 *      outSigSel   - output pin signal selection configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Applicable:
 *      phy
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portPtpOutputSigSel_set(rtk_time_outSigSel_t outSigSel);

/* Function Name:
 *      rtk_time_portPtpTransEnable_get
 * Description:
 *      Get the enable status for PTP transparent clock of the specified port.
 * Input:
 *      port        - port id
 * Output:
 *      pEnable     - pointer to PTP transparent clock enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      phy
 * Note:
 *      None
 * Changes:
 *      [SDK_NEXT_RELEASE_VERSION]
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portPtpTransEnable_get(rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_time_portPtpTransEnable_set
 * Description:
 *      Set the enable status for PTP transparent clock of the specified port.
 * Input:
 *      port        - port id
 *      enable      - PTP transparent clock enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Applicable:
 *      PHY
 * Note:
 *      None
 * Changes:
 *      [SDK_NEXT_RELEASE_VERSION]
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portPtpTransEnable_set(rtk_enable_t enable);

/* Function Name:
  *      Get the PTP rtk_time_portPtpPortctrl_get ability.
 * Description:
 *      Get the PTP port ability..
 * Input:
 *      port        - port id
 * Output:
 *      portcfg
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portPtpPortctrl_get(rtk_port_t port,rtk_ptp_port_ctrl_t *portcfg);

/* Function Name:
 *      rtk_time_portPtpPortctrl_set
 * Description:
 *      Set the PTP port ability.
 * Input:
 *      port        - port id
 *      portcfg  -  portrole,linkdelay,awaysts
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      phy
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
extern rtk_api_ret_t rtk_time_portPtpPortctrl_set(rtk_port_t port,rtk_ptp_port_ctrl_t portcfg);

/* Function Name:
 *      rtk_ptp_intControl_set
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
 */
extern rtk_api_ret_t rtk_time_PtpIntControl_set(rtk_ptp_intType_t type,rtk_enable_t enable);

/* Function Name:
 *      rtk_ptp_intControl_get
 * Description:
 *      Get PTP interrupt trigger status configuration.
 * Output:
 *      pEnable - Interrupt status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can get interrupt status configuration.
 */
extern rtk_api_ret_t rtk_time_PtpIntControl_get(rtk_ptp_intType_t type,rtk_enable_t *pEnable);

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
extern rtk_api_ret_t rtk_time_PtpIntStatus_get(rtk_ptp_intStatus_t *pStatusMask);

/* Function Name:
 *      rtk_time_portPtpTrap_set
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
extern rtk_api_ret_t rtk_time_portPtpTrap_set(rtk_port_t port,  rtk_ptp_porttrap_ctrl_t *trapctrl );

/* Function Name:
 *      rtk_time_portPtpTrap_get
 * Description:
 *      Get PTP packet trap of the specified port.
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
extern rtk_api_ret_t rtk_time_portPtpTrap_get(rtk_port_t port, rtk_ptp_porttrap_ctrl_t *trapctrl );

#endif /* __RTK_API_PTP_H__ */

