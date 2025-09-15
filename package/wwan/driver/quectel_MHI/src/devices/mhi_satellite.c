// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2019, The Linux Foundation. All rights reserved.*/

#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/dma-direction.h>
#include <linux/dma-mapping.h>
#include <linux/errno.h>
#include <linux/ipc_logging.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/rpmsg.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/mhi.h>

#define MHI_SAT_DRIVER_NAME "mhi_satellite"

static bool mhi_sat_defer_init = true; /* set by default */

/* logging macros */
#define IPC_LOG_PAGES (10)
#define IPC_LOG_LVL (MHI_MSG_LVL_INFO)
#define KLOG_LVL (MHI_MSG_LVL_ERROR)

#define MHI_SUBSYS_LOG(fmt, ...) do { \
	if (!subsys) \
		break; \
	if (mhi_sat_driver.klog_lvl <= MHI_MSG_LVL_INFO) \
		pr_info("[I][%s][%s] " fmt, __func__, subsys->name, \
			##__VA_ARGS__);\
	if (subsys->ipc_log && mhi_sat_driver.ipc_log_lvl <= \
	    MHI_MSG_LVL_INFO) \
		ipc_log_string(subsys->ipc_log, "[I][%s] " fmt, __func__, \
			       ##__VA_ARGS__); \
} while (0)

#define MHI_SAT_LOG(fmt, ...) do { \
	if (!subsys || !sat_cntrl) \
		break; \
	if (mhi_sat_driver.klog_lvl <= MHI_MSG_LVL_INFO) \
		pr_info("[I][%s][%s][%x] " fmt, __func__, subsys->name, \
			sat_cntrl->dev_id, ##__VA_ARGS__);\
	if (subsys->ipc_log && mhi_sat_driver.ipc_log_lvl <= \
	    MHI_MSG_LVL_INFO) \
		ipc_log_string(subsys->ipc_log, "[I][%s][%x] " fmt, __func__, \
			       sat_cntrl->dev_id, ##__VA_ARGS__); \
} while (0)

#define MHI_SAT_ERR(fmt, ...) do { \
	if (!subsys || !sat_cntrl) \
		break; \
	if (mhi_sat_driver.klog_lvl <= MHI_MSG_LVL_ERROR) \
		pr_err("[E][%s][%s][%x] " fmt, __func__, subsys->name, \
		       sat_cntrl->dev_id, ##__VA_ARGS__); \
	if (subsys->ipc_log && mhi_sat_driver.ipc_log_lvl <= \
	    MHI_MSG_LVL_ERROR) \
		ipc_log_string(subsys->ipc_log, "[E][%s][%x] " fmt, __func__, \
			       sat_cntrl->dev_id, ##__VA_ARGS__); \
} while (0)

#define MHI_SAT_ASSERT(cond, msg) do { \
	if (cond) \
		panic(msg); \
} while (0)

/* mhi sys error command */
#define MHI_TRE_CMD_SYS_ERR_PTR (0)
#define MHI_TRE_CMD_SYS_ERR_D0 (0)
#define MHI_TRE_CMD_SYS_ERR_D1 (MHI_PKT_TYPE_SYS_ERR_CMD << 16)

/* mhi state change event */
#define MHI_TRE_EVT_MHI_STATE_PTR (0)
#define MHI_TRE_EVT_MHI_STATE_D0(state) (state << 24)
#define MHI_TRE_EVT_MHI_STATE_D1 (MHI_PKT_TYPE_STATE_CHANGE_EVENT << 16)

/* mhi exec env change event */
#define MHI_TRE_EVT_EE_PTR (0)
#define MHI_TRE_EVT_EE_D0(ee) (ee << 24)
#define MHI_TRE_EVT_EE_D1 (MHI_PKT_TYPE_EE_EVENT << 16)

/* mhi config event */
#define MHI_TRE_EVT_CFG_PTR(base_addr) (base_addr)
#define MHI_TRE_EVT_CFG_D0(er_base, num) ((er_base << 16) | (num & 0xFFFF))
#define MHI_TRE_EVT_CFG_D1 (MHI_PKT_TYPE_CFG_EVENT << 16)

/* command completion event */
#define MHI_TRE_EVT_CMD_COMPLETION_PTR(ptr) (ptr)
#define MHI_TRE_EVT_CMD_COMPLETION_D0(code) (code << 24)
#define MHI_TRE_EVT_CMD_COMPLETION_D1 (MHI_PKT_TYPE_CMD_COMPLETION_EVENT << 16)

/* packet parser macros */
#define MHI_TRE_GET_PTR(tre) ((tre)->ptr)
#define MHI_TRE_GET_SIZE(tre) ((tre)->dword[0])
#define MHI_TRE_GET_CCS(tre) (((tre)->dword[0] >> 24) & 0xFF)
#define MHI_TRE_GET_ID(tre) (((tre)->dword[1] >> 24) & 0xFF)
#define MHI_TRE_GET_TYPE(tre) (((tre)->dword[1] >> 16) & 0xFF)
#define MHI_TRE_IS_ER_CTXT_TYPE(tre) (((tre)->dword[1]) & 0x1)

/* creates unique device ID based on connection topology */
#define MHI_SAT_CREATE_DEVICE_ID(dev, domain, bus, slot) \
	((dev & 0xFFFF) << 16 | (domain & 0xF) << 12 | (bus & 0xFF) << 4 | \
	(slot & 0xF))

/* mhi core definitions */
#define MHI_CTXT_TYPE_GENERIC (0xA)

struct __packed mhi_generic_ctxt {
	u32 reserved0;
	u32 type;
	u32 reserved1;
	u64 ctxt_base;
	u64 ctxt_size;
	u64 reserved[2];
};

enum mhi_pkt_type {
	MHI_PKT_TYPE_INVALID = 0x0,
	MHI_PKT_TYPE_RESET_CHAN_CMD = 0x10,
	MHI_PKT_TYPE_STOP_CHAN_CMD = 0x11,
	MHI_PKT_TYPE_START_CHAN_CMD = 0x12,
	MHI_PKT_TYPE_STATE_CHANGE_EVENT = 0x20,
	MHI_PKT_TYPE_CMD_COMPLETION_EVENT = 0x21,
	MHI_PKT_TYPE_EE_EVENT = 0x40,
	MHI_PKT_TYPE_CTXT_UPDATE_CMD = 0x64,
	MHI_PKT_TYPE_IOMMU_MAP_CMD = 0x65,
	MHI_PKT_TYPE_CFG_EVENT = 0x6E,
	MHI_PKT_TYPE_SYS_ERR_CMD = 0xFF,
};

enum mhi_cmd_type {
	MHI_CMD_TYPE_RESET = 0x10,
	MHI_CMD_TYPE_STOP = 0x11,
	MHI_CMD_TYPE_START = 0x12,
};

/* mhi event completion codes */
enum mhi_ev_ccs {
	MHI_EV_CC_INVALID = 0x0,
	MHI_EV_CC_SUCCESS = 0x1,
	MHI_EV_CC_BAD_TRE = 0x11,
};

/* satellite subsystem definitions */
enum subsys_id {
	SUBSYS_ADSP,
	SUBSYS_CDSP,
	SUBSYS_SLPI,
	SUBSYS_MODEM,
	SUBSYS_MAX,
};

static const char * const subsys_names[SUBSYS_MAX] = {
	[SUBSYS_ADSP] = "adsp",
	[SUBSYS_CDSP] = "cdsp",
	[SUBSYS_SLPI] = "slpi",
	[SUBSYS_MODEM] = "modem",
};

struct mhi_sat_subsys {
	const char *name;

	struct rpmsg_device *rpdev; /* rpmsg device */

	/*
	 * acquire either mutex or spinlock to walk controller list
	 * acquire both when modifying list
	 */
	struct list_head cntrl_list; /* controllers list */
	struct mutex cntrl_mutex; /* mutex to walk/modify controllers list */
	spinlock_t cntrl_lock; /* lock to walk/modify controllers list */

	void *ipc_log;
};

/* satellite IPC definitions */
#define SAT_MAJOR_VERSION (1)
#define SAT_MINOR_VERSION (0)
#define SAT_RESERVED_SEQ_NUM (0xFFFF)
#define SAT_MSG_SIZE(n) (sizeof(struct sat_header) + \
			     (n * sizeof(struct sat_tre)))
#define SAT_TRE_SIZE(msg_size) (msg_size  - sizeof(struct sat_header))
#define SAT_TRE_OFFSET(msg) (msg + sizeof(struct sat_header))
#define SAT_TRE_NUM_PKTS(payload_size) ((payload_size) / sizeof(struct sat_tre))

/* satellite IPC msg type */
enum sat_msg_id {
	SAT_MSG_ID_ACK = 0xA,
	SAT_MSG_ID_CMD = 0xC,
	SAT_MSG_ID_EVT = 0xE,
};

/* satellite IPC context type */
enum sat_ctxt_type {
	SAT_CTXT_TYPE_CHAN = 0x0,
	SAT_CTXT_TYPE_EVENT = 0x1,
	SAT_CTXT_TYPE_MAX,
};

/* satellite IPC context string */
#define TO_SAT_CTXT_TYPE_STR(type) (type >= SAT_CTXT_TYPE_MAX ? "INVALID" : \
					sat_ctxt_str[type])

const char * const sat_ctxt_str[SAT_CTXT_TYPE_MAX] = {
	[SAT_CTXT_TYPE_CHAN] = "CCA",
	[SAT_CTXT_TYPE_EVENT] = "ECA",
};

/* satellite IPC transfer ring element */
struct __packed sat_tre {
	u64 ptr;
	u32 dword[2];
};

/* satellite IPC header */
struct __packed sat_header {
	u16 major_ver;
	u16 minor_ver;
	u16 msg_id;
	u16 seq;
	u16 reply_seq;
	u16 payload_size;
	u32 dev_id;
	u8 reserved[8];
};

/* satellite driver definitions */
struct mhi_sat_packet {
	struct list_head node;

	struct mhi_sat_cntrl *cntrl; /* satellite controller reference */
	void *msg; /* incoming message */
};

struct mhi_sat_cntrl {
	struct list_head node;

	struct mhi_controller *mhi_cntrl; /* device MHI controller reference */
	struct mhi_sat_subsys *subsys;

	struct list_head dev_list;
	struct list_head addr_map_list; /* IOMMU mapped addresses list */
	struct mutex list_mutex; /* mutex for devices and address map lists */

	struct list_head packet_list;
	spinlock_t pkt_lock; /* lock to walk/modify received packets list */

	struct work_struct connect_work; /* subsystem connection worker */
	struct work_struct process_work; /* incoming packets processor */

	/* mhi core/controller configurations */
	u32 dev_id; /* unique device ID with BDF as per connection topology */
	int er_base; /* event rings base index */
	int er_max; /* event rings max index */
	int num_er; /* total number of event rings */

	/* satellite controller function counts */
	int num_devices; /* mhi devices current count */
	int max_devices; /* count of maximum devices for subsys/controller */
	u16 seq; /* internal sequence number for all outgoing packets */
	bool active; /* flag set if hello packet/MHI_CFG event was sent */

	/* command completion variables */
	u16 last_cmd_seq; /* sequence number of last sent command packet */
	enum mhi_ev_ccs last_cmd_ccs; /* last command completion event code */
	struct completion completion; /* command completion event wait */
	struct mutex cmd_wait_mutex; /* command completion wait mutex */
};

struct mhi_sat_device {
	struct list_head node;

	struct mhi_device *mhi_dev; /* mhi device pointer */
	struct mhi_sat_cntrl *cntrl; /* parent controller */

	bool chan_started;
};

struct mhi_sat_driver {
	enum MHI_DEBUG_LEVEL ipc_log_lvl; /* IPC log level */
	enum MHI_DEBUG_LEVEL klog_lvl; /* klog/dmesg levels */

	struct mhi_sat_subsys *subsys; /* pointer to subsystem array */
	unsigned int num_subsys;

	struct dentry *dentry; /* debugfs directory */
	bool deferred_init_done; /* flag for deferred init protection */
};

static struct mhi_sat_driver mhi_sat_driver;

static struct mhi_sat_subsys *find_subsys_by_name(const char *name)
{
	int i;
	struct mhi_sat_subsys *subsys = mhi_sat_driver.subsys;

	for (i = 0; i < mhi_sat_driver.num_subsys; i++, subsys++) {
		if (!strcmp(name, subsys->name))
			return subsys;
	}

	return NULL;
}

static struct mhi_sat_cntrl *find_sat_cntrl_by_id(struct mhi_sat_subsys *subsys,
						  u32 dev_id)
{
	struct mhi_sat_cntrl *sat_cntrl;
	unsigned long flags;

	spin_lock_irqsave(&subsys->cntrl_lock, flags);
	list_for_each_entry(sat_cntrl, &subsys->cntrl_list, node) {
		if (sat_cntrl->dev_id == dev_id) {
			spin_unlock_irqrestore(&subsys->cntrl_lock, flags);
			return sat_cntrl;
		}
	}
	spin_unlock_irqrestore(&subsys->cntrl_lock, flags);

	return NULL;
}

static struct mhi_sat_device *find_sat_dev_by_id(
				struct mhi_sat_cntrl *sat_cntrl, int id,
				enum sat_ctxt_type evt)
{
	struct mhi_sat_device *sat_dev;
	int compare_id;

	mutex_lock(&sat_cntrl->list_mutex);
	list_for_each_entry(sat_dev, &sat_cntrl->dev_list, node) {
		compare_id = (evt == SAT_CTXT_TYPE_EVENT) ?
				sat_dev->mhi_dev->dl_event_id :
				sat_dev->mhi_dev->dl_chan_id;

		if (compare_id == id) {
			mutex_unlock(&sat_cntrl->list_mutex);
			return sat_dev;
		}
	}
	mutex_unlock(&sat_cntrl->list_mutex);

	return NULL;
}

static bool mhi_sat_isvalid_header(struct sat_header *hdr, int len)
{
	/* validate payload size */
	if (len >= sizeof(*hdr) && (len != hdr->payload_size + sizeof(*hdr)))
		return false;

	/* validate SAT IPC version */
	if (hdr->major_ver != SAT_MAJOR_VERSION &&
	    hdr->minor_ver != SAT_MINOR_VERSION)
		return false;

	/* validate msg ID */
	if (hdr->msg_id != SAT_MSG_ID_CMD && hdr->msg_id != SAT_MSG_ID_EVT)
		return false;

	return true;
}

static int mhi_sat_wait_cmd_completion(struct mhi_sat_cntrl *sat_cntrl)
{
	struct mhi_sat_subsys *subsys = sat_cntrl->subsys;
	int ret;

	reinit_completion(&sat_cntrl->completion);

	MHI_SAT_LOG("Wait for command completion\n");
	ret = wait_for_completion_timeout(&sat_cntrl->completion,
		msecs_to_jiffies(sat_cntrl->mhi_cntrl->timeout_ms));
	if (!ret || sat_cntrl->last_cmd_ccs != MHI_EV_CC_SUCCESS) {
		MHI_SAT_ERR("Command completion failure:seq:%u:ret:%d:ccs:%d\n",
			sat_cntrl->last_cmd_seq, ret, sat_cntrl->last_cmd_ccs);
		return -EIO;
	}

	MHI_SAT_LOG("Command completion successful for seq:%u\n",
		    sat_cntrl->last_cmd_seq);

	return 0;
}

static int mhi_sat_send_msg(struct mhi_sat_cntrl *sat_cntrl,
			    enum sat_msg_id type, u16 reply_seq,
			    void *msg, u16 msg_size)
{
	struct mhi_sat_subsys *subsys = sat_cntrl->subsys;
	struct sat_header *hdr = msg;

	/* create sequence number for controller */
	sat_cntrl->seq++;
	if (sat_cntrl->seq == SAT_RESERVED_SEQ_NUM)
		sat_cntrl->seq = 0;

	/* populate header */
	hdr->major_ver = SAT_MAJOR_VERSION;
	hdr->minor_ver = SAT_MINOR_VERSION;
	hdr->msg_id = type;
	hdr->seq = sat_cntrl->seq;
	hdr->reply_seq = reply_seq;
	hdr->payload_size = SAT_TRE_SIZE(msg_size);
	hdr->dev_id = sat_cntrl->dev_id;

	/* save last sent command sequence number for completion event */
	if (type == SAT_MSG_ID_CMD)
		sat_cntrl->last_cmd_seq = sat_cntrl->seq;

	return rpmsg_send(subsys->rpdev->ept, msg, msg_size);
}

static void mhi_sat_process_cmds(struct mhi_sat_cntrl *sat_cntrl,
				 struct sat_header *hdr, struct sat_tre *pkt)
{
	struct mhi_sat_subsys *subsys = sat_cntrl->subsys;
	int num_pkts = SAT_TRE_NUM_PKTS(hdr->payload_size), i;

	for (i = 0; i < num_pkts; i++, pkt++) {
		enum mhi_ev_ccs code = MHI_EV_CC_INVALID;

		switch (MHI_TRE_GET_TYPE(pkt)) {
		case MHI_PKT_TYPE_IOMMU_MAP_CMD:
		{
			struct mhi_buf *buf;
			struct mhi_controller *mhi_cntrl = sat_cntrl->mhi_cntrl;
			dma_addr_t iova = DMA_ERROR_CODE;

			buf = kmalloc(sizeof(*buf), GFP_ATOMIC);
			if (!buf)
				goto iommu_map_cmd_completion;

			buf->phys_addr = MHI_TRE_GET_PTR(pkt);
			buf->len = MHI_TRE_GET_SIZE(pkt);

			iova = dma_map_resource(mhi_cntrl->dev, buf->phys_addr,
						buf->len, DMA_BIDIRECTIONAL, 0);
			if (dma_mapping_error(mhi_cntrl->dev, iova)) {
				kfree(buf);
				goto iommu_map_cmd_completion;
			}

			buf->dma_addr = iova;

			mutex_lock(&sat_cntrl->list_mutex);
			list_add_tail(&buf->node,
				      &sat_cntrl->addr_map_list);
			mutex_unlock(&sat_cntrl->list_mutex);

			code = MHI_EV_CC_SUCCESS;

iommu_map_cmd_completion:
			MHI_SAT_LOG("IOMMU MAP 0x%llx CMD processing %s\n",
				   MHI_TRE_GET_PTR(pkt),
				   (code == MHI_EV_CC_SUCCESS) ? "successful" :
				   "failed");

			pkt->ptr = MHI_TRE_EVT_CMD_COMPLETION_PTR(iova);
			pkt->dword[0] = MHI_TRE_EVT_CMD_COMPLETION_D0(code);
			pkt->dword[1] = MHI_TRE_EVT_CMD_COMPLETION_D1;
			break;
		}
		case MHI_PKT_TYPE_CTXT_UPDATE_CMD:
		{
			u64 ctxt_ptr = MHI_TRE_GET_PTR(pkt);
			u64 ctxt_size = MHI_TRE_GET_SIZE(pkt);
			int id = MHI_TRE_GET_ID(pkt);
			enum sat_ctxt_type evt = MHI_TRE_IS_ER_CTXT_TYPE(pkt);
			struct mhi_generic_ctxt gen_ctxt;
			struct mhi_buf buf;
			struct mhi_sat_device *sat_dev = find_sat_dev_by_id(
							 sat_cntrl, id, evt);
			int ret;

			MHI_SAT_ASSERT(!sat_dev,
				      "No device with given chan/evt ID");

			memset(&gen_ctxt, 0, sizeof(gen_ctxt));
			memset(&buf, 0, sizeof(buf));

			gen_ctxt.type = MHI_CTXT_TYPE_GENERIC;
			gen_ctxt.ctxt_base = ctxt_ptr;
			gen_ctxt.ctxt_size = ctxt_size;

			buf.buf = &gen_ctxt;
			buf.len = sizeof(gen_ctxt);
			buf.name = TO_SAT_CTXT_TYPE_STR(evt);

			ret = mhi_device_configure(sat_dev->mhi_dev,
						   DMA_BIDIRECTIONAL, &buf, 1);
			if (!ret)
				code = MHI_EV_CC_SUCCESS;

			MHI_SAT_LOG("CTXT UPDATE CMD %s:%d processing %s\n",
				buf.name, id, (code == MHI_EV_CC_SUCCESS) ?
				"successful" : "failed");

			pkt->ptr = MHI_TRE_EVT_CMD_COMPLETION_PTR(0);
			pkt->dword[0] = MHI_TRE_EVT_CMD_COMPLETION_D0(code);
			pkt->dword[1] = MHI_TRE_EVT_CMD_COMPLETION_D1;
			break;
		}
		case MHI_PKT_TYPE_START_CHAN_CMD:
		{
			int id = MHI_TRE_GET_ID(pkt);
			struct mhi_sat_device *sat_dev = find_sat_dev_by_id(
							 sat_cntrl, id,
							 SAT_CTXT_TYPE_CHAN);
			int ret;

			MHI_SAT_ASSERT(!sat_dev,
				      "No device with given channel ID\n");

			MHI_SAT_ASSERT(sat_dev->chan_started,
				       "Channel already started!");

			ret = mhi_prepare_for_transfer(sat_dev->mhi_dev);
			if (!ret) {
				sat_dev->chan_started = true;
				code = MHI_EV_CC_SUCCESS;
			}

			MHI_SAT_LOG("START CHANNEL %d CMD processing %s\n",
				id, (code == MHI_EV_CC_SUCCESS) ? "successful" :
				"failure");

			pkt->ptr = MHI_TRE_EVT_CMD_COMPLETION_PTR(0);
			pkt->dword[0] = MHI_TRE_EVT_CMD_COMPLETION_D0(code);
			pkt->dword[1] = MHI_TRE_EVT_CMD_COMPLETION_D1;
			break;
		}
		case MHI_PKT_TYPE_RESET_CHAN_CMD:
		{
			int id = MHI_TRE_GET_ID(pkt);
			struct mhi_sat_device *sat_dev =
				find_sat_dev_by_id(sat_cntrl, id,
						   SAT_CTXT_TYPE_CHAN);

			MHI_SAT_ASSERT(!sat_dev,
				"No device with given channel ID\n");

			MHI_SAT_ASSERT(!sat_dev->chan_started,
				"Resetting unstarted channel!");

			mhi_unprepare_from_transfer(sat_dev->mhi_dev);
			sat_dev->chan_started = false;

			MHI_SAT_LOG(
				"RESET CHANNEL %d CMD processing successful\n",
				id);

			pkt->ptr = MHI_TRE_EVT_CMD_COMPLETION_PTR(0);
			pkt->dword[0] = MHI_TRE_EVT_CMD_COMPLETION_D0(
					MHI_EV_CC_SUCCESS);
			pkt->dword[1] = MHI_TRE_EVT_CMD_COMPLETION_D1;
			break;
		}
		default:
			MHI_SAT_ASSERT(1, "Unhandled command!");
			break;
		}
	}
}

static void mhi_sat_process_worker(struct work_struct *work)
{
	struct mhi_sat_cntrl *sat_cntrl = container_of(work,
					struct mhi_sat_cntrl, process_work);
	struct mhi_sat_subsys *subsys = sat_cntrl->subsys;
	struct mhi_sat_packet *packet, *tmp;
	struct sat_header *hdr;
	struct sat_tre *pkt;
	LIST_HEAD(head);

	MHI_SAT_LOG("Entered\n");

	spin_lock_irq(&sat_cntrl->pkt_lock);
	list_splice_tail_init(&sat_cntrl->packet_list, &head);
	spin_unlock_irq(&sat_cntrl->pkt_lock);

	list_for_each_entry_safe(packet, tmp, &head, node) {
		hdr = packet->msg;
		pkt = SAT_TRE_OFFSET(packet->msg);

		list_del(&packet->node);

		mhi_sat_process_cmds(sat_cntrl, hdr, pkt);

		/* send response event(s) */
		mhi_sat_send_msg(sat_cntrl, SAT_MSG_ID_EVT, hdr->seq,
				 packet->msg,
				 SAT_MSG_SIZE(SAT_TRE_NUM_PKTS(
					      hdr->payload_size)));

		kfree(packet);
	}

	MHI_SAT_LOG("Exited\n");
}

static void mhi_sat_connect_worker(struct work_struct *work)
{
	struct mhi_sat_cntrl *sat_cntrl = container_of(work,
					struct mhi_sat_cntrl, connect_work);
	struct mhi_sat_subsys *subsys = sat_cntrl->subsys;
	struct sat_tre *pkt;
	void *msg;
	int ret;

	if (!subsys->rpdev || sat_cntrl->max_devices != sat_cntrl->num_devices
	    || sat_cntrl->active)
		return;

	MHI_SAT_LOG("Entered\n");

	msg = kmalloc(SAT_MSG_SIZE(3), GFP_ATOMIC);
	if (!msg)
		return;

	sat_cntrl->active = true;

	pkt = SAT_TRE_OFFSET(msg);

	/* prepare #1 MHI_CFG HELLO event */
	pkt->ptr = MHI_TRE_EVT_CFG_PTR(sat_cntrl->mhi_cntrl->base_addr);
	pkt->dword[0] = MHI_TRE_EVT_CFG_D0(sat_cntrl->er_base,
					   sat_cntrl->num_er);
	pkt->dword[1] = MHI_TRE_EVT_CFG_D1;
	pkt++;

	/* prepare M0 event */
	pkt->ptr = MHI_TRE_EVT_MHI_STATE_PTR;
	pkt->dword[0] = MHI_TRE_EVT_MHI_STATE_D0(MHI_STATE_M0);
	pkt->dword[1] = MHI_TRE_EVT_MHI_STATE_D1;
	pkt++;

	/* prepare AMSS event */
	pkt->ptr = MHI_TRE_EVT_EE_PTR;
	pkt->dword[0] = MHI_TRE_EVT_EE_D0(MHI_EE_AMSS);
	pkt->dword[1] = MHI_TRE_EVT_EE_D1;

	ret = mhi_sat_send_msg(sat_cntrl, SAT_MSG_ID_EVT, SAT_RESERVED_SEQ_NUM,
			       msg, SAT_MSG_SIZE(3));
	kfree(msg);
	if (ret) {
		MHI_SAT_ERR("Failed to send hello packet:%d\n", ret);
		sat_cntrl->active = false;
		return;
	}

	MHI_SAT_LOG("Device 0x%x sent hello packet\n", sat_cntrl->dev_id);
}

static void mhi_sat_process_events(struct mhi_sat_cntrl *sat_cntrl,
				   struct sat_header *hdr, struct sat_tre *pkt)
{
	int num_pkts = SAT_TRE_NUM_PKTS(hdr->payload_size);
	int i;

	for (i = 0; i < num_pkts; i++, pkt++) {
		if (MHI_TRE_GET_TYPE(pkt) ==
		    MHI_PKT_TYPE_CMD_COMPLETION_EVENT) {
			if (hdr->reply_seq != sat_cntrl->last_cmd_seq)
				continue;

			sat_cntrl->last_cmd_ccs = MHI_TRE_GET_CCS(pkt);
			complete(&sat_cntrl->completion);
		}
	}
}

static int mhi_sat_rpmsg_cb(struct rpmsg_device *rpdev, void *data, int len,
			    void *priv, u32 src)
{
	struct mhi_sat_subsys *subsys = dev_get_drvdata(&rpdev->dev);
	struct sat_header *hdr = data;
	struct sat_tre *pkt = SAT_TRE_OFFSET(data);
	struct mhi_sat_cntrl *sat_cntrl;
	struct mhi_sat_packet *packet;

	MHI_SAT_ASSERT(!mhi_sat_isvalid_header(hdr, len), "Invalid header!\n");

	/* find controller packet was sent for */
	sat_cntrl = find_sat_cntrl_by_id(subsys, hdr->dev_id);

	MHI_SAT_ASSERT(!sat_cntrl, "Packet for unknown device!\n");

	/* handle events directly regardless of controller active state */
	if (hdr->msg_id == SAT_MSG_ID_EVT) {
		mhi_sat_process_events(sat_cntrl, hdr, pkt);
		return 0;
	}

	/* Inactive controller cannot process incoming commands */
	if (unlikely(!sat_cntrl->active)) {
		MHI_SAT_ERR("Message for inactive controller!\n");
		return 0;
	}

	/* offload commands to process worker */
	packet = kmalloc(sizeof(*packet) + len, GFP_ATOMIC);
	if (!packet)
		return 0;

	packet->cntrl = sat_cntrl;
	packet->msg = packet + 1;
	memcpy(packet->msg, data, len);

	spin_lock_irq(&sat_cntrl->pkt_lock);
	list_add_tail(&packet->node, &sat_cntrl->packet_list);
	spin_unlock_irq(&sat_cntrl->pkt_lock);

	schedule_work(&sat_cntrl->process_work);

	return 0;
}

static void mhi_sat_rpmsg_remove(struct rpmsg_device *rpdev)
{
	struct mhi_sat_subsys *subsys = dev_get_drvdata(&rpdev->dev);
	struct mhi_sat_cntrl *sat_cntrl;
	struct mhi_sat_device *sat_dev;
	struct mhi_buf *buf, *tmp;

	MHI_SUBSYS_LOG("Enter\n");

	/* unprepare each controller/device from transfer */
	mutex_lock(&subsys->cntrl_mutex);
	list_for_each_entry(sat_cntrl, &subsys->cntrl_list, node) {
		sat_cntrl->active = false;

		flush_work(&sat_cntrl->connect_work);
		flush_work(&sat_cntrl->process_work);

		mutex_lock(&sat_cntrl->list_mutex);
		list_for_each_entry(sat_dev, &sat_cntrl->dev_list, node) {
			if (sat_dev->chan_started) {
				mhi_unprepare_from_transfer(sat_dev->mhi_dev);
				sat_dev->chan_started = false;
			}
		}

		list_for_each_entry_safe(buf, tmp, &sat_cntrl->addr_map_list,
					 node) {
			dma_unmap_resource(sat_cntrl->mhi_cntrl->dev,
					   buf->dma_addr, buf->len,
					   DMA_BIDIRECTIONAL, 0);
			list_del(&buf->node);
			kfree(buf);
		}
		mutex_unlock(&sat_cntrl->list_mutex);

		MHI_SAT_LOG("Removed RPMSG link\n");
	}
	mutex_unlock(&subsys->cntrl_mutex);

	subsys->rpdev = NULL;
}

static int mhi_sat_rpmsg_probe(struct rpmsg_device *rpdev)
{
	struct mhi_sat_subsys *subsys;
	struct mhi_sat_cntrl *sat_cntrl;
	const char *subsys_name;
	int ret;

	ret = of_property_read_string(rpdev->dev.parent->of_node, "label",
					&subsys_name);
	if (ret)
		return ret;

	/* find which subsystem has probed */
	subsys = find_subsys_by_name(subsys_name);
	if (!subsys)
		return -EINVAL;

	MHI_SUBSYS_LOG("Received RPMSG probe\n");

	dev_set_drvdata(&rpdev->dev, subsys);

	subsys->rpdev = rpdev;

	/* schedule work for each controller as GLINK has connected */
	spin_lock_irq(&subsys->cntrl_lock);
	list_for_each_entry(sat_cntrl, &subsys->cntrl_list, node)
		schedule_work(&sat_cntrl->connect_work);
	spin_unlock_irq(&subsys->cntrl_lock);

	return 0;
}

static struct rpmsg_device_id mhi_sat_rpmsg_match_table[] = {
	{ .name = "mhi_sat" },
	{ },
};

static struct rpmsg_driver mhi_sat_rpmsg_driver = {
	.id_table = mhi_sat_rpmsg_match_table,
	.probe = mhi_sat_rpmsg_probe,
	.remove = mhi_sat_rpmsg_remove,
	.callback = mhi_sat_rpmsg_cb,
	.drv = {
		.name = "mhi,sat_rpmsg",
	},
};

static void mhi_sat_dev_status_cb(struct mhi_device *mhi_dev,
				  enum MHI_CB mhi_cb)
{
}

static void mhi_sat_dev_remove(struct mhi_device *mhi_dev)
{
	struct mhi_sat_device *sat_dev = mhi_device_get_devdata(mhi_dev);
	struct mhi_sat_cntrl *sat_cntrl = sat_dev->cntrl;
	struct mhi_sat_subsys *subsys = sat_cntrl->subsys;
	struct mhi_buf *buf, *tmp;
	struct sat_tre *pkt;
	void *msg;
	int ret;

	/* remove device node from probed list */
	mutex_lock(&sat_cntrl->list_mutex);
	list_del(&sat_dev->node);
	mutex_unlock(&sat_cntrl->list_mutex);

	sat_cntrl->num_devices--;

	/* prepare SYS_ERR command if first device is being removed */
	if (sat_cntrl->active) {
		sat_cntrl->active = false;

		/* flush all pending work */
		flush_work(&sat_cntrl->connect_work);
		flush_work(&sat_cntrl->process_work);

		msg = kmalloc(SAT_MSG_SIZE(1), GFP_KERNEL);

		MHI_SAT_ASSERT(!msg, "Unable to malloc for SYS_ERR message!\n");

		pkt = SAT_TRE_OFFSET(msg);
		pkt->ptr = MHI_TRE_CMD_SYS_ERR_PTR;
		pkt->dword[0] = MHI_TRE_CMD_SYS_ERR_D0;
		pkt->dword[1] = MHI_TRE_CMD_SYS_ERR_D1;

		/* acquire cmd_wait_mutex before sending command */
		mutex_lock(&sat_cntrl->cmd_wait_mutex);

		ret = mhi_sat_send_msg(sat_cntrl, SAT_MSG_ID_CMD,
				       SAT_RESERVED_SEQ_NUM, msg,
				       SAT_MSG_SIZE(1));
		kfree(msg);
		if (ret) {
			MHI_SAT_ERR("Failed to notify SYS_ERR\n");
			mutex_unlock(&sat_cntrl->cmd_wait_mutex);
			goto exit_sys_err_send;
		}

		MHI_SAT_LOG("SYS_ERR command sent\n");

		/* blocking call to wait for command completion event */
		mhi_sat_wait_cmd_completion(sat_cntrl);

		mutex_unlock(&sat_cntrl->cmd_wait_mutex);
	}

exit_sys_err_send:
	/* exit if some devices are still present */
	if (sat_cntrl->num_devices)
		return;

	/* remove address mappings */
	mutex_lock(&sat_cntrl->list_mutex);
	list_for_each_entry_safe(buf, tmp, &sat_cntrl->addr_map_list, node) {
		dma_unmap_resource(sat_cntrl->mhi_cntrl->dev, buf->dma_addr,
				   buf->len, DMA_BIDIRECTIONAL, 0);
		list_del(&buf->node);
		kfree(buf);
	}
	mutex_unlock(&sat_cntrl->list_mutex);

	/* remove controller */
	mutex_lock(&subsys->cntrl_mutex);
	spin_lock_irq(&subsys->cntrl_lock);
	list_del(&sat_cntrl->node);
	spin_unlock_irq(&subsys->cntrl_lock);
	mutex_unlock(&subsys->cntrl_mutex);

	mutex_destroy(&sat_cntrl->cmd_wait_mutex);
	mutex_destroy(&sat_cntrl->list_mutex);
	MHI_SAT_LOG("Satellite controller node removed\n");
	kfree(sat_cntrl);
}

static int mhi_sat_dev_probe(struct mhi_device *mhi_dev,
			     const struct mhi_device_id *id)
{
	struct mhi_sat_device *sat_dev;
	struct mhi_sat_cntrl *sat_cntrl;
	struct device_node *of_node = mhi_dev->dev.of_node;
	struct mhi_sat_subsys *subsys = &mhi_sat_driver.subsys[id->driver_data];
	u32 dev_id = MHI_SAT_CREATE_DEVICE_ID(mhi_dev->dev_id, mhi_dev->domain,
					      mhi_dev->bus, mhi_dev->slot);
	int ret;

	/* find controller with unique device ID based on topology */
	sat_cntrl = find_sat_cntrl_by_id(subsys, dev_id);
	if (!sat_cntrl) {
		sat_cntrl = kzalloc(sizeof(*sat_cntrl), GFP_KERNEL);
		if (!sat_cntrl)
			return -ENOMEM;

		/*
		 * max_devices will be read from device tree node. Set it to
		 * -1 before it is populated to avoid false positive when
		 * RPMSG probe schedules connect worker but no device has
		 * probed in which case num_devices and max_devices are both
		 * zero.
		 */
		sat_cntrl->max_devices = -1;
		sat_cntrl->dev_id = dev_id;
		sat_cntrl->er_base = mhi_dev->dl_event_id;
		sat_cntrl->mhi_cntrl = mhi_dev->mhi_cntrl;
		sat_cntrl->last_cmd_seq = SAT_RESERVED_SEQ_NUM;
		sat_cntrl->subsys = subsys;
		init_completion(&sat_cntrl->completion);
		mutex_init(&sat_cntrl->list_mutex);
		mutex_init(&sat_cntrl->cmd_wait_mutex);
		spin_lock_init(&sat_cntrl->pkt_lock);
		INIT_WORK(&sat_cntrl->connect_work, mhi_sat_connect_worker);
		INIT_WORK(&sat_cntrl->process_work, mhi_sat_process_worker);
		INIT_LIST_HEAD(&sat_cntrl->dev_list);
		INIT_LIST_HEAD(&sat_cntrl->addr_map_list);
		INIT_LIST_HEAD(&sat_cntrl->packet_list);

		mutex_lock(&subsys->cntrl_mutex);
		spin_lock_irq(&subsys->cntrl_lock);
		list_add(&sat_cntrl->node, &subsys->cntrl_list);
		spin_unlock_irq(&subsys->cntrl_lock);
		mutex_unlock(&subsys->cntrl_mutex);

		MHI_SAT_LOG("Controller allocated for 0x%x\n", dev_id);
	}

	/* set maximum devices for subsystem from device tree */
	if (of_node) {
		ret = of_property_read_u32(of_node, "mhi,max-devices",
					   &sat_cntrl->max_devices);
		if (ret) {
			MHI_SAT_ERR("Could not find max-devices in DT node\n");
			return -EINVAL;
		}
	}

	/* get event ring base and max indexes */
	sat_cntrl->er_base = min(sat_cntrl->er_base, mhi_dev->dl_event_id);
	sat_cntrl->er_max = max(sat_cntrl->er_base, mhi_dev->dl_event_id);

	sat_dev = devm_kzalloc(&mhi_dev->dev, sizeof(*sat_dev), GFP_KERNEL);
	if (!sat_dev)
		return -ENOMEM;

	sat_dev->mhi_dev = mhi_dev;
	sat_dev->cntrl = sat_cntrl;

	mutex_lock(&sat_cntrl->list_mutex);
	list_add(&sat_dev->node, &sat_cntrl->dev_list);
	mutex_unlock(&sat_cntrl->list_mutex);

	mhi_device_set_devdata(mhi_dev, sat_dev);

	sat_cntrl->num_devices++;

	/* schedule connect worker if all devices for controller have probed */
	if (sat_cntrl->num_devices == sat_cntrl->max_devices) {
		/* number of event rings is 1 more than difference in IDs */
		sat_cntrl->num_er = (sat_cntrl->er_max - sat_cntrl->er_base) +
				     1;
		MHI_SAT_LOG("All satellite channels probed!\n");
		schedule_work(&sat_cntrl->connect_work);
	}

	return 0;
}

/* .driver_data stores subsys id */
static const struct mhi_device_id mhi_sat_dev_match_table[] = {
	/* ADSP */
	{ .chan = "ADSP_0", .driver_data = SUBSYS_ADSP },
	{ .chan = "ADSP_1", .driver_data = SUBSYS_ADSP },
	{ .chan = "ADSP_2", .driver_data = SUBSYS_ADSP },
	{ .chan = "ADSP_3", .driver_data = SUBSYS_ADSP },
	{ .chan = "ADSP_4", .driver_data = SUBSYS_ADSP },
	{ .chan = "ADSP_5", .driver_data = SUBSYS_ADSP },
	{ .chan = "ADSP_6", .driver_data = SUBSYS_ADSP },
	{ .chan = "ADSP_7", .driver_data = SUBSYS_ADSP },
	{ .chan = "ADSP_8", .driver_data = SUBSYS_ADSP },
	{ .chan = "ADSP_9", .driver_data = SUBSYS_ADSP },
	/* CDSP */
	{ .chan = "CDSP_0", .driver_data = SUBSYS_CDSP },
	{ .chan = "CDSP_1", .driver_data = SUBSYS_CDSP },
	{ .chan = "CDSP_2", .driver_data = SUBSYS_CDSP },
	{ .chan = "CDSP_3", .driver_data = SUBSYS_CDSP },
	{ .chan = "CDSP_4", .driver_data = SUBSYS_CDSP },
	{ .chan = "CDSP_5", .driver_data = SUBSYS_CDSP },
	{ .chan = "CDSP_6", .driver_data = SUBSYS_CDSP },
	{ .chan = "CDSP_7", .driver_data = SUBSYS_CDSP },
	{ .chan = "CDSP_8", .driver_data = SUBSYS_CDSP },
	{ .chan = "CDSP_9", .driver_data = SUBSYS_CDSP },
	/* SLPI */
	{ .chan = "SLPI_0", .driver_data = SUBSYS_SLPI },
	{ .chan = "SLPI_1", .driver_data = SUBSYS_SLPI },
	{ .chan = "SLPI_2", .driver_data = SUBSYS_SLPI },
	{ .chan = "SLPI_3", .driver_data = SUBSYS_SLPI },
	{ .chan = "SLPI_4", .driver_data = SUBSYS_SLPI },
	{ .chan = "SLPI_5", .driver_data = SUBSYS_SLPI },
	{ .chan = "SLPI_6", .driver_data = SUBSYS_SLPI },
	{ .chan = "SLPI_7", .driver_data = SUBSYS_SLPI },
	{ .chan = "SLPI_8", .driver_data = SUBSYS_SLPI },
	{ .chan = "SLPI_9", .driver_data = SUBSYS_SLPI },
	/* MODEM */
	{ .chan = "MODEM_0", .driver_data = SUBSYS_MODEM },
	{ .chan = "MODEM_1", .driver_data = SUBSYS_MODEM },
	{ .chan = "MODEM_2", .driver_data = SUBSYS_MODEM },
	{ .chan = "MODEM_3", .driver_data = SUBSYS_MODEM },
	{ .chan = "MODEM_4", .driver_data = SUBSYS_MODEM },
	{ .chan = "MODEM_5", .driver_data = SUBSYS_MODEM },
	{ .chan = "MODEM_6", .driver_data = SUBSYS_MODEM },
	{ .chan = "MODEM_7", .driver_data = SUBSYS_MODEM },
	{ .chan = "MODEM_8", .driver_data = SUBSYS_MODEM },
	{ .chan = "MODEM_9", .driver_data = SUBSYS_MODEM },
	{},
};

static struct mhi_driver mhi_sat_dev_driver = {
	.id_table = mhi_sat_dev_match_table,
	.probe = mhi_sat_dev_probe,
	.remove = mhi_sat_dev_remove,
	.status_cb = mhi_sat_dev_status_cb,
	.driver = {
		.name = MHI_SAT_DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

int mhi_sat_trigger_init(void *data, u64 val)
{
	struct mhi_sat_subsys *subsys;
	int i, ret;

	if (mhi_sat_driver.deferred_init_done)
		return -EIO;

	ret = register_rpmsg_driver(&mhi_sat_rpmsg_driver);
	if (ret)
		goto error_sat_trigger_init;

	ret = mhi_driver_register(&mhi_sat_dev_driver);
	if (ret)
		goto error_sat_trigger_register;

	mhi_sat_driver.deferred_init_done = true;

	return 0;

error_sat_trigger_register:
	unregister_rpmsg_driver(&mhi_sat_rpmsg_driver);

error_sat_trigger_init:
	subsys = mhi_sat_driver.subsys;
	for (i = 0; i < mhi_sat_driver.num_subsys; i++, subsys++) {
		ipc_log_context_destroy(subsys->ipc_log);
		mutex_destroy(&subsys->cntrl_mutex);
	}
	kfree(mhi_sat_driver.subsys);
	mhi_sat_driver.subsys = NULL;

	return ret;
}

DEFINE_SIMPLE_ATTRIBUTE(mhi_sat_debugfs_fops, NULL,
			mhi_sat_trigger_init, "%llu\n");

static int mhi_sat_init(void)
{
	struct mhi_sat_subsys *subsys;
	int i, ret;

	subsys = kcalloc(SUBSYS_MAX, sizeof(*subsys), GFP_KERNEL);
	if (!subsys)
		return -ENOMEM;

	mhi_sat_driver.subsys = subsys;
	mhi_sat_driver.num_subsys = SUBSYS_MAX;
	mhi_sat_driver.klog_lvl = KLOG_LVL;
	mhi_sat_driver.ipc_log_lvl = IPC_LOG_LVL;

	for (i = 0; i < mhi_sat_driver.num_subsys; i++, subsys++) {
		char log[32];

		subsys->name = subsys_names[i];
		mutex_init(&subsys->cntrl_mutex);
		spin_lock_init(&subsys->cntrl_lock);
		INIT_LIST_HEAD(&subsys->cntrl_list);
		scnprintf(log, sizeof(log), "mhi_sat_%s", subsys->name);
		subsys->ipc_log = ipc_log_context_create(IPC_LOG_PAGES, log, 0);
	}

	/* create debugfs entry if defer_init is enabled */
	if (mhi_sat_defer_init) {
		mhi_sat_driver.dentry = debugfs_create_dir("mhi_sat", NULL);
		if (IS_ERR_OR_NULL(mhi_sat_driver.dentry)) {
			ret = -ENODEV;
			goto error_sat_init;
		}

		debugfs_create_file("debug", 0444, mhi_sat_driver.dentry, NULL,
				    &mhi_sat_debugfs_fops);

		return 0;
	}

	ret = register_rpmsg_driver(&mhi_sat_rpmsg_driver);
	if (ret)
		goto error_sat_init;

	ret = mhi_driver_register(&mhi_sat_dev_driver);
	if (ret)
		goto error_sat_register;

	return 0;

error_sat_register:
	unregister_rpmsg_driver(&mhi_sat_rpmsg_driver);

error_sat_init:
	subsys = mhi_sat_driver.subsys;
	for (i = 0; i < mhi_sat_driver.num_subsys; i++, subsys++) {
		ipc_log_context_destroy(subsys->ipc_log);
		mutex_destroy(&subsys->cntrl_mutex);
	}
	kfree(mhi_sat_driver.subsys);
	mhi_sat_driver.subsys = NULL;

	return ret;
}

module_init(mhi_sat_init);

