/*
 * Copyright (c) 2011 Quantenna Communications, Inc.
 */

/*
 * This file contains host definitions which are common between the
 * host driver and the microcontroller/MAC code.
 */
#ifndef _LHOST_MUC_COMM_H
#define _LHOST_MUC_COMM_H

#include "qtn_uc_comm.h"
#include "qtn_cca.h"
#include "qtn_wmm_ac.h"
#include "net80211/ieee80211.h"
#include "net80211/ieee80211_crypto.h"
#include "muc_txrx_stats.h"
#include "qtn/qvsp_common.h"
#include "qtn/shared_defs.h"

/* packed definitions for each compiler */
#if defined(MUC_BUILD) || defined(DSP_BUILD) || defined(AUC_BUILD)
# define PACKED	__packed
# define LM(a,b)	(b)
# define lhost_volatile
# define muc_volatile	volatile
#else
# define PACKED __attribute__ ((packed))
# define LM(a,b)	(a)
# define lhost_volatile	volatile
# define muc_volatile
#endif // #if defined(MUC_BUILD) || defined(DSP_BUILD) || defined(AUC_BUILD)

#define HOST_TXD_NUMSEG		2

#define QTN_BBIC_11N		0x30
#define QTN_BBIC_11AC		0x40

#define QTN_VSP_STATS_TID_NUM	4

#define QTN_VSP_TIDS	{ 6, 5, 0, 1 }
#define QTN_VSP_STATS_TID2IDX	{0, 1, -1, -1, -1, 2, 3, -1}	/* make sure no tids using same index */

struct qtn_vsp_per_node_stats {
	struct qtn_per_tid_stats per_tid_stats[QTN_VSP_STATS_TID_NUM];
};

struct qtn_vsp_stats {
#define QVSP_FAT_MAX		1000
#define QVSP_FAT_NONE		((uint32_t)(-1))
	uint32_t	fat;		/* free airtime */
	uint32_t	intf_ms;	/* interference */
#if TOPAZ_QTM
	struct qtn_vsp_per_node_stats per_node_stats[QTN_NCIDX_MAX];
#endif
};

/**
 * \brief This enumeration represents the mode in use on the device.
 *
 * This enumeration is used to set the correct bandwidth.
 */

enum {
        QTN_11NAC_DISABLE  = 0,
        QTN_11NAC_ENABLE = 1,
};

/* Host tx descriptor */
struct host_txdesc {
	uint32_t	hd_version:8;	/* Descriptor version */
	uint32_t	hd_tid:4;	/* packet tid */
	uint32_t	hd_txstatus:2;	/* Transmit status: 1 sent to MuC, 2 tx success */
#define QTN_TXSTATUS_TX_ON_MUC 1
#define QTN_TXSTATUS_TX_SUCCESS 2
	uint32_t	hd_wmmac:2;	/* Reserved for WMM AC*/
	uint32_t	hd_pktlen:16;	/* Pkt len (incl. all headers) */
	uint32_t	hd_node_idx;	/* local node index */
	uint16_t	hd_seglen[HOST_TXD_NUMSEG];	/* Segment lenghts */
	uint32_t	hd_segaddr[HOST_TXD_NUMSEG];	/* Phys addr of each seg */
	uint32_t	hd_ts;		/* Timestamp of the pkt */
	uint32_t	hd_nextpa;	/* Phys addr of next host tx descr in fwd dir */
	uint32_t	hd_nextpa_rev;	/* Phys addr of next host tx descr in rev dir */
	void		*hd_nextva_rev;	/* Virtual addr (LHOST view) of next host tx descr in rev dir */
	uint32_t	hd_pa;		/* Physical addr of this host tx descr */
	void		*hd_va;		/* Virtual addr (LHOST view) of this host tx descr */
	uint32_t		hd_status;	/* Status of HTxD */
	void		(*hd_muc_txdone_cb)(void *, uint32_t, uint32_t); /* MuC callback after txdone */
	uint32_t	hd_muc_cb_arg1; /* parameter for hd_muc_txdone_cb */
	uint32_t	hd_muc_cb_arg2; /* parameter for hd_muc_txdone_cb */
	uint32_t	hd_txtsf;	/* record the tsf_lo on that frame was sent successfully */
	uint8_t		hd_mpdu[128];
	uint8_t		hd_msdu[128];
	uint8_t		hd_dma[128];
#define	HTXD_FLAG_AMSDU_DEST_CAPABLE	0x00000002	/* Can be used for AMSDU destination (append to) */
#define	HTXD_FLAG_AMSDU_SRC_CAPABLE	0x00000004	/* Can be used for AMSDU (copy from) */
#define HTXD_FLAG_NO_UPDATE_NAV		0x00000008	/* Don't update NAV for this frame */
#define HTXD_FLAG_NO_RETRY		0x00000010	/* Don't retry this frame if tx failed */
#define HTXD_FLAG_NO_RETURN		0x00000020	/* Don't return txdesc from MuC to lhost */
#define HTXD_FLAG_IMM_RETURN		0x00000040	/* Immediately return txdesc from Muc to lhost */
	uint32_t	hd_flags;
};

#define QTN_AMSDU_DEST_CAPABLE_SIZE		ETHER_MAX_LEN
#define QTN_AMSDU_DEST_CAPABLE_GUARD_SIZE	64
#define QTN_AMSDU_SRC_FRAME_SIZE		(QTN_AMSDU_DEST_CAPABLE_SIZE / 10)
#define QTN_AMSDU_DEST_CAPABLE_OCCUPY_SIZE	(QTN_AMSDU_DEST_CAPABLE_SIZE / 3 * 2)

#define HTXD_FLAG_SET(_htxd, _flag) \
	(((struct host_txdesc *)(_htxd))->hd_flags |= (_flag))
#define	HTXD_FLAG_CLR(_htxd, _flag) \
	(((struct host_txdesc *)(_htxd))->hd_flags &= ~(_flag))
#define	HTXD_FLAG_GET(_htxd, _flag) \
	(((struct host_txdesc *)(_htxd))->hd_flags & (_flag))
#define	HTXD_FLAG_ISSET(_htxd, _flag) \
	(!!(((struct host_txdesc *)(_htxd))->hd_flags & (_flag)))
#define HTXD_FLAG_KEEP_ONLY(_htxd, _flag) \
	(((struct host_txdesc *)(_htxd))->hd_flags &= (_flag))


/* host_ioctl_hifinfo */

#define NAMESIZE		16
#define VERSION_SIZE		16
#define MAC_ADDR_LEN		6
#define MAC_STR_BUF_SIZE	18

#define	HOST_NUM_IOCTLQ		1	/* Number of ioctl q's */
/*
 * LHost -> MuC TX queues are per node to allow variable backpressure per node.
 * One universal management data frame tx mailbox, and one ioctl mailbox
 */
#define HOST_NUM_MGMTQ			1
#define	HOST_NUM_DATAQ			(QTN_NCIDX_MAX)
#define HOST_NUM_DATASEM		1

#define HOST_IOCTL_INDEX_BASE		0
#define HOST_MGMT_INDEX_BASE		(HOST_IOCTL_INDEX_BASE + HOST_NUM_IOCTLQ)
#define HOST_DATA_INDEX_BASE		(HOST_MGMT_INDEX_BASE + HOST_NUM_MGMTQ)
#define	HOST_NUM_HOSTIFQ		(HOST_NUM_DATAQ + HOST_NUM_IOCTLQ + HOST_NUM_MGMTQ)
#define HOST_MBOX_SIZE			(sizeof(uint32_t) * HOST_NUM_HOSTIFQ)

#define QTN_PHY_RATE_PROP_SCALE		1024
#define QTN_MUC_NODE_PKT_LIMIT_MIN	16
#define QTN_MUC_NODE_PKT_LIMIT_DEFAULT	64
#define QTN_MUC_NODE_PKT_LIMIT_MAX	128

#define IEEE80211_TXPOW_ENCODE(x)	((255 * 65536) + (x * 256) + 1)
#define IEEE80211_TXPOW_DECODE(x)	(((x) - (255 * 65536) - 1) / 256)
#define RF_MIXER_VAL_HI		0x1
#define RF_MIXER_VAL_LO		0x7
#define RF_PGA_VAL_HI		0x3
#define RF_PGA_VAL_LO		0x0
#define IEEE80211_LOWGAIN_TXPOW_MAX	10
#define IEEE80211_LOWGAIN_TXPOW_MIN	9

#define IEEE80211_CHAN_SEC_SHIFT	4

struct host_ioctl_hifinfo {
	uint32_t	hi_mboxstart;			/* Start address for mbox */
	uint32_t	hi_rxdoneirq;			/* IRQ map for rx done */
	uint32_t	hi_txdoneirq;			/* IRQ map for tx done */
	uint32_t	hi_rxfifo;			/* Rx FIFO location */
	uint32_t	hi_scanirq;			/* IRQ map for Scan */
	uint32_t	hi_scanfifo;			/* Scan FIFO location */
	uint32_t	hi_dspgpios;
	uint32_t	hi_vsp_stats_phys;
	uint32_t	hi_vapnode_idx;			/* node_idx of the vap node for tx */
	uint8_t		hi_vapid;
	char		hi_name[NAMESIZE];		/* Device name */
	char		hi_version[VERSION_SIZE];	/* basic firmware version */
	char		hi_algover[VERSION_SIZE];	/* calibration algorithm version */
	uint8_t		hi_macaddr[MAC_ADDR_LEN];
	uint8_t		hi_semmap[HOST_NUM_HOSTIFQ];	/* Mapping of semaphores */
};

typedef int (*scan_done_fn)(int sc_devid, void *chan, int type, int status);

struct host_scandesc {
	uint8_t	sd_type;
	uint8_t	sd_devid;
	uint8_t	status;
	uint8_t	____pad;
	uint8_t*	sd_data;
	scan_done_fn *sd_ppfn;
	struct host_scandesc *sd_next;
};

struct host_rxdesc {
	uint8_t			hw_desc[128]; /* need to be aligned on 8 bytes */
	uint8_t			*skbuff;
	uint8_t			*rd_buffer;
	uint32_t		rs_statword;
	struct host_rxdesc	*rd_next;
	struct host_rxdesc	*rd_pa;
	struct host_rxdesc	*rd_va;
	void			*node;		/* Where the frame was from */
	uint32_t		rx_status_word0;
	uint32_t		rx_status_qosctrl;
	uint32_t		rx_status_pnlo;
	uint32_t		rx_status_pnhi;
	uint8_t			gain_db;
};

struct host_descfifo {
	struct host_rxdesc	*df_fifo;		/* Pointer to first descriptor in linked list */
	volatile uint32_t	df_numelems;		/* Num elems on fifo */
	volatile uint32_t	df_size;		/* Size of fifo */
	struct host_rxdesc * volatile hrdstart; /* the ptr to the host_rxdesc linked list  ready for indication */
};

struct host_scanfifo {
	uint32_t	sf_req;	/* Pointer to request mailbox */
	uint32_t	sf_res;	/* Pointer to result mailbox */
	uint8_t	sf_sem;	/* Semaphore for Scan fifo */
	uint8_t	tx_sem; /* Semaphore for Scan fifo */
	uint8_t	____pad[2];
};

struct host_rxfifo {
	struct host_descfifo *rf_fifo;	/* Data Descriptor fifo */
	uint8_t	rf_sem;		/* Semaphore for rx fifo */
	uint8_t	____pad[3];
};

struct host_ndp_mesg {
	uint8_t	macaddr_ta[6];
	uint8_t	bw;
	uint8_t	rxgain;
	uint8_t	mcs;
	uint8_t	____pad[3];
};


struct host_ioctl {
	lhost_volatile uint32_t	ioctl_dev;	/* Device to run IOCTL on */
	lhost_volatile uint32_t	ioctl_command;	/* Command type */
	lhost_volatile uint32_t	ioctl_arg1;	/* Single valued arg 1 */
	lhost_volatile uint32_t	ioctl_arg2;	/* Single valued arg 2 */
	volatile uint32_t		ioctl_argp;	/* Argument payload pointer */
	volatile uint32_t		ioctl_status;	/* Status from other side */
	volatile uint32_t		ioctl_rc;	/* Command return code */
	lhost_volatile struct host_ioctl *ioctl_next;	/* link to next msg in chain */
};

struct qtn_vap_args {
	char vap_name[17];
	uint8_t vap_id;
	uint8_t vap_macaddr[IEEE80211_ADDR_LEN];
};

struct qtn_setparams_args
{
	int	ni_param;
	int	ni_value;
	int	ni_len;
	unsigned char ni_data[64];
};

struct qtn_baparams_args {
	unsigned char ni_addr[8];
	enum ieee80211_ba_state	state;
	int	tid;
	int	type;
	int	start_seq_num;
	int	window_size;
	int	lifetime;
	uint16_t flags;
};

#define QTN_HLINK_RC_DONE		0x00000001
#define QTN_HLINK_RC_ERR		0x00000002
#define QTN_HLINK_STATUS_AVAIL		1

#define	IOCTL_DEV_VAPCREATE		4	/* Create a vap */
#define IOCTL_DEV_DEVOPEN		5	/* Bring the device up */
#define IOCTL_DEV_BEACON_START		6	/* Start Beacon */
#define	IOCTL_DEV_NEWASSOC		7	/* New associated node */
#define	IOCTL_DEV_NEWBSSID		8	/* New associated node */
#define IOCTL_DEV_SEND_NDP_ANNOUNCEMENT	10	/* Send NDP announcement */
#define IOCTL_DEV_SETPARAMS		11	/* Configure Parameters */
#define IOCTL_DEV_GETPARAMS		12	/* Configure Parameters */
#define IOCTL_DEV_BA_ADDED_TX		13
#define IOCTL_DEV_BA_ADDED_RX		14
#define IOCTL_DEV_BA_REMOVED_TX		15
#define IOCTL_DEV_BA_REMOVED_RX		16
#define	IOCTL_DEV_CHANGE_CHANNEL	17
#define	IOCTL_DEV_SETKEY		18
#define IOCTL_DEV_CALCMD		19	/* Send the cal cmd */
#define	IOCTL_DEV_DELKEY		20
#define	IOCTL_DEV_CMD			21	/* General commands */
#define IOCTL_DEV_DISASSOC		22	/* Configure node */
#define	IOCTL_DEV_SMPS			23	/* MIMO power save mode change */
#define	IOCTL_DEV_FORCEMICERROR		24
#define IOCTL_DEV_SET_SCANMODE		25
#define IOCTL_DEV_XMITCTL		26	/* transmission control (turning on or off) */
#define IOCTL_DEV_BEACON_STOP		27	/* Stop transmitting beacons */
#define IOCTL_DEV_SET_MACADDR		30
#define IOCTL_DEV_KILL_MUC		31
#define IOCTL_DEV_DUMP_LOG		32
#define IOCTL_DEV_SET_HRFLAGS		33
#define IOCTL_DEV_SAMPLE_CHANNEL	34
#define	IOCTL_DEV_CHANGE_CHAN_DEFERRED	35
#define	IOCTL_DEV_WMM_PARAMS		36
#define IOCTL_DEV_VAPDELETE		37	/* Delete a vap */
#define IOCTL_DEV_STORE_TXPOW		38	/* Store the Tx power, short-range workaround*/
#define IOCTL_DEV_USE_RTS_CTS		39	/* Enable-disable RTS-CTS */
#define IOCTL_DEV_RST_QUEUE_DEPTH	40
#define IOCTL_DEV_SET_POWER_SAVE	41	/* send request to MuC to change power save level */
#define IOCTL_DEV_VSP			42	/* Configure QVSP */
#define IOCTL_DEV_SET_11G_ERP           43      /* set 11bg ERP on/off */
#define IOCTL_DEV_BGSCAN_CHANNEL	44
#define IOCTL_DEV_UPDATE_DTLS_PRESENT   45      /* update wheather DTLS session deteced or not */
#define IOCTL_DEV_SET_OCAC		46
#define IOCTL_DEV_MEAS_CHANNEL		47	/* notify MUC to execute measurement */
#define IOCTL_DEV_GET_LINK_MARGIN_INFO	48	/* get rssi info */
#define	IOCTL_DEV_SET_TDLS_PARAM	49	/* set tdls related paramters */
#define	IOCTL_DEV_GET_TDLS_PARAM	50	/* set tdls related paramters */
#define	IOCTL_DEV_POWER_SAVE		51	/* enter/leave power save state */
#define	IOCTL_DEV_REMAIN_CHANNEL	52	/* Remain on target channel */
#define IOCTL_DEV_SCS_UPDATE_SCAN_STATS	53
#define IOCTL_DEV_SET_SCANMODE_STA	54
#define IOCTL_DEV_GET_MU_GRP		55	/* get MU groups other releated data */
#define IOCTL_DEV_SET_RX_GAIN_PARAMS	56	/* Set RX gain params */
#define IOCTL_DEV_GET_MU_ENABLE		57	/* get MU enable flag */
#define IOCTL_DEV_GET_PRECODE_ENABLE	58	/* get MU precode enable flag */
#define IOCTL_DEV_GET_MU_USE_EQ		59	/* get EQ enable flag */
#define IOCTL_DEV_SET_CHAN_POWER_TABLE	60	/* Set MuC power table */
#define	IOCTL_DEV_ENABLE_VLAN		61	/* Set Global Vlan mode */
#define	IOCTL_DEV_NODE_UPDATE		62	/* Update node information again after association */
#define IOCTL_DEV_FWT_SW_SYNC		63	/* sync FWT timestamp base between Lhost and MUC */

#define IOCTL_DEV_CMD_MEMDBG_DUMP	1	/* Dump MuC memory */
#define IOCTL_DEV_CMD_MEMDBG_DUMPCFG	2	/* Configuration for dumping MuC memory */
#define IOCTL_DEV_CMD_MEMDBG_DUMPNODES	3	/* Configuration for dumping MuC nodes */
#define IOCTL_DEV_CMD_SET_DRV_DBG	4	/* Set MUC debug message level*/
#define IOCTL_DEV_CMD_GET_DRV_DBG	5	/* Get MUC debug message level*/

#define	IOCTL_DEVATTACH_DEVFLAG_MASK			0xFFFF0000
#define	IOCTL_DEVATTACH_DEVFLAG_MASK_S			16
#define	IOCTL_DEVATTACH_DEVID_MASK			0x000000FF
#define	IOCTL_DEVATTACH_DEV_RFCHIP_FREQID_MASK		0x00000F00
#define	IOCTL_DEVATTACH_DEV_RFCHIP_FREQID_MASK_S	8
#define	IOCTL_DEVATTACH_DEV_RFCHIP_VERID_MASK		0x0000F000
#define	IOCTL_DEVATTACH_DEV_RFCHIP_VERID_MASK_S		12
#define	IOCTL_DEVATTACH_IRQNUM				0x000000FF
#define	IOCTL_DEVATTACH_IRQREG				0x00000F00
#define	IOCTL_DEVATTACH_IRQREG_S			8
#define	IOCTL_DEVATTACH_NMBOX_MASK			0x000000FF

#define QTN_CHAN_IEEE			(0xFF << 0)
#define QTN_CHAN_IEEE_S			(0)
#define QTN_CHAN_PWR			(0xFF << 8)
#define QTN_CHAN_PWR_S			(8)

#define QTNCHAN_TO_IEEENUM(chan)	(MS(chan, QTN_CHAN_IEEE))

#define QTN_CHAN_FLG_DFS		0x20000000
#define QTN_CHAN_FLG_HT40		0x40000000
#define QTN_CHAN_FLG_PRI_HI		0x80000000
#define QTN_CHAN_FLG_RSV01		0x01000000
#define QTN_CHAN_FLG_RSV02		0x02000000
#define QTN_CHAN_FLG_RSV04		0x04000000
#define QTN_CHAN_FLG_RSV08		0x08000000
#define QTN_CHAN_FLG_RSV10		0x10000000

#define QTN_CHAN_FLG_VHT80		0x00800000

#define QTN_BAND_FREQ			(0xFF << 0)
#define QTN_BAND_FREQ_S			(0)

#define	IOCTL_HLINK_DEVATTACH		1	/* Attach device */
#define	IOCTL_HLINK_DEVDETACH		2	/* Detach device */
#define	IOCTL_HLINK_DEVCHANGE		3	/* Change device state/flags */
#define IOCTL_HLINK_LOGATTACH		4	/* Attach Log */
#define IOCTL_HLINK_TEMP_ATTACH		5	/* Share temperature struct */
#define IOCTL_HLINK_SVCERRATTACH	6	/* Attach svcerr */
#define IOCTL_HLINK_RTNLEVENT		7	/* RTNL event */
#define IOCTL_HLINK_NDP_FRAME		8	/* NDP frame */
#define IOCTL_HLINK_FOPS_REQ		9	/* Recv File I/O req */
#define IOCTL_HLINK_MIC_ERR		10	/* TKIP MIC failure detected */
#define IOCTL_HLINK_BOOTED		11	/* MuC boot complete */
#define IOCTL_HLINK_DROP_BA		12	/* drop BA */
#define IOCTL_HLINK_DISASSOC_STA	13	/* disassociate station with a given aid */
#define IOCTL_HLINK_RFIC_CAUSED_REBOOT  14      /* detected RFIC abnormal reset, reboot the system */
#define IOCTL_HLINK_BA_ADD_START	15	/* start Tx ADDBA REQ sequence */
#define IOCTL_HLINK_PEER_RTS		16	/* Peer RTS enable or disable */
#define IOCTL_HLINK_DYN_WMM		17	/* Dynamic WMM enable or disable */
#define IOCTL_HLINK_TDLS_EVENTS		18	/* TDLS Events from MuCfw */
#define IOCTL_HLINK_RATE_TRAIN		19	/* Per-node rate training hash */

enum {
	BW_INVALID = 0,
	BW_HT20 = 20,
	BW_HT40 = 40,
	BW_HT80 = 80,
	BW_HT160 = 160
};

/* Fixed bw command offset */
#define QTN_BW_FIXED_BW		0x3
#define QTN_BW_FIXED_BW_S	0
#define QTN_BW_FIXED_EN		0x10
#define QTN_BW_FIXED_EN_S	4

enum {
	QTN_DISABLE_PN_VALIDATION = 0,
	QTN_ENABLE_PN_VALIDATION = 1
};

struct qtn_csa_info {
	uint64_t	req_tsf;		/* aim to change channels at this tsf */
	uint64_t	switch_tsf;		/* tsf just after channel change completed */
	uint32_t	pre_notification_tu;	/* pre-switch notification to lhost in TU */
	uint32_t	post_notification_tu;	/* post channel change notification */
	uint32_t	freq_band;		/* freqency band info */
	uint32_t	channel;		/* channel to switch to */

#define QTN_CSA_STATUS_MUC_SCHEDULED		0x00000001
#define QTN_CSA_STATUS_MUC_ERROR_SCHED		0x00000010
#define QTN_CSA_STATUS_MUC_PRE			0x00000002
#define QTN_CSA_STATUS_MUC_SWITCHED		0x00000004
#define QTN_CSA_STATUS_MUC_POST			0x00000008
#define QTN_CSA_STATUS_MUC_ERROR_SW		0x00000010
#define QTN_CSA_STATUS_MUC_CANCELLED		0x00000020
#define QTN_CSA_STATUS_MUC_COMPLETE		0x00000040
	uint32_t	muc_status;		/* status written by MuC */

#define QTN_CSA_RESTART_QUEUE			0x00000001
#define QTN_CSA_STATUS_LHOST_PRE_DONE		0x00000002
#define QTN_CSA_STATUS_LHOST_SWITCH_DONE	0x00000004
#define QTN_CSA_STATUS_LHOST_POST_DONE		0x00000008
#define QTN_CSA_CANCEL				0x00000010
#define QTN_CSA_STATUS_LHOST_ACTIVE		0x00000020
#define QTN_CSA_STATUS_LHOST_UNITS_OFFSET	0x00000040
	uint32_t	lhost_status;		/* flags written by lhost */
};

#define MEAS_RPI_HISTOGRAM_SIZE		8

enum meas_reason {
	QTN_MEAS_REASON_SUCC = 0,
	QTN_MEAS_REASON_OFF_CHANNEL_UNSUPPORT,
	QTN_MEAS_REASON_DURATION_TOO_SHORT,
	QTN_MEAS_REASON_TIMER_SCHED_FAIL,
	QTN_MEAS_REASON_TYPE_UNSUPPORT,
	QTN_MEAS_REASON_MAX,
};

enum meas_type {
	QTN_MEAS_TYPE_BASIC = 0,
	QTN_MEAS_TYPE_CCA,
	QTN_MEAS_TYPE_RPI,
	QTN_MEAS_TYPE_CHAN_LOAD,
	QTN_MEAS_TYPE_NOISE_HIS,
	QTN_MEAS_TYPE_MAX,
};

struct meas_time_slice {
	uint32_t meas_slice;	/* time slice */
	uint32_t meas_time_pri;	/* prime time count based on meas_slice */
	uint32_t meas_time_sec; /* secondary time count based on meas_slice */
};

struct qtn_meas_chan_info {
	uint32_t work_channel;			/* working channel to return to */
	int32_t	meas_type;			/* measurement type */
	int32_t	meas_reason;			/* measurement reason */
	struct meas_time_slice time_slice;	/* time slice for measurement long duration */
	uint32_t meas_channel;
	uint64_t meas_start_tsf;
	uint32_t meas_dur_ms;
	union {
		struct {
			uint32_t cca_busy_cnt;
			uint32_t cca_try_cnt;
			uint32_t cca_try_ms;
			uint32_t cca_busy_ms;
		} cca_and_chanload;
		uint8_t rpi_counts[MEAS_RPI_HISTOGRAM_SIZE];
		int32_t basic_radar_num;
		uint8_t basic;
	} inter_data;
};

enum scs_lot_tsf_pos {
	SCS_LOG_TSF_POS_LHOST_TASK_KICKOFF,
	SCS_LOG_TSF_POS_LHOST_IOCTL2MUC,
	SCS_LOG_TSF_POS_MUC_POLL_IOCTL_FROM_LHOST,
	SCS_LOG_TSF_POS_MUC_QOSNULL_SENT,
	SCS_LOG_TSF_POS_MUC_SMPL_START_BEFORE_CHAN_CHG,
	SCS_LOG_TSF_POS_MUC_SMPL_START_AFTER_CHAN_CHG,
	SCS_LOG_TSF_POS_MUC_SMPL_FINISH_BEFORE_CHAN_CHG,
	SCS_LOG_TSF_POS_MUC_SMPL_FINISH_AFTER_CHAN_CHG,
	SCS_LOG_TSF_POS_LHOST_CCA_INTR,
	SCS_LOG_TSF_POS_LHOST_CCA_WORK,
	SCS_LOG_TSF_POS_NUM
};

#define IEEE80211_SCS_LOG_TSF(_ic, _sample, _pos)	((_ic)->ic_get_tsf(&((_sample)->tsf[(_pos)])))
#define QDRV_SCS_LOG_TSF(_sample, _pos)			(hal_get_tsf(&((_sample)->tsf[(_pos)])))
#define MUC_SCS_LOG_TSF(_qh, _sample, _pos)		(hal_get_tsf((_qh), &((_sample)->tsf[(_pos)])))

struct qtn_samp_chan_info {
	struct out_cca_info	result;		/* results structure for CCA measurement */
	uint32_t		freq_band;
	uint32_t		samp_channel;	/* The channel on which sample will be taken */
	uint32_t		duration_msecs;	/* Duration in milliseconds to stay on off channel */
	uint64_t		start_tsf;	/* tsf at which to start sampling */

#define QTN_CCA_STATUS_IDLE		0x0
#define QTN_CCA_STATUS_HOST_IOCTL_SENT	0x1
#define QTN_CCA_STATUS_MUC_SCHEDULED	0x2
#define QTN_CCA_STATUS_MUC_STARTED	0x3
#define QTN_CCA_STATUS_MUC_COMPLETE	0x4
#define QTN_CCA_STATUS_MUC_CANCELLED	0x5
	uint32_t		status;

#define QTN_CCA_TYPE_BACKGROUND		0x1
#define QTN_CCA_TYPE_DIRECTLY		0x2
	uint32_t		type;

	uint32_t		qosnull_txdesc_host;	/* qosnull frame for channel sampling */
	uint32_t		qosnull_txdesc_bus;	/* qosnull frame in phyaddr */
	uint16_t		qosnull_frame_len;	/* the frame length of qosnull */
	uint16_t		tx_node_idx;		/* the node index that qosnull frame to */
	uint32_t		qosnull_txtsf;		/* the tsf_lo read from MAC on that qosnull frame was sent successfully */
	uint32_t		qosnull_nav;		/* the large NAV in qosnull frame */
	uint64_t		tsf[SCS_LOG_TSF_POS_NUM];	/* timestamps used for precise time control and profiling */
};

#define QTN_SCS_ASSOC_STA_MAX	12

struct qtn_scs_vsp_node_stats {
	uint32_t ni_associd;
	uint32_t tx_usecs;
	uint32_t rx_usecs;
};

struct qtn_scs_vsp_info {
	uint32_t	num_of_assoc;
	struct  qtn_scs_vsp_node_stats scs_vsp_node_stats[QTN_SCS_ASSOC_STA_MAX];
};

struct qtn_scs_scan_info {
	uint32_t	bw_sel;
	uint32_t	cca_idle;
	uint32_t	cca_busy;
	uint32_t	cca_tx;
	uint32_t	cca_intf;
	uint32_t	cca_try;
	uint32_t	bcn_rcvd;
	uint32_t	crc_err;
	uint32_t	lpre_err;
	uint32_t	spre_err;
};

#define QTN_SCS_MAX_OC_INFO	32
struct qtn_scs_oc_info {
	uint32_t	off_channel;
	uint32_t	off_chan_bw_sel;
	uint32_t	off_chan_cca_busy;
	uint32_t	off_chan_cca_sample_cnt;
	uint32_t	off_chan_cca_try_cnt;
	uint32_t	off_chan_beacon_recvd;
	uint32_t	off_chan_crc_errs;
	uint32_t	off_chan_sp_errs;
	uint32_t	off_chan_lp_errs;
};
/* Smart channel selection data shared between Lhost and MuC */
struct qtn_scs_info {
	uint32_t	oc_info_count;
	struct qtn_scs_oc_info oc_info[QTN_SCS_MAX_OC_INFO];
	uint32_t	bw_sel;
	uint32_t	cca_try;
	uint32_t	cca_busy;
	uint32_t	cca_idle;
	uint32_t	cca_tx;
	uint32_t	cca_interference;
	uint32_t	beacon_recvd;
	uint32_t	tx_usecs;
	uint32_t	rx_usecs;
	struct qtn_scs_vsp_info scs_vsp_info;
};

struct qtn_scs_info_set {
	uint32_t	valid_index; /* 0 or 1 */
	struct qtn_scs_info scs_info[2];
	struct qtn_scs_scan_info scan_info[IEEE80211_CHAN_MAX];
};

struct qtn_remain_chan_info {
	uint32_t chipid;
	uint32_t data_channel;		/* Data channel to return to */
	uint32_t off_channel;		/* The required remain channel */
	uint32_t duration_usecs;	/* Duration in microseconds to stay on remain channel */
	uint64_t start_tsf;		/* tsf at which to switch to remain channel */

#define QTN_REM_CHAN_STATUS_IDLE		0x0
#define QTN_REM_CHAN_STATUS_HOST_IOCTL_SENT	0x1
#define QTN_REM_CHAN_STATUS_MUC_SCHEDULED	0x2
#define QTN_REM_CHAN_STATUS_MUC_STARTED	0x3
#define QTN_REM_CHAN_STATUS_MUC_COMPLETE	0x4
#define QTN_REM_CHAN_STATUS_MUC_CANCELLED	0x5
	uint32_t status;		/* channel switch status */

	uint8_t peer_mac[IEEE80211_ADDR_LEN];	/* peer node mac address */
};


#define QTN_CCA_CNT2MS(_cnt)   RUBY_TIMER_MUC_CCA_CNT2MS(_cnt)
#define QTN_CCA_INTV           RUBY_TIMER_MUC_CCA_INTV

enum scan_chan_tsf_pos {
	SCAN_CHAN_TSF_LHOST_HOSTLINK_IOCTL = 0,
	SCAN_CHAN_TSF_MUC_IOCTL_PROCESS,
	SCAN_CHAN_TSF_MUC_SEND_START_FRM,
	SCAN_CHAN_TSF_MUC_SEND_START_FRM_DONE,
	SCAN_CHAN_TSF_MUC_GOTO_OFF_CHAN,
	SCAN_CHAN_TSF_MUC_GOTO_OFF_CHAN_DONE,
	SCAN_CHAN_TSF_MUC_SEND_PRBREQ_FRM,
	SCAN_CHAN_TSF_MUC_SEND_PRBREQ_FRM_DONE,
	SCAN_CHAN_TSF_MUC_GOTO_DATA_CHAN,
	SCAN_CHAN_TSF_MUC_GOTO_DATA_CHAN_DONE,
	SCAN_CHAN_TSF_MUC_SEND_FINISH_FRM,
	SCAN_CHAN_TSF_MUC_SEND_FINISH_FRM_DONE,
	SCAN_CHAN_TSF_LHOST_INTERRUPT,
	SCAN_CHAN_TSF_LHOST_SCANWORK,
	SCAN_CHAN_TSF_LOG_NUM
};

struct scan_chan_tsf_dbg {
	int pos_index;
	char *log_name;
};

#define QDRV_SCAN_LOG_TSF(_scan, _pos)		(hal_get_tsf(&((_scan)->tsf[(_pos)])))
#define MUC_SCAN_LOG_TSF(_qh, _scan, _pos)	(hal_get_tsf((_qh), &((_scan)->tsf[(_pos)])))

struct qtn_scan_chan_info {
	uint32_t	freq_band;
	uint32_t	scan_channel;		/* The channel on which sample will be taken */
#define TIME_MARGIN_BEFORE_STARTFRM	3000	/* microseconds, time overhead for others before start frame is sent*/
#define TIME_MARGIN_AFTER_STARTFRM	1000	/* microseconds, time overhead for others after start frame is sent*/
#define TIME_OFFSET_SEND_PROBE_REQ	3000	/* microseconds, the time offset for sending probe_req frame
						 * after switching to off channel*/
#define TIME_OFFSET_SEND_START_FRM	5000	/* microseconds, the time offset for sending start frame
						 * after set NETDEV_F_PAUSE_TX flag */
#define TIME_DUR_FOR_ALL_BEACONS	25000	/* microseconds, the time duration for transmitting all beacons */
#define TIME_MIN_WAIT_PROBE_REP		5000	/* microseconds, the minimal time for waiting for the probe
						 * response frame on scanning channel */
	uint32_t	dwell_msecs;		/* Duration in milliseconds to stay on scanning channel */
#define QTN_SCAN_CHAN_MUC_IDLE			0x0
#define QTN_SCAN_CHAN_MUC_STARTED		0x1
#define QTN_SCAN_CHAN_MUC_PROBING		0x2
#define QTN_SCAN_CHAN_MUC_COMPLETED		0x3
#define QTN_SCAN_CHAN_MUC_FAILED		0x4
#define QTN_SCAN_CHAN_MUC_SCHEDULED		0x5
	uint32_t	muc_status;		/* written only by MuC */
#define QTN_SCAN_CHAN_FLAG_ACTIVE		0x00000001
#define QTN_SCNA_CHAN_FLAG_PASSIVE_FAST		0x00000002
#define QTN_SCNA_CHAN_FLAG_PASSIVE_NORMAL	0x00000004
#define QTN_SCNA_CHAN_FLAG_PASSIVE_SLOW		0x00000008
#define QTN_SCAN_CHAN_TURNOFF_RF		0x00000010
	uint32_t	scan_flags;
	uint32_t	start_txdesc_host;	/* The frame sent before go scan channel,
						 * e.g. pwrsav frame in STA mode */
	uint32_t	start_txdesc_bus;	/* Start frame in phyaddr */
	uint16_t	start_node_idx;		/* the node index that frame to */
	uint16_t	start_frame_len;	/* frame length */
	uint32_t	prbreq_txdesc_host;	/* probe request frame for active scanning */
	uint32_t	prbreq_txdesc_bus;	/* probe request frame in phyaddr */
	uint16_t	prbreq_node_idx;	/* the node index that frame to */
	uint16_t	prbreq_frame_len;	/* frame length */
	uint32_t	finish_txdesc_host;	/* The frame sent after back data channel,
						 * e.g. the frame to announce waking up in STA mode */
	uint32_t	finish_txdesc_bus;	/* Complete frame in phyaddr */
	uint16_t	finish_node_idx;		/* the node index that frame to */
	uint16_t	finish_frame_len;	/* frame length */
	uint64_t	tsf[SCAN_CHAN_TSF_LOG_NUM];
};

enum qtn_ocac_tsf_log {
	OCAC_TSF_LOG_GOTO_OFF_CHAN = 0,
	OCAC_TSF_LOG_GOTO_OFF_CHAN_DONE,
	OCAC_TSF_LOG_GOTO_DATA_CHAN,
	OCAC_TSF_LOG_GOTO_DATA_CHAN_DONE,
	OCAC_TSF_LOG_NUM
};

struct qtn_ocac_info {
	uint32_t		freq_band;	/* frequency band, written by lhost */
	uint32_t		off_channel;	/* The off channel, "0" means to stop ocac in MuC, written by lhost*/
	uint32_t		qosnull_txdesc_host;	/* qosnull frame in virtual address, written by lhost */
	uint32_t		qosnull_txdesc_bus;	/* qosnull frame in physical address, written by lhost */
	uint16_t		qosnull_frame_len;	/* the frame length of qosnull */
	uint16_t		tx_node_idx;		/* the node index that qosnull frame to */
	uint16_t		dwell_time;	/* the required time on off channel in one beacon interval, written by lhost */
	uint16_t		secure_dwell;	/* milliseconds, the time on off channel within on off-channel action, using
							qosnull frame with large NAV to protect the traffic */
	uint16_t		threshold_fat;	/* the fat threshold to run off-channel CAC, written by lhost */
	uint16_t		threshold_traffic;	/* the traffic threshold to run off-channel CAC, written by lhost */
	uint16_t		threshold_fat_dec;	/* the threshold for consecutive fat decrease, written by lhost */
	uint16_t		traffic_ctrl;	/* whether to send qosnull or not, written by lhost */
	uint16_t		offset_txhalt;	/* milliseconds, the offset after beacon to halt tx, written by lhost */
	uint16_t		offset_offchan;	/* milliseconds, the offset after halt tx to switch off channel, written by lhost */

#define QTN_OCAC_ON_DATA_CHAN	0x1
#define QTN_OCAC_ON_OFF_CHAN	0x2
	uint16_t		chan_status;	/* current on which channel, written by MuC */
	uint16_t		actual_dwell_time;	/* the actual time on off channel, written by MuC */
	uint64_t		tsf_log[OCAC_TSF_LOG_NUM];	/* event tsf log, written by MuC */
};

struct qtn_rf_rxgain_params
{
	uint8_t *gain_entry_tbl;
	uint8_t lna_on_indx;
	uint8_t max_gain_idx;
	uint16_t cs_threshold_value;
};

/* MuC fops requst */
#define MUC_FOPS_MAX_FNAME_SIZE (50)
enum {
	MUC_FOPS_OPEN = 0,
	MUC_FOPS_READ,
	MUC_FOPS_WRITE,
	MUC_FOPS_LSEEK,
	MUC_FOPS_CLOSE,
};

enum {
	MUC_FOPS_PENDING = 0x011ADDED,
	MUC_FOPS_DONE    = 0xF035D0DE,
};

enum {
	MUC_FOPS_RDONLY = 0x0,
	MUC_FOPS_WRONLY = 0x1,
	MUC_FOPS_RDWR 	= 0x2,
	MUC_FOPS_APPEND = 0x4,
};

struct muc_fops_req	{
	volatile int32_t ret_val;
	volatile int32_t fd;
	volatile uint32_t req_state;
	volatile char *data_buff;
};

enum qdrv_cmd_muc_memdbgcnf_s {
	QDRV_CMD_MUC_MEMDBG_STATUS,
	QDRV_CMD_MUC_MEMDBG_FD_MAX,
	QDRV_CMD_MUC_MEMDBG_NODE_MAX,
	QDRV_CMD_MUC_MEMDBG_DUMP_MAX,
	QDRV_CMD_MUC_MEMDBG_RATETBL,
	QDRV_CMD_MUC_MEMDBG_MSG_SEND,
	QDRV_CMD_MUC_MEMDBG_TRACE,
	QDRV_CMD_MUC_MEMDBG_LAST
};

/* The following file indexes and file lists must all be kept in sync */
#define FOPS_FD_EP_SAMPLES		9
#define FOPS_FD_DCACHE_SAMPLES		10
#ifdef PROFILE_MUC_SAMPLE_IPTR_AUC
#define FOPS_FD_IPTR_SAMPLES		15
#else
#define FOPS_FD_IPTR_SAMPLES		11
#endif
#define FOPS_FD_UBOOT_ENV		12

#define LHOST_CAL_FILES		{	\
	NULL,				\
	"/proc/bootcfg/bf_factor",	\
	"/tmp/txpower.txt",		\
	"/proc/bootcfg/txpower.cal",	\
	"/proc/bootcfg/dc_iq.cal",	\
	"/mnt/jffs2/mon.out",		\
	"/mnt/jffs2/gmon.out",		\
	"/mnt/jffs2/pecount.out",	\
	"/proc/bootcfg/pdetector.cal",	\
	"/mnt/jffs2/profile_ep_muc",	\
	"/mnt/jffs2/profile_dcache_muc",\
	"/mnt/jffs2/profile_iptr_muc",	\
	"/proc/bootcfg/env",		\
	"/etc/mtest",			\
	"/proc/bootcfg/rx_iq.cal",	\
	"/mnt/jffs2/profile_iptr_auc",	\
}

#define MUC_CAL_FILES		{	\
	NULL,				\
	NULL,				\
	NULL,				\
	NULL,				\
	NULL,				\
	"mon.out",			\
	"gmon.out",			\
	NULL,				\
	NULL,				\
	NULL,				\
	NULL,				\
	NULL,				\
	NULL,				\
}

enum tdls_ioctl_params {
	IOCTL_TDLS_STATUS = 1,
	IOCTL_TDLS_UAPSD_IND_WND,
	IOCTL_TDLS_PTI_CTRL,
	IOCTL_TDLS_PTI,
	IOCTL_TDLS_PTI_PENDING,
	IOCTL_TDLS_DBG_LEVEL,
	IOCTL_TDLS_PTI_DELAY,
	IOCTL_TDLS_PTI_EVENT = 100
};

struct qtn_tdls_args {
	uint8_t		ni_macaddr[IEEE80211_ADDR_LEN];
	uint16_t	ni_ncidx;
	uint32_t	tdls_cmd;
	uint32_t	tdls_params;
};

struct qtn_node_args
{
	/* header */
	uint8_t	ni_macaddr[IEEE80211_ADDR_LEN];
	uint8_t	ni_bssid[IEEE80211_ADDR_LEN];
	uint8_t	ni_nrates;
	uint8_t	ni_rates[IEEE80211_RATE_MAXSIZE];
	uint8_t	ni_htnrates;
	uint8_t	ni_htrates[IEEE80211_HT_RATE_MAXSIZE];
	uint16_t	ni_associd;	/* assoc response */
	uint16_t	ni_node_idx;
	uint16_t	ni_flags;	/* special-purpose state */
	struct wmm_params	wmm_params[WME_NUM_AC];
	uint8_t	ni_implicit_ba_rx; /* The RX side of the implicit BA. Zero for no implicit RX BA */
	uint8_t	ni_implicit_ba_tx; /* The TX side of the implicit BA. Zero for no implicit TX BA */
	uint16_t	ni_implicit_ba_size; /* Size of the implicit BAs */
	uint8_t	ni_qtn_ie_flags;
	uint8_t ni_vendor;
	uint8_t ni_bbf_disallowed;      /* flag to disallow BBF */
	uint8_t ni_std_bf_disallowed;      /* flag to disallow standard BF */
	uint8_t ni_uapsd;	/* U-APSD per-node flags matching WMM STA Qos Info field */
	uint8_t	ni_htcap[sizeof(struct ieee80211_htcap)];	/* Processed HT capabilities */
	uint8_t	ni_htinfo[sizeof(struct ieee80211_htinfo)];	/* Processed HT info */
	uint8_t	ni_vhtcap[sizeof(struct ieee80211_vhtcap)];	/* Processed VHT capabilities */
	uint8_t	ni_vhtop[sizeof(struct ieee80211_vhtop)];	/* Processed VHT operational info */
	struct qtn_node_shared_stats *ni_shared_stats;
	uint32_t	ni_ver_sw;
	uint32_t	ni_qtn_flags;
	uint32_t	ni_tdls_status;
	uint8_t		ni_mu_grp[sizeof(struct ieee80211_vht_mu_grp)];
	uint16_t	ni_rsn_caps;		/* optional rsn capabilities */
	uint8_t		rsn_ucastcipher;	/* selected unicast cipher */
	uint16_t	tdls_peer_associd;	/* tdls peer AID allocated by AP, unique in BSS */
	uint32_t	ni_rate_train;
	uint32_t	ni_rate_train_peer;
};

struct qtn_beacon_args
{
	uint32_t	pkt_data;
	struct wmm_params	wmm_params[WME_NUM_AC];
	uint32_t	bintval;
	uint32_t	bo_tim;
	uint32_t	bo_tim_len;
	uint32_t	bo_tpc_rep;
	uint32_t	bo_chanswitch;
	uint32_t	bo_htcap;
	uint32_t	bo_htinfo;
	uint32_t	bo_vhtcap;
	uint32_t	bo_vhtop;
};

struct qtn_key {
	u_int8_t wk_keylen;		/* key length in bytes */
	u_int8_t wk_flags;
#define IEEE80211_KEY_XMIT	0x01	/* key used for xmit */
#define IEEE80211_KEY_RECV	0x02	/* key used for recv */
#define IEEE80211_KEY_GROUP	0x04	/* key used for WPA group operation */
#define IEEE80211_KEY_SWCRYPT	0x10	/* host-based encrypt/decrypt */
#define IEEE80211_KEY_SWMIC	0x20	/* host-based enmic/demic */
	u_int16_t wk_keyix;		/* key index */
	u_int8_t wk_key[IEEE80211_KEYBUF_SIZE + IEEE80211_MICBUF_SIZE];
#define wk_txmic    wk_key + IEEE80211_KEYBUF_SIZE + 0  /* XXX can't () right */
#define wk_rxmic    wk_key + IEEE80211_KEYBUF_SIZE + 8  /* XXX can't () right */
	u_int64_t wk_keyrsc[IEEE80211_RSC_MAX];    /* key receive sequence counter */
	u_int64_t wk_keytsc;		/* key transmit sequence counter */
	u_int32_t wk_cipher;		/* cipher */
	u_int32_t wk_ncidx;		/* node cache index */
};
#define IEEE80211_KEY_COMMON		/* common flags passed in by apps */\
	(IEEE80211_KEY_XMIT | IEEE80211_KEY_RECV | IEEE80211_KEY_GROUP)

struct qtn_key_args
{
	struct qtn_key	key;
	uint8_t	wk_addr[IEEE80211_ADDR_LEN];
};

struct qtn_power_save_args
{
	uint32_t enable;
	uint8_t ni_addr[IEEE80211_ADDR_LEN];
};

struct lhost_txdesc
{
	struct host_txdesc	hw_desc;	/* shared between muc and lhost */
	struct sk_buff		*skb;
	struct lhost_txdesc	*next;
};

#define MUC_TXSTATUS_READY		0x0
#define MUC_TXSTATUS_DONE		0x1

#define MUC_RXSTATUS_DONE		0x1

#define MUC_RXSTATUS_RXLEN		0xFFFF0000
#define MUC_RXSTATUS_RXLEN_S		16

struct qtn_link_margin_info {
	uint32_t	mcs;
	uint32_t	bw;
	int		rssi_avg;
	int		reason;
#define QTN_LINK_MARGIN_REASON_SUCC		0
#define QTN_LINK_MARGIN_REASON_NOSUCHNODE	1
	uint8_t		mac_addr[IEEE80211_ADDR_LEN];
};

#define QTN_RESERVED_DEVIDS		2
#define QTN_WLANID_FROM_DEVID(devid)	\
	((devid < QTN_RESERVED_DEVIDS)? 0 : (devid - QTN_RESERVED_DEVIDS))

struct qtn_mu_grp_args {
	/* MU group ID. 0 means the group is not used and grp_ni is empty*/
	uint8_t grp_id;
	/* mu QMat installation status */
	/* QMat is not installed and not used */
#define MU_QMAT_DISABLED	0
	/* QMat is installed and used */
#define MU_QMAT_ENABLED		1
	/* QMat is installed, used but not updated */
#define MU_QMAT_FREEZED		2
	/* QMat is installed, not used and not updated */
#define MU_QMAT_NOT_USED	3
	uint8_t qmat_installed;
	/* the index of the grp_ni[], is also the user position */
	uint16_t aid[IEEE80211_MU_GRP_NODES_MAX];
	uint8_t ncidx[IEEE80211_MU_GRP_NODES_MAX];
	/* matrix addr offsets in sram */
	unsigned int u0_1ss_u1_1ss;
	unsigned int u0_2ss_u1_1ss;
	unsigned int u0_3ss_u1_1ss;
	unsigned int u0_1ss_u1_2ss;
	unsigned int u0_1ss_u1_3ss;
	unsigned int u0_2ss_u1_2ss;
	/* stats */
	uint32_t upd_cnt;
	int32_t rank;
};

struct qtn_fwt_sw_params {
	uint32_t muc_hz;
	uint32_t muc_jiffies_base;
	uint64_t *muc_fwt_ts_mirror;
	uint64_t *muc_fwt_ts_mirror_bus;
	uint64_t m2h_op;
};

#endif	// _LHOST_MUC_COMM_H

