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

#ifndef __RTK_PHY_PHY_RTL8224__H__
#define __RTK_PHY_PHY_RTL8224__H__

#define PORT_NUM_IN_8224  (4)

#define RTL8224TOPDEVAD  (30)
#define RTL8224BASEID  (0x1)
#define RTL8224PHYID  (0x0)



/* Function Name:
 *      phy_8224drv_mapper_get
 * Description:
 *      Get DAL mapper function
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      dal_mapper_t *     - mapper pointer
 * Note:
 */
extern rt_phydrv_t *phy_8224drv_mapper_get(void);

/* Function Name:
 *      phy_8224_ptp_portmask
 * Description:
 *      PTP function initialization.
 * Input:
 *      portmask   range 0~0xf
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API is used to initialize PTP status.
 */
extern ret_t phy_8224_ptp_portmask(rtk_portmask_t portmask);
/* Function Name:
 *      phy_8224_ptp_init
 * Description:
 *      PTP function initialization.
 * Input:
 *      ptpinternalpmask    port range 0~0xf
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API is used to initialize PTP status.
 */
extern ret_t phy_8224_ptp_init(  rtk_portmask_t ptppmask);

/* Function Name:
 *      dal_rtl8371c_bypassptpEn_get
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
extern ret_t phy_8224_bypassptpEn_get( rtk_port_t port, rtk_enable_t *pEnable);


/* Function Name:
 *      phy_8224_bypassptpEn_set
 * Description:
 *      Get PTP status of the specified port.
 * Input:
 *      port    - port id
 *      pEnable - status
 * Output:
  *     None 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT         - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern ret_t phy_8224_bypassptpEn_set( rtk_port_t port, rtk_enable_t pEnable);

/* Function Name:
 *      phy_8224_ptpEn_get
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
extern ret_t phy_8224_ptpEn_get( rtk_port_t port, rtk_ptp_header_t type,rtk_enable_t *pEnable);


/* Function Name:
 *      phy_8224_ptpEn_get
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
extern ret_t phy_8224_ptpEn_set( rtk_port_t port, rtk_ptp_header_t type,rtk_enable_t pEnable);

/* Function Name:
 *      phy_8224_ptp_tpid_set
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
extern ret_t phy_8224_ptp_tpid_set(rtk_vlanType_t type, rtk_uint32 idx, rtk_uint32 Tpid);

/* Function Name:
 *      phy_8224_ptp_tpid_get
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
extern ret_t phy_8224_ptp_tpid_get(rtk_vlanType_t type, rtk_uint32 idx, rtk_uint32 *pTpid);

/* Function Name:
 *      phy_8224_ptp_Oper_get
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
extern ret_t phy_8224_ptp_Oper_get(rtk_time_operCfg_t *pOperCfg);

/* Function Name:
 *      phy_8224_ptp_Oper_set
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
extern ret_t phy_8224_ptp_Oper_set( rtk_time_operCfg_t pOperCfg);

/* Function Name:
 *      dal_rtl8371c_ptp_LatchTime_get
 * Description:
 *      Get the PTP latched time of specific port by hardware.
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
extern ret_t phy_8224_ptp_LatchTime_get( rtk_time_timeStamp_t *pLatchTime);

/* Function Name:
 *      phy_8224_ptp_refTimeOp_set
 * Description:
 *      Set the reference time of the specified device.
 * Input:
 *      extoption - reference timestamp value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 * Note:
 *      need gpio triger
 */
extern ret_t phy_8224_ptp_refTimeOp_set(rtk_uint32 extoption);

/* Function Name:
 *      dal_rtl8371c_ptp_refTime_set
 * Description:
 *      Set the reference time of the specified device.
 * Input:
 *      type
 *      timeStamp - reference timestamp value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 * Note:
 *      need gpio triger
 */
extern ret_t phy_8224_ptp_refTime_set(rtk_time_timeStamp_t timeStamp);


/* Function Name:
 *      phy_8224_ptp_refTime_get
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
 *      need gpio triger
 */
extern ret_t phy_8224_ptp_refTime_get(rtk_time_timeStamp_t *pTimeStamp);

/* Function Name:
 *      phy_8224_ptp_refTimeAdjust_set
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
extern  ret_t phy_8224_ptp_refTimeAdjust_set( rtk_ptp_sys_adjust_t sign, rtk_time_timeStamp_t timeStamp);

/* Function Name:
 *      phy_8224_ptp_TxTimestampFifo_get
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
extern ret_t phy_8224_ptp_TxTimestampFifo_get(rtk_time_txTimeEntry_t *pTimeEntry);

/* Function Name:
 *      phy_8224_ptp_1PPSOutput_get
 * Description:
 *      Get 1 PPS output configuration of the specified port.
 * Input:
 *      type        - phy or mac
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
extern ret_t phy_8224_ptp_1PPSOutput_get( rtk_uint32 *pPulseWidth, rtk_enable_t *pEnable);

/* Function Name:
 *      phy_8224_ptp_1PPSOutput_set
 * Description:
 *      Set 1 PPS output configuration of the specified port.
 * Input:
 *      type        - phy or mac
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
extern ret_t phy_8224_ptp_1PPSOutput_set( rtk_uint32 pulseWidth, rtk_enable_t enable);

/* Function Name:
 *      phy_8224_ptp_1PPSOutput_set
 * Description:
 *      Set 1 PPS output configuration of the specified port.
 * Input:
 *      type        - phy or mac
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
extern ret_t phy_8224_ptp_1PPSOutput_set( rtk_uint32 pulseWidth, rtk_enable_t enable);

/* Function Name:
 *      phy_8224_ptp_ClockOutput_get
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
extern ret_t phy_8224_ptp_ClockOutput_get(  rtk_time_clkOutput_t *pClkOutput);

/* Function Name:
 *      phy_8224_ptp_ClockOutput_set
 * Description:
 *      Set 1 PPS output configuration of the specified port.
 * Input:
*       type            - phy or mac
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
extern ret_t phy_8224_ptp_ClockOutput_set( rtk_time_clkOutput_t pClkOutput);

/* Function Name:
 *      phy_8224_ptp_portctrl_set
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
extern ret_t phy_8224_ptp_portctrl_set( rtk_port_t port, rtk_ptp_port_ctrl_t portcfg);

/* Function Name:
 *      phy_8224_ptp_portctrl_get
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
extern ret_t phy_8224_ptp_portctrl_get( rtk_port_t port, rtk_ptp_port_ctrl_t *pportcfg);

/* Function Name:
 *      phy_8224_ptp_TxImbal_set
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
extern ret_t  phy_8224_ptp_TxImbal_set(rtk_port_t port, rtk_uint32 TxImbal,rtk_uint32 RxImbal);

/* Function Name:
 *      phy_8224_ptp_TxImbal_get
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
extern ret_t phy_8224_ptp_TxImbal_get(rtk_port_t port, rtk_uint32 *pTxImbal,rtk_uint32 *pRxImbal);

/* Function Name:
 *      phy_8224_ptp_phyidtoportid_set
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
extern ret_t phy_8224_ptp_phyidtoportid_set(rtk_port_t port, rtk_port_t ptp_portino);

/* Function Name:
 *      phy_8224_ptp_phyidtoptpid_get
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
extern ret_t phy_8224_ptp_phyidtoptpid_get(rtk_port_t port, rtk_port_t *pptp_portino);

/* Function Name:
 *      phy_8224_ptp_PPSLatchTime_get
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
extern ret_t phy_8224_ptp_PPSLatchTime_get( rtk_time_timeStamp_t *pLatchTime);

/* Function Name:
 *      phy_8224_ptp_RefTimeFreqCfg_set
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
extern ret_t phy_8224_ptp_RefTimeFreqCfg_set(rtk_uint32 freq, rtk_enable_t apply);

/* Function Name:
 *      phy_8224_ptp_RefTimeFreqCfg_get
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
extern ret_t phy_8224_ptp_RefTimeFreqCfg_get(rtk_uint32 *cfgFreq,rtk_uint32 *curFreq);

/* Function Name:
 *      phy_8224_ptp_ClkSrcCtrl_set
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
extern ret_t phy_8224_ptp_ClkSrcCtrl_set(rtk_uint32 clksrc);



/* Function Name:
 *      phy_8224_ptp_ClkSrcCtrl_set
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
extern ret_t phy_8224_ptp_ClkSrcCtrl_get(rtk_uint32 *pclksrc);
#endif
