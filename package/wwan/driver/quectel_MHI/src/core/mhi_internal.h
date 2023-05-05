/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2018-2019, The Linux Foundation. All rights reserved. */

#ifndef _MHI_INT_H
#define _MHI_INT_H

#include <linux/version.h>
#ifndef writel_relaxed
#define writel_relaxed writel
#endif

#ifndef writel_relaxed_no_log
#define writel_relaxed_no_log writel_relaxed
#endif

#ifndef readq
static inline u64 readq(void __iomem *reg)
{
	return ((u64) readl(reg)) | (((u64) readl(reg + 4UL)) << 32);
}
#endif

#ifndef readq_relaxed
#define readq_relaxed readq
#endif

#ifndef readq_relaxed_no_log
#define readq_relaxed_no_log readq_relaxed
#endif

#ifndef U32_MAX
#define U32_MAX		((u32)~0U)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION( 3,10,53 ))
static inline void reinit_completion(struct completion *x)
{
	x->done = 0;
}
#endif

#ifndef __ATTR_RO
#define __ATTR_RO(_name) {						\
	.attr	= { .name = __stringify(_name), .mode = S_IRUGO },	\
	.show	= _name##_show,						\
}
#endif
#ifndef __ATTR_WO
#define __ATTR_WO(_name) {						\
	.attr	= { .name = __stringify(_name), .mode = S_IWUSR },	\
	.store	= _name##_store,					\
}
#endif
#ifndef __ATTR_RW
#define __ATTR_RW(_name) __ATTR(_name, (S_IWUSR | S_IRUGO),		\
			 _name##_show, _name##_store)
#endif
#ifndef DEVICE_ATTR_RO
#define DEVICE_ATTR_RO(_name) \
	struct device_attribute dev_attr_##_name = __ATTR_RO(_name)
#endif
#ifndef DEVICE_ATTR_WO
#define DEVICE_ATTR_WO(_name) \
	struct device_attribute dev_attr_##_name = __ATTR_WO(_name)
#endif
#ifndef DEVICE_ATTR_RW
#define DRIVER_ATTR_RW(_name) \
	struct driver_attribute driver_attr_##_name = __ATTR_RW(_name)
#endif

#ifdef EXPORT_SYMBOL
#undef EXPORT_SYMBOL
#define EXPORT_SYMBOL(sym)
#endif

extern struct bus_type mhi_bus_type;

/* MHI mmio register mapping */
#define PCI_INVALID_READ(val) (val == U32_MAX)

#define MHIREGLEN (0x0)
#define MHIREGLEN_MHIREGLEN_MASK (0xFFFFFFFF)
#define MHIREGLEN_MHIREGLEN_SHIFT (0)

#define MHIVER (0x8)
#define MHIVER_MHIVER_MASK (0xFFFFFFFF)
#define MHIVER_MHIVER_SHIFT (0)

#define MHICFG (0x10)
#define MHICFG_NHWER_MASK (0xFF000000)
#define MHICFG_NHWER_SHIFT (24)
#define MHICFG_NER_MASK (0xFF0000)
#define MHICFG_NER_SHIFT (16)
#define MHICFG_NHWCH_MASK (0xFF00)
#define MHICFG_NHWCH_SHIFT (8)
#define MHICFG_NCH_MASK (0xFF)
#define MHICFG_NCH_SHIFT (0)

#define CHDBOFF (0x18)
#define CHDBOFF_CHDBOFF_MASK (0xFFFFFFFF)
#define CHDBOFF_CHDBOFF_SHIFT (0)

#define ERDBOFF (0x20)
#define ERDBOFF_ERDBOFF_MASK (0xFFFFFFFF)
#define ERDBOFF_ERDBOFF_SHIFT (0)

#define BHIOFF (0x28)
#define BHIOFF_BHIOFF_MASK (0xFFFFFFFF)
#define BHIOFF_BHIOFF_SHIFT (0)

#define BHIEOFF (0x2C)
#define BHIEOFF_BHIEOFF_MASK (0xFFFFFFFF)
#define BHIEOFF_BHIEOFF_SHIFT (0)

#define DEBUGOFF (0x30)
#define DEBUGOFF_DEBUGOFF_MASK (0xFFFFFFFF)
#define DEBUGOFF_DEBUGOFF_SHIFT (0)

#define MHICTRL (0x38)
#define MHICTRL_MHISTATE_MASK (0x0000FF00)
#define MHICTRL_MHISTATE_SHIFT (8)
#define MHICTRL_RESET_MASK (0x2)
#define MHICTRL_RESET_SHIFT (1)

#define MHISTATUS (0x48)
#define MHISTATUS_MHISTATE_MASK (0x0000FF00)
#define MHISTATUS_MHISTATE_SHIFT (8)
#define MHISTATUS_SYSERR_MASK (0x4)
#define MHISTATUS_SYSERR_SHIFT (2)
#define MHISTATUS_READY_MASK (0x1)
#define MHISTATUS_READY_SHIFT (0)

#define CCABAP_LOWER (0x58)
#define CCABAP_LOWER_CCABAP_LOWER_MASK (0xFFFFFFFF)
#define CCABAP_LOWER_CCABAP_LOWER_SHIFT (0)

#define CCABAP_HIGHER (0x5C)
#define CCABAP_HIGHER_CCABAP_HIGHER_MASK (0xFFFFFFFF)
#define CCABAP_HIGHER_CCABAP_HIGHER_SHIFT (0)

#define ECABAP_LOWER (0x60)
#define ECABAP_LOWER_ECABAP_LOWER_MASK (0xFFFFFFFF)
#define ECABAP_LOWER_ECABAP_LOWER_SHIFT (0)

#define ECABAP_HIGHER (0x64)
#define ECABAP_HIGHER_ECABAP_HIGHER_MASK (0xFFFFFFFF)
#define ECABAP_HIGHER_ECABAP_HIGHER_SHIFT (0)

#define CRCBAP_LOWER (0x68)
#define CRCBAP_LOWER_CRCBAP_LOWER_MASK (0xFFFFFFFF)
#define CRCBAP_LOWER_CRCBAP_LOWER_SHIFT (0)

#define CRCBAP_HIGHER (0x6C)
#define CRCBAP_HIGHER_CRCBAP_HIGHER_MASK (0xFFFFFFFF)
#define CRCBAP_HIGHER_CRCBAP_HIGHER_SHIFT (0)

#define CRDB_LOWER (0x70)
#define CRDB_LOWER_CRDB_LOWER_MASK (0xFFFFFFFF)
#define CRDB_LOWER_CRDB_LOWER_SHIFT (0)

#define CRDB_HIGHER (0x74)
#define CRDB_HIGHER_CRDB_HIGHER_MASK (0xFFFFFFFF)
#define CRDB_HIGHER_CRDB_HIGHER_SHIFT (0)

#define MHICTRLBASE_LOWER (0x80)
#define MHICTRLBASE_LOWER_MHICTRLBASE_LOWER_MASK (0xFFFFFFFF)
#define MHICTRLBASE_LOWER_MHICTRLBASE_LOWER_SHIFT (0)

#define MHICTRLBASE_HIGHER (0x84)
#define MHICTRLBASE_HIGHER_MHICTRLBASE_HIGHER_MASK (0xFFFFFFFF)
#define MHICTRLBASE_HIGHER_MHICTRLBASE_HIGHER_SHIFT (0)

#define MHICTRLLIMIT_LOWER (0x88)
#define MHICTRLLIMIT_LOWER_MHICTRLLIMIT_LOWER_MASK (0xFFFFFFFF)
#define MHICTRLLIMIT_LOWER_MHICTRLLIMIT_LOWER_SHIFT (0)

#define MHICTRLLIMIT_HIGHER (0x8C)
#define MHICTRLLIMIT_HIGHER_MHICTRLLIMIT_HIGHER_MASK (0xFFFFFFFF)
#define MHICTRLLIMIT_HIGHER_MHICTRLLIMIT_HIGHER_SHIFT (0)

#define MHIDATABASE_LOWER (0x98)
#define MHIDATABASE_LOWER_MHIDATABASE_LOWER_MASK (0xFFFFFFFF)
#define MHIDATABASE_LOWER_MHIDATABASE_LOWER_SHIFT (0)

#define MHIDATABASE_HIGHER (0x9C)
#define MHIDATABASE_HIGHER_MHIDATABASE_HIGHER_MASK (0xFFFFFFFF)
#define MHIDATABASE_HIGHER_MHIDATABASE_HIGHER_SHIFT (0)

#define MHIDATALIMIT_LOWER (0xA0)
#define MHIDATALIMIT_LOWER_MHIDATALIMIT_LOWER_MASK (0xFFFFFFFF)
#define MHIDATALIMIT_LOWER_MHIDATALIMIT_LOWER_SHIFT (0)

#define MHIDATALIMIT_HIGHER (0xA4)
#define MHIDATALIMIT_HIGHER_MHIDATALIMIT_HIGHER_MASK (0xFFFFFFFF)
#define MHIDATALIMIT_HIGHER_MHIDATALIMIT_HIGHER_SHIFT (0)

#define MHI_READ_REG_FIELD(_VAR,_REG,_FIELD)                                              \
    ((_VAR & _REG ## _ ## _FIELD ## _MASK) >> _REG ## _ ## _FIELD ## _SHIFT)

#define MHI_WRITE_REG_FIELD(_VAR,_REG,_FIELD,_VAL)                                        \
   do {                                                                                      \
      _VAR &= ~_REG ## _ ## _FIELD ## _MASK;                                       \
      _VAR |= (_VAL << _REG ## _ ## _FIELD ## _SHIFT);                             \
   }  while(0)

/* Host request register */
#define MHI_SOC_RESET_REQ_OFFSET (0xB0)
#define MHI_SOC_RESET_REQ BIT(0)

/* MHI misc capability registers */
#define MISC_OFFSET (0x24)
#define MISC_CAP_MASK (0xFFFFFFFF)
#define MISC_CAP_SHIFT (0)

#define CAP_CAPID_MASK (0xFF000000)
#define CAP_CAPID_SHIFT (24)
#define CAP_NEXT_CAP_MASK (0x00FFF000)
#define CAP_NEXT_CAP_SHIFT (12)

/* MHI Timesync offsets */
#define TIMESYNC_CFG_OFFSET (0x00)
#define TIMESYNC_CFG_CAPID_MASK (CAP_CAPID_MASK)
#define TIMESYNC_CFG_CAPID_SHIFT (CAP_CAPID_SHIFT)
#define TIMESYNC_CFG_NEXT_OFF_MASK (CAP_NEXT_CAP_MASK)
#define TIMESYNC_CFG_NEXT_OFF_SHIFT (CAP_NEXT_CAP_SHIFT)
#define TIMESYNC_CFG_NUMCMD_MASK (0xFF)
#define TIMESYNC_CFG_NUMCMD_SHIFT (0)
#define TIMESYNC_DB_OFFSET (0x4)
#define TIMESYNC_TIME_LOW_OFFSET (0x8)
#define TIMESYNC_TIME_HIGH_OFFSET (0xC)

#define TIMESYNC_CAP_ID (2)

/* MHI BHI offfsets */
#define BHI_BHIVERSION_MINOR (0x00)
#define BHI_BHIVERSION_MAJOR (0x04)
#define BHI_IMGADDR_LOW (0x08)
#define BHI_IMGADDR_HIGH (0x0C)
#define BHI_IMGSIZE (0x10)
#define BHI_RSVD1 (0x14)
#define BHI_IMGTXDB (0x18)
#define BHI_TXDB_SEQNUM_BMSK (0x3FFFFFFF)
#define BHI_TXDB_SEQNUM_SHFT (0)
#define BHI_RSVD2 (0x1C)
#define BHI_INTVEC (0x20)
#define BHI_RSVD3 (0x24)
#define BHI_EXECENV (0x28)
#define BHI_STATUS (0x2C)
#define BHI_ERRCODE (0x30)
#define BHI_ERRDBG1 (0x34)
#define BHI_ERRDBG2 (0x38)
#define BHI_ERRDBG3 (0x3C)
#define BHI_SERIALNU (0x40)
#define BHI_SBLANTIROLLVER (0x44)
#define BHI_NUMSEG (0x48)
#define BHI_MSMHWID(n) (0x4C + (0x4 * n))
#define BHI_OEMPKHASH(n) (0x64 + (0x4 * n))
#define BHI_RSVD5 (0xC4)
#define BHI_STATUS_MASK (0xC0000000)
#define BHI_STATUS_SHIFT (30)
#define BHI_STATUS_ERROR (3)
#define BHI_STATUS_SUCCESS (2)
#define BHI_STATUS_RESET (0)

/* MHI BHIE offsets */
#define BHIE_MSMSOCID_OFFS (0x0000)
#define BHIE_TXVECADDR_LOW_OFFS (0x002C)
#define BHIE_TXVECADDR_HIGH_OFFS (0x0030)
#define BHIE_TXVECSIZE_OFFS (0x0034)
#define BHIE_TXVECDB_OFFS (0x003C)
#define BHIE_TXVECDB_SEQNUM_BMSK (0x3FFFFFFF)
#define BHIE_TXVECDB_SEQNUM_SHFT (0)
#define BHIE_TXVECSTATUS_OFFS (0x0044)
#define BHIE_TXVECSTATUS_SEQNUM_BMSK (0x3FFFFFFF)
#define BHIE_TXVECSTATUS_SEQNUM_SHFT (0)
#define BHIE_TXVECSTATUS_STATUS_BMSK (0xC0000000)
#define BHIE_TXVECSTATUS_STATUS_SHFT (30)
#define BHIE_TXVECSTATUS_STATUS_RESET (0x00)
#define BHIE_TXVECSTATUS_STATUS_XFER_COMPL (0x02)
#define BHIE_TXVECSTATUS_STATUS_ERROR (0x03)
#define BHIE_RXVECADDR_LOW_OFFS (0x0060)
#define BHIE_RXVECADDR_HIGH_OFFS (0x0064)
#define BHIE_RXVECSIZE_OFFS (0x0068)
#define BHIE_RXVECDB_OFFS (0x0070)
#define BHIE_RXVECDB_SEQNUM_BMSK (0x3FFFFFFF)
#define BHIE_RXVECDB_SEQNUM_SHFT (0)
#define BHIE_RXVECSTATUS_OFFS (0x0078)
#define BHIE_RXVECSTATUS_SEQNUM_BMSK (0x3FFFFFFF)
#define BHIE_RXVECSTATUS_SEQNUM_SHFT (0)
#define BHIE_RXVECSTATUS_STATUS_BMSK (0xC0000000)
#define BHIE_RXVECSTATUS_STATUS_SHFT (30)
#define BHIE_RXVECSTATUS_STATUS_RESET (0x00)
#define BHIE_RXVECSTATUS_STATUS_XFER_COMPL (0x02)
#define BHIE_RXVECSTATUS_STATUS_ERROR (0x03)

/* convert ticks to micro seconds by dividing by 19.2 */
#define TIME_TICKS_TO_US(x) (div_u64((x) * 10, 192))

struct mhi_event_ctxt {
	u32 reserved : 8;
	u32 intmodc : 8;
	u32 intmodt : 16;
	u32 ertype;
	u32 msivec;

	u64 rbase __packed __aligned(4);
	u64 rlen __packed __aligned(4);
	u64 rp __packed __aligned(4);
	u64 wp __packed __aligned(4);
};

struct mhi_chan_ctxt {
	u32 chstate : 8;
	u32 brstmode : 2;
	u32 pollcfg : 6;
	u32 reserved : 16;
	u32 chtype;
	u32 erindex;

	u64 rbase __packed __aligned(4);
	u64 rlen __packed __aligned(4);
	u64 rp __packed __aligned(4);
	u64 wp __packed __aligned(4);
};

struct mhi_cmd_ctxt {
	u32 reserved0;
	u32 reserved1;
	u32 reserved2;

	u64 rbase __packed __aligned(4);
	u64 rlen __packed __aligned(4);
	u64 rp __packed __aligned(4);
	u64 wp __packed __aligned(4);
};

struct mhi_tre {
	u64 ptr;
	u32 dword[2];
};

/* Channel context state */
enum mhi_dev_ch_ctx_state {
	MHI_DEV_CH_STATE_DISABLED,
	MHI_DEV_CH_STATE_ENABLED,
	MHI_DEV_CH_STATE_RUNNING,
	MHI_DEV_CH_STATE_SUSPENDED,
	MHI_DEV_CH_STATE_STOP,
	MHI_DEV_CH_STATE_ERROR,
	MHI_DEV_CH_STATE_RESERVED,
	MHI_DEV_CH_STATE_32BIT = 0x7FFFFFFF
};

/* Channel type */
enum mhi_dev_ch_ctx_type {
	MHI_DEV_CH_TYPE_NONE,
	MHI_DEV_CH_TYPE_OUTBOUND_CHANNEL,
	MHI_DEV_CH_TYPE_INBOUND_CHANNEL,
	MHI_DEV_CH_RESERVED
};

/* Channel context type */
struct mhi_dev_ch_ctx {
	enum mhi_dev_ch_ctx_state	ch_state;
	enum mhi_dev_ch_ctx_type	ch_type;
	uint32_t			err_indx;
	uint64_t			rbase;
	uint64_t			rlen;
	uint64_t			rp;
	uint64_t			wp;
} __packed;

enum mhi_dev_ring_element_type_id {
	MHI_DEV_RING_EL_INVALID = 0,
	MHI_DEV_RING_EL_NOOP = 1,
	MHI_DEV_RING_EL_TRANSFER = 2,
	MHI_DEV_RING_EL_RESET = 16,
	MHI_DEV_RING_EL_STOP = 17,
	MHI_DEV_RING_EL_START = 18,
	MHI_DEV_RING_EL_MHI_STATE_CHG = 32,
	MHI_DEV_RING_EL_CMD_COMPLETION_EVT = 33,
	MHI_DEV_RING_EL_TRANSFER_COMPLETION_EVENT = 34,
	MHI_DEV_RING_EL_EE_STATE_CHANGE_NOTIFY = 64,
	MHI_DEV_RING_EL_UNDEF
};

enum mhi_dev_ring_state {
	RING_STATE_UINT = 0,
	RING_STATE_IDLE,
	RING_STATE_PENDING,
};

enum mhi_dev_ring_type {
	RING_TYPE_CMD = 0,
	RING_TYPE_ER,
	RING_TYPE_CH,
	RING_TYPE_INVAL
};

/* Event context interrupt moderation */
enum mhi_dev_evt_ctx_int_mod_timer {
	MHI_DEV_EVT_INT_MODERATION_DISABLED
};

/* Event ring type */
enum mhi_dev_evt_ctx_event_ring_type {
	MHI_DEV_EVT_TYPE_DEFAULT,
	MHI_DEV_EVT_TYPE_VALID,
	MHI_DEV_EVT_RESERVED
};

/* Event ring context type */
struct mhi_dev_ev_ctx {
	uint32_t				res1:16;
	enum mhi_dev_evt_ctx_int_mod_timer	intmodt:16;
	enum mhi_dev_evt_ctx_event_ring_type	ertype;
	uint32_t				msivec;
	uint64_t				rbase;
	uint64_t				rlen;
	uint64_t				rp;
	uint64_t				wp;
} __packed;

/* Command context */
struct mhi_dev_cmd_ctx {
	uint32_t				res1;
	uint32_t				res2;
	uint32_t				res3;
	uint64_t				rbase;
	uint64_t				rlen;
	uint64_t				rp;
	uint64_t				wp;
} __packed;

/* generic context */
struct mhi_dev_gen_ctx {
	uint32_t				res1;
	uint32_t				res2;
	uint32_t				res3;
	uint64_t				rbase;
	uint64_t				rlen;
	uint64_t				rp;
	uint64_t				wp;
} __packed;

/* Transfer ring element */
struct mhi_dev_transfer_ring_element {
	uint64_t				data_buf_ptr;
	uint32_t				len:16;
	uint32_t				res1:16;
	uint32_t				chain:1;
	uint32_t				res2:7;
	uint32_t				ieob:1;
	uint32_t				ieot:1;
	uint32_t				bei:1;
	uint32_t				res3:5;
	enum mhi_dev_ring_element_type_id	type:8;
	uint32_t				res4:8;
} __packed;

/* Command ring element */
/* Command ring No op command */
struct mhi_dev_cmd_ring_op {
	uint64_t				res1;
	uint32_t				res2;
	uint32_t				res3:16;
	enum mhi_dev_ring_element_type_id	type:8;
	uint32_t				chid:8;
} __packed;

/* Command ring reset channel command */
struct mhi_dev_cmd_ring_reset_channel_cmd {
	uint64_t				res1;
	uint32_t				res2;
	uint32_t				res3:16;
	enum mhi_dev_ring_element_type_id	type:8;
	uint32_t				chid:8;
} __packed;

/* Command ring stop channel command */
struct mhi_dev_cmd_ring_stop_channel_cmd {
	uint64_t				res1;
	uint32_t				res2;
	uint32_t				res3:16;
	enum mhi_dev_ring_element_type_id	type:8;
	uint32_t				chid:8;
} __packed;

/* Command ring start channel command */
struct mhi_dev_cmd_ring_start_channel_cmd {
	uint64_t				res1;
	uint32_t				seqnum;
	uint32_t				reliable:1;
	uint32_t				res2:15;
	enum mhi_dev_ring_element_type_id	type:8;
	uint32_t				chid:8;
} __packed;

enum mhi_dev_cmd_completion_code {
	MHI_CMD_COMPL_CODE_INVALID = 0,
	MHI_CMD_COMPL_CODE_SUCCESS = 1,
	MHI_CMD_COMPL_CODE_EOT = 2,
	MHI_CMD_COMPL_CODE_OVERFLOW = 3,
	MHI_CMD_COMPL_CODE_EOB = 4,
	MHI_CMD_COMPL_CODE_UNDEFINED = 16,
	MHI_CMD_COMPL_CODE_RING_EL = 17,
	MHI_CMD_COMPL_CODE_RES
};

/* Event ring elements */
/* Transfer completion event */
struct mhi_dev_event_ring_transfer_completion {
	uint64_t				ptr;
	uint32_t				len:16;
	uint32_t				res1:8;
	enum mhi_dev_cmd_completion_code	code:8;
	uint32_t				res2:16;
	enum mhi_dev_ring_element_type_id	type:8;
	uint32_t				chid:8;
} __packed;

/* Command completion event */
struct mhi_dev_event_ring_cmd_completion {
	uint64_t				ptr;
	uint32_t				res1:24;
	enum mhi_dev_cmd_completion_code	code:8;
	uint32_t				res2:16;
	enum mhi_dev_ring_element_type_id	type:8;
	uint32_t				res3:8;
} __packed;

/* MHI state change event */
struct mhi_dev_event_ring_state_change {
	uint64_t				ptr;
	uint32_t				res1:24;
	uint32_t /*enum mhi_dev_state*/			mhistate:8;
	uint32_t				res2:16;
	enum mhi_dev_ring_element_type_id	type:8;
	uint32_t				res3:8;
} __packed;

enum mhi_dev_execenv {
	MHI_DEV_SBL_EE = 1,
	MHI_DEV_AMSS_EE = 2,
	MHI_DEV_UNRESERVED
};

/* EE state change event */
struct mhi_dev_event_ring_ee_state_change {
	uint64_t				ptr;
	uint32_t				res1:24;
	enum mhi_dev_execenv			execenv:8;
	uint32_t				res2:16;
	enum mhi_dev_ring_element_type_id	type:8;
	uint32_t				res3:8;
} __packed;

/* Generic cmd to parse common details like type and channel id */
struct mhi_dev_ring_generic {
	uint64_t				ptr;
	uint32_t				res1:24;
	uint32_t /*enum mhi_dev_state*/			mhistate:8;
	uint32_t				res2:16;
	enum mhi_dev_ring_element_type_id	type:8;
	uint32_t				chid:8;
} __packed;

struct mhi_config {
	uint32_t	mhi_reg_len;
	uint32_t	version;
	uint32_t	event_rings;
	uint32_t	channels;
	uint32_t	chdb_offset;
	uint32_t	erdb_offset;
};

/* Possible ring element types */
union mhi_dev_ring_element_type {
	struct mhi_dev_cmd_ring_op			cmd_no_op;
	struct mhi_dev_cmd_ring_reset_channel_cmd	cmd_reset;
	struct mhi_dev_cmd_ring_stop_channel_cmd	cmd_stop;
	struct mhi_dev_cmd_ring_start_channel_cmd	cmd_start;
	struct mhi_dev_transfer_ring_element		cmd_transfer;
	struct mhi_dev_event_ring_transfer_completion	evt_tr_comp;
	struct mhi_dev_event_ring_cmd_completion	evt_cmd_comp;
	struct mhi_dev_event_ring_state_change		evt_state_change;
	struct mhi_dev_event_ring_ee_state_change	evt_ee_state;
	struct mhi_dev_ring_generic			generic;
};

struct bhi_vec_entry {
	u64 dma_addr;
	u64 size;
};

enum mhi_cmd_type {
	MHI_CMD_TYPE_NOP = 1,
	MHI_CMD_TYPE_RESET = 16,
	MHI_CMD_TYPE_STOP = 17,
	MHI_CMD_TYPE_START = 18,
	MHI_CMD_TYPE_TSYNC = 24,
};

/* no operation command */
#define MHI_TRE_CMD_NOOP_PTR (0)
#define MHI_TRE_CMD_NOOP_DWORD0 (0)
#define MHI_TRE_CMD_NOOP_DWORD1 (MHI_CMD_TYPE_NOP << 16)

/* channel reset command */
#define MHI_TRE_CMD_RESET_PTR (0)
#define MHI_TRE_CMD_RESET_DWORD0 (0)
#define MHI_TRE_CMD_RESET_DWORD1(chid) ((chid << 24) | \
					(MHI_CMD_TYPE_RESET << 16))

/* channel stop command */
#define MHI_TRE_CMD_STOP_PTR (0)
#define MHI_TRE_CMD_STOP_DWORD0 (0)
#define MHI_TRE_CMD_STOP_DWORD1(chid) ((chid << 24) | (MHI_CMD_TYPE_STOP << 16))

/* channel start command */
#define MHI_TRE_CMD_START_PTR (0)
#define MHI_TRE_CMD_START_DWORD0 (0)
#define MHI_TRE_CMD_START_DWORD1(chid) ((chid << 24) | \
					(MHI_CMD_TYPE_START << 16))

/* time sync cfg command */
#define MHI_TRE_CMD_TSYNC_CFG_PTR (0)
#define MHI_TRE_CMD_TSYNC_CFG_DWORD0 (0)
#define MHI_TRE_CMD_TSYNC_CFG_DWORD1(er) ((MHI_CMD_TYPE_TSYNC << 16) | \
					  (er << 24))

#define MHI_TRE_GET_CMD_CHID(tre) (((tre)->dword[1] >> 24) & 0xFF)
#define MHI_TRE_GET_CMD_TYPE(tre) (((tre)->dword[1] >> 16) & 0xFF)

/* event descriptor macros */
#define MHI_TRE_EV_PTR(ptr) (ptr)
#define MHI_TRE_EV_DWORD0(code, len) ((code << 24) | len)
#define MHI_TRE_EV_DWORD1(chid, type) ((chid << 24) | (type << 16))
#define MHI_TRE_GET_EV_PTR(tre) ((tre)->ptr)
#define MHI_TRE_GET_EV_CODE(tre) (((tre)->dword[0] >> 24) & 0xFF)
#define MHI_TRE_GET_EV_LEN(tre) ((tre)->dword[0] & 0xFFFF)
#define MHI_TRE_GET_EV_CHID(tre) (((tre)->dword[1] >> 24) & 0xFF)
#define MHI_TRE_GET_EV_TYPE(tre) (((tre)->dword[1] >> 16) & 0xFF)
#define MHI_TRE_GET_EV_STATE(tre) (((tre)->dword[0] >> 24) & 0xFF)
#define MHI_TRE_GET_EV_EXECENV(tre) (((tre)->dword[0] >> 24) & 0xFF)
#define MHI_TRE_GET_EV_SEQ(tre) ((tre)->dword[0])
#define MHI_TRE_GET_EV_TIME(tre) ((tre)->ptr)
#define MHI_TRE_GET_EV_COOKIE(tre) lower_32_bits((tre)->ptr)
#define MHI_TRE_GET_EV_VEID(tre) (((tre)->dword[0] >> 16) & 0xFF)

/* transfer descriptor macros */
#define MHI_TRE_DATA_PTR(ptr) (ptr)
#define MHI_TRE_DATA_DWORD0(len) (len & MHI_MAX_MTU)
#define MHI_TRE_DATA_DWORD1(bei, ieot, ieob, chain) ((2 << 16) | (bei << 10) \
	| (ieot << 9) | (ieob << 8) | chain)

/* rsc transfer descriptor macros */
#define MHI_RSCTRE_DATA_PTR(ptr, len) (((u64)len << 48) | ptr)
#define MHI_RSCTRE_DATA_DWORD0(cookie) (cookie)
#define MHI_RSCTRE_DATA_DWORD1 (MHI_PKT_TYPE_COALESCING << 16)

enum MHI_CMD {
	MHI_CMD_RESET_CHAN,
	MHI_CMD_START_CHAN,
	MHI_CMD_TIMSYNC_CFG,
};

enum MHI_PKT_TYPE {
	MHI_PKT_TYPE_INVALID = 0x0,
	MHI_PKT_TYPE_NOOP_CMD = 0x1,
	MHI_PKT_TYPE_TRANSFER = 0x2,
	MHI_PKT_TYPE_COALESCING = 0x8,
	MHI_PKT_TYPE_RESET_CHAN_CMD = 0x10,
	MHI_PKT_TYPE_STOP_CHAN_CMD = 0x11,
	MHI_PKT_TYPE_START_CHAN_CMD = 0x12,
	MHI_PKT_TYPE_STATE_CHANGE_EVENT = 0x20,
	MHI_PKT_TYPE_CMD_COMPLETION_EVENT = 0x21,
	MHI_PKT_TYPE_TX_EVENT = 0x22,
	MHI_PKT_TYPE_RSC_TX_EVENT = 0x28,
	MHI_PKT_TYPE_EE_EVENT = 0x40,
	MHI_PKT_TYPE_TSYNC_EVENT = 0x48,
	MHI_PKT_TYPE_STALE_EVENT, /* internal event */
};

/* MHI transfer completion events */
enum MHI_EV_CCS {
	MHI_EV_CC_INVALID = 0x0,
	MHI_EV_CC_SUCCESS = 0x1,
	MHI_EV_CC_EOT = 0x2,
	MHI_EV_CC_OVERFLOW = 0x3,
	MHI_EV_CC_EOB = 0x4,
	MHI_EV_CC_OOB = 0x5,
	MHI_EV_CC_DB_MODE = 0x6,
	MHI_EV_CC_UNDEFINED_ERR = 0x10,
	MHI_EV_CC_BAD_TRE = 0x11,
};

enum MHI_CH_STATE {
	MHI_CH_STATE_DISABLED = 0x0,
	MHI_CH_STATE_ENABLED = 0x1,
	MHI_CH_STATE_RUNNING = 0x2,
	MHI_CH_STATE_SUSPENDED = 0x3,
	MHI_CH_STATE_STOP = 0x4,
	MHI_CH_STATE_ERROR = 0x5,
};

enum MHI_BRSTMODE {
	MHI_BRSTMODE_DISABLE = 0x2,
	MHI_BRSTMODE_ENABLE = 0x3,
};

#define MHI_INVALID_BRSTMODE(mode) (mode != MHI_BRSTMODE_DISABLE && \
				    mode != MHI_BRSTMODE_ENABLE)

#define MHI_IN_PBL(ee) (ee == MHI_EE_PBL || ee == MHI_EE_PTHRU || \
			ee == MHI_EE_EDL)

#define MHI_IN_MISSION_MODE(ee) (ee == MHI_EE_AMSS || ee == MHI_EE_WFW)

enum MHI_ST_TRANSITION {
	MHI_ST_TRANSITION_PBL,
	MHI_ST_TRANSITION_READY,
	MHI_ST_TRANSITION_SBL,
	MHI_ST_TRANSITION_MISSION_MODE,
	MHI_ST_TRANSITION_FP,
	MHI_ST_TRANSITION_MAX,
};

extern const char * const mhi_state_tran_str[MHI_ST_TRANSITION_MAX];
#define TO_MHI_STATE_TRANS_STR(state) (((state) >= MHI_ST_TRANSITION_MAX) ? \
				"INVALID_STATE" : mhi_state_tran_str[state])

extern const char * const mhi_state_str[MHI_STATE_MAX];
#define TO_MHI_STATE_STR(state) ((state >= MHI_STATE_MAX || \
				  !mhi_state_str[state]) ? \
				"INVALID_STATE" : mhi_state_str[state])

enum {
	MHI_PM_BIT_DISABLE,
	MHI_PM_BIT_POR,
	MHI_PM_BIT_M0,
	MHI_PM_BIT_M2,
	MHI_PM_BIT_M3_ENTER,
	MHI_PM_BIT_M3,
	MHI_PM_BIT_M3_EXIT,
	MHI_PM_BIT_FW_DL_ERR,
	MHI_PM_BIT_SYS_ERR_DETECT,
	MHI_PM_BIT_SYS_ERR_PROCESS,
	MHI_PM_BIT_SHUTDOWN_PROCESS,
	MHI_PM_BIT_LD_ERR_FATAL_DETECT,
	MHI_PM_BIT_MAX
};

/* internal power states */
enum MHI_PM_STATE {
	MHI_PM_DISABLE = BIT(MHI_PM_BIT_DISABLE), /* MHI is not enabled */
	MHI_PM_POR = BIT(MHI_PM_BIT_POR), /* reset state */
	MHI_PM_M0 = BIT(MHI_PM_BIT_M0),
	MHI_PM_M2 = BIT(MHI_PM_BIT_M2),
	MHI_PM_M3_ENTER = BIT(MHI_PM_BIT_M3_ENTER),
	MHI_PM_M3 = BIT(MHI_PM_BIT_M3),
	MHI_PM_M3_EXIT = BIT(MHI_PM_BIT_M3_EXIT),
	/* firmware download failure state */
	MHI_PM_FW_DL_ERR = BIT(MHI_PM_BIT_FW_DL_ERR),
	MHI_PM_SYS_ERR_DETECT = BIT(MHI_PM_BIT_SYS_ERR_DETECT),
	MHI_PM_SYS_ERR_PROCESS = BIT(MHI_PM_BIT_SYS_ERR_PROCESS),
	MHI_PM_SHUTDOWN_PROCESS = BIT(MHI_PM_BIT_SHUTDOWN_PROCESS),
	/* link not accessible */
	MHI_PM_LD_ERR_FATAL_DETECT = BIT(MHI_PM_BIT_LD_ERR_FATAL_DETECT),
};

#define MHI_REG_ACCESS_VALID(pm_state) ((pm_state & (MHI_PM_POR | MHI_PM_M0 | \
		MHI_PM_M2 | MHI_PM_M3_ENTER | MHI_PM_M3_EXIT | \
		MHI_PM_SYS_ERR_DETECT | MHI_PM_SYS_ERR_PROCESS | \
		MHI_PM_SHUTDOWN_PROCESS | MHI_PM_FW_DL_ERR)))
#define MHI_PM_IN_ERROR_STATE(pm_state) (pm_state >= MHI_PM_FW_DL_ERR)
#define MHI_PM_IN_FATAL_STATE(pm_state) (pm_state == MHI_PM_LD_ERR_FATAL_DETECT)
#define MHI_DB_ACCESS_VALID(pm_state) (pm_state & MHI_PM_M0)
#define MHI_WAKE_DB_CLEAR_VALID(pm_state) (pm_state & (MHI_PM_M0 | \
						MHI_PM_M2 | MHI_PM_M3_EXIT))
#define MHI_WAKE_DB_SET_VALID(pm_state) (pm_state & MHI_PM_M2)
#define MHI_WAKE_DB_FORCE_SET_VALID(pm_state) MHI_WAKE_DB_CLEAR_VALID(pm_state)
#define MHI_EVENT_ACCESS_INVALID(pm_state) (pm_state == MHI_PM_DISABLE || \
					    MHI_PM_IN_ERROR_STATE(pm_state))
#define MHI_PM_IN_SUSPEND_STATE(pm_state) (pm_state & \
					   (MHI_PM_M3_ENTER | MHI_PM_M3))

/* accepted buffer type for the channel */
enum MHI_XFER_TYPE {
	MHI_XFER_BUFFER,
	MHI_XFER_SKB,
	MHI_XFER_SCLIST,
	MHI_XFER_NOP, /* CPU offload channel, host does not accept transfer */
	MHI_XFER_DMA, /* receive dma address, already mapped by client */
	MHI_XFER_RSC_DMA, /* RSC type, accept premapped buffer */
};

#define NR_OF_CMD_RINGS (1)
#define CMD_EL_PER_RING (128)
#define PRIMARY_CMD_RING (0)
#define MHI_DEV_WAKE_DB (127)
#define MHI_MAX_MTU (0xffff)

enum MHI_ER_TYPE {
	MHI_ER_TYPE_INVALID = 0x0,
	MHI_ER_TYPE_VALID = 0x1,
};

enum mhi_er_data_type {
	MHI_ER_DATA_ELEMENT_TYPE,
	MHI_ER_CTRL_ELEMENT_TYPE,
	MHI_ER_TSYNC_ELEMENT_TYPE,
	MHI_ER_DATA_TYPE_MAX = MHI_ER_TSYNC_ELEMENT_TYPE,
};

enum mhi_ch_ee_mask {
	MHI_CH_EE_PBL = BIT(MHI_EE_PBL),
	MHI_CH_EE_SBL = BIT(MHI_EE_SBL),
	MHI_CH_EE_AMSS = BIT(MHI_EE_AMSS),
	MHI_CH_EE_RDDM = BIT(MHI_EE_RDDM),
	MHI_CH_EE_PTHRU = BIT(MHI_EE_PTHRU),
	MHI_CH_EE_WFW = BIT(MHI_EE_WFW),
	MHI_CH_EE_EDL = BIT(MHI_EE_EDL),
};

enum mhi_ch_type {
	MHI_CH_TYPE_INVALID = 0,
	MHI_CH_TYPE_OUTBOUND = DMA_TO_DEVICE,
	MHI_CH_TYPE_INBOUND = DMA_FROM_DEVICE,
	MHI_CH_TYPE_INBOUND_COALESCED = 3,
};

struct db_cfg {
	bool reset_req;
	bool db_mode;
	u32 pollcfg;
	enum MHI_BRSTMODE brstmode;
	dma_addr_t db_val;
	void (*process_db)(struct mhi_controller *mhi_cntrl,
			   struct db_cfg *db_cfg, void __iomem *io_addr,
			   dma_addr_t db_val);
};

struct mhi_pm_transitions {
	enum MHI_PM_STATE from_state;
	u32 to_states;
};

struct state_transition {
	struct list_head node;
	enum MHI_ST_TRANSITION state;
};

/* Control Segment */
struct mhi_ctrl_seg
{
   struct mhi_tre hw_in_chan_ring[NUM_MHI_IPA_IN_RING_ELEMENTS]  __packed __aligned(NUM_MHI_IPA_IN_RING_ELEMENTS*16);
   struct mhi_tre hw_out_chan_ring[NUM_MHI_IPA_OUT_RING_ELEMENTS]  __packed __aligned(NUM_MHI_IPA_OUT_RING_ELEMENTS*16);
#ifdef ENABLE_IP_SW0
   struct mhi_tre sw_in_chan_ring[NUM_MHI_SW_IP_RING_ELEMENTS]  __packed __aligned(NUM_MHI_IPA_IN_RING_ELEMENTS*16);
   struct mhi_tre sw_out_chan_ring[NUM_MHI_SW_IP_RING_ELEMENTS]  __packed __aligned(NUM_MHI_IPA_OUT_RING_ELEMENTS*16);
#endif
   struct mhi_tre diag_in_chan_ring[NUM_MHI_DIAG_IN_RING_ELEMENTS]  __packed __aligned(NUM_MHI_IPA_OUT_RING_ELEMENTS*16);
   struct mhi_tre chan_ring[NUM_MHI_CHAN_RING_ELEMENTS*2*12]  __packed __aligned(NUM_MHI_CHAN_RING_ELEMENTS*16);
   struct mhi_tre event_ring[NUM_MHI_EVT_RINGS][NUM_MHI_EVT_RING_ELEMENTS]  __packed __aligned(NUM_MHI_EVT_RING_ELEMENTS*16);
   struct mhi_tre cmd_ring[NR_OF_CMD_RINGS][CMD_EL_PER_RING]  __packed __aligned(CMD_EL_PER_RING*16);

   struct mhi_chan_ctxt chan_ctxt[NUM_MHI_XFER_RINGS] __aligned(128);
   struct mhi_event_ctxt er_ctxt[NUM_MHI_EVT_RINGS]  __aligned(128);
   struct mhi_cmd_ctxt cmd_ctxt[NR_OF_CMD_RINGS]  __aligned(128);
} __aligned(4096);

struct mhi_ctxt {
	struct mhi_event_ctxt *er_ctxt;
	struct mhi_chan_ctxt *chan_ctxt;
	struct mhi_cmd_ctxt *cmd_ctxt;
	dma_addr_t er_ctxt_addr;
	dma_addr_t chan_ctxt_addr;
	dma_addr_t cmd_ctxt_addr;
	struct mhi_ctrl_seg *ctrl_seg;
	dma_addr_t ctrl_seg_addr;
};

struct mhi_ring {
	dma_addr_t dma_handle;
	dma_addr_t iommu_base;
	u64 *ctxt_wp; /* point to ctxt wp */
	void *pre_aligned;
	void *base;
	void *rp;
	void *wp;
	size_t el_size;
	size_t len;
	size_t elements;
	size_t alloc_size;
	void __iomem *db_addr;
};

struct mhi_cmd {
	struct mhi_ring ring;
	spinlock_t lock;
};

struct mhi_buf_info {
	dma_addr_t p_addr;
	void *v_addr;
	void *bb_addr;
	void *wp;
	size_t len;
	void *cb_buf;
	bool used; /* indicate element is free to use */
	bool pre_mapped; /* already pre-mapped by client */
	enum dma_data_direction dir;
};

struct mhi_event {
	u32 er_index;
	u32 intmod;
	u32 msi;
	int chan; /* this event ring is dedicated to a channel */
	u32 priority;
	enum mhi_er_data_type data_type;
	struct mhi_ring ring;
	struct db_cfg db_cfg;
	u32 used_elements;
	bool hw_ring;
	bool cl_manage;
	bool offload_ev; /* managed by a device driver */
	spinlock_t lock;
	struct mhi_chan *mhi_chan; /* dedicated to channel */
	struct tasklet_struct task;
	int (*process_event)(struct mhi_controller *mhi_cntrl,
			     struct mhi_event *mhi_event,
			     u32 event_quota);
	struct mhi_controller *mhi_cntrl;
};

struct mhi_chan {
	u32 chan;
	const char *name;
	/*
	 * important, when consuming increment tre_ring first, when releasing
	 * decrement buf_ring first. If tre_ring has space, buf_ring
	 * guranteed to have space so we do not need to check both rings.
	 */
	struct mhi_ring buf_ring;
	struct mhi_ring tre_ring;

	u32 used_elements;
	u32 used_events[MHI_EV_CC_DB_MODE+1];

	u32 er_index;
	u32 intmod;
	enum mhi_ch_type type;
	enum dma_data_direction dir;
	struct db_cfg db_cfg;
	u32 ee_mask;
	enum MHI_XFER_TYPE xfer_type;
	enum MHI_CH_STATE ch_state;
	enum MHI_EV_CCS ccs;
	bool lpm_notify;
	bool configured;
	bool offload_ch;
	bool pre_alloc;
	bool auto_start;
	bool wake_capable; /* channel should wake up system */
	/* functions that generate the transfer ring elements */
	int (*gen_tre)(struct mhi_controller *mhi_cntrl,
		       struct mhi_chan *mhi_chan, void *buf, void *cb,
		       size_t len, enum MHI_FLAGS flags);
	int (*queue_xfer)(struct mhi_device *mhi_dev,
			  struct mhi_chan *mhi_chan, void *buf,
			  size_t len, enum MHI_FLAGS flags);
	/* xfer call back */
	struct mhi_device *mhi_dev;
	void (*xfer_cb)(struct mhi_device *mhi_dev, struct mhi_result *result);
	struct mutex mutex;
	struct completion completion;
	rwlock_t lock;
	u32 ring;
	u32 tiocm;
	struct list_head node;
};

struct tsync_node {
	struct list_head node;
	u32 sequence;
	u64 local_time;
	u64 remote_time;
	struct mhi_device *mhi_dev;
	void (*cb_func)(struct mhi_device *mhi_dev, u32 sequence,
			u64 local_time, u64 remote_time);
};

struct mhi_timesync {
	u32 er_index;
	void __iomem *db;
	void __iomem *time_reg;
	enum MHI_EV_CCS ccs;
	struct completion completion;
	spinlock_t lock; /* list protection */
	struct mutex lpm_mutex; /* lpm protection */
	struct list_head head;
};

struct mhi_bus {
	struct list_head controller_list;
	struct mutex lock;
	struct dentry *dentry;
};

/* default MHI timeout */
#define MHI_TIMEOUT_MS (3000)
extern struct mhi_bus mhi_bus;

/* debug fs related functions */
int mhi_debugfs_mhi_chan_show(struct seq_file *m, void *d);
int mhi_debugfs_mhi_event_show(struct seq_file *m, void *d);
int mhi_debugfs_mhi_states_show(struct seq_file *m, void *d);
int mhi_debugfs_trigger_reset(void *data, u64 val);

void mhi_deinit_debugfs(struct mhi_controller *mhi_cntrl);
void mhi_init_debugfs(struct mhi_controller *mhi_cntrl);

/* power management apis */
enum MHI_PM_STATE __must_check mhi_tryset_pm_state(
					struct mhi_controller *mhi_cntrl,
					enum MHI_PM_STATE state);
const char *to_mhi_pm_state_str(enum MHI_PM_STATE state);
void mhi_reset_chan(struct mhi_controller *mhi_cntrl,
		    struct mhi_chan *mhi_chan);
enum mhi_ee mhi_get_exec_env(struct mhi_controller *mhi_cntrl);
int mhi_queue_state_transition(struct mhi_controller *mhi_cntrl,
			       enum MHI_ST_TRANSITION state);
void mhi_pm_st_worker(struct work_struct *work);
void mhi_fw_load_worker(struct work_struct *work);
void mhi_pm_sys_err_worker(struct work_struct *work);
void mhi_pm_ready_worker(struct work_struct *work);
int mhi_ready_state_transition(struct mhi_controller *mhi_cntrl);
void mhi_ctrl_ev_task(unsigned long data);
int mhi_pm_m0_transition(struct mhi_controller *mhi_cntrl);
void mhi_pm_m1_transition(struct mhi_controller *mhi_cntrl);
int mhi_pm_m3_transition(struct mhi_controller *mhi_cntrl);
void mhi_notify(struct mhi_device *mhi_dev, enum MHI_CB cb_reason);
int mhi_process_data_event_ring(struct mhi_controller *mhi_cntrl,
				struct mhi_event *mhi_event, u32 event_quota);
int mhi_process_ctrl_ev_ring(struct mhi_controller *mhi_cntrl,
			     struct mhi_event *mhi_event, u32 event_quota);
int mhi_process_tsync_event_ring(struct mhi_controller *mhi_cntrl,
				 struct mhi_event *mhi_event, u32 event_quota);
int mhi_send_cmd(struct mhi_controller *mhi_cntrl, struct mhi_chan *mhi_chan,
		 enum MHI_CMD cmd);
int __mhi_device_get_sync(struct mhi_controller *mhi_cntrl);

/* queue transfer buffer */
int mhi_gen_tre(struct mhi_controller *mhi_cntrl, struct mhi_chan *mhi_chan,
		void *buf, void *cb, size_t buf_len, enum MHI_FLAGS flags);
int mhi_queue_buf(struct mhi_device *mhi_dev, struct mhi_chan *mhi_chan,
		  void *buf, size_t len, enum MHI_FLAGS mflags);
int mhi_queue_skb(struct mhi_device *mhi_dev, struct mhi_chan *mhi_chan,
		  void *buf, size_t len, enum MHI_FLAGS mflags);
int mhi_queue_sclist(struct mhi_device *mhi_dev, struct mhi_chan *mhi_chan,
		  void *buf, size_t len, enum MHI_FLAGS mflags);
int mhi_queue_nop(struct mhi_device *mhi_dev, struct mhi_chan *mhi_chan,
		  void *buf, size_t len, enum MHI_FLAGS mflags);
int mhi_queue_dma(struct mhi_device *mhi_dev, struct mhi_chan *mhi_chan,
		  void *buf, size_t len, enum MHI_FLAGS mflags);

/* register access methods */
void mhi_db_brstmode(struct mhi_controller *mhi_cntrl, struct db_cfg *db_cfg,
		     void __iomem *db_addr, dma_addr_t wp);
void mhi_db_brstmode_disable(struct mhi_controller *mhi_cntrl,
			     struct db_cfg *db_mode, void __iomem *db_addr,
			     dma_addr_t wp);
int __must_check mhi_read_reg(struct mhi_controller *mhi_cntrl,
			      void __iomem *base, u32 offset, u32 *out);
int __must_check mhi_read_reg_field(struct mhi_controller *mhi_cntrl,
				    void __iomem *base, u32 offset, u32 mask,
				    u32 shift, u32 *out);
void mhi_write_reg(struct mhi_controller *mhi_cntrl, void __iomem *base,
		   u32 offset, u32 val);
void mhi_write_reg_field(struct mhi_controller *mhi_cntrl, void __iomem *base,
			 u32 offset, u32 mask, u32 shift, u32 val);
void mhi_ring_er_db(struct mhi_event *mhi_event);
void mhi_write_db(struct mhi_controller *mhi_cntrl, void __iomem *db_addr,
		  dma_addr_t wp);
void mhi_ring_cmd_db(struct mhi_controller *mhi_cntrl, struct mhi_cmd *mhi_cmd);
void mhi_ring_chan_db(struct mhi_controller *mhi_cntrl,
		      struct mhi_chan *mhi_chan);
int mhi_get_capability_offset(struct mhi_controller *mhi_cntrl, u32 capability,
			      u32 *offset);
int mhi_init_timesync(struct mhi_controller *mhi_cntrl);
int mhi_create_timesync_sysfs(struct mhi_controller *mhi_cntrl);
void mhi_destroy_timesync(struct mhi_controller *mhi_cntrl);

/* memory allocation methods */
static inline void *mhi_alloc_coherent(struct mhi_controller *mhi_cntrl,
				       size_t size,
				       dma_addr_t *dma_handle,
				       gfp_t gfp)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION( 5,0,0 ))
	void *buf = dma_zalloc_coherent(mhi_cntrl->dev, size, dma_handle, gfp);
#else
	void *buf = dma_alloc_coherent(mhi_cntrl->dev, size, dma_handle, gfp | __GFP_ZERO);
#endif

	MHI_LOG("size = %zd, dma_handle = %llx\n", size, (u64)*dma_handle);
	if (buf)
		atomic_add(size, &mhi_cntrl->alloc_size);

	return buf;
}
static inline void mhi_free_coherent(struct mhi_controller *mhi_cntrl,
				     size_t size,
				     void *vaddr,
				     dma_addr_t dma_handle)
{
	atomic_sub(size, &mhi_cntrl->alloc_size);
	dma_free_coherent(mhi_cntrl->dev, size, vaddr, dma_handle);
}
struct mhi_device *mhi_alloc_device(struct mhi_controller *mhi_cntrl);
static inline void mhi_dealloc_device(struct mhi_controller *mhi_cntrl,
				      struct mhi_device *mhi_dev)
{
	kfree(mhi_dev);
}
int mhi_destroy_device(struct device *dev, void *data);
void mhi_create_devices(struct mhi_controller *mhi_cntrl);
int mhi_alloc_bhie_table(struct mhi_controller *mhi_cntrl,
			 struct image_info **image_info, size_t alloc_size);
void mhi_free_bhie_table(struct mhi_controller *mhi_cntrl,
			 struct image_info *image_info);

int mhi_map_single_no_bb(struct mhi_controller *mhi_cntrl,
			 struct mhi_buf_info *buf_info);
int mhi_map_single_use_bb(struct mhi_controller *mhi_cntrl,
			  struct mhi_buf_info *buf_info);
void mhi_unmap_single_no_bb(struct mhi_controller *mhi_cntrl,
			    struct mhi_buf_info *buf_info);
void mhi_unmap_single_use_bb(struct mhi_controller *mhi_cntrl,
			     struct mhi_buf_info *buf_info);

/* initialization methods */
int mhi_init_chan_ctxt(struct mhi_controller *mhi_cntrl,
		       struct mhi_chan *mhi_chan);
void mhi_deinit_chan_ctxt(struct mhi_controller *mhi_cntrl,
			  struct mhi_chan *mhi_chan);
int mhi_init_mmio(struct mhi_controller *mhi_cntrl);
int mhi_init_dev_ctxt(struct mhi_controller *mhi_cntrl);
void mhi_deinit_dev_ctxt(struct mhi_controller *mhi_cntrl);
int mhi_init_irq_setup(struct mhi_controller *mhi_cntrl);
void mhi_deinit_free_irq(struct mhi_controller *mhi_cntrl);
int mhi_dtr_init(void);

/* isr handlers */
irqreturn_t mhi_one_msi_handlr(int irq_number, void *dev);
irqreturn_t mhi_msi_handlr(int irq_number, void *dev);
irqreturn_t mhi_intvec_threaded_handlr(int irq_number, void *dev);
irqreturn_t mhi_intvec_handlr(int irq_number, void *dev);
void mhi_ev_task(unsigned long data);

#ifdef CONFIG_MHI_DEBUG

#define MHI_ASSERT(cond, msg) do { \
	if (cond) \
		panic(msg); \
} while (0)

#else

#define MHI_ASSERT(cond, msg) do { \
	if (cond) { \
		MHI_ERR(msg); \
		WARN_ON(cond); \
	} \
} while (0)

#endif

#endif /* _MHI_INT_H */
