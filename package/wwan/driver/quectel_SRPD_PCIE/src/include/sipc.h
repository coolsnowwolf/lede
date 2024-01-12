/*
 * Copyright (C) 2012-2019 Spreadtrum Communications Inc.
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

#ifndef __SIPC_H
#define __SIPC_H

#include <linux/poll.h>

/* ****************************************************************** */
/* SMSG interfaces */

/* sipc processor ID definition */
enum {
	SIPC_ID_AP = 0,		/* Application Processor */
	SIPC_ID_MINIAP,		/* mini AP processor */
	SIPC_ID_CPW,		/* WCDMA processor */
	SIPC_ID_WCN,		/* Wireless Connectivity */
	SIPC_ID_GNSS,		/* Gps processor(gnss) */
	SIPC_ID_PSCP,		/* Protocol stack processor */
	SIPC_ID_PM_SYS,		/* Power management processor */
	SIPC_ID_NR_PHY,		/* New Radio PHY processor */
	SIPC_ID_V3_PHY,		/* MODEM v3 PHY processor */
	SIPC_ID_NR,		/* Max processor number */
};

#define SIPC_ID_LTE		SIPC_ID_PSCP

/* share-mem ring buffer short message */
struct smsg {
	u8		channel;	/* channel index */
	u8		type;		/* msg type */
	u16		flag;		/* msg flag */
	u32		value;		/* msg value */
};

/* smsg channel definition */
enum {
	SMSG_CH_CTRL = 0,	/* some emergency control */
	SMSG_CH_COMM,		/* general communication channel */
	SMSG_CH_IMSBR_DATA,	/* ims bridge data channel */
	SMSG_CH_IMSBR_CTRL,	/* ims bridge control channel */
	SMSG_CH_PIPE,		/* general pipe channel */
	SMSG_CH_PLOG,		/* pipe for debug log/dump */
	SMSG_CH_TTY,		/* virtual serial for telephony */
	SMSG_CH_DATA0,		/* 2G/3G wirleless data */
	SMSG_CH_DATA1,		/* 2G/3G wirleless data */
	SMSG_CH_DATA2,		/* 2G/3G wirleless data */
	SMSG_CH_VBC,		/* audio conrol channel */
	SMSG_CH_PLAYBACK,	/* audio playback channel */
	SMSG_CH_CAPTURE,	/* audio capture channel */
	SMSG_CH_MONITOR_AUDIO,	/* audio monitor channel */
	SMSG_CH_CTRL_VOIP,	/* audio voip conrol channel */
	SMSG_CH_PLAYBACK_VOIP,	/* audio voip playback channel */
	SMSG_CH_CAPTURE_VOIP,	/* audio voip capture channel */
	SMSG_CH_MONITOR_VOIP,	/* audio voip monitor channel */
	SMSG_CH_DATA3,		/* 2G/3G wirleless data */
	SMSG_CH_DATA4,		/* 2G/3G wirleless data */
	SMSG_CH_DATA5,		/* 2G/3G wirleless data */
	SMSG_CH_DIAG,		/* pipe for debug log/dump */
	SMSG_CH_PM_CTRL,	/* power management control */
	SMSG_CH_DUAL_SIM_PLUG,	/* dual sim plug channel */
	SMSG_CH_END			/* will not allow add channel in here */
};

/* smsg channel definition */
enum {
	/* 2G/3G wirleless data, channel 24~39 */
	SMSG_CH_DATA_BASE = 24,
	SMSG_CH_DATA6 = SMSG_CH_DATA_BASE,
	SMSG_CH_DATA7,
	SMSG_CH_DATA8,
	SMSG_CH_DATA9,
	SMSG_CH_DATA10,
	SMSG_CH_DATA11,
	SMSG_CH_DATA12,
	SMSG_CH_DATA13,

	/* general pipe channel,  channel 40~59 */
	SMSG_CH_PIPE_BASE = 40,
	SMSG_CH_NV = SMSG_CH_PIPE_BASE,
	SMSG_CH_DVFS,
	SMSG_CH_PIPE2,
	SMSG_CH_PIPE3,

	/* pipe for debug log/dump channel 60~79 */
	SMSG_CH_PLOG_BASE = 60,
	SMSG_CH_PLOG0 = SMSG_CH_PLOG_BASE,
	SMSG_CH_PLOG1,
	SMSG_CH_PLOG2,
	SMSG_CH_PLOG3,

	/* virtual serial for telephony,  channel 80~99*/
	SMSG_CH_TTY_BASE  = 80,
	SMSG_CH_TTY0 = SMSG_CH_TTY_BASE,
	SMSG_CH_TTY1,
	SMSG_CH_TTY2,
	SMSG_CH_TTY3,

	/* some emergency control, channel 100~119 */
	SMSG_CH_CTRL_BASE = 100,
	SMSG_CH_PMSYS_DBG = SMSG_CH_CTRL_BASE,
	SMSG_CH_CTRL1,
	SMSG_CH_CTRL2,
	SMSG_CH_CTRL3,

	/* general communication, channel 120~129 */
	SMSG_CH_COMM_BASE = 120,
	SMSG_CH_COMM_SIPA = SMSG_CH_COMM_BASE,
	SMSG_CH_COMM1,
	SMSG_CH_COMM2,
	SMSG_CH_COMM3,

	/* audio  channel, channel 130 ~149 */
	SMSG_CH_AUDIO_BASE = 130,
	SMSG_CH_AGDSP_ACCESS = SMSG_CH_AUDIO_BASE,/* audio conrol channel */
	SMSG_CH_PLAYBACK_DEEP,
	SMSG_CH_AUDIO2,
	SMSG_CH_AUDIO3,

	/* VOIP  channel, channel 150 ~169 */
	SMSG_CH_VOIP_BASE =  150,
	SMSG_CH_VOIP0 = SMSG_CH_VOIP_BASE,/* audio voip conrol channel */
	SMSG_CH_VOIP_DEEP,	/* audio voip playback channel */
	SMSG_CH_VOIP2,	/* audio voip capture channel */
	SMSG_CH_VOIP3,	/* audio voip monitor channel */

	/* RPC server channel, channel 170~189  */
	SMSG_CH_RPC_BASE  = 170,
	SMSG_CH_RPC0 = SMSG_CH_RPC_BASE,
	SMSG_CH_RPC1,
	SMSG_CH_RPC2,
	SMSG_CH_RPC3,

	/* RESERVE group 1, channel 190 ~209 */
	SMSG_CH_RESERVE1_BASE =  190,

	/* RESERVE group 2, channel 210 ~129 */
	SMSG_CH_RESERVE2_BASE =  210,

	/* RESERVE group 3, channel 230 ~244 */
	SMSG_CH_RESERVE3_BASE =  230,

	/* RESERVE group 4, channel 245 ~254 */
	SMSG_CH_RESERVE4_BASE =  245,

	/* total channel number  255, the max chanel number is 254*/
	SMSG_CH_NR = 255
};
#define INVALID_CHANEL_INDEX SMSG_CH_NR

/* modem type */
enum {
	SOC_MODEM = 0,
	PCIE_MODEM,
};

/* only be configed in sipc_config is valid channel */
struct sipc_config {
	u8 channel;
	char *name;
};

static const struct sipc_config sipc_cfg[] = {
	{SMSG_CH_CTRL, "com control"}, /* chanel 0 */
	{SMSG_CH_COMM, "com communication"}, /* chanel 1 */
	{SMSG_CH_PM_CTRL, "pm contrl"}, /* chanel 22 */
	{SMSG_CH_PMSYS_DBG, "pm debug contrl"}, /* chanel 100 */
	{SMSG_CH_DUAL_SIM_PLUG, "dual sim plug"}, /* chanel 23 */
	{SMSG_CH_PIPE, "pipe0"}, /* chanel 4 */
	{SMSG_CH_PLOG, "plog"}, /* chanel 5 */
	{SMSG_CH_DIAG, "diag"}, /* chanel 21 */
	{SMSG_CH_TTY, "stty chanel"}, /* chanel 6 */
	{SMSG_CH_DATA0, "seth0"}, /* chanel 7 */
	{SMSG_CH_DATA1, "seth1"}, /* chanel 8 */
	{SMSG_CH_DATA2, "seth2"}, /* chanel 9 */
	{SMSG_CH_DATA3, "seth3"}, /* chanel 18 */
	{SMSG_CH_DATA4, "seth4"}, /* chanel 19 */
	{SMSG_CH_DATA5, "seth5"}, /* chanel 20 */
	{SMSG_CH_DATA6, "seth6"}, /* chanel 24 */
	{SMSG_CH_DATA7, "seth7"}, /* chanel 25 */
	{SMSG_CH_DATA8, "seth8"}, /* chanel 26 */
	{SMSG_CH_DATA9, "seth9"}, /* chanel 27 */
	{SMSG_CH_DATA10, "seth10"}, /* chanel 28 */
	{SMSG_CH_DATA11, "seth11"}, /* chanel 29 */
	{SMSG_CH_DATA12, "seth12"}, /* chanel 30 */
	{SMSG_CH_DATA13, "seth13"}, /* chanel 31 */
	{SMSG_CH_VBC, "audio control"}, /* chanel 10 */
	{SMSG_CH_PLAYBACK, "audio playback"}, /* chanel 11 */
	{SMSG_CH_CAPTURE, "audio capture"}, /* chanel 12 */
	{SMSG_CH_MONITOR_AUDIO, "audio monitor"}, /* chanel 13 */
	{SMSG_CH_AGDSP_ACCESS, "agdsp access"}, /* chanel 13 */
	{SMSG_CH_CTRL_VOIP, "VOIP conrol"}, /* chanel 14 */
	{SMSG_CH_PLAYBACK_VOIP, "VOIP playback"}, /* chanel 15 */
	{SMSG_CH_CAPTURE_VOIP, "VOIP capture"}, /* chanel 16 */
	{SMSG_CH_MONITOR_VOIP, "VOIP monitor"}, /* chanel 17 */
	{SMSG_CH_PLAYBACK_DEEP, "audio playback deep"},  /*channel 131*/
	{SMSG_CH_IMSBR_DATA, "imsbr data"}, /* chanel 2 */
	{SMSG_CH_IMSBR_CTRL, "imsbr control"},  /* channel 3 */
	{SMSG_CH_VOIP_DEEP, "audio voip deep"},  /*channel 151*/
	{SMSG_CH_DVFS, "dvfs"},  /* channel 41 */
	{SMSG_CH_COMM_SIPA, "sipa"},  /* channel 120 */
	{SMSG_CH_NV, "nvsync"}, /* channel 40 */
};

#define SMSG_VALID_CH_NR (sizeof(sipc_cfg)/sizeof(struct sipc_config))


/* smsg type definition */
enum {
	SMSG_TYPE_NONE = 0,
	SMSG_TYPE_OPEN,		/* first msg to open a channel */
	SMSG_TYPE_CLOSE,	/* last msg to close a channel */
	SMSG_TYPE_DATA,		/* data, value=addr, no ack */
	SMSG_TYPE_EVENT,	/* event with value, no ack */
	SMSG_TYPE_CMD,		/* command, value=cmd */
	SMSG_TYPE_DONE,		/* return of command */
	SMSG_TYPE_SMEM_ALLOC,	/* allocate smem, flag=order */
	SMSG_TYPE_SMEM_FREE,	/* free smem, flag=order, value=addr */
	SMSG_TYPE_SMEM_DONE,	/* return of alloc/free smem */
	SMSG_TYPE_FUNC_CALL,	/* RPC func, value=addr */
	SMSG_TYPE_FUNC_RETURN,	/* return of RPC func */
	SMSG_TYPE_DIE,
	SMSG_TYPE_DFS,
	SMSG_TYPE_DFS_RSP,
	SMSG_TYPE_ASS_TRG,
	SMSG_TYPE_HIGH_OFFSET, /* client sipc get high offset from host */
	SMSG_TYPE_NR,		/* total type number */
};

/* flag for OPEN/CLOSE msg type */
#define	SMSG_OPEN_MAGIC		0xBEEE
#define	SMSG_CLOSE_MAGIC	0xEDDD

/**
* sipc_get_wakeup_flag
* after the wakeup flag be set, the fist smsg will be print
* @parameters: void
* @return: int
*/
int sipc_get_wakeup_flag(void);

/**
* sipc_set_wakeup_flag
* after the wakeup flag be set, the fist smsg will be print
* @parameters: void
* @return: no return value
*/
void sipc_set_wakeup_flag(void);

/**
* sipc_clear_wakeup_flag
* clear the wake up flag
* @parameters: void
* @return: no return value
*/
void sipc_clear_wakeup_flag(void);

/**
 * smsg_ch_open -- open a channel for smsg
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @timeout: milliseconds, 0 means no wait, -1 means unlimited
 * @return: 0 on success, <0 on failure
 */
int smsg_ch_open(u8 dst, u8 channel, int timeout);

/**
 * smsg_ch_close -- close a channel for smsg
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @timeout: milliseconds, 0 means no wait, -1 means unlimited
 * @return: 0 on success, <0 on failure
 */
int smsg_ch_close(u8 dst, u8 channel, int timeout);

 /**
 * smsg_senddie -- send a MSG_TYPE_DIE message to pubcp
 *
 * @dst: dest processor ID
 * @return: 0 on success, <0 on failure
 */
int smsg_senddie(u8 dst);

/**
 * smsg_send -- send smsg
 *
 * @dst: dest processor ID
 * @msg: smsg body to be sent
 * @timeout: milliseconds, 0 means no wait, -1 means unlimited
 * @return: 0 on success, <0 on failure
 */
int smsg_send(u8 dst, struct smsg *msg, int timeout);

/**
 * smsg_recv -- poll and recv smsg
 *
 * @dst: dest processor ID
 * @msg: smsg body to be received, channel should be filled as input
 * @timeout: milliseconds, 0 means no wait, -1 means unlimited
 * @return: 0 on success, <0 on failure
 */
int smsg_recv(u8 dst, struct smsg *msg, int timeout);

/**
 * sipc_channel2index
 *
 * only be configed in sipc_config is valid channel
 * @ch: channel number
 * @return:  channel index ,if return index is INVALID_CHANEL_INDEX ,
 *               it indicate it is a invalid chanel
 */
u8 sipc_channel2index(u8 ch);

int smsg_ch_wake_unlock(u8 dst, u8 channel);

#if defined(CONFIG_DEBUG_FS)
void sipc_debug_putline(struct seq_file *m, char c, int n);
#endif

/* quickly fill a smsg body */
static inline void smsg_set(struct smsg *msg, u8 channel,
		u8 type, u16 flag, u32 value)
{
	msg->channel = channel;
	msg->type = type;
	msg->flag = flag;
	msg->value = value;
}

/* ack an open msg for modem recovery */
static inline void smsg_open_ack(u8 dst, u16 channel)
{
	struct smsg mopen;

	pr_info("%s: channel %d-%d!\n", __func__, dst, channel);
	smsg_set(&mopen, channel, SMSG_TYPE_OPEN, SMSG_OPEN_MAGIC, 0);
	smsg_send(dst, &mopen, -1);
}

/* ack an close msg for modem recovery */
static inline void smsg_close_ack(u8 dst, u16 channel)
{
	struct smsg mclose;

	pr_info("%s: channel %d-%d!\n", __func__, dst, channel);
	smsg_set(&mclose, channel, SMSG_TYPE_CLOSE, SMSG_CLOSE_MAGIC, 0);
	smsg_send(dst, &mclose, -1);
}

/* ****************************************************************** */
/* SMEM interfaces */
/**
 * smem_get_area -- get sipc smem
 *
 * @dst: dest processor ID
 * @base: base pointer
 * @size: size pointer
 * @return: 0 success, < 0 failed
 */
int smem_get_area(u8 dst, u32 *base, u32 *size);

/**
 * smem_alloc -- allocate shared memory block
 *
 * @dst: dest processor ID
 * @size: size to be allocated, page-aligned
 * @return: phys addr or 0 if failed
 */
u32 smem_alloc(u8 dst, u32 size);

/**
 * smem_free -- free shared memory block
 *
 * @dst: dest processor ID
 * @addr: smem phys addr to be freed
 * @order: size to be freed
 */
void smem_free(u8 dst, u32 addr, u32 size);

/**
 * shmem_ram_unmap -- for sipc unmap ram address
 *
 * @mem: vir mem
 */
void shmem_ram_unmap(u8 dst, const void *mem);

/**
 * shmem_ram_vmap_nocache -- for sipc map ram address
 *
 * @start: start address
 * @size: size to be allocated, page-aligned
 * @return: phys addr or 0 if failed
 */
void *shmem_ram_vmap_nocache(u8 dst, phys_addr_t start, size_t size);

/**
 * shmem_ram_vmap_cache -- for sipc map ram address
 *
 * @start: start address
 * @size: size to be allocated, page-aligned
 * @return: phys addr or 0 if failed
 */
void *shmem_ram_vmap_cache(u8 dst, phys_addr_t start, size_t size);

void smem_free(u8 dst, u32 addr, u32 size);

/**
 * modem_ram_unmap -- for modem unmap ram address
 *
 * @mem: vir mem
 * @modem_type: soc modem, pcie modem
 */
void modem_ram_unmap(u32 modem_type, const void *mem);

/**
 * shmem_ram_vmap_nocache -- for modem map ram address
 *
 * @modem_type: soc modem, pcie modem
 * @start: start address
 * @size: size to be allocated, page-aligned
 * @return: phys addr or 0 if failed
 */
void *modem_ram_vmap_nocache(u32 modem_type, phys_addr_t start, size_t size);

/**
 * modem_ram_vmap_cache -- for modem map ram address
 *
 * @modem_type: soc modem, pcie modem
 * @start: start address
 * @size: size to be allocated, page-aligned
 * @return: phys addr or 0 if failed
 */
void *modem_ram_vmap_cache(u32 modem_type, phys_addr_t start, size_t size);
/**
 * sbuf_set_no_need_wake_lock
 *
 * @dst: dest processor ID
 * @bufnum: which buffer to be set
 * @return: none
 */

void sbuf_set_no_need_wake_lock(u8 dst, u8 channel, u32 bufnum);

/**
 * sbuf_create -- create pipe ring buffers on a channel
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @txbufsize: tx buffer size
 * @rxbufsize: rx buffer size
 * @bufnum: how many buffers to be created
 * @return: 0 on success, <0 on failure
 */

int sbuf_create(u8 dst, u8 channel, u32 bufnum,
		u32 txbufsize, u32 rxbufsize);

/**
 * sbuf_destroy -- destroy the pipe ring buffers on a channel
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @return: 0 on success, <0 on failure
 */
void sbuf_destroy(u8 dst, u8 channel);
void sbuf_down(u8 dst, u8 channel);

/**
 * sbuf_write -- write data to a sbuf
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @bufid: buffer ID
 * @buf: data to be written
 * @len: data length
 * @timeout: milliseconds, 0 means no wait, -1 means unlimited
 * @return: written bytes on success, <0 on failure
 */
int sbuf_write(u8 dst, u8 channel, u32 bufid,
		void *buf, u32 len, int timeout);

/**
 * sbuf_read -- write data to a sbuf
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @bufid: buffer ID
 * @buf: data to be written
 * @len: data length
 * @timeout: milliseconds, 0 means no wait, -1 means unlimited
 * @return: read bytes on success, <0 on failure
 */
int sbuf_read(u8 dst, u8 channel, u32 bufid,
		void *buf, u32 len, int timeout);

/**
 * sbuf_poll_wait -- poll sbuf read/write, used in spipe driver
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @bufid: buffer ID
 * @file: struct file handler
 * @wait: poll table
 * @return: POLLIN or POLLOUT
 */
int sbuf_poll_wait(u8 dst, u8 channel, u32 bufid,
		struct file *file, poll_table *wait);

/**
 * sbuf_status -- get sbuf status
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @return: 0 when ready, <0 when broken
 */
int sbuf_status(u8 dst, u8 channel);

#define	SBUF_NOTIFY_READY	0x00
#define	SBUF_NOTIFY_READ	0x01
#define	SBUF_NOTIFY_WRITE	0x02
/**
 * sbuf_register_notifier -- register a callback that's called
 *		when a tx sbuf is available or a rx sbuf is received.
 *		non-blocked sbuf_read can be called.
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @bufid: buf ID
 * @handler: a callback handler
 * @event: NOTIFY_READ, NOTIFY_WRITE, or both
 * @data: opaque data passed to the receiver
 * @return: 0 on success, <0 on failure
 */
int sbuf_register_notifier(u8 dst, u8 channel, u32 bufid,
		void (*handler)(int event, void *data), void *data);


int sipc_init(void);
void sipc_exit(void);

int spipe_init(void);
void spipe_exit(void);
void spipe_device_down(void);
void spool_device_down(void);

int spool_init(void);
void spool_exit(void);

int modem_power_manager_init(void);
void modem_power_manager_exit(void);

#if 0
int modem_ctrl_init(void);
void modem_ctrl_exit(void);
#endif

int sipc_init_debugfs(void);

/* ****************************************************************** */
/* SBLOCK interfaces */

/* sblock structure: addr is the uncached virtual address */
struct sblock {
	void		*addr;
	u32	length;
#ifdef CONFIG_SPRD_SIPC_ZERO_COPY_SIPX
	u16        index;
	u16        offset;
#endif
};

/**
 * sblock_create -- create sblock manager on a channel
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @txblocknum: tx block number
 * @txblocksize: tx block size
 * @rxblocknum: rx block number
 * @rxblocksize: rx block size
 * @return: 0 on success, <0 on failure
 */
int sblock_create(u8 dst, u8 channel,
		u32 txblocknum, u32 txblocksize,
		u32 rxblocknum, u32 rxblocksize);

/**
 * sblock_create_ex -- merge sblock_create and block_register_notifier
 * in one function
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @txblocknum: tx block number
 * @txblocksize: tx block size
 * @rxblocknum: rx block number
 * @rxblocksize: rx block size
 * @event: SBLOCK_NOTIFY_GET, SBLOCK_NOTIFY_RECV, or both
 * @data: opaque data passed to the receiver
 * @return: 0 on success, <0 on failure
 */
int sblock_create_ex(u8 dst, u8 channel,
			u32 txblocknum, u32 txblocksize,
			u32 rxblocknum, u32 rxblocksize,
			void (*handler)(int event, void *data), void *data);

/* sblock_pcfg_create -- create preconfigured SBLOCK channel.
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @tx_blk_num: tx block number
 * @tx_blk_sz: tx block size
 * @rx_blk_num: rx block number
 * @rx_blk_sz: rx block size
 * @return: 0 on success, <0 on failure
 *
 * The function only allocates the memory for the channel, and will not
 * open the channel. The client shall open the channel using
 * sblock_pcfg_open and close the channel using sblock_close.
 */
int sblock_pcfg_create(u8 dst, u8 channel,
		       u32 tx_blk_num, u32 tx_blk_sz,
		       u32 rx_blk_num, u32 rx_blk_sz);

/* sblock_pcfg_open -- request to open preconfigured SBLOCK channel.
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @notifier: the event notification callback function. This function can
 *	      not sleep. If this parameter is NULL, no event will be
 *	      reported.
 * @event: SBLOCK_NOTIFY_GET, SBLOCK_NOTIFY_RECV, or both
 * @client: opaque data passed to the receiver
 * @return: if the channel is established, return 0; if the open procedure
 *          is started and not finished, return SIPC_ERR_IN_PROGRESS;
 *	    otherwise return a negative error code.
 *
 * The function starts the open procedure. If the open procedure is not
 * finished when the function returns, the SBLOCK system will report
 * the open result later through the notifier callback.
 */
int sblock_pcfg_open(uint8_t dest, uint8_t channel,
		     void (*notifier)(int event, void *client),
		     void *client);

/* sblock_close -- request to close SBLOCK channel.
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @return: if the channel is closed, return 0; if the close procedure
 *          is started and not finished, return SIPC_ERR_IN_PROGRESS;
 *	    otherwise return a negative error code.
 *
 * The function starts the close procedure. If the close procedure is not
 * finished when the function returns, the SBLOCK system will report
 * the close result later through the notification callback that the
 * client set by sblock_pcfg_open.
 */
int sblock_close(uint8_t dest, uint8_t channel);

/* sblock_get_smem_cp_addr - get the shared memory CP address.
 * @dest: destination ID
 * @channel: channel number
 * @paddr: pointer to the variable to receive the address.
 */
int sblock_get_smem_cp_addr(uint8_t dest, uint8_t channel,
			    uint32_t *paddr);
/**
 * sblock_destroy -- destroy sblock manager on a channel
 *
 * @dst: dest processor ID
 * @channel: channel ID
 */
void sblock_destroy(u8 dst, u8 channel);
void sblock_down(u8 dst, u8 channel);

#define	SBLOCK_NOTIFY_GET	0x01
#define	SBLOCK_NOTIFY_RECV	0x02
#define	SBLOCK_NOTIFY_STATUS	0x04
#define	SBLOCK_NOTIFY_OPEN	0x08
#define	SBLOCK_NOTIFY_CLOSE	0x10
#define SBLOCK_NOTIFY_OPEN_FAILED 0x20

/**
 * sblock_register_notifier -- register a callback that's called
 *		when a tx sblock is available or a rx block is received.
 *		non-blocked sblock_get or sblock_receive can be called.
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @handler: a callback handler
 * @event: SBLOCK_NOTIFY_GET, SBLOCK_NOTIFY_RECV, or both
 * @data: opaque data passed to the receiver
 * @return: 0 on success, <0 on failure
 */
int sblock_register_notifier(u8 dst, u8 channel,
		void (*handler)(int event, void *data), void *data);

/**
 * sblock_get  -- get a free sblock for sender
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @blk: return a gotten sblock pointer
 * @timeout: milliseconds, 0 means no wait, -1 means unlimited
 * @return: 0 on success, <0 on failure
 */
int sblock_get(u8 dst, u8 channel, struct sblock *blk, int timeout);

/**
 * sblock_send  -- send a sblock with smsg, it should be from sblock_get
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @blk: the sblock to be sent
 * @return: 0 on success, <0 on failure
 */
int sblock_send(u8 dst, u8 channel, struct sblock *blk);

/**
 * sblock_send_prepare  -- send a sblock without smsg,
 * it should be from sblock_get
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @blk: the sblock to be sent
 * @return: 0 on success, <0 on failure
 */
int sblock_send_prepare(u8 dst, u8 channel, struct sblock *blk);

/**
 * sblock_send_finish  -- trigger an smsg to notify that sblock has been sent
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @return: 0 on success, <0 on failure
 */
int sblock_send_finish(u8 dst, u8 channel);

/**
 * sblock_receive  -- receive a sblock, it should be released after it's handled
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @blk: return a received sblock pointer
 * @timeout: milliseconds, 0 means no wait, -1 means unlimited
 * @return: 0 on success, <0 on failure
 */
int sblock_receive(u8 dst, u8 channel,
		struct sblock *blk, int timeout);

/**
 * sblock_release  -- release a sblock from reveiver
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @return: 0 on success, <0 on failure
 */
int sblock_release(u8 dst, u8 channel, struct sblock *blk);

/**
 * sblock_get_arrived_count  -- get the count of sblock(s) arrived at
 * AP (sblock_send on CP) but not received (sblock_receive on AP).
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @return: >=0  the count of blocks
 */
int sblock_get_arrived_count(u8 dst, u8 channel);



/**
 * sblock_get_free_count  -- get the count of available sblock(s) resident in
 * sblock pool on AP.
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @return: >=0  the count of blocks
 */
int sblock_get_free_count(u8 dst, u8 channel);


/**
 * sblock_put  -- put a free sblock for sender
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @blk: sblock pointer
 * @return: void
 */
void sblock_put(u8 dst, u8 channel, struct sblock *blk);

/**
 * sblock_poll_wait  -- poll sblock read/write
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @filp: strcut file handle
 * @wait: poll table
 * @return: POLLIN or POLLOUT
 */
unsigned int sblock_poll_wait(u8 dst, u8 channel, struct file *filp, poll_table *wait);

/**
 * sblock_query  -- sblock query status
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @return: 0 on success, <0 on failure
 */
int sblock_query(u8 dst, u8 channel);


/* ****************************************************************** */

#define SIPX_ACK_BLK_LEN                (100)

/**
 * sipx_chan_create -- create a sipx channel
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @return: 0 on success, <0 on failure
 */
int sipx_chan_create(u8 dst, u8 channel);

/**
 * sipx_chan_destroy -- destroy seblock manager on a channel
 *
 * @dst: dest processor ID
 * @channel: channel ID
 */
int sipx_chan_destroy(u8 dst, u8 channel);

/**
 * sipx_get_ack_blk_len  -- get sipx ack block max length
 *
 * @dst: dest processor ID
 * @return: length
 */
u32 sipx_get_ack_blk_len(u8 dst);

/**
 * sipx_get  -- get a free sblock for sender
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @blk: return a gotten sblock pointer
 * @is_ack: if want to get block for ack packet
 * @return: 0 on success, <0 on failure
 */
int sipx_get(u8 dst, u8 channel, struct sblock *blk, int is_ack);

/**
 * sipx_chan_register_notifier -- register a callback that's called
 * when a tx sblock is available or a rx block is received.
 * on-blocked sblock_get or sblock_receive can be called.
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @handler: a callback handler
 * @event: SBLOCK_NOTIFY_GET, SBLOCK_NOTIFY_RECV, or both
 * @data: opaque data passed to the receiver
 * @return: 0 on success, <0 on failure
 */
int sipx_chan_register_notifier(u8 dst, u8 channel,
		void (*handler)(int event, void *data), void *data);

/**
 * sipx_send  -- send a sblock with smsg, it should be from seblock_get
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @blk: the sblock to be sent
 * @return: 0 on success, <0 on failure
 */
int sipx_send(u8 dst, u8 channel, struct sblock *blk);

/**
 * sipx_flush  -- trigger an smsg to notify that sblock has been sent
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @return: 0 on success, <0 on failure
 */
int sipx_flush(u8 dst, u8 channel);

/**
 * sipx_receive  -- receive a sblock, it should be released after it's handled
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @blk: return a received sblock pointer
 * @return: 0 on success, <0 on failure
 */
int sipx_receive(u8 dst, u8 channel, struct sblock *blk);

/**
 * sipx_release  -- release a sblock from reveiver
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @return: 0 on success, <0 on failure
 */
int sipx_release(u8 dst, u8 channel, struct sblock *blk);

/**
 * sipx_get_arrived_count  -- get the count of sblock(s) arrived at
 * AP (sblock_send on CP) but not received (sblock_receive on AP).
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @return: >=0  the count of blocks
 */
int sipx_get_arrived_count(u8 dst, u8 channel);

/**
 * sipx_get_free_count  -- get the count of available sblock(s) resident in
 * normal pool on AP.
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @return: >=0  the count of blocks
 */
int sipx_get_free_count(u8 dst, u8 channel);

/**
 * sipx_put  -- put a free sblock for sender
 *
 * @dst: dest processor ID
 * @channel: channel ID
 * @blk: sblock pointer
 * @return: void
 */
int sipx_put(u8 dst, u8 channel, struct sblock *blk);

/* ****************************************************************** */

#ifdef CONFIG_SPRD_SIPC_ZERO_COPY_SIPX

#define SBLOCK_CREATE(dst, channel,\
		txblocknum, txblocksize, txpoolsize, \
		rxblocknum, rxblocksize, rxpoolsize)  \
sipx_chan_create(dst, channel)


#define SBLOCK_DESTROY(dst, channel) \
	sipx_chan_destroy(dst, channel)

#define SBLOCK_GET(dst, channel, blk, ack, timeout) \
	sipx_get(dst, channel, blk, ack)

#define SBLOCK_REGISTER_NOTIFIER(dst, channel, handler, data) \
	sipx_chan_register_notifier(dst, channel, handler, data)

#define SBLOCK_SEND(dst, channel, blk) \
	sipx_send(dst, channel, blk)

#define SBLOCK_SEND_PREPARE(dst, channel, blk) \
	sipx_send(dst, channel, blk)

#define SBLOCK_SEND_FINISH(dst, channel)\
	sipx_flush(dst, channel)

#define SBLOCK_RECEIVE(dst, channel, blk, timeout) \
	sipx_receive(dst, channel, blk)

#define SBLOCK_RELEASE(dst, channel, blk) \
	sipx_release(dst, channel, blk)

#define SBLOCK_GET_ARRIVED_COUNT(dst, channel) \
	sipx_get_arrived_count(dst, channel)

#define SBLOCK_GET_FREE_COUNT(dst, channel) \
	sipx_get_free_count(dst, channel)

#define SBLOCK_PUT(dst, channel, blk) \
	sipx_put(dst, channel, blk)


#else /* CONFIG_SPRD_SIPC_ZERO_COPY_SIPX */

#define SBLOCK_CREATE(dst, channel,\
		txblocknum, txblocksize, txpoolsize, \
		rxblocknum, rxblocksize, rxpoolsize)  \
sblock_create(dst, channel,\
		txblocknum, txblocksize,\
		rxblocknum, rxblocksize)

#define SBLOCK_DESTROY(dst, channel) \
	sblock_destroy(dst, channel)

#define SBLOCK_GET(dst, channel, blk, ack, timeout) \
	sblock_get(dst, channel, blk, timeout)

#define SBLOCK_REGISTER_NOTIFIER(dst, channel, handler, data) \
	sblock_register_notifier(dst, channel, handler, data)

#define SBLOCK_SEND(dst, channel, blk) \
	sblock_send(dst, channel, blk)

#define SBLOCK_SEND_PREPARE(dst, channel, blk) \
	sblock_send_prepare(dst, channel, blk)

#define SBLOCK_SEND_FINISH(dst, channel)\
	sblock_send_finish(dst, channel)

#define SBLOCK_RECEIVE(dst, channel, blk, timeout) \
	sblock_receive(dst, channel, blk, timeout)

#define SBLOCK_RELEASE(dst, channel, blk) \
	sblock_release(dst, channel, blk)

#define SBLOCK_GET_ARRIVED_COUNT(dst, channel) \
	sblock_get_arrived_count(dst, channel)

#define SBLOCK_GET_FREE_COUNT(dst, channel) \
	sblock_get_free_count(dst, channel)

#define SBLOCK_PUT(dst, channel, blk) \
	sblock_put(dst, channel, blk)

#endif /* CONFIG_SPRD_SIPC_ZERO_COPY_SIPX */

#ifdef CONFIG_ARM64
/**
 * unalign_copy_from_user  -- unaligned data accesses to addresses
 * marked as device will always trigger an exception, this fuction
 * can avoid this exception
 *
 * @to: dest, normal memory
 * @from: src, device memory and alignment access must be considered
 * @n: bytes
 * @return: bytes not copied
 */
static inline unsigned long unalign_copy_to_user(void __user *to,
		const void *from,
		unsigned long n)
{
	/* from is not 8 byte aligned and n is less than 16 bytes */
	if (((unsigned long)from & 7) && (n < 16)) {
		while (n) {
			if (copy_to_user(to++, from++, 1))
				break;
			n--;
		}
		return n;
	}

	return copy_to_user(to, from, n);
}

/**
 * unalign_copy_from_user  -- unaligned data accesses to addresses
 * marked as device will always trigger an exception, this fuction
 * can avoid this exception
 *
 * @to: dest, device memory and alignment access must be considered
 * @from: src, normal memory
 * @n: bytes
 * @return: bytes not copied
 */
static inline unsigned long unalign_copy_from_user(void *to,
		const void __user *from,
		unsigned long n)
{
	unsigned c1, c2, c3;

	/* to is 8 byte aligned and n is less than 16 bytes */
	c1 = !((unsigned long)to & 0x7) && (n < 16);
	if (c1)
		return copy_from_user(to, from, n);

	/* to and from are 8 byte aligned */
	c2 = !((unsigned long)to & 0x7) && !((unsigned long)from & 0x7);
	if (c2)
		return copy_from_user(to, from, n);

	/* to and from are the same offset and n is more than 15 bytes */
	c3 = !(((unsigned long)to ^ (unsigned long)from) & 0x7) && (n > 15);
	if (c3)
		return copy_from_user(to, from, n);

	while (n) {
		if (copy_from_user(to++, from++, 1))
			break;
		n--;
	}

	return n;
}

static inline void unalign_memcpy(void *to, const void *from, size_t n)
{
	if (((unsigned long)to & 7) == ((unsigned long)from & 7)) {
		while (((unsigned long)from & 7) && n) {
			*(char *)(to++) = *(char *)(from++);
			n--;
		}
		memcpy(to, from, n);
	} else if (((unsigned long)to & 3) == ((unsigned long)from & 3)) {
		while (((unsigned long)from & 3) && n) {
			*(char *)(to++) = *(char *)(from++);
			n--;
		}
		while (n >= 4) {
			*(u32 *)(to) = *(u32 *)(from);
			to += 4;
			from += 4;
			n -= 4;
		}
		while (n) {
			*(char *)(to++) = *(char *)(from++);
			n--;
		}
	} else {
		while (n) {
			*(char *)(to++) = *(char *)(from++);
			n--;
		}
	}
}
#else
static inline unsigned long unalign_copy_to_user(void __user *to,
		const void *from,
		unsigned long n)
{
	return copy_to_user(to, from, n);
}
static inline unsigned long unalign_copy_from_user(void *to,
		const void __user *from,
		unsigned long n)
{
	return copy_from_user(to, from, n);
}
static inline void *unalign_memcpy(void *to, const void *from, size_t n)
{
	return memcpy(to, from, n);
}
#endif

#endif
