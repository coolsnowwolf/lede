/******************************************************************************
**
** FILE NAME    : ifxmips_ptm_fw_regs_vr9.h
** PROJECT      : UEIP
** MODULES      : PTM
**
** DATE         : 7 Jul 2009
** AUTHOR       : Xu Liang
** DESCRIPTION  : PTM driver header file (firmware register for VR9)
** COPYRIGHT    :       Copyright (c) 2006
**                      Infineon Technologies AG
**                      Am Campeon 1-12, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date        $Author         $Comment
** 07 JUL 2009  Xu Liang        Init Version
*******************************************************************************/



#ifndef IFXMIPS_PTM_FW_REGS_VR9_H
#define IFXMIPS_PTM_FW_REGS_VR9_H



/*
 *  Host-PPE Communication Data Address Mapping
 */
#define FW_VER_ID                               ((volatile struct fw_ver_id *)              SB_BUFFER(0x2001))
#define CFG_STD_DATA_LEN                        ((volatile struct cfg_std_data_len *)       SB_BUFFER(0x2011))
#define TX_QOS_CFG                              ((volatile struct tx_qos_cfg *)             SB_BUFFER(0x2012))
#define EG_BWCTRL_CFG                           ((volatile struct eg_bwctrl_cfg *)          SB_BUFFER(0x2013))
#define PSAVE_CFG                               ((volatile struct psave_cfg *)              SB_BUFFER(0x2014))
#define GPIO_ADDR                               SB_BUFFER(0x2019)
#define GPIO_MODE                               ((volatile struct gpio_mode *)              SB_BUFFER(0x201C))
#define GPIO_WM_CFG                             ((volatile struct gpio_wm_cfg *)            SB_BUFFER(0x201D))
#define TEST_MODE                               ((volatile struct test_mode *)              SB_BUFFER(0x201F))
#define WTX_QOS_Q_DESC_CFG(i)                   ((volatile struct wtx_qos_q_desc_cfg *)     SB_BUFFER(0x2FF0 + (i) * 2))    /*  i < 8   */
#define WTX_EG_Q_PORT_SHAPING_CFG(i)            ((volatile struct wtx_eg_q_shaping_cfg *)   SB_BUFFER(0x2680 + (i) * 4))    /*  i < 1   */
#define WTX_EG_Q_SHAPING_CFG(i)                 ((volatile struct wtx_eg_q_shaping_cfg *)   SB_BUFFER(0x2684 + (i) * 4))    /*  i < 8   */
#define TX_QUEUE_CFG(i)                         WTX_EG_Q_PORT_SHAPING_CFG(i)    //  i < 9
#define RX_BC_CFG(i)                            ((volatile struct rx_bc_cfg *)              SB_BUFFER(0x3E80 + (i) * 0x20)) //  i < 2
#define TX_BC_CFG(i)                            ((volatile struct tx_bc_cfg *)              SB_BUFFER(0x3EC0 + (i) * 0x20)) //  i < 2
#define RX_GAMMA_ITF_CFG(i)                     ((volatile struct rx_gamma_itf_cfg *)       SB_BUFFER(0x3D80 + (i) * 0x20)) //  i < 4
#define TX_GAMMA_ITF_CFG(i)                     ((volatile struct tx_gamma_itf_cfg *)       SB_BUFFER(0x3E00 + (i) * 0x20)) //  i < 4
#define WAN_RX_MIB_TABLE(i)                     ((volatile struct wan_rx_mib_table *)       SB_BUFFER(0x5B00 + (i) * 8))    //  i < 4
#define WAN_TX_MIB_TABLE(i)                     ((volatile struct wan_tx_mib_table *)       SB_BUFFER(0x5B20 + (i) * 8))    //  i < 8
#define TX_CTRL_K_TABLE(i)                      SB_BUFFER(0x47F0 + (i)) //  i < 16
//  following MIB for debugging purpose
#define RECEIVE_NON_IDLE_CELL_CNT(i)            SB_BUFFER(5020 + (i))
#define RECEIVE_IDLE_CELL_CNT(i)                SB_BUFFER(5022 + (i))
#define TRANSMIT_CELL_CNT(i)                    SB_BUFFER(5024 + (i))
#define FP_RECEIVE_PKT_CNT                      SB_BUFFER(5026)

#define UTP_CFG                                 SB_BUFFER(0x2018)   //  bit 0~3 - 0x0F: in showtime, 0x00: not in showtime

/*
 *  Descriptor Base Address
 */
#define CPU_TO_WAN_TX_DESC_BASE                 ((volatile struct tx_descriptor *)SB_BUFFER(0x3D00))
#define __ETH_WAN_TX_QUEUE_NUM                  g_wanqos_en
#define __ETH_WAN_TX_QUEUE_LEN                  ((WAN_TX_DESC_NUM_TOTAL / __ETH_WAN_TX_QUEUE_NUM) < 256 ? (WAN_TX_DESC_NUM_TOTAL / __ETH_WAN_TX_QUEUE_NUM) : 255)
#define __ETH_WAN_TX_DESC_BASE(i)               (0x5C00 + (i) * 2 * __ETH_WAN_TX_QUEUE_LEN)
#define WAN_TX_DESC_BASE(i)                     ((volatile struct tx_descriptor *)SB_BUFFER(__ETH_WAN_TX_DESC_BASE(i))) //  i < __ETH_WAN_TX_QUEUE_NUM, __ETH_WAN_TX_QUEUE_LEN each queue
#define WAN_SWAP_DESC_BASE                      ((volatile struct tx_descriptor *)SB_BUFFER(0x2E80))
#define FASTPATH_TO_WAN_TX_DESC_BASE            ((volatile struct tx_descriptor *)SB_BUFFER(0x2580))
#define DMA_RX_CH1_DESC_BASE                    FASTPATH_TO_WAN_TX_DESC_BASE
#define WAN_RX_DESC_BASE                        ((volatile struct rx_descriptor *)SB_BUFFER(0x2600))
#define DMA_TX_CH1_DESC_BASE                    WAN_RX_DESC_BASE

/*
 *  Descriptor Number
 */
#define CPU_TO_WAN_TX_DESC_NUM                  64
#define WAN_TX_DESC_NUM                         __ETH_WAN_TX_QUEUE_LEN
#define WAN_SWAP_DESC_NUM                       64
#define WAN_TX_DESC_NUM_TOTAL                   512
#define FASTPATH_TO_WAN_TX_DESC_NUM             64
#define DMA_RX_CH1_DESC_NUM                     FASTPATH_TO_WAN_TX_DESC_NUM
#define WAN_RX_DESC_NUM                         64
#define DMA_TX_CH1_DESC_NUM                     WAN_RX_DESC_NUM



#endif  //  IFXMIPS_PTM_FW_REGS_VR9_H
