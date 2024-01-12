/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2018-2019, The Linux Foundation. All rights reserved. */

#ifndef _MHI_H_
#define _MHI_H_

#define PCIE_MHI_DRIVER_VERSION "V1.3.4"
#define ENABLE_MHI_MON
//#define ENABLE_IP_SW0

#include <linux/miscdevice.h>
typedef enum
{
   MHI_CLIENT_LOOPBACK_OUT     = 0,
   MHI_CLIENT_LOOPBACK_IN      = 1,
   MHI_CLIENT_SAHARA_OUT       = 2,
   MHI_CLIENT_SAHARA_IN        = 3,
   MHI_CLIENT_DIAG_OUT         = 4,
   MHI_CLIENT_DIAG_IN          = 5,
   MHI_CLIENT_SSR_OUT          = 6,
   MHI_CLIENT_SSR_IN           = 7,
   MHI_CLIENT_QDSS_OUT         = 8,
   MHI_CLIENT_QDSS_IN          = 9,
   MHI_CLIENT_EFS_OUT          = 10,
   MHI_CLIENT_EFS_IN           = 11,
   MHI_CLIENT_MBIM_OUT         = 12,
   MHI_CLIENT_MBIM_IN          = 13,
   MHI_CLIENT_QMI_OUT          = 14,
   MHI_CLIENT_QMI_IN           = 15,
   MHI_CLIENT_QMI_2_OUT        = 16,
   MHI_CLIENT_QMI_2_IN         = 17,
   MHI_CLIENT_IP_CTRL_1_OUT    = 18,
   MHI_CLIENT_IP_CTRL_1_IN     = 19,
   MHI_CLIENT_IPCR_OUT         = 20,
   MHI_CLIENT_IPCR_IN          = 21,
   MHI_CLIENT_TEST_FW_OUT      = 22,
   MHI_CLIENT_TEST_FW_IN       = 23,
   MHI_CLIENT_RESERVED_0       = 24,
   MHI_CLIENT_BOOT_LOG_IN      = 25,
   MHI_CLIENT_DCI_OUT          = 26,
   MHI_CLIENT_DCI_IN           = 27,
   MHI_CLIENT_QBI_OUT          = 28,
   MHI_CLIENT_QBI_IN           = 29,
   MHI_CLIENT_RESERVED_1_LOWER = 30,
   MHI_CLIENT_RESERVED_1_UPPER = 31,
   MHI_CLIENT_DUN_OUT          = 32,
   MHI_CLIENT_DUN_IN           = 33,
   MHI_CLIENT_EDL_OUT          = 34,
   MHI_CLIENT_EDL_IN           = 35,
   MHI_CLIENT_ADB_FB_OUT       = 36,
   MHI_CLIENT_ADB_FB_IN        = 37,
   MHI_CLIENT_RESERVED_2_LOWER = 38,
   MHI_CLIENT_RESERVED_2_UPPER = 41,
   MHI_CLIENT_CSVT_OUT         = 42,
   MHI_CLIENT_CSVT_IN          = 43,
   MHI_CLIENT_SMCT_OUT         = 44,
   MHI_CLIENT_SMCT_IN          = 45,
   MHI_CLIENT_IP_SW_0_OUT      = 46,
   MHI_CLIENT_IP_SW_0_IN       = 47,
   MHI_CLIENT_IP_SW_1_OUT      = 48,
   MHI_CLIENT_IP_SW_1_IN       = 49,
   MHI_CLIENT_RESERVED_3_LOWER = 50,
   MHI_CLIENT_RESERVED_3_UPPER = 59,
   MHI_CLIENT_TEST_0_OUT       = 60,
   MHI_CLIENT_TEST_0_IN        = 61,
   MHI_CLIENT_TEST_1_OUT       = 62,
   MHI_CLIENT_TEST_1_IN        = 63,
   MHI_CLIENT_TEST_2_OUT       = 64,
   MHI_CLIENT_TEST_2_IN        = 65,
   MHI_CLIENT_TEST_3_OUT       = 66,
   MHI_CLIENT_TEST_3_IN        = 67,
   MHI_CLIENT_RESERVED_4_LOWER = 68,
   MHI_CLIENT_RESERVED_4_UPPER = 91,
   MHI_CLIENT_OEM_0_OUT        = 92,
   MHI_CLIENT_OEM_0_IN         = 93,
   MHI_CLIENT_OEM_1_OUT        = 94,
   MHI_CLIENT_OEM_1_IN         = 95,
   MHI_CLIENT_OEM_2_OUT        = 96,
   MHI_CLIENT_OEM_2_IN         = 97,
   MHI_CLIENT_OEM_3_OUT        = 98,
   MHI_CLIENT_OEM_3_IN         = 99,
   MHI_CLIENT_IP_HW_0_OUT      = 100,
   MHI_CLIENT_IP_HW_0_IN       = 101,
   MHI_CLIENT_ADPL             = 102,
   MHI_CLIENT_RESERVED_5_LOWER = 103,
   MHI_CLIENT_RESERVED_5_UPPER = 127,
   MHI_MAX_CHANNELS            = 128
}MHI_CLIENT_CHANNEL_TYPE;

/* Event Ring Index */
typedef enum
{
	SW_EVT_RING       = 0,
	PRIMARY_EVENT_RING = SW_EVT_RING,
#ifdef ENABLE_IP_SW0
	SW_0_OUT_EVT_RING,
	SW_0_IN_EVT_RING,
#endif
	IPA_OUT_EVENT_RING,
	IPA_IN_EVENT_RING,
	ADPL_EVT_RING,

	MAX_EVT_RING_IDX
}MHI_EVT_RING_IDX;

#define MHI_VERSION                  0x01000000
#define MHIREGLEN_VALUE              0x100 /* **** WRONG VALUE *** */
#define MHI_MSI_INDEX                1
#define MAX_NUM_MHI_DEVICES          1
#define NUM_MHI_XFER_RINGS           128
#define NUM_MHI_EVT_RINGS            MAX_EVT_RING_IDX
#define NUM_MHI_HW_EVT_RINGS         3
#define NUM_MHI_XFER_RING_ELEMENTS   16
#define NUM_MHI_EVT_RING_ELEMENTS    (NUM_MHI_IPA_IN_RING_ELEMENTS*2) //must *2, event ring full will make x55 dump
#define NUM_MHI_IPA_IN_RING_ELEMENTS    512
#define NUM_MHI_IPA_OUT_RING_ELEMENTS    512 //donot use ul agg, so increase
#define NUM_MHI_DIAG_IN_RING_ELEMENTS    128
#define NUM_MHI_SW_IP_RING_ELEMENTS    512

/*
* for if set Interrupt moderation time as 1ms,
and transfer more than NUM_MHI_CHAN_RING_ELEMENTS data are sent to the modem in 1ms.
e.g. firehose upgrade.
modem will not trigger irq for these transfer.
*/
#define NUM_MHI_CHAN_RING_ELEMENTS    32 //8
#define MHI_EVT_CMD_QUEUE_SIZE       160
#define MHI_EVT_STATE_QUEUE_SIZE     128
#define MHI_EVT_XFER_QUEUE_SIZE      1024

#define CHAN_INBOUND(_x)   ((_x)%2)

#define CHAN_SBL(_x)       (((_x) == MHI_CLIENT_SAHARA_OUT)   ||  \
                            ((_x) == MHI_CLIENT_SAHARA_IN)    ||  \
                            ((_x) == MHI_CLIENT_BOOT_LOG_IN))

#define CHAN_EDL(_x)       (((_x) == MHI_CLIENT_EDL_OUT)   ||  \
                            ((_x) == MHI_CLIENT_EDL_IN))
               
struct mhi_chan;
struct mhi_event;
struct mhi_ctxt;
struct mhi_cmd;
struct image_info;
struct bhi_vec_entry;
struct mhi_timesync;
struct mhi_buf_info;

/**
 * enum MHI_CB - MHI callback
 * @MHI_CB_IDLE: MHI entered idle state
 * @MHI_CB_PENDING_DATA: New data available for client to process
 * @MHI_CB_LPM_ENTER: MHI host entered low power mode
 * @MHI_CB_LPM_EXIT: MHI host about to exit low power mode
 * @MHI_CB_EE_RDDM: MHI device entered RDDM execution enviornment
 * @MHI_CB_EE_MISSION_MODE: MHI device entered Mission Mode exec env
 * @MHI_CB_SYS_ERROR: MHI device enter error state (may recover)
 * @MHI_CB_FATAL_ERROR: MHI device entered fatal error
 */
enum MHI_CB {
	MHI_CB_IDLE,
	MHI_CB_PENDING_DATA,
	MHI_CB_LPM_ENTER,
	MHI_CB_LPM_EXIT,
	MHI_CB_EE_RDDM,
	MHI_CB_EE_MISSION_MODE,
	MHI_CB_SYS_ERROR,
	MHI_CB_FATAL_ERROR,
};

/**
 * enum MHI_DEBUG_LEVL - various debugging level
 */
enum MHI_DEBUG_LEVEL {
	MHI_MSG_LVL_VERBOSE,
	MHI_MSG_LVL_INFO,
	MHI_MSG_LVL_ERROR,
	MHI_MSG_LVL_CRITICAL,
	MHI_MSG_LVL_MASK_ALL,
};

/*
GSI_XFER_FLAG_BEI: Block event interrupt
1: Event generated by this ring element must not assert an interrupt to the host
0: Event generated by this ring element must assert an interrupt to the host

GSI_XFER_FLAG_EOT: Interrupt on end of transfer
1: If an EOT condition is encountered when processing this ring element, an event is generated by the device with its completion code set to EOT.
0: If an EOT condition is encountered for this ring element, a completion event is not be generated by the device, unless IEOB is 1

GSI_XFER_FLAG_EOB: Interrupt on end of block
1: Device notifies host after processing this ring element by sending a completion event
0: Completion event is not required after processing this ring element

GSI_XFER_FLAG_CHAIN: Chain bit that identifies the ring elements in a TD
*/

/**
 * enum MHI_FLAGS - Transfer flags
 * @MHI_EOB: End of buffer for bulk transfer
 * @MHI_EOT: End of transfer
 * @MHI_CHAIN: Linked transfer
 */
enum MHI_FLAGS {
	MHI_EOB,
	MHI_EOT,
	MHI_CHAIN,
};

/**
 * enum mhi_device_type - Device types
 * @MHI_XFER_TYPE: Handles data transfer
 * @MHI_TIMESYNC_TYPE: Use for timesync feature
 * @MHI_CONTROLLER_TYPE: Control device
 */
enum mhi_device_type {
	MHI_XFER_TYPE,
	MHI_TIMESYNC_TYPE,
	MHI_CONTROLLER_TYPE,
};

/**
 * enum mhi_ee - device current execution enviornment
 * @MHI_EE_PBL - device in PBL
 * @MHI_EE_SBL - device in SBL
 * @MHI_EE_AMSS - device in mission mode (firmware fully loaded)
 * @MHI_EE_RDDM - device in ram dump collection mode
 * @MHI_EE_WFW - device in WLAN firmware mode
 * @MHI_EE_PTHRU - device in PBL but configured in pass thru mode
 * @MHI_EE_EDL - device in emergency download mode
 */
enum mhi_ee {
	MHI_EE_PBL = 0x0,
	MHI_EE_SBL = 0x1,
	MHI_EE_AMSS = 0x2,
	MHI_EE_RDDM = 0x3,
	MHI_EE_WFW = 0x4,
	MHI_EE_PTHRU = 0x5,
	MHI_EE_EDL = 0x6,
	MHI_EE_FP   = 0x7,            /* FlashProg, Flash Programmer Environment                            */
	MHI_EE_MAX_SUPPORTED = MHI_EE_FP,
	MHI_EE_DISABLE_TRANSITION, /* local EE, not related to mhi spec */
	MHI_EE_MAX,
};

/**
 * enum mhi_dev_state - device current MHI state
 */
enum mhi_dev_state {
	MHI_STATE_RESET = 0x0,
	MHI_STATE_READY = 0x1,
	MHI_STATE_M0 = 0x2,
	MHI_STATE_M1 = 0x3,
	MHI_STATE_M2 = 0x4,
	MHI_STATE_M3 = 0x5,
	MHI_STATE_BHI  = 0x7,
	MHI_STATE_SYS_ERR  = 0xFF,
	MHI_STATE_MAX,
};

extern const char * const mhi_ee_str[MHI_EE_MAX];
#define TO_MHI_EXEC_STR(ee) (((ee) >= MHI_EE_MAX) ? \
			     "INVALID_EE" : mhi_ee_str[ee])

/**
 * struct image_info - firmware and rddm table table
 * @mhi_buf - Contain device firmware and rddm table
 * @entries - # of entries in table
 */
struct image_info {
	struct mhi_buf *mhi_buf;
	struct bhi_vec_entry *bhi_vec;
	u32 entries;
};

/**
 * struct mhi_controller - Master controller structure for external modem
 * @dev: Device associated with this controller
 * @of_node: DT that has MHI configuration information
 * @regs: Points to base of MHI MMIO register space
 * @bhi: Points to base of MHI BHI register space
 * @bhie: Points to base of MHI BHIe register space
 * @wake_db: MHI WAKE doorbell register address
 * @dev_id: PCIe device id of the external device
 * @domain: PCIe domain the device connected to
 * @bus: PCIe bus the device assigned to
 * @slot: PCIe slot for the modem
 * @iova_start: IOMMU starting address for data
 * @iova_stop: IOMMU stop address for data
 * @fw_image: Firmware image name for normal booting
 * @edl_image: Firmware image name for emergency download mode
 * @fbc_download: MHI host needs to do complete image transfer
 * @rddm_size: RAM dump size that host should allocate for debugging purpose
 * @sbl_size: SBL image size
 * @seg_len: BHIe vector size
 * @fbc_image: Points to firmware image buffer
 * @rddm_image: Points to RAM dump buffer
 * @max_chan: Maximum number of channels controller support
 * @mhi_chan: Points to channel configuration table
 * @lpm_chans: List of channels that require LPM notifications
 * @total_ev_rings: Total # of event rings allocated
 * @hw_ev_rings: Number of hardware event rings
 * @sw_ev_rings: Number of software event rings
 * @msi_required: Number of msi required to operate
 * @msi_allocated: Number of msi allocated by bus master
 * @irq: base irq # to request
 * @mhi_event: MHI event ring configurations table
 * @mhi_cmd: MHI command ring configurations table
 * @mhi_ctxt: MHI device context, shared memory between host and device
 * @timeout_ms: Timeout in ms for state transitions
 * @pm_state: Power management state
 * @ee: MHI device execution environment
 * @dev_state: MHI STATE
 * @status_cb: CB function to notify various power states to but master
 * @link_status: Query link status in case of abnormal value read from device
 * @runtime_get: Async runtime resume function
 * @runtimet_put: Release votes
 * @time_get: Return host time in us
 * @lpm_disable: Request controller to disable link level low power modes
 * @lpm_enable: Controller may enable link level low power modes again
 * @priv_data: Points to bus master's private data
 */
struct mhi_controller {
	struct list_head node;
	struct mhi_device *mhi_dev;

	/* device node for iommu ops */
	struct device *dev;
	struct device_node *of_node;

	/* mmio base */
	phys_addr_t base_addr;
	void __iomem *regs;
	void __iomem *bhi;
	void __iomem *bhie;
	void __iomem *wake_db;

	/* device topology */
	u32 vendor;
	u32 dev_id;
	u32 domain;
	u32 bus;
	u32 slot;
	u32 cntrl_idx;  
	struct device *cntrl_dev;

	/* addressing window */
	dma_addr_t iova_start;
	dma_addr_t iova_stop;

	/* fw images */
	const char *fw_image;
	const char *edl_image;

	/* mhi host manages downloading entire fbc images */
	bool fbc_download;
	size_t rddm_size;
	size_t sbl_size;
	size_t seg_len;
	u32 session_id;
	u32 sequence_id;
	struct image_info *fbc_image;
	struct image_info *rddm_image;

	/* physical channel config data */
	u32 max_chan;
	struct mhi_chan *mhi_chan;
	struct list_head lpm_chans; /* these chan require lpm notification */

	/* physical event config data */
	u32 total_ev_rings;
	u32 hw_ev_rings;
	u32 sw_ev_rings;
	u32 msi_required;
	u32 msi_allocated;
	u32 msi_irq_base;
	int *irq; /* interrupt table */
	struct mhi_event *mhi_event;

	/* cmd rings */
	struct mhi_cmd *mhi_cmd;

	/* mhi context (shared with device) */
	struct mhi_ctxt *mhi_ctxt;

	u32 timeout_ms;

	/* caller should grab pm_mutex for suspend/resume operations */
	struct mutex pm_mutex;
	bool pre_init;
	rwlock_t pm_lock;
	u32 pm_state;
	enum mhi_ee ee;
	enum mhi_dev_state dev_state;
	bool wake_set;
	atomic_t dev_wake;
	atomic_t alloc_size;
	atomic_t pending_pkts;
	struct list_head transition_list;
	spinlock_t transition_lock;
	spinlock_t wlock;

	/* debug counters */
	u32 M0, M2, M3;

	/* worker for different state transitions */
	struct work_struct st_worker;
	struct work_struct fw_worker;
	struct work_struct syserr_worker;
	struct delayed_work ready_worker;
	wait_queue_head_t state_event;

	/* shadow functions */
	void (*status_cb)(struct mhi_controller *mhi_cntrl, void *priv,
			  enum MHI_CB reason);
	int (*link_status)(struct mhi_controller *mhi_cntrl, void *priv);
	void (*wake_get)(struct mhi_controller *mhi_cntrl, bool override);
	void (*wake_put)(struct mhi_controller *mhi_cntrl, bool override);
	int (*runtime_get)(struct mhi_controller *mhi_cntrl, void *priv);
	void (*runtime_put)(struct mhi_controller *mhi_cntrl, void *priv);
	void (*runtime_mark_last_busy)(struct mhi_controller *mhi_cntrl, void *priv);
	u64 (*time_get)(struct mhi_controller *mhi_cntrl, void *priv);
	int (*lpm_disable)(struct mhi_controller *mhi_cntrl, void *priv);
	int (*lpm_enable)(struct mhi_controller *mhi_cntrl, void *priv);
	int (*map_single)(struct mhi_controller *mhi_cntrl,
			  struct mhi_buf_info *buf);
	void (*unmap_single)(struct mhi_controller *mhi_cntrl,
			     struct mhi_buf_info *buf);

	/* channel to control DTR messaging */
	struct mhi_device *dtr_dev;

	/* bounce buffer settings */
	bool bounce_buf;
	size_t buffer_len;

	/* supports time sync feature */
	struct mhi_timesync *mhi_tsync;
	struct mhi_device *tsync_dev;

	/* kernel log level */
	enum MHI_DEBUG_LEVEL klog_lvl;
	int klog_slient;

	/* private log level controller driver to set */
	enum MHI_DEBUG_LEVEL log_lvl;

	/* controller specific data */
	void *priv_data;
	void *log_buf;
	struct dentry *dentry;
	struct dentry *parent;

	struct miscdevice miscdev;

#ifdef ENABLE_MHI_MON
	spinlock_t lock;

	/* Ref */
	int nreaders;			/* Under mon_lock AND mbus->lock */
	struct list_head r_list;	/* Chain of readers (usually one) */
	struct kref ref;		/* Under mon_lock */

	/* Stats */
	unsigned int cnt_events;
	unsigned int cnt_text_lost;
#endif
};

#ifdef ENABLE_MHI_MON
struct mhi_tre;
struct mon_reader {
	struct list_head r_link;
	struct mhi_controller *m_bus;
	void *r_data;		/* Use container_of instead? */

	void (*rnf_submit)(void *data, u32 chan, dma_addr_t wp, struct mhi_tre *mhi_tre, void *buf, size_t len);
	void (*rnf_receive)(void *data, u32 chan, dma_addr_t wp, struct mhi_tre *mhi_tre, void *buf, size_t len);
	void (*rnf_complete)(void *data, u32 chan, dma_addr_t wp, struct mhi_tre *mhi_tre);
};
#endif

/**
 * struct mhi_device - mhi device structure associated bind to channel
 * @dev: Device associated with the channels
 * @mtu: Maximum # of bytes controller support
 * @ul_chan_id: MHI channel id for UL transfer
 * @dl_chan_id: MHI channel id for DL transfer
 * @tiocm: Device current terminal settings
 * @priv: Driver private data
 */
struct mhi_device {
	struct device dev;
	u32 vendor;
	u32 dev_id;
	u32 domain;
	u32 bus;
	u32 slot;
	size_t mtu;
	int ul_chan_id;
	int dl_chan_id;
	int ul_event_id;
	int dl_event_id;
	u32 tiocm;
	const struct mhi_device_id *id;
	const char *chan_name;
	struct mhi_controller *mhi_cntrl;
	struct mhi_chan *ul_chan;
	struct mhi_chan *dl_chan;
	atomic_t dev_wake;
	enum mhi_device_type dev_type;
	void *priv_data;
	int (*ul_xfer)(struct mhi_device *mhi_dev, struct mhi_chan *mhi_chan,
		       void *buf, size_t len, enum MHI_FLAGS flags);
	int (*dl_xfer)(struct mhi_device *mhi_dev, struct mhi_chan *mhi_chan,
		       void *buf, size_t size, enum MHI_FLAGS flags);
	void (*status_cb)(struct mhi_device *mhi_dev, enum MHI_CB reason);
};

/**
 * struct mhi_result - Completed buffer information
 * @buf_addr: Address of data buffer
 * @dir: Channel direction
 * @bytes_xfer: # of bytes transferred
 * @transaction_status: Status of last trasnferred
 */
struct mhi_result {
	void *buf_addr;
	enum dma_data_direction dir;
	size_t bytes_xferd;
	int transaction_status;
};

/**
 * struct mhi_buf - Describes the buffer
 * @page: buffer as a page
 * @buf: cpu address for the buffer
 * @phys_addr: physical address of the buffer
 * @dma_addr: iommu address for the buffer
 * @skb: skb of ip packet
 * @len: # of bytes
 * @name: Buffer label, for offload channel configurations name must be:
 * ECA - Event context array data
 * CCA - Channel context array data
 */
struct mhi_buf {
	struct list_head node;
	struct page *page;
	void *buf;
	phys_addr_t phys_addr;
	dma_addr_t dma_addr;
	struct sk_buff *skb;
	size_t len;
	const char *name; /* ECA, CCA */
};

/**
 * struct mhi_driver - mhi driver information
 * @id_table: NULL terminated channel ID names
 * @ul_xfer_cb: UL data transfer callback
 * @dl_xfer_cb: DL data transfer callback
 * @status_cb: Asynchronous status callback
 */
struct mhi_driver {
	const struct mhi_device_id *id_table;
	int (*probe)(struct mhi_device *mhi_dev,
		     const struct mhi_device_id *id);
	void (*remove)(struct mhi_device *mhi_dev);
	void (*ul_xfer_cb)(struct mhi_device *mhi_dev, struct mhi_result *res);
	void (*dl_xfer_cb)(struct mhi_device *mhi_dev, struct mhi_result *res);
	void (*status_cb)(struct mhi_device *mhi_dev, enum MHI_CB mhi_cb);
	struct device_driver driver;
};

#define to_mhi_driver(drv) container_of(drv, struct mhi_driver, driver)
#define to_mhi_device(dev) container_of(dev, struct mhi_device, dev)

static inline void mhi_device_set_devdata(struct mhi_device *mhi_dev,
					  void *priv)
{
	mhi_dev->priv_data = priv;
}

static inline void *mhi_device_get_devdata(struct mhi_device *mhi_dev)
{
	return mhi_dev->priv_data;
}

/**
 * mhi_queue_transfer - Queue a buffer to hardware
 * All transfers are asyncronous transfers
 * @mhi_dev: Device associated with the channels
 * @dir: Data direction
 * @buf: Data buffer (skb for hardware channels)
 * @len: Size in bytes
 * @mflags: Interrupt flags for the device
 */
static inline int mhi_queue_transfer(struct mhi_device *mhi_dev,
				     enum dma_data_direction dir,
				     void *buf,
				     size_t len,
				     enum MHI_FLAGS mflags)
{
	if (dir == DMA_TO_DEVICE)
		return mhi_dev->ul_xfer(mhi_dev, mhi_dev->ul_chan, buf, len,
					mflags);
	else
		return mhi_dev->dl_xfer(mhi_dev, mhi_dev->dl_chan, buf, len,
					mflags);
}

static inline void *mhi_controller_get_devdata(struct mhi_controller *mhi_cntrl)
{
	return mhi_cntrl->priv_data;
}

static inline void mhi_free_controller(struct mhi_controller *mhi_cntrl)
{
	kfree(mhi_cntrl);
}

/**
 * mhi_driver_register - Register driver with MHI framework
 * @mhi_drv: mhi_driver structure
 */
int mhi_driver_register(struct mhi_driver *mhi_drv);

/**
 * mhi_driver_unregister - Unregister a driver for mhi_devices
 * @mhi_drv: mhi_driver structure
 */
void mhi_driver_unregister(struct mhi_driver *mhi_drv);

/**
 * mhi_device_configure - configure ECA or CCA context
 * For offload channels that client manage, call this
 * function to configure channel context or event context
 * array associated with the channel
 * @mhi_div: Device associated with the channels
 * @dir: Direction of the channel
 * @mhi_buf: Configuration data
 * @elements: # of configuration elements
 */
int mhi_device_configure(struct mhi_device *mhi_div,
			 enum dma_data_direction dir,
			 struct mhi_buf *mhi_buf,
			 int elements);

/**
 * mhi_device_get - disable all low power modes
 * Only disables lpm, does not immediately exit low power mode
 * if controller already in a low power mode
 * @mhi_dev: Device associated with the channels
 */
void mhi_device_get(struct mhi_device *mhi_dev);

/**
 * mhi_device_get_sync - disable all low power modes
 * Synchronously disable all low power, exit low power mode if
 * controller already in a low power state
 * @mhi_dev: Device associated with the channels
 */
int mhi_device_get_sync(struct mhi_device *mhi_dev);

/**
 * mhi_device_put - re-enable low power modes
 * @mhi_dev: Device associated with the channels
 */
void mhi_device_put(struct mhi_device *mhi_dev);

/**
 * mhi_prepare_for_transfer - setup channel for data transfer
 * Moves both UL and DL channel from RESET to START state
 * @mhi_dev: Device associated with the channels
 */
int mhi_prepare_for_transfer(struct mhi_device *mhi_dev);

/**
 * mhi_unprepare_from_transfer -unprepare the channels
 * Moves both UL and DL channels to RESET state
 * @mhi_dev: Device associated with the channels
 */
void mhi_unprepare_from_transfer(struct mhi_device *mhi_dev);

/**
 * mhi_get_no_free_descriptors - Get transfer ring length
 * Get # of TD available to queue buffers
 * @mhi_dev: Device associated with the channels
 * @dir: Direction of the channel
 */
int mhi_get_no_free_descriptors(struct mhi_device *mhi_dev,
				enum dma_data_direction dir);

/**
 * mhi_poll - poll for any available data to consume
 * This is only applicable for DL direction
 * @mhi_dev: Device associated with the channels
 * @budget: In descriptors to service before returning
 */
int mhi_poll(struct mhi_device *mhi_dev, u32 budget);

/**
 * mhi_ioctl - user space IOCTL support for MHI channels
 * Native support for setting  TIOCM
 * @mhi_dev: Device associated with the channels
 * @cmd: IOCTL cmd
 * @arg: Optional parameter, iotcl cmd specific
 */
long mhi_ioctl(struct mhi_device *mhi_dev, unsigned int cmd, unsigned long arg);

/**
 * mhi_alloc_controller - Allocate mhi_controller structure
 * Allocate controller structure and additional data for controller
 * private data. You may get the private data pointer by calling
 * mhi_controller_get_devdata
 * @size: # of additional bytes to allocate
 */
struct mhi_controller *mhi_alloc_controller(size_t size);

/**
 * of_register_mhi_controller - Register MHI controller
 * Registers MHI controller with MHI bus framework. DT must be supported
 * @mhi_cntrl: MHI controller to register
 */
int of_register_mhi_controller(struct mhi_controller *mhi_cntrl);

void mhi_unregister_mhi_controller(struct mhi_controller *mhi_cntrl);

/**
 * mhi_bdf_to_controller - Look up a registered controller
 * Search for controller based on device identification
 * @domain: RC domain of the device
 * @bus: Bus device connected to
 * @slot: Slot device assigned to
 * @dev_id: Device Identification
 */
struct mhi_controller *mhi_bdf_to_controller(u32 domain, u32 bus, u32 slot,
					     u32 dev_id);

/**
 * mhi_prepare_for_power_up - Do pre-initialization before power up
 * This is optional, call this before power up if controller do not
 * want bus framework to automatically free any allocated memory during shutdown
 * process.
 * @mhi_cntrl: MHI controller
 */
int mhi_prepare_for_power_up(struct mhi_controller *mhi_cntrl);

/**
 * mhi_async_power_up - Starts MHI power up sequence
 * @mhi_cntrl: MHI controller
 */
int mhi_async_power_up(struct mhi_controller *mhi_cntrl);
int mhi_sync_power_up(struct mhi_controller *mhi_cntrl);

/**
 * mhi_power_down - Start MHI power down sequence
 * @mhi_cntrl: MHI controller
 * @graceful: link is still accessible, do a graceful shutdown process otherwise
 * we will shutdown host w/o putting device into RESET state
 */
void mhi_power_down(struct mhi_controller *mhi_cntrl, bool graceful);

/**
 * mhi_unprepare_after_powre_down - free any allocated memory for power up
 * @mhi_cntrl: MHI controller
 */
void mhi_unprepare_after_power_down(struct mhi_controller *mhi_cntrl);

/**
 * mhi_pm_suspend - Move MHI into a suspended state
 * Transition to MHI state M3 state from M0||M1||M2 state
 * @mhi_cntrl: MHI controller
 */
int mhi_pm_suspend(struct mhi_controller *mhi_cntrl);

/**
 * mhi_pm_resume - Resume MHI from suspended state
 * Transition to MHI state M0 state from M3 state
 * @mhi_cntrl: MHI controller
 */
int mhi_pm_resume(struct mhi_controller *mhi_cntrl);

/**
 * mhi_download_rddm_img - Download ramdump image from device for
 * debugging purpose.
 * @mhi_cntrl: MHI controller
 * @in_panic: If we trying to capture image while in kernel panic
 */
int mhi_download_rddm_img(struct mhi_controller *mhi_cntrl, bool in_panic);

/**
 * mhi_force_rddm_mode - Force external device into rddm mode
 * to collect device ramdump. This is useful if host driver assert
 * and we need to see device state as well.
 * @mhi_cntrl: MHI controller
 */
int mhi_force_rddm_mode(struct mhi_controller *mhi_cntrl);

/**
 * mhi_get_remote_time_sync - Get external soc time relative to local soc time
 * using MMIO method.
 * @mhi_dev: Device associated with the channels
 * @t_host: Pointer to output local soc time
 * @t_dev: Pointer to output remote soc time
 */
int mhi_get_remote_time_sync(struct mhi_device *mhi_dev,
			     u64 *t_host,
			     u64 *t_dev);

/**
 * mhi_get_mhi_state - Return MHI state of device
 * @mhi_cntrl: MHI controller
 */
enum mhi_dev_state mhi_get_mhi_state(struct mhi_controller *mhi_cntrl);

/**
 * mhi_set_mhi_state - Set device state
 * @mhi_cntrl: MHI controller
 * @state: state to set
 */
void mhi_set_mhi_state(struct mhi_controller *mhi_cntrl,
		       enum mhi_dev_state state);


/**
 * mhi_is_active - helper function to determine if MHI in active state
 * @mhi_dev: client device
 */
static inline bool mhi_is_active(struct mhi_device *mhi_dev)
{
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;

	return (mhi_cntrl->dev_state >= MHI_STATE_M0 &&
		mhi_cntrl->dev_state <= MHI_STATE_M3);
}

/**
 * mhi_debug_reg_dump - dump MHI registers for debug purpose
 * @mhi_cntrl: MHI controller
 */
void mhi_debug_reg_dump(struct mhi_controller *mhi_cntrl);

#ifdef CONFIG_MHI_DEBUG

#define MHI_VERB(fmt, ...) do { \
		if (mhi_cntrl->klog_lvl <= MHI_MSG_LVL_VERBOSE) \
			pr_debug("[D][mhi%d][%s] " fmt, mhi_cntrl->cntrl_idx, __func__, ##__VA_ARGS__);\
} while (0)

#else

#define MHI_VERB(fmt, ...)

#endif

#define MHI_LOG(fmt, ...) do {	\
		if (mhi_cntrl->klog_lvl <= MHI_MSG_LVL_INFO) \
			pr_info("[I][mhi%d][%s] " fmt, mhi_cntrl->cntrl_idx, __func__, ##__VA_ARGS__);\
		else if (!mhi_cntrl->klog_slient) \
			printk(KERN_DEBUG "[I][mhi%d][%s] " fmt, mhi_cntrl->cntrl_idx, __func__, ##__VA_ARGS__);\
} while (0)

#define MHI_ERR(fmt, ...) do {	\
		if (mhi_cntrl->klog_lvl <= MHI_MSG_LVL_ERROR) \
			pr_err("[E][mhi%d][%s] " fmt, mhi_cntrl->cntrl_idx, __func__, ##__VA_ARGS__); \
} while (0)

#define MHI_CRITICAL(fmt, ...) do { \
		if (mhi_cntrl->klog_lvl <= MHI_MSG_LVL_CRITICAL) \
			pr_alert("[C][mhi%d][%s] " fmt, mhi_cntrl->cntrl_idx, __func__, ##__VA_ARGS__); \
} while (0)

int mhi_register_mhi_controller(struct mhi_controller *mhi_cntrl);
void mhi_unregister_mhi_controller(struct mhi_controller *mhi_cntrl);

#ifndef MHI_NAME_SIZE
#define MHI_NAME_SIZE 32
/**
 *  * struct mhi_device_id - MHI device identification
 *   * @chan: MHI channel name
 *    * @driver_data: driver data;
 *     */
struct mhi_device_id {
	const char chan[MHI_NAME_SIZE];
	unsigned long driver_data;
};
#endif

#endif /* _MHI_H_ */
