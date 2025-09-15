/******************************************************************************
**
** FILE NAME    : ifxmips_atm_fw_regs_common.h
** PROJECT      : UEIP
** MODULES     	: ATM (ADSL)
**
** DATE         : 1 AUG 2005
** AUTHOR       : Xu Liang
** DESCRIPTION  : ATM Driver (Firmware Register Structures)
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



#ifndef IFXMIPS_ATM_FW_REGS_COMMON_H
#define IFXMIPS_ATM_FW_REGS_COMMON_H


#if defined(CONFIG_DANUBE)
  #include "ifxmips_atm_fw_regs_danube.h"
#elif defined(CONFIG_AMAZON_SE)
  #include "ifxmips_atm_fw_regs_amazon_se.h"
#elif defined(CONFIG_AR9)
  #include "ifxmips_atm_fw_regs_ar9.h"
#elif defined(CONFIG_VR9)
  #include "ifxmips_atm_fw_regs_vr9.h"
#else
  #error Platform is not specified!
#endif



/*
 *  PPE ATM Cell Header
 */
#if defined(__BIG_ENDIAN)
    struct uni_cell_header {
        unsigned int        gfc     :4;
        unsigned int        vpi     :8;
        unsigned int        vci     :16;
        unsigned int        pti     :3;
        unsigned int        clp     :1;
    };
#else
    struct uni_cell_header {
        unsigned int        clp     :1;
        unsigned int        pti     :3;
        unsigned int        vci     :16;
        unsigned int        vpi     :8;
        unsigned int        gfc     :4;
    };
#endif  //  defined(__BIG_ENDIAN)

/*
 *  Inband Header and Trailer
 */
#if defined(__BIG_ENDIAN)
    struct rx_inband_trailer {
        /*  0 - 3h  */
        unsigned int        uu      :8;
        unsigned int        cpi     :8;
        unsigned int        stw_res1:4;
        unsigned int        stw_clp :1;
        unsigned int        stw_ec  :1;
        unsigned int        stw_uu  :1;
        unsigned int        stw_cpi :1;
        unsigned int        stw_ovz :1;
        unsigned int        stw_mfl :1;
        unsigned int        stw_usz :1;
        unsigned int        stw_crc :1;
        unsigned int        stw_il  :1;
        unsigned int        stw_ra  :1;
        unsigned int        stw_res2:2;
        /*  4 - 7h  */
        unsigned int        gfc     :4;
        unsigned int        vpi     :8;
        unsigned int        vci     :16;
        unsigned int        pti     :3;
        unsigned int        clp     :1;
    };

    struct tx_inband_header {
        /*  0 - 3h  */
        unsigned int        gfc     :4;
        unsigned int        vpi     :8;
        unsigned int        vci     :16;
        unsigned int        pti     :3;
        unsigned int        clp     :1;
        /*  4 - 7h  */
        unsigned int        uu      :8;
        unsigned int        cpi     :8;
        unsigned int        pad     :8;
        unsigned int        res1    :8;
    };
#else
    struct rx_inband_trailer {
        /*  0 - 3h  */
        unsigned int        stw_res2:2;
        unsigned int        stw_ra  :1;
        unsigned int        stw_il  :1;
        unsigned int        stw_crc :1;
        unsigned int        stw_usz :1;
        unsigned int        stw_mfl :1;
        unsigned int        stw_ovz :1;
        unsigned int        stw_cpi :1;
        unsigned int        stw_uu  :1;
        unsigned int        stw_ec  :1;
        unsigned int        stw_clp :1;
        unsigned int        stw_res1:4;
        unsigned int        cpi     :8;
        unsigned int        uu      :8;
        /*  4 - 7h  */
        unsigned int        clp     :1;
        unsigned int        pti     :3;
        unsigned int        vci     :16;
        unsigned int        vpi     :8;
        unsigned int        gfc     :4;
    };

    struct tx_inband_header {
        /*  0 - 3h  */
        unsigned int        clp     :1;
        unsigned int        pti     :3;
        unsigned int        vci     :16;
        unsigned int        vpi     :8;
        unsigned int        gfc     :4;
        /*  4 - 7h  */
        unsigned int        res1    :8;
        unsigned int        pad     :8;
        unsigned int        cpi     :8;
        unsigned int        uu      :8;
    };
#endif  //  defined(__BIG_ENDIAN)

/*
 *  MIB Table Maintained by Firmware
 */
struct wan_mib_table {
    u32                     res1;
    u32                     wrx_drophtu_cell;
    u32                     wrx_dropdes_pdu;
    u32                     wrx_correct_pdu;
    u32                     wrx_err_pdu;
    u32                     wrx_dropdes_cell;
    u32                     wrx_correct_cell;
    u32                     wrx_err_cell;
    u32                     wrx_total_byte;
    u32                     res2;
    u32                     wtx_total_pdu;
    u32                     wtx_total_cell;
    u32                     wtx_total_byte;
};

/*
 *  Host-PPE Communication Data Structure
 */

#if defined(__BIG_ENDIAN)
    struct fw_ver_id {
        unsigned int    family      :4;
        unsigned int    fwtype      :4;
        unsigned int    interface   :4;
        unsigned int    fwmode      :4;
        unsigned int    major       :8;
        unsigned int    minor       :8;
    };

    struct wrx_queue_config {
        /*  0h  */
        unsigned int    res2        :27;
        unsigned int    dmach       :4;
        unsigned int    errdp       :1;
        /*  1h  */
        unsigned int    oversize    :16;
        unsigned int    undersize   :16;
        /*  2h  */
        unsigned int    res1        :16;
        unsigned int    mfs         :16;
        /*  3h  */
        unsigned int    uumask      :8;
        unsigned int    cpimask     :8;
        unsigned int    uuexp       :8;
        unsigned int    cpiexp      :8;
    };

    struct wrx_queue_context {
        /*  0h  */
        unsigned int    curr_len    :16;
        unsigned int    res0        :12;
        unsigned int    mfs         :1;
        unsigned int    ec          :1;
        unsigned int    clp1        :1;
        unsigned int    aal5dp      :1;

        /*  1h  */
        unsigned int    intcrc;

        /*  2h, 3h  */
        unsigned int    curr_des0;
        unsigned int    curr_des1;

        /*  4h - 0xE    */
        unsigned int    res1[11];

        unsigned int    last_dword;
    };

    struct wtx_port_config {
        unsigned int    res1        :27;
        unsigned int    qid         :4;
        unsigned int    qsben       :1;
    };

    struct wtx_queue_config {
        unsigned int    res1        :16;
        unsigned int    same_vc_qmap:8;
        unsigned int    res2        :1;
        unsigned int    sbid        :1;
        unsigned int    qsb_vcid    :4; //  Which QSB queue (VCID) does this TX queue map to.
        unsigned int    res3        :1;
        unsigned int    qsben       :1;
    };

    struct wrx_desc_context {
        unsigned int dmach_wrptr    : 16;
        unsigned int dmach_rdptr    : 16;

        unsigned int res0           : 16;
        unsigned int dmach_fcnt     : 16;

        unsigned int res1           : 11;
        unsigned int desbuf_wrptr   : 5;
        unsigned int res2           : 11;
        unsigned int desbuf_rdptr   : 5;

        unsigned int res3           : 27;
        unsigned int desbuf_vcnt    : 5;
    };

    struct wrx_dma_channel_config {
        /*  0h  */
        unsigned int    res1        :1;
        unsigned int    mode        :2;
        unsigned int    rlcfg       :1;
        unsigned int    desba       :28;
        /*  1h  */
        unsigned int    chrl        :16;
        unsigned int    clp1th      :16;
        /*  2h  */
        unsigned int    deslen      :16;
        unsigned int    vlddes      :16;
    };

    struct wtx_dma_channel_config {
        /*  0h  */
        unsigned int    res2        :1;
        unsigned int    mode        :2;
        unsigned int    res3        :1;
        unsigned int    desba       :28;
        /*  1h  */
        unsigned int    res1        :32;
        /*  2h  */
        unsigned int    deslen      :16;
        unsigned int    vlddes      :16;
    };

    struct htu_entry {
        unsigned int    res1        :1;
        unsigned int    clp         :1;
        unsigned int    pid         :2;
        unsigned int    vpi         :8;
        unsigned int    vci         :16;
        unsigned int    pti         :3;
        unsigned int    vld         :1;
    };

    struct htu_mask {
        unsigned int    set         :1;
        unsigned int    clp         :1;
        unsigned int    pid_mask    :2;
        unsigned int    vpi_mask    :8;
        unsigned int    vci_mask    :16;
        unsigned int    pti_mask    :3;
        unsigned int    clear       :1;
    };

   struct htu_result {
        unsigned int    res1        :12;
        unsigned int    cellid      :4;
        unsigned int    res2        :5;
        unsigned int    type        :1;
        unsigned int    ven         :1;
        unsigned int    res3        :5;
        unsigned int    qid         :4;
    };

    struct rx_descriptor {
        /*  0 - 3h  */
        unsigned int    own         :1;
        unsigned int    c           :1;
        unsigned int    sop         :1;
        unsigned int    eop         :1;
        unsigned int    res1        :3;
        unsigned int    byteoff     :2;
        unsigned int    res2        :2;
        unsigned int    id          :4;
        unsigned int    err         :1;
        unsigned int    datalen     :16;
        /*  4 - 7h  */
        unsigned int    res3        :4;
        unsigned int    dataptr     :28;
    };

    struct tx_descriptor {
        /*  0 - 3h  */
        unsigned int    own         :1;
        unsigned int    c           :1;
        unsigned int    sop         :1;
        unsigned int    eop         :1;
        unsigned int    byteoff     :5;
        unsigned int    res1        :5;
        unsigned int    iscell      :1;
        unsigned int    clp         :1;
        unsigned int    datalen     :16;
        /*  4 - 7h  */
        unsigned int    res2        :4;
        unsigned int    dataptr     :28;
    };
#else
    struct wrx_queue_config {
        /*  0h  */
        unsigned int    errdp       :1;
        unsigned int    dmach       :4;
        unsigned int    res2        :27;
        /*  1h  */
        unsigned int    undersize   :16;
        unsigned int    oversize    :16;
        /*  2h  */
        unsigned int    mfs         :16;
        unsigned int    res1        :16;
        /*  3h  */
        unsigned int    cpiexp      :8;
        unsigned int    uuexp       :8;
        unsigned int    cpimask     :8;
        unsigned int    uumask      :8;
    };

    struct wtx_port_config {
        unsigned int    qsben       :1;
        unsigned int    qid         :4;
        unsigned int    res1        :27;
    };

    struct wtx_queue_config {
        unsigned int    qsben       :1;
        unsigned int    res3        :1;
        unsigned int    qsb_vcid    :4; //  Which QSB queue (VCID) does this TX queue map to.
        unsigned int    sbid        :1;
        unsigned int    res2        :1;
        unsigned int    same_vc_qmap:8;
        unsigned int    res1        :16;
    };

    struct wrx_dma_channel_config
    {
        /*  0h  */
        unsigned int    desba       :28;
        unsigned int    rlcfg       :1;
        unsigned int    mode        :2;
        unsigned int    res1        :1;
        /*  1h  */
        unsigned int    clp1th      :16;
        unsigned int    chrl        :16;
        /*  2h  */
        unsigned int    vlddes      :16;
        unsigned int    deslen      :16;
    };

    struct wtx_dma_channel_config {
        /*  0h  */
        unsigned int    desba       :28;
        unsigned int    res3        :1;
        unsigned int    mode        :2;
        unsigned int    res2        :1;
        /*  1h  */
        unsigned int    res1        :32;
        /*  2h  */
        unsigned int    vlddes      :16;
        unsigned int    deslen      :16;
    };

    struct rx_descriptor {
        /*  4 - 7h  */
        unsigned int    dataptr     :28;
        unsigned int    res3        :4;
        /*  0 - 3h  */
        unsigned int    datalen     :16;
        unsigned int    err         :1;
        unsigned int    id          :4;
        unsigned int    res2        :2;
        unsigned int    byteoff     :2;
        unsigned int    res1        :3;
        unsigned int    eop         :1;
        unsigned int    sop         :1;
        unsigned int    c           :1;
        unsigned int    own         :1;
    };

    struct tx_descriptor {
        /*  4 - 7h  */
        unsigned int    dataptr     :28;
        unsigned int    res2        :4;
        /*  0 - 3h  */
        unsigned int    datalen     :16;
        unsigned int    clp         :1;
        unsigned int    iscell      :1;
        unsigned int    res1        :5;
        unsigned int    byteoff     :5;
        unsigned int    eop         :1;
        unsigned int    sop         :1;
        unsigned int    c           :1;
        unsigned int    own         :1;
    };
#endif  //  defined(__BIG_ENDIAN)

#if defined(ENABLE_ATM_RETX) && ENABLE_ATM_RETX
  #if defined(__BIG_ENDIAN)

    struct Retx_adsl_ppe_intf {
        unsigned int res0_0             : 16;
        unsigned int dtu_sid            : 8;
        unsigned int dtu_timestamp      : 8;

        unsigned int res1_0             : 16;
        unsigned int local_time         : 8;
        unsigned int res1_1             : 5;
        unsigned int is_last_cw         : 1;
        unsigned int reinit_flag        : 1;
        unsigned int is_bad_cw          : 1;
    };

    struct Retx_adsl_ppe_intf_rec {

        unsigned int local_time         : 8;
        unsigned int res1_1             : 5;
        unsigned int is_last_cw         : 1;
        unsigned int reinit_flag        : 1;
        unsigned int is_bad_cw          : 1;

        unsigned int dtu_sid            : 8;
        unsigned int dtu_timestamp      : 8;

    };

    struct Retx_mode_cfg {
        unsigned int    res0            :8;
        unsigned int    invld_range     :8;     //  used for rejecting the too late arrival of the retransmitted DTU
        unsigned int    buff_size       :8;     //  the total number of cells in playout buffer is 32 * buff_size
        unsigned int    res1            :7;
        unsigned int    retx_en         :1;
      };

    struct Retx_Tsync_cfg {
        unsigned int    fw_alpha        :16;    //  number of consecutive HEC error cell causes that the cell delineation state machine transit from SYNC to HUNT (0 means never)
        unsigned int    sync_inp        :16;    //  reserved
    };

    struct Retx_Td_cfg {
        unsigned int    res0            :8;
        unsigned int    td_max          :8;    //  maximum delay between the time a DTU is first created at transmitter and the time the DTU is sent out of ReTX layer at receiver
        unsigned int    res1            :8;
        unsigned int    td_min          :8;     //  minimum delay between the time a DTU is first created at transmitter and the time the DTU is sent out of ReTX layer at receiver
    };

    struct Retx_MIB_Timer_cfg {
        unsigned int    ticks_per_sec   : 16;
        unsigned int    tick_cycle      : 16;
    };

    struct DTU_stat_info {
        unsigned int complete           : 1;
        unsigned int bad                : 1;
        unsigned int res0_0             : 14;
        unsigned int time_stamp         : 8;
        unsigned int cell_cnt           : 8;

        unsigned int dtu_rd_ptr         : 16;
        unsigned int dtu_wr_ptr         : 16;
    };

    struct Retx_ctrl_field {
        unsigned int res0               : 1;

        unsigned int l2_drop            : 1;
        unsigned int res1               : 13;
        unsigned int retx               : 1;

        unsigned int dtu_sid            : 8;
        unsigned int cell_sid           : 8;
    };

  #else
    #error Little Endian is not supported yet.
  #endif

  struct dsl_param {
    unsigned int    update_flag;            //  00
    unsigned int    res0;                   //  04
    unsigned int    MinDelayrt;             //  08
    unsigned int    MaxDelayrt;             //  0C
    unsigned int    res1;                   //  10
    unsigned int    res2;                   //  14
    unsigned int    RetxEnable;             //  18
    unsigned int    ServiceSpecificReTx;    //  1C
    unsigned int    res3;                   //  20
    unsigned int    ReTxPVC;                //  24
    unsigned int    res4;                   //  28
    unsigned int    res5;                   //  2C
    unsigned int    res6;                   //  30
    unsigned int    res7;                   //  34
    unsigned int    res8;                   //  38
    unsigned int    res9;                   //  3C
    unsigned int    res10;                  //  40
    unsigned int    res11;                  //  44
    unsigned int    res12;                  //  48
    unsigned int    res13;                  //  4C
    unsigned int    RxDtuCorruptedCNT;      //  50
    unsigned int    RxRetxDtuUnCorrectedCNT;//  54
    unsigned int    RxLastEFB;              //  58
    unsigned int    RxDtuCorrectedCNT;      //  5C
  };
#endif



#endif  //  IFXMIPS_ATM_FW_REGS_COMMON_H
