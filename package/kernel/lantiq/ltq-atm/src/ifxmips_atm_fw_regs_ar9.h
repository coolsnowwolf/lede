/******************************************************************************
**
** FILE NAME    : ifxmips_atm_fw_regs_ar9.h
** PROJECT      : UEIP
** MODULES     	: ATM (ADSL)
**
** DATE         : 1 AUG 2005
** AUTHOR       : Xu Liang
** DESCRIPTION  : ATM Driver (Firmware Registers)
** COPYRIGHT    : 	Copyright (c) 2006
**			Infineon Technologies AG
**			Am Campeon 1-12, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date        $Author         $Comment
**  4 AUG 2005  Xu Liang        Initiate Version
** 23 OCT 2006  Xu Liang        Add GPL header.
**  9 JAN 2007  Xu Liang        First version got from Anand (IC designer)
*******************************************************************************/



#ifndef IFXMIPS_ATM_FW_REGS_AR9_H
#define IFXMIPS_ATM_FW_REGS_AR9_H



/*
 *  Host-PPE Communication Data Address Mapping
 */
#define FW_VER_ID                       ((volatile struct fw_ver_id *)      SB_BUFFER(0x2001))
#define CFG_WRX_HTUTS                   SB_BUFFER(0x2400)   /*  WAN RX HTU Table Size, must be configured before enable PPE firmware.   */
#define CFG_WRX_QNUM                    SB_BUFFER(0x2401)   /*  WAN RX Queue Number */
#define CFG_WRX_DCHNUM                  SB_BUFFER(0x2402)   /*  WAN RX DMA Channel Number, no more than 8, must be configured before enable PPE firmware.   */
#define CFG_WTX_DCHNUM                  SB_BUFFER(0x2403)   /*  WAN TX DMA Channel Number, no more than 16, must be configured before enable PPE firmware.  */
#define CFG_WRDES_DELAY                 SB_BUFFER(0x2404)   /*  WAN Descriptor Write Delay, must be configured before enable PPE firmware.  */
#define WRX_DMACH_ON                    SB_BUFFER(0x2405)   /*  WAN RX DMA Channel Enable, must be configured before enable PPE firmware.   */
#define WTX_DMACH_ON                    SB_BUFFER(0x2406)   /*  WAN TX DMA Channel Enable, must be configured before enable PPE firmware.   */
#define WRX_HUNT_BITTH                  SB_BUFFER(0x2407)   /*  WAN RX HUNT Threshold, must be between 2 to 8.  */
#define WRX_QUEUE_CONFIG(i)             ((struct wrx_queue_config*)         SB_BUFFER(0x2500 + (i) * 20))
#define WRX_QUEUE_CONTEXT(i)            ((struct wrx_queue_context*)        SB_BUFFER(0x2504 + (i) * 20))
#define WRX_DMA_CHANNEL_CONFIG(i)       ((struct wrx_dma_channel_config*)   SB_BUFFER(0x2640 + (i) * 7))
#define WRX_DESC_CONTEXT(i)             ((struct wrx_desc_context*)         SB_BUFFER(0x2643 + (i) * 7))
#define WTX_PORT_CONFIG(i)              ((struct wtx_port_config*)          SB_BUFFER(0x2440 + (i)))
#define WTX_QUEUE_CONFIG(i)             ((struct wtx_queue_config*)         SB_BUFFER(0x3800 + (i) * 27))
#define WTX_DMA_CHANNEL_CONFIG(i)       ((struct wtx_dma_channel_config*)   SB_BUFFER(0x3801 + (i) * 27))
#define WAN_MIB_TABLE                   ((struct wan_mib_table*)            SB_BUFFER(0x2410))
#define HTU_ENTRY(i)                    ((struct htu_entry*)                SB_BUFFER(0x2010 + (i)))
#define HTU_MASK(i)                     ((struct htu_mask*)                 SB_BUFFER(0x2030 + (i)))
#define HTU_RESULT(i)                   ((struct htu_result*)               SB_BUFFER(0x2050 + (i)))

#if defined(ENABLE_ATM_RETX) && ENABLE_ATM_RETX

  #define RETX_MODE_CFG                     ((volatile struct Retx_mode_cfg *)      SB_BUFFER(0x2408))
  #define RETX_TSYNC_CFG                    ((volatile struct Retx_Tsync_cfg *)     SB_BUFFER(0x2409))
  #define RETX_TD_CFG                       ((volatile struct Retx_Td_cfg *)        SB_BUFFER(0x240A))
  #define RETX_MIB_TIMER_CFG                ((volatile struct Retx_MIB_Timer_cfg *) SB_BUFFER(0x240B))
  #define RETX_PLAYOUT_BUFFER_BASE          SB_BUFFER(0x240D)
  #define RETX_SERVICE_HEADER_CFG           SB_BUFFER(0x240E)
  #define RETX_MASK_HEADER_CFG              SB_BUFFER(0x240F)

  #define RETX_ADSL_PPE_INTF                ((volatile struct Retx_adsl_ppe_intf *) PPE_REG_ADDR(0x0D78))
  #define BAD_REC_RETX_ADSL_PPE_INTF        ((volatile struct Retx_adsl_ppe_intf *) SB_BUFFER(0x23AC))
  #define FIRST_BAD_REC_RETX_ADSL_PPE_INTF  ((volatile struct Retx_adsl_ppe_intf *) SB_BUFFER(0x23AE))

  #define PB_BUFFER_USAGE                   SB_BUFFER(0x2100)
  #define DTU_STAT_INFO                     ((volatile struct DTU_stat_info *)      SB_BUFFER(0x2180))
  #define DTU_VLD_STAT                      SB_BUFFER(0x2380)


  //=====================================================================
  // retx firmware mib, for debug purpose
  //      address : 0x2388 - 0x238F
  //      size    : 8
  //=====================================================================
  #define URETX_RX_TOTAL_DTU                    SB_BUFFER(0x2388)
  #define URETX_RX_BAD_DTU                      SB_BUFFER(0x2389)
  #define URETX_RX_GOOD_DTU                     SB_BUFFER(0x238A)
  #define URETX_RX_CORRECTED_DTU                SB_BUFFER(0x238B)
  #define URETX_RX_OUTOFDATE_DTU                SB_BUFFER(0x238C)
  #define URETX_RX_DUPLICATE_DTU                SB_BUFFER(0x238D)
  #define URETX_RX_TIMEOUT_DTU                  SB_BUFFER(0x238E)

  #define URETX_ALPHA_SWITCH_TO_HUNT_TIMES      SB_BUFFER(0x238F)

  // cell counter for debug purpose
  #define WRX_BC0_CELL_NUM                      SB_BUFFER(0x23E0)
  #define WRX_BC0_DROP_CELL_NUM                 SB_BUFFER(0x23E1)
  #define WRX_BC0_NONRETX_CELL_NUM              SB_BUFFER(0x23E2)
  #define WRX_BC0_RETX_CELL_NUM                 SB_BUFFER(0x23E3)
  #define WRX_BC0_OUTOFDATE_CELL_NUM            SB_BUFFER(0x23E4)
  #define WRX_BC0_DIRECTUP_NUM                  SB_BUFFER(0x23E5)
  #define WRX_BC0_PBW_TOTAL_NUM                 SB_BUFFER(0x23E6)
  #define WRX_BC0_PBW_SUCC_NUM                  SB_BUFFER(0x23E7)
  #define WRX_BC0_PBW_FAIL_NUM                  SB_BUFFER(0x23E8)
  #define WRX_BC1_CELL_NUM                      SB_BUFFER(0x23E9)

  // debug info (interface)

  #define DBG_DTU_INTF_WRPTR                    SB_BUFFER(0x2390)
  #define DBG_INTF_FCW_DUP_CNT                  SB_BUFFER(0x2391)
  #define DBG_INTF_SID_CHANGE_IN_DTU_CNT        SB_BUFFER(0x2392)
  #define DBG_INTF_LCW_DUP_CNT                  SB_BUFFER(0x2393)

  #define DBG_RFBI_DONE_INT_CNT                 SB_BUFFER(0x2394)
  #define DBG_DREG_BEG_END                      SB_BUFFER(0x2395)
  #define DBG_RFBI_BC0_INVALID_CNT              SB_BUFFER(0x2396)
  #define DBG_RFBI_LAST_T                       SB_BUFFER(0x2397)

  #define DBG_RFBI_INTV0                        SB_BUFFER(0x23EE)
  #define DBG_RFBI_INTV1                        SB_BUFFER(0x23EF)

  #define DBG_INTF_INFO(i)                      ((volatile struct Retx_adsl_ppe_intf_rec *) SB_BUFFER(0x23F0 + i))

  // Internal status
  #define URetx_curr_time                       SB_BUFFER(0x2398)
  #define URetx_sec_counter                     SB_BUFFER(0x2399)
  #define RxCURR_EFB                            SB_BUFFER(0x239A)
  #define RxDTURetransmittedCNT                 SB_BUFFER(0x239B)

  //=====================================================================
  // standardized MIB counter
  //      address : 0x239C - 0x239F
  //      size    : 4
  //=====================================================================
  #define RxLastEFBCNT                          SB_BUFFER(0x239C)
  #define RxDTUCorrectedCNT                     SB_BUFFER(0x239D)
  #define RxDTUCorruptedCNT                     SB_BUFFER(0x239E)
  #define RxRetxDTUUncorrectedCNT               SB_BUFFER(0x239F)


  //=====================================================================
  // General URetx Context
  //      address : 0x23A0 - 0x23AF
  //      size    : 16
  //=====================================================================
  #define NEXT_DTU_SID_OUT                      SB_BUFFER(0x23A0)
  #define LAST_DTU_SID_IN                       SB_BUFFER(0x23A1)
  #define NEXT_CELL_SID_OUT                     SB_BUFFER(0x23A2)
  #define ISR_CELL_ID                           SB_BUFFER(0x23A3)
  #define PB_CELL_SEARCH_IDX                    SB_BUFFER(0x23A4)
  #define PB_READ_PEND_FLAG                     SB_BUFFER(0x23A5)
  #define RFBI_FIRST_CW                         SB_BUFFER(0x23A6)
  #define RFBI_BAD_CW                           SB_BUFFER(0x23A7)
  #define RFBI_INVALID_CW                       SB_BUFFER(0x23A8)
  #define RFBI_RETX_CW                          SB_BUFFER(0x23A9)
  #define RFBI_CHK_DTU_STATUS                   SB_BUFFER(0x23AA)

  //=====================================================================
  // per PVC counter for RX error_pdu and correct_pdu
  //      address : 0x23B0 - 0x23CF
  //      size    : 32
  //=====================================================================
  #define WRX_PER_PVC_CORRECT_PDU_BASE          SB_BUFFER(0x23B0)
  #define WRX_PER_PVC_ERROR_PDU_BASE            SB_BUFFER(0x23C0)

  #define __WRXCTXT_L2_RdPtr(i)                 SB_BUFFER(0x2422 + (i))
  #define __WRXCTXT_L2Pages(i)                  SB_BUFFER(0x2424 + (i))

  #define __WTXCTXT_TC_WRPTR(i)                 SB_BUFFER(0x2450 + (i))
  #define __WRXCTXT_PortState(i)                SB_BUFFER(0x242A + (i))

#endif



#endif  //  IFXMIPS_ATM_FW_REGS_AR9_H
