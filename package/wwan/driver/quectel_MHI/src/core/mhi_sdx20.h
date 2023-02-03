#ifndef __SDX20_MHI_H
#define __SDX20_MHI_H

#include <linux/types.h>

/* MHI control data structures alloted by the host, including
 * channel context array, event context array, command context and rings */

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

enum mhi_dev_state {
	MHI_DEV_RESET_STATE = 0,
	MHI_DEV_READY_STATE,
	MHI_DEV_M0_STATE,
	MHI_DEV_M1_STATE,
	MHI_DEV_M2_STATE,
	MHI_DEV_M3_STATE,
	MHI_DEV_MAX_STATE,
	MHI_DEV_SYSERR_STATE = 0xff
};

/* MHI state change event */
struct mhi_dev_event_ring_state_change {
	uint64_t				ptr;
	uint32_t				res1:24;
	enum mhi_dev_state			mhistate:8;
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
	enum mhi_dev_state			mhistate:8;
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

#define NUM_CHANNELS			128
#define HW_CHANNEL_BASE			100
#define HW_CHANNEL_END			107
#define MHI_ENV_VALUE			2
#define MHI_MASK_ROWS_CH_EV_DB		4
#define TRB_MAX_DATA_SIZE		8192
#define MHI_CTRL_STATE			25
#define IPA_DMA_SYNC                    1
#define IPA_DMA_ASYNC                   0

/*maximum trasnfer completion events buffer*/
#define MAX_TR_EVENTS			50
/*maximum event requests */
#define MHI_MAX_EVT_REQ			50

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

/* Transfer ring element type */
union mhi_dev_ring_ctx {
	struct mhi_dev_cmd_ctx		cmd;
	struct mhi_dev_ev_ctx		ev;
	struct mhi_dev_ch_ctx		ch;
	struct mhi_dev_gen_ctx		generic;
};

/* MHI host Control and data address region */
struct mhi_host_addr {
	uint32_t	ctrl_base_lsb;
	uint32_t	ctrl_base_msb;
	uint32_t	ctrl_limit_lsb;
	uint32_t	ctrl_limit_msb;
	uint32_t	data_base_lsb;
	uint32_t	data_base_msb;
	uint32_t	data_limit_lsb;
	uint32_t	data_limit_msb;
};

/* MHI physical and virtual address region */
struct mhi_meminfo {
	struct device	*dev;
	uintptr_t	pa_aligned;
	uintptr_t	pa_unaligned;
	uintptr_t	va_aligned;
	uintptr_t	va_unaligned;
	uintptr_t	size;
};

struct mhi_addr {
	uint64_t	host_pa;
	uintptr_t	device_pa;
	uintptr_t	device_va;
	size_t		size;
	dma_addr_t	phy_addr;
	void		*virt_addr;
	bool		use_ipa_dma;
};

struct mhi_interrupt_state {
	uint32_t	mask;
	uint32_t	status;
};

enum mhi_dev_channel_state {
	MHI_DEV_CH_UNINT,
	MHI_DEV_CH_STARTED,
	MHI_DEV_CH_PENDING_START,
	MHI_DEV_CH_PENDING_STOP,
	MHI_DEV_CH_STOPPED,
	MHI_DEV_CH_CLOSED,
};

enum mhi_dev_ch_operation {
	MHI_DEV_OPEN_CH,
	MHI_DEV_CLOSE_CH,
	MHI_DEV_READ_CH,
	MHI_DEV_READ_WR,
	MHI_DEV_POLL,
};

enum mhi_ctrl_info {
	MHI_STATE_CONFIGURED = 0,
	MHI_STATE_CONNECTED = 1,
	MHI_STATE_DISCONNECTED = 2,
	MHI_STATE_INVAL,
};

enum mhi_dev_tr_compl_evt_type {
	SEND_EVENT_BUFFER,
	SEND_EVENT_RD_OFFSET,
};

enum mhi_dev_transfer_type {
	MHI_DEV_DMA_SYNC,
	MHI_DEV_DMA_ASYNC,
};
#endif /* _SDX20_MHI_H_ */
