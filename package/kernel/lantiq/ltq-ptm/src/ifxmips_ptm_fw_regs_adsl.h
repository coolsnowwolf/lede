/******************************************************************************
**
** FILE NAME    : ifxmips_ptm_fw_regs_adsl.h
** PROJECT      : UEIP
** MODULES      : PTM
**
** DATE         : 7 Jul 2009
** AUTHOR       : Xu Liang
** DESCRIPTION  : PTM driver header file (firmware register for ADSL)
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



#ifndef IFXMIPS_PTM_FW_REGS_ADSL_H
#define IFXMIPS_PTM_FW_REGS_ADSL_H



#if defined(CONFIG_DANUBE)
  #include "ifxmips_ptm_fw_regs_danube.h"
#elif defined(CONFIG_AMAZON_SE)
  #include "ifxmips_ptm_fw_regs_amazon_se.h"
#elif defined(CONFIG_AR9)
  #include "ifxmips_ptm_fw_regs_ar9.h"
#elif defined(CONFIG_VR9)
  #error VR9 is not ADSL PTM mode!
#else
  #error Platform is not specified!
#endif



/*
 *  MIB Table Maintained by Firmware
 */

struct wan_mib_table {
    unsigned int            wrx_correct_pdu;            /* 0 */
    unsigned int            wrx_correct_pdu_bytes;      /* 1 */
    unsigned int            wrx_tccrc_err_pdu;          /* 2 */
    unsigned int            wrx_tccrc_err_pdu_bytes;    /* 3 */
    unsigned int            wrx_ethcrc_err_pdu;         /* 4 */
    unsigned int            wrx_ethcrc_err_pdu_bytes;   /* 5 */
    unsigned int            wrx_nodesc_drop_pdu;        /* 6 */
    unsigned int            wrx_len_violation_drop_pdu; /* 7 */
    unsigned int            wrx_idle_bytes;             /* 8 */
    unsigned int            wrx_nonidle_cw;             /* 9 */
    unsigned int            wrx_idle_cw;                /* A */
    unsigned int            wrx_err_cw;                 /* B */
    unsigned int            wtx_total_pdu;              /* C */
    unsigned int            wtx_total_bytes;            /* D */
    unsigned int            res0;                       /* E */
    unsigned int            res1;                       /* F */
};


/*
 *  Host-PPE Communication Data Structure
 */

#if defined(__BIG_ENDIAN)

  struct fw_ver_id {
    unsigned int family         :4;
    unsigned int fwtype         :4;
    unsigned int interface      :4;
    unsigned int fwmode         :4;
    unsigned int major          :8;
    unsigned int minor          :8;
  };

  struct wrx_port_cfg_status {
    /* 0h */
    unsigned int mfs            :16;
    unsigned int res0           :12;
    unsigned int dmach          :3;
    unsigned int res1           :1;

    /* 1h */
    unsigned int res2           :14;
    unsigned int local_state    :2;     //  init with 0, written by firmware only
    unsigned int res3           :15;
    unsigned int partner_state  :1;     //  init with 0, written by firmware only

  };

  struct wrx_dma_channel_config {
    /*  0h  */
    unsigned int res3           :1;
    unsigned int res4           :2;
    unsigned int res5           :1;
    unsigned int desba          :28;
    /*  1h  */
    unsigned int res1           :16;
    unsigned int res2           :16;
    /*  2h  */
    unsigned int deslen         :16;
    unsigned int vlddes         :16;
  };

  struct wtx_port_cfg {
    /* 0h */
    unsigned int tx_cwth2       :8;
    unsigned int tx_cwth1       :8;
    unsigned int res0           :16;
  };

  struct wtx_dma_channel_config {
    /*  0h  */
    unsigned int res3           :1;
    unsigned int res4           :2;
    unsigned int res5           :1;
    unsigned int desba          :28;

    /*  1h  */
    unsigned int res1           :16;
    unsigned int res2           :16;

    /*  2h  */
    unsigned int deslen         :16;
    unsigned int vlddes         :16;
  };

  struct eth_efmtc_crc_cfg {
    /*  0h  */
    unsigned int res0               :6;
    unsigned int tx_eth_crc_gen     :1;
    unsigned int tx_tc_crc_gen      :1;
    unsigned int tx_tc_crc_len      :8;
    unsigned int res1               :5;
    unsigned int rx_eth_crc_present :1;
    unsigned int rx_eth_crc_check   :1;
    unsigned int rx_tc_crc_check    :1;
    unsigned int rx_tc_crc_len      :8;
  };

  /* DMA descriptor */
  struct rx_descriptor {
    /*  0 - 3h  */
    unsigned int own            :1;
    unsigned int c              :1;
    unsigned int sop            :1;
    unsigned int eop            :1;
    unsigned int res1           :3;
    unsigned int byteoff        :2;
    unsigned int res2           :2;
    unsigned int id             :4;
    unsigned int err            :1;
    unsigned int datalen        :16;
    /*  4 - 7h  */
    unsigned int res3           :4;
    unsigned int dataptr        :28;
  };

  struct tx_descriptor {
    /*  0 - 3h  */
    unsigned int own            :1;
    unsigned int c              :1;
    unsigned int sop            :1;
    unsigned int eop            :1;
    unsigned int byteoff        :5;
    unsigned int res1           :5;
    unsigned int iscell         :1;
    unsigned int clp            :1;
    unsigned int datalen        :16;
    /*  4 - 7h  */
    unsigned int res2           :4;
    unsigned int dataptr        :28;
  };

#else /* defined(__BIG_ENDIAN) */

  struct wrx_port_cfg_status {
    /* 0h */
    unsigned int res1           :1;
    unsigned int dmach          :3;
    unsigned int res0           :12;
    unsigned int mfs            :16;

    /* 1h */
    unsigned int partner_state  :1;
    unsigned int res3           :15;
    unsigned int local_state    :2;
    unsigned int res2           :14;
  };

  struct wrx_dma_channel_config {
    /*  0h  */
    unsigned int desba          :28;
    unsigned int res5           :1;
    unsigned int res4           :2;
    unsigned int res3           :1;
    /*  1h  */
    unsigned int res2           :16;
    unsigned int res1           :16;
    /*  2h  */
    unsigned int vlddes         :16;
    unsigned int deslen         :16;
  };

  struct wtx_port_cfg {
    /* 0h */
    unsigned int res0           :16;
    unsigned int tx_cwth1       :8;
    unsigned int tx_cwth2       :8;
  };

  struct wtx_dma_channel_config {
    /*  0h  */
    unsigned int desba          :28;
    unsigned int res5           :1;
    unsigned int res4           :2;
    unsigned int res3           :1;
    /*  1h  */
    unsigned int res2           :16;
    unsigned int res1           :16;
    /*  2h  */
    unsigned int vlddes         :16;
    unsigned int deslen         :16;
  };

  struct eth_efmtc_crc_cfg {
    /*  0h  */
    unsigned int rx_tc_crc_len      :8;
    unsigned int rx_tc_crc_check    :1;
    unsigned int rx_eth_crc_check   :1;
    unsigned int rx_eth_crc_present :1;
    unsigned int res1               :5;
    unsigned int tx_tc_crc_len      :8;
    unsigned int tx_tc_crc_gen      :1;
    unsigned int tx_eth_crc_gen     :1;
    unsigned int res0               :6;
  };

  /* DMA descriptor */
  struct rx_descriptor {
    /*  4 - 7h  */
    unsigned int dataptr        :28;
    unsigned int res3           :4;
    /*  0 - 3h  */
    unsigned int datalen        :16;
    unsigned int err            :1;
    unsigned int id             :4;
    unsigned int res2           :2;
    unsigned int byteoff        :2;
    unsigned int res1           :3;
    unsigned int eop            :1;
    unsigned int sop            :1;
    unsigned int c              :1;
    unsigned int own            :1;
  };

  struct tx_descriptor {
    /*  4 - 7h  */
    unsigned int dataptr        :28;
    unsigned int res2           :4;
    /*  0 - 3h  */
    unsigned int datalen        :16;
    unsigned int clp            :1;
    unsigned int iscell         :1;
    unsigned int res1           :5;
    unsigned int byteoff        :5;
    unsigned int eop            :1;
    unsigned int sop            :1;
    unsigned int c              :1;
    unsigned int own            :1;
  };
#endif  /* defined(__BIG_ENDIAN) */



#endif  //  IFXMIPS_PTM_FW_REGS_ADSL_H
