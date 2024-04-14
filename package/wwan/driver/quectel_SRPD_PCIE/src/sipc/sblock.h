/*
 * Copyright (C) 2019 Spreadtrum Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __SBLOCK_H
#define __SBLOCK_H

/* flag for CMD/DONE msg type */
#define SMSG_CMD_SBLOCK_INIT		0x1
#define SMSG_DONE_SBLOCK_INIT		0x2

/* flag for EVENT msg type */
#define SMSG_EVENT_SBLOCK_SEND		0x1
#define SMSG_EVENT_SBLOCK_RELEASE	0x2

#define SBLOCK_STATE_IDLE		0
#define SBLOCK_STATE_READY		1

#define SBLOCK_BLK_STATE_DONE		0
#define SBLOCK_BLK_STATE_PENDING	1

struct sblock_blks {
	u32	addr;	/*phy address*/
	u32	length;
};

/* ring block header */
struct sblock_ring_header {
	/* get|send-block info */
	u32	txblk_addr;	/* tx blocks start addr */
	u32	txblk_count;	/* tx blocks num */
	u32	txblk_size;	/* one tx  block size */
	u32	txblk_blks;	/* tx_ring or tx_pool start addr */
	u32	txblk_rdptr;	/* tx_ring or tx_pool read point */
	u32	txblk_wrptr;	/* tx_ring or tx_pool write point */

	/* release|recv-block info */
	u32	rxblk_addr;
	u32	rxblk_count;
	u32	rxblk_size;
	u32	rxblk_blks;
	u32	rxblk_rdptr;
	u32	rxblk_wrptr;
};

struct sblock_header {
	struct sblock_ring_header ring;
	struct sblock_ring_header pool;
};

struct sblock_ring_header_op {
	/*
	 * this points  point to share memory
	 * for update rdptr and wtptr on share memory
	 */
	volatile u32	*tx_rd_p;
	volatile u32	*tx_wt_p;
	volatile u32	*rx_rd_p;
	volatile u32	*rx_wt_p;

	/*
	 * this member copy from share memory,
	 * because this contents will not change on  share memory
	 */
	u32	tx_addr;	/* txblk_addr */
	u32	tx_count;	/* txblk_count */
	u32	tx_size;	/* txblk_size */
	u32	tx_blks;	/* txblk_blks */
	u32	rx_addr;
	u32	rx_count;
	u32	rx_size;
	u32	rx_blks;
};

struct sblock_header_op {
	struct sblock_ring_header_op ringhd_op;
	struct sblock_ring_header_op poolhd_op;
};

struct sblock_ring {
	struct sblock_header	*header;
	struct sblock_header_op header_op;

	struct sprd_pms	*tx_pms;
	struct sprd_pms	*rx_pms;
	char	tx_pms_name[20];
	char	rx_pms_name[20];

	void	*txblk_virt; /* virt of header->txblk_addr */
	void	*rxblk_virt; /* virt of header->rxblk_addr */

	/* virt of header->ring->txblk_blks */
	struct sblock_blks	*r_txblks;
	/* virt of header->ring->rxblk_blks */
	struct sblock_blks	*r_rxblks;
	/* virt of header->pool->txblk_blks */
	struct sblock_blks	*p_txblks;
	/* virt of header->pool->rxblk_blks */
	struct sblock_blks	*p_rxblks;

	unsigned int	poll_mask;
	/* protect the poll_mask menber */
	spinlock_t	poll_lock;

	int	*txrecord; /* record the state of every txblk */
	int	*rxrecord; /* record the state of every rxblk */
	int	yell;	   /* need to notify cp */
	spinlock_t	r_txlock;  /* send */
	spinlock_t	r_rxlock;  /* recv */
	spinlock_t	p_txlock;  /* get */
	spinlock_t	p_rxlock;  /* release */

	wait_queue_head_t	getwait;
	wait_queue_head_t	recvwait;
};

struct sblock_mgr {
	u8	dst;
	u8	channel;
	int	pre_cfg; /*support in host mode only */
	u32	state;

	void	*smem_virt;
	u32	smem_addr;
	u32	smem_addr_debug;
	u32	smem_size;
	u32	dst_smem_addr;

	/*
	 * this address stored in share memory,
	 * be used to calculte the block virt address.
	 * in host mode, it is client physial address(dst_smem_addr),
	 * in client mode, it is own physial address(smem_addr).
	 */
	u32 stored_smem_addr;

	u32	txblksz;
	u32	rxblksz;
	u32	txblknum;
	u32	rxblknum;

	struct sblock_ring	*ring;
	struct task_struct	*thread;

	void	(*handler)(int event, void *data);
	void	*data;
};

#ifdef CONFIG_64BIT
#define SBLOCK_ALIGN_BYTES (8)
#else
#define SBLOCK_ALIGN_BYTES (4)
#endif

static inline u32 sblock_get_index(u32 x, u32 y)
{
	return (x / y);
}

static inline u32 sblock_get_ringpos(u32 x, u32 y)
{
	return is_power_of_2(y) ? (x & (y - 1)) : (x % y);
}
#endif
