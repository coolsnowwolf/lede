/******************************************************************************
**
** FILE NAME    : ifxmips_atm_core.h
** PROJECT      : UEIP
** MODULES      : ATM
**
** DATE         : 7 Jul 2009
** AUTHOR       : Xu Liang
** DESCRIPTION  : ATM driver header file (core functions)
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
** 17 JUN 2009  Xu Liang        Init Version
*******************************************************************************/

#ifndef IFXMIPS_ATM_CORE_H
#define IFXMIPS_ATM_CORE_H


#define INT_NUM_IM2_IRL24	(INT_NUM_IM2_IRL0 + 24)
#define INT_NUM_IM2_IRL13	(INT_NUM_IM2_IRL0 + 13)
#define CONFIG_IFXMIPS_DSL_CPE_MEI
#define IFX_REG_W32(_v, _r)               __raw_writel((_v), (volatile unsigned int *)(_r))
#define IFX_REG_R32(_r)                    __raw_readl((volatile unsigned int *)(_r))
#define IFX_REG_W32_MASK(_clr, _set, _r)   IFX_REG_W32((IFX_REG_R32((_r)) & ~(_clr)) | (_set), (_r))
#define SET_BITS(x, msb, lsb, value)    (((x) & ~(((1 << ((msb) + 1)) - 1) ^ ((1 << (lsb)) - 1))) | (((value) & ((1 << (1 + (msb) - (lsb))) - 1)) << (lsb)))

struct ltq_atm_ops {
	void (*init)(void);
	void (*shutdown)(void);

	int (*start)(int pp32);
	void (*stop)(int pp32);

	void (*fw_ver)(unsigned int *major, unsigned int *minor);
};

#include <linux/atomic.h>
#include <lantiq_atm.h>

/*
 * ####################################
 *              Definition
 * ####################################
 */

/*
 *  Compile Options
 */

#define ENABLE_DEBUG                    1

#define ENABLE_ASSERT                   1

#define INLINE

#define DEBUG_DUMP_SKB                  1

#define DEBUG_QOS                       1

#define DISABLE_QOS_WORKAROUND          0

#define ENABLE_DBG_PROC                 1

#define ENABLE_FW_PROC                  1

#ifdef CONFIG_IFX_ATM_TASKLET
  #define ENABLE_TASKLET                1
#endif

#ifdef CONFIG_IFX_ATM_RETX
  #define ENABLE_ATM_RETX               1
#endif

#if defined(CONFIG_DSL_MEI_CPE_DRV) && !defined(CONFIG_IFXMIPS_DSL_CPE_MEI)
  #define CONFIG_IFXMIPS_DSL_CPE_MEI    1
#endif

/*
 *  Debug/Assert/Error Message
 */

#define ifx_atm_dbg_enable 1

#define DBG_ENABLE_MASK_ERR             (1 << 0)
#define DBG_ENABLE_MASK_DEBUG_PRINT     (1 << 1)
#define DBG_ENABLE_MASK_ASSERT          (1 << 2)
#define DBG_ENABLE_MASK_DUMP_SKB_RX     (1 << 8)
#define DBG_ENABLE_MASK_DUMP_SKB_TX     (1 << 9)
#define DBG_ENABLE_MASK_DUMP_QOS        (1 << 10)
#define DBG_ENABLE_MASK_DUMP_INIT       (1 << 11)
#define DBG_ENABLE_MASK_MAC_SWAP        (1 << 12)
#define DBG_ENABLE_MASK_ALL             (DBG_ENABLE_MASK_ERR | DBG_ENABLE_MASK_DEBUG_PRINT | DBG_ENABLE_MASK_ASSERT | DBG_ENABLE_MASK_DUMP_SKB_RX | DBG_ENABLE_MASK_DUMP_SKB_TX | DBG_ENABLE_MASK_DUMP_QOS | DBG_ENABLE_MASK_DUMP_INIT | DBG_ENABLE_MASK_MAC_SWAP)

#if defined(ENABLE_ASSERT) && ENABLE_ASSERT
  #define ASSERT(cond, format, arg...)  do { if ( (ifx_atm_dbg_enable & DBG_ENABLE_MASK_ASSERT) && !(cond) ) printk(KERN_ERR __FILE__ ":%d:%s: " format "\n", __LINE__, __FUNCTION__, ##arg); } while ( 0 )
#else
  #define ASSERT(cond, format, arg...)
#endif


/*
 *  Constants
 */
#define DEFAULT_TX_LINK_RATE            3200    //  in cells

/*
 *  ATM Port, QSB Queue, DMA RX/TX Channel Parameters
 */
#define ATM_PORT_NUMBER                 2
#define MAX_QUEUE_NUMBER                16
#define OAM_RX_QUEUE                    15
#define QSB_RESERVE_TX_QUEUE            0
#define FIRST_QSB_QID                   1
#define MAX_PVC_NUMBER                  (MAX_QUEUE_NUMBER - FIRST_QSB_QID)
#define MAX_RX_DMA_CHANNEL_NUMBER       8
#define MAX_TX_DMA_CHANNEL_NUMBER       16
#define DATA_BUFFER_ALIGNMENT           EMA_ALIGNMENT
#define DESC_ALIGNMENT                  8
#define DEFAULT_RX_HUNT_BITTH           4

/*
 *  RX DMA Channel Allocation
 */
#define RX_DMA_CH_OAM                   0
#define RX_DMA_CH_AAL                   1
#define RX_DMA_CH_TOTAL                 2
#define RX_DMA_CH_OAM_DESC_LEN          32
#define RX_DMA_CH_OAM_BUF_SIZE          ((CELL_SIZE + 14) & ~15)
#define RX_DMA_CH_AAL_BUF_SIZE          (2048 - 48)

/*
 *  OAM Constants
 */
#define OAM_HTU_ENTRY_NUMBER            3
#define OAM_F4_SEG_HTU_ENTRY            0
#define OAM_F4_TOT_HTU_ENTRY            1
#define OAM_F5_HTU_ENTRY                2
#define OAM_F4_CELL_ID                  0
#define OAM_F5_CELL_ID                  15
#if defined(ENABLE_ATM_RETX) && ENABLE_ATM_RETX
  #undef  OAM_HTU_ENTRY_NUMBER
  #define OAM_HTU_ENTRY_NUMBER          4
  #define OAM_ARQ_HTU_ENTRY             3
#endif

/*
 *  RX Frame Definitions
 */
#define MAX_RX_PACKET_ALIGN_BYTES       3
#define MAX_RX_PACKET_PADDING_BYTES     3
#define RX_INBAND_TRAILER_LENGTH        8
#define MAX_RX_FRAME_EXTRA_BYTES        (RX_INBAND_TRAILER_LENGTH + MAX_RX_PACKET_ALIGN_BYTES + MAX_RX_PACKET_PADDING_BYTES)

/*
 *  TX Frame Definitions
 */
#define MAX_TX_HEADER_ALIGN_BYTES       12
#define MAX_TX_PACKET_ALIGN_BYTES       3
#define MAX_TX_PACKET_PADDING_BYTES     3
#define TX_INBAND_HEADER_LENGTH         8
#define MAX_TX_FRAME_EXTRA_BYTES        (TX_INBAND_HEADER_LENGTH + MAX_TX_HEADER_ALIGN_BYTES + MAX_TX_PACKET_ALIGN_BYTES + MAX_TX_PACKET_PADDING_BYTES)

#define CELL_SIZE                       ATM_AAL0_SDU

#if defined(ENABLE_ATM_RETX) && ENABLE_ATM_RETX
  #define RETX_PLAYOUT_BUFFER_ORDER     6
  #define RETX_PLAYOUT_BUFFER_SIZE      (PAGE_SIZE * (1 << RETX_PLAYOUT_BUFFER_ORDER))
  #define RETX_PLAYOUT_FW_BUFF_SIZE     (RETX_PLAYOUT_BUFFER_SIZE / (32 * 56 /* cell size */))
  #define RETX_POLLING_INTERVAL         (HZ / 100 > 0 ? HZ / 100 : 1)
#endif

typedef struct {
	unsigned int h;
	unsigned int l;
} ppe_u64_t;

struct port {
	unsigned int tx_max_cell_rate;
	unsigned int tx_current_cell_rate;

	struct atm_dev *dev;
};

struct connection {
	struct atm_vcc         *vcc;

	volatile struct tx_descriptor *tx_desc;
	unsigned int tx_desc_pos;
	struct sk_buff **tx_skb;
	spinlock_t lock;

	unsigned int aal5_vcc_crc_err; /* number of packets with CRC error */
	unsigned int aal5_vcc_oversize_sdu; /* number of packets with oversize error */

	unsigned int port;
};

struct atm_priv_data {
	unsigned long conn_table;
	struct connection conn[MAX_PVC_NUMBER];

	volatile struct rx_descriptor *aal_desc;
	unsigned int aal_desc_pos;

	volatile struct rx_descriptor *oam_desc;
	unsigned char *oam_buf;
	unsigned int oam_desc_pos;

	struct port port[ATM_PORT_NUMBER];

	unsigned int wrx_pdu;        /*  successfully received AAL5 packet       */
	unsigned int wrx_drop_pdu;   /*  AAL5 packet dropped by driver on RX     */
	unsigned int wtx_pdu;        /*  successfully transmitted AAL5 packet    */
	unsigned int wtx_err_pdu;    /*  error AAL5 packet                       */
	unsigned int wtx_drop_pdu;   /*  AAL5 packet dropped by driver on TX     */

	unsigned int wrx_oam;        /*  successfully received OAM cell          */
	unsigned int wrx_drop_oam;   /*  OAM cell dropped by driver on RX        */
	unsigned int wtx_oam;        /*  successfully transmitted OAM cell       */
	unsigned int wtx_err_oam;    /*  error during transmiting OAM cell       */
	unsigned int wtx_drop_oam;   /*  OAM cell dropped by driver on TX        */

	ppe_u64_t wrx_total_byte;
	ppe_u64_t wtx_total_byte;
	unsigned int prev_wrx_total_byte;
	unsigned int prev_wtx_total_byte;

	void *aal_desc_base;
	void *oam_desc_base;
	void *oam_buf_base;
	void *tx_desc_base;
	void *tx_skb_base;
};

#include "ifxmips_atm_ppe_common.h"
#include "ifxmips_atm_fw_regs_common.h"

#endif
