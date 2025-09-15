/******************************************************************************
**
** FILE NAME    : ifxmips_ptm_fw_regs_vdsl.h
** PROJECT      : UEIP
** MODULES      : PTM
**
** DATE         : 7 Jul 2009
** AUTHOR       : Xu Liang
** DESCRIPTION  : PTM driver header file (firmware register for VDSL)
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



#ifndef IFXMIPS_PTM_FW_REGS_VDSL_H
#define IFXMIPS_PTM_FW_REGS_VDSL_H



#if defined(CONFIG_DANUBE)
  #error Danube is not VDSL PTM mode!
#elif defined(CONFIG_AMAZON_SE)
  #error Amazon-SE is not VDSL PTM mode!
#elif defined(CONFIG_AR9)
  #error AR9 is not VDSL PTM mode!
#elif defined(CONFIG_VR9)
  #include "ifxmips_ptm_fw_regs_vr9.h"
#else
  #error Platform is not specified!
#endif



/*
 *  MIB Table Maintained by Firmware
 */

struct wan_rx_mib_table {
    unsigned int            res1[2];
    unsigned int            wrx_dropdes_pdu;
    unsigned int            wrx_total_bytes;
    unsigned int            res2[4];
    //  wrx_total_pdu is implemented with hardware counter (not used by PTM TC)
    //  check register "TC_RX_MIB_CMD"
    //  "HEC_INC" used to increase preemption Gamma interface (wrx_total_pdu)
    //  "AIIDLE_INC" used to increase normal Gamma interface (wrx_total_pdu)
};

struct wan_tx_mib_table {
    //unsigned int            wtx_total_pdu;        //  version before 0.26
    //unsigned int            small_pkt_drop_cnt;
    //unsigned int            total_pkt_drop_cnt;
    unsigned int            wrx_total_pdu;          //  version 0.26 and onwards
    unsigned int            wrx_total_bytes;
    unsigned int            wtx_total_pdu;
    unsigned int            wtx_total_bytes;

    unsigned int            wtx_cpu_dropsmall_pdu;
    unsigned int            wtx_cpu_dropdes_pdu;
    unsigned int            wtx_fast_dropsmall_pdu;
    unsigned int            wtx_fast_dropdes_pdu;
};


/*
 *  Host-PPE Communication Data Structure
 */

#if defined(__BIG_ENDIAN)

  struct fw_ver_id_new {    //  @2000
    /*  0 - 3h  */
    unsigned int family         :4;
    unsigned int package        :4;
    unsigned int major          :8;
    unsigned int middle         :8;
    unsigned int minor          :8;
    /*  4 - 7h  */
    unsigned int features       :32;
  };

  struct fw_ver_id {        //  @2001
    unsigned int family         :4;
    unsigned int fwtype         :4;
    unsigned int interface      :4;
    unsigned int fwmode         :4;
    unsigned int major          :8;
    unsigned int minor          :8;
  };

  struct cfg_std_data_len {
    unsigned int res1           :14;
    unsigned int byte_off       :2;     //  byte offset in RX DMA channel
    unsigned int data_len       :16;    //  data length for standard size packet buffer
  };

  struct tx_qos_cfg {
    unsigned int time_tick      :16;    //  number of PP32 cycles per basic time tick
    unsigned int overhd_bytes   :8;     //  number of overhead bytes per packet in rate shaping
    unsigned int eth1_eg_qnum   :4;     //  number of egress QoS queues (< 8);
    unsigned int eth1_burst_chk :1;     //  always 1, more accurate WFQ
    unsigned int eth1_qss       :1;     //  1: FW QoS, 0: HW QoS
    unsigned int shape_en       :1;     //  1: enable rate shaping, 0: disable
    unsigned int wfq_en         :1;     //  1: WFQ enabled, 0: strict priority enabled
  };

  struct psave_cfg {
    unsigned int res1           :15;
    unsigned int start_state    :1;     //  1: start from partial PPE reset, 0: start from full PPE reset
    unsigned int res2           :15;
    unsigned int sleep_en       :1;     //  1: enable sleep mode, 0: disable sleep mode
  };

  struct eg_bwctrl_cfg {
    unsigned int fdesc_wm       :16;    //  if free descriptors in QoS/Swap channel is less than this watermark, large size packets are discarded
    unsigned int class_len      :16;    //  if packet length is not less than this value, the packet is recognized as large packet
  };

  struct test_mode {
    unsigned int res1           :30;
    unsigned int mib_clear_mode :1;     //  1: MIB counter is cleared with TPS-TC software reset, 0: MIB counter not cleared
    unsigned int test_mode      :1;     //  1: test mode, 0: normal mode
  };

  struct gpio_mode {
    unsigned int res1           :3;
    unsigned int gpio_bit_bc1   :5;
    unsigned int res2           :3;
    unsigned int gpio_bit_bc0   :5;

    unsigned int res3           :7;
    unsigned int gpio_bc1_en    :1;

    unsigned int res4           :7;
    unsigned int gpio_bc0_en    :1;
  };

  struct gpio_wm_cfg {
    unsigned int stop_wm_bc1    :8;
    unsigned int start_wm_bc1   :8;
    unsigned int stop_wm_bc0    :8;
    unsigned int start_wm_bc0   :8;
  };

  struct rx_bc_cfg {
    unsigned int res1           :14;
    unsigned int local_state    :2;     //  0: local receiver is "Looking", 1: local receiver is "Freewheel Sync False", 2: local receiver is "Synced", 3: local receiver is "Freewheel Sync Truee"
    unsigned int res2           :15;
    unsigned int remote_state   :1;     //  0: remote receiver is "Out-of-Sync", 1: remote receiver is "Synced"
    unsigned int to_false_th    :16;    //  the number of consecutive "Miss Sync" for leaving "Freewheel Sync False" to "Looking" (default 3)
    unsigned int to_looking_th  :16;    //  the number of consecutive "Miss Sync" for leaving "Freewheel Sync True" to "Freewheel Sync False" (default 7)
    unsigned int res_word[30];
  };

  struct rx_gamma_itf_cfg {
    unsigned int res1           :31;
    unsigned int receive_state  :1;     //  0: "Out-of-Fragment", 1: "In-Fragment"
    unsigned int res2           :16;
    unsigned int rx_min_len     :8;     //  min length of packet, padding if packet length is smaller than this value
    unsigned int rx_pad_en      :1;     //  0:  padding disabled, 1: padding enabled
    unsigned int res3           :2;
    unsigned int rx_eth_fcs_ver_dis :1; //  0: ETH FCS verification is enabled, 1: disabled
    unsigned int rx_rm_eth_fcs      :1; //  0: ETH FCS field is not removed, 1: ETH FCS field is removed
    unsigned int rx_tc_crc_ver_dis  :1; //  0: TC CRC verification enabled, 1: disabled
    unsigned int rx_tc_crc_size     :2; //  0: 0-bit, 1: 16-bit, 2: 32-bit
    unsigned int rx_eth_fcs_result;     //  if the ETH FCS result matches this magic number, then the packet is valid packet
    unsigned int rx_tc_crc_result;      //  if the TC CRC result matches this magic number, then the packet is valid packet
    unsigned int rx_crc_cfg     :16;    //  TC CRC config, please check the description of SAR context data structure in the hardware spec
    unsigned int res4           :16;
    unsigned int rx_eth_fcs_init_value; //  ETH FCS initialization value
    unsigned int rx_tc_crc_init_value;  //  TC CRC initialization value
    unsigned int res_word1;
    unsigned int rx_max_len_sel :1;     //  0: normal, the max length is given by MAX_LEN_NORMAL, 1: fragment, the max length is given by MAX_LEN_FRAG
    unsigned int res5           :2;
    unsigned int rx_edit_num2   :4;     //  number of bytes to be inserted/removed
    unsigned int rx_edit_pos2   :7;     //  first byte position to be edited
    unsigned int rx_edit_type2  :1;     //  0: remove, 1: insert
    unsigned int rx_edit_en2    :1;     //  0: disable insertion or removal of data, 1: enable
    unsigned int res6           :3;
    unsigned int rx_edit_num1   :4;     //  number of bytes to be inserted/removed
    unsigned int rx_edit_pos1   :7;     //  first byte position to be edited
    unsigned int rx_edit_type1  :1;     //  0: remove, 1: insert
    unsigned int rx_edit_en1    :1;     //  0: disable insertion or removal of data, 1: enable
    unsigned int res_word2[2];
    unsigned int rx_inserted_bytes_1l;
    unsigned int rx_inserted_bytes_1h;
    unsigned int rx_inserted_bytes_2l;
    unsigned int rx_inserted_bytes_2h;
    int rx_len_adj;                     //  the packet length adjustment, it is sign integer
    unsigned int res_word3[16];
  };

  struct tx_bc_cfg {
    unsigned int fill_wm        :16;    //  default 2
    unsigned int uflw_wm        :16;    //  default 2
    unsigned int res_word[31];
  };

  struct tx_gamma_itf_cfg {
    unsigned int res_word1;
    unsigned int res1           :8;
    unsigned int tx_len_adj     :4;     //  4 * (not TX_ETH_FCS_GEN_DIS) + TX_TC_CRC_SIZE
    unsigned int tx_crc_off_adj :4;     //  4 + TX_TC_CRC_SIZE
    unsigned int tx_min_len     :8;     //  min length of packet, if length is less than this value, packet is padded
    unsigned int res2           :3;
    unsigned int tx_eth_fcs_gen_dis :1; //  0: ETH FCS generation enabled, 1: disabled
    unsigned int res3           :2;
    unsigned int tx_tc_crc_size :2;     //  0: 0-bit, 1: 16-bit, 2: 32-bit
    unsigned int res4           :24;
    unsigned int queue_mapping  :8;     //  TX queue attached to this Gamma interface
    unsigned int res_word2;
    unsigned int tx_crc_cfg     :16;    //  TC CRC config, please check the description of SAR context data structure in the hardware spec
    unsigned int res5           :16;
    unsigned int tx_eth_fcs_init_value; //  ETH FCS initialization value
    unsigned int tx_tc_crc_init_value;  //  TC CRC initialization value
    unsigned int res_word3[25];
  };

  struct wtx_qos_q_desc_cfg {
    unsigned int threshold      :8;
    unsigned int length         :8;
    unsigned int addr           :16;
    unsigned int rd_ptr         :16;
    unsigned int wr_ptr         :16;
  };

  struct wtx_eg_q_shaping_cfg {
    unsigned int t              :8;
    unsigned int w              :24;
    unsigned int s              :16;
    unsigned int r              :16;
    unsigned int res1           :8;
    unsigned int d              :24;    //  ppe internal variable
    unsigned int res2           :8;
    unsigned int tick_cnt       :8;     //  ppe internal variable
    unsigned int b              :16;    //  ppe internal variable
  };

  /* DMA descriptor */
  struct rx_descriptor {
    /*  0 - 3h  */
    unsigned int own            :1;     //  0: Central DMA TX or MIPS, 1: PPE
    unsigned int c              :1;     //  PPE tells current descriptor is complete
    unsigned int sop            :1;
    unsigned int eop            :1;
    unsigned int res1           :3;
    unsigned int byteoff        :2;
    unsigned int res2           :7;
    unsigned int datalen        :16;
    /*  4 - 7h  */
    unsigned int res3           :4;
    unsigned int dataptr        :28;    //  byte address
  };

  struct tx_descriptor {
    /*  0 - 3h  */
    unsigned int own            :1;     //  CPU path - 0: MIPS, 1: PPE Dispatcher, Fastpath - 0: PPE Dispatcher, 1: Central DMA, QoS Queue - 0: PPE Dispatcher, 1: PPE DMA, SWAP Channel - 0: MIPS, 1: PPE Dispatcher
    unsigned int c              :1;     //  MIPS or central DMA tells PPE the current descriptor is complete
    unsigned int sop            :1;
    unsigned int eop            :1;
    unsigned int byteoff        :5;
    unsigned int qid            :4;     //  TX Queue ID, bit 3 is reserved
    unsigned int res1           :3;
    unsigned int datalen        :16;
    /*  4 - 7h  */
    unsigned int small          :1;     //  0: standard size, 1: less than standard size
    unsigned int res2           :3;
    unsigned int dataptr        :28;    //  byte address
  };

#else /* defined(__BIG_ENDIAN) */
  #error structures are defined in big endian
#endif  /* defined(__BIG_ENDIAN) */



#endif  //  IFXMIPS_PTM_FW_REGS_VDSL_H

