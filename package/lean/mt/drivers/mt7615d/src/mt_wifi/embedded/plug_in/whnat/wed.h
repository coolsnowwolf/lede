/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name: wifi_offload
	wed.h
*/

#ifndef _WED_H
#define _WED_H

#include "woe_basic.h"


/*WED Token setting*/
#define WED_TOKEN_STATUS_INIT	1
#define WED_TOKEN_UNIT			128
#define WED_TOKEN_STATUS_UNIT	32
#define WED_TOKEN_RSV			0
/*buffer mgmet initial token cnt*/
/*max cr4 support token id, mt7622 can support up to 8192*/
#define WED_TOKEN_ID_MAX		4096

#ifdef WED_DYNAMIC_BM_SUPPORT
#define WED_TOKEN_EXPEND_SIZE	128
#define WED_TOKEN_LOW			2
#define WED_TOKEN_HIGH			(2*WED_TOKEN_LOW-1)
#endif /*WED_DYNAMIC_BM_SUPPORT*/
/*buffer manager token id range, start -> end*/
#define WED_TOKEN_START			WIFI_TX_TOKEN_CNT
#define WED_TOKEN_END			(WED_TOKEN_ID_MAX-1)

/*buffer mgmet can allocate max token number*/
#define WED_TOKEN_CNT_MAX		(WED_TOKEN_ID_MAX-WED_TOKEN_START)
#ifdef WED_DYNAMIC_BM_SUPPORT
#define WED_TOKEN_CNT			1152
#else
#define WED_TOKEN_CNT			WED_TOKEN_CNT_MAX
#endif

#define WED_PKT_NUM_GET(_wed) (_wed->res_ctrl.tx_ctrl.res.pkt_num)
#define WED_DLY_INT_VALUE 0xC014C014
#define WED_WDMA_RECYCLE_TIME 0xffff

#ifdef RT_BIG_ENDIAN
struct whnat_txdmad {
	/* Word 0 */
	unsigned int sdp0;

	/* Word 1 */
	unsigned int ddone:1;
	unsigned int last0:1;
	unsigned int sdl0:14;
	unsigned int burst:1;
	unsigned int last1:1;
	unsigned int sdlen1:14;
	/* Word 2 */
	unsigned int sdp1;
	/* Word 3*/
	unsigned int rsv:16;
	unsigned int winfo:16;
}  __packed;
#else
struct whnat_txdmad {
	/* Word	0 */
	unsigned int sdp0;
	/* Word	1 */
	unsigned int sdlen1:14;
	unsigned int last1:1;
	unsigned int burst:1;
	unsigned int sdl0:14;
	unsigned int last0:1;
	unsigned int ddone:1;
	/*Word2 */
	unsigned int sdp1;
	/* Word 3*/
	unsigned int winfo:16;
	unsigned int rsv:16;
}  __packed;
#endif /* RT_BIG_ENDIAN */


struct wed_token_info {
	unsigned int token_id;
	unsigned int len;
	unsigned int desc_len;
	unsigned int fd_len;
	dma_addr_t pkt_pa;
	dma_addr_t desc_pa;
	dma_addr_t fdesc_pa;
	void *pkt_va;
	void *desc_va;
	void *fdesc_va;
	struct sk_buff *pkt;
	struct list_head list;
};

struct wed_buf_res {
	unsigned int token_num;
	unsigned int dmad_len;
	unsigned int fd_len;
	unsigned int pkt_len;
	unsigned int pkt_num;
	struct whnat_dma_buf fbuf;
	struct whnat_dma_buf des_buf;
	struct list_head pkt_head;
};

struct wed_tx_ring_ctrl {
	unsigned int ring_num;
	unsigned int ring_len;
	unsigned int txd_len;
	struct whnat_dma_buf *desc;
	struct whnat_ring *ring;
};

struct wed_tx_ctrl {
	struct wed_buf_res res;
	struct wed_tx_ring_ctrl ring_ctrl;
};

struct wed_res_ctrl {
	struct wed_tx_ctrl tx_ctrl;
};

#ifdef ERR_RECOVERY
struct wed_ser_state {
	/*WED_TX_DMA*/
	unsigned int tx_dma_stat;
	unsigned int tx0_mib;
	unsigned int tx1_mib;
	unsigned int tx0_cidx;
	unsigned int tx0_didx;
	unsigned int tx1_cidx;
	unsigned int tx1_didx;
	/*WED_WDMA*/
	unsigned int wdma_stat;
	unsigned int wdma_rx0_mib;
	unsigned int wdma_rx1_mib;
	unsigned int wdma_rx0_recycle_mib;
	unsigned int wdma_rx1_recycle_mib;
	/*WED_WPDMA*/
	unsigned int wpdma_stat;
	unsigned int wpdma_tx0_mib;
	unsigned int wpdma_tx1_mib;
	/*WED_BM*/
	unsigned int bm_tx_stat;
	unsigned int txfree_to_bm_mib;
	unsigned int txbm_to_wdma_mib;
};

struct wed_ser_ctrl {
	struct task_struct *ser_task;
	struct wed_ser_state state;
	unsigned int tx_dma_err_cnt;
	unsigned int tx_dma_ser_cnt;
	unsigned int wdma_err_cnt;
	unsigned int tx_bm_err_cnt;
	unsigned int wdma_ser_cnt;
	unsigned int wpdma_idle_cnt;
	unsigned int wpdma_ser_cnt;
	unsigned int tx_bm_ser_cnt;
	unsigned int period_time;
	unsigned char recovery;
	spinlock_t ser_lock;
};
#endif /*ERR_RECOVERY*/

struct wed_entry {
	struct platform_device *pdev;
	unsigned long base_addr;
	unsigned int irq;
	unsigned int ext_int_mask;
#ifdef WED_DYNAMIC_BM_SUPPORT
	struct tasklet_struct tbuf_alloc_task;
	struct tasklet_struct tbuf_free_task;
#endif /*WED_DYNAMIC_BM_SUPPORT*/
#ifdef ERR_RECOVERY
	struct wed_ser_ctrl ser_ctrl;
#endif /*ERR_RECOVERY*/
	struct wed_res_ctrl res_ctrl;
	void *proc;
	void *whnat;
};



enum {
	WED_PROC_TX_RING_BASIC = 0,
	WED_PROC_TX_BUF_BASIC = 1,
	WED_PROC_TX_BUF_INFO = 2,
	WED_PROC_TX_RING_CELL = 3,
	WED_PROC_TX_RING_RAW = 4,
	WED_PROC_DBG_INFO = 5,
	WED_PROC_TX_DYNAMIC_FREE = 6,
	WED_PROC_TX_DYNAMIC_ALLOC = 7,
	WED_PROC_TX_FREE_CNT = 8,
	WED_PROC_TX_RESET = 9,
	WED_PROC_RX_RESET = 10,
	WED_PROC_END
};


int wed_init(struct platform_device *pdev, unsigned char idx, struct wed_entry *wed);
void wed_exit(struct platform_device *pdev, struct wed_entry *wed);
int wed_ring_init(struct wed_entry *entry);
void wed_ring_exit(struct wed_entry *entry);
int wed_ring_reset(struct wed_entry *entry);
int wed_token_buf_init(struct wed_entry *entry);
void wed_token_buf_exit(struct wed_entry *entry);
void wed_proc_handle(struct wed_entry *entry, char choice, char *arg);
#ifdef WED_DYNAMIC_BM_SUPPORT
int wed_token_buf_expend(struct wed_entry *entry, unsigned int grp_num);
void wed_token_buf_reduce(struct wed_entry *entry, unsigned int grp_num);
#endif /*WED_DYNAMIC_BM_SUPPORT*/

unsigned char wed_num_get(void);
unsigned char wed_slot_map_get(unsigned int idx);

void wed_eint_handle(struct wed_entry *wed, unsigned int status);

#define WED_TX_RING_GET(_wed, _idx) (&(_wed)->res_ctrl.tx_ctrl.ring_ctrl.ring[_idx])

void dump_wed_value(struct wed_entry *wed, char *name, unsigned int addr);

#ifdef ERR_RECOVERY
int wed_ser_init(struct wed_entry *wed);
void wed_ser_exit(struct wed_entry *wed);
void wed_ser_dump(struct wed_entry *wed);
#endif /*ERR_RECOVERY*/


#endif /*_WED_H*/
